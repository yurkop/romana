#ifndef rootclass_H
#define rootclass_H 1

#include "TH1.h"
#include "decoder.h"

class rootclass {
public:
  TH1F *h_count;

public:
  rootclass();
  virtual ~rootclass(){};

  void fillhist(pulse_vect *pulse);
  void saveroot(char *name);
  // void bookhist();
};

#endif
