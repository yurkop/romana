#ifndef libmana_H
#define libmana_H 1

#include "common.h"

using namespace std;

//#include "mainframe.h"
//#include "chandlg.h"
//#include "numdlg.h"
//#include "pardlg.h"

//#ifdef ROMASH
//#include "romana.h"
//#endif

#include <TH1.h>
#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>
#include "TGMenu.h"
#include <TLegend.h>
#include "TGTab.h"
#include <TGFileDialog.h>
#include "TGMsgBox.h"

UShort_t ClassToBuf(const char* name, char* var, char* buf);
void BufToClass(const char* name, char* var, char* buf, int size);


void SplitFilename(string str, char *folder, char *name);
void SplitFilename(string str, char *folder, char *name, char* ext);
void new_hist();
void set_hist_attr();
void saveroot(char *name);
void readroot(char *name);
void clear_hist();
void greset();
//int newfile();
void readpar_root(const char* fname);
//void savepar_root(const char* fname);
//void readpar_gz(const char* fname);
//void savepar_gz(const char* fname);
void smooth(int n, int i);
void fill_nim(int ch);
void nim_peak(int ch, int j);
void findpeaks_nim(int ch, int n2, unsigned short* dat);
void peakana_ng(int ch, int n2, double* dat);
void findpeaks_ng(int ch, int n2, double* dat);
void peakana_gam(int ch, int n2, double* dat);
void findpeaks_gam(int ch, int n2, double* dat);
int deltat(int ng, unsigned int tt);
void startbeam(int ch);
int analyze();
//int searchsync();
//void fillevent();
//void fillnewevent();
//int nextevent();
void anabuf_adc2();
void anabuf_avm16();
void anabuf_adcm();
//int readbuf();
short int bits(int n, int i1, int i2);
//void readmonitor(char* fname);
void fitpeak(TH1* hh, double ww);
int getmax(TH1F* hist[]);
void swap_bytes(unsigned short* buf);

//void drawevent(int i, int opt_ch, int deriv);

void allevents();
void example();

void dumpbuf(int nn);
void dumpevent();

void mkstart();
void mktof();
void peaktime(int ch, double* dat, int method, int twin);
void delete_hist();

void FillHist_old(EventClass* evt);

typedef struct {

  Char_t nch; // number of channels in the event
  Char_t ch[MAX_CH]; // ch[nch] - current channel number
  Char_t flag[MAX_CH]; //flag[nch] - flag
  Float_t E[MAX_CH];  //E[nch] - energy
  Float_t T[MAX_CH];  //T[nch] - time
  Float_t W[MAX_CH];  //W[nch] - width
  //Int_t npk; // total number of peaks (can be many in one channel)
  //Int_t fpk[MAX_CH]; // pk[nch] - first peak ...

} event_t;


//-----------------------------------------------
class MainFrame : public TGMainFrame {
  //RQ_OBJECT("MainFrame")
 public:
  TGMainFrame          *fMain;
private:
  //TGMainFrame          *fMain;
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

  static const Int_t n_stat=9;
  TGTextEntry* fStat[n_stat];


  //TList* hlist;

  // TH1F* h_ampl[MAX_CH]; //amplitude - area of the peak
  // TH1F* h_height[MAX_CH]; //height of the peak
  // TH1F* h_time[MAX_CH]; // real time
  // TH1F* h_tof[MAX_CH]; // time of flight

public:
  MainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MainFrame();

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
  void DoSave();
  void DoExit();
  void DoStop();
  //void Do1event();
  //void DoNevents();
  void Do1buf();
  void DoNbuf();
  void DoTab(Int_t num);

  bool TestFile();
  //void Show();
  //void MakeEvents();

  void EventInfo(Int_t, Int_t, Int_t, TObject*);
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

//--------------------------------------
class MyMainFrame: public MainFrame {
 public:
  MyMainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MyMainFrame();
  ClassDef(MyMainFrame, 0)
};

//--------------------------------------
class ColorMsgBox: public TGTransientFrame {
 public:
  ColorMsgBox(const TGWindow *p, const TGWindow *main,
	      const char *title, const char *msg, EMsgBoxIcon icon,
	      Int_t buttons, Int_t *ret_code);
  ~ColorMsgBox();
  //ClassDef(ColorMsgBox, 0)
};

#endif
