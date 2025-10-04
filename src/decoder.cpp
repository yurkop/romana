#include "decoder.h"
#include <iostream>
#define ID_ADCM  0x2A50
#define ID_CMAP  0x504D
#define ID_EVNT  0x5645
#define ID_CNTR  0x5443

extern CRS* crs;

Decoder::~Decoder() {
  Decode_Stop();
}

void Decoder::Decode_Start(Long64_t r_size, Long64_t o_size) {
  // r_size - total size of buffer (на самом деле + o_size)
  // o_size - offset size

  // Новый код с безопасным управлением памятью:
  buffer_storage.resize(o_size + r_size);

  Buf_ring.b1 = buffer_storage.data() + o_size;
  Buf_ring.b3 = Buf_ring.b1 + r_size;
  Buf_ring.b = Buf_ring.b1;

  /*
    buf_list.clear();
    buf_it = buf_list.insert(buf_list.end(),BufClass());
    buf_it->b1 = Buf_ring.b1;
    buf_it->b = buf_it->b1;
    buf_it->b3 = buf_it->b1+buf_size;
    //buf_it->flag=??;  // считаем, что сделаны Findlast + анализ
    */


  if (crs->b_acq) {// стартуем поток копирования - только если acquisition
    copy_running = true;
    copy_thread = std::make_unique<std::thread>([this]() {
      // Лямбда-функция вместо метода класса
      while (true) {
	std::vector<CopyData> local_queue;
                
	{
	  std::unique_lock<std::mutex> lock(queue_mutex);
	  queue_cond.wait(lock, [this]{ 
	    return !copy_queue.empty() || !copy_running;
	  });
      
	  // Выходим только когда остановка И очередь пуста
	  if (!copy_running && copy_queue.empty()) {
	    break;
	  }
                    
	  if (!copy_queue.empty()) {
	    local_queue.swap(copy_queue);
	  }
	}
                
	for (auto& item : local_queue) {
	  Buf_ring.Ring_Write(item.data, item.size);
	}
      }
    });
  }
}

void Decoder::Decode_Stop() {
  if (!copy_running) return;

  copy_running = false;
  queue_cond.notify_all();

  // Ждем пока поток обработает ВСЕ данные из очереди
  if (copy_thread && copy_thread->joinable()) {
    copy_thread->join(); // Поток сам выйдет когда queue пуста И copy_running=false
  }
  copy_thread.reset(); // освобождаем память

  // Очистка очереди
  //std::lock_guard<std::mutex> lock(queue_mutex);
  //copy_queue.clear();
}

// Метод для добавления данных в очередь извне
void Decoder::Add_to_copy_queue(UChar_t* data, size_t size) {
  std::lock_guard<std::mutex> lock(queue_mutex);
  copy_queue.push_back({data, size});
  queue_cond.notify_one(); // Будим поток даже если copy_running=false
}

UChar_t* Decoder::FindEvent(UChar_t* begin, UChar_t* end) {
  // возвращает начало первого найденного события в буфере от begin до end
  // если не найдено, возврвщает 0

  if ((end-begin)%8) {
    prnt("sss;",BRED,"Error: end-begin is not a multiple of 8",RST);
    return 0;
  }

  int step=1;
  if (begin>end) step=-1;

  UChar_t* res=0;

  UInt_t frmt;
  union82 uu; // текущее положение в буфере
  uu.b = begin;
  
  switch (crs->module) {
  case 1: //adcm raw
    /*
      доделать, проверить!
      while (uu.b > buf_it->b1) {
      --uu.ui;
      if (*uu.ui==0x2a500100) {
      //outbuf.Buf=uu.b;
      }
      }
    */
    //prnt("ss ls;",BRED,"Error: adcm raw no last event:",buf_it->bufnum,RST);
    //break;
    return 0;
  case 3: //adcm dec
    while (uu.b != end) {
      uu.us+=step;
      if (*uu.us==ID_EVNT) {
	res = uu.b;
	break;
      }
    }
    //prnt("ss ls",BRED,"Error: no last event:",buf_it->bufnum,RST);
    break;
  case 22:
    while (uu.b != end) {
      uu.us+=step;
      //find frmt==0 -> this is the start of a pulse
      frmt = *(uu.b+1) & 0x70;
      if (frmt==0) {
	res = uu.b;
	break;
      }
    }
    break;
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
    while (uu.b != end) {
      uu.ul+=step;
      //find frmt==0 -> this is the start of a pulse
      frmt = *(uu.b+6) & 0xF0;
      if (frmt==0) {
	res = uu.b;
	break;
      }
    }
    // если дошли до начала и не нашли, b остается равным 0
    break;
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
    while (uu.b != end) {
      uu.ul+=step;
      //find frmt==1 -> this is the start of a pulse
      frmt = *(uu.b+7) & 0x80; //event start bit
      if (frmt) {
        res = uu.b;
	break;
      }
    }
    // если дошли до начала и не нашли, b остается равным 0
    break;

  default:
    cout << "Wrong module: " << crs->module << endl;
  }

  //prnt("ss ls;",BRED,"Step1. FindLast3:",buf_it->bufnum,RST);
  return res;

} //FindEvent
