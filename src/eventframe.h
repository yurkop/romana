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

//#include "TThread.h"
#include <list>

//-----------------------------------------------
class EventFrame: public TGCompositeFrame {

protected:
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

   TGLabel                *fLabel2;

   TGCheckButton          *fDeriv[2];
   TGCheckButton          *fPeak[10];
   TGCheckButton          *fChn[MAX_CH+1];
   //TGRadioButton          *fChn[MAX_CH+1];

   TGStatusBar            *fStat1;
   TGStatusBar            *fStat2;
   
   TGLayoutHints          *fLay1;
   TGLayoutHints          *fLay2;
   TGLayoutHints          *fLay3;
   TGLayoutHints          *fLay4;
   TGLayoutHints          *fLay5;
   TGLayoutHints          *fLay6;
   TGLayoutHints          *fLay7;

 public:
   TRootEmbeddedCanvas    *fCanvas;

 public:

   int ntab; //tab number where eventframe is placed
   int ievent; //current event to be drawn
   std::list<EventClass>::iterator d_event;
   std::list<EventClass> Tevents; //local event list, consist of one element
   // used for online monitoring
   std::list<EventClass> *Levents; //points either to Tevents or to crs->Levents
   //EventClass *d_event; //current event to draw
   int ndiv;
   int divtype[3];//0: pulse; 1: deriv; 2: 2nd deriv
   //int NGr;
   TGraph Gr[3][MAX_CH];
   TH1F *hh[3][MAX_CH];
   double gx1,gx2,gy1[3],gy2[3];
   TH2F fPaint[3];
   THStack* hst[3];
   //TMultiGraph* mgr[3];

   //bool bprint;

   //TThread *trd;

public:
   EventFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
   virtual ~EventFrame();

   //void StartThread();

   void Clear();
   //void Start();
   //void DoReset();
   void FillHstack(int dr);
   void DrawEvent2();
   void DrawPeaks(double y1,double y2);
   void DrawEvent();
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
