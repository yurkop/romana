#include "decoder.C"

const double Period=10;

#define MIN_X 3
#define MAX_X 6
#define nPIN_X 4
#define MIN_Y 1
#define MAX_Y 8
#define nPIN_Y 8
#define XPIN_MIN 28
#define XPIN_MAX 31
#define YPIN_MIN 20
#define YPIN_MAX 27
#define MINGAM 0
#define MAXGAM 17

class RootClass {
public:
  Long64_t Nprint;
  Long64_t Nevt;
  Long64_t Tst0;

  //histograms
  TH1F* h_mult;

  TH1F* h_mult_ch[MAX_CH+1];
  
  TH1F* h_time[MAX_CH+1];

  TH1F* h_resolution[nPIN_X][nPIN_Y];

  TH2D* h_counts[MAX_CH+1];

  TH1F* h_ampl[MAX_CH+1];
  TH1F* h_t_3sig_ampl[MAX_CH+1];
  TH1F* h_energy[MAX_CH+1];
  TH1F* h_t_3sig_energy[MAX_CH+1];
  TH1F* h_tof_x[MAX_CH+1];
  TH1F* h_e_3sig_tof_x[MAX_CH+1];
  TH1F* h_tof_y[MAX_CH+1];
  TH1F* h_e_3sig_tof_y[MAX_CH+1];

  TH1F* h_ampl_strip[nPIN_X][MAX_CH+1];
  TH1F* h_t_3sig_ampl_strip[nPIN_X][MAX_CH+1];
  TH1F* h_energy_strip[nPIN_X][MAX_CH+1];
  TH1F* h_t_3sig_energy_strip[nPIN_X][MAX_CH+1];
  TH1F* h_tof_x_strip[nPIN_X][MAX_CH+1];
  TH1F* h_e_3sig_tof_x_strip[nPIN_X][MAX_CH+1];
  TH1F* h_tof_y_strip[nPIN_X][MAX_CH+1];
  TH1F* h_e_3sig_tof_y_strip[nPIN_X][MAX_CH+1];

  TH1F* h_ampl_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_t_3sig_ampl_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_energy_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_t_3sig_energy_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_tof_x_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_e_3sig_tof_x_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_tof_y_pix[nPIN_X][nPIN_Y][MAX_CH+1];
  TH1F* h_e_3sig_tof_y_pix[nPIN_X][nPIN_Y][MAX_CH+1];

  //TH1F* h_mult;
  //TH1F* h_energy[MAX_CH+1];
  //TH1F* h_time[MAX_CH+1];
  //TH1F* h_tof[MAX_CH+1];

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

  //delete all histograms
  gROOT->cd();
  TIter next(gDirectory->GetList());
  TObject* obj;
  while ( (obj = (TObject*)next()) ) {
    if (obj->InheritsFrom(TH1::Class())) {
      delete obj;
    }
  }

  /*
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
  */


  h_mult = new TH1F("mult","mult",MAX_CH,0.5,MAX_CH+0.5);
 
  for (int i=0;i<=MAX_CH;i++) {

    sprintf(ss,"mult%02d",i);
    h_mult_ch[i] = new TH1F(ss,ss,MAX_CH,0.5,MAX_CH+0.5);

    sprintf(ss,"time_ch%02d",i);
    h_time[i] = new TH1F(ss,ss,10000,0,100000);

    sprintf(ss,"counts_ch%02d",i);
    h_counts[i] = new TH2D(ss,ss,nPIN_X,MIN_X-0.5,MAX_X+0.5,nPIN_Y,MIN_Y-0.5,MAX_Y+0.5);

    sprintf(ss,"ampl_ch%02d",i);
    h_ampl[i] = new TH1F(ss,ss,10000,0,10000);
    sprintf(ss,"ampl_t_3sig_ch%02d",i);
    h_t_3sig_ampl[i] = new TH1F(ss,ss,10000,0,10000);
    
    sprintf(ss,"energy_ch%02d",i);
    h_energy[i] = new TH1F(ss,ss,10000,0,10000);
    sprintf(ss,"energy_t_3sig_ch%02d",i);
    h_t_3sig_energy[i] = new TH1F(ss,ss,10000,0,10000);

    sprintf(ss,"tof_x_ch%02d",i);
    h_tof_x[i] = new TH1F(ss,ss,8000,-200,200);
    sprintf(ss,"tof_x_e_3sig_ch%02d",i);
    h_e_3sig_tof_x[i] = new TH1F(ss,ss,8000,-200,200);

    sprintf(ss,"tof_y_ch%02d",i);
    h_tof_y[i] = new TH1F(ss,ss,8000,-200,200);
    sprintf(ss,"tof_y_e_3sig_ch%02d",i);
    h_e_3sig_tof_y[i] = new TH1F(ss,ss,8000,-200,200);
        
    for (int j=0;j<nPIN_X;j++) {

      sprintf(ss,"ampl_strip_x%02d_ch%02d",MIN_X+j,i);
      h_ampl_strip[j][i] = new TH1F(ss,ss,10000,0,10000);
      sprintf(ss,"ampl_t_3sig_strip_x%02d_ch%02d",MIN_X+j,i);
      h_t_3sig_ampl_strip[j][i] = new TH1F(ss,ss,10000,0,10000);
    
      sprintf(ss,"energy_strip_x%02d_ch%02d",MIN_X+j,i);
      h_energy_strip[j][i] = new TH1F(ss,ss,10000,0,10000);
      sprintf(ss,"energy_t_3sig_strip_x%02d_ch%02d",MIN_X+j,i);
      h_t_3sig_energy_strip[j][i] = new TH1F(ss,ss,10000,0,10000);

      sprintf(ss,"tof_x_strip_x%02d_ch%02d",MIN_X+j,i);
      h_tof_x_strip[j][i] = new TH1F(ss,ss,8000,-200,200);
      sprintf(ss,"tof_x_e_3sig_strip_x%02d_ch%02d",MIN_X+j,i);
      h_e_3sig_tof_x_strip[j][i] = new TH1F(ss,ss,8000,-200,200);

      sprintf(ss,"tof_y_strip_x%02d_ch%02d",MIN_X+j,i);
      h_tof_y_strip[j][i] = new TH1F(ss,ss,8000,-200,200);
      sprintf(ss,"tof_y_e_3sig_strip_x%02d_ch%02d",MIN_X+j,i);
      h_e_3sig_tof_y_strip[j][i] = new TH1F(ss,ss,8000,-200,200);

      for (int k=0;k<nPIN_Y;k++) {

	sprintf(ss,"ampl_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_ampl_pix[j][k][i] = new TH1F(ss,ss,10000,0,10000);
	sprintf(ss,"ampl_t_3sig_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_t_3sig_ampl_pix[j][k][i] = new TH1F(ss,ss,10000,0,10000);
    
	sprintf(ss,"energy_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_energy_pix[j][k][i] = new TH1F(ss,ss,10000,0,10000);
	sprintf(ss,"energy_t_3sig_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_t_3sig_energy_pix[j][k][i] = new TH1F(ss,ss,10000,0,10000);

	sprintf(ss,"tof_x_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_tof_x_pix[j][k][i] = new TH1F(ss,ss,8000,-200,200);
	sprintf(ss,"tof_x_e_3sig_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_e_3sig_tof_x_pix[j][k][i] = new TH1F(ss,ss,8000,-200,200);

	sprintf(ss,"tof_y_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_tof_y_pix[j][k][i] = new TH1F(ss,ss,8000,-200,200);
	sprintf(ss,"tof_y_e_3sig_pix_x%02d_y%02d_ch%02d",MIN_X+j,MIN_Y+k,i);
	h_e_3sig_tof_y_pix[j][k][i] = new TH1F(ss,ss,8000,-200,200);
      }
    }
  }

  for (int i=0;i<nPIN_X;i++) {
    for (int j=0;j<nPIN_Y;j++) {
      sprintf(ss,"resolution_x%02d_y%02d",MIN_X+i,MIN_Y+j);
      h_resolution[i][j] = new TH1F(ss,ss,8000,-200,200); 
    }
  }
  
}
//-----------------------------------
void RootClass::FillHist(Event* ev) {

  double T0=999999;
  int nx=-1,ny=-1,npin=-1;
  double dt;

  if (Tst0<0) {
    Tst0=ev->Tstmp;
  }

  for (UInt_t i=0;i<ev->peaks.size();i++) {
    int ch = ev->peaks[i].Chan;
    // if (ch>=XPIN_MIN && ch<=XPIN_MAX) {
    //   if (ev->peaks[i].Time<T0) {
    // 	T0=ev->peaks[i].Time;
    // 	nx=ch-x1;
    //   }
    // }
  }

  //h_mult->Fill(ev->peaks.size());
  for (UInt_t i=0;i<ev->peaks.size();i++) {
    int ch = ev->peaks[i].Chan;
    // if (ch>=y1 && ch<=y2) {
    //   dt=ev->peaks[i].Time-T0;
    //   ny=ch-y1;
    // }

    if (ch<0 || ch>=MAX_CH) {
      cout << "ch out of range: " << ch << " " << ev->Tstmp << endl;
    }
    else {
      //h_energy[ch]->Fill(ev->peaks[i].Area);
      //h_time[ch]->Fill(ev->Tstmp*1e-8);
      //h_tof[ch]->Fill(ev->peaks[i].Time);
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
