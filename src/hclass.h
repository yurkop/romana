#ifndef hclass_H
#define hclass_H 1

#include "common.h"
#include <TH1.h>
#include <TH2.h>

class HMap; //forward declaration

//-----------------------------------------------
class HMap: public TNamed {

 public:

  HMap(const char* dname, TH1* hist, Bool_t* s, Bool_t* w,
       Char_t *cuts);
  ~HMap();
  HMap(const HMap& other);
  HMap& operator=(const HMap& other);
  
  TH1* hst;
  Bool_t* chk; //item is checked
  Bool_t* wrk; //item is in the WORK directory (and in WORK_CUT*)
  Char_t *cut_index; //список окон, заданных на этой гистограмме (hst)
  //TList* list_cuts; 
  HMap* h_cuts[MAXCUTS];
  //TList* list_h_cuts;
  //список копий этой гистограммы (map), которые будут
  //заполняться при попадании события в какое-то окно
  //(не обязательно в этой гистограмме)

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
  TH1F* h_per[MAX_CH][MAXCUTS]; // period

  TH2F* h_2d[1][MAXCUTS];

  HMap* m_ampl[MAX_CH][MAXCUTS]; //amplitude - area of the peak
  HMap* m_height[MAX_CH][MAXCUTS]; //height of the peak
  HMap* m_time[MAX_CH][MAXCUTS]; // real time
  HMap* m_tof[MAX_CH][MAXCUTS]; // time of flight
  HMap* m_mtof[MAX_CH][MAXCUTS]; // time of flight
  HMap* m_per[MAX_CH][MAXCUTS]; // period

  HMap* m_2d[1][MAXCUTS];

  TCutG* cutG[MAXCUTS];
  //HMap *cutmap[MAXCUTS];

  TList* hilist;

  Long64_t T_prev[MAX_CH];

  Bool_t wfalse;

 public:

  HClass();
  virtual ~HClass();

   //void NameTitle();
  void Make_1d(const char* dname, const char* name, const char* title,
	       HMap* mm[MAX_CH][MAXCUTS], TH1F* hh[MAX_CH][MAXCUTS],
	       Float_t bins, Float_t min, Float_t max,
	       Bool_t bb, Bool_t* sel, Bool_t* wrk, Char_t *cuts);
  void Make_2d(const char* dname, const char* name, const char* title,
	       HMap* mm[][MAXCUTS], TH2F* hh[][MAXCUTS],
	       Float_t bins, Float_t min, Float_t max,
	       Bool_t bb, Bool_t* sel, Bool_t* wrk, Char_t *cuts);
  void Make_hist();
   //void NewBins();
   //void Reset_hist();
   //void FillHist(EventClass* evt);

   ClassDef(HClass, 0)
};

#endif
