#ifndef hclass_H
#define hclass_H 1

#include "common.h"
#include <TH1.h>
#include <TH2.h>
#include <TFormula.h>

class HMap; //forward declaration

//-----------------------------------------------
class HMap: public TNamed {

 public:

  HMap();
  HMap(const char* dname, TH1* hist, Bool_t* s, Bool_t* w,
       Int_t *cuts);
  ~HMap();
  HMap(const HMap& other);
  HMap& operator=(const HMap& other);
  
  TH1* hst;
  Bool_t* chk; //item is checked
  Bool_t* wrk; //item is in the WORK directory (and in WORK_CUT*, WORK_MT)
  UShort_t* bitwk; //bit mask - item is checked in the WORK* folder
  Int_t *cut_index; //список окон (cuts), заданных на этой гистограмме (hst)
  // положение бита в этой маске соответствует номеру соответствующего окна
  // максимальная размерность: 32 бита, значит число окон
  // не может быть больше 32
  // bit mask: 1 - cut is in this histogram; 0 - cut is not here
  HMap* h_cuts[MAXCUTS];
  //список копий этой гистограммы (map), которые будут
  //заполняться при попадании события в какое-то окно/cut [i]
  //(не обязательно в этой гистограмме)
  //и будут храниться в папке WORK_cut[i]
  HMap* h_MT;
  //копия этой гистограммы (map), которая будет
  //заполняться при попадании события в мастер триггер
  //и будет храниться в папке WORK_MT

  ClassDef(HMap, 0)
};

//-----------------------------------------------
class HClass {

 public:

  HMap* m_time[MAX_CH+NGRP]; // real time
  HMap* m_area[MAX_CH+NGRP]; //area of the peak
  HMap* m_area0[MAX_CH+NGRP]; //area of the peak w/o bkg
  HMap* m_base[MAX_CH+NGRP]; //baseline - background
  HMap* m_slope1[MAX_CH+NGRP]; //slope1 - baseline
  HMap* m_slope2[MAX_CH+NGRP]; //slope2 - peak
  HMap* m_height[MAX_CH+NGRP]; //height of the peak
  HMap* m_tof[MAX_CH+NGRP]; // time of flight
  HMap* m_mtof[MAX_CH+NGRP]; // time of flight
  HMap* m_etof[MAX_CH+NGRP]; // Energy from time of flight
  HMap* m_per[MAX_CH+NGRP]; // period
  HMap* m_width[MAX_CH+NGRP]; // width
  HMap* m_width2[MAX_CH+NGRP]; // width

  HMap* m_pulse[MAX_CH]; // pulse shape
  HMap* m_deriv[MAX_CH]; // pulse shape (deriv)

  HMap* m_a0a1[1];
  HMap* m_area_base[MAX_CH];
  HMap* m_area_sl1[MAX_CH];
  HMap* m_area_sl2[MAX_CH];
  HMap* m_slope_12[MAX_CH];
  HMap* m_area_time[MAX_CH];
  HMap* m_area_width[MAX_CH];
  HMap* m_width_12[MAX_CH];

  TCutG* cutG[MAXCUTS];
  //Name: cut[i]
  //Title: histogram name on which this cut is made
  //nr of points: 1 - formula; 2 - 1d; >2 - 2d
  Double_t cut_flag[MAXCUTS]; //признак срабатывания окна
  char cuttitle[MAXCUTS][99];
  int cutcolor[MAXCUTS];
  TFormula* cform[MAXCUTS]; //starts from 1, not from 0
  bool b_formula; //at least one cut formula exists

  //HMap *cutmap[MAXCUTS];

  TList* map_list;
  TList* hist_list;
  TList* dir_list;

  Long64_t T_prev[MAX_CH];

  Bool_t wfalse;

 public:

  HClass();
  virtual ~HClass();

   //void NameTitle();
  void Make_1d(const char* dname, const char* name, const char* title,
	       HMap* map[], Hdef* hd);
  void Make_1d_pulse(const char* dname, const char* name,
		     const char* title, HMap* map[], Hdef* hd);
  void Make_2d(const char* dname, const char* name, const char* title,
   	       HMap* map[], Hdef* hd, Hdef* hd1, Hdef* hd2);
  void Make_a0a1(const char* dname, const char* name, const char* title,
	       HMap* map[], Hdef* hd);
  void Make_hist();
  void Clone_Hist(HMap* map);
  void Remove_Clones(HMap* map);
  void Make_cuts();
   //void NewBins();
   //void Reset_hist();
   //void FillHist(EventClass* evt);

   ClassDef(HClass, 0)
};

#endif
