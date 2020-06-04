#ifndef toptions_H
#define toptions_H 1

#include "common.h"

#include <TObject.h>
#include <TROOT.h>
#include <TCutG.h>
//#include <TDatime.h>
#include <TTimeStamp.h>

/*
struct pmap2 {
  void* par; //address of the parameter
  // P_Def type; //p_fnum p_inum p_chk p_cmb p_txt
  // char all; //1 - all parameters, >1 - channel type
  // byte cmd; //for Command_crs
  // byte chan; //for Command_crs
};
*/

//------------------------------------
/*
class HBuf {
public:
  HBuf() {};
  virtual ~HBuf() {};

  UShort_t mod;
  UShort_t sz;

  char *buf; //[sz]
  ClassDef(HBuf, 1)
};
*/
//------------------------------------
class Hdef {
public:
  Hdef();
  virtual ~Hdef() {};

  Float_t bins,min,max,bins2;
  Bool_t b; //book histogram or not
  Bool_t c[MAX_CH+NGRP];
  Bool_t w[MAX_CH+NGRP];
  Int_t cut[MAX_CH+NGRP];
  ClassDef(Hdef, 1)
};
//------------------------------------

class Coptions: public TObject {
public:
  Coptions();
  virtual ~Coptions() {};

  //Version_t ver;
  // parameters of the crs32 or crs2 module

  //Int_t chtype[MAX_CH+MAX_TP]; //channel type
  Int_t smooth[MAX_CH+MAX_TP]; //Hardware Smooth - SS=0..10; S=2^SS
  Int_t deadTime[MAX_CH+MAX_TP]; // B = 1..16383
  Int_t preWr[MAX_CH+MAX_TP]; // pre-length M = 0..4094
  Int_t durWr[MAX_CH+MAX_TP]; // total length N = 1…32763 (internally - multiple of 4)
  Int_t delay[MAX_CH+MAX_TP]; //hardware delay in samples
  Int_t trg[MAX_CH+MAX_TP]; // Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative
  Int_t kderiv[MAX_CH+MAX_TP]; // K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  Int_t threshold[MAX_CH+MAX_TP]; // T = –2048 .. +2047
  Int_t adcGain[MAX_CH+MAX_TP]; // G = 0..12
  Bool_t acdc[MAX_CH+MAX_TP]; // AC-1; DC-0
  Bool_t inv[MAX_CH+MAX_TP]; //0 - no inversion; 1 - inversion (individual)
  // 0 - only triggered channel is written; 
  // 1 - both channels are written with any trigger
  Bool_t enabl[MAX_CH+MAX_TP]; //1 - enabled; 0 - disabled
  Bool_t pls[MAX_CH+MAX_TP]; //1 - send pulses (format 2,3); 0 - don't send
  //UInt_t Mask[MAX_CH+MAX_TP];
  Bool_t forcewr; //only for crs2
  Bool_t St_trig; //force trigger on start
  Int_t DTW; //Start dead time window
  Int_t Smpl; //Sampling rate divider
  Int_t SPeriod; //Start imitator period

public:
  void InitPar(int zero);
  void GetPar(const char* name, Int_t module, Int_t i, Int_t type_ch, Int_t &par, Int_t &min, Int_t &max);

  ClassDef(Coptions, 115)
};

//------------------------------------

class Toptions: public TObject {
public:
  Toptions();
  virtual ~Toptions() {};

  Int_t chtype[MAX_CH+MAX_TP]; //channel type
  Bool_t dsp[MAX_CH+MAX_TP]; //true - use dsp for data analysis
  Bool_t St[MAX_CH+MAX_TP]; //[Start]
  //Bool_t Nt[MAX_CH+MAX_TP]; //[Mrk] flag to use channel for ntof
  Bool_t Grp[MAX_CH+MAX_TP][NGRP]; // flag to use channel in group histograms
  //UInt_t ch_flag[MAX_CH+MAX_TP];
  Int_t sS[MAX_CH+MAX_TP]; //[nsmoo] software smoothing 0..100
  Int_t Drv[MAX_CH+MAX_TP]; //[kdrv] parameter of derivative
  Int_t Thr[MAX_CH+MAX_TP];//[thresh]
  Float_t Delay[MAX_CH+MAX_TP]; //[delay]
  Int_t Base1[MAX_CH+MAX_TP]; //[bkg1]
  Int_t Base2[MAX_CH+MAX_TP]; //[bkg2]
  Int_t Peak1[MAX_CH+MAX_TP]; //[peak1]
  Int_t Peak2[MAX_CH+MAX_TP]; //[peak2]
  Int_t dT[MAX_CH+MAX_TP];//[deadT]
  Int_t Pile[MAX_CH+MAX_TP]; //[pile]
  //Int_t pile2[MAX_CH+MAX_TP];
  Int_t sTg[MAX_CH+MAX_TP]; // [strg] Soft Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative; -1 - use hardware trigger
  Int_t timing[MAX_CH+MAX_TP];
  Int_t T1[MAX_CH+MAX_TP]; // [twin1]
  Int_t T2[MAX_CH+MAX_TP]; // [twin2]
  Int_t W1[MAX_CH+MAX_TP]; // [wwin1]
  Int_t W2[MAX_CH+MAX_TP]; // [wwin2]

  Float_t E0[MAX_CH+MAX_TP]; // [emult0]
  Float_t E1[MAX_CH+MAX_TP]; // [emult]
  Float_t E2[MAX_CH+MAX_TP]; // [emult2]
  Float_t Bc[MAX_CH+MAX_TP]; // [bcor]

  Float_t elim1[MAX_CH+MAX_TP];
  Float_t elim2[MAX_CH+MAX_TP];


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

  Float_t Period; //inverse frequency of digitizer (in ns):
  // 5 ns for CRS; 10 ns for adcm; ?? ns for adcm64

  Int_t nthreads;
  Int_t Nchan;
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

  //Int_t sel_hdiv; //number of divisions in histframe

  //histframe vars
  //Bool_t fopen; //true: open-; false: open+
  Bool_t b_stack; // draw stack
  Bool_t b_norm; // normalize histograms in the stack
  Int_t xdiv; // ndiv on X
  Int_t ydiv; // ndiv on Y
  Int_t icheck; //first histogram to plot among checked
  
  Bool_t decode;
  //Bool_t analyze_or_dsp; //true - raw analyze, false - use dsp
  Bool_t checkdsp;

  Bool_t raw_write;
  Bool_t raw_flag; //0 - write direct raw stream; 1 - write processed events
  Bool_t dec_write;
  Bool_t root_write;
  Int_t raw_compr; //raw data compr level
  Int_t dec_compr; //decoded data compr level
  Int_t root_compr; //decoded data compr level
  //char fname_raw[199];
  //char fname_dec[199];
  //char fname_root[199];
  char Filename[199];

  char ch_name[MAX_TP][6];

  Int_t ev_min; //minimal length of events list
  Int_t ev_max; //maximal length of events list

  Int_t tgate; // coincidence window for events (in samples)
  Int_t tveto; // veto window for pulses from the same channel
  //Int_t tgate2; // coincidence window for histograms (in ...)

  UInt_t mult1; // minimal multiplicity
  UInt_t mult2; // maximal multiplicity

  Int_t seltab;

  Int_t start_ch;
  Float_t ntof_period;
  Float_t Flpath;
  Float_t TofZero;

  Int_t prof_nx;
  Int_t prof_ny;
  Int_t Ing_x[16];
  Int_t Ing_y[16];
  Int_t Prof_x[8];
  Int_t Prof_y[8];
  Int_t Prof64[5];
  Int_t Prof64_W[3]; //Windows: offset, period, width (in smp)

  //Int_t prof_ch[MAX_CH];

  Int_t num_events;
  Int_t num_buf; //number of buffers to analyze in DoNbuf

  //Int_t b_osc,b_leg,b_logy,b_time;
  Bool_t b_logy;
  Bool_t b_stat;
  Bool_t b_gcuts;
  Bool_t b_deriv[3];
  Bool_t b_peak[16];

  //std::vector<Float_t> cut[3];
  Int_t ncuts;
  Int_t pcuts[MAXCUTS]; //number of points in gcut
  Float_t gcut[MAXCUTS][2][MAX_PCUTS]; //20 cuts; xy; 10 points

  char formula[100];
  char cut_form[MAXCUTS][100];
  Int_t maintrig;
  //char maintrig[22];

  Hdef h_time;
  Hdef h_area;
  Hdef h_area0;
  Hdef h_base;
  Hdef h_slope1;
  Hdef h_slope2;
  Hdef h_hei;
  Hdef h_tof;
  Hdef h_ntof;
  Hdef h_etof;
  Hdef h_ltof;
  Hdef h_per;
  Hdef h_width;
  Hdef h_width2;
  //Hdef h_width3;
  Hdef h_pulse;
  Hdef h_deriv;

  Hdef h_axay;
  Hdef h_area_base;
  Hdef h_area_sl1;
  Hdef h_area_sl2;
  Hdef h_slope_12;
  Hdef h_area_time;
  Hdef h_area_width;
  Hdef h_area_width2;
  Hdef h_area_ntof;
  Hdef h_prof;
  Hdef h_prof_x;
  Hdef h_prof_y;
  Hdef h_prof_ax;
  Hdef h_prof_ay;
  //Hdef h_area_width3;
  //Hdef h_width_12;
public:
  //void InitPar(Int_t module);

  //void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);


  ClassDef(Toptions, 115)
};

//ClassImp(Toptions)

#endif
