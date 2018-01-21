#ifndef histframe_H
#define histframe_H 1

#include "common.h"
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGraph.h>
#include "TGDoubleSlider.h"
#include "TGDoubleSlider.h"
#include <TG3DLine.h>
#include <TH2.h>
#include <TMultiGraph.h>
#include <TGStatusBar.h>
#include <TGListTree.h>
//#include <TCutG.h>

//#include "TThread.h"
#include <list>

/*
//-----------------------------------------------
class MECanvas: public TRootEmbeddedCanvas {
 public:
 MECanvas(const char* name = 0, const TGWindow* p = 0, UInt_t w = 10, 
	  UInt_t h = 10, UInt_t options = kSunkenFrame|kDoubleBorder,
	  Pixel_t back = GetDefaultFrameBackground())
   : TRootEmbeddedCanvas(name,p,w,h,options,back) {};


  //~TECanvas() {};

  Bool_t HandleDNDDrop(TDNDData *data);

};
*/
//-----------------------------------------------
class HistFrame: public TGCompositeFrame {

 public:
   TGCanvas               *fCanvas;
   TRootEmbeddedCanvas    *fEc;
   //MECanvas    *fEc;
   TGListTree             *fListTree;    // list tree with histograms
   TList* hlist;
   //std::list<TH1*> hlist2;   

   static const int NR=7;
   TGRadioButton *Rb[NR];

   int ntab; //tab number where eventframe is placed

   Bool_t changed;
   Bool_t started;

   int ndiv;
   int xdiv;
   int ydiv;

 public:

   //int ievent; //current event to be drawn
   //std::list<EventClass>::iterator d_event;
   //std::list<EventClass> Tevents;
   //std::list<EventClass> *Levents;
   //EventClass *d_event; //current event to draw
   //int divtype[3];//0: pulse; 1: deriv; 2: 2nd deriv
   //int NGr;
   //TGraph Gr[3][MAX_CH];
   //TH1F *hh[3][MAX_CH];
   //double gx1,gx2,gy1[3],gy2[3];
   //TH2F fPaint[3];
   //TMultiGraph* mgr[3];

   //bool bprint;

   //TThread *trd;

public:
   HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
   virtual ~HistFrame();

   //void Upd()  { Emit("Upd()"); }   // *SIGNAL*

   //void NameTitle();
   //void Make_hist();
   TGListTreeItem* Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open=0, const TGPicture *closed=0, Bool_t checkbox = kTRUE);
   void Make_Ltree();
   void Clear_Ltree();
   //void NewBins();
   //void Reset_hist();
   //void FillHist(EventClass* evt);
   void DoClick(TGListTreeItem* item,Int_t but);
   //void DoClick(TGListTreeItem*, Int_t, UInt_t, Int_t, Int_t);
   void DoCheck(TObject* obj, Bool_t check);
   void DoLog();
   //void DoKey(TGListTreeItem* entry, UInt_t keysym);
   void SelectDiv(int nn);
   void DoRadio();
   void DoButton();
   //void DoPeaks2();
   void AddCutG();
   void ShowCutG();
   void ClearCutG();
   void DoPeaks();
   void DoReset();
   void Update();
   void DrawHist();
   void ReDraw();
   void DataDropped(TGListTreeItem *, TDNDData *data);

   ClassDef(HistFrame, 0)
};


void *trd_handle(void* ptr);


#endif
