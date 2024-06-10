#ifndef decoder_H
#define decoder_H 1

#include "pulseclass.h"
#include <zlib.h>
#include <deque>
//#include <boost/circular_buffer.hpp>

using namespace std;

struct DecRecord {
  char* buf; // указатель на буфер с данными
  UInt_t len; // длина буфера в байтах
  int status; // статус буфера (пустой, заполненный, проанализированный?)
};

//typedef std::pair<unsigned char*,int> DecPair;

class DecoderClass {
 public:
  std::deque<DecRecord> declist;
  gzFile *zfile;
  //string zfname;
  //char dec_opt[10];

 public:
  DecoderClass();
  virtual ~DecoderClass();

  //void Reset_Dec();
  
  void Decode79(UInt_t iread, UInt_t ibuf);
};


#endif
