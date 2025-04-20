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
#include <TG3DLine.h>

#include <TGSlider.h>

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

typedef std::vector<Pmap>::iterator piter;
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
  //TGLayoutHints* LayCC1a ;
  TGLayoutHints* LayCC2 ;
  TGLayoutHints* LayET0 ;
  TGLayoutHints* LayET1 ;
  TGLayoutHints* LayLC1 ;
  TGLayoutHints* LayLC2 ;
  TGLayoutHints* LayLT0 ;
  TGLayoutHints* LayLT1 ;
  TGLayoutHints* LayLT1a ;
  TGLayoutHints* LayLT1b ;
  TGLayoutHints* LayLT2 ;
  TGLayoutHints* LayLT3 ;
  TGLayoutHints* LayLT4 ;
  TGLayoutHints* LayLT4a ;
  TGLayoutHints* LayLT5 ;
  TGLayoutHints* LayLT6 ;
  TGLayoutHints* LayLT7 ;
  TGLayoutHints* LayLE0 ;
  TGLayoutHints* LayEE0 ;
  TGLayoutHints* LayEE1 ;
  TGLayoutHints* LayRT0 ;
  //TGLayoutHints* LayL1 ;

  //TGLayoutHints* LayLB ;
  TGLayoutHints* LayTrig ;


  //int jtrig;
  //bool notbuilt;
  //int pmax; //максимальный канал (вместо MAX_CH), который записан в параметрах

  TGDockableFrame        *fDock;

protected:

  ULong_t tcol[MAX_TP+3]; //MAX_TP, other, copy, swap
  TGComboBox* fCombo[MAX_CH+1]; //MAX_CH, all
  //TGTextEntry* cname[MAX_TP];
  //TGHorizontalFrame *cframe[MAX_CHTP];
  TGHorizontalFrame *hparl[3][MAX_CHTP]; // горизонтальные фреймы в chanpar; должно заменить cframe
  TGTextEntry* clab[MAX_CHTP]; //содержит номер канала
  TGTextButton* cbut; // кнопка ALL/all/*
  Int_t cbut_id;

  TGCanvas* fCanvas1;
  TGCompositeFrame* fcont1;

  int nfld; //number of fields in a line

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  TGNumberFormat::EStyle k_r2;
  TGNumberFormat::EStyle k_chk;
  TGNumberFormat::EStyle k_lab;

public:
  std::vector<Pmap> Plist;
  std::vector<UChar_t> Clist; //list of isanable states for daqdisable

public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg();

  void DoMap(TGFrame *f, void *d, P_Def t, int all, UInt_t cmd=0, void *d2=0,
	     UShort_t off=0, UChar_t step=1);

  void SetNum(Pmap pp, UShort_t off, Double_t num);
  void SetChk(Pmap pp, UShort_t off, Bool_t num);
  void SetCombo(Pmap pp, UShort_t off, Int_t num);
  void SetTxt(Pmap pp, const char* txt);
  bool Chk_all(int all, int i);
  void DoNum();
  void DoAct(int id, UShort_t off, Double_t fnum, bool dq);
  void DoDaqNum();
  void DoChk(Bool_t on);
  void DoDaqChk(Bool_t on);
  void DoCheckHist(Bool_t on);
  void DoCombo();
  //void DoCombo2();
  // void DoCombo2(Event_t*);
  void DoTxt();
  void DoOneType(int i);
  void DoTypes();
  void DoOpen();
  void DoAll();
  void ColorLine(int line, ULong_t col);
  void CopyParLine(int sel, int index, int line);
  void CopyField(Pmap* pp, int from, int to);
  void DoColor(Pmap* pp, Float_t val);
  void UpdateField(int nn);
  void UpdateColumn(int id);
  void Update();
  void EnableField(int nn, bool state);
  void AllEnabled(bool state);
  void DaqDisable();
  void DaqEnable();
  //void SelectEnabled(bool state, const char* text);
  TGFrame *FindWidget(void* p);

  void Check_opt(TGHorizontalFrame *hfr1, int width, void* x1,
		 const char* tip1, UInt_t cmd1, const char* cname="");

  void Num_opt(TGHorizontalFrame *hfr1, int width, void* x1, void* x1a,
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

  void AddLine_1opt(TGCompositeFrame* frame, int width, void *x1, void* x1a,
		   const char* tip1, const char* label,
		   TGNumberFormat::EStyle style1, 
		   double min1=0, double max1=0,
		   UInt_t cmd1=0, TGLayoutHints* Lay1=0);

  // void Rebuild();

  ClassDef(ParDlg, 0)
};

//-----------------------------------------------
class ChanParDlg: public ParDlg {

public:
  // TGTextEntry *fStat2[MAX_CH+1];
  // TGTextEntry *fStat3[MAX_CH+1];
  // TGTextEntry *fStatBad[MAX_CH+1];

  TGHorizontal3DLine *hsep[3];
  TGHorizontalFrame *head_frame[3];

  TGHorizontalFrame *hforce[3];
  TGCheckButton *fforce;

  //TGCheckButton *fchkSoft;
  //TGCheckButton *fchkHard;

  TGCanvas* fCnv[3];

  TGCanvas* fCanvas0;
  TGCanvas* fCanvas2;
  TGHSplitter *hsplitter;
  TGCompositeFrame* fcont2;

  TGGroupFrame* cGrp;
  //TGLabel *cLabel;
  TrigFrame* tTrig;

  TGVScrollBar* vscroll;
  TGHScrollBar*	hscroll;
  
  Int_t oldscroll;
  Int_t nrows; //actual nrows calculated from opt.Nrows and opt.Nchan


public:
  ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ChanParDlg() {};

  void Build();
  void BuildColumns(int jj);
  void AddColumn(int jj, int kk, int ii, P_Def pdef,
		 int wd, int daq, double min, double max, const char* pname,
		 void* apar=0, void* apar2=0, UInt_t cmd=1, int s2=0);
  //void AddHeader();
  void AddChan(int j, int kk, int wd, int all, TGHorizontalFrame *hfr,
	       UShort_t off=0);
  void AddCombo(int j, int wd, int all, TGHorizontalFrame *hfr);
  void AddChkPar(int kk, int wd, int all, int daq, TGHorizontalFrame *hfr, void* apar, UShort_t off=0, UInt_t cmd=1, void* apar2=0, UChar_t step=1);
  void AddNumPar(int i, int kk, int wd, int all, int daq, P_Def pdef, double min, double max, TGHorizontalFrame *hfr, const char* name, void* apar, UShort_t off=0, UInt_t cmd=1, void* apar2=0);
  void AddStatDaq(int jj,int kk,int wd,
		  void* apar,TGHorizontalFrame* hfr);
  // void AddStatDaq(int kk, int wd, TGTextEntry* &fStat,
  // 		   TGHorizontalFrame* hfr);
  //void AddChCombo(int i, int &id, int &kk, int &all);
  void UpdateStatus(int rst=0);
  void DoScroll(int pos);
  void HandleMouseWheel(Event_t *event);
  void Update();

  ClassDef(ChanParDlg, 0)
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
  void AddFileName(TGCompositeFrame* frame);
  int AddFiles(TGCompositeFrame* frame);
  //void AddHist(TGCompositeFrame* frame);
  int AddOpt(TGCompositeFrame* frame);
  int AddNtof(TGCompositeFrame* frame);
  int AddLogic(TGCompositeFrame* frame);
  //void AddTrigger(TGGroupFrame* frame);
  void AddLine_dec_format(TGCompositeFrame* frame, int width);
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

  TGVerticalFrame* frame1d[2];
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
  void AddLine_hist(TGCompositeFrame* frame, Mdef* md);
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
