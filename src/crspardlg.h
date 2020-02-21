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
  p_open
};

struct pmap {
  TGWidget* field; //address of the input widget
  void* data; //address of the parameter
  void* data2; //address of the second (parallel) parameter
  P_Def type; //p_fnum p_inum p_chk p_cmb p_txt
  char all; //1 - all parameters, >1 - channel type
  byte cmd; //for Command_crs (1 - start/stop crs; 0 - do nothing)
  //byte chan; //for Command_crs :seems to be not needed (21.01.2020)
};

//-----------------------------------------------
class ParDlg: public TGCompositeFrame {

public:
  int jtrig;
  bool notbuilt;
  int pmax; //максимальный канал (вместо MAX_CH), который записан в параметрах

  TGDockableFrame        *fDock;

protected:

  //MAX_TP [MAX_TP+1] - Other
  //MAX_TP+1 [MAX_TP+2] - Copy
  ULong_t tcol[MAX_TP+1];
  TGComboBox* fCombo[MAX_CH+1];
  TGTextEntry* cname[MAX_TP];

  TGCanvas* fCanvas1;
  TGCompositeFrame* fcont1;

  int nfld; //number of fields in a line

  //int id_write[3]; //id of different *write* text fields

public:
  std::vector<pmap> Plist;

  TGHorizontalFrame *cframe[MAX_CH+MAX_TP+1];
  TGTextEntry* clab[MAX_CH+MAX_TP+1];

public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg();

  void DoMap(TGWidget *f, void *d, P_Def t, int all, byte cmd=0, void *d2=0);

  void SetNum(pmap pp, Double_t num);
  void SetChk(pmap pp, Bool_t num);
  void SetCombo(pmap pp, Int_t num);
  void SetTxt(pmap pp, const char* txt);
  void DoNum();
  void DoChk();
  void DoCombo(bool cp);
  void DoCombo2(Event_t*);
  void DoTxt();
  void DoTypes();
  void DoChkWrite();
  void DoOpen();
  void CopyParLine(int sel, int line);
  void CopyField(int from, int to);
  void NumField1(int nn, bool bb);
  void UpdateField(int nn);
  void Update();
  void EnableField(int nn, bool state);
  void AllEnabled(bool state);
  void SelectEnabled(bool state, const char* text);
  TGWidget *FindWidget(void* p);
  void Rebuild();

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

  int id_usb;
  //int id_tstop;

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  TGNumberFormat::EStyle k_r2;
  TGNumberFormat::EStyle k_r3;

  TGTextEntry* tTrig;

public:
  /*
    void AddLine3(TGCompositeFrame* frame, int width, void *x1, void *x2, 
    const char* tip1, const char* tip2, const char* label,
    TGNumberFormat::EStyle style, 
    //TGNumberFormat::EAttribute attr, 
    double min=0, double max=0);
  */
  void AddLine_opt(TGGroupFrame* frame, int width, void *x1, void *x2, 
		   const char* tip1, const char* tip2, const char* label,
		   TGNumberFormat::EStyle style1, 
		   TGNumberFormat::EStyle style2, 
		   //TGNumberFormat::EAttribute attr, 
		   double min1=0, double max1=0,
		   double min2=0, double max2=0, int iconnect=0);
  // void AddLine_txt(TGGroupFrame* frame, int width, char* opt_fname, 
  // 		   const char* tip1, const char* label);
  void AddLine_hist(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  void AddLine_2d(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  // void AddLine_prof(TGGroupFrame* frame, Hdef* hd,
  // 		    const char* tip, const char* label);



  // void AddLine_hist(TGGroupFrame* frame, Bool_t* b1,
  // 		    Float_t *x1, Float_t *x2, Float_t *x3, 
  // 		    const char* tip, const char* label);
  void AddLine_mean(TGHorizontalFrame *hfr1, Hdef* hd,
		    const char* tip, const char* label);
  void Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
		    P_Def pp, const char* tip);
  void AddLine_prof(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  // void AddWrite(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
  // 		Int_t* compr, char* opt_fname);
  void AddChk(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
	      Int_t* compr, Bool_t* rflag);
  void AddFiles(TGCompositeFrame* frame);
  void AddHist(TGCompositeFrame* frame);
  void AddOpt(TGCompositeFrame* frame);
  void AddLogic(TGCompositeFrame* frame);
  void AddAna(TGCompositeFrame* frame);
  void DoParNum();
  void DoParNum_Daq();
  void DoNum_SetBuf();
  void DoCheckHist();
  void DoCheckPulse();
  void DoCheckProf();
  void DoNumProf();
  //void CheckFormula();

  void Add2d();

  void Update();

  ClassDef(ParParDlg, 0)
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
  //void DoChanMap(TGWidget *f, void *d, P_Def t, int all, byte cmd, //byte chan, void* d2=0);
  void DoChanNum();
  void AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
	       void* apar, double min, double max, P_Def ptype);
  void ClearLines();

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
	       const char* name, void* apar, void* apar2=0);
  //void ResetStatus();
  void UpdateStatus(int rst=0);
  void DoDaqNum();
  void DoCheck();

  //void Update();
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
