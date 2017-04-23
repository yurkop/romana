//----- HistFrame ----------------
#include "histframe.h"
#include "romana.h"
#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
//#include <TROOT.h>


#include <TRandom.h>

//TLine ln1;

extern Toptions opt;
extern MyMainFrame *myM;
//extern BufClass* Buffer;

extern CRS* crs;
extern ParParDlg *parpar;

extern ULong_t fGreen;
extern ULong_t fRed;
extern ULong_t fCyan;

extern TH1F *hrms;
extern TH1F *htdc_a[MAX_CH]; //absolute tof (start=0)

//TText txt;

//TMutex *Emut;

TGLayoutHints* fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
TGLayoutHints* fLay2 = new TGLayoutHints(kLHintsExpandY,2,2,2,2);
TGLayoutHints* fLay3 = new TGLayoutHints(kLHintsExpandX | kLHintsBottom,20,20,20,20);
TGLayoutHints* fLay4 = new TGLayoutHints(kLHintsLeft,2,2,2,2);
TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
TGLayoutHints* fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
TGLayoutHints* fLay7 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,5,0,0,0);

//------------------------------

Bool_t MECanvas::HandleDNDDrop(TDNDData *data) {
  //TRootEmbeddedCanvas(data);
  cout << "DND" << endl;
  return TRootEmbeddedCanvas::HandleDNDDrop(data);
}

//------------------------------

HistFrame::HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  cout << "HistFrame: " << gDNDManager << endl;

  ntab=nt;

  const TGPicture *pic = 0;
  TGListTreeItem *item;
  TGListTreeItem *idir;

  char ss[100];

  //Frames.....

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc

  fHor1 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor1, fLay1);

  fHor2 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor2, fLay3);

  //fEc = new TRootEmbeddedCanvas("Hcanvas", fHor1, 10, 10);
  fEc = new MECanvas("Hcanvas", fHor1, 10, 10);
  fHor1->AddFrame(fEc, fLay1);

  //TGVertical3DLine *separator1 = new TGVertical3DLine(fHor1);
  //fHor1->AddFrame(separator1, fLay2);

  fCanvas = new TGCanvas(fHor1, 150, 100);
  fHor1->AddFrame(fCanvas, fLay7);
  fListTree = new TGListTree(fCanvas, kHorizontalFrame);
  fListTree->Associate(this);
  fEc->SetDNDTarget(kTRUE);

  //fCanvas = fEc->GetCanvas();

  //TH1F *hpx = (TH1F *)GetObject("1D Hist");
  pic = gClient->GetPicture("h1_t.xpm");
  item = fListTree->AddItem(0, hrms->GetName(), hrms, pic, pic);
  fListTree->SetToolTipItem(item, "1D Histogram");
  item->SetDNDSource(kTRUE);

  idir = fListTree->AddItem(0, "Time");
  idir->SetDNDSource(kTRUE);

  for (int i=0;i<MAX_CH;i++) {
    item = fListTree->AddItem(idir, htdc_a[i]->GetName(), htdc_a[i],
			      pic, pic);
    fListTree->SetToolTipItem(item, "1D Histogram");
    item->SetDNDSource(kTRUE);
  }


  /*
  */
  
}

HistFrame::~HistFrame()
{

  //cout << "~Emut" << endl;
  //delete Emut;
  //gSystem->Sleep(100);

  //if (trd) {
  //trd->Delete();
  //}

  //cout << "~trd" << endl;
  //gSystem->Sleep(100);

  cout << "~HistFrame()" << endl;
  //fMain->DeleteWindow();

  //myM->fEv=NULL;
}


extern HistFrame* EvtFrm;

// void HistFrame::StartThread()
// {
//   trd = new TThread("trd", trd_handle, (void*) 0);
//   trd->Run();
// }

void HistFrame::CloseWindow()
{
  delete this;
}

// void HistFrame::DoReset() {
//   myM->DoReset();
// }

/*

void HistFrame::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  if (id==101) {
    opt.num_buf=te->GetNumber();
    cout << "num_buf: " << te->GetNumber() << endl;
  }
  

}

void HistFrame::Do1buf() {

  crs->Do1Buf();

}

void HistFrame::DoNbuf() {

  // if (!crs->f_raw) {
  //   cout << "File not open" << endl;
  //   return;
  // }

  //cout << "DoAna" << endl;

  if (crs->b_fana) {
    fNbuf->ChangeBackground(fGreen);
  }
  else {
    fNbuf->ChangeBackground(fCyan);
  }

  crs->DoNBuf();

}

*/

void HistFrame::First() {
  d_event = Levents->begin();
  DrawEvent2();
}

void HistFrame::Last() {
  d_event = --Levents->end();
  DrawEvent2();
}

void HistFrame::Plus1() {
  ++d_event;
  if (d_event==Levents->end()) {
    --d_event;
  }
  DrawEvent2();
}

void HistFrame::Minus1() {
  if (d_event!=Levents->begin())
    --d_event;
  DrawEvent2();
}

void HistFrame::PlusN() {
  for (int i=0;i<opt.num_events;i++) {
    ++d_event;
    if (d_event==Levents->end()) {
      --d_event;
      break;
    }
  }
  DrawEvent2();
}

void HistFrame::MinusN() {
  for (int i=0;i<opt.num_events;i++) {
    if (d_event!=Levents->begin()) {
      --d_event;
    }
    else {
      break;
    }
  }
  DrawEvent2();
}

void HistFrame::Clear()
{

}

void HistFrame::DoSlider() {

  /*
    float x1,x2;
    fHslider->GetPosition(x1,x2);

    printf("DosLider: %f %f\n",x1,x2);

    float dx=gx2-gx1;
    float dx1=dx*x1;
    float dx2=dx*x2;

    for (int n=0;n<ndiv;n++) {
    fPaint[n].GetXaxis()->SetRangeUser(gx1+dx1,gx1+dx2);
    }
  */

  //printf("DosLider: %d\n",nEvents);
  ReDraw();

}

void HistFrame::DoChkDeriv() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  //cout << "DoDeriv: " << id << endl;

  opt.b_deriv[id] = !opt.b_deriv[id];
  btn->SetState((EButtonState) opt.b_deriv[id]);

  //DrawEvent();

}

void HistFrame::DoChkPeak() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  opt.b_peak[id] = !opt.b_peak[id];

  btn->SetState((EButtonState) opt.b_peak[id]);

  if (id==0) {
    if (opt.b_peak[id]) {
      for (int i=1;i<9;i++) {
	fPeak[i]->SetState(kButtonEngaged);
	fPeak[i]->SetState((EButtonState) opt.b_peak[i]);
      }
    }
    else {
      for (int i=1;i<9;i++) {
	fPeak[i]->SetState(kButtonDisabled);
      }
    }
  }

  ReDraw();

}

void HistFrame::DoPulseOff() {

  //TGButton *btn = (TGButton *) gTQSender;
  //Int_t id = btn->WidgetId();
  //cout << "DoPulseOff: " << id << " " << fChn[id]->IsOn() << endl;

  ReDraw();

}

void HistFrame::FillHist(int dr) {
}

void HistFrame::DrawEvent2() {


} //DrawEvent2

void HistFrame::DrawPeaks(double y1,double y2) {


}

void HistFrame::DrawEvent() {

} //DrawEvent

void HistFrame::DoGraph(int ndiv, int dd) {

  /*
    fPaint[dd].Draw("AXIS");

    for (int i=0;i<NGr;i++) {
    Gr[dd][i].Draw("LP");
    }
  */


  /*
    for (int i=0;i<NGr;i++) {
    if (i==0) {
    Gr[dd][i].Draw("ALP");
    }
    else {
    Gr[dd][i].Draw("LP");
    }
    }
  */

  /*

    double ygr[DSIZE];
    char ss[6];

    TLine *lpk[DSIZE];
    TLine *l1[DSIZE];
    //TLine *l0[DSIZE];
    TLine *l2[DSIZE];
    double x1,x2,y1,y2;
    double thresh;

    int p1,p2;

    //double min=9999999;

    char label[100];

    //int flag;

    int opt_ch = opt.channels[nch];

    if (opt_ch==ch_off2) {
    printf("ch_off2: %d\n",nch);
    return;
    }

    y1=0.8-1*0.02;

    TLegend *leg = new TLegend(0.75,y1,0.99,0.99);

    //tp->cd(i);
    //printf("evlength: %d\n",evlength);
    TGraph *gr1;

    if (opt_ch==ch_nim) {
    thresh=opt.nim_thresh;
    if (!deriv) {
    for (int j=0;j<evlength;j++) {
    ygr[j]=Event[j]-Event[0];
    //ygr[j]=Event[i][j]-BKGR;
    }
    }
    else {
    for (int j=1;j<evlength;j++) {
    ygr[j]=Event[j]-Event[j-1];
    }
    ygr[0]=0;
    }
    }
    else {
    if (opt_ch==ch_gam)
    thresh=opt.gam_thresh;
    if (opt_ch==ch_ng)
    thresh=opt.ng_thresh;

    if (!deriv) {
    for (int j=0;j<evlength;j++) {
    ygr[j]=sEvent[j]-sEvent[0];
    //ygr[j]=sEvent[i][j]-BKGR;
    }
    }
    else {
    for (int j=1;j<evlength;j++) {
    ygr[j]=sEvent[j]-sEvent[j-1];
    }
    ygr[0]=0;
    }
    }

    gr1 = new TGraph (evlength, xgr, ygr);
    //printf("gr1\n");

    gr1->SetLineColor(1);
    gr1->SetMarkerStyle(20);
    gr1->SetMarkerSize(0.5);
    //gr1->SetFillColor(0);
    //gr1->SetLineWidth(3);
    sprintf(ss,"%d",nch);
    gr1->SetTitle(ss);
    gr1->Draw("ALP");

    //leg->AddEntry(gr1,"Event","l");
    //printf("leg1\n");

    gPad->Update();
    y1=gPad->GetUymin();
    y2=gPad->GetUymax();

    if (deriv) {
    x1=gPad->GetUxmin();
    x2=gPad->GetUxmax();

    TLine* lthr=new TLine(x1,thresh,x2,thresh);
    lthr->SetLineColor(3);
    lthr->Draw();

    TLine* l0=new TLine(x1,0,x2,0);
    l0->SetLineColor(2);
    l0->Draw();

    }

    for (int j=nPk-1;j>=0;j--) {

    int pp=Peaks[j].time - tstamp64;
    if (pp<0) {
    break;
    }

    if (nch!=Peaks[j].ch) continue;

    int jj=nPk-j;

    //printf("zzzz: %d %d\n",j,pp);
    lpk[jj] = new TLine(pp,y1,pp,y2);
    
    if (opt_ch==ch_nim) {
    lpk[jj]->SetLineColor(6);
    strcpy(label,"NIM signal");
    }
    else if (opt_ch==ch_ng || opt_ch==ch_gam) {

    if (opt_ch==ch_ng) {
    p1=pp+opt.ng_border1;
    p2=pp+opt.ng_border2;
    }
    else if (opt_ch==ch_gam) {
    p1=pp+opt.gam_border1;
    p2=pp+opt.gam_border2;
    }
    l1[jj] = new TLine(p1,y1,p1,y2);
    l2[jj] = new TLine(p2,y1,p2,y2);
    l1[jj]->SetLineStyle(2);
    l2[jj]->SetLineStyle(4);

    //flag=Peaks[j].flag;

    switch (Peaks[j].flag) {
    case f_pileup:
    lpk[jj]->SetLineColor(7);
    sprintf(label,"T pile-up: %d",pp);
    break;
    case f_badleft:
    lpk[jj]->SetLineColor(41);
    sprintf(label,"Bad left: %d",pp);
    break;
    case f_badright:
    lpk[jj]->SetLineColor(41);
    sprintf(label,"Bad right: %d",pp);
    break;
    case f_nim:
    lpk[jj]->SetLineColor(6);
    sprintf(label,"NIM signal: %d",pp);
    break;
    case f_romash:
    lpk[jj]->SetLineColor(2);
    sprintf(label,"rom: %d %0.2f",pp,Peaks[j].A);
    break;
    case f_demon:
    lpk[jj]->SetLineColor(2);
    sprintf(label,"dem: %d %0.2f %0.2f",pp,Peaks[j].A,Peaks[j].W);
    break;
    case f_gamma:
    lpk[jj]->SetLineColor(2);
    sprintf(label,"g: %d %0.2f %0.2f",pp,Peaks[j].A,Peaks[j].W);
    break;
    case f_neu:
    lpk[jj]->SetLineColor(3);
    sprintf(label,"n: %d %0.2f %0.2f",pp,Peaks[j].A,Peaks[j].W);
    break;
    case f_tail:
    lpk[jj]->SetLineColor(4);
    sprintf(label,"t: %d %0.2f %0.2f",pp,Peaks[j].A,Peaks[j].W);
    break;
    default:
    lpk[jj]->SetLineColor(8);
    sprintf(label,"unknown: %d %d %0.2f %0.2f",pp,Peaks[j].flag,
    Peaks[j].A,Peaks[j].W);
    break;
    }

    l1[jj]->Draw();
    //l0[j]->Draw();
    l2[jj]->Draw();

    } // else if opt_ch...

    lpk[jj]->Draw();

    leg->AddEntry(lpk[jj],label,"l");

    } // for j

    if (opt.b_leg && !deriv) leg->Draw();

  */

} //DoGraph

void HistFrame::ReDraw() {

  //cv->Update();
  
}
