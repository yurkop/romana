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

typedef std::vector<double> dvect;
typedef std::vector<dvect> d2vect;

//-----------------------------------------------
class HistFrame: public TGCompositeFrame {

public:
  TGDockableFrame        *fDock;

  TGCanvas               *gCanvas; //hist Gcanvas
  TGCanvas               *gCanvas2; //cuts Gcanvas
  TGCanvas               *gCanvas3; //roi Gcanvas
  TRootEmbeddedCanvas    *fEc;

  TGDoubleHSlider        *fHslider;
  TGDoubleVSlider        *fVslider;

  TGListTree             *fListTree;    // list tree with histograms
  TGListTree             *fCutTree;    // list tree with cuts
  TGListTree             *fRoiTree;    // list tree with roi

  TGListTreeItem         *iWork;
  TGListTreeItem         *iWork_cut[MAXCUTS];
  TGListTreeItem         *iWork_MT;

  TGStatusBar            *fStatus;


  TList* hmap_list;  //list of marked hmaps (histograms)
  TList* st_hlist;      //list of plotted histograms for stack
  TH1F* st_plot;        //histogram for plotting stack;

  TH1* pad_hist[MAX_PADS];     //copies of histograms for plotting in pads;
  HMap *pad_map[MAX_PADS];       //maps plotted in pads

  TText ttxt;

  TGCheckButton* chknorm;
  TGCheckButton* chklog;
  TGCheckButton* chkstat;
  TGCheckButton* chkgcuts;
  TGRadioButton *Rb[2];

  TGTextEntry* tForm;

  int ntab; //tab number where eventframe is placed

  Bool_t wrk_check[MAXCUTS+1]; //is work* checked before deleting ltree
  //work,work_cut[MAXCUTS]
  //Bool_t wrk_check_MT; //is work_MT checked before deleting ltree
  Bool_t changed;
  Bool_t started;
  Int_t in_gcut; //0-cancel;1-cut;2-roi
  int np_gcut; //number of points in gcut

  //TLine line;
  int ndiv;

  bool b_adj; //apply b_adj calibration
  d2vect ee_calib;
  int bpol;
  int fitpol;
  double fitsig;
  int nofit;

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
  void RoiClick(TGListTreeItem* item,Int_t but);
  void DoCheck(TObject* obj, Bool_t check);
  void DoNorm();
  void DoLog();
  void DoStat();
  void DoKey(TGListTreeItem* entry, UInt_t keysym, UInt_t mask);
  //void SelectDiv(int nn);
  void DoNum();
  void DoRadio();
  void DoButton();
  void DoSlider();
  void GetHMinMax(TH1* hh, double x1, double x2,
		  double &y1, double &y2);
  void X_Slider(TH1* hh, double &a1, double &a2);
  void Y_Slider(TH1* hh, double a1, double a2, double y1, double y2);
  void AddCutG(TPolyLine *pl, HMap* map);
  int Find_icut();
  void MakeCutG(TPolyLine *pl, HMap* map);
  void ItemROI(HMap* map, int iroi);
  void MakeROI(TPolyLine *pl, HMap* map);
  void StartMouse();
  void AddFormula();
  //void DoFormula();
  void ShowCutG();
  void ClearCutG();
  string CutsToStr();
  void EditCutG();
  void DoPeaks();
  void PeakFit(HMap* map, TH1* hist1, TH1* hist2, int i, d2vect &d2);
  void DelMaps(TGListTreeItem *idir);
  void Do_Ecalibr(PopFrame* pop);
  void Do_Tcalibr(PopFrame* pop);
  //void Do_Ecalibr();
  void HiReset();
  void Update();
  void DrawStack();
  void Make_Hmap_List();
  void DrawHist();
  void OneRebinPreCalibr(HMap* &map, TH1* &hist, bool badj);
  void AllRebinDraw();
  void DrawCuts(int npad);
  void DrawRoi(Hdef* hd, TVirtualPad* pad);//int npad);
  void ReDraw();
  //void DataDropped(TGListTreeItem *, TDNDData *data);

  ClassDef(HistFrame, 0)
};

void *trd_handle(void* ptr);


#endif
