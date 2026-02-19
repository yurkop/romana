#pragma once
#include "romana.h"

typedef std::list<PulseClass> pulsecontainer; // список импульсов в буфере
typedef std::vector<EventClass> eventcontainer; // вектор событий в буфере
typedef std::vector<EventClass>::iterator eventiter; // итератор по вектору событий

/*
class PulseBuf {
public:
  pulsecontainer PLS; // список импульсов в буфере
  UInt_t source_buffer_id; // ID исходного буфера, откуда события
  // Флаг готовности
  std::atomic<UChar_t> flag{0};
  // 0 - скопировано, 1 - обработано, 2 - отсортировано, 3 - make_events.
};
*/

class EventBuf {
public:
  pulsecontainer PLS; // список импульсов в буфере
  eventcontainer EVT; // список событий в буфере
  UInt_t source_buffer_id; // ID исходного буфера, откуда события
  // Флаг готовности
  std::atomic<UChar_t> flag{0};
  // 0: буфер создан, данные скопированы, можно декодировать
  // 1: буфер декодирован, можно сортировать
  // 2: активная сортировка закончена, но может участвовать в пассивной
  // 3: сортировка закончена, можно создавать события
  // 4: события созданы, можно анализировать
  // 5: анализ закончен, можно удалять
};

#ifdef TIMING
void LogTime(int stage, Long64_t duration_us, Long64_t size=0, Long64_t npls=0);

class SimpleTimer {
    std::chrono::steady_clock::time_point start;
    //Decoder* dec;
    int stage;
    Long64_t size;
    Long64_t npls;

public:
  SimpleTimer(int s, Long64_t z=0, Long64_t np=0);
  ~SimpleTimer();
  void SetSize(Long64_t sz);
  void SetNpls(Long64_t np);
};

#endif

class Decoder {
private:
  const UInt_t worker_timeouts = 1; // 1 или 2E6(бесконечность)
  const UInt_t copy_timeout_ms = 110 * worker_timeouts;
  const UInt_t decode_timeout_ms = 50 * worker_timeouts;
  const UInt_t resorting_timeout_ms = 70 * worker_timeouts;
  const UInt_t makeevent_timeout_ms = 80 * worker_timeouts;
  const UInt_t ana_timeout_ms = 90 * worker_timeouts;

  Long64_t max_time_diff; // 1 секунда, например
  // UInt_t next_expected_bufnum = 1; // номер ожидаемого буфера

  static std::atomic<bool> tables_initialized;
  // прямой массив как быстрая альтернатива switch-case
  static const int MAX_HASH = 128;
  // 1. Определяем тип функции
  using FindBackwardFunc = UChar_t *(Decoder::*)(union82, UChar_t *);
  using DecodeFunc = void (Decoder::*)(BufClass &, pulsecontainer &);
  // 2. Объявляем таблицу
  static FindBackwardFunc find_backward_table[MAX_HASH];
  static DecodeFunc decode_table[MAX_HASH];
  // 3. Объявляем метод инициализации
  static void init_all_tables();

  Double_t b_len[MAX_CH], p_len[MAX_CH],
      w_len[MAX_CH]; // length of window for bkg, peak and width integration in
                     // DSP
  Double_t b_mean[MAX_CH], p_mean[MAX_CH],
      w_mean[MAX_CH]; // length of window for bkg, peak and width integration in
                      // DSP

  // sorted_move определено как шаблонная функция внутри
  // класса Decoder в заголовочном файле (decoder.h) Компилятор должен видеть
  // полное определение шаблона в каждой единице трансляции (translation unit),
  // где он используется

  // Пересорировка последнего элемента в контейнере. Перемещает последний
  // элемент в нужное место.

  template <typename Container> void sorted_move(Container &cont) {
    static_assert(
        std::is_same_v<Container, std::list<typename Container::value_type>> ||
        std::is_same_v<Container, std::vector<typename Container::value_type>> ||
        std::is_same_v<Container, std::deque<typename Container::value_type>>,
        "Container must be list, vector or deque"
    );

    if (cont.size() <= 1)
      return;

    // перемещаем последний элемент в нужное место
    auto last = std::prev(cont.end());

  // Сравниваем с предпоследним: нужно, т.к. splice с предпоследним запрещено
    auto prev = std::prev(last);
    if (!(*last < *prev)) {
      return; // уже на месте
    }

    // Проверка на аномалию
    if (prev->Tstamp64 > last->Tstamp64 + max_time_diff) {
        // Аномалия - элемент "сильно меньше"
        return;  // оставляем на месте
    }
    
    // для list
    if constexpr (std::is_same_v<Container,
                                 std::list<typename Container::value_type>>) {
      // Начинаем с пред-предпоследнего элемента (предпоследний уже прошли)
      for (auto rit = std::next(cont.rbegin()); rit != cont.rend(); ++rit) {
        if (!(*last < *rit)) {
          // Нашли позицию
          cont.splice(rit.base(), cont, last);
          return;
        }
      }
      // Должен быть в начале
      cont.splice(cont.begin(), cont, last);
    } else { // Для vector/deque
      auto value = *last;
      auto it = std::upper_bound(cont.begin(), last, value);
      if (it != last) { // Если нужно переместить
        // Сдвигаем элементы между it и last
        std::rotate(it, last, cont.end());
      }
      // Иначе уже на месте
    }
  }

  //-------------------------------------

// вставка в отсортированный контейнер с одновременной сортировкой

// sorted_insert определено как шаблонная функция внутри класса Decoder в
// заголовочном файле (decoder.h) Компилятор должен видеть полное определение
// шаблона в каждой единице трансляции (translation unit), где он используется

template <typename Container, typename T>
typename Container::iterator sorted_insert(Container &cont, const T &value) {
  if (cont.empty() || !(value < cont.back())) {
    cont.push_back(value);
    return std::prev(cont.end());
  }
  // Определяем стратегию по типу контейнера
  if constexpr (std::is_same_v<Container, std::list<T>>) {
    // Для list - линейный поиск с конца
    for (auto it = cont.rbegin(); it != cont.rend(); ++it) {
      if (!(value < *it)) {
        return cont.insert(it.base(), value);
      }
    }
    return cont.insert(cont.begin(), value);
  } else {
    // Для vector/deque - бинарный поиск
    auto it = std::upper_bound(cont.begin(), cont.end(), value);
    return cont.insert(it, value);
  }
  }

  // Вспомогательные методы для Send_for_Decode
  bool CheckBufferRanges(BufClass &range);
  bool HandleRingBufferWrap(BufClass &range);
  bool FindLast(BufClass &range);
  void SendToDecodeQueue(BufClass &range);
  bool PrepareDecodeBuffer(BufClass &range);

  // Вспомогательные методы для Resorting_Worker
  void ResortSingleBuffer(std::list<EventBuf>::iterator itr);
  bool UpdateBoundary(std::list<EventBuf>::iterator itr);
  // Long64_t Add_Offset_To_Buffer(std::list<EventBuf>::iterator itr);

  void Make_Events(std::list<EventBuf>::iterator itr);

  // Контейнер активных позиций, откуда потоки могут читать
  std::list<UChar_t *> active_read_starts;
  std::mutex active_mutex; // защищает active_read_starts

public:
  // bool first_call = false;
  int crs_module = 0;

#ifdef TIMING
  double timing[MAX_TIMING];
  const char *timing_label[MAX_TIMING] = {
      "Acquire",   "Copy",      "Decode", "DecodePls",
      "Resorting", "MakeEvent", "Ana",    "Delete",
  };
  const char *timing_units[MAX_TIMING] = {" us/kB", " us/kB", " us/kB",
                                          "ns/pls", "ns/pls", "ns/pls",
                                          "ns/pls", "ns/pls"};
#endif

  std::vector<UChar_t>
      buffer_storage;
  BufClass
      Buf_ring; // указатель на буфер, куда пишутся и где декодируются данные
  // начало Buf_ring.b1 сдвинуто вправо на o_size

  Long64_t ring_size; // размер кольцевого буфера

  double ring_used{0}; // процент использования кольцевого буфера

  std::mutex buf_mutex;
  std::mutex ana_mutex;
  UInt_t next_buffer_id = 0; // Генератор ID: номер буфера (=идентификатор)
  // Управление безопасной записью
  std::atomic<UChar_t *> write_ptr{nullptr}; // текущая позиция записи
  
  // std::deque<std::atomic<UChar_t *>> worker_ptrs; // позиции каждого worker'а
  // NOTE: worker_ptrs обновляется сразу после извлечения буфера,
  //       до начала его декодирования. Это НОРМАЛЬНО, потому что:
  //         - буфер уже полностью записан и не меняется;
  //         - Decode_Worker обязательно прочитает весь диапазон [write_start,
  //         write_end];
  //         - worker_ptrs защищает от wrap-around в кольцевом буфере;
  //         - задержка обновления приведёт к ложному освобождению памяти.
  //       Таким образом, раннее обновление — корректно и необходимо.

  // Поток копирования и связанные переменные
  std::unique_ptr<std::thread> copy_thread;
  std::atomic<bool> copy_running{false};
  std::list<BufClass> copy_queue;
  std::mutex copy_mutex;
  std::condition_variable copy_cond;

  // Потоки декодера и связанные переменные
  UInt_t num_decode_threads{0}; // количество потоков декодера
  struct DecodeItem {
    BufClass buf; // Исходный BufClass для декодирования
    std::list<EventBuf>::iterator local_buf_itr;
    // Итератор на соответствующий EventBuf в Bufpulses
  };
  std::list<std::unique_ptr<std::thread>> decode_threads;
  std::atomic<bool> decode_running{false};
  std::list<DecodeItem> decode_queue;
  std::mutex decode_mutex;
  std::condition_variable decode_cond;

  // Поток межбуферной сортировки и связанные переменные
  std::unique_ptr<std::thread> resorting_thread;
  std::atomic<bool> resorting_running{false};
  std::mutex resorting_mutex;
  std::condition_variable resorting_cond;

  // Поток создания событий и связанные переменные
  std::unique_ptr<std::thread> makeevent_thread;
  std::atomic<bool> makeevent_running{false};
  std::condition_variable makeevent_cond;

  // Поток анализа и связанные переменные
  std::unique_ptr<std::thread> ana_thread;
  std::atomic<bool> ana_running{false};
  std::condition_variable ana_cond;

  // boundary (итератор) - граница между resorting и make_events.
  // Пересортировка происходит ТОЛЬКО для буферов от конца до
  // boundary, включая его.
  // Обновляется ТОЛЬКО в UpdateBoundary (потоком RESORTING) под
  // boundary_mutex.
  // В MAKE_EVENT создаются события от начала списка до boundary (не включая
  // его).
  std::list<EventBuf>::iterator boundary;
  std::mutex boundary_mutex;
  // NOTE: boundary используется в двух потоках (RESORTING и MAKE_EVENT),
  // но все операции чтения и записи защищены boundary_mutex.
  // safe_boundary — локальная копия, полученная под мьютексом,
  // поэтому удаление происходит по консистентному снимку состояния.
  // Гарантируется отсутствие состояния гонки.

  // time offset
  // std::atomic<Long64_t> T64_offset{0};
  // std::atomic<UInt_t> offset_buffer_id{UINT_MAX};
  // std::atomic<bool> offset_flag{false};

  // std::unique_ptr<std::thread> splice_thread;
  // std::atomic<bool> splice_running{false};
  // // std::list<BufClass> splice_queue;
  // // std::mutex splice_mutex;
  // std::condition_variable splice_cond;

  /*
  std::list<PulseBuf> Bufpulses;
  // список буферов. в каждом буфере - контейнер с импульсами
  // NOTE: local_buf_ptr указывает на элемент в Bufpulses, который
  // гарантированно не будет удалён до завершения обработки:
  //  - Decode_Worker использует указатель ДО установки флага в 1;
  //  - Удаление в MakeEvent происходит ТОЛЬКО для буферов до boundary,
  //    и только после достижения флага значения 3;
  //  - Таким образом, use-after-free невозможен при штатной работе.
  // ВАЖНО: Decoder_Reset() должен вызываться ТОЛЬКО после Decoder_Stop(),
  //        иначе возможна ситуация обращения к удалённому объекту.
  std::list<PulseBuf>::iterator pulses_end;
  // "вечный end" - указатель на пустой буфер в конце, который
  // никогда не инвалидируется (в отличие от Bufpulses.end() - инвалидируется
  // при emplace_back())
*/

  std::list<EventBuf> Bufevents;
  // список буферов. в каждом буфере - контейнер с событиями
  std::list<EventBuf>::iterator events_end;
  // "вечный end" - указатель на пустой буфер в конце, который
  // никогда не инвалидируется (в отличие от Bufevents.end() - инвалидируется
  // при emplace_back())

  std::atomic<size_t> Bufsize{0}; //текущее число событий в Bufevents
  std::atomic<size_t> Nevents{0}; //полное число проанализированных событий

  // eventlist Levents; // global list of events
  // std::list<pulsecontainer> allpulses; //список списков импульсов в буфере
public:
  Decoder();
  ~Decoder();
  void Decoder_Reset();
  void Decoder_Resize(Long64_t r_size, Long64_t o_size);
  void Decoder_Start(Long64_t r_size, Long64_t o_size, bool b_acq, int module);
  void Decoder_Stop() /*noexcept*/;

  void Copy_Start();
  void Decode_Start(int num_threads);
  void Resorting_Start();
  void MakeEvent_Start();
  void Ana_Start();

  void Copy_Worker(); // блок копирования
  void Send_for_Decode(BufClass &range);
  void Decode_Worker(UInt_t thread_id); // рабочий поток декодирования
  void Resorting_Worker(); // рабочий поток пересортировки между буферами
  void MakeEvent_Worker();    // рабочий поток создания событий
  void Ana_Worker(); // рабочий поток анализа событий

  void Delete_Buf1(std::list<EventBuf>::iterator itr); // удаление буферов из Bufevents
  //void Delete_Buf2(std::list<EventBuf>::iterator itr); // удаление буферов из Bufevents

  // Метод для добавления данных в очередь извне
  void Add_to_copy_queue(UChar_t *data, size_t size);
  bool CanWriteData(Long64_t data_size);
  inline bool IsEventStart_1(union82 &u82);
  inline bool IsEventStart_3(union82 &u82);
  inline bool IsEventStart_22(union82 &u82);
  inline bool IsEventStart_36(union82 &u82);
  inline bool IsEventStart_79(union82 &u82);
  bool IsEventStart(union82 &u82);
  UChar_t *FindEvent_backward_1(union82 From, UChar_t *To);
  UChar_t *FindEvent_backward_3(union82 From, UChar_t *To);
  UChar_t *FindEvent_backward_22(union82 From, UChar_t *To);
  UChar_t *FindEvent_backward_36(union82 From, UChar_t *To);
  UChar_t *FindEvent_backward_79(union82 From, UChar_t *To);
  UChar_t *FindEvent_backward(union82 From, UChar_t *To);
  // void Dec_End(eventlist &Blist, BufClass& Buf);
  void Decode_switch(BufClass &Buf, pulsecontainer &pc);
  void Decode22(BufClass &Buf, pulsecontainer &pc);
  void Decode36(BufClass &Buf, pulsecontainer &pc);
  void Add_pulse_to_container(pulsecontainer &pc, PulseClass &pls);
  void Event_Insert_Pulse(eventlist &Elist, PulseClass &pls);
  void PulseAna(PulseClass &ipls);
  void MakePk(PkClass &pk, PulseClass &ipls);
};
