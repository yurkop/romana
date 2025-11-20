// однопоточный декодер
// в этих 3 строчках нужно указать путь к папке romana/src
#include "../src/dec.h"
#include "../src/pulseclass.h"
#include "../src/dec.cpp"

#include <iostream>
#include <zlib.h>
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"
#include <bitset>

class Decoder_class {
public:
  gzFile ff;
  BufClass Buf;

  Decoder_class(const char* fname);
  void Decode();
};

Decoder_class::Decoder_class(const char* fname) {
  UShort_t fmt, mod;
  Int_t sz;

  ff = gzopen(fname,"rb");
  if (!ff) {
    cout << "Can't open file: " << fname << endl;
    return;
  }

  gzread(ff,&fmt,sizeof(Short_t));
  if (fmt==129) {
    gzread(ff,&mod,sizeof(Short_t));
    gzread(ff,&sz,sizeof(Int_t));
  }
  else {
    mod=fmt;
    fmt=129;
    gzread(ff,&sz,sizeof(UShort_t));
  }

  if (fmt!=129 || mod>100 || sz>5e5){//возможно, это текстовый файл
    //или старый файл без параметров
    cout << "Header not found: " << fname << " " << fmt
	 << " " << mod << " " << sz << endl;
    exit(-1);
  }

  cout << "Header: " << fmt << " " << mod << " " << sz << endl;

  char* buf = new char[sz];
  gzread(ff,buf,sz);


  //F_start test
  Long64_t fstart=0;
  time_t tt=0;
  std::string str(buf,sz);
  size_t pos=str.find("F_start");
  if (pos!=std::string::npos) {
    char* buf2 = buf+pos;
    buf2 += strlen(buf2)+1+sizeof(short);
    fstart = *(Long64_t*) buf2;

    char txt[100];
    tt = (fstart+788907600000)*0.001;
    struct tm *ptm = localtime(&tt);
    strftime(txt,sizeof(txt),"%F %T",ptm);

    cout << "F_start: " << " " << txt << " " << fstart << " " << tt << endl;
  }
  else {
    cout << "F_start not found" << endl;
  }

  delete[] buf;

}

//***************************************************************
void Decoder_class::Decode() {

  std::list<EventClass> Levents;
  //EventClass evt;

  const int LEN=4*1024*1024; //64MB

  Buf.buffer_storage.resize(LEN);
  Buf.b1 = Buf.buffer_storage.data();

  int nbytes;
  cout << "Decode: " << endl;
  do {
    Buf.u82.b = Buf.b1;
    nbytes = gzread(ff,Buf.b1,LEN);
    //cout << "nbytes1: " << nbytes << endl;
    Buf.b3 = Buf.b1 + nbytes;
    while (Buf.u82.b+7 < Buf.b3) {
      auto evt = Levents.insert(Levents.end(),EventClass());
      //evt = EventClass();
      if (!Dec79(Buf,*evt)) break;  // Выходим если данных не хватает
    }
    cout << "nbytes2: " << nbytes << endl;
  } while (nbytes);
  int nn=0;
  for (auto evt = Levents.begin(); evt!=Levents.end(); ++evt) {
    cout << "evt: " << nn << " " << evt->Tstmp << endl;
    ++nn;
  }
  //delete[] buf;
}
//***************************************************************
void decoder(const char* fname="") {
  if (strlen(fname)==0) {
    cout << "usage: .x decoder.C(\"filename\")" << endl;
    return;
  }
  Decoder_class dec(fname);
  dec.Decode();
}
//***************************************************************
