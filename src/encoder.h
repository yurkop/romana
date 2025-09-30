#pragma once

#include "libcrs.h"
#include "TMutex.h"
#include "TThread.h"

class Encoder {
public:
  BufClass Buf_ring2; // = Buf_ring + OFF_SIZE
  BufClass Buf_ring; //указатель на буфер, куда пишутся [декодированные] данные
  // Реально создается Buf_ring2, КОНЕЦ которого на OFF_SIZE
  // сдвинут ВПРАВО от конца Buf_ring

  buf_iter buf_it;

  std::list<BufClass> buf_list;
  char wr_opt[5];
  gzFile gzf;
  string wr_name;
  Long64_t wr_bytes;

  bool b_wrt;
  int w_compr=1;
  int w_module=79;

  Long64_t buf_size; // размер одного буфера
  Long64_t wtime_prev,wtime_0,wtime_1; //prev, start, last wdog
  //Long64_t wr_bytes_prev;
  Long64_t size_prev;
  Double_t wrate_mean;


  TMutex wr_mut;
  TMutex cmut;
  TThread* trd_write=0;
  int wrt_thread_run=0;

  struct ThreadArgs {
    Encoder* instance;
    void* user_arg;
    ThreadArgs(Encoder* inst, void* arg) : instance(inst), user_arg(arg) {}
  };

public:
  Encoder();
  virtual ~Encoder();
  void Encode_Start(int rst, int mm, bool bb, int cc,
		    Long64_t r_size, Long64_t b_size, Long64_t o_size);
  void Encode_Stop(int end_ana, bool opt_wrt);
  void Reset_Wrt();
  void Flush3(int end_ana);
  int Write3(UChar_t* buf, int len);
  void Handle_write(void *ctx);
  static void* StaticWrapper(void* arg);
};

//------------------------------
class EDec: public Encoder {

public:
  EDec();
  void Fill_Dec(event_iter evt);
  void Fill_Dec79(event_iter evt);
};
