#ifndef crspardlg_H
#define crspardlg_H 1

#include "common.h"
#include <TGFrame.h>
#include <TGCanvas.h>
#include <TGLabel.h>
#include <TGComboBox.h>
#include <TGNumberEntry.h>
#include <TGStatusBar.h>
#include <TGSplitFrame.h>
//#include <TGListBox.h>
#include <TGDockableFrame.h>

//#define p_fnum 1
//#define p_inum 2
//#define p_chk 3
//#define p_cmb 4
//#define p_txt 5

typedef unsigned char byte;

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
  TGWidget* field; //address of the input widget
  void* data; //address of the parameter
  void* data2; //address of the second (parallel) parameter
  P_Def type; //p_fnum p_inum p_chk p_cmb p_txt
  char all; //1 - all/ALL/* parameters; >1 - channel type
  byte cmd; //опции (биты)
  //0x1: (bit0) 1: start/stop DAQ
  //0xE: (bit1-3) change color

  //0xF0: (bit4-7) action:

  // in DoDaqNum:
  // 1 - DoReset
  // 2 - Hireset
  // 3 - Hi->Update()
  // 5 - group4

  // in DoDaqChk:
  // 5 - group4

  // in UpdateField:
  // 2 - 2d hist (2 fields)
  // 3 - 1d hist (3 fields)
  // 4 - profilometer hist


  //byte chan; //for Command_crs :seems to be not needed (21.01.2020)
};

//-----------------------------------------------
class ParDlg: public TGCompositeFrame {

public:

  TGLayoutHints* LayCC0 ;
  TGLayoutHints* LayCC0a;
  TGLayoutHints* LayCC1 ;
  TGLayoutHints* LayET3 ;
  TGLayoutHints* LayLT0 ;
  TGLayoutHints* LayLT2 ;
  TGLayoutHints* LayLT3 ;
  TGLayoutHints* LayLT4 ;
  TGLayoutHints* LayLC1 ;
  TGLayoutHints* LayLT5 ;
  TGLayoutHints* LayLE0 ;
  TGLayoutHints* LayEE0 ;


  int jtrig;
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

public:
  std::vector<pmap> Plist;

public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg();

  void DoMap(TGWidget *f, void *d, P_Def t, int all, byte cmd=0, void *d2=0);

  void SetNum(pmap pp, Double_t num);
  void SetChk(pmap pp, Bool_t num);
  void SetCombo(pmap pp, Int_t num);
  void SetTxt(pmap pp, const char* txt);
  bool Chk_all(int all, int i);
  void DoNum();
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
  void UpdateField(int nn);
  void Update();
  void EnableField(int nn, bool state);
  void AllEnabled(bool state);
  void SelectEnabled(bool state, const char* text);
  TGWidget *FindWidget(void* p);
  // void Rebuild();

  ClassDef(ParDlg, 0)
};

//-----------------------------------------------
class ParParDlg: public ParDlg {
public:
  ParParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParParDlg() {};

protected:

  //TGHorizontalFrame *hor;
  //TGVerticalFrame *ver1;
  //TGVerticalFrame *ver2;

  TGSplitFrame *hor;
  TGSplitFrame *ver1;
  TGSplitFrame *ver2;
  
  TGGroupFrame* frame1d;
  TGGroupFrame* frame2d;

  const char* tip1;
  const char* tip2;
  const char* label;
  //void *x1,*x2;

  //int id_read;
  //int id_tstop;

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  //TGNumberFormat::EStyle k_r2;
  //TGNumberFormat::EStyle k_r3;

  TGNumberFormat::EStyle k_mon;

  TGTextEntry* tTrig;

public:
  /*
    void AddLine3(TGCompositeFrame* frame, int width, void *x1, void *x2, 
    const char* tip1, const char* tip2, const char* label,
    TGNumberFormat::EStyle style, 
    //TGNumberFormat::EAttribute attr, 
    double min=0, double max=0);
  */

  void One_opt(TGHorizontalFrame *hfr1, int width, void* x1,
    const char* tip1, TGNumberFormat::EStyle style1,
    double min1, double max1, byte cmd1);

  void AddLine_opt(TGGroupFrame* frame, int width, void *x1, void *x2, 
   const char* tip1, const char* tip2, const char* label,
   TGNumberFormat::EStyle style1, 
   TGNumberFormat::EStyle style2, 
		   //TGNumberFormat::EAttribute attr, 
   double min1=0, double max1=0,
   double min2=0, double max2=0, byte cmd1=0, byte cmd2=0);
  // void AddLine_hist(TGGroupFrame* frame, Hdef* hd,
  //   const char* tip, const char* label);
  // void AddLine_2d(TGGroupFrame* frame, Hdef* hd,
  //   const char* tip, const char* label, int type);
  // void AddLine_mean(TGHorizontalFrame *hfr1, Hdef* hd,
  //   const char* tip, const char* label);
  // void Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
  //   P_Def pp, const char* tip);
  // void AddLine_prof(TGGroupFrame* frame, Hdef* hd,
  //   const char* tip, const char* label);
  void AddChk(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
   Int_t* compr, Bool_t* rflag);
  void AddFiles(TGCompositeFrame* frame);
  //void AddHist(TGCompositeFrame* frame);
  void AddOpt(TGCompositeFrame* frame);
  void AddLogic(TGCompositeFrame* frame);
  void AddAna(TGCompositeFrame* frame);
  // void DoCheckPulse();

  void Add2d();

  void Update();

  ClassDef(ParParDlg, 0)
};

//-----------------------------------------------
class HistParDlg: public ParDlg {
public:
  HistParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~HistParDlg() {};

protected:

  TGSplitFrame *hor;
  TGSplitFrame *ver1;
  TGSplitFrame *ver2;
  
  TGGroupFrame* frame1d;
  TGGroupFrame* frame2d;

  const char* tip1;
  const char* tip2;
  const char* label;

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  TGNumberFormat::EStyle k_mon;

  TGTextEntry* tTrig;

public:

  void AddHist(TGCompositeFrame* frame2);
  void AddLine_hist(TGGroupFrame* frame, Hdef* hd,
    const char* tip, const char* label);
  void AddLine_2d(TGGroupFrame* frame, Hdef* hd,
    const char* tip, const char* label, int type);
  void AddLine_mean(TGHorizontalFrame *hfr1, Hdef* hd,
    const char* tip, const char* label);
  void Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
    P_Def pp, const char* tip);
  void AddLine_prof(TGGroupFrame* frame, Hdef* hd,
    const char* tip, const char* label);
  void Add2d();
  void Update();

  ClassDef(HistParDlg, 0)
};

//-----------------------------------------------
class ChanParDlg: public ParDlg {

public:
  TGCanvas* fCanvas2;
  TGCompositeFrame* fcont2;
  TGHorizontalFrame *head_frame;

public:
  ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ChanParDlg() {};

  void AddChCombo(int i, int &id, int &kk, int &all);
  void AddChkPar(int &kk, TGHorizontalFrame *cframe,
		 Bool_t* dat, int all, const char* ttip, int cmd=0);
  void AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
    void* apar, double min, double max, P_Def ptype, byte cmd=0);
  // void ClearLines();

  ClassDef(ChanParDlg, 0)
};

//-----------------------------------------------
class DaqParDlg: public ChanParDlg {

public:
  TGTextEntry *fStat2[MAX_CH+1];
  TGTextEntry *fStat3[MAX_CH+1];
  TGTextEntry *fStatBad[MAX_CH+1];

public:
  DaqParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~DaqParDlg() {};

  void Build();
  void AddHeader();
  void AddLine_daq(int i, TGCompositeFrame* fcont1);
  void AddNumDaq(int i, int kk, int all, TGHorizontalFrame *hframe1,
    const char* name, void* apar, void* apar2=0, byte cmd=1);
  void AddStat_daq(TGTextEntry* &fStat, TGHorizontalFrame* &cframe,
		   const char* ttip);
  void UpdateStatus(int rst=0);

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
