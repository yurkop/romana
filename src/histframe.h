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

struct vpeak {
  double p,h,w; //pos,height,width
};

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

  TGListTreeItem         *iMAIN;
  TGListTreeItem         *iMAIN_cut[MAXCUTS];
  //TGListTreeItem         *iMAIN_MT;

  TGStatusBar            *fStatus;


  TList* hmap_chklist;  //list of marked hmaps (histograms)
  //TList* st_hlist;      //list of plotted histograms for stack
  TH1F* st_plot;        //histogram for plotting stack;

  //TH1* pad_hist[MAX_PADS];     //copies of histograms for plotting in pads;
  //HMap *pad_map[MAX_PADS];       //maps plotted in pads
  std::vector <HMap*> pad_map;       //maps plotted in pads
  //std::vector <TH1*> pad_hist;       //copies of histograms plotted in pads
  std::vector <TNamed*> pad_hist;    //copies of histograms or graphs
                                     //plotted in pads
                                     //or in DrawStack
                                     //hists/graphs must be deleted before clear()

  vector<TString> dopt = {"","zcol"};

  TText ttxt;

  TGCheckButton* chknorm;
  TGCheckButton* chklog;
  TGCheckButton* chkstat;
  TGCheckButton* chkd;
  TGRadioButton *Rb[2];

  TGTextEntry* tForm;

  int ntab; //tab number where eventframe is placed

  //Bool_t wrk_check[MAXCUTS+1]; //is MAIN* checked before deleting ltree
  //MAIN,MAIN_cut[MAXCUTS]
  //Bool_t wrk_check_MT; //is MAIN_MT checked before deleting ltree
  Bool_t changed;
  Bool_t started;
  Bool_t stack_off;// =1 когда только переключили со стэка на hist
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

  //void AddButtons(TGHorizontalFrame *fHor3, TGLayoutHints* LayLC1, int num);
  TGListTreeItem* Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open=0, const TGPicture *closed=0);
  void Make_Ltree();
  void Clear_Ltree();
  TGListTreeItem* FindItem(const char* name);
  void AddMainCuts(HMap* hmap);
  void AddMAIN(TGListTreeItem *item);
  void ClearMAIN();
  void DoClick(TGListTreeItem* item,Int_t but);
  void CheckAll(Bool_t on);
  void AddCheckMAIN();
  void SortMain();

  void CutClick(TGListTreeItem* item,Int_t but);
  void RoiClick(TGListTreeItem* item,Int_t but);
  void OptToCheck();
  void CheckToOpt(TObject* obj, Bool_t check);
  //void DoKey(TGListTreeItem* entry, UInt_t keysym, UInt_t mask);
  void DoRadio();
  void DoButton();
  void DoDrawopt();
  void DoSlider();
  void GetHMinMax(TH1* hh, double x1, double x2,
		  double &y1, double &y2);
  void X_Slider(TH1* hh, double &a1, double &a2);
  void Y_Slider(TH1* hh, double a1, double a2, double y1, double y2);
  void AddCutG(TPolyLine *pl, HMap* map);
  int Find_icut();
  void MakeCutG(TPolyLine *pl, HMap* map);
  //void ItemROI(HMap* map, int ich, int iroi);
  void ItemROI(HMap* map, int iroi);
  void MakeROI(TPolyLine *pl, HMap* map);
  void StartMouse();
  void AddFormula();
  //void DoFormula();
  void ClearCutG();
  string CutsToStr();
  void EditCutG();
  void PeakSearch(TH1* hh, std::vector<vpeak> &vv);
  void DoPeaks_old();
  void DoPeaks(TH1* hh);
  void PeakFit(HMap* map, TH1* hist1, TH1* hist2, int i, d2vect &d2);
  void DelMaps(TGListTreeItem *idir);
  void Do_Ecalibr(PopFrame* pop);
  void Do_Tcalibr(PopFrame* pop);
  //void Do_Ecalibr();
  void DoRst();
  void HiReset();
  void Update();
  void DrawStack();
  void Make_Hmap_ChkList();
  bool CheckPads();
  void DrawHist();
  void OneRebinPreCalibr(HMap* &map, TH1* &hist, bool badj);
  void AllRebinDraw();
  void DrawCuts(int npad);
  void DrawRoi(Hdef* hd, TVirtualPad* pad);//int npad);
  void ReDraw();
  //void DataDropped(TGListTreeItem *, TDNDData *data);

  ClassDef(HistFrame, 0)
};

//void *trd_handle(void* ptr);


#endif
