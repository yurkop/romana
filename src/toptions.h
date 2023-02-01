#ifndef toptions_H
#define toptions_H 1

#include "common.h"

#include <TObject.h>
#include <TROOT.h>
#include <TCutG.h>
//#include <TDatime.h>
#include <TTimeStamp.h>


/*
#include <list>
//------------------------------------
class HCuts {
public:
  //HCuts() {};
  virtual ~HCuts() {};
  std::list<std::pair<int,TCutG*>> ll;
  //TList ll;
  //std::list<int*> gcuts;
  //std::list<*TCutG> gcuts;
  ClassDef(HCuts, 1)
};
*/


//------------------------------------
class Hdef {
public:
  Hdef();
  virtual ~Hdef() {};
  //Hdef(const Hdef& other);
  //Hdef& operator=(const Hdef& other);

  Float_t bins,min,max,bins2;
  Bool_t b; //book histogram or not
  Bool_t c[MAX_CH+NGRP]; //check histogram
  Bool_t w[MAX_CH+NGRP]; //work - гистограмма присутствует в WORK*
  Int_t cut[MAX_CH+NGRP]; //see cut_index in HMap
  // список окон (cuts), заданных на этой гистограмме (hst)
  // положение бита в этой маске соответствует номеру соответствующего окна
  // максимальная размерность: 32 бита, значит число окон
  // не может быть больше 32
  // bit mask: 1 - cut is in this histogram; 0 - cut is not here

  //Float_t roi[MAX_CH+NGRP][MAXROI];
  Float_t roi[MAXROI][2];
  
  Int_t rb,rb2; //rebin
  ClassDef(Hdef, 7)
};
//------------------------------------

class Coptions: public TObject {
public:
  Coptions();
  virtual ~Coptions() {};

  //Version_t ver;
  // parameters of the crs32 or crs2 module

  Int_t crs_ch[MAX_CHTP]; //CRS channel type:
  //0 - CRS2/CRS32 (11 bit)
  //1 - CRS-6/16 (16 bit)
  //2 - CRS-8/16 or CRS-128 (16 bit)

  //Int_t test; // [ts1] [ts2] test
  Int_t hS[MAX_CHTP]; // [smooth] Hardware Smooth - SS=0..10; S=2^SS
  Int_t Dt[MAX_CHTP]; // [deadTime] B = 1..16383
  Int_t Pre[MAX_CHTP]; // [preWr] pre-length M = 0..4094
  Int_t Len[MAX_CHTP]; // [durWr] total length N = 1…32763 (internally - multiple of 4)
  Int_t hD[MAX_CHTP]; // [delay] hardware delay in samples
  Int_t Trg[MAX_CHTP]; // [trg] Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative
  Int_t Drv[MAX_CHTP]; // [kderiv] K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  Int_t Thr[MAX_CHTP]; // [threshold] T = –2048 .. +2047
  Int_t G[MAX_CHTP]; // [adcGain] G = 0..12
  Int_t fdiv[MAX_CHTP]; // frequency divider (=2 for 16-bit/100 MHz)
  Bool_t AC[MAX_CHTP]; // [acdc] AC-1; DC-0
  Bool_t Inv[MAX_CHTP]; // [inv] 0 - no inversion; 1 - inversion (individual)
  Bool_t on[MAX_CHTP]; // [enabl] 1 - enabled; 0 - disabled
  Bool_t pls[MAX_CHTP]; // 1 - send pulses (format 2,3); 0 - don't send

  Int_t coinc_w[2]; //ширина окна совпадений для групп 0,1
  Int_t mult_w1[2]; //минимальная множественность для групп 0,1
  Int_t mult_w2[2]; //максимальная множественность для групп 0,1
  Bool_t group[MAX_CHTP][2];
  Int_t ratediv[MAX_CHTP];


  //UInt_t Mask[MAX_CHTP];
  Bool_t forcewr; //only for crs2
  // 0 - only triggered channel is written; 
  // 1 - both channels are written with any trigger
  Int_t Trigger; // [St_trig]   0 - discr; 1 - START; 2 - coinc
  //Bool_t St_trig; //force trigger on start
  Int_t Smpl; //Sampling rate divider
  Int_t St_Per; //Start imitator period
  Int_t F24; //24-bit format for CRS-8, CRS-128
  Int_t DTW; //Start dead time window

  Long64_t F_start; //start of the acquisition
  Long64_t F_stop; //stop of the acquisition (usually time of last modification of file)

public:
  void InitPar(int zero);
  void GetPar(const char* name, Int_t module, Int_t i, Int_t type_ch, Int_t &par, Int_t &min, Int_t &max);
  Int_t ChkLen(Int_t i, Int_t module);

  ClassDef(Coptions, 131)
};

//------------------------------------

class Toptions: public TObject {
public:
  Toptions();
  virtual ~Toptions() {};

  char gitver[24]; //git version written in the .par file
  Int_t maxch;
  Int_t maxtp;

  Int_t chkall; //type of "all" action
  Bool_t star[MAX_CHTP]; //asterix
  Int_t chtype[MAX_CHTP]; //ch type, starts from 1 (see MAX_TP in common.h)
  Bool_t dsp[MAX_CHTP]; //true - use dsp for data analysis
  Bool_t St[MAX_CHTP]; //[Start]
  Bool_t Ms[MAX_CHTP]; // [Master] Master/slave (see Spin)
  //Bool_t Nt[MAX_CHTP]; //[Mrk] flag to use channel for ntof
  Bool_t Grp[MAX_CHTP][NGRP]; // flag to use channel in group histograms
  //UInt_t ch_flag[MAX_CHTP];
  Int_t sS[MAX_CHTP]; //[nsmoo] software smoothing 0..100
  Int_t sDrv[MAX_CHTP]; //[Drv] [kdrv] parameter of derivative
  Int_t sThr[MAX_CHTP];//[Thr] [thresh]
  Float_t sD[MAX_CHTP]; //[Delay] [delay]
  Int_t Base1[MAX_CHTP]; //[bkg1]
  Int_t Base2[MAX_CHTP]; //[bkg2]
  Int_t Peak1[MAX_CHTP]; //[peak1]
  Int_t Peak2[MAX_CHTP]; //[peak2]
  Int_t dTm[MAX_CHTP];//[deadT][dT]
  Int_t Pile[MAX_CHTP]; //[pile]
  //Int_t pile2[MAX_CHTP];
  Int_t sTg[MAX_CHTP]; // [strg] Soft Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative; -1 - use hardware trigger
  //Int_t timing[MAX_CHTP];
  Int_t T1[MAX_CHTP]; // [twin1]
  Int_t T2[MAX_CHTP]; // [twin2]
  Int_t W1[MAX_CHTP]; // [wwin1]
  Int_t W2[MAX_CHTP]; // [wwin2]

  Int_t calibr_t[MAX_CHTP]; //type of calibration
  // 0 - no calibration; 1 - linear; 2 - parabola; 3 - spline
  Float_t E0[MAX_CHTP]; // [emult0]
  Float_t E1[MAX_CHTP]; // [emult]
  Float_t E2[MAX_CHTP]; // [emult2]
  Float_t Bc[MAX_CHTP]; // [bcor]

  Float_t E_auto; //value for auto energy pre-calibration
  Float_t adj[MAX_CH+NGRP+1][3];

  Float_t elim1[MAX_CHTP];
  Float_t elim2[MAX_CHTP];
  //----------------------------------------------
  // Important common parameters

  //Long64_t F_start; //start of the acquisition
  Float_t T_acq; //duration of the acquisition / file (in seconds)

  Float_t Period; //period (inversed frequency) of digitizer (in ns):
  // 5 ns for CRS; 10 ns for adcm; ?? ns for adcm64

  Int_t nthreads;
  Int_t Nchan;
  Float_t Tstart; //Tlim1
  Float_t Tstop; //Tlim2
  //Float_t Tstart,Tstop;
  Int_t tsleep;
  Int_t usb_size; //in kB
  Int_t rbuf_size; //in kB

  //Int_t event_buf; //length of event buffer
  //analysis starts only after filling first event_buf

  Int_t event_lag; //maximal lag of event analysis in mks
  // if current pulse has tstamp smaller than last event's T
  // by this value, the pulse is inserted into event_list
  // without  (if zero - calculate automatically)

  //Int_t sel_hdiv; //number of divisions in histframe

  //histframe vars
  //Bool_t fopen; //true: open-; false: open+
  Bool_t b_stack; // draw stack
  Bool_t b_norm; // normalize histograms in the stack
  Int_t xdiv; // ndiv on X
  Int_t ydiv; // ndiv on Y
  Int_t icheck; //first histogram to plot among checked
  
  //Bool_t decode;
  Bool_t directraw;
  //Bool_t analyze_or_dsp; //true - raw analyze, false - use dsp
  Bool_t checkdsp;

  Bool_t raw_write;
  Bool_t fProc; //0 - write direct raw stream; 1 - write processed events
  Bool_t fTxt; //1 - write txt file with events
  Bool_t dec_write;
  Bool_t root_write;
  Int_t raw_compr; //raw data compr level
  Int_t dec_compr; //decoded data compr level
  Int_t root_compr; //root compr level
  Int_t dec_format;
  //char fname_raw[199];
  //char fname_dec[199];
  //char fname_root[199];
  //TString S_Filename;
  char Filename[100]; //->

  //TString S_ch_name[MAX_TP];
  char ch_name[MAX_TP][20];
  //char ch_name2[3][4][5][MAX_TP][20]; //->
  char drawopt[30];

  Int_t ev_min; //minimal length of events list
  Int_t ev_max; //maximal length of events list

  //software logic
  Int_t tgate; // coincidence window for events (in samples)
  Int_t tveto; // veto window for pulses from the same channel
  Int_t mult1; // minimal multiplicity
  Int_t mult2; // maximal multiplicity
  Int_t maintrig;

  //hardware logic
  //Bool_t hard_logic; //0 - use software logic; 1 - use hardware logic

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
  Int_t Prof64[5]; //position channels for Prof64 + clock(?) [4]
  char Prof64_TSP[10];
  Int_t Prof64_W[3]; //Windows: period, offset, width (in smp)
  Int_t Prof64_THR; //Threshold
  Int_t Prof64_GAT; //Coincidence gate with ing27 alpha

  //Int_t prof_ch[MAX_CH];

  Int_t num_events;
  Int_t num_buf; //number of buffers to analyze in DoNbuf

  //Int_t b_osc,b_leg,b_logy,b_time;
  Bool_t b_logy;
  Bool_t b_stat;
  Bool_t b_gcuts;
  Bool_t b_roi;
  Bool_t b_deriv[3];
  Bool_t b_peak[MXPK];

  Float_t adcm_period;
  //HCuts cuts;
  //std::vector<Float_t> cut[3];
  //std::list<Float_t> gcut2;

  Int_t ncuts;
  Int_t pcuts[MAXCUTS]; // number of points in gcut: 1-formula; 2-1d; >2-2d
                        // 0- no cut
  Float_t gcut[MAXCUTS][2][MAX_PCUTS]; //20 cuts; xy; 10 points
  char cut_form[MAXCUTS][100];

  char formula[100];
  //char maintrig[22];

  Float_t SimAmp;
  Float_t SimSig;

  Hdef h_rate;
  Hdef h_hwrate;
  Hdef h_mult;
  Hdef h_area;
  Hdef h_area0;
  Hdef h_base;
  Hdef h_slope1;
  //Hdef h_slope2;
  Hdef h_simul;
  Hdef h_hei;
  Hdef h_time;
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
  //Hdef h_area_sl2;
  //Hdef h_slope_12;
  Hdef h_time_simul;
  Hdef h_area_time;
  Hdef h_area_width;
  Hdef h_area_width2;
  Hdef h_area_ntof;
  Hdef h_prof;
  // Hdef h_prof_x;
  // Hdef h_prof_y;
  // Hdef h_prof_ax;
  // Hdef h_prof_ay;
  // Hdef h_prof_nm;
  Hdef h_prof64;
  //Hdef h_area_width3;
  //Hdef h_width_12;
public:
  //void InitPar(Int_t module);

  //void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);


  ClassDef(Toptions, 131)
};

//ClassImp(Toptions)

#endif
