#ifndef hclass_H
#define hclass_H 1

#include "common.h"
#include <TH1.h>
#include <TH2.h>

//-----------------------------------------------
class HClass {

 public:

  TH1F* h_ampl[MAX_CH][MAXCUTS]; //amplitude - area of the peak
  TH1F* h_height[MAX_CH][MAXCUTS]; //height of the peak
  TH1F* h_time[MAX_CH][MAXCUTS]; // real time
  TH1F* h_tof[MAX_CH][MAXCUTS]; // time of flight
  TH1F* h_mtof[MAX_CH][MAXCUTS]; // time of flight

  TH2F* h_2d[MAXCUTS];

  TCutG* cutG[MAXCUTS];

 public:

  HClass();
  virtual ~HClass();

   //void NameTitle();
   void Make_hist();
   void NewBins();
   //void Reset_hist();
   //void FillHist(EventClass* evt);

   ClassDef(HClass, 0)
};

#endif
