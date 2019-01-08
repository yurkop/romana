#ifndef eventframe_H
#define eventframe_H 1

#include "common.h"
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGraph.h>
#include "TGDoubleSlider.h"
#include "TGDoubleSlider.h"
#include <TG3DLine.h>
#include <TH2.h>
#include <THStack.h>
#include <TMultiGraph.h>
#include <TGStatusBar.h>
#include <TGTextEntry.h>
#include <TGDockableFrame.h>

//#include "TThread.h"
#include <list>

//-----------------------------------------------
class EventFrame: public TGCompositeFrame {

public:

  TGDockableFrame        *fDock;

protected:

  Int_t chcol[MAX_CH];
  ULong_t gcol[MAX_CH];
  //ULong_t fcol[MAX_CH];
  TGVertical3DLine       *separator1;
  TGHorizontalFrame      *fHor;
  TGHorizontalFrame      *fHor2;

  TGVerticalFrame        *fVer0; //contains canvas, Hslider, fHor1
  TGVerticalFrame        *fVer_d; //contains deriv etc
  TGVerticalFrame        *fVer_ch; //contains fGroupCh

  
  //TGHorizontalFrame      *fHor_ch; //contains fVer[..]
  //TGVerticalFrame        *fVer[(MAX_CH-1)/16+1];
  //TGVerticalFrame        *fVer_ch2;

  TGHorizontalFrame      *fHor_but; //contains buttons
  TGVerticalFrame        *fVer_st; //status etc //contains fHor_st + statusbar
  TGHorizontalFrame      *fHor_st; //contains fVer_d, fVer_ch

  TGGroupFrame           *fGroupCh; //contains channels: frCh[i]
  TGHorizontalFrame      *frCh[MAX_CH+1];
  TGCheckButton          *fDeriv[3];
  TGCheckButton          *fPeak[16];
  TGCheckButton          *fChn[MAX_CH+1];

  //TGTextButton           *freset;
  //TGTextButton           *f1buf;
  //TGTextButton           *fNbuf;
  TGTextButton           *fOne;
  TGTextButton           *fNev;
  TGTextButton           *fmOne;
  TGTextButton           *fmNev;
  TGTextButton           *fFirst;
  TGTextButton           *fLast;
  TGTextButton           *fChk1;
  TGTextButton           *fChk2;
  TGTextEntry            *tEnt;
  
  TGDoubleHSlider        *fHslider;
  TGDoubleVSlider        *fVslider;

  //TGLabel                *fLabel2;

  //TGRadioButton          *fChn[MAX_CH+1];

  static const int        nstat=4;
  TGTextEntry            *fStat[nstat];
  //TGStatusBar            *fStat2[4];

  TFormula               *formula;

  TGLayoutHints          *fLay1;
  TGLayoutHints          *fLay2;
  TGLayoutHints          *fLay3;
  TGLayoutHints          *fLay4;
  TGLayoutHints          *fLay5;
  TGLayoutHints          *fLay6;
  TGLayoutHints          *fLay7;
  TGLayoutHints          *fLay8;
  TGLayoutHints          *fLay9;
  TGLayoutHints          *fLay16;


public:
  TRootEmbeddedCanvas    *fCanvas;

public:

  int ntab; //tab number where eventframe is placed
  int ievent; //current event to be drawn
  std::list<EventClass>::iterator d_event;
  std::list<EventClass> Tevents; //local event list, consists of one element
  // used for online monitoring
  std::list<EventClass> *Pevents; //points either to Tevents or to crs->Levents
  //EventClass *d_event; //current event to draw
  int ndiv;
  //int divtype[3];//0: pulse; 1: deriv; 2: 2nd deriv
  //int NGr;
  TH1F* fHist[3];
  TGraph *Gr[3][MAX_CH];
  double gx1[MAX_CH],gx2[MAX_CH],gy1[3][MAX_CH],gy2[3][MAX_CH];
  double mx1,mx2,my1[3],my2[3];
  //TH1F *histo[3][MAX_CH];
  //double gx1,gx2,gy1[3],gy2[3];
  //TH2F fPaint[3];
  //THStack* hst[3];
  //TMultiGraph* mgr[3];

  //bool bprint;

  //TThread *trd;

public:
  EventFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
  virtual ~EventFrame();

  void ShPtr(int zz);
  void Rebuild();
  void AddCh();
  void DoReset();
  void FillGraph(int dr);
  void SetRanges(int dr);
  void DrawEvent2();
  void DrawPeaks(int dr, PulseClass* pulse, double y1,double y2);
  void DoGraph(int ndiv, int deriv);
  void ReDraw();
  void Plus1();
  void Minus1();
  void PlusN();
  void MinusN();
  void First();
  void Last();
  void DoCheckPoint();
  void DoFormula();
  void DoSlider();
  void DoChkDeriv();
  void DoChkPeak();
  void DoPulseOff();
  void DoUndock();

  ClassDef(EventFrame, 0)
};


void *trd_handle(void* ptr);


#endif
