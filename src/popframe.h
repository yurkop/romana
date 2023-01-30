#ifndef popframe_H
#define popframe_H 1

#include "common.h"
#include <TText.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGTextEdit.h>
#include <TPolyLine.h>

// class Calibr_Lines {
// };

// struct ee_map {
//   double ee,cc; //energy, channel
//   int iroi; //number of roi
// };

class HMap;
//-----------------------------------------------
class PopFrame {

public:

  TGLayoutHints *LayLC0,*LayLC2,*LayEE2,*LayBut1,*LayBut2;

  void* ptr;

  TGTransientFrame *fMain;   // main frame of this widget
  TGTextEdit       *fEdit;   // text edit widget

  TList chklist;

  TGHorizontalFrame *hframe;
  TGLabel* fLabel;
  TGNumberEntry *fWidth, *fRange, *fSubstr, *fNum;
  TGNumberEntryField *fAdj[MAX_CH+NGRP+1][3];

  TGNumberFormat::EStyle ki=TGNumberFormat::kNESInteger;
  TGNumberFormat::EStyle kr = TGNumberFormat::kNESReal;
  TGNumberFormat::EAttribute ka = TGNumberFormat::kNEAAnyNumber;
  TGNumberFormat::ELimit kn = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit kl = TGNumberFormat::kNELLimitMinMax;

  TRootEmbeddedCanvas  *fCanvas;
  TText txt;
  Int_t npeaks;
  Int_t npol;
  Double_t fwhm;
  Double_t range;
  Double_t ee[10];

  Float_t E0[MAX_CHTP]; // [emult0]
  Float_t E1[MAX_CHTP]; // [emult]
  Float_t E2[MAX_CHTP]; // [emult2]
  Float_t adj[MAX_CH+NGRP+1][3];
  Int_t calibr_t[MAX_CHTP]; //type of calibration

  Float_t sD[MAX_CHTP];

public:
  PopFrame(const TGWindow *main, UInt_t w, UInt_t h, Int_t menu_id, void* p=0);
  virtual ~PopFrame();
  void AddProfTime(UInt_t w, UInt_t h);
  void AddAdj(TGCompositeFrame* fcont1, HMap* map, int i);
  void AddEcalibr(UInt_t w, UInt_t h);
  void AddTcalibr();
  void AddSimul();
  void DoAdj();
  void DoENum();
  void Do_Save_Ecalibr();
  void Do_Auto_Ecalibr();
  void Do_Ecalibr();
  void E_Update();
  void Do_EApply();
  void Do_Default();
  void Do_Revert();
  void Do_Tcalibr();
  void Do_TApply();

  void CloseWindow();
  //ClassDef(PopFrame, 0)
};

#endif
