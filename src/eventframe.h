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

//#include "TThread.h"
#include <list>

//-----------------------------------------------
class EventFrame: public TGCompositeFrame {

protected:
  Float_t RGB[MAX_CH][3] =
    {
      {0,0,0},//0
      {1,0,0},
      {0,1,0},
      {0,0,1},
      {0,1,1},
      {1,0,1}, //5
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

  Int_t chcol[MAX_CH];
  ULong_t gcol[MAX_CH];
  ULong_t fcol[MAX_CH];

  //TGTransientFrame       *fMain;
   TGVertical3DLine       *separator1;
   TGHorizontalFrame      *fHor;
   TGHorizontalFrame      *fHor2;

   //TGTextButton           *freset;
   TGTextButton           *f1buf;
   TGTextButton           *fNbuf;
   TGTextButton           *fOne;
   TGTextButton           *fNev;
   TGTextButton           *fmOne;
   TGTextButton           *fmNev;
   TGTextButton           *fFirst;
   TGTextButton           *fLast;

   TGDoubleHSlider        *fHslider;
   TGDoubleVSlider        *fVslider;

   //TGLabel                *fLabel2;

   TGCheckButton          *fDeriv[3];
   TGCheckButton          *fPeak[16];
   TGCheckButton          *fChn[MAX_CH+1];
   //TGRadioButton          *fChn[MAX_CH+1];

   static const int        nstat=4;
   TGTextEntry            *fStat[nstat];
   TGStatusBar            *fStat2[4];

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

   //void StartThread();

   //void Clear();
   //void Start();
   void DoReset();
   void FillGraph(int dr);
   void SetRanges(int dr);
   //void FillHist(int dr);
   //void FillHstack(int dr);
   void DrawEvent2();
   void DrawPeaks(int dr, PulseClass* pulse, double y1,double y2);
   //void DrawEvent();
   void DoGraph(int ndiv, int deriv);
   void ReDraw();
   /* void DoNum(); */
   /* void Do1buf(); */
   /* void DoNbuf(); */
   //void DoColor();
   void Plus1();
   void Minus1();
   void PlusN();
   void MinusN();
   void First();
   void Last();
   void DoSlider();
   void DoChkDeriv();
   void DoChkPeak();
   void DoPulseOff();
   //void DoDraw(int);
   //void CloseWindow();

  ClassDef(EventFrame, 0)
};


void *trd_handle(void* ptr);


#endif
