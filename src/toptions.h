#ifndef toptions_H
#define toptions_H 1

#include "common.h"

#include <TObject.h>
#include <TROOT.h>
#include <TCutG.h>
//#include <TDatime.h>
#include <TTimeStamp.h>

class Coptions: public TObject {
 public:
  Coptions();
  virtual ~Coptions() {};

  //Version_t ver;
  // parameters of the crs32 or crs2 module

  Int_t chtype[MAX_CH+ADDCH]; //channel type
  Int_t smooth[MAX_CH+ADDCH]; //Hardware Smooth - SS=0..10; S=2^SS
  Int_t deadTime[MAX_CH+ADDCH]; // B = 1..16383
  Int_t preWr[MAX_CH+ADDCH]; // pre-length M = 0..4094
  Int_t durWr[MAX_CH+ADDCH]; // total length N = 1…32763 (internally - multiple of 4)
  Int_t kderiv[MAX_CH+ADDCH]; // K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  Int_t threshold[MAX_CH+ADDCH]; // T = –2048 .. +2047
  Int_t adcGain[MAX_CH+ADDCH]; // G = 0..12
  Bool_t acdc[MAX_CH+ADDCH]; // AC-1; DC-0
  Bool_t inv[MAX_CH+ADDCH]; //0 - no inversion; 1 - inversion (individual)
  // 0 - only triggered channel is written; 
  // 1 - both channels are written with any trigger
  Bool_t enabl[MAX_CH+ADDCH]; //1 - enabled; 0 - disabled
  Bool_t forcewr; //only for crs2
  //ChDef chtype[MAX_CH+ADDCH]; //channel type
  //----------------------------------------------

 public:
  void InitPar(Int_t module);
  void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);

  ClassDef(Coptions, 2)
};

//------------------------------------

class Toptions: public TObject {
 public:
  Toptions();
  virtual ~Toptions() {};

  /*
  //----------------------------------------------
  // parameters of the crs32 or crs2 module

  Int_t smooth[MAX_CH+ADDCH]; //Hardware Smooth - SS=0..10; S=2^SS
  Int_t deadTime[MAX_CH+ADDCH]; // B = 1..16383
  Int_t preWr[MAX_CH+ADDCH]; // pre-length M = 0..4094
  Int_t durWr[MAX_CH+ADDCH]; // total length N = 1…32763 (internally - multiple of 4)
  Int_t kderiv[MAX_CH+ADDCH]; // K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  Int_t threshold[MAX_CH+ADDCH]; // T = –2048 .. +2047
  Int_t adcGain[MAX_CH+ADDCH]; // G = 0..12
  Bool_t acdc[MAX_CH+ADDCH]; // AC-1; DC-0
  Bool_t inv[MAX_CH+ADDCH]; //0 - no inversion; 1 - inversion (individual)
  Bool_t forcewr; //only for crs2
  // 0 - only triggered channel is written; 
  // 1 - both channels are written with any trigger
  Bool_t enabl[MAX_CH+ADDCH]; //1 - enabled; 0 - disabled

  ChDef chtype[MAX_CH+ADDCH]; //channel type

  //----------------------------------------------
  */
  
  Bool_t Start[MAX_CH+ADDCH]; //
  //UInt_t ch_flag[MAX_CH+ADDCH];
  Int_t nsmoo[MAX_CH+ADDCH]; //software smoothing 0..100
  Int_t bkg1[MAX_CH+ADDCH];
  Int_t bkg2[MAX_CH+ADDCH];
  Int_t peak1[MAX_CH+ADDCH];
  Int_t peak2[MAX_CH+ADDCH];
  Int_t pile[MAX_CH+ADDCH];
  //Int_t pile2[MAX_CH+ADDCH];
  Int_t timing[MAX_CH+ADDCH];
  Int_t twin1[MAX_CH+ADDCH];
  Int_t twin2[MAX_CH+ADDCH];

  Float_t emult[MAX_CH+ADDCH];
  Float_t elim1[MAX_CH+ADDCH];
  Float_t elim2[MAX_CH+ADDCH];


  Int_t channels[MAX_CH+1]; //+all
  Int_t color[MAX_CH+1]; //+all
  Int_t lcolor[MAX_L];
  char chname[MAX_CH+1][16]; //+all

  //----------------------------------------------
  // Important common parameters

  //TDatime F_start,F_stop; //start and stop of the acquisition (?)
  TTimeStamp F_start; //start and of the acquisition / start event in a file
  Float_t T_acq; //duration of the acquisition / file (in seconds)

  Int_t Tstart,Tstop;
  Int_t tsleep;
  Int_t usb_size; //in kB
  Int_t rbuf_size; //in kB

  Int_t event_buf; //length of event buffer
  //analysis starts only after filling first event_buf
  Int_t event_lag; //maximal lag of event analysis in mks
  // if current pulse has tstamp smaller than last event's T
  // by this value, the pulse is inserted into event_list
  // without  (if zero - calculate automatically)

  //Bool_t chinv[MAX_CH+1];

  Int_t sel_hdiv; //number of divisions in histframe
  Int_t icheck; //first histogram to plot among checked
  
  Bool_t raw_write;
  Bool_t dec_write;
  Int_t raw_compr; //raw data compr level
  Int_t dec_compr; //decoded data compr level

  Bool_t decode;
  char fname_raw[255];
  char fname_dec[255];

  Int_t ev_min; //minimal length of events list
  Int_t ev_max; //maximal length of events list

  Int_t tgate; // coincidence window for events (in samples)
  //Int_t tgate2; // coincidence window for histograms (in ...)

  UInt_t mult1; // minimal multiplicity
  UInt_t mult2; // maximal multiplicity

  Int_t seltab;

  Float_t time_min,time_max;
  Float_t tof_min,tof_max;
  Float_t mtof_min,mtof_max;
  Float_t amp_min,amp_max;
  Float_t hei_min,hei_max;

  Float_t time_bins;
  Float_t tof_bins;
  Float_t mtof_bins;
  Float_t amp_bins;
  Float_t hei_bins;

  /*
  Bool_t time_chk[MAX_CH];
  Bool_t tof_chk[MAX_CH];
  Bool_t mtof_chk[MAX_CH];
  Bool_t amp_chk[MAX_CH];
  Bool_t hei_chk[MAX_CH];
  */



  Int_t starts_thr1,starts_thr2;
  Float_t beam1,beam2;
  //Float_t wgam1,wgam2;
  //Float_t wneu1,wneu2;
  //Float_t wtail1,wtail2;

  //Int_t nsmoo;
  Int_t start_ch;
  Int_t psd_ch;
  //Int_t mon_ch;
  Int_t num_events;
  Int_t num_buf; //number of buffers to analyze in DoNbuf

  Float_t tplus;

  Int_t b_osc,b_leg,b_logy,b_time;
  Int_t draw_opt;

  TCutG *gcut[3];

  Float_t T0,LL;

  //double vmon[6];


  Float_t Ecalibr[2];

  Int_t Nevt;
  Float_t Tof;

  Int_t LongStamp;

  Bool_t b_deriv[3];
  Bool_t b_peak[16];

  Int_t rBSIZE; //size of the read buffer
  Int_t EBufsize; //size of the event buffer
  Int_t period; //period of digitizing in ns

 public:
  //void InitPar(Int_t module);

  //void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);


  ClassDef(Toptions, 94)
};

ClassImp(Toptions)

#endif
