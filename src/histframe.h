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
#include <TGDockableFrame.h>

//#include "TThread.h"
#include <list>

class HMap; //forward declaration
void DynamicExec();

//-----------------------------------------------
class HistFrame: public TGCompositeFrame {

public:
  TGDockableFrame        *fDock;

  TGCanvas               *gCanvas; //hist Gcanvas
  TGCanvas               *gCanvas2; //cuts Gcanvas
  TRootEmbeddedCanvas    *fEc;

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc
  TGHorizontalFrame      *fHor3; //for cuts
  TGVerticalFrame        *fVer0; //canvas && hslider



  TGDoubleHSlider        *fHslider;
  TGDoubleVSlider        *fVslider;


  TGListTree             *fListTree;    // list tree with histograms
  TGListTree             *fCutTree;    // list tree with histograms

  TGListTreeItem         *iWork;
  TGListTreeItem         *iWork_cut[MAXCUTS];
  TGListTreeItem         *iWork_MT;
  TList* hlist;
  //THStack* hstack;
  TList* st_list;    //stack list
  TH1F* st_plot;    //histogram for plotting stack;

  TGCheckButton* chknorm;
  TGCheckButton* chklog;
  TGCheckButton* chkstat;
  TGCheckButton* chkgcuts;
  //static const int NR=7;
  //TGRadioButton *Rb[NR];
  TGRadioButton *Rb[2];

  TGTextEntry* tForm;

  int ntab; //tab number where eventframe is placed

  Bool_t wrk_check[MAXCUTS+1]; //is work* checked before deleting ltree
  //work,work_cut[MAXCUTS]
  Bool_t wrk_check_MT; //is work_MT checked before deleting ltree
  Bool_t changed;
  Bool_t started;
  Bool_t in_gcut;
  int np_gcut; //number of points in gcut

  //TLine line;

  int ndiv;
  //int xdiv;
  //int ydiv;
  TObject *padmap[MAX_PADS];

public:
  HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt);
  virtual ~HistFrame();

  TGListTreeItem* Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open=0, const TGPicture *closed=0);
  void Make_Ltree();
  void Clear_Ltree();
  TGListTreeItem* FindItem(const char* name);
  void Clone_Ltree(HMap* hmap);
  void DoClick(TGListTreeItem* item,Int_t but);
  void CutClick(TGListTreeItem* item,Int_t but);
  void DoCheck(TObject* obj, Bool_t check);
  void DoNorm();
  void DoLog();
  void DoStat();
  void DoKey(TGListTreeItem* entry, UInt_t keysym);
  //void SelectDiv(int nn);
  void DoNum();
  void DoRadio();
  void DoButton();
  void DoSlider();
  void GetHMinMax(TH1* hh, double x1, double x2,
		  double &y1, double &y2);
  void X_Slider(TH1* hh, double &a1, double &a2);
  void Y_Slider(TH1* hh, double a1, double a2, double y1, double y2);
  void AddCutG(TPolyLine *pl, TObject* hobj);
  void MakeCutG(TPolyLine *pl, TObject* hobj);
  void DoCutG();
  void AddFormula();
  //void DoFormula();
  void ShowCutG();
  void ClearCutG();
  void DoPeaks();
  void Do_Ecalibr();
  void HiReset();
  void Update();
  void DrawHist();
  void DrawStack();
  void ReDraw();
  //void DataDropped(TGListTreeItem *, TDNDData *data);

  ClassDef(HistFrame, 0)
};

void *trd_handle(void* ptr);


#endif
