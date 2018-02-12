#ifndef histframe_H
#define histframe_H 1

#include "common.h"
//#include "hclass.h"
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
#include <TPolyLine.h>
//#include <TCutG.h>

//#include "TThread.h"
#include <list>

class HMap; //forward declaration
void DynamicExec();

//-----------------------------------------------
class HistFrame: public TGCompositeFrame {

 public:
  TGCanvas               *gCanvas; //hist Gcanvas
  TGCanvas               *gCanvas2; //cuts Gcanvas
   TRootEmbeddedCanvas    *fEc;
   TGListTree             *fListTree;    // list tree with histograms
   TGListTree             *fCutTree;    // list tree with histograms

   TGListTreeItem         *iWork;
   TGListTreeItem         *iWork_cut[MAXCUTS];
   TList* hlist;

   TGCheckButton* chklog;
   TGCheckButton* chkgcuts;
   static const int NR=7;
   TGRadioButton *Rb[NR];

   int ntab; //tab number where eventframe is placed

   Bool_t changed;
   Bool_t started;
   Bool_t in_gcut;
   int np_gcut; //number of points in gcut

   //TLine line;

   int ndiv;
   int xdiv;
   int ydiv;
   TObject *padmap[MAX_PADS];

 public:
   HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
   virtual ~HistFrame();

   TGListTreeItem* Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open=0, const TGPicture *closed=0);
   void Make_Ltree();
   void Clear_Ltree();
   TGListTreeItem* FindItem(TGListTree* lTree, const char* name);
   void Clone_Ltree(HMap* hmap);
   void DoClick(TGListTreeItem* item,Int_t but);
   void CutClick(TGListTreeItem* item,Int_t but);
   void DoCheck(TObject* obj, Bool_t check);
   void DoLog();
   void DoKey(TGListTreeItem* entry, UInt_t keysym);
   void SelectDiv(int nn);
   void DoRadio();
   void DoButton();
   void AddCutG(TPolyLine *pl, TObject* hobj);
   void MakeCutG(int icut, TPolyLine *pl, TObject* hobj);
   void DoCutG();
   void ShowCutG();
   void ClearCutG();
   void DoPeaks();
   void HiReset();
   void Update();
   void DrawHist();
   void ReDraw();
   void DataDropped(TGListTreeItem *, TDNDData *data);

   ClassDef(HistFrame, 0)
};


void *trd_handle(void* ptr);


#endif
