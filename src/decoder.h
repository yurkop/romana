#pragma once
#include "romana.h"

class LocalBuf {
public:
  eventlist B;  // list of buffers for decoding
  Long64_t num; // номер буфера
};
class Decoder {
private:
  static std::atomic<bool> tables_initialized;
  // прямой массив как быстрая альтернатива switch-case
  static const int MAX_HASH = 128;
  // 1. Определяем тип функции
  using FindBackwardFunc = UChar_t*(Decoder::*)(union82, UChar_t*);    
  using DecodeFunc = void (Decoder::*)(BufClass&);    
  // 2. Объявляем таблицу
  static FindBackwardFunc find_backward_table[MAX_HASH];
  static DecodeFunc decode_table[MAX_HASH];
  // 3. Объявляем метод инициализации
  static void init_all_tables();


   Double_t b_len[MAX_CH],
    p_len[MAX_CH],
    w_len[MAX_CH]; //length of window for bkg, peak and width integration in DSP
  Double_t b_mean[MAX_CH],
    p_mean[MAX_CH],
    w_mean[MAX_CH]; //length of window for bkg, peak and width integration in DSP

    
public:
  bool first_call=false;
  int crs_module=0;

  std::vector<UChar_t> buffer_storage;
  BufClass Buf_ring; //указатель на буфер, куда пишутся данные
  // начало Buf_ring.b1 сдвинуто вправо на o_size
  BufClass BufProc; //тот же Buf_ring, но используется для process (декод)
  std::mutex buf_mutex;
  UInt_t bufnum=0; // номер буфера (=идентификатор)

  // Управление безопасной записью
  std::atomic<UChar_t*> write_ptr{nullptr};        // текущая позиция записи
  std::deque<std::atomic<UChar_t*>> worker_ptrs;  // позиции каждого worker'а

  // Поток копирования и связанные переменные
  std::unique_ptr<std::thread> copy_thread;
  std::atomic<bool> copy_running{false};
  std::list<BufClass> copy_queue;
  std::mutex copy_mutex;
  std::condition_variable copy_cond;

  // Потоки анализа и связанные переменные
  UInt_t num_process_threads{4};  // количество потоков анализа
  std::vector<std::unique_ptr<std::thread>> process_threads;
  std::atomic<bool> process_running{false};
  std::list<BufClass> process_queue;
  std::mutex process_mutex;
  std::condition_variable process_cond;

  std::list<LocalBuf> Bufevents; //list of buffers for decoding

public:
  Decoder();
  ~Decoder();
  //void Decode_Resize(Long64_t r_size, Long64_t o_size);
  void Decode_Start(Long64_t r_size, Long64_t o_size, bool b_acq, int module);
  void Copy_Start(bool b_acq);
  void Process_Start(int num_threads);
  void Decode_Stop() noexcept;

  void Copy_Worker();  // блок копирования
  void Send_for_Process(UChar_t* p1, UChar_t* p2, union82 &p3);
  void Process_Worker(UInt_t thread_id); // рабочий поток анализа

  // Метод для добавления данных в очередь извне
  void Add_to_copy_queue(UChar_t* data, size_t size);
  UChar_t* get_safe_write_limit();
  inline bool IsEventStart_1(union82 &u82);
  inline bool IsEventStart_3(union82 &u82);
  inline bool IsEventStart_22(union82 &u82);
  inline bool IsEventStart_36(union82 &u82);
  inline bool IsEventStart_79(union82 &u82);
  bool IsEventStart(union82 &u82);
  UChar_t* FindEvent_backward_1(union82 From, UChar_t* To);
  UChar_t* FindEvent_backward_3(union82 From, UChar_t* To);
  UChar_t* FindEvent_backward_22(union82 From, UChar_t* To);
  UChar_t* FindEvent_backward_36(union82 From, UChar_t* To);
  UChar_t* FindEvent_backward_79(union82 From, UChar_t* To);
  UChar_t* FindEvent_backward(union82 From, UChar_t* To);
  void Dec_Init(eventlist* &Blist, BufClass& Buf);
  //void Dec_End(eventlist &Blist, BufClass& Buf);
  void Decode_switch(BufClass& Buf);
  void Decode22(BufClass& Buf);
  void Decode36(BufClass& Buf);
  void Event_Insert_Pulse(eventlist &Elist, PulseClass &pls);
  void PulseAna(PulseClass &ipls);
  void MakePk(PkClass &pk, PulseClass &ipls);
};
