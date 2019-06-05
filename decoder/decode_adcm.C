#include "decoder.C"

const double Period=10;

class RootClass {
public:
  Long64_t Nprint;
  Long64_t Nevt;
  Long64_t Tst0;

  //histograms
  TH1F* h_mult;
  TH1F* h_energy[MAX_CH+1];
  TH1F* h_time[MAX_CH+1];
  TH1F* h_tof[MAX_CH+1];

public:
  RootClass();
  virtual ~RootClass() {};

  void MakeHist(int nprint);
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
  //++Nevt;
}

//-----------------------------------
RootClass::RootClass() {
  Nevt=0;
  Tst0=-1;
}

//-----------------------------------
void RootClass::MakeHist(int nprint) {
  char ss[100];
  Nprint=nprint;

  gROOT->cd();
  TIter next(gDirectory->GetList());

  TObject* obj;
  while ( (obj = (TObject*)next()) ) {
    if (obj->InheritsFrom(TH1::Class())) {
      delete obj;
    }
  }

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
  const int x1=28,x2=31,y1=20,y2=27;

  double T0=999999;
  int nx=-1,ny=-1,npin=-1;
  double dt;

  if (Tst0<0) {
    Tst0=ev->Tstmp;
  }

  for (UInt_t i=0;i<ev->peaks.size();i++) {
    int ch = ev->peaks[i].Chan;
    if (ch>=x1 && ch<=x2) {
      if (ev->peaks[i].Time<T0) {
	T0=ev->peaks[i].Time;
	nx=ch-x1;
      }
    }
  }

  h_mult->Fill(ev->peaks.size());
  for (UInt_t i=0;i<ev->peaks.size();i++) {
    int ch = ev->peaks[i].Chan;
    if (ch>=y1 && ch<=y2) {
      dt=ev->peaks[i].Time-T0;
      ny=ch-y1;
    }


    if (ch<0 || ch>=MAX_CH) {
      cout << "ch out of range: " << ch << " " << ev->Tstmp << endl;
    }
    else {
      h_energy[ch]->Fill(ev->peaks[i].Area);
      h_time[ch]->Fill(ev->Tstmp*1e-8);
      h_tof[ch]->Fill(ev->peaks[i].Time);
    }
  }
  ++Nevt;
  if (Nprint && Nevt%Nprint==0) {
    double tt = (ev->Tstmp-Tst0)*1e-8;
    cout << "Nevt: " << Nevt << " Time: " << tt << endl;
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

void decode_adcm(const char* fname, int nprint=0) {
  rt.MakeHist(nprint);
  decoder(fname);
}
