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
  byte cmd; //for Command_crs
  byte chan; //for Command_crs
};

//-----------------------------------------------
class ParDlg: public TGCompositeFrame {

protected:

  ULong_t tcol[ADDCH];

  // TGLayoutHints* fL0;
  // TGLayoutHints* fL1;
  // TGLayoutHints* fL2;
  // TGLayoutHints* fL3;
  // TGLayoutHints* fL4;
  // TGLayoutHints* fL5;
  // TGLayoutHints* fL6;
  // TGLayoutHints* fLexp;

  TGCanvas* fCanvas;
  TGCompositeFrame* fcont1;

  int nfld; //number of fields in a line

  int id_write[3]; //id of different *write* text fields

public:
  std::vector<pmap> Plist;

  TGHorizontalFrame *cframe[MAX_CH+ADDCH];
  TGTextEntry* clab[MAX_CH+ADDCH];

public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg();

  void DoMap(TGWidget *f, void *d, P_Def t, int all, void *d2=0);

  void SetNum(pmap pp, Double_t num);
  void SetChk(pmap pp, Bool_t num);
  void SetCombo(pmap pp, Int_t num);
  void SetTxt(pmap pp, const char* txt);
  void DoNum();
  void DoChk();
  void DoCombo();
  void DoTxt();
  void DoChkWrite();
  void DoOpen();
  void CopyParLine(int sel, int line);
  void CopyField(int from, int to);
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
		   double min2=0, double max2=0, char* connect=NULL);
  void AddLine_txt(TGGroupFrame* frame, int width, char* opt_fname, 
		   const char* tip1, const char* label);
  void AddLine_hist(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  void AddLine_2d(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  // void AddLine_hist(TGGroupFrame* frame, Bool_t* b1,
  // 		    Float_t *x1, Float_t *x2, Float_t *x3, 
  // 		    const char* tip, const char* label);
  void AddLine_mean(TGGroupFrame* frame, Hdef* hd,
		    const char* tip, const char* label);
  void AddWrite(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
		Int_t* compr, char* opt_fname);
  void AddFiles(TGCompositeFrame* frame);
  void AddHist(TGCompositeFrame* frame);
  void AddOpt(TGCompositeFrame* frame);
  void AddLogic(TGCompositeFrame* frame);
  void AddAna(TGCompositeFrame* frame);
  void DoParNum();
  void DoNum_SetBuf();
  void DoCheckHist();
  void DoCheckPulse();
  void CheckFormula();
  void Update();

  ClassDef(ParParDlg, 0)
};

//-----------------------------------------------
class ChanParDlg: public ParDlg {

protected:
  TGCanvas* fCanvas2;
  TGCompositeFrame* fcont2;
  TGHorizontalFrame *head_frame;

public:
  ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ChanParDlg() {};

  void AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
	       void* apar, double min, double max, P_Def ptype);
  void DoChanMap(TGWidget *f, void *d, P_Def t, int all, byte cmd, byte chan,
	     void* d2=0);
  void DoChanNum();

  ClassDef(ChanParDlg, 0)
};

//-----------------------------------------------
class CrsParDlg: public ChanParDlg {

public:
  int trig;
  TGTextEntry *fStat[MAX_CH+1];
  TGTextEntry *fStatBad[MAX_CH+1];

public:
  CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~CrsParDlg() {};

  void Make_crspar(const TGWindow *p,UInt_t w,UInt_t h);
  void AddHeader();
  void AddLine_crs(int i, TGCompositeFrame* fcont1);
  void AddNumCrs(int i, int kk, int all, TGHorizontalFrame *hframe1,
	       const char* name, void* apar, void* apar2=0);
  void ResetStatus();
  void UpdateStatus();
  void DoCrsNum();
  void DoCheck();

  ClassDef(CrsParDlg, 0)
};

//-----------------------------------------------
class AnaParDlg: public ChanParDlg {

public:
  AnaParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~AnaParDlg() {};

  void Make_AnaPar(const TGWindow *p,UInt_t w,UInt_t h);

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

  void Make_PikPar(const TGWindow *p,UInt_t w,UInt_t h);

  void AddHeader();
  void AddLine_Pik(int i, TGCompositeFrame* fcont1);

  ClassDef(PikParDlg, 0)
};

#endif
