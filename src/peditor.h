#ifndef PEditor_H
#define PEditor_H 1

#include <TGFrame.h>
#include <TGTextEdit.h>
#include "libmana.h"

//-----------------------------------------------
class PEditor {

 private:
  TGTransientFrame *fMain;   // main frame of this widget
  TGTextEdit       *fEdit;   // text edit widget

  MENU_COM menu_id;
  //TString          *str;
 public:
  PEditor(const TGWindow *main, MENU_COM mn, UInt_t w, UInt_t h);
  virtual ~PEditor();

  void   LoadFile(const char *file);
  void   Load_Ing();
  void   LoadPar8();
  void   LoadPar64();
  void   LoadCuts();
  //void   LoadBuffer(const char *buffer);
  //void   AddBuffer(const char *buffer);

  TGTextEdit *GetEditor() const { return fEdit; }

  void   SetTitle();
  void   Popup();

  // slots
  void   CloseWindow();
  void   DoTCalibr();
  void   DoSaveProf();
  void   DoSavePar();
  void   DoPExit();
  void   DoOpen();
  void   DoSave();
  void   DoClose();
  ClassDef(PEditor, 0)
};

#endif
