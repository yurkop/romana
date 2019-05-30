#include <iostream>
#include <zlib.h>
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"

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

class RootClass {
public:
  TH1F* h_mult;
  TH1F* h_energy[MAX_CH+1];
  TH1F* h_time[MAX_CH+1];
  TH1F* h_tof[MAX_CH+1];

public:
  RootClass();
  virtual ~RootClass() {};

  void FillHist(Event* ev);
  void SaveHist(const char *name);

};


Long64_t Nevt=0;
RootClass rt;

using namespace std;
void begin_of_file();
void end_of_file();
void Process_event(Event* ev);

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

//-----------------------------------
void begin_of_file() {
}

//-----------------------------------
void end_of_file() {
  rt.SaveHist("out.root");
}

//-----------------------------------
void Process_event(Event* ev) {
  //cout << Nevt << " " << ev->Tstmp << endl;
  rt.FillHist(ev);
  ++Nevt;
}
//-----------------------------------
RootClass::RootClass() {
  char ss[100];

  h_mult = new TH1F("h_mult","h_mult",64,0,64);

  for (int i=0;i<=MAX_CH;i++) {
    sprintf(ss,"h_energy_%02d",i);
    h_energy[i] = new TH1F(ss,ss,10000,0,10000);
  }

  for (int i=0;i<=MAX_CH;i++) {
    sprintf(ss,"h_time_%02d",i);
    h_time[i] = new TH1F(ss,ss,4000,0,1000);
  }

  for (int i=0;i<=MAX_CH;i++) {
    sprintf(ss,"h_tof_%02d",i);
    h_tof[i] = new TH1F(ss,ss,4000,-200,200);
  }

}
//-----------------------------------
void RootClass::FillHist(Event* ev) {
  h_mult->Fill(ev->peaks.size());
  for (UInt_t i=0;i<ev->peaks.size();i++) {
    int ch = ev->peaks[i].Chan;
    if (ch<0 || ch>=MAX_CH) {
      cout << "ch out of range: " << ch << " " << ev->Tstmp << endl;
    }
    else {
      h_energy[ch]->Fill(ev->peaks[i].Area);
      h_time[ch]->Fill(ev->Tstmp*1e-8);
      h_tof[ch]->Fill(ev->peaks[i].Time);
    }
  }
}
//-----------------------------------
void RootClass::SaveHist(const char *name) {

  TFile * tf = new TFile(name,"RECREATE");

  gROOT->cd();

  TIter next(gDirectory->GetList());

  tf->cd();

  TH1 *h;
  TObject* obj;
  while ( (obj = (TObject*)next()) ) {
    if (obj->InheritsFrom(TH1::Class())) {
      h=(TH1*) obj;
      //h->Print();
      if (h->GetEntries() > 0) {
	h->Write();
      }
    }
  }

  cout << "Histograms are saved in file: " << name << endl;

  tf->Close();
}
