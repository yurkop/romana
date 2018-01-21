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
//void fitpeak(TH1* hh, double ww);
void swap_bytes(unsigned short* buf);

//void drawevent(int i, int opt_ch, int deriv);

void allevents();
void example();

void dumpbuf(int nn);
void dumpevent();

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

 public:
  TGMainFrame          *fMain;
private:
  TRootEmbeddedCanvas  *fEcanvas;

  TGMenuBar            *fMenuBar;     // main menu bar
  TGPopupMenu          *fMenuFile;    // "File" popup menu entry
  TGPopupMenu          *fMenuHelp;    // "Help" popup menu entry
  TGTextButton *fStart;
  TGTextButton *fAna;
  TGTextButton *fNb;

 public:
  //bool                   bRun;
  TGTab                *fTab;

  Pixel_t fGreen;
  Pixel_t fRed;
  Pixel_t fCyan;
  Pixel_t fBluevio;

  static const Int_t n_stat=9;
  TGTextEntry* fStat[n_stat];

public:
  MainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MainFrame();

  void SetTitle(char* fname);
  void DoStartStop();

  void DoOpen();
  void DoAna();
  void DoRWinit(EFileDialogMode);
  void DoReadRoot();
  void DoReset();
  void DoSave();
  void DoExit();
  //void DoStop();
  void Do1buf();
  void DoNbuf();
  void DoTab(Int_t num);

  bool TestFile();

  void EventInfo(Int_t, Int_t, Int_t, TObject*);
  void DoCross();

  void UpdateStatus();

  //void DoSetNumBuf();
  void HandleMenu(Int_t);

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
