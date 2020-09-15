#include <iostream>
#include <zlib.h>
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"

using namespace std;

const int MAX_CH=64;
//const double Period=10; //for conversion between samples and ns

TRandom rnd;

class Peak {
public:
  Double_t Area;
  Double_t Time;
  Double_t Width;
  Int_t Chan;

  Peak() {};
};

class Event {
public:
  vector<Peak> peaks;
  Long64_t Tstmp;
  Bool_t State;
  
  Event() {
    Tstmp=-1;
  };
};

class Decoder_class {
public:
  static const Long64_t sixbytes=0xFFFFFFFFFFFF;
  ULong64_t word;
  UChar_t* w8;
  Event ev;
  gzFile ff;
  UShort_t sz;
  UShort_t mod;
  char* buf;

  Decoder_class() {w8 = (UChar_t*) &word;};

  void Decode(const char* fname);
  void Decode78();
  void Decode79();
};

//Long64_t Nevt=0;
//RootClass rt;

void begin_of_file();
void end_of_file();
void Process_event(Event* ev);

//-----------------------------------
void begin_of_file() {
}

//-----------------------------------
void end_of_file() {
  //rt.SaveHist("out.root");
}

//-----------------------------------
void Process_event(Event* ev) {
  //cout << Nevt << " " << ev->Tstmp << endl;
  //rt.FillHist(ev);
  //++Nevt;
}
//***************************************************************
void Decoder_class::Decode78() {
  int res=1;
  while (res) {
    res=gzread(ff,&word,8);

    UChar_t frmt = w8[7] & 0x80; //event start bit
    if (frmt) { //event start
      if (ev.Tstmp>=0) { //if old event exists, analyze it
	Process_event(&ev);
      }
      ev.Tstmp = word & sixbytes; //in samples, use *Period for ns
      ev.State = Bool_t(word & 0x1000000000000);
      ev.peaks.clear();
    }
    else {
      ev.peaks.push_back(Peak());
      Peak *pk = &ev.peaks.back();

      Short_t* buf2 = (Short_t*) &word;
      pk->Area = (buf2[0]+rnd.Rndm()-0.5);
      pk->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples, use *Period for ns
      pk->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;
      pk->Chan = buf2[3];
    }
  }

}

//***************************************************************
void Decoder_class::Decode79() {
  int res=1;
  while (res) {
    res=gzread(ff,&word,8);

    UChar_t frmt = w8[7] & 0x80; //event start bit
    if (frmt) { //event start
      if (ev.Tstmp>=0) { //if old event exists, analyze it
	Process_event(&ev);
      }
      ev.Tstmp = word & sixbytes; //in samples, use *Period for ns
      ev.State = Bool_t(word & 0x1000000000000);
      ev.peaks.clear();
    }
    else {
      ev.peaks.push_back(Peak());
      Peak *pk = &ev.peaks.back();

      Short_t* buf2 = (Short_t*) &word;
      UShort_t* buf2u = (UShort_t*) &word;
      pk->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
      pk->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples, use *Period for ns
      pk->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;
      pk->Chan = buf2[3];
    }
  }

}

//***************************************************************
void Decoder_class::Decode(const char* fname) {
  // const Long64_t sixbytes=0xFFFFFFFFFFFF;
  // ULong64_t word;
  // UChar_t* w8 = (UChar_t*) &word;
  // Event ev;
  UShort_t fmt, mod;
  Int_t sz;

  ff = gzopen(fname,"rb");
  if (!ff) {
    cout << "Can't open file: " << fname << endl;
    return;
  }

  gzread(ff,&fmt,sizeof(Short_t));
  if (fmt>128) {
    gzread(ff,&mod,sizeof(Short_t));
    gzread(ff,&sz,sizeof(Int_t));
  }
  else {
    mod=fmt;
    gzread(ff,&sz,sizeof(UShort_t));
  }

  buf = new char[sz];
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

  return;
  begin_of_file();

  switch (mod) {
  case 79:
    Decode79();
    break;
  case 78:
    Decode78();
    break;
  default:
    cout << "Unknown mod: " << mod << endl;
    gzclose(ff);
    return;
  }
  
  if (ev.Tstmp>=0) { //analyze last event
    Process_event(&ev);
  }

  gzclose(ff);
  end_of_file();

}
//***************************************************************
void decoder(const char* fname) {
  Decoder_class* dec = new Decoder_class();
  dec->Decode(fname);
}
//***************************************************************
