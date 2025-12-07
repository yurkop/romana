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

Decoder::Decoder() {
  // exchange возвращает старое значение (false) и заменяет его на true
  if (!tables_initialized.exchange(true)) { // Atomic, thread-safe
    init_all_tables(); // ОДНА функция инициализации всех таблиц
  }
}
Decoder::~Decoder() { Decode_Stop(); }

void Decoder::Decode_Resize(Long64_t r_size, Long64_t o_size) {
  // r_size - nominal size of buffer (на самом деле + o_size)
  // o_size - offset size (offset в начале - перед буфером)

  buffer_storage.resize(o_size + r_size);

  Buf_ring.b1 = buffer_storage.data() + o_size;
  Buf_ring.b3 = Buf_ring.b1 + r_size;
  Buf_ring.u82.b = Buf_ring.b1;

  BufProc.b1 = Buf_ring.b1;
  BufProc.b3 = Buf_ring.b3;
  BufProc.u82.b = Buf_ring.b1;

  // буфер: -o_size .. 0 .. r_size
  // указатели:  b1 .. b .. b3
}

void Decoder::Decode_Start(Long64_t r_size, Long64_t o_size, bool b_acq,
                           int module) {

  Decode_Resize(r_size, o_size);

  crs_module = module;
  Bufpulses.clear();
  Sorted_pulses.clear();
  // allpulses.clear();
  first_call = true;

  for (int i = 0; i < MAX_CH; i++) {
    b_len[i] = opt.B2[i] - opt.B1[i] + 1;
    p_len[i] = opt.P2[i] - opt.P1[i] + 1;
    w_len[i] = opt.W2[i] - opt.W1[i] + 1;
    b_mean[i] = (opt.B2[i] + opt.B1[i]) * 0.5;
    p_mean[i] = (opt.P2[i] + opt.P1[i]) * 0.5;
    w_mean[i] = (opt.W2[i] + opt.W1[i]) * 0.5;
  }

  // стартуем потоки Process и копирования
  Resorting_Start();
  Splice_Start();
  Process_Start(4);
  if (b_acq)
    Copy_Start();
}

void Decoder::Decode_Stop() /*noexcept*/ {
  try {
    // Останавливаем копирование
    if (copy_running) {
      copy_running = false;
      copy_cond.notify_all(); // Будим поток копирования

      // Ждем пока поток обработает ВСЕ данные из очереди
      if (copy_thread && copy_thread->joinable()) {
        copy_thread->join();
        // Поток сам выйдет когда queue пуста И copy_running=false
      }
      copy_thread.reset(); // освобождаем память
    }

    // Останавливаем декодирование
    if (process_running) {
      process_running = false;
      process_cond.notify_all(); // Будим потоки декодирования

      for (auto &thread : process_threads) {
        if (thread && thread->joinable()) {
          thread->join();
        }
        // thread.reset(); - не нужен, т.к. clear все очищает
      }
      process_threads.clear();
    }

    // Останавливаем пересортировку
    if (resorting_running) {
      resorting_running = false;
      resorting_cond.notify_all(); // Будим поток пересортировки

      if (resorting_thread && resorting_thread->joinable()) {
        resorting_thread->join();
      }
      resorting_thread.reset(); // освобождаем память
    }

    // Останавливаем склейку
    if (splice_running) {
      splice_running = false;
      splice_cond.notify_all(); // Будим поток склейки

      if (splice_thread && splice_thread->joinable()) {
        splice_thread->join();
      }
      splice_thread.reset(); // освобождаем память
    }

    // Очистка очередей не нужна, т.к. они уже должны быть пустыми
  } catch (...) {
    // Логируем, но подавляем исключения
    std::cerr << "Exception in Decode_Stop()" << std::endl;
  }
}

void Decoder::Copy_Start() {
  copy_running = true;
  copy_thread = std::make_unique<std::thread>([this]() { Copy_Worker(); });
}

void Decoder::Process_Start(int num_threads) {
  num_process_threads = num_threads;
  process_running = true;
  process_threads.clear();

  // Создаем рабочие потоки анализа и инициализируем atomic указатели
  write_ptr.store(Buf_ring.b1, std::memory_order_release);
  worker_ptrs.clear();
  for (UInt_t i = 0; i < num_process_threads; ++i) {
    process_threads.push_back(
        std::make_unique<std::thread>([this, i]() { Process_Worker(i); }));
    worker_ptrs.emplace_back(Buf_ring.b1);
  }
}

void Decoder::Resorting_Start() {
  resorting_running = true;
  resorting_thread =
      std::make_unique<std::thread>([this]() { Resorting_Worker(); });
}

void Decoder::Splice_Start() {
  splice_running = true;
  splice_thread = std::make_unique<std::thread>([this]() { Splice_Worker(); });
}

// Рабочий поток копирования
void Decoder::Copy_Worker() {
  prnt("ss ds;", BGRN, "Copy worker started with timeout", copy_timeout_ms,
       RST);

  while (true) {
    std::list<BufClass> local_queue;
    {
      // Общее правило:
      // Для ВСЕХ wait_for с таймаутом нужно:
      //     Проверять условие остановки потока (!running && empty())
      //     Проверять на таймаут (очередь/список пуст после wait_for)
      //     Использовать continue для пропуска итерации при таймауте
      //     Обеспечивать освобождение мьютекса перед continue/break

      // 1. Ждем данные под замком с таймаутом
      std::unique_lock<std::mutex> lock(copy_mutex);
      copy_cond.wait_for(
          lock, std::chrono::milliseconds(copy_timeout_ms),
          [this]() { return !copy_queue.empty() || !copy_running; });
      // Если поток остановлен и очередь пуста, выходим
      if (!copy_running && copy_queue.empty())
        break;

      // 2. Быстро забираем ВСЕ данные под замком если copy_queue не пуста
      if (copy_queue.empty())
        continue;

      local_queue.swap(copy_queue); // O(1) операция!
    } // ОСВОБОЖДАЕМ замок сразу после доступа к очереди

    try {
      // throw std::runtime_error("Test exception");
      // throw "C-string exception";

      UChar_t *pos = write_ptr.load(std::memory_order_acquire);

      size_t ddd = 0;
      for (auto &item : local_queue) {
        size_t data_size = item.b3 - item.b1;
        ddd = data_size;
        if (!CanWriteData(data_size)) {
          prnt("sss;", BYEL, "Data dropped - buffer full", RST);
          continue;
        }

        Buf_ring.Ring_Write(item);
        write_ptr.store(Buf_ring.u82.b, std::memory_order_release);
      }

      prnt("s x x l d;", "pos:", pos, Buf_ring.u82.b, ddd, first_call);
      // cout << "pos: " << hex << pos << dec << " " << first_call << endl;

      // pos = конец предыдущей записи в Buf_ring: write_start
      // Buf_ring.u82 = кoнец текущей записи в Buf_ring: write_end
      // BufProc.u82.b = конец предыдущего анализа: analysis_start
      BufferRange range(pos, Buf_ring.u82.b, BufProc.u82.b);
      Send_for_Process(range);

    } catch (const std::exception &e) {
      prnt("ss ss;", BRED, "Exception in Copy_Worker:", e.what(), RST);
    } catch (...) {
      prnt("sss;", BRED, "Unknown exception in Copy_Worker", RST);
    }
  }

  prnt("ss;", BGRN, "Copy worker finished", RST);
}

// Вспомогательные методы для Send_for_Process

// отслеживет переход через конец буфера
bool Decoder::HandleRingBufferWrap(BufferRange &range) {
  // UChar_t *analysis_start, UChar_t *write_start, UChar_t *write_end, BufClass
  // &process_buf) {
  if (range.write_end < range.write_start) {
    // это значит, что запись перешла через конец кольцевого буфера
    // копируем необработанные данные из конца буфера в офсет (перед началом)
    size_t data_size =
        range.write_start - range.analysis_start; // необработанный кусок буфера
    UChar_t *new_analysis_start = Buf_ring.b1 - data_size;

    if (new_analysis_start < buffer_storage.data()) {
      prnt("sss x xs;", BRED, "RingBuf offset overflow:", BCYN,
           new_analysis_start, range.write_start, RST);
      return false;
    }

    memmove(new_analysis_start, range.analysis_start, data_size);
    range.analysis_start = new_analysis_start;
  }
  // else: range.analysis_start не изменился

  return true;
}

bool Decoder::FindLastEvent(BufferRange &range) {
  // ищем последнее событие - от конца записи до начала анализа
  // анализ будет до начала последнего события

  // конец записи должен быть позже начала анализа
  if (range.write_end < range.analysis_start) {
    prnt("sss x xs;", BRED, "Write end before analysis start:", BCYN,
         range.write_end, range.analysis_start, RST);
    return false;
  }
  union82 from_pos;
  from_pos.b = range.write_end;
  range.analysis_end = FindEvent_backward(from_pos, range.analysis_start);
  // BufProc.u82.b = process_buf.b3;
  return true;
}

bool Decoder::CheckBufferRanges(BufferRange &range) {
  // проверяем границы буферов
  if (first_call && range.analysis_start == range.write_start) {
    prnt("sss x xs;", BMAG, "First call:", BCYN, range.analysis_start,
         range.write_start, RST);
    first_call = false;
    return true;
  }
  if (range.analysis_start >= range.write_start) {
    prnt("sss x xs;", BRED, "Analysis ahead of write:", BCYN,
         range.analysis_start, range.write_start, RST);
    return false;
  }
  return true;
}

bool Decoder::PrepareProcessBuffer(BufferRange &range) {
  if (!CheckBufferRanges(range) || !HandleRingBufferWrap(range) ||
      !FindLastEvent(range)) {
    return false;
  }
  return true;
}

void Decoder::SendToProcessQueue(BufferRange &range) {
  std::lock_guard<std::mutex> lock(process_mutex);
  BufClass process_buf;
  process_buf.b1 = range.analysis_start;
  process_buf.b3 = range.analysis_end;
  process_buf.buffer_id = next_buffer_id;
  next_buffer_id++;
  process_queue.push_back(process_buf);
  process_cond.notify_one();
}

void Decoder::Send_for_Process(BufferRange &range) {

  if (!PrepareProcessBuffer(range)) {
    BufProc.u82.b = range.write_end; // конец анализа = конец записи
    return;
  }

  BufProc.u82.b = range.analysis_end; // новый конец анализа
  SendToProcessQueue(range);
}

// НОВЫЙ Рабочий поток декодирования
void Decoder::Process_Worker(UInt_t thread_id) {
  prnt("ss d ds;", BGRN,
       "Process worker started with timeout:", process_timeout_ms, thread_id,
       RST);

  while (process_running) {
    std::list<BufClass> local_buffers;
    {
      std::unique_lock<std::mutex> lock(process_mutex);
      process_cond.wait_for(
          lock, std::chrono::milliseconds(process_timeout_ms),
          [this]() { return !process_queue.empty() || !process_running; });

      // ПРОВЕРКА на process_running: выходим сразу, если очередь пуста
      if (!process_running && process_queue.empty())
        break;

      if (!process_queue.empty()) {
        // Забираем ВСЕ доступные буферы для минимизации потерь сигналов
        local_buffers.swap(process_queue);

        // Обновляем позицию этого worker'а после анализа
        // В МЬЮТЕКСЕ для согласованности с CanWriteData()
        if (!local_buffers.empty()) {
          worker_ptrs[thread_id].store(local_buffers.back().b3,
                                       std::memory_order_release);
        }

        prnt("ss d s d ss;", BBLU, "Thread", thread_id, "got",
             (int)local_buffers.size(), "buffers", RST);
      } else {
        continue; // пропускаем если очередь пуста
      }
    } // ОСВОБОЖДАЕМ замок сразу после доступа к очереди

    // Обрабатываем все буферы в пачке БЕЗ мьютекса
    for (auto &Buf : local_buffers) {
      LocalBuf *newbuf_ptr = nullptr;
      try {
        // // Тестовые исключения
        // static int test_counter = 0;
        // if (test_counter++ == 5) {
        // 	throw std::runtime_error("Test exception after 5 iterations");
        // }
        // throw "C-string exception";  // нестандартное исключение

        LocalBuf &newbuf = Dec_Init(Buf);
        Decode_switch(Buf, newbuf.B);
        newbuf.is_ready = true;
        newbuf_ptr = &newbuf;
      } catch (...) {
        // ПРОСТАЯ ЗАГЛУШКА при ошибке обработки
        prnt("ss d l s;", BRED, "Exception processing buffer", thread_id,
             Buf.buffer_id, RST);

        // Без try-catch - если здесь исключение, система неработоспособна
        std::lock_guard<std::mutex> lock(buf_mutex);
        Bufpulses.emplace_back(LocalBuf());
        newbuf_ptr = &Bufpulses.back();
        newbuf_ptr->source_buffer_id = Buf.buffer_id;
        newbuf_ptr->is_ready = true;
      }

      // ОБЩАЯ ЛОГИКА - ВСЕГДА выполняем если newbuf_ptr создан
      if (newbuf_ptr != nullptr) {
        std::lock_guard<std::mutex> lock(buf_mutex);
        // Сортируем по номеру буфера
        Bufpulses.sort([](const LocalBuf &a, const LocalBuf &b) {
          return a.source_buffer_id < b.source_buffer_id;
        });

        // Проверяем, что это наш буфер (должен совпадать номер)
        if (newbuf_ptr->source_buffer_id == Buf.buffer_id) {
          resorting_cond.notify_one();
        }
      }

      // ПРОСТО выводим на экран начало и конец
      // Защищаем от случая, когда newbuf_ptr указывает на заглушку
      if (newbuf_ptr != nullptr) {
        prnt("ss d d s x s x s ls;", BCYN, "Thread/buf", thread_id,
             Buf.buffer_id, "analyzing from:", Buf.b1, "to:", Buf.b3,
             "Bufevents:", Bufpulses.size(), RST);
      }
    }
  }

  prnt("ss ds;", BGRN, "Process worker finished, thread:", thread_id, RST);
} // Process_Worker

// Поток склейки (splice) пока не трогаем, но потом его нужно будет
//  переименовать. По смыслу этот поток должен будет из импульсов
//  создавать события (коллекцию импульсов) - например, make_events.

// Задается переменная opt.buf_lag = число буферов(контейнеров), которое
// участвует в пересортировке, но не участвует в создании событий (make_events)

// Задаем следующие списки контейнеров:

// std::list<LocalBuf> Bufpulses - в каждом контейнере "B" импульсы
//  из проанализированного буфера. Все импульсы внутри контейнера отсортированы.

// std::list<LocalBuf> "придумай_имя1" - список, в котором
//  происходит пересортировка. На входе в поток пересортировки члены этого
//  списка - это Bufpulses, которые поток пересортировки быстро
//  забрал под мьютексом. На выходе - список с пересортированными
//  между импульсами. Т.е. в нем импульсы идут по возрастанию от
//  первого импульса первого буфера к последнему импульсу последнего
//  буфера. Расстояние между буферами не меньше, чем opt.tgate. Этот
//  список участвует только в потоке пересортировки, для него
//  мьютекс не нужен.

// std::list<LocalBuf> "придумай_имя2" - список, который после
//  пересортировки отправляется в make_events. В этот список
//  отправляем контейнеры с головы списка пересортировки, как только
//  длина этого списка начинает превышать opt.buf_lag. Это контейнер,
//  который передается из потока пересортировки в поток создания
//  событий.

// По-видимому, нужно предусмотреть возможный сценарий, когда в
//  результате пересортировки какой-то из членов списка становится
//  пустым.

// СТАРЫЙ Рабочий поток пересортировки
// void Decoder::Resorting_Worker() {
//   prnt("ss;", BGRN, "Resorting worker started", RST);

//   while (resorting_running) {
//     try {
//       std::list<LocalBuf> buffers_to_process;

//       // БЛОК 1: Быстро забрать готовые буферы под мьютексом
//       {
//         std::unique_lock<std::mutex> lock(buf_mutex);

//         // Ждем КОНКРЕТНЫЙ следующий буфер, который ГОТОВ
//         resorting_cond.wait(lock, [this]() {
//           if (!resorting_running)
//             return true;
//           if (Bufpulses.empty())
//             return false;

//           const auto &front_buf = Bufpulses.front();
//           return front_buf.is_ready && front_buf.source_buffer_id ==
//           next_expected_bufnum;
//         });

//         if (!resorting_running)
//           break;

//         // Быстро забираем ВСЕ последовательные готовые буферы
//         while (!Bufpulses.empty() && Bufpulses.front().is_ready &&
//                Bufpulses.front().source_buffer_id == next_expected_bufnum) {

//           buffers_to_process.splice(buffers_to_process.end(), Bufpulses,
//                                     Bufpulses.begin());
//           next_expected_bufnum++;
//         }
//       } // МЬЮТЕКС ОСВОБОЖДЕН

//       // БЛОК 2: Обработка буферов БЕЗ мьютекса
//       for (auto &buf : buffers_to_process) {
//         prnt("ss l s ls;", BYEL, "Resorting buffer:", buf.source_buffer_id,
//              "events:", buf.B.size(), RST);
//         // здесь нужно добавить пересортировку:
//         // все импульсы в буфере buf.B, которые удовлетворяют условию
//         // Tstamp64 <= last.Tstamp64 + opt.tgate, где last - последний
//         импульс в
//         // предыдущем буфере, перемещаем в предыдущий буфер с одновременной
//         // сортировкой.
//         // Если дошли до начала предыдущего буфера, то переходим к следующему
//         // предыдущему и т.д.
//         //
//       } // for
//       // Очищаем временный список
//       //buffers_to_process.clear();

//       // БЛОК 3: Переносим обработанные буферы в Sorted_pulses и уведомляем
//       Splice_Worker if (!buffers_to_process.empty()) {
//         std::lock_guard<std::mutex> lock(resorting_mutex);
//         Sorted_pulses.splice(Sorted_pulses.end(), buffers_to_process);
//         splice_cond.notify_one();
//       }

//     } // try
//     catch (const std::exception &e) {
//       prnt("ss ss;", BRED, "Exception in Splice_Worker:", e.what(), RST);
//     } catch (...) {
//       prnt("sss;", BRED, "Unknown exception in Splice_Worker", RST);
//     }
//   } // while

//   prnt("ss;", BGRN, "Resorting worker finished", RST);
// } // СТАРЫЙ Resorting_Worker

bool Decoder::ResortSingleBuffer(std::list<LocalBuf> &ResortingList,
                                 std::list<LocalBuf>::iterator new_buf_it) {
  ResortingList.remove_if([](const LocalBuf &buf) { return buf.B.empty(); });

  if (ResortingList.empty()) {
    return !new_buf_it->B.empty();
  }

  auto pulse_it = new_buf_it->B.begin();
  while (pulse_it != new_buf_it->B.end()) {

    // ВЫЧИСЛЯЕМ ОДИН РАЗ минимальное время последнего импульса в буфере
    // Если последний импульс в буфере имеет время >= min_last_time,
    // то текущий импульс попадает в окно opt.tgate
    Long64_t min_last_time = pulse_it->Tstamp64 - opt.tgate;

    auto resort_buf_it = ResortingList.rbegin();

    // Начинаем с ПОСЛЕДНЕГО буфера
    if (resort_buf_it->B.back().Tstamp64 < min_last_time) {
      // Импульс не попадает в окно САМОГО ПОСЛЕДНЕГО буфера
      // Все последующие импульсы имеют большее время → прекращаем ВСЮ
      // пересортировку Этот импульс и все следующие остаются в исходном буфере
      break; // Выходим из while (pulse_it != new_buf_it->B.end())
    }

    bool was_moved = false;

    // Проверяем буферы от последнего к первому
    while (resort_buf_it != ResortingList.rend()) {
      // проверяем, что последний импульс в буфере достаточно "новый"
      if (resort_buf_it->B.back().Tstamp64 < min_last_time) {
        // Последний импульс в буфере СЛИШКОМ СТАРЫЙ
        // Импульс не попадает в окно этого буфера
        break;
      }

      // Импульс попадает в окно - пробуем вставить
      auto inserted_pos = sorted_insert(resort_buf_it->B, *pulse_it);

      if (inserted_pos == resort_buf_it->B.begin()) {
        // Импульс вставился в начало этого буфера

        if (std::next(resort_buf_it) == ResortingList.rend()) {
          // Это САМЫЙ РАННИЙ буфер - импульс на своем месте
          pulse_it = new_buf_it->B.erase(pulse_it);
          was_moved = true;
          break;
        } else {
          // Это НЕ самый ранний буфер
          // Удаляем временную вставку и проверяем следующий буфер
          resort_buf_it->B.erase(inserted_pos);
          ++resort_buf_it; // Переходим к более раннему буферу
          // Проверка min_last_time останется той же для следующего буфера
        }
      } else {
        // Импульс вставился не в начало - на своем месте
        pulse_it = new_buf_it->B.erase(pulse_it);
        was_moved = true;
        break;
      }
    }

    if (!was_moved) {
      ++pulse_it;
    }
  }

  // Возвращаем true, если буфер не пустой после обработки
  return !new_buf_it->B.empty();
}

void Decoder::ResortNewBuffers(std::list<LocalBuf> &ResortingList,
                               std::list<LocalBuf> &new_buffers) {

  // Удаляем пустые буферы сразу
  new_buffers.remove_if([](const LocalBuf &buf) { return buf.B.empty(); });

  // Обрабатываем буферы
  for (auto buf_it = new_buffers.begin(); buf_it != new_buffers.end();) {
    // Выполняем пересортировку
    bool should_add = ResortSingleBuffer(ResortingList, buf_it);

    if (should_add) {
      // Перемещаем буфер в ResortingList
      auto splice_it = buf_it++;
      ResortingList.splice(ResortingList.end(), new_buffers, splice_it);
    } else {
      // Буфер стал пустым - просто пропускаем (не добавляем в ResortingList)
      ++buf_it;
    }
  }

  prnt("ss ls;", BCYN,
       "ResortNewBuffers completed, processed buffers:", new_buffers.size(),
       RST);
}

// Рабочий поток пересортировки
void Decoder::Resorting_Worker() {
  prnt("ss ds;", BGRN,
       "Resorting worker started with timeout:", resorting_timeout_ms, RST);

  // Основной список для пересортировки (сохраняется между итерациями)
  std::list<LocalBuf> ResortingList;

  while (resorting_running) {
    try {
      // Временный список для новых буферов (только для текущей итерации)
      std::list<LocalBuf> new_buffers;

      // БЛОК 1: Забираем новые буферы из Bufpulses в new_buffers
      {
        std::unique_lock<std::mutex> lock(buf_mutex);

        // Ждем КОНКРЕТНЫЙ следующий буфер, который ГОТОВ (проверяем весь
        // список)
        resorting_cond.wait_for(
            lock, std::chrono::milliseconds(resorting_timeout_ms), [this]() {
              if (!resorting_running)
                return true;

              // Ищем ЛЮБОЙ готовый буфер с ожидаемым ID во всем списке
              for (const auto &buf : Bufpulses) {
                if (buf.is_ready &&
                    buf.source_buffer_id == next_expected_bufnum) {
                  return true;
                }
              }
              return false;
            });
        // здесь пробудился поток: или вышли по условию, или по таймауту

        // ПРОВЕРКА на resorting_running: выходим сразу, если очередь пуста
        if (!resorting_running && Bufpulses.empty())
          break;

        // Находим и забираем ВСЕ последовательные готовые буферы (начиная с
        // ожидаемого)
        bool found_expected = false;
        auto it = Bufpulses.begin();
        while (it != Bufpulses.end()) {
          if (it->is_ready && it->source_buffer_id == next_expected_bufnum) {
            // Нашли ожидаемый буфер - переносим в new_buffers
            auto transfer_it = it++;
            new_buffers.splice(new_buffers.end(), Bufpulses, transfer_it);
            next_expected_bufnum++;
            found_expected = true;

            prnt("ss l s ls;", BMAG,
                 "Found new buffer:", new_buffers.back().source_buffer_id,
                 "total new buffers:", new_buffers.size(), RST);
          } else if (found_expected) {
            // Уже нашли ожидаемый буфер, но следующий не подходит -
            // останавливаемся
            break;
          } else {
            // Еще не нашли ожидаемый буфер - продолжаем поиск
            ++it;
          }
        }
      } // МЬЮТЕКС ОСВОБОЖДЕН

      // БЛОК 2: Пересортировка новых буферов относительно существующих
      if (!new_buffers.empty()) {
        ResortNewBuffers(ResortingList, new_buffers);

        // Временная заглушка: просто добавляем новые буферы в конец
        ResortingList.splice(ResortingList.end(), new_buffers);
        prnt("ss l l ls;", BYEL, "Added", new_buffers.size(),
             "new buffers to ResortingList, total:", ResortingList.size(), RST);
      }

      // БЛОК 3: Передача готовых буферов в Sorted_pulses
      if (ResortingList.size() > opt.buf_lag) {
        size_t buffers_to_transfer = ResortingList.size() - opt.buf_lag;

        std::lock_guard<std::mutex> lock(resorting_mutex);

        // ✅ Переносим буферы с головы списка ОДНОЙ операцией
        auto transfer_end = ResortingList.begin();
        std::advance(transfer_end, buffers_to_transfer);

        Sorted_pulses.splice(Sorted_pulses.end(), ResortingList,
                             ResortingList.begin(), transfer_end);

        prnt("ss l l ls;", BCYN, "Transferred", buffers_to_transfer,
             "buffers to Sorted_pulses, remaining:", ResortingList.size(), RST);

        // TODO: Уведомить поток создания событий
        // make_events_cond.notify_one();
      }

    } catch (const std::exception &e) {
      prnt("ss ss;", BRED, "Exception in Resorting_Worker:", e.what(), RST);
    } catch (...) {
      prnt("sss;", BRED, "Unknown exception in Resorting_Worker", RST);
    }
  }

  prnt("ss;", BGRN, "Resorting worker finished", RST);
} // Resorting_Worker

// Рабочий поток склейки
void Decoder::Splice_Worker() {
  prnt("ss;", BGRN, "Splice worker started", RST);

  while (splice_running) {
    try {
      std::list<LocalBuf> buffers_to_process;

      // БЛОК 1: Быстро забрать готовые буферы под мьютексом
      {
        std::unique_lock<std::mutex> lock(buf_mutex);

        // Ждем КОНКРЕТНЫЙ следующий буфер, который ГОТОВ
        splice_cond.wait(lock, [this]() {
          if (!splice_running)
            return true;
          if (Bufpulses.empty())
            return false;

          const auto &front_buf = Bufpulses.front();
          return front_buf.is_ready &&
                 front_buf.source_buffer_id == next_expected_bufnum;
        });

        if (!splice_running)
          break;

        // Быстро забираем ВСЕ последовательные готовые буферы
        while (!Bufpulses.empty() && Bufpulses.front().is_ready &&
               Bufpulses.front().source_buffer_id == next_expected_bufnum) {

          buffers_to_process.splice(buffers_to_process.end(), Bufpulses,
                                    Bufpulses.begin());
          next_expected_bufnum++;
        }
      } // МЬЮТЕКС ОСВОБОЖДЕН

      // БЛОК 2: Обработка буферов БЕЗ мьютекса
      for (auto &buf : buffers_to_process) {
        prnt("ss l s ls;", BYEL, "Splicing buffer:", buf.source_buffer_id,
             "events:", buf.B.size(), RST);
        /*   YK доделать
                // Перемещаем события в глобальный список
                if (!buf.B.empty() && !Levents.empty()) {

                  Long64_t T_last = Levents.rbegin()->Tstmp + opt.tgate;
                  auto it = buf.B.begin();
                  while (it != buf.B.end() && it->Tstmp <= T_last) {
                    // Вставляем все импульсы события в глобальный список
                    for (auto &pulse : it->pulses) {
                      Event_Insert_Pulse(Levents, pulse);
                    }
                    it = buf.B.erase(it);
                  }
                  // Перемещаем оставшиеся события
                  Levents.splice(Levents.end(), buf.B);

                  size_t after_size = buf.B.size();
                  if (after_size != 0) {
                    prnt("ss l l ls;", BRED, "SPLICE ERROR: buffer",
           buf.source_buffer_id, "not emptied:", after_size, RST);
                  }
                }
        */
      } // for
      // Очищаем временный список
      buffers_to_process.clear();
    } // try
    catch (const std::exception &e) {
      prnt("ss ss;", BRED, "Exception in Splice_Worker:", e.what(), RST);
    } catch (...) {
      prnt("sss;", BRED, "Unknown exception in Splice_Worker", RST);
    }
  } // while

  prnt("ss;", BGRN, "Splice worker finished", RST);
} // Splice_Worker

// Метод для добавления данных в очередь извне
void Decoder::Add_to_copy_queue(UChar_t *data, size_t size) {
  std::lock_guard<std::mutex> lock(copy_mutex);
  // copy_queue.push_back({data, size});
  BufClass buf;
  buf.b1 = data;
  buf.b3 = data + size;

  prnt("ss x ls;", BBLU, "Copy:", buf.b1, size, RST);

  copy_queue.push_back(std::move(buf));
  copy_cond.notify_one(); // Будим поток даже если copy_running=false
}

bool Decoder::CanWriteData(size_t data_size) {
  // Берет текущую позицию записи: current_write = write_ptr
  // Для каждого worker'а вычисляет worker_ptr: указывает на КОНЕЦ данных,
  // которые worker БУДЕТ обрабатывать.
  // Находит БЛИЖАЙШИЙ worker_ptr: Выбирает worker, который ближе всего к
  //   текущей позиции записи - это и есть самый "опасный" ограничитель.
  // Вычисляет безопасный предел
  //     : Позицию, до которой можно безопасно писать.
  // возвращает true, если есть место для записи

  if (first_call) {
    return true; // При первом вызове пропускаем проверки
  }

  UChar_t *current_write = write_ptr.load(std::memory_order_acquire);
  UChar_t *safe_limit = Buf_ring.b1;

  // Находим ближайший worker_ptr
  size_t min_distance = SIZE_MAX;
  for (auto &worker_ptr : worker_ptrs) {
    UChar_t *ptr = worker_ptr.load(std::memory_order_acquire);
    size_t distance;

    if (ptr >= current_write) {
      distance = ptr - current_write;
    } else {
      distance = (ptr - Buf_ring.b1) + (Buf_ring.b3 - current_write);
    }

    if (distance < min_distance) {
      min_distance = distance;
      safe_limit = ptr;
    }
  }

  // Вычисляем доступное место
  size_t available_space;
  if (safe_limit >= current_write) {
    available_space = safe_limit - current_write;
  } else {
    available_space =
        (safe_limit - Buf_ring.b1) + (Buf_ring.b3 - current_write);
  }

  return data_size <= available_space;
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

LocalBuf &Decoder::Dec_Init(BufClass &Buf) {
  std::lock_guard<std::mutex> lock(buf_mutex);

  Bufpulses.emplace_back(LocalBuf());
  LocalBuf &new_buf = Bufpulses.back();
  new_buf.source_buffer_id = Buf.buffer_id;
  new_buf.is_ready = false; // Пока не готов
  return new_buf;
  // if (!IsEventStart_36(Buf.u82)) {
  // }

  // if (!is_start) {
  //   //prnt("ss d ds;",KYEL,"bad buf start: ",nevents,frmt,RST);
  //   ++crs->errors[ER_START]; //bad buf start
  //   // добавить поиск первого события (?)
  // }
}
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

  PulseClass ipls = crs->dummy_pulse;

  union82 uu; // текущее положение в буфере
  uu.b = Buf.b1;
  UChar_t frmt;

  // uu.b+1 - чтобы не выйти за границы если нет 2-байтного выравнивания
  while (uu.b + 1 < Buf.b3) {
    frmt = (uu.b[1] & 0x70) >> 4;
    short data = *uu.us & 0xFFF;
    UChar_t ch = (*uu.us & 0x8000) >> 15;

    if (frmt && ch != ipls.Chan) { // channel mismatch
      ++crs->errors[ER_MIS];
      ipls.ptype |= P_BADCH;
      uu.us++;
      continue;
    }

    // prnt("ss l d d ds;",BGRN,"CH:",idx2,frmt,ch,ipls.ptype,RST);

    if (frmt == 0) {
      // ipls.ptype&=~P_NOLAST; //pulse has stop

      // analyze previous pulse
      if (ipls.ptype == 0) {
        PulseAna(ipls);
        sorted_insert(pc, ipls);
        // YK Event_Insert_Pulse(*Blist, ipls);
      }

      // test ch, then create new pulse
      if (ch >= opt.Nchan) { // bad channel
        ++crs->errors[ER_CH];
        ipls.ptype |= P_BADCH;
        uu.us++;
        continue;
      }

      ipls = PulseClass();
      crs->npulses++;
      ipls.Chan = ch;
      ipls.Tstamp64 = data; //+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];
    } else if (frmt < 4) {
      Long64_t t64 = data;
      ipls.Tstamp64 += (t64 << (frmt * 12));
    } else if (frmt == 4) {
      ipls.Counter = data;
    } else if (frmt == 5) {
      if ((int)ipls.sData.size() >= cpar.Len[ipls.Chan]) {
        // cout << "2: Nsamp error: " << ipls.sData.size()
        //      << " " << (int) ch << " " << (int) ipls.Chan
        //      << " " << idx2
        //      << endl;
        ipls.ptype |= P_BADSZ;
      }
      // else {
      ipls.sData.push_back((data << 21) >> 21);
      // cout << "decode2b: " << idx2 << endl;
      // }
    }

    uu.us++;
  } // while

  if (ipls.ptype == 0) {
    PulseAna(ipls);
    sorted_insert(pc, ipls);
    // YK Event_Insert_Pulse(*Blist, ipls);
  }

  // Dec_End(Blist,0,255);
} // decode22

// void Decoder::Decode36(BufClass &Buf, eventlist *Blist) {
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
  uu.b = Buf.b1;
  UChar_t frmt;

  // uu.b+7 - чтобы не выйти за границы если нет 8-байтного выравнивания
  while (uu.b + 7 < Buf.b3) {
    frmt = (uu.b[6] & 0xF0) >> 4;
    data = *uu.ul & sixbytes;
    UChar_t ch = uu.b[7];

    // if (initevt!=3) {
    //   prnt("ss l d d d
    //   ds;",BGRN,"d35:",idx1,ch,frmt,initevt,ipls.ptype,RST);
    // }

    if (frmt && ch != ipls.Chan) { // channel mismatch
      // if (initevt!=3) {
      // 	prnt("ss d d d ds;",BRED,"m35:",ch,ipls.Chan,frmt,initevt,RST);
      // }
      ++crs->errors[ER_MIS];
      ipls.ptype |= P_BADCH;
      uu.ul++;
      continue;
    }

    // prnt("sss;",BRED,"Good",RST);
    // prnt(" d",frmt);

    switch (frmt) {
    case 0:
      // prnt("ss l d d ds;",BBLU,"CH:",idx1,ch,ipls.Chan,ipls.ptype,RST);
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
        // idx1+=8;
        // continue;
        break;
      }

      ipls = PulseClass();
      crs->npulses++;
      ipls.Chan = ch;
      ipls.Tstamp64 = data; //+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];

      if (ch == 255) {    // start channel
        ipls.Spin |= 128; // bit 7 - hardware counters
        // prnt("ss ds;",BYEL,"STARTCH:",ch,RST);
      }

      break;

    case 1:
      ipls.Spin |= uu.b[5] & 1;
      ipls.Counter = data & 0xFFFFFFFFFF;
      // prnt("ss d l ls;",BMAG,"CONT1:",ch,ipls.Tstamp64,ipls.Counter,RST);
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
          // Float_t f33 = f32+1;
          // Float_t f34 = f32-1;
          // Double_t f32 = (d32<<8)>>8;

          // printf("d32_0: %d %x
          // %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f32/256.0f);
          // printf("d32_1: %d %x
          // %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f33/256.0f);
          // printf("d32_2: %d %x
          // %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f34/256.0f);

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

// void Decoder::Add_pulse_to_container(pulsecontainer &pc, PulseClass &pls) {
//   sorted_insert(pc, pls);
// }

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
      // if (ipls.Peaks.size()!=1) {
      // 	cout << "size!!!: " << ipls.Peaks.size() << endl;
      // }
      ipls2.PeakAna33();
      crs->CheckDSP(ipls, ipls2);
      opt.sTg[ipls.Chan] = t_tg;
    }
    // else {
    //   // для sTg=3,6,7 ищем пересечение нижнего порога/нуля
    //   switch (opt.sTg[ipls.Chan]) {
    //   case 3:
    //   case 6:
    // 	break;
    //   default:
    // 	break;
    //   }
    // }
  }

  // prnt("ss d l d f fs;",BRED, "WD:", ipls.Chan, ipls.Tstamp64, ipls.Pos,
  // ipls.Time, ipls.Width, RST);

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
  // ipls.Peaks.push_back(PeakClass());
  // PeakClass *ipk=&ipls.Peaks[0];
  Float_t Area0;

#ifdef APK
  ipls.ppk = pk;
#endif

  ipls.Pos = cpar.Pre[ipls.Chan];

  Area0 = pk.A / p_len[ipls.Chan];
  ipls.Base = pk.C / b_len[ipls.Chan];
  ipls.Area = Area0 - ipls.Base;
  // ipls.Area0=pk.A/p_len[ipls.Chan];
  // ipls.Base=pk.C/b_len[ipls.Chan];
  // ipls.Area=ipls.Area0 - ipls.Base;

  // if (ipls.Tstamp64<500000) {
  //   prnt("ss d l d d f f f fs;",BYEL,"AA:",ipls.Chan,ipls.Tstamp64,
  // 	 pk.A,pk.C,ipls.Base,ipls.Area,p_len[ipls.Chan],b_len[ipls.Chan],RST);
  // }

  ipls.Height = pk.H;

  // if (cpar.Trg[ipls.Chan]>=0) { //use hardware timing
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
      // cout << "ZZZZ pos: " << ipls.Pos << " " << kk << endl;
      // if (ipls.Pos>=kk && (int)ipls.sData.size()>ipls.Pos+1) {
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
    // ipls.Time=pk.RX;
  } else { // use software timing
    ipls.PeakAna33(true);
  }

  // if (ipls.Tstamp64<500000) {
  //   prnt("ss d l l l f f f fs;",BYEL,"AA:",ipls.Chan,ipls.Tstamp64,
  // 	 pk.QX,pk.RX,ipls.Time,ipls.Area,p_len[ipls.Chan],b_len[ipls.Chan],RST);
  // }

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
    // prnt("ss d l d f fs;",BGRN, "WD:", ipls.Chan, ipls.Tstamp64,
    // ipls.Pos, ipls.Time, ipls.Width, RST);
    if (ipls.Width < -99)
      ipls.Width = -99;
    if (ipls.Width > 99)
      ipls.Width = 99;
  }

  // if (ipls.Tstamp64<500000) {
  //   prnt("ss d l d d f f f f fs;",BBLU,"BB:",ipls.Chan,ipls.Tstamp64,
  // 	 pk.A,pk.C,ipls.Base,ipls.Area,p_len[ipls.Chan],b_len[ipls.Chan],
  // 	 ipls.Sl2,RST);
  // }

  // ipls.Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipls.Area;

  // if (opt.Bc[ipls.Chan]) {
  //   ipls.Area+=opt.Bc[ipls.Chan]*ipls.Base;
  // }

  // if (ipls.Chan==15) {
  //   prnt("ssl d d fs;","PP33:
  //   ",BBLU,ipls.Tstamp64,ipls.Pos,cpar.Pre[ipls.Chan],ipls.Time,RST);
  // }

} // MakePk
