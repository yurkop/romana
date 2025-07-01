#ifndef toptions_H
#define toptions_H 1

#include "common.h"

#include <TObject.h>
#include <TROOT.h>
#include <TCutG.h>
//#include <TDatime.h>
#include <TTimeStamp.h>

#define CDEF 166

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


const int MM = 8; //количество типов модулей для mcpar
typedef std::array<int,2*MM> arr;

const int MOP = 15; // size of OptPar array

//------------------------------------
class GG {
public:
  std::map<void*,arr> mcpar;
};

class Hdef: public TNamed  {
  // класс содержит параметры гистограмм в простых переменных,
  // которые можно легко сохранить в файл 
public:
  Hdef();
  virtual ~Hdef() {};
  //Hdef(const Hdef& other);
  //Hdef& operator=(const Hdef& other);


  Float_t bins,min,max,bins2;
  Int_t cut[MAX_CH+NGRP]; //see cut_index in HMap
  // список окон (cuts), заданных на этой гистограмме (hst)
  // положение бита в этой маске соответствует номеру соответствующего окна
  // максимальная размерность: 32 бита, значит число окон
  // не может быть больше 32
  // bit mask: 1 - cut is in this histogram; 0 - cut is not here

  //Float_t roi[MAX_CH+NGRP][MAXROI];
  Float_t roi[MAXROI][2];
  
  Int_t rb,rb2; //rebin

  Bool_t b; //book histogram or not
  Bool_t c[MAX_CH+NGRP]; //check histogram
  Bool_t w[MAX_CH+NGRP]; //MAIN - гистограмма присутствует в MAIN*
  Bool_t htp; // type: false - Float; true - Double

  ClassDef(Hdef, CDEF)
};
//------------------------------------

class Coptions: public TObject {
public:
  Coptions();
  virtual ~Coptions() {};

  // в квадратных скобках после комментария приводятся старые имена
  // для обратной совместимости. Их количество не ограничено.
  // Все имена (новые и старые) должны быть уникальны
  // (общее пространоство имен для cpar и opt).

  //Version_t ver;
  // parameters of the crs32 or crs2 module

  // crs_ch перенесено в libcrs
  //Int_t crs_ch[MAX_CHTP]; //CRS channel type:
  //0 - CRS2/CRS32 (11 bit)
  //1 - CRS-6/16 (16 bit)
  //2 - CRS-8/16 or CRS-128 (16 bit)
  //255 - no module (undefined)

  //Int_t test; // [ts1] [ts2] test
  Int_t hS[MAX_CHTP]; // [smooth] Hardware Smooth - SS=0..10; S=2^SS
  Int_t Dt[MAX_CHTP]; // [deadTime] B = 1..16383
  Int_t Pre[MAX_CHTP]; // [preWr] pre-length M = 0..4094
  Int_t Len[MAX_CHTP]; // [durWr] total length N = 1…32763 (internally - multiple of 4)
  Int_t hD[MAX_CHTP]; // [delay] hardware delay in samples
  Int_t Trg[MAX_CHTP]; // [trg] Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative
  Int_t Drv[MAX_CHTP]; // [kderiv] K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  Int_t Thr[MAX_CHTP]; // [threshold] T = –2048 .. +2047
  Int_t LT[MAX_CHTP]; // lower threshold T = –2048 .. +2047
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
  Int_t RD[MAX_CHTP]; // [ratediv] - rate divider

  UInt_t RMask; //raw data mask - common mask for raw data transfer
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

  //Int_t Thr2; // lower threshold for trig 3,4 T = –2048 .. +2047

  UChar_t device[4] = {}; // device_code, Serial_n, nplates, ver_po

public:
  void InitPar(int zero);
  std::string GetDevice(int module, int opt=1);
  void InitMinMax();
  void GetParm(const char* name, Int_t i, void *par, Int_t &min, Int_t &max);
  //void GetPar(const char* name, Int_t module, Int_t i, Int_t crs_ch, Int_t &par, Int_t &min, Int_t &max);
  Int_t ChkLen(Int_t i, Int_t module);

  ClassDef(Coptions, CDEF)
};

//------------------------------------

class Toptions: public TObject {
public:
  Toptions();
  virtual ~Toptions() {};

  // в квадратных скобках после комментария приводятся старые имена
  // для обратной совместимости. Их количество не ограничено.
  // Все имена (новые и старые) должны быть уникальны
  // (общее пространоство имен для cpar и opt).

  char gitver[24]; //git version written in the .par file
  Int_t maxch;
  Int_t maxtp;

  Int_t chkall; //type of "all" action
  Bool_t star[MAX_CHTP]; //asterix
  Int_t chtype[MAX_CHTP]; //ch type, starts from 1 (see MAX_TP in common.h)
  Bool_t dsp[MAX_CHTP]; //true - send dsp data
  Bool_t Dsp[MAX_CHTP]; //true - use dsp for data analysis
  Bool_t Pls[MAX_CHTP]; //true - write pulses in Dec
  Bool_t St[MAX_CHTP]; //[Start]
  Bool_t Ms[MAX_CHTP]; // [Master] Master/slave (see Spin)
  //Bool_t Nt[MAX_CHTP]; //[Mrk] flag to use channel for ntof
  Bool_t Grp[MAX_CHTP][NGRP]; // flag to use channel in group histograms
  //UInt_t ch_flag[MAX_CHTP];
  Int_t sS[MAX_CHTP]; //[nsmoo] software smoothing 0..100
  Int_t sDrv[MAX_CHTP]; //[Drv] [kdrv] parameter of derivative
  Int_t sThr[MAX_CHTP]; //[Thr] [thresh]
  Float_t sD[MAX_CHTP]; //[Delay] [delay]
  Int_t B1[MAX_CHTP]; //[Base1] [bkg1]
  Int_t B2[MAX_CHTP]; //[Base2] [bkg2]
  Int_t P1[MAX_CHTP]; //[Peak1] [peak1]
  Int_t P2[MAX_CHTP]; //[Peak2] [peak2]
  Int_t dTm[MAX_CHTP];//[deadT][dT]
  Int_t Pile[MAX_CHTP]; //[pile]
  //Int_t pile2[MAX_CHTP];
  Int_t sTg[MAX_CHTP]; // [strg] Soft Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative; -1 - use hardware trigger
  //Int_t timing[MAX_CHTP];

  Int_t DD[MAX_CHTP]; // CFD delay
  Int_t FF[MAX_CHTP]; // CFD fraction*10
  Int_t T1[MAX_CHTP]; // [twin1]
  Int_t T2[MAX_CHTP]; // [twin2]
  Int_t W1[MAX_CHTP]; // [wwin1]
  Int_t W2[MAX_CHTP]; // [wwin2]

  Int_t Mt[MAX_CHTP]; //analysis method
  Int_t calibr_t[MAX_CHTP]; //type of calibration
  // 0 - no calibration; 1 - linear; 2 - parabola; 3 - spline
  Float_t E0[MAX_CHTP]; // [emult0]
  Float_t E1[MAX_CHTP]; // [emult]
  Float_t E2[MAX_CHTP]; // [emult2]
  //Float_t Bc[MAX_CHTP]; // [bcor]
  Int_t Pz[MAX_CHTP];

  Float_t E_auto; //value for auto energy pre-calibration
  Float_t adj[MAX_CH+NGRP+1][3];

  Float_t elim1[MAX_CHTP];
  Float_t elim2[MAX_CHTP];

  //Bool_t a_base[MAX_CHTP]; //анализировать base,slope,RMS
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

  //Int_t event_lag; //maximal lag of event analysis in mks
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
  char dec_mask[20];
  //char fname_raw[199];
  //char fname_dec[199];
  //char fname_root[199];
  //TString S_Filename;
  char Filename[255]; //->

  bool addrandom; //true: add random number while filling histograms
  // (see Fill_01; Fill_02) 

  //TString S_ch_name[MAX_TP];
  char ch_name[MAX_TP][20];
  //char ch_name2[3][4][5][MAX_TP][20]; //->
  char drawopt[30];

  Int_t ev_min; //Event lag: minimal length of events list
  Int_t ev_max; //Event_list size: maximal length of events list

  //software logic
  Int_t tgate; // coincidence window for events (in samples)
  Int_t tveto; // veto window for pulses from the same channel
  Int_t mult1; // minimal multiplicity
  Int_t mult2; // maximal multiplicity
  Int_t maintrig;

  //hardware logic
  //Bool_t hard_logic; //0 - use software logic; 1 - use hardware logic

  Int_t seltab;

  Bool_t b_ntof; //analyze ntof ot not
  Int_t start_ch;
  Float_t ntof_period;
  Float_t Flpath;
  Float_t TofZero;

  Int_t Prof_type; //Profilometer type: 64 or 8
  Int_t Ing_type; //Profilometer type: 64 or 8

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
  Int_t Prof64_X=0; //X-offset of the profilometer in mm
  Int_t Prof64_Y=0; //Y-offset of the profilometer in mm

  //Int_t prof_ch[MAX_CH];

  Int_t num_events;
  Int_t num_buf; //number of buffers to analyze in DoNbuf

  //Int_t b_osc,b_leg,b_logy,b_time;
  Bool_t b_logy;
  Bool_t b_stat;
  Bool_t b_gcuts;
  Bool_t b_roi;
  Bool_t b_fpeaks;
  Bool_t b_deriv[NDIV];
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

  Float_t SimSim[20]; // used for Simul: see toptions.cpp

  //parameters for PeakSearch
  Float_t Peak_thr;
  Float_t Peak_wid;
  Float_t Peak_bwidth;
  Float_t Peak_maxpeaks;
  Float_t Peak_smooth;
  Bool_t Peak_show_sm;
  Bool_t Peak_use_mean;
  Bool_t Peak_print;

  //parameters for OptPar
  Int_t OptPar[MOP][4];


  int wrk_check[128];//каждый бит: номер папки MAIN_cut[bit]. 0-й бит: MAIN
  //в папке MAIN (старая wrk) можно чекнуть максимум 128 гистограмм

  Int_t Nrows;
  Int_t ScrollPos;

  Float_t hx_slider[2];
  Float_t hy_slider[2];

  // hnum: 
  //    0 - не определено
  //    1..9 - простые переменные из PulseClass
  //    21..49 - common for the whole event

  // format:
  // Hdef var // hnum hname xtitle ytitle tip 
  Hdef h_rate; // 21 Rate T(sec) Counts Software count rates as a funtion of time in seconds
  Hdef h_hwrate; // 22 HWRate T(sec) Counts Hardware counters as a funtion of time in seconds

  Hdef h_mult; // 49 Mult Multiplicity Counts Event multiplicity

  Hdef h_area; // 1 Area Channel Counts Area of the pulse or energy, calibrated (see Analysis->E0,E1,E2 for calibration)
  //Hdef h_area0; // 9 Area0 Channel Counts Area w/o background, not calibrated
  Hdef h_time; // 11 Time t(ns) Counts Time (relative to the starts - see Analysis->St), in ns
  Hdef h_hei; // 2 Height Channel Counts Maximal pulse height (in channels)
  Hdef h_width; // 3 Width width(a.u.) Counts Pulse width
  Hdef h_per; // 15 Period t(mks) Counts Pulse period (distance between two consecutive pulses), in mks
  Hdef h_base; // 4 Base Channel Counts Base line, calibrated



  Hdef h_slope1; // 5 Sl1 Slope Counts Slope1 (baseline)
  Hdef h_slope2; // 6 Sl2 Slope Counts Slope2 (peak)
  Hdef h_RMS1; // 7 RMS1 RMS Counts Noise1 (baseline)
  Hdef h_RMS2; // 8 RMS2 RMS Counts Noise2 (peak)
  Hdef h_RTime; // 9 RTime RiseTime Counts RiseTime of the pulse

  Hdef h_ampl; // 48 Ampl Channel Counts Distribution of amplitudes from (inside) oscillogram

  Hdef h_ntof; // 12 Ntof t(mks) Counts Neutron time of flight, in mks
  Hdef h_etof; // 13 Etof Energy(eV) Counts Neutron energy from NTOF, in eV
  Hdef h_ltof; // 14 Ltof Lambda(A) Counts Neutron wavelength from NTOF, in A

  Hdef h_pulse; // 51 Pulse samples Amplitude Average pulse shape
  Hdef h_deriv; // 52 Deriv samples Amplitude Average pulse shape derivative
  Hdef h_fft; // 53 FFT samples Amplitude Average FFT

  Hdef h_prof; // 61 Profilometer X(mm) Y(mm) 2d histograms for Profilometer
  Hdef h_prof_int; // 62 Prof_int - - Integral histograms for profilometer

  TList l2d; //нужен только для сохранения и чтения 2d гистограмм в root файл

#ifdef YUMO
  int yumo_x1;
  int yumo_x2;
  int yumo_y1;
  int yumo_y2;
  Float_t yumo_peak1;
  Float_t yumo_peak2;
  Hdef h_yumo_1d; // 71 Yumo_1d Time(ns) Counts 1d histograms for yumo
  Hdef h_yumo_2d; // 72 Yumo_2d X(ns) Y(ns) 2d histograms for yumo
  Hdef h_yumo_3d; // 73 Yumo_3d Time(ns) Counts 3d histogram for yumo
#endif

  //Hdef h_width2;
  //Hdef h_width3;
  //Hdef h_simul;

  // Hdef h_prof_x;
  // Hdef h_prof_y;
  // Hdef h_prof_ax;
  // Hdef h_prof_ay;
  // Hdef h_prof_nm;

  /* YK_OLD
  Hdef h_axay;
  Hdef h_area_base;
  Hdef h_area_sl1;
  Hdef h_area_sl2;
  Hdef h_slope_12;
  //Hdef h_time_simul;
  Hdef h_area_time;
  Hdef h_area_width;
  Hdef h_area_width2;
  Hdef h_area_ntof;
  //Hdef h_area_width3;
  //Hdef h_width_12;
  */

public:
  //void InitPar(Int_t module);

  //void GetPar(const char* name, Int_t module, Int_t i, Int_t &par, Int_t &min, Int_t &max);


  ClassDef(Toptions, CDEF)
};

//ClassImp(Toptions)

#endif
