#include <iostream>
#include <zlib.h>
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"

using namespace std;

const int MAX_CH=64;
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

Long64_t Nevt=0;
//RootClass rt;

void begin_of_file();
void end_of_file();
void Process_event(Event* ev);
void Decode79();

//***************************************************************
void decoder(const char* fname) {
  const Long64_t sixbytes=0xFFFFFFFFFFFF;
  ULong64_t word;
  UChar_t* w8 = (UChar_t*) &word;
  Event ev;

  gzFile ff = gzopen(fname,"rb");
  if (!ff) {
    cout << "Can't open file: " << fname << endl;
    return;
  }

  UShort_t sz;
  UShort_t mod;
  gzread(ff,&mod,sizeof(mod));
  gzread(ff,&sz,sizeof(sz));

  char* buf = new char[sz];
  gzread(ff,buf,sz);

  if (mod!=79) {
    cout << "mod= " << mod << endl;
    gzclose(ff);
    return;
  }

  begin_of_file();

  int res=1;
  while (res) {
    res=gzread(ff,&word,8);

    UChar_t frmt = w8[7] & 0x80; //event start bit
    if (frmt) { //event start
      if (ev.Tstmp>=0) { //if old event exists, analyze it
	Process_event(&ev);
      }
      ev.Tstmp = word & sixbytes;
      ev.State = Bool_t(word & 0x1000000000000);
      ev.peaks.clear();
    }
    else {
      ev.peaks.push_back(Peak());
      Peak *pk = &ev.peaks.back();

      Short_t* buf2 = (Short_t*) &word;
      UShort_t* buf2u = (UShort_t*) &word;
      pk->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
      pk->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01;
      pk->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;
      pk->Chan = buf2[3];
    }
  }

  if (ev.Tstmp>=0) { //analyze last event
    Process_event(&ev);
  }

  gzclose(ff);
  end_of_file();

}
