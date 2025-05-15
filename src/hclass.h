#ifndef hclass_H
#define hclass_H 1

#include "common.h"
#include "pulseclass.h"
#include "libmana.h"
#include <TH1.h>
#include <TH2.h>
#include <TFormula.h>
#include <TGraphErrors.h>
#include <bitset>

class HMap; //forward declaration

//-----------------------------------------------
class Mdef {
  // класс содержит указатель на Hdef
  // (сам Hdef должен быть создан в другом месте);
  // общее имя для всех гистограмм данного типа;
  // вектор указателей на мапы гистограмм (N штук).
  // сами указатели создаются при генерации гистограмм
  // N [=v_map.size()] автоматически увеличивается при добавлении нового map
public:
  Int_t hnum=0;
  // =1..49 - standard 1d hist
  // =51 - mean pulses
  // =52 - mean deriv
  // =53 - mean FFT
  // =61 - prof
  // =62 - prof_int
  // >100 && <10000 - 2d hist

  size_t ptr=0; //offset of PulseClass member for VarPulse

  Hdef *hd; // *hd должен быть создан и удален в другом месте
  TString h_name, name, x_title, y_title, tip;

  Mdef *mx=0,*my=0;
  std::vector <HMap*> v_map;
public:

  virtual ~Mdef() {};

  Float_t VarPulse(EventClass* e, PulseClass* p){return *(Float_t*)((char*)p + ptr);}
  Float_t  VarTime(EventClass* e, PulseClass* p);
  Float_t  VarRate(EventClass* e, PulseClass* p);
  Float_t  VarPeriod(EventClass* e, PulseClass* p);
  Float_t  VarNtof(EventClass* e, PulseClass* p);
  Float_t  VarEtof(EventClass* e, PulseClass* p);
  Float_t  VarLtof(EventClass* e, PulseClass* p);

  Float_t Def(EventClass* e, PulseClass* p) {return -99999;};
  Float_t (Mdef::*GetX)(EventClass* e, PulseClass* p) = &Mdef::Def;

  void Time_Extend(UChar_t ch, Double_t T);
  void Time_Extend_2d(UChar_t ch, Double_t xx, Double_t yy);

  void Fill_01(HMap* map, Float_t x, Double_t *hcut_flag, int ncut);
  void Fill_02(HMap* map, Float_t x, Float_t y, Double_t *hcut_flag,
	       int ncut);

  void Fill_1d(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_1d_Extend(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_2d(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_2d_Extend(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_axay(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_HWRate(EventClass* evt, Double_t *hcut_flag, int ncut);
  void FillMult(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_Mean1(HMap* map, Float_t* Data, int nbins, int ideriv, int ncut);
  void Fill_FFT(HMap* map, Float_t* Data, int nbins, int ch, int ncut);
  void FillMeanPulse(EventClass* evt, Double_t *hcut_flag, int ncut);
  void Fill_Ampl(EventClass* evt, Double_t *hcut_flag, int ncut);
  void FillProf(EventClass* evt, Double_t *hcut_flag, int ncut);
#ifdef YUMO
  void FillYumo(EventClass* evt, Double_t *hcut_flag, int ncut);
#endif

  //void Fill_1d_cut(EventClass* evt, int i);
  //void Fill_1d_Extend_cut(EventClass* evt, Double_t *hcut_flag);
  //void Fill_2d_cut(EventClass* evt, Double_t *hcut_flag);
  //void Fill_2d_Extend_cut(EventClass* evt, Double_t *hcut_flag);
  //void FillMult_cut(EventClass* evt, int i);
  //void FillMeanPulse_cut(EventClass* evt, Double_t *hcut_flag);


  void MFillDef(EventClass* evt, Double_t *hcut_flag, int ncut) {
    //prnt("sss;",BRED, "Def:", RST);
  };
  void (Mdef::*MFill)(EventClass* evt, Double_t *hcut_flag, int ncut) = &Mdef::MFillDef;

  //void MFDef_cut(EventClass* evt, int i) {};
  //void (Mdef::*MFill_cut)(EventClass* evt, int i) = &Mdef::MFDef_cut;

  bool is2d(); //true if 2D (100<hnum<10000)
  ClassDef(Mdef, CDEF)
};

typedef std::vector<Mdef> mdef_list; //было std::list
typedef std::vector<Mdef*> mdef_p_list; //было std::list
typedef mdef_list::iterator mdef_iter;

//-----------------------------------------------
class HMap: public TNamed {
  //Name: histogram name
  //Title: folder name
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

  //ULong64_t Nent; //number of entries for mean pulses & deriv

  //Bool_t notcut = true; // false - if this map is a cut

  //UChar_t flg; // 0 - normal histogram, 1 - cut histogram
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

  //char ngrp=0;
  //std::bitset<NGRP> grp;


  ClassDef(HMap, 0)
};

//-----------------------------------------------
class HClass {

 public:

  //size_t p_ptr[8]; //offsets of PulseClass members
  // Base;   0
  // Area0;  1
  // Area;   2
  // Slope1; 3
  // Slope2; 4
  // Height; 5
  // Width;  6
  // Time;   7

  //static const int MAX_ING=256;

  //std::vector<int> vcuts; //vector of
  Mdef *md_prof;
  Mdef *md_prof_int;
  Mdef* md_pulse;


  TCutG* cutG[MAXCUTS];
  //Name: cut[i]
  //Title: histogram name on which this cut is made
  //nr of points: 1 - formula; 2 - 1d; >2 - 2d
  //Double_t cut_flag[MAXCUTS]; //признак срабатывания окна
  char cuttitle[MAXCUTS][99];
  int cutcolor[MAXCUTS];
  TFormula* cform[MAXCUTS]; //starts from 1, not from 0
  bool b_formula; //at least one cut formula exists
  bool b_base[MAX_CHTP]; // использовать Base,Sl1,Sl2,RMS1,RMS2

  //HMap *cutmap[MAXCUTS];

  TList* map_list; //list of basic maps (excluding cuts)
  //map_list - owner: при удалении map_list удаляются все члены 
  TList* allmap_list; //list of all maps (including cuts)
  //allmap_list - not owner: при удалении allmap_list члены не удаляются
  TList* dir_list; //list of folders
  //dir_list - owner: при удалении dir_list удаляются все члены 

  mdef_list Mlist; // содержит все существующие Mdef (содержащие Hdef)
  //в т.ч. несозданных гистограмм
  //создается при инициализации; может дополняться в Add_h2
  //кроме гистограмм, считанных из root файла

  mdef_p_list MFill_list; // содержит указатели! Mdef* всех
  // активных (созданных) гистограмм (у которых hd.b=1) -
  // для которых вызывается MFill.
  // Заполняется в Make_hist

  mdef_p_list Mainlist; // содержит указатели! Mdef* всех гистограмм в Main
  // список нужен для заполнения cut-гистограмм в FillHist
  //заполняется в ??? (Ana_Start???)

  mdef_list MFilelist; // содержит (не указатели!) Mdef гистограмм
  // из файлов


  TVirtualFFT* fft[MAX_CH];

 public:

  HClass();
  virtual ~HClass();

  void Make_Mlist();
  Mdef* Add_h2(int id1, int id2);
  mdef_iter Add_file(const char *name);
  void Make_hist();

  mdef_iter Find_Mdef(int id);
  void MapHist(mdef_iter md, TH1* hh, int i);
  void HHist1(mdef_iter md, TH1* &hh, char* name, char* title, int i);
  void HHist2(mdef_iter md, TH1* &hh, char* name, char* title, int i,
	      int n1, float min1, float max1, int n2, float min2, float max2);
  void Make_1d(mdef_iter md, int maxi);
  void Make_1d_pulse(mdef_iter md);
  void Make_prof(mdef_iter md);
  void Make_prof_int(mdef_iter md, Hdef* hd2);
  int Make_2d(mdef_iter md);

#ifdef YUMO
  Mdef *md_yumo2, *md_yumo1;
  //int yumo_x1,yumo_x2,yumo_y1,yumo_y2; //channels for x1,x2,y1,y2;
  std::vector<int> yumo_xy;
  void Yumo_xy();
  void Make_Yumo_2d(mdef_iter md);
  void Make_Yumo_1d(mdef_iter md);
#endif

  void FillHist(EventClass* evt, Double_t *hcut_flag);

  void Clone_Hist(HMap* map);
  void Remove_Clones(HMap* map);
  void Make_cuts();

   ClassDef(HClass, 0)
};

#endif
