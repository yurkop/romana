//----- PopFrame ----------------
#include "popframe.h"
#include "libmana.h"
#include "histframe.h"
#include "toptions.h"
#include "hclass.h"
//#include <iostream>
#include <TCanvas.h>
#include <TPolyMarker.h>
#include <TF1.h>

//extern Common* com;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern Toptions opt;

using namespace std;
PopFrame::PopFrame(const TGWindow *main, UInt_t w, UInt_t h, Int_t menu_id)
{
  txt.SetTextSize(0.15);
  txt.SetTextAlign(22);
  fMain = new TGTransientFrame(gClient->GetRoot(), main, w, h);
  fMain->Connect("CloseWindow()", "PopFrame", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.
  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  if (menu_id==M_PROF_TIME) {
    AddProf(w,h);
  }
  else if (menu_id==M_ECALIBR) {
    AddEcalibr();
  }
  else if (menu_id==M_TCALIBR) {
    AddTcalibr();
  }

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

void PopFrame::AddProf(UInt_t w, UInt_t h) {
  fCanvas = new TRootEmbeddedCanvas("Events",fMain,w,h);
  fMain->AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  TGTextButton* fOK = new TGTextButton(fMain, "  &OK  ");
  fOK->Connect("Clicked()", "PopFrame", this, "DoProfTime()");
  fMain->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));
}

void PopFrame::AddEcalibr() {
  ee[0]=4438;
  fwhm=10;
  npol=1;
  range=300;

  TGLayoutHints* LayLC2   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2);
  TGLayoutHints* LayCC2   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 2,2,2,2);

  for (int i=0;i<11;i++) {
    pframe[i]=0;
  }

  fMain->SetWindowName("Energy Calibration");
  npeaks=1;

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, fwhm, 8, 11, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "Peak width");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, range, 8, 12, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "+/- fit range");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, npol, 8, 13, ki, ka, kl, 0, 2);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "Substrate polynomial");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, npeaks, 8, 14, ki, ka, kl, 1, 10);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "Number of peaks");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayCC2);
  fLabel = new TGLabel(hframe, " ------- Peaks ------- ");
  hframe->AddFrame(fLabel,LayCC2);

  AddPeaks();
}

void PopFrame::AddTcalibr() {
  ee[0]=4438;
  fwhm=10;
  npol=1;
  range=300;

  TGLayoutHints* LayLC2   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2);
  TGLayoutHints* LayCC2   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 2,2,2,2);

  for (int i=0;i<11;i++) {
    pframe[i]=0;
  }

  fMain->SetWindowName("Time Calibration");
  npeaks=1;

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, fwhm, 8, 11, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "Peak width");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, range, 8, 12, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "+/- fit range");
  hframe->AddFrame(fLabel,LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,LayLC2);
  fNum = new TGNumberEntry(hframe, npeaks, 8, 14, ki, ka, kl, 1, 10);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum,LayLC2);
  fLabel = new TGLabel(hframe, "Number of peaks");
  hframe->AddFrame(fLabel,LayLC2);

  pframe[10] = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(pframe[10],new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));
  TGTextButton* fOK = new TGTextButton(pframe[10], "  &OK  ");
  fOK->Connect("Clicked()", "HistFrame", HiFrm, "Do_Ecalibr()");
  pframe[10]->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

}

void PopFrame::DoProfTime() {

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
    //opt.Prof64_W[0]=fit->GetParameters()[0]/opt.Period;
    opt.Prof64_W[1]=fit->GetParameters()[1]/opt.Period;
    //opt.Prof64_W[2]=10;
  }
  else {
    cv->cd(2);
    txt.DrawTextNDC(0.5,0.5,"Can't find 31 peaks");
  }
  cv->Update();
}

void PopFrame::AddPeaks() {
  for (int i=0;i<11;i++) {
    if (pframe[i]) {
      fMain->RemoveFrame(pframe[i]);
      pframe[i]=0;
    }
  }

  for (int i=0;i<npeaks;i++) {
    if (!pframe[i]) {
      pframe[i] = new TGHorizontalFrame(fMain,10,10);
      fMain->AddFrame(pframe[i],new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2));
      fNum = new TGNumberEntry(pframe[i], ee[i], 8, i, kr, ka, kn);
      fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				      "DoENum()");
      pframe[i]->AddFrame(fNum,new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2));
      fLabel = new TGLabel(pframe[i], "Energy (kEv)");
      pframe[i]->AddFrame(fLabel,new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 2,2,2,2));
    }
  }

  pframe[10] = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(pframe[10],new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));
  TGTextButton* fOK = new TGTextButton(pframe[10], "  &OK  ");
  fOK->Connect("Clicked()", "HistFrame", HiFrm, "Do_Ecalibr()");
  pframe[10]->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

  fMain->Resize(fMain->GetDefaultSize());
  fMain->MapSubwindows();
  fMain->Layout();

}

void PopFrame::DoENum() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  if (id>=0 && id<10) {
    ee[id]=te->GetNumber();
    //cout << "ee: " << id << " " << ee[id] << endl;
  }
  else if (id==11) {
    fwhm=te->GetNumber();
  }
  else if (id==12) {
    range=te->GetNumber();
  }
  else if (id==13) {
    npol=te->GetNumber();
  }
  else if (id==14) {
    npeaks=te->GetNumber();
    AddPeaks();
  }
}
