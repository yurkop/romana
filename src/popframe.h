#ifndef popframe_H
#define popframe_H 1

#include "common.h"
#include <TGLabel.h>
#include <TGListBox.h>
#include <TGNumberEntry.h>
#include <TGTextEdit.h>
#include <TPolyLine.h>
#include <TRootEmbeddedCanvas.h>
#include <TText.h>

// class Calibr_Lines {
// };

// struct ee_map {
//   double ee,cc; //energy, channel
//   int iroi; //number of roi
// };

class HMap;
class PClass {
public:
  void *var;
  int ivar;
  int min = 0, max = 2, step = 1;
  bool bb = false;
  // int zz[3] = {0,2,1}; //min,max,step
  // int mmin,mmax;
  std::string name;
};

typedef std::vector<PClass> PVect;

class PSet {
public:
  PVect pv;
  std::vector<double> res;
};

//-----------------------------------------------
class PopFrame {

public:
  TGLayoutHints *LayLC0, *LayLC1, *LayLC2, *LayCC3, *LayCC4, *LayEE2, *LayCB1,
      *LayCT1, *LayBut1, *LayBut1a, *LayBut2, *LayBut3;

  MENU_COM m_id;
  void *ptr;

  TGTransientFrame *fMain; // main frame of this widget
  TGTextEdit *fEdit;       // text edit widget

  TGListBox *fListBox;

  TList chklist;

  TGHorizontalFrame *hframe;
  TGLabel *fLabel;
  TGNumberEntry *fWidth, *fRange, *fSubstr, *fNum;
  TGNumberEntryField *fAdj[MAX_CH + NGRP + 1][3];

  TGNumberFormat::EStyle ki = TGNumberFormat::kNESInteger;
  TGNumberFormat::EStyle kr = TGNumberFormat::kNESReal;
  TGNumberFormat::EAttribute ka = TGNumberFormat::kNEAAnyNumber;
  TGNumberFormat::ELimit kn = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit kl = TGNumberFormat::kNELLimitMinMax;

  TRootEmbeddedCanvas *fCanvas;
  TText txt;
  Int_t npeaks;
  // Double_t ee[10];
  Double_t time_ref = 0;
  Int_t delay;
  Int_t n_iter;

  Float_t E0[MAX_CHTP]; // [emult0]
  Float_t E1[MAX_CHTP]; // [emult]
  Float_t E2[MAX_CHTP]; // [emult2]
  Float_t adj[MAX_CH + NGRP + 1][3];
  Int_t calibr_t[MAX_CHTP]; // type of calibration

  Float_t local_sD[MAX_CHTP];

  PSet ps_all;
  PSet ps;
  std::vector<PSet> pss;

public:
  PopFrame(const TGWindow *main, UInt_t w, UInt_t h, MENU_COM menu_id,
           void *p = 0);
  virtual ~PopFrame();
  void AddProfTime(UInt_t w, UInt_t h);
  void AddAdj(TGCompositeFrame *fcont1, HMap *map, int i);
  void AddNum(double val, int id, const char *label, const char *tip = 0);
  void AddChk(bool val, int id, const char *label, const char *tip = 0);
  void AddEcalibr(UInt_t w, UInt_t h);
  void AddTcalibr();
  void AddPeaks();
  void AddOnePar(TGCompositeFrame *fcont1, int id, void *var, const char *nm,
                 int mmin, int mmax);
  void AddOptPar();
#ifdef CYUSB
  void AddDevice();
#endif
  // #ifdef YUMO
  //   void AddYumo();
  // #endif
  void AddTest();
  void DoPar();
  void DoAdj();
  void DoENum();
  void Do_Save_Ecalibr();
  void Do_Auto_Ecalibr();
  void Do_Ecalibr();
  void E_Update();
  void Do_EApply();
  void Do_Default();
  void Do_Revert();
  // void Do_Tcalibr();
  void Do_TApply();
  void Do_TRevert();
  void DoOpt(UInt_t it);
  void Do_OptPar();
  void Do_Test();

  void CloseWindow();
  // ClassDef(PopFrame, 0)
};

#endif
