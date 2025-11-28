#pragma once
#include "romana.h"

class LocalBuf {
public:
  eventlist B;          // list of buffers for decoding
  UInt_t source_buffer_id;  // ID исходного буфера, откуда события
  bool is_ready{false}; // Флаг готовности
};
class Decoder {
private:
  UInt_t next_expected_bufnum = 1; // номер ожидаемого буфера

  static std::atomic<bool> tables_initialized;
  // прямой массив как быстрая альтернатива switch-case
  static const int MAX_HASH = 128;
  // 1. Определяем тип функции
  using FindBackwardFunc = UChar_t *(Decoder::*)(union82, UChar_t *);
  using DecodeFunc = void (Decoder::*)(BufClass&, eventlist*);
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

// вспомогательные структуры для Send_for_Process
struct BufferRange {
UChar_t *write_start; // - предыдущий конец записи
UChar_t *write_end;   // - текущий конец записи
UChar_t *analysis_start; // - вход: конец предыдущего анализа, выход: начало нового
UChar_t *analysis_end; // - выход: конец нового анализа

BufferRange(UChar_t *ws, UChar_t *we, UChar_t *as)
: write_start(ws), write_end(we), analysis_start(as) {}
};

  // 🔧 Вспомогательные методы для Send_for_Process
  bool CheckBufferRanges(BufferRange &range);
  bool HandleRingBufferWrap(BufferRange &range);
  bool FindLastEvent(BufferRange &range);
  void SendToProcessQueue(BufferRange &range);
  bool PrepareProcessBuffer(BufferRange &range);

public:
  bool first_call = false;
  int crs_module = 0;

  std::vector<UChar_t> buffer_storage;
  BufClass Buf_ring; // указатель на буфер, куда пишутся данные
  // начало Buf_ring.b1 сдвинуто вправо на o_size
  BufClass BufProc; // тот же Buf_ring, но используется для process (декод)
  std::mutex buf_mutex;
  UInt_t next_buffer_id = 0; // Генератор ID: номер буфера (=идентификатор)
  // Управление безопасной записью
  std::atomic<UChar_t *> write_ptr{nullptr}; // текущая позиция записи
  std::deque<std::atomic<UChar_t *>> worker_ptrs; // позиции каждого worker'а

  // Поток копирования и связанные переменные
  std::unique_ptr<std::thread> copy_thread;
  std::atomic<bool> copy_running{false};
  std::list<BufClass> copy_queue;
  std::mutex copy_mutex;
  std::condition_variable copy_cond;

  // Потоки анализа и связанные переменные
  UInt_t num_process_threads{4}; // количество потоков анализа
  std::vector<std::unique_ptr<std::thread>> process_threads;
  std::atomic<bool> process_running{false};
  std::list<BufClass> process_queue;
  std::mutex process_mutex;
  std::condition_variable process_cond;

  // Поток склейки и связанные переменные
  std::unique_ptr<std::thread> splice_thread;
  std::atomic<bool> splice_running{false};
  // std::list<BufClass> splice_queue;
  std::mutex splice_mutex;
  std::condition_variable splice_cond;

  std::list<LocalBuf> Bufevents; // list of buffers for decoding
  eventlist Levents;             // global list of events

public:
  Decoder();
  ~Decoder();
  void Decode_Resize(Long64_t r_size, Long64_t o_size);
  void Decode_Start(Long64_t r_size, Long64_t o_size, bool b_acq, int module);
  void Decode_Stop() /*noexcept*/;

  void Copy_Start();
  void Process_Start(int num_threads);
  void Splice_Start();

  void Copy_Worker(); // блок копирования
  void Send_for_Process(BufferRange &range);
  void Process_Worker(UInt_t thread_id); // рабочий поток анализа
  void Splice_Worker(); // рабочий поток склейки

  // Метод для добавления данных в очередь извне
  void Add_to_copy_queue(UChar_t *data, size_t size);
  bool CanWriteData(size_t data_size);
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
  LocalBuf &Dec_Init(BufClass &Buf);
  // void Dec_End(eventlist &Blist, BufClass& Buf);
  void Decode_switch(BufClass &Buf, eventlist* Blist);
  void Decode22(BufClass &Buf, eventlist* Blist);
  void Decode36(BufClass &Buf, eventlist* Blist);
  void Event_Insert_Pulse(eventlist &Elist, PulseClass &pls);
  void PulseAna(PulseClass &ipls);
  void MakePk(PkClass &pk, PulseClass &ipls);
};
