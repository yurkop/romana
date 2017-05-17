#ifndef mainframe_H
#define mainframe_H 1

#include <TRootEmbeddedCanvas.h>
#include "TGMenu.h"
#include "TGTab.h"
#include <TLegend.h>
#include <TGFileDialog.h>
#include <TGNumberEntry.h>
//#include "chandlg.h"
//#include "numdlg.h"
//#include "pardlg.h"
#include "eventframe.h"
#include "histframe.h"

//-----------------------------------------------
class MainFrame : public TGMainFrame {
  //RQ_OBJECT("MainFrame")
private:
  TGMainFrame          *fMain;
  TRootEmbeddedCanvas  *fEcanvas;
  //TCanvas              *fcanvas;
  //TCanvas              *fAna;
  //TGNumberEntry        *n_events;
  TGNumberEntry        *n_buffers;
  TGMenuBar            *fMenuBar;     // main menu bar
  TGPopupMenu          *fMenuFile;    // "File" popup menu entry
  TGPopupMenu          *fMenuOptions;    
  TGPopupMenu          *fMenuHist;    
  TGPopupMenu          *fMenuAna;    
  TGPopupMenu          *fMenuHelp;    // "Help" popup menu entry
  TLegend              *fLeg[8];
  TLegend              *fLeg1ev;
  //THStack              *fHS[6];
  //TH2F                 *fHist[6];
  //TGraph               *fGr[6];
  TGTextButton *fStart;
  TGTextButton *fAna;
  TGTextButton *fNb;

 public:
  bool                   bRun;
  TGTab                *fTab;

  Pixel_t fGreen;
  Pixel_t fRed;
  Pixel_t fCyan;
  Pixel_t fBluevio;

  //TList* hlist;

  // TH1F* h_ampl[MAX_CH]; //amplitude - area of the peak
  // TH1F* h_height[MAX_CH]; //height of the peak
  // TH1F* h_time[MAX_CH]; // real time
  // TH1F* h_tof[MAX_CH]; // time of flight

public:
  MainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MainFrame();

  TGStatusBar          *fBar1;
  TGStatusBar          *fBar2;

  //void Make_hist();

  void SetTitle(char* fname);
  void DoStartStop();
  void DoOpen();
  void DoAna();
  void DoRWinit(EFileDialogMode);
  void DoReadRoot();
  void DoReset();
  void DoClear();
  void InitCanvas(int);
  //void DoCheckGcut();
  void DoCheckOsc();
  void DoCheckLeg();
  void DoCheckLogY();
  void DoCheckTime();
  void DoCheckTree();
  void DoSave();
  void DoGcut(int);
  void DoExit();
  void DoStop();
  void DoAllevents();
  void DoFindBeam();
  void DoChkPoint();
  //void Do1event();
  //void DoNevents();
  void Do1buf();
  void DoNbuf();
  void DoTab(Int_t num);

  //void MakeEvents();

  void EventInfo(Int_t, Int_t, Int_t, TObject*);
  void DoSync();
  void DoCross();
  //void DoInitMON();
  //void DoReadMakan();
  //void MakeMonitor();

  void UpdateStatus();

  void DoSetNumBuf();
  void HandleMenu(MENU_COM);
  //void HandleHelp();

  void exec3event(Int_t, Int_t, Int_t, TObject *);

  //void FillHist(EventClass* evt);
  //NumDlg               *fNumD;
  //ParDlg               *fPar;
  //ChanDlg              *fChan;
  //EventFrame           *fEv;

  //TGCanvas             *fTst;

  int nPads;

  ClassDef(MainFrame, 0)
};

#endif
