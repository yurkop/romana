#ifndef hclass_H
#define hclass_H 1

#include "common.h"
#include <TH1.h>
#include <TH2.h>

//-----------------------------------------------
class HMap: public TNamed {

 public:

  HMap(const char* dname, TH1* hist, Bool_t* s, Bool_t* w);
  ~HMap();
  HMap(const HMap& other);
  HMap& operator=(const HMap& other);
  
  TH1* hst;
  Bool_t* chk;
  Bool_t* wrk;

  ClassDef(HMap, 0)
};

//-----------------------------------------------
class HClass {

 public:

  TH1F* h_ampl[MAX_CH][MAXCUTS]; //amplitude - area of the peak
  TH1F* h_height[MAX_CH][MAXCUTS]; //height of the peak
  TH1F* h_time[MAX_CH][MAXCUTS]; // real time
  TH1F* h_tof[MAX_CH][MAXCUTS]; // time of flight
  TH1F* h_mtof[MAX_CH][MAXCUTS]; // time of flight
  TH1F* h_period[MAX_CH][MAXCUTS]; // period

  TH2F* h_2d[1][MAXCUTS];

  TCutG* cutG[MAXCUTS];

  TList* hilist;

  Long64_t T_prev[MAX_CH];

 public:

  HClass();
  virtual ~HClass();

   //void NameTitle();
  void Make_1d(const char* dname, const char* name, const char* title,
	       TH1F* hh[MAX_CH][MAXCUTS],
	       Float_t bins, Float_t min, Float_t max,
	       Bool_t bb, Bool_t* sel, Bool_t* wrk);
  void Make_2d(const char* dname, const char* name, const char* title,
	       TH2F* hh[][MAXCUTS],
	       Float_t bins, Float_t min, Float_t max,
	       Bool_t bb, Bool_t* sel, Bool_t* wrk);
   void Make_hist();
   //void NewBins();
   //void Reset_hist();
   //void FillHist(EventClass* evt);

   ClassDef(HClass, 0)
};

#endif
