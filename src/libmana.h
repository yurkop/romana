#ifndef libmana_H
#define libmana_H 1

#include "common.h"
#include <TGDockableFrame.h>
#include <TGTextEdit.h>

using namespace std;

//#include "mainframe.h"
//#include "chandlg.h"
//#include "numdlg.h"
//#include "pardlg.h"

//#ifdef ROMASH
//#include "romana.h"
//#endif

#include <TH1.h>
//#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TGNumberEntry.h>
#include "TGMenu.h"
#include <TLegend.h>
#include "TGTab.h"
#include <TGFileDialog.h>
#include "TGMsgBox.h"

void debug_mess(bool cond, const char* mess, double par1, int par2=-9999);

UShort_t ClassToBuf(const char* name, const char* varname, char* var, char* buf);
void BufToClass(const char* name, const char* varname, char* var, char* buf, int size);

void SplitFilename(string str, string &folder, string &name);
void SplitFilename(string str, string &folder, string &name, string &ext);

void example();
void saveroot(const char *name);
void readroot(const char *name);
short int bits(int n, int i1, int i2);
Bool_t getbit(int n, int bit);
void setbit(int &n, int bit, int set);

bool TestFile();

//-----------------------------------------------
class MainFrame : public TGMainFrame {

  //public:
  //TGMainFrame          *fMain;
public:
  TRootEmbeddedCanvas  *fEcanvas;

  TGMenuBar            *fMenuBar;     // main menu bar
  //TGPopupMenu          *fMenuFile;    // "File" popup menu entry
  //TGPopupMenu          *fMenuHelp;    // "Help" popup menu entry
  TGTextButton *fStart;
  TGTextButton *fReset;
  TGTextButton *fAna;
  TGTextButton *fNb;

  //TGLayoutHints* Lay11;
  //TGLayoutHints* Lay12;

  TGTab                *fTab;
  TGCompositeFrame     *tabfr[10];

  // Pixel_t fGreen;
  // Pixel_t fRed;
  // Pixel_t fCyan;
  // Pixel_t fBluevio;

  //bool fremake;
  int local_nch;
  static const Int_t n_stat=11;
  TGTextEntry* fStat[n_stat];

public:
  MainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MainFrame();

  void MakeTabs();
  void Rebuild();
  void SetTitle(char* fname);
  void DoStartStop();

  void DoOpen(Int_t id);
  void DoClose();
  void DoAna();
  void DoRWinit(EFileDialogMode);
  void DoReadRoot();
  void Export();
  //void Export();
  void DoReset();
  void DoSave();
  void DoExit();
  //void DoResetUSB();
  //void DoStop();
  void Do1buf();
  void DoNbuf();
  void DoTab(Int_t num);

  //void ParLock();
  //void ParUnLock();

  void EventInfo(Int_t, Int_t, Int_t, TObject*);
  void DoCross();

  void UpdateStatus(int rst=0);

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

//////////////////////////////////////////////////////////////////////////
class TGMatrixLayout2 : public TGLayoutManager {

private:
   TGMatrixLayout2(const TGMatrixLayout2&);
   TGMatrixLayout2& operator=(const TGMatrixLayout2&);

protected:
   TGCompositeFrame *fMain;      // container frame
   TList            *fList;      // list of frames to arrange

public:
   Int_t   fSep;                      // interval between frames
   Int_t   fHints;                    // layout hints (currently not used)
   UInt_t  fRows;                     // number of rows
   UInt_t  fColumns;                  // number of columns

   TGMatrixLayout2(TGCompositeFrame *main, UInt_t r, UInt_t c, Int_t s=0, Int_t h=0);

   virtual void Layout();
   virtual TGDimension GetDefaultSize() const;
   virtual void SavePrimitive(ostream &out, Option_t * = "");

   ClassDef(TGMatrixLayout2,0)  // Matrix layout manager
};

class Editor {

private:
   TGTransientFrame *fMain;   // main frame of this widget
   TGTextEdit       *fEdit;   // text edit widget
   TGTextButton     *fOK;     // OK button
   TGLayoutHints    *fL1;     // layout of TGTextEdit
   TGLayoutHints    *fL2;     // layout of OK button

public:
   Editor(const TGWindow *main, UInt_t w, UInt_t h);
   virtual ~Editor();

   void   LoadFile(const char *file);
   void   LoadBuffer(const char *buffer);
   void   AddBuffer(const char *buffer);

   TGTextEdit *GetEditor() const { return fEdit; }

   void   SetTitle();
   void   Popup();

   // slots
   void   CloseWindow();
   void   DoOK();
   void   DoOpen();
   void   DoSave();
   void   DoClose();
};


#endif
