#pragma once

#include "libcrs.h"
#include "TMutex.h"
#include "TThread.h"

class Encoder {
public:
  BufClass DecBuf_ring2; // = DecBuf_ring + OFF_SIZE
  BufClass DecBuf_ring; //указатель на буфер, куда пишутся декодированные данные
  // Реально создается DecBuf_ring2, КОНЕЦ которого на OFF_SIZE
  // сдвинут ВПРАВО от конца DecBuf_ring

  buf_iter decbuf_it;

  TMutex decw_mut;
  TMutex cmut;

  std::list<BufClass> dec_list;
  char dec_opt[5];
  gzFile f_dec;
  string decname;
  Long64_t decbytes;
  int wrt_thread_run=0;
  TThread* trd_dec_write=0;



  struct ThreadArgs {
    Encoder* instance;
    void* user_arg;
    ThreadArgs(Encoder* inst, void* arg) : instance(inst), user_arg(arg) {}
  };

public:
  Encoder();
  virtual ~Encoder();
  void Encode_Start(int rst);
  void Encode_Stop(int end_ana);
  void Reset_Dec(Short_t mod);
  void Fill_Dec(event_iter evt);
  void Fill_Dec79(event_iter evt);
  void Flush_Dec3(int end_ana);
  int Wr_Dec(UChar_t* buf, int len);
  void Handle_dec_write(void *ctx);
  static void* StaticWrapper(void* arg);
};
