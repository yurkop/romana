#pragma once

#include "pulseclass.h"

#include <list>
#include <TROOT.h>

union union82 { // текущее положение в Dec буфере
  Long64_t* l=0;
  ULong64_t* ul;
  UChar_t* b; // input: указатель на конец "полных" событий в буфере
              // или на конец буфера (для записи)
              // output(dec|raw): текущий указатель на конец буфера
  UShort_t* us;
  Short_t* s;
  UInt_t* ui;
  Int_t* i;
  Float_t* f;
};

class BufClass {
public:
  //std::atomic<UChar_t*> b1{nullptr};  // ← атомарный указатель на начало буфера
  //std::atomic<UChar_t*> b3{nullptr};  // ← атомарный указатель на конец буфера
  UChar_t* b1=0; // указатель на начало буфера
  UChar_t* b3=0; // указатель на физический конец буфера

  union82 u82; //текущий указатель
  //size_t Size=0;
  UInt_t buffer_id;  // ID этого буфера данных
  //std::vector<UChar_t> buffer_storage;
  // input: всегда должно быть: b1 <= b < b3

  UChar_t flag=0;
  //0 - empty, can be filled;
  //1 - filled, can be analyzed;
  //2 - analyzed, can be deleted.
  //9 - output: буфер готов, можно писать

  // public:
  //   BufClass(size_t sz);
  //   ~BufClass();
  void Ring_Write(BufClass &buf);
  //void Ring_Write(const UChar_t* data, size_t data_size);
};

typedef std::list<BufClass>::iterator buf_iter;

bool Dec79(BufClass& Buf, EventClass& evt);
