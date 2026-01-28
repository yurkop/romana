#include "decoder.h"
#include <iostream>
#define ID_ADCM 0x2A50
#define ID_CMAP 0x504D
#define ID_EVNT 0x5645
#define ID_CNTR 0x5443

extern CRS *crs;
extern HClass *hcl;
extern Coptions cpar;
extern Toptions opt;

//
// Long64_t max_time_diff; // 1 секунда, например
// double ring_used; // процент использования кольцевого буф (выводить в
// виджете)
UInt_t max_deque_size =
    30; // максимальный размер очереди из буферов (Bufevents)
int threads = 4; // количество потоков

int print2 = 0; // 0: no print; 1: print

void prnt2(const char *fmt, ...) {
  if (!print2)
    return;
  va_list args;
  va_start(args, fmt);
  vprnt(fmt, args);
  va_end(args);
}

void pr_zero(pulsecontainer &B, const char* txt) {
  int nn=0;
  for (auto it = B.begin(); it != B.end(); it++) {
    if (it->Tstamp64 == 0) {
      prnt("ss s l d d ls;", KRED, "Zero Tstamp64 in:", txt, B.size(), nn, it->Chan,
           it->Tstamp64, RST);
    }
    nn++;
  }
}

Decoder::Decoder() {
  // exchange возвращает старое значение (false) и заменяет его на true
  if (!tables_initialized.exchange(true)) { // Atomic, thread-safe
    init_all_tables(); // ОДНА функция инициализации всех таблиц
  }
  Decoder_Reset();
}
Decoder::~Decoder() { Decoder_Stop(); }

void Decoder::Decoder_Reset() {
  Decoder_Stop(); // ← Сначала остановить все потоки
  next_buffer_id = 0;
  decode_queue.clear();
  Bufevents.clear();
  Bufevents.emplace_back();
  events_end = std::prev(Bufevents.end()); // Указывает на "вечный end"
  events_end->source_buffer_id=UINT_MAX;
  //events_end->flag.store(0); // Специальный флаг, чтобы не обрабатывать
  boundary = events_end; // в начале boundary указывает на end

  Nevents.store(0, std::memory_order_release);
  Bufsize.store(0, std::memory_order_release);
  // Levents.clear();

  // // Сбрасываем параметры коррекции времени
  // T64_offset.store(0, std::memory_order_release);
  // offset_buffer_id.store(UINT_MAX, std::memory_order_release);
  // offset_flag.store(false, std::memory_order_release);

  {
    std::lock_guard<std::mutex> lock(active_mutex);
    active_read_starts.clear();
  }
}

void Decoder::Decoder_Resize(Long64_t r_size, Long64_t o_size) {
  // r_size - nominal size of buffer (на самом деле + o_size)
  // o_size - offset size (offset в начале - перед буфером)

  // NOTE: write_start ДОЛЖЕН быть выровнен по 8 байт
  // иначе *Buf.u82.ul вызовет SIGBUS на ARM/RISC-V
  // Поэтому o_size выравнивается до кратного 8
  o_size = (o_size + 7) & ~7ULL; // округление вверх до кратного 8

  buffer_storage.resize(o_size + r_size);

  Buf_ring.write_start = buffer_storage.data() + o_size;
  Buf_ring.write_end = Buf_ring.write_start + r_size;
  Buf_ring.u82.b = Buf_ring.write_start;

  Buf_ring.analysis_start = Buf_ring.write_start;
  Buf_ring.analysis_end = Buf_ring.write_start;

  ring_size = r_size;

  // BufProc.write_start = Buf_ring.write_start;
  // BufProc.write_end = Buf_ring.write_end;
  // BufProc.u82.b = Buf_ring.write_start;

  // буфер: -o_size .. 0 .. r_size
  // указатели:  write_start .. b .. write_end
}

void Decoder::Decoder_Start(Long64_t r_size, Long64_t o_size, bool b_acq,
                            int module) {

  Decoder_Resize(r_size, o_size);

  crs_module = module;
  // Bufevents.clear();

  for (int i = 0; i < MAX_CH; i++) {
    b_len[i] = opt.B2[i] - opt.B1[i] + 1;
    p_len[i] = opt.P2[i] - opt.P1[i] + 1;
    w_len[i] = opt.W2[i] - opt.W1[i] + 1;
    b_mean[i] = (opt.B2[i] + opt.B1[i]) * 0.5;
    p_mean[i] = (opt.P2[i] + opt.P1[i]) * 0.5;
    w_mean[i] = (opt.W2[i] + opt.W1[i]) * 0.5;
  }

  write_ptr.store(Buf_ring.write_start, std::memory_order_release);

  // стартуем потоки Decode и копирования
  Ana_Start();
  MakeEvent_Start();
  Resorting_Start();
  Decode_Start(threads);
  // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  if (b_acq)
    Copy_Start();
}

void Decoder::Decoder_Stop() /*noexcept*/ {
  try {
    // Останавливаем копирование
    if (copy_running.load(std::memory_order_acquire)) {
      copy_running.store(false, std::memory_order_release);
      copy_cond.notify_all(); // Будим поток копирования

      // Ждем пока поток обработает ВСЕ данные из очереди
      if (copy_thread && copy_thread->joinable()) {
        copy_thread->join();
        // Поток сам выйдет когда queue пуста И copy_running=false
      }
      copy_thread.reset(); // освобождаем память
    }

    // Останавливаем декодирование
    if (decode_running.load(std::memory_order_acquire)) {
      prnt("sss;", BYEL, "Decode_stop!!!!!!!", RST);
      decode_running.store(false, std::memory_order_release);
      decode_cond.notify_all(); // Будим потоки декодирования

      for (auto &thread : decode_threads) {
        if (thread && thread->joinable()) {
          thread->join();
        }
        // thread.reset(); - не нужен, т.к. clear все очищает
      }
      decode_threads.clear();
      prnt("sss;", BYEL, "Decode_stop2!!!!!!!", RST);
    }

    // Останавливаем пересортировку
    if (resorting_running.load(std::memory_order_acquire)) {
      prnt("sss;", BYEL, "Resorting_stop!!!!!!!", RST);
      resorting_running.store(false, std::memory_order_release);
      resorting_cond.notify_all(); // Будим поток пересортировки

      if (resorting_thread && resorting_thread->joinable()) {
        resorting_thread->join();
      }
      resorting_thread.reset(); // освобождаем память
      prnt("sss;", BYEL, "Resorting_stop2!!!!!!!", RST);
    }

    // Останавливаем создание событий
    if (makeevent_running.load(std::memory_order_acquire)) {
      prnt("sss;", BYEL, "Make_event_stop!!!!!!!", RST);
      makeevent_running.store(false, std::memory_order_release);
      makeevent_cond.notify_all(); // Будим поток создания событий

      if (makeevent_thread && makeevent_thread->joinable()) {
        makeevent_thread->join();
      }
      makeevent_thread.reset(); // освобождаем память
      prnt("sss;", BYEL, "Make_event_stop2!!!!!!!", RST);
    }

    // Останавливаем анализ событий
    if (ana_running.load(std::memory_order_acquire)) {
      prnt("sss;", BYEL, "Ana_stop!!!!!!!", RST);
      ana_running.store(false, std::memory_order_release);
      ana_cond.notify_all(); // Будим поток анализа событий

      if (ana_thread && ana_thread->joinable()) {
        ana_thread->join();
      }
      ana_thread.reset(); // освобождаем память
      prnt("sss;", BYEL, "Ana_stop2!!!!!!!", RST);
    }

    // Очистка очередей не нужна, т.к. они уже должны быть пустыми
  } catch (...) {
    // Логируем, но подавляем исключения
    std::cerr << "Exception in Decoder_Stop()" << std::endl;
  }
}

void Decoder::Copy_Start() {
  copy_running.store(true, std::memory_order_release);
  copy_thread = std::make_unique<std::thread>([this]() { Copy_Worker(); });
}

void Decoder::Decode_Start(int num_threads) {
  num_decode_threads = num_threads;
  decode_running.store(true, std::memory_order_release);
  decode_threads.clear();

  // decode_threads.reserve(num_threads); // если вектор: ВЫДЕЛЯЕМ ПАМЯТЬ
  // ЗАРАНЕЕ!

  // std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Создаем рабочие потоки декодера и инициализируем atomic указатели
  // worker_ptrs.clear();
  for (UInt_t i = 0; i < num_decode_threads; ++i) {
    decode_threads.push_back(
        std::make_unique<std::thread>([this, i]() { Decode_Worker(i); }));
    // worker_ptrs.emplace_back(Buf_ring.write_start);
  }
}

void Decoder::Resorting_Start() {
  resorting_running.store(true, std::memory_order_release);
  resorting_thread = std::make_unique<std::thread>(
      [this]() { Resorting_Worker(); });
}

void Decoder::MakeEvent_Start() {
  makeevent_running.store(true, std::memory_order_release);
  makeevent_thread = std::make_unique<std::thread>(
      [this]() { MakeEvent_Worker(); });
}

void Decoder::Ana_Start() {
  ana_running.store(true, std::memory_order_release);
  ana_thread = std::make_unique<std::thread>([this]() { Ana_Worker(); });
}

// Рабочий поток копирования
void Decoder::Copy_Worker() {
  prnt2("ss ds;", BGRN, "Copy worker started with timeout", copy_timeout_ms,
        RST);

  while (true) {
    std::list<BufClass> local_queue;
// --- WAIT: ожидание данных ---
    {
#ifdef PROFILING
      SimpleTimer timer(this, "Copy_Wait");
#endif
      // Общее правило:
      // Для ВСЕХ wait_for с таймаутом нужно:
      //     Проверять условие остановки потока (!running && empty())
      //     Проверять на таймаут (очередь/список пуст после wait_for)
      //     Использовать continue для пропуска итерации при таймауте
      //     Обеспечивать освобождение мьютекса перед continue/break

      // 1. Ждем данные под замком с таймаутом
      std::unique_lock<std::mutex> lock(copy_mutex);
      copy_cond.wait_for(lock, std::chrono::milliseconds(copy_timeout_ms),
                         [this]() {
                           return !copy_queue.empty() ||
                                  !copy_running.load(std::memory_order_acquire);
                         });
      // Если поток остановлен и очередь пуста, выходим
      if (!copy_running.load(std::memory_order_acquire) && copy_queue.empty())
        break;

      // 2. Быстро забираем ВСЕ данные под замком если copy_queue не пуста
      if (copy_queue.empty())
        continue;
      local_queue.swap(copy_queue); // O(1) операция!
    } // ОСВОБОЖДАЕМ замок сразу после доступа к очереди

    {
#ifdef PROFILING
      SimpleTimer timer(this, "Copy_Exec");
#endif
      try {
        // throw std::runtime_error("Test exception");
        // throw "C-string exception";

        // конец предыдущей записи в Buf_ring: новый write_start
        UChar_t *prev_write_end = write_ptr.load(std::memory_order_acquire);

        // prnt2("ss f d ls;", BGRN, "Copy0:", ring_used, next_buffer_id,
        //       Bufevents.size(), RST);
        bool written = false;
        for (auto &item : local_queue) {
          Long64_t data_size = item.write_end - item.write_start;
          if (!CanWriteData(data_size)) {
            prnt("ss fs;", BRED, "Data dropped. Available space(%):", ring_used,
                 RST);
            continue;
          }

          written = true;
          Buf_ring.Ring_Write(item);
          write_ptr.store(Buf_ring.u82.b, std::memory_order_release);
        }

        // cout << "pos: " << hex << pos << dec << " " << first_call << endl;

        if (written) {
          // prev_write_end = конец предыдущей записи в Buf_ring: новый
          // write_start.
          // Buf_ring.u82 = кoнец текущей записи в Buf_ring: новый
          // write_end.
          // analysis_end = конец предыдущего анализа в Buf_ring: новый
          // analysis_start
          BufClass range(prev_write_end, Buf_ring.u82.b, Buf_ring.analysis_end);

          /*
          // все в процентах от общего размера буфера, относительно начала
          буфера
          // w1 = конец предыдущей записи в Buf_ring: новый write_start
          // w2 = кoнец текущей записи в Buf_ring: новый write_end
          // a1 = конец предыдущего анализа в Buf_ring: новый analysis_start
          double w1 = 100.0 * (prev_write_end - Buf_ring.write_start) /
          ring_size; double w2 = 100.0 * (Buf_ring.u82.b - Buf_ring.write_start)
          / ring_size; double a1 = 100.0 * (Buf_ring.analysis_end -
          Buf_ring.write_start) / ring_size;
          // размер последней записи
          double write_size = 100.0 * (Buf_ring.u82.b - prev_write_end) /
          ring_size;
          // на сколько запись опережает анализ
          double ahead = 100.0 * (prev_write_end - Buf_ring.analysis_end) /
          ring_size; prnt("s f f f f f;", "w1 w2 a1 sz ahead:", w1, w2, a1,
          write_size, ahead);
          */

          // prnt2("ss f d ls;", BGRN, "Copy1:", ring_used, next_buffer_id,
          //       Bufevents.size(), RST);
          Send_for_Decode(
              range); // range будет инвалидирован в SendToDecodeQueue
        }

      } catch (const std::exception &e) {
        prnt("ss ss;", BRED, "Exception in Copy_Worker:", e.what(), RST);
      } catch (...) {
        prnt("sss;", BRED, "Unknown exception in Copy_Worker", RST);
      }
    } // замер завершён
  }

  prnt2("sss;", BGRN, "Copy worker finished", RST);
}

bool Decoder::CanWriteData(Long64_t data_size) {
  // return true; // временная заглушка

  // Берет текущую позицию записи: current_write = write_ptr
  // Для каждого worker'а вычисляет worker_ptr: указывает на КОНЕЦ данных,
  // которые worker БУДЕТ обрабатывать.
  // Находит БЛИЖАЙШИЙ worker_ptr: Выбирает worker, который ближе всего к
  //   текущей позиции записи - это и есть самый "опасный" ограничитель.
  // Вычисляет безопасный предел
  //     : Позицию, до которой можно безопасно писать.
  // возвращает true, если есть место для записи
  const Long64_t SAFETY_MARGIN = 8;
  // writer никогда не приблизится к worker_ptr'ам ближе, чем эта граница

  UChar_t *current_write = write_ptr.load(std::memory_order_acquire);

  // cout << "Can1: " << (void*) current_write << " " <<
  // active_read_starts.size() << endl;

  // Находим ближайший worker_ptr
  Long64_t min_distance = LLONG_MAX;

  // Вместо worker_ptrs — используем active_read_starts
  {
    std::lock_guard<std::mutex> lock(active_mutex);
    for (UChar_t *ptr : active_read_starts) {
      Long64_t distance;

      if (ptr > current_write) {
        distance = ptr - current_write;
      } else {
        distance =
            (ptr - Buf_ring.write_start) + (Buf_ring.write_end - current_write);
      }

      // cout << "distance: " << distance << " " << ring_size << " " << (void*)
      // ptr << endl;
      if (distance < min_distance) {
        min_distance = distance;
      }
    }
  }

  // Если нет активных потоков — максимальное расстояние (всё свободно)
  if (min_distance == LLONG_MAX) {
    min_distance = ring_size;
  }

  // cout << "Can2" << endl;
  ring_used = 100.0 * (1 - double(min_distance) / ring_size);

  // cout << "ring_used: " << ring_used << " " << min_distance << " " <<
  // ring_size << endl;

  // cout << "Can3" << endl;
  return min_distance >= data_size + SAFETY_MARGIN;
}

/*
bool Decoder::CanWriteData(Long64_t data_size) {
  // return true; // временная заглушка


  // Берет текущую позицию записи: current_write = write_ptr
  // Для каждого worker'а вычисляет worker_ptr: указывает на КОНЕЦ данных,
  // которые worker БУДЕТ обрабатывать.
  // Находит БЛИЖАЙШИЙ worker_ptr: Выбирает worker, который ближе всего к
  //   текущей позиции записи - это и есть самый "опасный" ограничитель.
  // Вычисляет безопасный предел
  //     : Позицию, до которой можно безопасно писать.
  // возвращает true, если есть место для записи

  const Long64_t SAFETY_MARGIN = 8;
  // writer никогда не приблизится к worker_ptr'ам ближе, чем эта граница

  UChar_t *current_write = write_ptr.load(std::memory_order_acquire);
  //UChar_t *safe_limit = Buf_ring.write_start;

  // Находим ближайший worker_ptr
  Long64_t min_distance = LLONG_MAX;
  for (auto &worker_ptr : worker_ptrs) {
    UChar_t *ptr = worker_ptr.load(std::memory_order_acquire);
    Long64_t distance;

    if (ptr > current_write) {
      distance = ptr - current_write;
    } else { //при равенстве distance будет равняться размеру буфера
      distance =
          (ptr - Buf_ring.write_start) + (Buf_ring.write_end - current_write);
    }

    if (distance < min_distance) {
      min_distance = distance;
    }
  }
  ring_used = 100.0 * (1 - double(min_distance) / ring_size);

  cout << "ring_used: " << ring_used << " " << min_distance << " " << ring_size
<< endl;

  return min_distance >= data_size + SAFETY_MARGIN; //
}
*/

void Decoder::Send_for_Decode(BufClass &range) {
  if (PrepareDecodeBuffer(range))
    SendToDecodeQueue(range);
  else // если проблема, конец анализа перемещаем на конец записи, анализ не
       // запускаем
    Buf_ring.analysis_end = Buf_ring.u82.b;
}

// Вспомогательные методы для Send_for_Decode
bool Decoder::PrepareDecodeBuffer(BufClass &range) {
  if (!CheckBufferRanges(range) || !HandleRingBufferWrap(range) ||
      !FindLast(range)) {
    return false;
  }
  return true;
}

bool Decoder::CheckBufferRanges(BufClass &range) {
  // проверяем границы буферов
  // было ">=", но это приводило к проблеме в начале анализа, т.к. в начале они
  // равны
  if (range.analysis_start > range.write_start) {
    double a1 =
        100.0 * (range.analysis_start - Buf_ring.write_start) / ring_size;
    double w1 = 100.0 * (range.write_start - Buf_ring.write_start) / ring_size;
    double delta =
        100.0 * (range.write_start - range.analysis_start) / ring_size;
    prnt("ss f f fs;", BRED, "Analysis ahead of write:", a1, w1, delta, RST);
    return false;
  }
  return true;
}

// отслеживет переход через конец буфера
bool Decoder::HandleRingBufferWrap(BufClass &range) {
  if (range.write_end < range.write_start) {
    // это значит, что запись перешла через конец кольцевого буфера ->
    // копируем необработанные данные из конца буфера в офсет (перед началом)
    size_t data_size =
        range.write_start - range.analysis_start; // необработанный кусок буфера
    UChar_t *new_analysis_start = Buf_ring.write_start - data_size;

    if (new_analysis_start < buffer_storage.data()) {
      double sz = 100.0 * data_size / ring_size;
      double offset =
          100.0 * (Buf_ring.write_start - buffer_storage.data()) / ring_size;
      prnt("ss f s fs;", BRED, "RingBuf offset overflow. Data size(%):", sz,
           "offset(%):", offset, RST);
      return false;
    }

    memmove(new_analysis_start, range.analysis_start, data_size);
    range.analysis_start = new_analysis_start;
  }
  // else: range.analysis_start не изменился

  return true;
}

bool Decoder::FindLast(BufClass &range) {
  // ищем последний импульс - от конца записи до начала анализа
  // анализ будет до начала последнего импульса

  // конец записи должен быть позже начала анализа
  if (range.write_end < range.analysis_start) {
    double w2 = 100.0 * (range.write_end - Buf_ring.write_start) / ring_size;
    double a1 =
        100.0 * (range.analysis_start - Buf_ring.write_start) / ring_size;
    prnt("ss f fs;", BRED, "Write end before analysis start:", w2, a1, RST);
    return false;
  }
  union82 from_pos;
  from_pos.b = range.write_end;
  Buf_ring.analysis_end = range.analysis_end =
      FindEvent_backward(from_pos, range.analysis_start);
  return true;
}

void Decoder::SendToDecodeQueue(BufClass &range) {
  // СОЗДАЕМ DecodeItem
  DecodeItem item;
  // СОЗДАЕМ EventBuf и записываем в item
  item.local_buf_itr = Bufevents.emplace(events_end);
  item.local_buf_itr->source_buffer_id = next_buffer_id;
  range.buffer_id = next_buffer_id;
  item.buf = std::move(range);

  // обновляем boundary - границу удаления буферов
  // Примечание: предполагается, что `SendToDecodeQueue` вызывается одним
  // потоком (single-producer). Если вызовы станут конкурентными, нужно защитить
  // это присваивание, например:
  //   std::lock_guard<std::mutex> lock(boundary_mutex);
  //   if (boundary == Bufevents.end()) boundary = Bufevents.begin();
  if (boundary == events_end)
    boundary = Bufevents.begin();

  // if (offset_flag.load(std::memory_order_acquire)) {
  //   // если offset_flag, значит уже в другом потоке офсет найден и определен
  //   offset_flag.store(false, std::memory_order_release); //сбрасываем
  //   // записываем offset_buffer_id
  //   if (local_buf_itr->source_buffer_id <
  //   offset_buffer_id.load(std::memory_order_acquire)) {
  //     offset_buffer_id.store(local_buf_itr->source_buffer_id,
  //     std::memory_order_release);
  //   }
  // }

  // prnt2("ss f d ls;", BGRN, "Copy2:", ring_used, next_buffer_id,
  //       Bufevents.size(), RST);

  // Добавляем в decode_queue (защищаем мьютексом)
  {
    std::lock_guard<std::mutex> lock(decode_mutex);
    decode_queue.push_back(std::move(item));
    next_buffer_id++;
    decode_cond.notify_one();
  }
}

// НОВЫЙ Рабочий поток декодирования
void Decoder::Decode_Worker(UInt_t thread_id) {
  prnt2("ss d ds;", BGRN,
        "Decode worker started with timeout:", decode_timeout_ms, thread_id,
        RST);

  while (true) {
    DecodeItem item;
    bool got_item = false;

    // Пытаемся быстро взять ОДИН буфер под мьютксом
    {
      std::unique_lock<std::mutex> lock(decode_mutex);
      decode_cond.wait_for(
          lock, std::chrono::milliseconds(decode_timeout_ms), [this]() {
            return !decode_queue.empty() ||
                   !decode_running.load(std::memory_order_acquire);
          });

      // ПРОВЕРКА на decode_running: выходим сразу, если очередь пуста
      if (!decode_running.load(std::memory_order_acquire) &&
          decode_queue.empty())
        break;

      if (!decode_queue.empty()) {
        // Берем первый буфер из очереди (FIFO)
        item = std::move(decode_queue.front());
        decode_queue.pop_front();
        got_item = true;

        // // Обновляем позицию worker'а
        // worker_ptrs[thread_id].store(item.buf.write_end,
        //                              std::memory_order_release);
      }
    } // ОСВОБОЖДАЕМ замок сразу после доступа к очереди

    // Обрабатываем один буфер БЕЗ мьютекса
    if (got_item) {

      // Добавляем в active_read_starts
      UChar_t *my_read_start = item.buf.write_end;

      std::list<UChar_t *>::iterator my_it;
      {
        std::lock_guard<std::mutex> lock(active_mutex);
        //active_read_starts.push_back(my_read_start);
        //my_it = std::prev(active_read_starts.end());
        my_it = active_read_starts.insert(active_read_starts.end(), my_read_start);
      }

      try {
        // // Тестовые исключения
        // static int test_counter = 0;
        // if (test_counter++ == 5) {
        // 	throw std::runtime_error("Test exception after 5 iterations");
        // }
        // throw "C-string exception";  // нестандартное исключение

        pr_zero(item.local_buf_itr->PLS,"before");
        Decode_switch(item.buf, item.local_buf_itr->PLS);
        // std::this_thread::sleep_for(std::chrono::milliseconds(70));
        pr_zero(item.local_buf_itr->PLS,"after");

      } catch (...) {
        // ПРОСТАЯ ЗАГЛУШКА при ошибке обработки

        // При падении обработки буфера не теряется его идентификатор,
        // буфер может быть неполный, но помечается как готовый.
        // Это предотвращает зависание Resorting_Worker, система продолжает
        // работать, просто пропустив проблемный буфер.

        prnt("ss d l s;", BRED, "Exception in Decode_Worker", thread_id,
             item.buf.buffer_id, RST);
      }

      {
        std::lock_guard<std::mutex> lock(buf_mutex);
        // Устанавливаем флаг готовности (лучше все под мьютексом)
        item.local_buf_itr->flag.store(1, std::memory_order_release);
        //  уведомляем resorting_worker
        resorting_cond.notify_one();
      }

      // // t1 - первый импульс, t2 - последний
      // double t1 = 0, t2 = 0;
      // if (!item.local_buf_itr->B.empty()) {
      //   t1 = item.local_buf_itr->B.front().Tstamp64 / 1e9 * opt.Period;
      //   t2 = item.local_buf_itr->B.back().Tstamp64 / 1e9 * opt.Period;
      // }
      // prnt("ss d d s l l f fs;", BCYN, "Thread/buf", thread_id,
      //       item.buf.buffer_id, "done. Bufevents:", Bufevents.size(),
      //       item.local_buf_itr->B.size(), t1, t2, RST);

      // удаляем из списка активных
      {
        std::lock_guard<std::mutex> lock(active_mutex);
        active_read_starts.erase(my_it);
      }
    }
  }

  prnt2("ss d ls;", BGRN, "Decode worker finished, thread:", thread_id,
        decode_queue.size(), RST);
} // Decode_Worker

void Decoder::Resorting_Worker() {

  max_time_diff = 1 * 1e9 / opt.Period; // 1 секунда

  /*
  const char *worker_name = "Resorting";
  int wait_value = 1;
  int stor_value = 2;
  std::condition_variable *cond = &resorting_cond;
  std::condition_variable *next_cond = &makeevent_cond;
  UInt_t timeout_ms = resorting_timeout_ms;
  std::atomic<bool> *running = &resorting_running;
  char* col=(char*)KMAG;
  std::mutex *mut = &buf_mutex;
  */

  prnt2("ss ds;", BGRN, "Resorting worker started with timeout",
        resorting_timeout_ms, RST);

  // itr указывает на последний ОБРАБОТАННЫЙ буфер
  // next указывает на следующий буфер после обработанного
  auto itr = events_end; // в начале оба указывают на конец
  auto next = itr;
  // NOTE: next — итератор на буфер, готовый к обработке.
  // Инициализируется как events_end, обновляется через ++next.
  // Критически важен для поддержания порядка обработки.
  // NOTE: itr и next управляют цепочкой обработки буферов.
  // При достижении конца (next == events_end) цикл продолжает ожидание новых
  // буферов. Условие wait_for корректно обрабатывает events_end, не вызывая
  // разыменования.

  while (true) {
    // 1. ЖДЁМ под мьютексом
    {
#ifdef PROFILING
      SimpleTimer timer(this, "Resort_Wait");
#endif
      std::unique_lock<std::mutex> lock(buf_mutex);

      // Условие: ждём пока itr не будет ready
      resorting_cond.wait_for(lock, std::chrono::milliseconds(resorting_timeout_ms), [&]() {
        if (!resorting_running.load(std::memory_order_acquire))
          return true;
        next = (itr == events_end) ? Bufevents.begin() : std::next(itr);
        if (next == events_end)
          return false;
        return next->flag.load(std::memory_order_acquire) == 1;
      });

      // next указывает на конец или на неготовый буфер
      if (next == events_end ||
          next->flag.load(std::memory_order_acquire) != 1) {
        // prnt("ss d ls;", KGRN, "Resorting: No buffers:", next->source_buffer_id, Bufevents.size(),
        //      RST);
        // нет готовых буферов
        if (resorting_running.load(std::memory_order_acquire))
          continue; // продолжаем
        else
          break; // Выходим
      }
      // есть готовый буфер
      // else {
      //   prnt("ss d ls;", BGRN,
      //        "Resorting: woke up. Next buffer:", next->source_buffer_id,
      //        Bufevents.size(), RST);
      // }
    } // конец мьютекса

    // 2. ОБРАБАТЫВАЕМ
    // bool processed_any = false;
    {
#ifdef PROFILING
      SimpleTimer timer(this, "Resort_Exec");
#endif
      // cout << "Resorting: " << next->source_buffer_id << endl;
      while (next != events_end &&
             next->flag.load(std::memory_order_acquire) == 1) {
        // проверяем на конец списка (может быть, если выход по таймауту)

        bool moved=false;
        // Процессинг буфера - без мьютекса
        try {
          double t1 = -3, t2 = -3;
          if (next->PLS.size()) {
            t1 = next->PLS.front().Tstamp64 / 1e9 * opt.Period;
            t2 = next->PLS.back().Tstamp64 / 1e9 * opt.Period;
          }
          // prnt("ss d l f fs;", BCYN, "R1:", next->source_buffer_id,
          //      next->B.size(), t1, t2, RST);

          // Пересортрировка одного буфера
          ResortSingleBuffer(next);
          next->flag.store(2, std::memory_order_release);

          t1 = -3, t2 = -3;
          if (next->PLS.size()) {
            t1 = next->PLS.front().Tstamp64 / 1e9 * opt.Period;
            t2 = next->PLS.back().Tstamp64 / 1e9 * opt.Period;
          }
          // prnt("ss d l f fs;", BBLU, "R2:", next->source_buffer_id,
          //      next->B.size(), t1, t2, RST);

          // Обновляем (сдвигаем вправо) границу удаления буферов
          moved=UpdateBoundary(next);
          // prnt("ss d ds;", BYEL, "U:", next->source_buffer_id,
          //      boundary->source_buffer_id, RST);

        } catch (...) {
          prnt("ss ds;", BRED, "Exception in resorting_worker",
               next->source_buffer_id, RST);
        }

        // processed_any = true;


        if (moved) { // блокируем мьютекс
          std::lock_guard<std::mutex> lock(buf_mutex);
          // next->flag.store(3, std::memory_order_release);
          makeevent_cond.notify_one();
        } // lock

        prnt2("ss ds;", KMAG, "Resorting done:", next->source_buffer_id, RST);
        itr = next;
        ++next;

      } // while (внутренний)
    } // замер завершён

    // if (processed_any)
    //   prnt2("ss s ls;", col, worker_name,
    //        "processed. Remaining:", Bufevents.size(), RST);

    // cout << "make event4" << endl;
  } // while (внешний)

  prnt("ss ls;", BGRN, "Resorting worker finished. Bufevents:", Bufevents.size(), RST);
}

// Ищем от нового к старому
// Запоминаем последний подходящий буфер
// Если находим буфер где импульс будет в середине → сразу вставляем
// Если встречаем слишком старый буфер → возвращаемся к последнему подходящему
// Если дошли до начала → вставляем в начало самого старого подходящего
void Decoder::ResortSingleBuffer(std::list<EventBuf>::iterator itr) {
  // itr указывает на текущий буфер, который нужно пересортировать
  if (itr == Bufevents.begin() || itr->PLS.empty()) {
    // Это первый буфер, не с чем сравнивать или он пустой
    return;
  }

  // if (!itr->B.empty()) {
  //   prnt("ss d l f fs;", BRED, "Single1:", itr->source_buffer_id,
  //   itr->B.size(),
  //        itr->B.front().Tstamp64 / 1e9 * opt.Period,
  //        itr->B.back().Tstamp64 / 1e9 * opt.Period, RST);
  // }

  // bool time_checked=false;
  // // Проверяем, не нужно ли скорректировать время - только если offset_flag
  // уже
  // // установлен
  // if (offset_flag.load(std::memory_order_acquire)) {
  //   // если ource_buffer_id не перешел за offset_buffer_id, корректируем
  //   if (itr->source_buffer_id <
  //       offset_buffer_id.load(std::memory_order_acquire)) { // корректируем
  //     Add_Offset_To_Buffer(itr);
  //     time_checked = true; // сбрасываем флаг, чтобы больше не проверять
  //     время
  //   } else {               // иначе сбрасываем offset_buffer_id
  //     offset_buffer_id.store(UINT_MAX, std::memory_order_release);
  //   }
  // }

  auto pulse_it = itr->PLS.begin();

  while (pulse_it != itr->PLS.end()) {
    // Запоминаем следующий импульс на случай удаления текущего
    auto next_pulse_it = std::next(pulse_it);

    // T0 - временная отметка текущего импульса
    Long64_t T0 = pulse_it->Tstamp64;
    if (T0 == 0) {
      prnt("ss d ls;", KRED, "Zero Tstamp64 in Res_SingleBuf:", pulse_it->Chan,
           T0, RST);
    }

    std::list<EventBuf>::iterator last_suitable = events_end;

    // Поиск целевого буфера, начинаем с текущего, в цикле сразу же перемещаем
    // Идем назад от текущего буфера к началу, не переходим через boundary
    auto target_buf_itr = itr;
    while (target_buf_itr != boundary &&
           target_buf_itr != Bufevents.begin()) {
      auto prev_buf_itr = std::prev(target_buf_itr);

      if (prev_buf_itr->PLS.empty()) {
        target_buf_itr = prev_buf_itr;
        continue;
      }

      // T1 - временная отметка последнего импульса проверяемого буфера
      Long64_t T1 = prev_buf_itr->PLS.back().Tstamp64;
      Long64_t delta_t = T0 - T1;

      // if (!time_checked) {
      //   time_checked=true;
      //   if (delta_t < -opt.tgate) {
      //     // обновляем временной сдвиг
      //     T64_offset.fetch_add(-delta_t, std::memory_order_acq_rel);
      //     // устанавливаем флаг сброса времени
      //     offset_flag.store(true, std::memory_order_release);
      //     // корректируем время у всего буфера
      //     T0 = Add_Offset_To_Buffer(itr);
      //     delta_t = T0 - T1; //пересчитываем delta_t
      //   }
      // }
      // Проверяем условие окончания сортировки
      if (delta_t > opt.tgate) {
        // Этот буфер слишком старый
        if (last_suitable != events_end) {
          // Если ранее был найден last_suitable, вставляем в начало последнего
          // подходящего буфера
          last_suitable->PLS.splice(last_suitable->PLS.begin(), itr->PLS, pulse_it);
          break;
        }
        // Нет подходящего буфера → импульс остаётся в своём
        // ВСЕ последующие импульсы будут ЕЩЁ новее → тоже не найдут
        return; // Завершаем обработку ВСЕГО буфера!
      }

      // Буфер подходит по времени
      last_suitable = prev_buf_itr;

      if (T0 >= prev_buf_itr->PLS.front().Tstamp64) {
        // Импульс будет в середине/конце → конечный выбор
        prev_buf_itr->PLS.splice(prev_buf_itr->PLS.end(), itr->PLS, pulse_it);
        sorted_move(prev_buf_itr->PLS);
        break;
      }

      // Импульс будет в начале → проверяем более старый
      target_buf_itr = prev_buf_itr;
    }

    // Если дошли до начала списка и есть подходящий
    if ((target_buf_itr == boundary ||
         target_buf_itr == Bufevents.begin()) &&
        last_suitable != events_end) {
      double T_from = pulse_it->Tstamp64 / 1e9 * opt.Period;
      double T_to = last_suitable->PLS.begin()->Tstamp64 / 1e9 * opt.Period;
      prnt("ss d d l f fs;", BMAG,
           "last_suitable:", boundary->source_buffer_id,
           last_suitable->source_buffer_id, Bufevents.size(), T_from, T_to,
           RST);
      if (Bufevents.size() > max_deque_size - 1) {
        // почему -1? (было -2 до введения events_end)
        crs->errors[ER_PBUF]++;
        prnt("ss d d ls;", BRED, "ResortEdge:", itr->source_buffer_id,
             last_suitable->source_buffer_id, Bufevents.size(), RST);
      }
      last_suitable->PLS.splice(last_suitable->PLS.begin(), itr->PLS, pulse_it);
    }

    pulse_it = next_pulse_it;
  }
}

bool Decoder::UpdateBoundary(std::list<EventBuf>::iterator itr) {
  // Обновляем (сдвигаем вправо) границу
  // начинаем от boundary и двигаемся вправо пока выполняются ВСЕ
  // условия (значит, можно make_event):
  // 1) flag>=2 (пересортирован) И
  // 2) ожидаемый размер >= max_deque_size ИЛИ dt > max_time_diff,
  // где dt - разница времени между последним импульсом в itr и
  // последним импульсом в boundary

  std::lock_guard<std::mutex> lock(boundary_mutex);
  size_t sz = Bufevents.size() - 1; // минус events_end
  // Сколько останется от boundary до конца

  //auto it = boundary;
  //auto prev_boundary = boundary;

  Long64_t dt = -1e10;
  Long64_t dt_prev = dt;
  bool moved=false;

  while (boundary != itr && boundary != events_end) {
    bool b_move = false;

    // Проверка флага: только если уже отсортирован
    if (boundary->flag.load(std::memory_order_acquire) >= 2) {
      // Проверка размера
      if (sz >= max_deque_size || boundary->PLS.empty()) {
        b_move = true;
      }
      // Проверка временной разницы, если есть данные в буферах
      else if (!itr->PLS.empty()) { // !boundary->PLS.empty() уже проверено выше
        dt_prev=dt;
        dt = itr->PLS.back().Tstamp64 - boundary->PLS.back().Tstamp64;
        if (dt > max_time_diff) {
          b_move = true;
        }
      }
    }

    // cout << "flag: " << (int)it->flag << " " << should_remove << " " << b1
    //      << " " << b2 << " " << dt / 1e9 * opt.Period << " "
    //      << it->source_buffer_id << endl;
    if (!b_move)
      break; // этот буфер еще может сортироваться, вправо не двигаемся

    boundary->flag.store(3, std::memory_order_release);
    ++boundary; // считаем, что этот уже отсортирован, двигаем границу
    --sz;
    moved=true;
  }

  prnt2("ss d d d l f fs;", BYEL, "boundary done:", itr->source_buffer_id,
        boundary->source_buffer_id,
        boundary->flag.load(std::memory_order_acquire), sz,
        dt / 1e9 * opt.Period, dt_prev / 1e9 * opt.Period, RST);

  return moved;
}

void Decoder::MakeEvent_Worker() {

  prnt2("ss ds;", BGRN, "MakeEvent worker started with timeout",
        makeevent_timeout_ms, RST);

  // itr указывает на последний ОБРАБОТАННЫЙ буфер
  // next указывает на следующий буфер после обработанного
  auto itr = events_end; // в начале оба указывают на end()
  auto next = itr;
  // NOTE: next — итератор на буфер, готовый к обработке.
  // Инициализируется как events_end, обновляется через ++next.
  // Критически важен для поддержания порядка обработки.
  // NOTE: itr и next управляют цепочкой обработки буферов.
  // При достижении конца (next == events_end) цикл продолжает ожидание новых
  // буферов. Условие wait_for корректно обрабатывает events_end, не вызывая
  // разыменования.

  while (true) {
    // 1. ЖДЁМ под мьютексом
    {
#ifdef PROFILING
      SimpleTimer timer(this, "MakeEv_Wait");
#endif
      std::unique_lock<std::mutex> lock(buf_mutex);

      // Условие: ждём пока itr не будет ready
      makeevent_cond.wait_for(lock, std::chrono::milliseconds(makeevent_timeout_ms), [&]() {
        if (!makeevent_running.load(std::memory_order_acquire))
          return true;
        next = (itr == events_end) ? Bufevents.begin() : std::next(itr);
        // prnt("ss d d d d ls;", BRED, "next:", next->source_buffer_id,
        //      itr->source_buffer_id, events_end->source_buffer_id,
        //      Bufevents.begin()->source_buffer_id, Bufevents.size(), RST);
        if (next == events_end)
          return false;
        // cout << "next->flag: " << (int) next->flag.load(std::memory_order_acquire) << endl;
        return next->flag.load(std::memory_order_acquire) == 3;
      });

      // next указывает на конец или на неготовый буфер
      if (next == events_end ||
          next->flag.load(std::memory_order_acquire) != 3) {
        // prnt("ss d ls;", KGRN, "MakeEvent: No buffers:", next->source_buffer_id, Bufevents.size(),
        //      RST);
        // нет готовых буферов
        if (makeevent_running.load(std::memory_order_acquire))
          continue; // продолжаем
        else
          break; // Выходим
      }
      // есть готовый буфер
      // else {
      //   prnt("ss d ls;", BGRN,
      //        "MakeEvent: woke up. Next buffer:", next->source_buffer_id,
      //        Bufevents.size(), RST);
      // }
    } // конец мьютекса

    // 2. ОБРАБАТЫВАЕМ
    // bool processed_any = false;
    {
#ifdef PROFILING
      SimpleTimer timer(this, "MakeEv_Exec");
#endif
      while (next != boundary && next != events_end &&
             next->flag.load(std::memory_order_acquire) == 3) {
        // проверяем на конец списка (может быть, если выход по таймауту)

        // Процессинг буфера - без мьютекса
        try {
          Make_Events(next);
        } catch (...) {
          prnt("ss ds;", BRED, "Exception in makeevent_worker",
               next->source_buffer_id, RST);
        }

        { // блокируем мьютекс
          std::lock_guard<std::mutex> lock(ana_mutex);
          next->flag.store(4, std::memory_order_release);
          ana_cond.notify_one();
        } // lock

        prnt2("ss ds;", BCYN, "MakeEvent done:", next->source_buffer_id, RST);
        itr = next;
        ++next;

      } // while (внутренний)
    } // замер завершён

  } // while (внешний)

  prnt2("ss ls;", BGRN, "MakeEvent worker finished. Bufevents:", Bufevents.size(), RST);
} // MakeEvent_Worker

void Decoder::Make_Events(std::list<EventBuf>::iterator itr) {
  // создает контейнер событий из импульсов. удаляет импульсы.
  
  // cout << "make events: " << itr->source_buffer_id << " " << itr->PLS.size() <<
  // endl;
  if (itr->PLS.empty())
    return; // ← Защита от пустого буфера

  // prnt("ss d f fs;", BGRN, "M:", itr->source_buffer_id,
  //      itr->PLS.front().Tstamp64 / 1e9 * opt.Period,
  //      itr->PLS.back().Tstamp64 / 1e9 * opt.Period, RST);

  /*
  Levents.emplace_back();
  auto ev = &Levents.back();
  auto start = itr->PLS.begin();
  ev->Tstmp = start->Tstamp64;
  ev->pulses.push_back(std::move(*start));
  ++start;
  for (auto pulse_it = start; pulse_it != itr->PLS.end(); ++pulse_it) {
    if (pulse_it->Tstamp64 > ev->Tstmp + opt.tgate) {
      Levents.emplace_back();
      ev = &Levents.back();
      ev->Tstmp = pulse_it->Tstamp64;
    }
    ev->pulses.push_back(std::move(*pulse_it));
  }
  */

  //auto evect = &itr->EVT; // вектор событий для этого буфера
  // текущее событие в векторе событий

  eventiter ev = itr->EVT.end();
  Long64_t TT = -9999999999;

  for (auto pulse_it = itr->PLS.begin(); pulse_it != itr->PLS.end();
       ++pulse_it) {
    if (pulse_it->Tstamp64 > TT) {
      ev = itr->EVT.emplace(itr->EVT.end());
      ev->Tstmp = pulse_it->Tstamp64;
      TT = ev->Tstmp + opt.tgate;
    }
    ev->pulses.push_back(std::move(*pulse_it));
  }
  Bufsize.fetch_add(itr->EVT.size(), std::memory_order_release);
  Nevents.fetch_add(itr->EVT.size(), std::memory_order_relaxed);
  //itr->flag.store(4, std::memory_order_release);
  itr->PLS.clear(); // очищаем список импульсов
}

void Decoder::Ana_Worker() {
  prnt2("sss;", BGRN, "Ana worker started.", RST);

  while (ana_running.load(std::memory_order_acquire)) {
    { // 1. Ждём по условию: есть данные или пора выходить
#ifdef PROFILING
      SimpleTimer timer(this, "Ana_Wait");
      cout << "Ana wait... " << ana_running << endl;
#endif
      std::unique_lock<std::mutex> lock(ana_mutex);
      ana_cond.wait_for(lock, std::chrono::milliseconds(ana_timeout_ms));
    }

    {
#ifdef PROFILING
      SimpleTimer timer(this, "Ana_Exec");
#endif
      if (Bufevents.size() > 1) {
        // есть как минимум один буфер, кроме events_end
        // 2. Обходим Bufevents и обрабатываем буферы с flag == 4
        for (auto it = Bufevents.begin(); it != events_end; ++it) {
          // Проверяем, что буфер готов к анализу
          int fl = it->flag.load(std::memory_order_acquire);
          prnt2("ss d d ds;", KRED, "Ana:", it->source_buffer_id,
               boundary->source_buffer_id, fl, RST);
          if (fl < 4)
            break; // дальше буферы ещё не готовы
          else if (fl == 4) {
            try {
              // Здесь можно добавить логику анализа каждого события
              // Например:
              // for (auto& ev : it->B) {
              //   crs->AnalyzeEvent(ev);
              // }

              // Помечаем как обработанный
            } catch (...) {
              prnt("ss ds;", BRED, "Exception in Ana_Worker processing buffer:",
                   it->source_buffer_id, RST);
            }
            it->flag.store(5, std::memory_order_release);
          }
        }

        // буфер перед boundary нельзя удалять
        auto before_boundary = (boundary == Bufevents.begin())
                                 ? Bufevents.begin()
                                 : std::prev(boundary);

        // 3. Удаляем старые буферы (flag == 5), сохраняя минимум opt.ev_max
        // событий
        Long64_t removed_events = 0;
        while (Bufevents.size() > 1) {
          if (Bufevents.begin() == before_boundary ||
              Bufevents.begin()->flag.load(std::memory_order_acquire) < 5)
            break; // ещё не проанализирован — выходим

          // Удаляем с учётом количества событий
          Long64_t current_size = Bufsize.load(std::memory_order_acquire);
          Long64_t new_size = current_size - Bufevents.begin()->EVT.size();
          if (new_size < opt.ev_max)
            break;

          removed_events += Bufevents.begin()->EVT.size();

          prnt2("ss d d ds;", BRED, "Pop:", Bufevents.begin()->source_buffer_id,
               boundary->source_buffer_id,
               Bufevents.begin()->flag.load(std::memory_order_acquire), RST);

          Bufevents.pop_front();
        }

        Bufsize.fetch_sub(removed_events, std::memory_order_release);
      }

      // 4. Выход, если больше не работаем
      // if (!ana_running.load(std::memory_order_acquire)) {
      //   break;
      // }
    } // замер завершён

  } // while (внешний)

  prnt2("ss ls;", BGRN, "Ana worker finished. Bufevents:", Bufevents.size(),
        RST);
}

/*
// добавляет офсет ко всем временным отметкам в буфере
Long64_t Decoder::Add_Offset_To_Buffer(std::list<EventBuf>::iterator itr) {
  cout << "add_offset: " << T64_offset.load(std::memory_order_acquire) << endl;
  if (!itr->B.empty()) {
    Long64_t dt64 = T64_offset.load(std::memory_order_acquire);
    for (auto &p : itr->B) {
      p.Tstamp64 += dt64;
    }
  }
  return itr->B.front().Tstamp64;
}
*/

// Метод для добавления данных в очередь извне
void Decoder::Add_to_copy_queue(UChar_t *data, size_t size) {
  std::lock_guard<std::mutex> lock(copy_mutex);
  // copy_queue.push_back({data, size});
  BufClass buf;
  buf.write_start = data;
  buf.write_end = data + size;

  // prnt2("ss ls;", KYEL, "Add_to_copy:", copy_queue.size(), RST);

  copy_queue.push_back(std::move(buf));
  copy_cond.notify_one(); // Будим поток даже если copy_running=false
}

bool Decoder::IsEventStart_1(union82 &u82) { // adcm raw
  return *u82.ui == 0x2a500100;
}
bool Decoder::IsEventStart_3(union82 &u82) { // adcm dec
  return *u82.us == ID_EVNT;
}
bool Decoder::IsEventStart_22(union82 &u82) { // CRS2
  return !(u82.b[1] & 0x70);                  // fmt==0
}
bool Decoder::IsEventStart_36(union82 &u82) { // CRS raw
  return !(u82.b[6] & 0xF0);                  // fmt==0
}
bool Decoder::IsEventStart_79(union82 &u82) { // CRS dec
  return u82.b[7] & 0x80;                     // fmt==1
}

UChar_t *Decoder::FindEvent_backward_1(union82 From, UChar_t *To) {
  while (From.b > To) {
    From.us--;
    if (IsEventStart_1(From))
      return From.b;
  }
  return To;
}
UChar_t *Decoder::FindEvent_backward_3(union82 From, UChar_t *To) {
  while (From.b > To) {
    From.us--;
    if (IsEventStart_3(From))
      return From.b;
  }
  return To;
}
UChar_t *Decoder::FindEvent_backward_22(union82 From, UChar_t *To) {
  while (From.b > To) {
    From.us--;
    if (IsEventStart_22(From))
      return From.b;
  }
  return To;
}
UChar_t *Decoder::FindEvent_backward_36(union82 From, UChar_t *To) {
  while (From.b > To) {
    From.ul--;
    if (IsEventStart_36(From))
      return From.b;
  }
  return To;
}
UChar_t *Decoder::FindEvent_backward_79(union82 From, UChar_t *To) {
  while (From.b > To) {
    From.ul--;
    if (IsEventStart_79(From))
      return From.b;
  }
  return To;
}

// ОПРЕДЕЛЯЕМ статические переменные и таблицы
// bool Decoder::tables_initialized = false;
std::atomic<bool> Decoder::tables_initialized{false};
Decoder::FindBackwardFunc Decoder::find_backward_table[MAX_HASH];
Decoder::DecodeFunc Decoder::decode_table[MAX_HASH];

//////////
void Decoder::init_all_tables() {
  // Обнуляем всю таблицу
  std::fill_n(find_backward_table, MAX_HASH, nullptr);
  std::fill_n(decode_table, MAX_HASH, nullptr);
  // std::fill_n(find_forward_table, MAX_HASH, nullptr);

  // Заполняем конкретные модули
  find_backward_table[1] = &Decoder::FindEvent_backward_1;
  find_backward_table[3] = &Decoder::FindEvent_backward_3;
  find_backward_table[22] = &Decoder::FindEvent_backward_22;

  // find_forward_table[1] = &Decoder::FindEvent_forward_1;
  // find_forward_table[3] = &Decoder::FindEvent_forward_3;
  // find_forward_table[22] = &Decoder::FindEvent_forward_22;

  // Заполняем диапазоны
  for (int mod : {32, 33, 34, 35, 36, 41, 42, 43, 44, 45, 51, 52, 53, 54}) {
    find_backward_table[mod] = &Decoder::FindEvent_backward_36;
    // find_forward_table[mod] = &Decoder::FindEvent_forward_36;
  }

  for (int mod : {75, 76, 77, 78, 79, 80}) {
    find_backward_table[mod] = &Decoder::FindEvent_backward_79;
    // find_forward_table[mod] = &Decoder::FindEvent_forward_79;
  }

  // decode_table[1] = &Decoder::FindEvent_backward_1;
  // decode_table[3] = &Decoder::FindEvent_backward_3;
  decode_table[22] = &Decoder::Decode22;

  for (int mod : {35, 36, 43, 44, 45, 53, 54}) {
    decode_table[mod] = &Decoder::Decode36;
  }
}

UChar_t *Decoder::FindEvent_backward(union82 From, UChar_t *To) {
  // Ленивая инициализация при первом вызове
  // Автоматическая потокобезопасность
  // std::call_once(init_all_flag,init_all_tables); //Один вызов для всех
  // методов

  // Быстрая диспетчеризация
  if (crs_module >= 0 && crs_module < MAX_HASH &&
      find_backward_table[crs_module]) {
    return (this->*find_backward_table[crs_module])(From, To);
  }

  cout << "Wrong module: " << crs_module << endl;
  return To;
}

/*
UChar_t* Decoder::FindEvent_backward(union82 From, UChar_t *To) {
  // возвращает начало последнего события в буфере
  // ищет от From-1 до To; From должно быть > To
  // если не найдено, возвращает To

  switch (crs_module) {
  case 1: // adcm raw
    return FindEvent_backward_1(From,To);
  case 3: // adcm dec
    return FindEvent_backward_3(From,To);
  case 22:
    return FindEvent_backward_22(From,To);
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 41:
  case 42:
  case 43:
  case 44:
  case 51:
  case 52:
  case 53:
  case 54:
  case 45:
    return FindEvent_backward_36(From,To);
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
    return FindEvent_backward_79(From,To);
  default:
    cout << "Wrong module: " << crs_module << endl;
  }
  return To;
} // FindEvent_backward
*/

/*
void Decoder::Dec_End(eventlist &Blist, BufClass& Buf) {
  //Bufevents.pop_back();



  //Blist.push_back(EventClass());
  //Blist->back().Nevt=iread;
  //Blist->back().Spin=sp;
}
*/
void Decoder::Decode_switch(BufClass &Buf, pulsecontainer &pc) {
  // Ленивая инициализация при первом вызове
  // Автоматическая потокобезопасность
  // std::call_once(init_all_flag, init_all_tables);//Один вызов для всех
  // методов

  // Быстрая диспетчеризация
  if (crs_module >= 0 && crs_module < MAX_HASH && decode_table[crs_module]) {
    (this->*decode_table[crs_module])(Buf, pc);
    return;
  }

  cout << "Wrong module: " << crs_module << endl;
}

/*
void Decoder::Decode_switch(BufClass& Buf) {

  switch (crs_module) {
  case 32:
  case 33:
  case 34:
  case 41:
  case 51:
    //Decode34(dec_iread[ibuf]-1,ibuf);
    //break;
  case 42:
  case 52:
    //Decode34(Buf);
    break;
  case 35:
  case 36:
  case 43:
  case 44:
  case 45:
  case 53:
  case 54:
    Decode36(Buf);
    break;
  case 80:
    //Decode80(Buf);
    break;
  case 79:
    //Decode79(Buf);
    break;
  case 78:
    //Decode78(Buf);
    break;
  case 77:
    //Decode77(Buf);
    break;
  case 76:
    //Decode76(Buf);
    break;
  case 75:
    //Decode75(Buf);
    break;
  case 22:
    Decode22(Buf);
    break;
  case 1:
    //Decode_adcm(Buf);
    break;
  case 3:
    //Decode_adcm_dec(Buf);
    break;
  default:
    cout << "Wrong module: " << crs_module << endl;
  }

} //Decode_switch
*/

void Decoder::Decode22(BufClass &Buf, pulsecontainer &pc) {
  // xxxx- нет!!!-> декодирует один импульс из буфера Buf, записывает его в
  // pls

  // PulseClass ipls = crs->dummy_pulse;
  // создаём первый импульс с флагом P_BADPEAK, чтобы он не попал в обработку
  // Pos=-32221, чтобы не сработал channel mismatch
  pc.emplace_back(-32221, P_BADPEAK);
  PulseClass *ipls = &pc.back();

  // Анализируем между analysis_start и analysis_end
  union82 uu; // текущее положение в буфере
  uu.b = Buf.analysis_start;
  UChar_t frmt;

  // uu.b+1 - чтобы не выйти за границы если нет 2-байтного выравнивания
  while (uu.b + 1 < Buf.analysis_end) {

    frmt = (uu.b[1] & 0x70) >> 4;
    short data = *uu.us & 0xFFF;
    UChar_t ch = (*uu.us & 0x8000) >> 15;

    if (frmt && ch != ipls->Chan && ipls->Pos != -32221) { // channel mismatch
      ++crs->errors[ER_MIS];
      ipls->ptype |= P_BADCH;
      uu.us++;
      continue;
    }

    if (frmt == 0) {
      // ipls.ptype&=~P_NOLAST; //pulse has stop

      // analyze previous pulse
      if (ipls->ptype == 0) {
        PulseAna(*ipls);
        if (ipls->Tstamp64==0) {
          prnt("ss d ls;", KRED, "Zero Tstamp64 in Decode22:", ipls->Chan,
               ipls->Tstamp64, RST);
        }
        sorted_move(pc);
        pc.emplace_back();
        ipls = &pc.back();
      } else {
        ipls->ptype = 0;
      }

      // test ch, then create new pulse
      if (ch >= opt.Nchan) { // bad channel
        ++crs->errors[ER_CH];
        ipls->ptype |= P_BADCH;
        uu.us++;
        continue;
      }

      // ipls = PulseClass();
      crs->npulses++;
      ipls->Chan = ch;
      ipls->Tstamp64 = data; //+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];
    } else if (frmt < 4) {
      Long64_t t64 = data;
      ipls->Tstamp64 += (t64 << (frmt * 12));
    } else if (frmt == 4) {
      ipls->Counter = data;
    } else if (frmt == 5) {
      if ((int)ipls->sData.size() >= cpar.Len[ipls->Chan]) {
        ipls->ptype |= P_BADSZ;
      }
      // else {
      ipls->sData.push_back((data << 21) >> 21);
      // cout << "decode2b: " << idx2 << endl;
      // }
    }

    uu.us++;
  } // while

  // обрабатываем последний импульс
  if (ipls->ptype == 0) {
    PulseAna(*ipls);
    sorted_move(pc);
  }
  else { // если плохой - удаляем
    pc.pop_back();
  }

} // decode22

void Decoder::Decode36(BufClass &Buf, pulsecontainer &pc) {
  // xxxx- нет!!!-> декодирует один импульс из буфера Buf, записывает его в
  // pls

  //?? сделать проверку inbuf.flag (нужно ли??)
  ULong64_t data;
  Int_t d32;
  Short_t d16;
  PkClass pk;
  PulseClass ipls = crs->dummy_pulse;

  union82 uu; // текущее положение в буфере
  uu.b = Buf.write_start;
  UChar_t frmt;

  // uu.b+7 - чтобы не выйти за границы если нет 8-байтного выравнивания
  while (uu.b + 7 < Buf.write_end) {
    frmt = (uu.b[6] & 0xF0) >> 4;
    data = *uu.ul & sixbytes;
    UChar_t ch = uu.b[7];

    if (frmt && ch != ipls.Chan) { // channel mismatch
      ++crs->errors[ER_MIS];
      ipls.ptype |= P_BADCH;
      uu.ul++;
      continue;
    }

    switch (frmt) {
    case 0:
      if (*uu.ul == 0) {
        ++crs->errors[ER_ZERO]; // zero data
        uu.ul++;
        continue;
      }

      // первый dummy_pulse, у которого ptype=P_BADPEAK
      if (ipls.ptype == 0) { // analyze old pulse (если не dummy)
        if (opt.Dsp[ipls.Chan]) {
          crs->MakePk(pk, ipls);
        }
        PulseAna(ipls);
        sorted_insert(pc, ipls);
        // YK Event_Insert_Pulse(*Blist, ipls);
      }

      // test ch, then create new pulse

      if (ch != 255 && ch >= opt.Nchan) { // bad channel
        ++crs->errors[ER_CH];
        ipls.ptype |= P_BADCH;
        break;
      }

      ipls = PulseClass();
      crs->npulses++;
      ipls.Chan = ch;
      ipls.Tstamp64 = data; //+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];

      if (ch == 255) {    // start channel
        ipls.Spin |= 128; // bit 7 - hardware counters
      }

      break;

    case 1:
      ipls.Spin |= uu.b[5] & 1;
      ipls.Counter = data & 0xFFFFFFFFFF;
      break;
    case 2:
      if ((int)ipls.sData.size() >= cpar.Len[ipls.Chan]) {
        ipls.ptype |= P_BADSZ;
        ++crs->errors[ER_LEN];
        uu.ul++;
        continue;
      }
      for (int i = 0; i < 4; i++) {
#ifdef BITS
        (data >>= BITS) <<= BITS;
#endif
        d32 = data & 0x7FF; // 11 bit
        ipls.sData.push_back((d32 << 21) >> 21);
        data >>= 12;
      }
      break;
    case 3:
      if ((int)ipls.sData.size() >= cpar.Len[ipls.Chan]) {
        ipls.ptype |= P_BADSZ;
        ++crs->errors[ER_LEN];
        uu.ul++;
        continue;
      }
      if (cpar.F24) {
        for (int i = 0; i < 2; i++) {
#ifdef BITS
          (data >>= BITS) <<= BITS;
#endif
          d32 = data & 0xFFFFFF; // 24 bit
          Float_t f32 = (d32 << 8) >> 8;
          ipls.sData.push_back(f32 / 256.0f);
          data >>= 24;
        }
      } else {
        for (int i = 0; i < 3; i++) {
#ifdef BITS
          (data >>= BITS) <<= BITS;
#endif
          d16 = data & 0xFFFF; // 16 bit
          ipls.sData.push_back(d16);
          data >>= 16;
        }
      }
      break;
    case 4: // C – [24]; A – [24]
      pk.A = data & 0xFFFFFF;
      pk.A = (pk.A << 8) >> 8;
      data >>= 24;
      pk.C = data & 0xFFFFFF;
      pk.C = (pk.C << 8) >> 8;
      break;
    case 5: // RX – [12]; QX – [36]
      pk.QX = data & 0xFFFFFFFFF;
      pk.QX = (pk.QX << 28) >> 28;
      data >>= 36;
      pk.RX = data & 0xFFF;
      pk.RX = (pk.RX << 20) >> 20;
      break;
    case 6: // AY – [28]; [E]; H – [16]
      pk.H = data & 0xFFFF;
      data >>= 16;
      pk.E = data & 1;
      data >>= 4;
      pk.AY = data & 0xFFFFFFF;
      pk.AY = (pk.AY << 4) >> 4;
      break;
    case 8: // RX – [20]; C – [28]
      pk.C = data & 0xFFFFFFF;
      pk.C = (pk.C << 4) >> 4;
      data >>= 28;
      pk.RX = data & 0xFFFFF;
      pk.RX = (pk.RX << 12) >> 12;
      break;
    case 9: // A – [28]
      pk.A = data & 0xFFFFFFF;
      pk.A = (pk.A << 4) >> 4;
      break;
    case 10: // QX – [40]
      pk.QX = data & 0xFFFFFFFFFF;
      pk.QX = (pk.QX << 24) >> 24;
      break;
    case 11: { // Counters
      ipls.Counter = data;
      ipls.Spin |= 128; // bit 7 - hardware counters

      double dt = (ipls.Tstamp64 - crs->Tst3o[ipls.Chan]) * 1e-9 * opt.Period;
      if (dt) {
        crs->rate_hard[ipls.Chan] =
            (ipls.Counter - crs->npulses3o[ipls.Chan]) / dt;
      }
      crs->Tst3o[ipls.Chan] = ipls.Tstamp64;
      crs->npulses3o[ipls.Chan] = ipls.Counter;

      // prnt("ss d l l f
      // fs;",BBLU,"CONT:",ch,ipls.Tstamp64,ipls.Counter,dt,rate3[ipls.Chan],RST);
      break;
    }
    case 12:
      if (data & 1) {
        ++crs->errors[ER_OVF];
        ipls.Spin |= 128; // bit 7 - hardware counters
        ipls.Spin |= 4;   // bit 2 - ER_OVF
      }
      // prnt("ss d l ls;",KGRN,"OVF:",ch,ipls.Tstamp64,data&1,RST);
      break;
    case 13: {
      int bit23 = (data & 0x800000) >> 23;
      if (!bit23) {
        // cout << "bit23: " << data << " " << bit23 << endl;
        ++crs->errors[ER_CFD];
      }
      pk.CF1 = data & 0x7FFFFF;    // 23
      pk.CF1 = (pk.CF1 << 9) >> 9; // 32-23
      data >>= 24;
      pk.CF2 = data & 0x7FFFFF;    // 23
      pk.CF2 = (pk.CF2 << 9) >> 9; // 32-23
      // prnt("ss d l d d
      // ds;",KGRN,"CFD:",ch,ipls.Tstamp64,bit23,pk.CF1,pk.CF2,RST);
    } break;
    default:
      ++crs->errors[ER_FRMT];
    } // switch (frmt);

    uu.ul++;
  } // while

  if (ipls.ptype == 0) {
    if (opt.Dsp[ipls.Chan]) {
      crs->MakePk(pk, ipls);
    }
    PulseAna(ipls);
    sorted_insert(pc, ipls);
    // YK Event_Insert_Pulse(*Blist, ipls);
  }

  // Dec_End(Blist,0,255);
} // decode36

void Decoder::Event_Insert_Pulse(eventlist &Elist, PulseClass &pls) {
  // вставляем импульс в список событий, анализируя окно совпадений
  // при необходимости, создается новое событие

  // Отбрасываем плохие импульсы
  if (pls.ptype) { // any bad pulse
    // cout << "bad pulse: " << (int) pls->ptype << " " << (int) pls->Chan
    // 	 << " " << pls->Counter << " " << pls->Tstamp64 << endl;
    if (pls.Chan < 254)
      ++crs->npulses_bad[pls.Chan];
    return;
  }

  // считаем только нормальные импульсы; пропускаем счетчики (Spin 128)
  if (!(pls.Spin & 128)) {
    ++crs->npulses2[pls.Chan];
  }

  /*
  // оставляем только дробную часть в pls->Time, остальное загоняем в
  Tstamp64 int i_dt = pls->Time; pls->Time -= i_dt; pls->Simul2 -= i_dt;
  pls->Tstamp64+=i_dt;
  */

  // добавляем софтовую задержку
  Long64_t T64 = pls.Tstamp64 + Long64_t(opt.sD[pls.Chan] / opt.Period);

  // ищем совпадение от конца списка до начала, но не больше, чем opt.ev_min
  int nn = 0;
  for (auto rit = Elist.rbegin(); rit != Elist.rend(); ++rit, ++nn) {
    if (nn >= opt.ev_min) { // event lag exceeded
      // вставляем новое событие ПОСЛЕ текущего (сразу после event lag)
      // для следующего импульса, если он придет с Tstmp близким к текущему,
      // это (текущее) событие попадет в проверку.
      auto it = Elist.emplace(rit.base(), EventClass());
      // auto it = Elist.emplace(rit, EventClass()).base();
      it->Nevt = crs->nevents;
      it->AddPulse(&pls);
      ++crs->nevents;
      ++crs->errors[ER_LAG]; // event lag exceeded

      return;
    }
    Long64_t dt = (T64 - rit->Tstmp);
    // KK dt = (pls->Tstamp64 - rit->Tstmp);

    if (dt > opt.tgate) {
      // pls пришел позже, чем tgate -> добавляем новое событие ПОСЛЕ
      // текущего
      auto it = Elist.emplace(rit.base(), EventClass());
      it->Nevt = crs->nevents;
      it->AddPulse(&pls);
      ++crs->nevents;
      return;
    } else if (TMath::Abs(dt) <= opt.tgate) { // add pls to existing event
      // coincidence event
      rit->AddPulse(&pls);
      return;
    }
    // else: dt < -opt.tgate - импульс пришел раньше, продолжаем поиск к
    // началу списка
  }

  // дошли до начала списка; вставляем новый event в начало (перед первым
  // событием)
  auto it = Elist.emplace(Elist.begin(), EventClass());
  it->Nevt = crs->nevents;
  it->AddPulse(&pls);
  ++crs->nevents;

} // Event_Insert_Pulse

void Decoder::PulseAna(PulseClass &ipls) {
  if (!opt.Dsp[ipls.Chan]) { // Dsp==0 -> анализируем импульс
    if (opt.sS[ipls.Chan] > 1) {
      ipls.Smooth(opt.sS[ipls.Chan]);
    } else if (opt.sS[ipls.Chan] < -1) {
      ipls.Smooth_hw(-opt.sS[ipls.Chan]);
    }
    ipls.PeakAna33();
  } else { // Dsp!=0 -> не анализируем
    if (opt.checkdsp) {
      int t_tg = opt.sTg[ipls.Chan];
      opt.sTg[ipls.Chan] = cpar.Trg[ipls.Chan];
      PulseClass ipls2 = ipls;
      ipls.Pos = cpar.Pre[ipls.Chan];
      ipls2.PeakAna33();
      crs->CheckDSP(ipls, ipls2);
      opt.sTg[ipls.Chan] = t_tg;
    }
  }

  ipls.Ecalibr(ipls.Area);
  if (hcl->b_base[ipls.Chan]) {
    ipls.Ecalibr(ipls.Base);
    ipls.Ecalibr(ipls.Sl1);
    ipls.Ecalibr(ipls.Sl2);
    ipls.Ecalibr(ipls.RMS1);
    ipls.Ecalibr(ipls.RMS2);
    // ipls.Bcalibr();
  }
} // PulseAna

void Decoder::MakePk(PkClass &pk, PulseClass &ipls) {
  Float_t Area0;

#ifdef APK
  ipls.ppk = pk;
#endif

  ipls.Pos = cpar.Pre[ipls.Chan];

  Area0 = pk.A / p_len[ipls.Chan];
  ipls.Base = pk.C / b_len[ipls.Chan];
  ipls.Area = Area0 - ipls.Base;

  ipls.Height = pk.H;

  if (opt.Dsp[ipls.Chan] != 2) { // use hardware timing
    // Rtime
    if (pk.RX != 0)
      ipls.Rtime = Double_t(pk.QX) / pk.RX;
    else {
      ++crs->errors[ER_RTIME];
      ipls.Rtime = -999;
      // YK_old;
    }
    // Time
    switch (cpar.Trg[ipls.Chan]) {
    case 3:
    case 6: {
      int kk = cpar.Drv[ipls.Chan];
      ipls.FindZero(kk, cpar.Trg[ipls.Chan], cpar.Thr[ipls.Chan],
                    cpar.LT[ipls.Chan]);
      ipls.Time -= ipls.Pos;
      break;
    }
    case 7:
      if (pk.CF1 != pk.CF2)
        ipls.Time =
            -1 + Float_t(cpar.LT[ipls.Chan] - pk.CF1) / (pk.CF2 - pk.CF1);
      else
        ipls.Time = 0;
      break;
    default:
      ipls.Time = ipls.Rtime;
    } // switch

    ipls.Rtime -= ipls.Time;
  } else { // use software timing
    ipls.PeakAna33(true);
  }

  Float_t wdth = pk.AY / w_len[ipls.Chan];

  if (opt.Mt[ipls.Chan] != 3) {
    if (ipls.Area) {
      ipls.Width = wdth - ipls.Base;
      ipls.Width /= ipls.Area;
    } else {
      ++crs->errors[ER_WIDTH];
      ipls.Width = 0;
    }
  } else {
    ipls.Sl2 = (ipls.Base - wdth) / (b_mean[ipls.Chan] - w_mean[ipls.Chan]);
    ipls.Area -= (p_mean[ipls.Chan] - b_mean[ipls.Chan]) * ipls.Sl2;

    ipls.Width = ipls.Pos - cpar.Pre[ipls.Chan] - ipls.Time;
    if (ipls.Width < -99)
      ipls.Width = -99;
    if (ipls.Width > 99)
      ipls.Width = 99;
  }

} // MakePk

#ifdef PROFILING
void Decoder::LogTime(const char* stage, long long duration_us) {
    // if (print2) {
    prnt("ss s l ss;", BMAG, "Timing", stage, duration_us, "μs", RST);
    // }
}
#endif
