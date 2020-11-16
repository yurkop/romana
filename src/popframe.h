#ifndef popframe_H
#define popframe_H 1

#include "common.h"
#include <TText.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>

//-----------------------------------------------
class PopFrame {

public:
  //void** fVar;
  TGTransientFrame *fMain;   // main frame of this widget
  TGHorizontalFrame *pframe[11];

  TGHorizontalFrame *hframe;
  TGLabel* fLabel;
  TGNumberEntry* fNum;
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

public:
  PopFrame(const TGWindow *main, UInt_t w, UInt_t h, Int_t menu_id);
  virtual ~PopFrame();
  void AddProf(UInt_t w, UInt_t h);
  void AddEcalibr();
  void AddTcalibr();
  void DoProfTime();
  void AddPeaks();
  void DoENum();

  void   CloseWindow();
  //ClassDef(PopFrame, 0)
};

#endif
