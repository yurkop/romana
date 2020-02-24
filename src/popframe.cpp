//----- PopFrame ----------------
#include "popframe.h"
#include "libmana.h"
#include "histframe.h"
#include "toptions.h"
#include "hclass.h"
//#include <iostream>
#include <TCanvas.h>
#include <TPolyMarker.h>

extern Common* com;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern Toptions opt;

using namespace std;
PopFrame::PopFrame(const TGWindow *main, UInt_t w, UInt_t h)
{
  txt.SetTextSize(0.15);
  txt.SetTextAlign(22);
  fMain = new TGTransientFrame(gClient->GetRoot(), main, w, h);
  fMain->Connect("CloseWindow()", "PopFrame", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.
  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  fCanvas = new TRootEmbeddedCanvas("Events",fMain,w,h);
  fMain->AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));

 
  TGTextButton* fOK = new TGTextButton(fMain, "  &OK  ");
  fOK->Connect("Clicked()", "PopFrame", this, "DoOK()");
  fMain->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

  fMain->MapSubwindows();
  fMain->Resize();
  // editor covers right half of parent window
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kCenter);
  fMain->MapWindow();
}
PopFrame::~PopFrame()
{
  fMain->DeleteWindow();  // deletes fMain
}
void PopFrame::CloseWindow()
{
  // Called when closed via window manager action.
  //cout << "fVar1: " << fVar << " " << this << endl;
  //fVar=NULL;
  delete this;
  myM->p_pop=0;
  //*fVar=0;
  //cout << "fVar2: " << *fVar << " " << this << endl;
}

void PopFrame::DoOK()
{
  TH1* hh;
  TGraph* gr;
  TF1* fit;
  int res=0;

  char ss[10];
  sprintf(ss,"tof_%02d",opt.Prof64[4]);
  TGListTreeItem *item=HiFrm->FindItem(ss);
  if (item) {
    res++;
    HMap* map=(HMap*) item->GetUserData();
    hh = map->hst;

    hh->ShowPeaks(2,"",0.05);
    TList *functions = hh->GetListOfFunctions();
    TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
    if (pm) {
      res++;
      //functions->Remove(pm);
      int nn = pm->GetN();
      if (nn==31) {
	res++;
	gr = new TGraph(nn,pm->GetX(),pm->GetX());
	gr->Sort();
	for (int i=0;i<nn;i++) {
	  gr->GetX()[i]=i;
	}
	gr->SetNameTitle("Time_Calibr","Time Calibration");
	gr->Fit("pol1");
    
	fit = gr->GetFunction("pol1");
	if (fit) {
	  res++;
	}
      }
    }
  }

  TCanvas *cv=fCanvas->GetCanvas();
  cv->Clear();
  cv->Divide(1,2);
  if (res==4) {
    cv->cd(1);
    hh->Draw();
    cv->cd(2);
    gr->Draw("AL*");
    opt.Prof64_TC[0]=fit->GetParameters()[0]/opt.Period;
    opt.Prof64_TC[1]=fit->GetParameters()[1]/opt.Period;
  }
  else {
    cv->cd(2);
    txt.DrawTextNDC(0.5,0.5,"Can't find 31 peaks");
  }
  cv->Update();
}
