#ifndef libmana_H
#define libmana_H 1

#include "common.h"
#include "toptions.h"


#include <sys/times.h>

//#include "libcrs.h"
//#include "eventframe.h"


#include "colors.h"
#include "popframe.h"
#include <TGDockableFrame.h>
#include <TTimeStamp.h>
#include <TDataMember.h>
#include <list>

enum MENU_COM {
  M_READINIT,
  M_SAVEINIT,
  M_READROOT,
  M_SAVEROOT,
  //M_SAVEASCII,
  M_FILE_BROWSE,
  M_RESET_USB,
  M_EXPORT,
  M_FILE_EXIT,
  M_EDIT_PROF8,
  M_EDIT_PROF64,
  M_EDIT_CUTG,
  M_PROF_TIME,
  //M_PRECALIBR,
  M_ECALIBR,
  M_TCALIBR,
  M_PEAKS,
  M_TEST,
  M_HELP,
};

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

void prnt(const char* fmt, ...);
void debug_mess(bool cond, const char* mess, double par1, int par2=-9999);

void SplitFilename(string str, string &folder, string &name);
void SplitFilename(string str, string &folder, string &name, string &ext);

void MakeVarList(int cp, int op);
Int_t ClassToBuf(const char* clname, const char* varname, char* var, char* buf);
int BufToClass(char* buf, char* buf2, int op);
void SaveParTxt(const char* fname);
int FindVar(char* buf, int sz, const char* name, char* var);

void example();
void saveroot(const char *name);
int readpar_root(const char* pname, int ropt=1);
int readroot(const char *name);
short int bits(int n, int i1, int i2);
Bool_t getbit(int n, int bit);
void setbit(int &n, int bit, int set);
string numstr(string inpstr);

bool TestFile();

TTimeStamp prtime(const char* txt, int set=1, const char* col=BRED);
int CheckMem(bool pr=false);

class MyMainFrame;
class CRS;
class EventFrame;
class HistFrame;
class ErrFrame;
class HClass;

//-----------------------------------------------
class GlbClass : public TNamed {
 public:
  //std::list<void*> lst;
  Toptions* g_opt;
  Coptions* g_cpar;
  CRS* g_crs;
  MyMainFrame* g_myM;
  EventFrame* g_EvtFrm;
  HistFrame* g_HiFrm;
  ErrFrame* g_ErrFrm;
  HClass* g_hcl;

  //std::vector<Pmap> PopVarlist;

  GlbClass();
  virtual ~GlbClass() {};
};
//-----------------------------------------------
class VarClass {
 public:
  TString name;
  char* Var;
  TDataMember *Dm;
};
//-----------------------------------------------
class MainFrame : public TGMainFrame {

 public:

  // Int_t chcol[MAX_CH+NGRP];
  // ULong_t gcol[MAX_CH+NGRP];

  TTimer* fTimer;

  TGLayoutHints* LayEE1;
  TGLayoutHints* LayEE2;

  TRootEmbeddedCanvas  *fEcanvas;

  TGMenuBar    *fMenuBar;     // main menu bar

  TGGroupFrame *fGr1, *fGr2;

  TGTextButton *fStart,
    *fContinue,
    *fReset2,
    *fAna,
    *fNb;

  //PEditor* p_ed;
  //PopFrame* p_pop;

  TGTab                *fTab;
  std::vector<TGCompositeFrame*> tabfr;

  int local_nch;
  int local_nrows;
  static const Int_t n_stat=13;
  static const Int_t n_stat2=11;
  TGTextEntry* fStat[n_stat];

 public:
  MainFrame(const TGWindow *p,UInt_t w,UInt_t h);
  virtual ~MainFrame();

  void MakeTabs(bool reb=false);
  void Rebuild();
  void SetTitle(char* fname);

  void EnableBut(TGGroupFrame* fGr, bool enbl);

  void DoStartStop(int rst);
  void DoOpen(Int_t popt);
  void DoClose();
  void DoAna();
  void DoRWinit(EFileDialogMode);
  void DoReadRoot();
  void Export();
  //void Export();
  void DoReset();
  void DoSaveRoot();
  //void DoSaveAscii();
  void CloseWindow();
  void DoExit();
  //void DoResetUSB();
  //void DoStop();
  void Do1buf();
  void DoNbuf();
  void DoTab(Int_t num);

  //void ParLock();
  //void ParUnLock();

  void EventInfo(Int_t, Int_t, Int_t, TObject*);
  // void DoCross();

  void UpdateTimer(int rst=0);
  //void UpdateStatus(int rst=0);

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

#endif
