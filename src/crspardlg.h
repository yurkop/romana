#ifndef crspardlg_H
#define crspardlg_H 1

#include "common.h"
#include "hclass.h"
#include <TGFrame.h>
#include <TGCanvas.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGNumberEntry.h>
#include <TGStatusBar.h>
#include <TGSplitter.h>
//#include <TGSplitFrame.h>
//#include <TGListBox.h>
#include <TGDockableFrame.h>

//#define p_fnum 1
//#define p_inum 2
//#define p_chk 3
//#define p_cmb 4
//#define p_txt 5

//typedef unsigned char byte;
//typedef std::list<TGFrame*> wlist;

enum P_Def {
  p_null,
  p_fnum,
  p_inum,
  p_chk,
  p_cmb,
  p_txt,
  p_but
};

struct pmap {
  TGFrame* field; //address of the input widget
  void* data; //address of the parameter
  void* data2; //address of the second (parallel) parameter
  P_Def type; //p_fnum p_inum p_chk p_cmb p_txt
  char all; //1 - all/ALL/* parameters; >1 - channel type
  //UChar_t cmd; //опции (биты)
  UInt_t cmd; //опции (биты)
  //0x1: (bit0) 1: start/stop DAQ
  //0xE: (bit1-3) change color
  //0xF0: (bit4-7) Action (1..15)
  //0x100 (bit8) disble during acq
  //0x200 (bit9) disble fields not existing in certain devices
  //0x400 (bit1) enable/disble fields for ntof analysis

  //Action: 
  // in DoDaqNum:
  // 1 - DoReset
  // 2 - Hireset
  // 3 - Hi->Update()
  // // 4 - match Trg & Drv for CRS2 (not realized)
  // 5 - group4
  // 6 - проверка Len кратно 3 или 4
  // 7 - установка tsleep в Timer

  // in DoDaqChk:
  // 5 - group4 for module51

  /*
  // in UpdateField (chk):
  // 2 - 2d hist (2 fields)
  // 3 - 1d hist (4 fields)
  // 4 - profilometer hist
  */

  //UChar_t chan; //for Command_crs :seems to be not needed (21.01.2020)
};

typedef std::vector<pmap>::iterator piter;
//-----------------------------------------------
class TrigFrame: public TGHorizontalFrame {
public:
  TrigFrame(TGGroupFrame *p, int opt=0);
  virtual ~TrigFrame() {};

  void DoCheckTrigger();
  void UpdateTrigger();

  TGCheckButton *fchkTrig[3];
  
};
//-----------------------------------------------
class ParDlg: public TGCompositeFrame {

public:

  TGLayoutHints* LayCC0 ;
  TGLayoutHints* LayCC0a;
  TGLayoutHints* LayCC1 ;
  TGLayoutHints* LayCC2 ;
  TGLayoutHints* LayET3 ;
  TGLayoutHints* LayLC1 ;
  TGLayoutHints* LayLC2 ;
  TGLayoutHints* LayLT0 ;
  TGLayoutHints* LayLT1 ;
  TGLayoutHints* LayLT1a ;
  TGLayoutHints* LayLT2 ;
  TGLayoutHints* LayLT3 ;
  TGLayoutHints* LayLT4 ;
  TGLayoutHints* LayLT4a ;
  TGLayoutHints* LayLT5 ;
  TGLayoutHints* LayLT6 ;
  TGLayoutHints* LayLE0 ;
  TGLayoutHints* LayEE0 ;
  TGLayoutHints* LayEE1 ;
  TGLayoutHints* LayRC1 ;


  //int jtrig;
  bool notbuilt;
  int pmax; //максимальный канал (вместо MAX_CH), который записан в параметрах

  TGDockableFrame        *fDock;

protected:

  ULong_t tcol[MAX_TP+3]; //MAX_TP, other, copy, swap
  TGComboBox* fCombo[MAX_CH+1]; //MAX_CH, all
  //TGTextEntry* cname[MAX_TP];
  TGHorizontalFrame *cframe[MAX_CHTP];
  TGTextEntry* clab[MAX_CHTP];
  TGTextButton* cbut;
  Int_t cbut_id;

  TGCanvas* fCanvas1;
  TGCompositeFrame* fcont1;

  int nfld; //number of fields in a line

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  TGNumberFormat::EStyle k_chk;
  TGNumberFormat::EStyle k_lab;

public:
  std::vector<pmap> Plist;
  std::vector<UChar_t> Clist; //list of isanable states for daqdisable

public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg();

  void DoMap(TGFrame *f, void *d, P_Def t, int all, UInt_t cmd=0, void *d2=0);

  void SetNum(pmap pp, Double_t num);
  void SetChk(pmap pp, Bool_t num);
  void SetCombo(pmap pp, Int_t num);
  void SetTxt(pmap pp, const char* txt);
  bool Chk_all(int all, int i);
  void DoNum();
  void DoAct(int id, int intbool, Double_t fnum);
  void DoDaqNum();
  void DoChk(Bool_t on);
  void DoDaqChk(Bool_t on);
  void DoCheckHist(Bool_t on);
  void DoCombo();
  // void DoCombo2(Event_t*);
  void DoTxt();
  void DoOneType(int i);
  void DoTypes();
  void DoOpen();
  void DoAll();
  void CopyParLine(int sel, int line);
  void CopyField(int from, int to);
  void DoColor(pmap* pp, Float_t val);
  void UpdateField(int nn);
  void Update();
  void EnableField(int nn, bool state);
  void AllEnabled(bool state);
  void DaqDisable();
  void DaqEnable();
  //void SelectEnabled(bool state, const char* text);
  TGFrame *FindWidget(void* p);
  void AddClab(TGCompositeFrame* cfr, TGTextEntry* &clb,
	       int i, const char* txt, int &kk, int t);
  void AddChname(TGCompositeFrame* cfr, int i7, int &kk, int c);


  void Check_opt(TGHorizontalFrame *hfr1, int width, void* x1,
		 const char* tip1, UInt_t cmd1, const char* cname="");

  void Num_opt(TGHorizontalFrame *hfr1, int width, void* x1,
	       const char* tip1, TGNumberFormat::EStyle style1,
	       double min1, double max1, UInt_t cmd1, TGLayoutHints* Lay);

  void AddLine_opt(TGCompositeFrame* frame, int width, void *x1, void *x2, 
		   const char* tip1, const char* tip2, const char* label,
		   TGNumberFormat::EStyle style1, 
		   TGNumberFormat::EStyle style2,
		   //TGNumberFormat::EAttribute attr, 
		   double min1=0, double max1=0,
		   double min2=0, double max2=0, UInt_t cmd1=0, UInt_t cmd2=0,
		   TGLayoutHints* Lay1=0, TGLayoutHints* Lay2=0);

  void AddLine_1opt(TGCompositeFrame* frame, int width, void *x1, 
		   const char* tip1, const char* label,
		   TGNumberFormat::EStyle style1, 
		   double min1=0, double max1=0,
		   UInt_t cmd1=0, TGLayoutHints* Lay1=0);

  // void Rebuild();

  ClassDef(ParDlg, 0)
};

//-----------------------------------------------
class ParParDlg: public ParDlg {
public:
  ParParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParParDlg() {};

protected:

  //TGCheckButton *fchkSoft;
  //TGCheckButton *fchkHard;

  TGCheckButton *fchkTrig[3];

  //wlist soft_list;
  //wlist hard_list;

  const char* tip1;
  const char* tip2;
  const char* label;

public:
  TrigFrame* tTrig;

public:

  void AddChk(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
	      Int_t* compr, Bool_t* rflag);
  int AddFiles(TGCompositeFrame* frame);
  //void AddHist(TGCompositeFrame* frame);
  int AddOpt(TGCompositeFrame* frame);
  int AddNtof(TGCompositeFrame* frame);
  int AddLogic(TGCompositeFrame* frame);
  //void AddTrigger(TGGroupFrame* frame);
  int AddExpert(TGCompositeFrame* frame);
  int AddSimul(TGCompositeFrame* frame);
  void DoCheckNtof(Bool_t on);

  //void Add2d();

  void Update();
  //void UpdateLL(wlist &llist, Bool_t state);
  //void UpdateTrigger();

  ClassDef(ParParDlg, 0)
};

//-----------------------------------------------
class HistParDlg: public ParDlg {
public:
  HistParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~HistParDlg() {};

protected:

  TGGroupFrame* frame1d;
  TGGroupFrame* frame2d;
  //std::list<TGHorizontalFrame*> h2_frame;

  TGComboBox *cmb1,*cmb2;  
  //TGNumberEntry *cNum1, *cNum2;

  //TGTextEntry* tTrig;
  TList list2d;

public:

  void AddHist(TGCompositeFrame* frame2);
  void AddHist_2d();
  //void RemHist_2d(TGCompositeFrame* frame2);
  void AddLine_hist(TGGroupFrame* frame, Mdef* md);
  void AddLine_mean(TGHorizontalFrame *hfr1, Mdef* md);
  void Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
		    P_Def pp, UInt_t cmd, const char* tip);
  void AddLine_prof(TGHorizontalFrame *hfr1, Mdef* md);
  void AddLine_prof_int(TGHorizontalFrame *hfr1, Mdef* md);
  void AddLine_2d(TGGroupFrame* frame, Mdef* md);
  //, Hdef* hd,
  //const char* tip, const char* label, int type);
  void Add2d();
  void Rem2d();
  void Update();

  ClassDef(HistParDlg, 0)
};

//-----------------------------------------------
class ChanParDlg: public ParDlg {

public:
  TGCanvas* fCanvas0;
  TGCanvas* fCanvas2;
  TGHSplitter *hsplitter;
  TGCompositeFrame* fcont2;
  TGHorizontalFrame *head_frame;

public:
  ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ChanParDlg() {};

  void AddChCombo(int i, int &id, int &kk, int &all);
  void AddChkPar(int &kk, TGHorizontalFrame *cframe,
		 Bool_t* dat, int all, const char* ttip, UInt_t cmd=0);
  void AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
    void* apar, double min, double max, P_Def ptype, UInt_t cmd=0);
  // void ClearLines();

  ClassDef(ChanParDlg, 0)
};

//-----------------------------------------------
class DaqParDlg: public ChanParDlg {

public:
  TGTextEntry *fStat2[MAX_CH+1];
  TGTextEntry *fStat3[MAX_CH+1];
  TGTextEntry *fStatBad[MAX_CH+1];

  //TGCheckButton *fchkSoft;
  //TGCheckButton *fchkHard;

  TGGroupFrame* cGrp;
  //TGLabel *cLabel;
  TrigFrame* tTrig;

public:
  DaqParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~DaqParDlg() {};

  void Build();
  void AddHeader();
  void AddLine_daq(int i, TGCompositeFrame* fcont1);
  void AddNumDaq(int i, int kk, int all, TGHorizontalFrame *hframe1,
    const char* name, void* apar, void* apar2=0, UInt_t cmd=1);
  void AddStat_daq(TGTextEntry* &fStat, TGHorizontalFrame* &cframe,
		   const char* ttip, int &kk);
  void UpdateStatus(int rst=0);
  void Update();

  ClassDef(DaqParDlg, 0)
};

//-----------------------------------------------
class AnaParDlg: public ChanParDlg {

public:
  AnaParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~AnaParDlg() {};

  void Build();

  void AddHeader();
  void AddLine_Ana(int i, TGCompositeFrame* fcont1);
  //void DoChanNum();

  ClassDef(AnaParDlg, 0)
};

//-----------------------------------------------
class PikParDlg: public ChanParDlg {

public:
  PikParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~PikParDlg() {};

  void Build();

  void AddHeader();
  void AddLine_Pik(int i, TGCompositeFrame* fcont1);

  ClassDef(PikParDlg, 0)
};

//--------------------------------------
class ErrFrame: public ParDlg {
public:
  TGTextEntry* fErr[MAX_ERR];
  int errflag;

  //routines
  ErrFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ErrFrame();

  void Reset();
  void ErrUpdate();

  ClassDef(ErrFrame, 0)
};

#endif
