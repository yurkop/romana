#ifndef eventframe_H
#define eventframe_H 1

#include "common.h"
#include "pulseclass.h"
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
#include <TFormula.h>
#include <TVirtualFFT.h>

//#include "TThread.h"
#include <list>

namespace EF {
  const Float_t RGB[32][3] = 
    {
      {0,0,0},//0
      {1,0,0},
      {0,1,0},
      {0,0,1},
      {1,0,1},
      {0,1,1}, //5
      {0.63,0.67,0.02},
      {0.71,0.11,0.52},
      {0.51,0.95,0.46},
      {0.16,0.71,0.77},
      {0.77,0.96,0.16},//10
      {0.68,0.61,0.98},
      {0.15,0.45,0.74},
      {0.03,0.62,0.07},
      {0.47,0.37,0.50},
      {0.72,0.34,0.04},//15
      {0.51,0.54,0.59},
      {0.52,0.78,0.11},
      {0.01,0.86,0.66},
      {0.78,0.69,0.14},
      {0.91,0.65,0.90},//20
      {0.16,0.30,0.29},
      {0.41,0.69,0.03},
      {0.77,0.22,0.80},
      {0.40,0.37,0.98},
      {0.56,0.06,0.34},//25
      {0.89,0.64,0.79},
      {0.24,0.99,0.28},
      {0.84,0.89,0.10},
      {0.12,0.25,0.18},
      {0.97,0.85,0.74},//30
      {0.55,0.29,0.20}
    };
}

//-----------------------------------------------
class EventFrame: public TGCompositeFrame {

public:

  TGDockableFrame        *fDock;

  Int_t chcol[MAX_CH+NGRP];
  ULong_t gcol[MAX_CH+NGRP];

  static const int NDIV=5; //количество делений окна Events (3 или 4)
  //static const int NGR=5; //количество графов -> перенесено в common.h
  //static const int NGR=6; //количество графов (всегда 6!!!) <- почему?
  //Gr[4] Gr[5] - 

protected:

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
  TGCheckButton          *fDeriv[NDIV];
  TGCheckButton          *fPeak[MXPK];
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
  TGTextButton           *fPrint;
  
  TGDoubleHSlider        *fHslider;
  TGDoubleVSlider        *fVslider;

  //TGLabel                *fLabel2;

  //TGRadioButton          *fChn[MAX_CH+1];

  static const int        nstat=4;
  TGTextEntry            *fStat[nstat];
  //TGStatusBar            *fStat2[4];

  TFormula               *formula;

  TVirtualFFT *fftr2c;

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
  TH1F* fHist[NDIV];
  TGraph *Gr[NGR][MAX_CH];
  double gx1[MAX_CH],gx2[MAX_CH],gy1[NGR][MAX_CH],gy2[NGR][MAX_CH];
  double mx1,mx2,my1[NDIV],my2[NDIV];
  //TH1F *histo[3][MAX_CH];
  //double gx1,gx2,gy1[3],gy2[3];
  //TH2F fPaint[3];
  //THStack* hst[3];
  //TMultiGraph* mgr[3];

  //bool bprint;

  //TThread *trd;

  TText ttxt;
  int tx,ty;

public:
  EventFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
  virtual ~EventFrame();

  bool fChn_on(int ch);
  void ShPtr(int zz);
  void Rebuild();
  void ChkpkUpdate();
  void AddCh();
  void DoReset();
  void FillDeriv1(int dr, int i, PulseClass* pulse, double dt);
  void FillGraph(int dr);
  void SetXRanges();
  void SetYRanges(int dr, double x1, double x2);
  void DrawEvent2();
  void DrawPeaks(int dr, int j, PulseClass* pulse, double y1,double y2);
  void DrawShapeTxt(PulseClass* pulse);
  void DrawProf(int i, double y1, double y2);
  void ReDraw();
  void Plus1();
  void Minus1();
  void PlusN();
  void MinusN();
  void First();
  void Last();
  void DoCheckPoint();
  void DoPrintE();
  void DoPrintP();
  void DoFormula();
  void DoSlider();
  void DoChkDeriv();
  void DoChkPeak();
  void DoPulseOff();
  void DoUndock();

  ClassDef(EventFrame, 0)
};


//void *trd_handle(void* ptr);


#endif
