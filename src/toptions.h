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

  //Int_t chtype[MAX_CH+ADDCH]; //channel type
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

  Int_t chtype[MAX_CH+ADDCH]; //channel type
  Bool_t Start[MAX_CH+ADDCH]; //
  Bool_t Mt[MAX_CH+ADDCH]; // flag to use channel for mtof
  //UInt_t ch_flag[MAX_CH+ADDCH];
  Int_t nsmoo[MAX_CH+ADDCH]; //software smoothing 0..100
  Int_t kdrv[MAX_CH+ADDCH]; //K=0 - trigger on the signal; k!=0 - on derivative
  Int_t thresh[MAX_CH+ADDCH];
  Int_t bkg1[MAX_CH+ADDCH];
  Int_t bkg2[MAX_CH+ADDCH];
  Int_t peak1[MAX_CH+ADDCH];
  Int_t peak2[MAX_CH+ADDCH];
  Int_t deadT[MAX_CH+ADDCH];
  Int_t pile[MAX_CH+ADDCH];
  //Int_t pile2[MAX_CH+ADDCH];
  Int_t timing[MAX_CH+ADDCH];
  Int_t twin1[MAX_CH+ADDCH];
  Int_t twin2[MAX_CH+ADDCH];

  Float_t emult[MAX_CH+ADDCH];
  Float_t elim1[MAX_CH+ADDCH];
  Float_t elim2[MAX_CH+ADDCH];


  //Int_t channels[MAX_CH+1]; //+all
  //Int_t color[MAX_CH+1]; //+all
  //Int_t lcolor[MAX_L];
  //char chname[MAX_CH+1][16]; //+all

  //----------------------------------------------
  // Important common parameters

  //TDatime F_start,F_stop; //start and stop of the acquisition (?)
  //TTimeStamp F_start; //start and of the acquisition / start event in a file
  Long64_t F_start; //start of the acuisition
  Float_t T_acq; //duration of the acquisition / file (in seconds)

  Float_t Tstart,Tstop;
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
  
  Bool_t decode;
  Bool_t analyze;

  Bool_t raw_write;
  Bool_t dec_write;
  Bool_t root_write;
  Int_t raw_compr; //raw data compr level
  Int_t dec_compr; //decoded data compr level
  Int_t root_compr; //decoded data compr level
  char fname_raw[99];
  char fname_dec[99];
  char fname_root[99];

  Int_t ev_min; //minimal length of events list
  Int_t ev_max; //maximal length of events list

  Int_t tgate; // coincidence window for events (in samples)
  Int_t tveto; // veto window for pulses from the same channel
  //Int_t tgate2; // coincidence window for histograms (in ...)

  UInt_t mult1; // minimal multiplicity
  UInt_t mult2; // maximal multiplicity

  Int_t seltab;

  Float_t time_min,time_max;
  Float_t tof_min,tof_max;
  Float_t mtof_min,mtof_max;
  Float_t amp_min,amp_max;
  Float_t hei_min,hei_max;
  Float_t per_min,per_max;
  Float_t h2d_min,h2d_max;

  Float_t time_bins;
  Float_t tof_bins;
  Float_t mtof_bins;
  Float_t amp_bins;
  Float_t hei_bins;
  Float_t per_bins;
  Float_t h2d_bins;

  //b-flags: - create histograms and analyze them
  Bool_t b_time;
  Bool_t b_tof;
  Bool_t b_mtof;
  Bool_t b_amp;
  Bool_t b_hei;
  Bool_t b_per;
  Bool_t b_h2d;

  //s-flags: Show histograms in fListTree
  Bool_t s_time[MAX_CH];
  Bool_t s_tof[MAX_CH];
  Bool_t s_mtof[MAX_CH];
  Bool_t s_amp[MAX_CH];
  Bool_t s_hei[MAX_CH];
  Bool_t s_per[MAX_CH];
  Bool_t s_h2d[MAX_CH];

  //w-flags: Put histograms in WORK* folders
  Bool_t w_time[MAX_CH];
  Bool_t w_tof[MAX_CH];
  Bool_t w_mtof[MAX_CH];
  Bool_t w_amp[MAX_CH];
  Bool_t w_hei[MAX_CH];
  Bool_t w_per[MAX_CH];
  Bool_t w_h2d[MAX_CH];

  //index of cuts
  Char_t cut_time[MAX_CH*MAXCUTS];
  Char_t cut_tof[MAX_CH*MAXCUTS];
  Char_t cut_mtof[MAX_CH*MAXCUTS];
  Char_t cut_amp[MAX_CH*MAXCUTS];
  Char_t cut_hei[MAX_CH*MAXCUTS];
  Char_t cut_per[MAX_CH*MAXCUTS];
  Char_t cut_h2d[MAX_CH*MAXCUTS];

  Int_t start_ch;

  Int_t num_events;
  Int_t num_buf; //number of buffers to analyze in DoNbuf

  //Int_t b_osc,b_leg,b_logy,b_time;
  Bool_t b_logy;
  Bool_t b_gcuts;
  Bool_t b_deriv[3];
  Bool_t b_peak[16];
  
  //std::vector<Float_t> cut[3];
  Int_t ncuts;
  Int_t pcuts[MAXCUTS]; //number of points in gcut
  Float_t gcut[MAXCUTS][2][MAX_PCUTS]; //20 cuts; xy; 10 points

  char formula[90];

 public:
  //void InitPar(Int_t module);

  //void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);


  ClassDef(Toptions, 108)
};

ClassImp(Toptions)

#endif
