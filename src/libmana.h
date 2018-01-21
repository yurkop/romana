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

void saveroot(char *name);
void readroot(char *name);
short int bits(int n, int i1, int i2);

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
