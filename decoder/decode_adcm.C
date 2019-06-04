#include "decoder.C"

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


RootClass rt;

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

void decode_adcm(const char* fname) {
  decoder(fname);
}

