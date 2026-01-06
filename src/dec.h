#pragma once

#include "pulseclass.h"

#include <TROOT.h>
#include <list>

union union82 { // текущее положение в Dec буфере
  Long64_t *l = 0;
  ULong64_t *ul;
  UChar_t *b; // input: указатель на конец "полных" событий в буфере
              // или на конец буфера (для записи)
              // output(dec|raw): текущий указатель на конец буфера
  UShort_t *us;
  Short_t *s;
  UInt_t *ui;
  Int_t *i;
  Float_t *f;
};

class BufClass {
public:
  // std::atomic<UChar_t*> b1{nullptr};  // ← атомарный указатель на начало
  // буфера std::atomic<UChar_t*> b3{nullptr};  // ← атомарный указатель на
  // конец буфера
  UChar_t *write_start = 0; // указатель на начало буфера для записи
  UChar_t *write_end = 0; // указатель на (физический) конец буфера для записи
  UChar_t *analysis_start = 0; // - конец предыдущего = начало нового анализа
  UChar_t *analysis_end = 0; // - конец нового анализа

  union82 u82; // текущий указатель для записи
  // size_t Size=0;
  UInt_t buffer_id; // ID этого буфера данных: !!!вобще-то, он нигде не
                    // используется и не нужен

  // std::vector<UChar_t> buffer_storage;
  //  input: всегда должно быть: b1 <= b < b3

  UChar_t flag = 0;
  // 0 - empty, can be filled;
  // 1 - filled, can be analyzed;
  // 2 - analyzed, can be deleted.
  // 9 - output: буфер готов, можно писать

public:
  BufClass() {};
  BufClass(UChar_t *w1, UChar_t *w3, UChar_t *a1)
      : write_start(w1), write_end(w3), analysis_start(a1){};
  //   ~BufClass();
  void Ring_Write(BufClass &buf);
};

typedef std::list<BufClass>::iterator buf_iter;

bool Dec79(BufClass &Buf, EventClass &evt);
