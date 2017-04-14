#ifndef crspardlg_H
#define crspardlg_H 1

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
  P_Def type; //p_fnum p_inum p_chk p_cmb p_txt
  char all; //1 - all parameters, >1 - channel type
};

//-----------------------------------------------
class ParDlg: public TGCompositeFrame {

 protected:

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

 public:
  std::vector<pmap> Plist;

 public:
  ParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~ParDlg() {};

  void DoMap(TGWidget *f, void *d, P_Def t, int all);

  void SetNum(pmap pp, Double_t num);
  void SetChk(pmap pp, Bool_t num);
  void SetCombo(pmap pp, Int_t num);
  void SetTxt(pmap pp, const char* txt);
  void DoNum_SetBuf();
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
  TGWidget *FindWidget(void* p);

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

  TGNumberFormat::EStyle k_int;
  TGNumberFormat::EStyle k_r0;
  TGNumberFormat::EStyle k_r1;
  TGNumberFormat::EStyle k_r2;
  TGNumberFormat::EStyle k_r3;

 public:
  /*
  void AddLine3(TGCompositeFrame* frame, int width, void *x1, void *x2, 
		const char* tip1, const char* tip2, const char* label,
		TGNumberFormat::EStyle style, 
		//TGNumberFormat::EAttribute attr, 
		double min=0, double max=0);
  */
  void AddLine3(TGGroupFrame* frame, int width, void *x1, void *x2, 
		const char* tip1, const char* tip2, const char* label,
		TGNumberFormat::EStyle style, 
		//TGNumberFormat::EAttribute attr, 
		double min=0, double max=0, char* connect=NULL);
  void AddWrite(const char* txt, Bool_t* opt_chk, char* opt_fname);
  void AddHist(TGCompositeFrame* frame);
  void AddOpt(TGCompositeFrame* frame);
  void AddAna(TGCompositeFrame* frame);
  //void Update();

};

//-----------------------------------------------
class CrsParDlg: public ParDlg {
  //class CrsParDlg {

 protected:

 public:
  CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~CrsParDlg() {};

  void Make_crspar(const TGWindow *p,UInt_t w,UInt_t h);
  void Make_chanpar(const TGWindow *p,UInt_t w,UInt_t h);

  //void AddNum(int id, int k, 
  //	      TGHorizontalFrame *hframe1, const char* name);
  void AddHeader1();
  void AddHeader2();
  void AddLine1(int i);
  void AddLine2(int i);
  void AddNum1(int i, int kk, int all, TGHorizontalFrame *hframe1,
	      const char* name, void* apar);
  void AddNum2(int i, int kk, int all, TGHorizontalFrame *hframe1,
	       void* apar, double min, double max, P_Def ptype);
  //void DoPar(int k, int i, Long_t num);
  //void *MapPar(int id);
  //void Update();

  ClassDef(CrsParDlg, 0)
    };

#endif
