#ifndef hclass_H
#define hclass_H 1

#include "common.h"
//#include "toptions.h"
#include <TH1.h>
#include <TH2.h>
#include <TFormula.h>
#include <TGraphErrors.h>

class HMap; //forward declaration

//-----------------------------------------------
class HMap: public TNamed {

 public:

  HMap();
  //HMap(const char* dname);
  HMap(const char* dname, Hdef* hd1);
  HMap(const char* dname, TH1* hist, Hdef* hd1, int i);
  HMap(const char* dname, TGraphErrors* gr1, Hdef* hd1, int i);
  HMap(const char* dname, TGraphErrors* igr, int i);
  ~HMap();
  HMap(const HMap& other);
  HMap& operator=(const HMap& other);
  
  TH1* hst;
  TGraphErrors* gr;
  Hdef* hd; //Hdef, которому принадлежит этот Hmap
  UShort_t nn; //index in Hdef
  UChar_t flg; // 0 - normal histogram, 1 - cut histogram
  //Int_t *rb; //rebin
  //Bool_t* chk; //item is checked
  //Bool_t* wrk; //item is in the MAIN directory (and in MAIN_CUT*, MAIN_MT)

  //UShort_t* bitwk; //bit mask - item is checked in the MAIN* folder

  // Int_t *cut_index; // = Hdef->cut
  // список окон (cuts), заданных на этой гистограмме (hst)
  // положение бита в этой маске соответствует номеру соответствующего окна
  // максимальная размерность: 32 бита, значит число окон
  // не может быть больше 32
  // bit mask: 1 - cut is in this histogram; 0 - cut is not here

  HMap* h_cuts[MAXCUTS];
  //список копий этой гистограммы (map), которые будут
  //заполняться при попадании события в какое-то окно/cut [i]
  //(не обязательно в этой гистограмме)
  //и будут храниться в папке MAIN_cut[i]

  //HMap* h_MT;
  //копия этой гистограммы (map), которая будет
  //заполняться при попадании события в мастер триггер
  //и будет храниться в папке MAIN_MT

  ClassDef(HMap, 0)
};

//-----------------------------------------------
class HClass {

 public:

  //static const int MAX_ING=256;

  HMap* m_rate[MAX_CH+NGRP]; // software rate in real time
  HMap* m_hwrate[MAX_CH+NGRP]; // hardware counter in real time
  HMap* m_mult[MAX_CH+NGRP]; // event multiplicity
  HMap* m_area[MAX_CH+NGRP]; //area of the peak
  HMap* m_area0[MAX_CH+NGRP]; //area of the peak w/o bkg
  HMap* m_base[MAX_CH+NGRP]; //baseline - background
  HMap* m_slope1[MAX_CH+NGRP]; //slope1 - baseline
  HMap* m_slope2[MAX_CH+NGRP]; //slope2 - peak
  //HMap* m_simul[MAX_CH+NGRP]; //simul
  HMap* m_height[MAX_CH+NGRP]; //height of the peak
  HMap* m_time[MAX_CH+NGRP]; // time of flight
  HMap* m_ntof[MAX_CH+NGRP]; // neutron time of flight
  HMap* m_etof[MAX_CH+NGRP]; // Energy from time of flight
  HMap* m_ltof[MAX_CH+NGRP]; // Wavelength from time of flight
  HMap* m_per[MAX_CH+NGRP]; // period
  HMap* m_width[MAX_CH+NGRP]; // width
  HMap* m_width2[MAX_CH+NGRP]; // width
  //HMap* m_width3[MAX_CH+NGRP]; // width

  HMap* m_pulse[MAX_CH]; // pulse shape
  HMap* m_deriv[MAX_CH]; // pulse shape (deriv)

  HMap* m_axay[MAX_CH*(MAX_CH+1)/2];
  HMap* m_area_base[MAX_CH];
  HMap* m_area_sl1[MAX_CH];
  HMap* m_area_sl2[MAX_CH];
  HMap* m_slope_12[MAX_CH];
  //HMap* m_time_simul[MAX_CH];
  HMap* m_area_time[MAX_CH];
  HMap* m_area_width[MAX_CH];
  HMap* m_area_width2[MAX_CH];
  HMap* m_area_ntof[MAX_CH];
  HMap* m_prof[256];
  HMap* m_prof64[6];
  //HMap* m_prof_y[1];
  //HMap* m_prof_ax[1];
  //HMap* m_prof_ay[1];

  //HMap* m_area_width3[MAX_CH];
  //HMap* m_width_12[MAX_CH];


  //std::vector<int> vcuts; //vector of



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

  TList* map_list; //list of basic maps (excluding cuts) 
  TList* allmap_list; //list of all maps (including cuts)
  TList* dir_list; //list of folders

  Long64_t T_prev[MAX_CH];

  Bool_t wfalse;

  //profilometer:
  std::vector<int> px;
  std::vector<int> py;
  std::vector<int> ax;
  std::vector<int> ay;
  HMap *h_p,*h_a;
  int h_off; // 1 or 33 
  int h_xy; // 0: X; 1: Y; -1: ???
  int ch_alpha;
  Float_t h_sum[2][64]; //[xy][pos]
  
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
  void Make_axay(const char* dname, const char* name, const char* title,
	       HMap* map[], Hdef* hd, Hdef* hd1); //, int nmax);
  void Make_prof(const char* dname,const char* name,
		 const char* title,HMap* map[],Hdef* hd);
  void Make_prof64(const char* dname, HMap* map[], HMap* map2[],
		    Hdef* hd, Hdef* hd2);

  void Make_Mult(const char* dname, const char* name, const char* title,
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
