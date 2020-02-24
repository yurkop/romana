#ifndef popframe_H
#define popframe_H 1

#include "common.h"
#include <TText.h>
#include <TRootEmbeddedCanvas.h>

//-----------------------------------------------
class PopFrame {

private:
  //void** fVar;
  TGTransientFrame *fMain;   // main frame of this widget

  TRootEmbeddedCanvas  *fCanvas;
  TText txt;

public:
  PopFrame(const TGWindow *main, UInt_t w, UInt_t h);
  virtual ~PopFrame();
  void DoOK();

  void   CloseWindow();
  //ClassDef(PopFrame, 0)
};

#endif
