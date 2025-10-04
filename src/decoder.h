#pragma once
#include "romana.h"

class Decoder {
public:
  struct CopyData {
    UChar_t* data;  // указатель на данные USB
    size_t size;    // размер данных
  };

  std::vector<UChar_t> buffer_storage;
  BufClass Buf_ring; //указатель на буфер, куда пишутся данные
  // начало Buf_ring.b1 сдвинуто вправо на o_size

  // Поток копирования и связанные переменные
  std::unique_ptr<std::thread> copy_thread;
  std::atomic<bool> copy_running{false};
  std::vector<CopyData> copy_queue;
  std::mutex queue_mutex;
  std::condition_variable queue_cond;

public:
  ~Decoder();
  void Decode_Start(Long64_t r_size, Long64_t o_size);
  void Decode_Stop();

  // Метод для добавления данных в очередь извне
  void Add_to_copy_queue(UChar_t* data, size_t size);
  UChar_t* FindEvent(UChar_t* begin, UChar_t* end);

};
