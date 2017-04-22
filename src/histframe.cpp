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

//TText txt;

//TMutex *Emut;

HistFrame::HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kHorizontalFrame)
{

  Levents = &Tevents;
  d_event = Levents->begin();
  cout << "d_event: " << &(*d_event) << endl;
  //d_event = new EventClass();
  //Emut = new TMutex();

  ntab=nt;
  // build widgets

  //bprint=false;

  //cout << "HistFrame: " << p->GetName() << endl;
  
  char ss[100];

  fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
  fLay2 = new TGLayoutHints(kLHintsExpandY,2,2,2,2);
  fLay3 = new TGLayoutHints(kLHintsExpandX,2,2,2,2);
  fLay4 = new TGLayoutHints(kLHintsLeft,2,2,2,2);
  fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
  fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);

  //Frames.....

  TGVerticalFrame        *fVer0; //contains canvas, fHor1
  TGVerticalFrame        *fVer_d; //contains deriv etc
  TGVerticalFrame        *fVer_ch; //contains fHor_ch
  TGHorizontalFrame      *fHor_ch; //contains fVer[..]
  TGVerticalFrame        *fVer[(MAX_CH-1)/16+1];

  TGHorizontalFrame      *fHor_but; //contains buttons
  TGVerticalFrame        *fVer_st; //status etc //contains fHor_st + statusbar
  TGHorizontalFrame      *fHor_st; //contains fVer_d, fVer_ch

  TGVertical3DLine       *separator1;

  fVer0 = new TGVerticalFrame(this, 10, 10);
  AddFrame(fVer0, fLay1);

  separator1 = new TGVertical3DLine(this);
  AddFrame(separator1, fLay2);

  fVer_st = new TGVerticalFrame(this, 10, 10);
  AddFrame(fVer_st, fLay4);

  fHor_st = new TGHorizontalFrame(fVer_st, 10, 10);
  fVer_st->AddFrame(fHor_st, fLay4);


  fVer_d = new TGVerticalFrame(fHor_st, 10, 10);
  fHor_st->AddFrame(fVer_d, fLay4);

  fCanvas = new TRootEmbeddedCanvas("Events",fVer0,w,h);
  fVer0->AddFrame(fCanvas, fLay1);

  fHslider = new TGDoubleHSlider(fVer0, 300, kDoubleScaleBoth);
  fHslider->SetRange(0,1);
  fHslider->SetPosition(0,1);
  fVer0->AddFrame(fHslider, fLay3);

  fHslider->Connect("PositionChanged()", "HistFrame", 
		    this, "DoSlider()");

  fHor_but = new TGHorizontalFrame(fVer0, 10, 10);
  fVer0->AddFrame(fHor_but, fLay4);

  /*
  
  //freset = new TGTextButton(fHor_but,"Reset");
  //freset->Connect("Clicked()","HistFrame",this,"DoReset()");
  //fHor_but->AddFrame(freset, fLay4);

  f1buf = new TGTextButton(fHor_but,"1 buf");
  f1buf->Connect("Clicked()","HistFrame",this,"Do1buf()");
  fHor_but->AddFrame(f1buf, fLay4);

  fNbuf = new TGTextButton(fHor_but,"N buf");
  fNbuf->ChangeBackground(fGreen);
  fNbuf->Connect("Clicked()","HistFrame",this,"DoNbuf()");
  fHor_but->AddFrame(fNbuf, fLay4);

  int id;
  id = parpar->Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(fHor_but, 0, 0, id,
					   TGNumberFormat::kNESInteger,
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   1,100000);
  parpar->DoMap(fNum1->GetNumberEntry(),&opt.num_buf,p_inum,0);
  fNum1->Resize(65, fNum1->GetDefaultHeight());
  //fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "HistFrame", this,
  //				   "DoNum()");
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  fHor_but->AddFrame(fNum1,fLay4);

  //cout << "num_buf: " << id << " " << opt.num_buf << endl;

  //fHor_but->AddSeparator();
  */

  fFirst = new TGTextButton(fHor_but,"First");
  fFirst->Connect("Clicked()","HistFrame",this,"First()");
  fHor_but->AddFrame(fFirst, fLay5);

  fLast = new TGTextButton(fHor_but,"Last");
  fLast->Connect("Clicked()","HistFrame",this,"Last()");
  fHor_but->AddFrame(fLast, fLay4);

  fmOne = new TGTextButton(fHor_but," -1 ");
  fmOne->Connect("Clicked()","HistFrame",this,"Minus1()");
  fHor_but->AddFrame(fmOne, fLay4);

  fOne = new TGTextButton(fHor_but," +1 ");
  fOne->Connect("Clicked()","HistFrame",this,"Plus1()");
  fHor_but->AddFrame(fOne, fLay4);

  fmNev = new TGTextButton(fHor_but," -N ");
  fmNev->Connect("Clicked()","HistFrame",this,"MinusN()");
  fHor_but->AddFrame(fmNev, fLay4);

  fNev = new TGTextButton(fHor_but," +N ");
  fNev->Connect("Clicked()","HistFrame",this,"PlusN()");
  fHor_but->AddFrame(fNev, fLay4);

  int id;
  id = parpar->Plist.size()+1;
  TGNumberEntry* fNum2 = new TGNumberEntry(fHor_but, 0, 0, id,
					   TGNumberFormat::kNESInteger,
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   1,1000000);
  parpar->DoMap(fNum2->GetNumberEntry(),&opt.num_events,p_inum,0);
  fNum2->Resize(70, fNum2->GetDefaultHeight());
  //fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "HistFrame", this,
  //				   "DoNum()");
  fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  fHor_but->AddFrame(fNum2,fLay4);



  fDeriv[0] = new TGCheckButton(fVer_d, "'", 0);
  fDeriv[1] = new TGCheckButton(fVer_d, "\"", 1);
  fDeriv[0]->SetState((EButtonState) opt.b_deriv[0]);
  fDeriv[1]->SetState((EButtonState) opt.b_deriv[1]);
  fDeriv[0]->Connect("Clicked()","HistFrame",this,"DoChkDeriv()");
  fDeriv[1]->Connect("Clicked()","HistFrame",this,"DoChkDeriv()");
  fVer_d->AddFrame(fDeriv[0], fLay4);
  fVer_d->AddFrame(fDeriv[1], fLay4);
  fDeriv[0]->SetToolTipText("Show 1st derivative");
  fDeriv[1]->SetToolTipText("Show 2nd derivative");

  fPeak[0] = new TGCheckButton(fVer_d, "peaks", 0);
  fPeak[1] = new TGCheckButton(fVer_d, "Pos", 1);
  fPeak[2] = new TGCheckButton(fVer_d, "Time\'", 2);
  fPeak[3] = new TGCheckButton(fVer_d, "Time\"", 3);
  fPeak[4] = new TGCheckButton(fVer_d, "left", 4);
  fPeak[5] = new TGCheckButton(fVer_d, "right", 5);
  fPeak[6] = new TGCheckButton(fVer_d, "Bleft", 6);
  fPeak[7] = new TGCheckButton(fVer_d, "Bright", 7);
  fPeak[8] = new TGCheckButton(fVer_d, "thresh", 8);

  fPeak[0]->SetToolTipText("Show peaks");
  fPeak[1]->SetToolTipText("Peak Position");
  fPeak[2]->SetToolTipText("Exact time from 1st derivative");
  fPeak[3]->SetToolTipText("Exact time from 2nd derivative");
  fPeak[4]->SetToolTipText("Left border of the peak integration");
  fPeak[5]->SetToolTipText("Right border of the peak integration");
  fPeak[6]->SetToolTipText("Left border of the background integration");
  fPeak[7]->SetToolTipText("Right border of the background integration");
  fPeak[8]->SetToolTipText("Threshold (only in 1st derivative)");

  for (int i=0;i<9;i++) {
    fPeak[i]->SetState((EButtonState) opt.b_peak[i]);
    fPeak[i]->Connect("Clicked()","HistFrame",this,"DoChkPeak()");
    fVer_d->AddFrame(fPeak[i], fLay4);
  }


  fVer_ch = new TGVerticalFrame(fHor_st, 10, 10);
  fHor_st->AddFrame(fVer_ch, fLay4);

  fLabel2 = new TGLabel(fVer_ch, "Channels");
  fVer_ch->AddFrame(fLabel2, fLay4);

  fHor_ch = new TGHorizontalFrame(fVer_ch, 10, 10);
  fVer_ch->AddFrame(fHor_ch, fLay1);

  for (int i=0;i<MAX_CH;i++) {
    fChn[i] = NULL;
  }

  int nx=(MAX_CH-1)/16+1;
  int k;
  for (int i=0;i<nx;i++) {
    fVer[i] = new TGVerticalFrame(fHor_ch, 10, 10);
    fHor_ch->AddFrame(fVer[i], fLay1);

    for (int j=0;j<16;j++) {
      k=i*16+j;
      if (k<MAX_CH) {
	sprintf(ss,"%d",k);
	fChn[k] = new TGCheckButton(fVer[i], ss, k);
	int col=gROOT->GetColor(opt.color[k])->GetPixel();
	int fcol=0xffffff-col;
	//printf("Color: %d %x %x\n",i,col,fcol);

	fChn[k]->SetBackgroundColor(col);
	fChn[k]->SetForegroundColor(fcol);
	fChn[k]->Connect("Clicked()","HistFrame",this,"DoPulseOff()");
	fChn[k]->SetState(kButtonDisabled);
	fVer[i]->AddFrame(fChn[k], fLay6);
      }
    }
  }


  fStat1=new TGStatusBar(fVer_st,10,10);
  fStat2=new TGStatusBar(fVer_st,10,10);
  fVer_st->AddFrame(fStat1,fLay3);
  fVer_st->AddFrame(fStat2,fLay3);


  //trd=0;
  //cout << "fVer0->Opt: "<<fVer0->GetOptions() << endl;

  //cout << "fCanvas: " << fCanvas << endl;

  for (int i=0;i<MAX_CH;i++) {
    for (int j=0;j<3;j++) {
      hh[j][i]=0;
      sprintf(ss,"ch_%02d_%d",i,j);
      hh[j][i]=new TH1F(ss,ss,10,0.,10.);
      //hh[j][i]->SetLineColor(i+1);
      hh[j][i]->SetDirectory(0);
    }
  }  

  /*
    for (int i=0;i<MAX_CH;i++) {
    for (int j=0;j<3;j++) {
    sprintf(ss,"gr_%d_%d",i,j);
    Gr[j][i]=TGraph(ss,ss,10,0.,10.);
    hh[j][i]->SetLineColor(i+1);
    }
    }  
  */

  for (int j=0;j<3;j++) {
    sprintf(ss,"hst_%d",j);
    hst[j]=new THStack(ss,"");
  }

  //for (int j=0;j<3;j++) {
  ////sprintf(ss,"mgr_%d",j);
  ////mgr[j]=new TMultiGraph(ss,ss);
  //mgr[j]=0;
  //}

  //StartThread();
  
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
  fCanvas->GetCanvas()->Clear();
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

  TCanvas *cv=fCanvas->GetCanvas();
  cv->Clear();

  //printf("Draw0:\n");

  //Emut->Lock();
  if (Levents->empty()) {
    //Emut->UnLock();
    //txt.DrawTextNDC(0.2,0.7,"Empty event");
    cv->Update();
    return;
  }

  //printf("Draw1:\n");

  ndiv=1;

  if (d_event->pulses.empty()) {
    //TText tt;
    //txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    //Emut->UnLock();
    return;
  }

  //printf("Draw22: %lld\n", d_event->T);

  FillHist(0);

  if (opt.b_deriv[0]) {
    FillHist(1);
    ndiv++;
  }    
  if (opt.b_deriv[1]) {
    FillHist(2);
    ndiv++;
  }


  //cout << "hst: " << hst[0]->GetNhists() << endl;

  cv->Divide(1,ndiv);
  for (int j=0;j<ndiv;j++) {
    cv->cd(j+1);
    hst[j]->Draw("nostack");
    DrawPeaks(hst[j]->GetMinimum("nostack"),
	      hst[j]->GetMaximum("nostack")*1.05);
  }

  //cv->cd(1);
  //char ss[99];
  //sprintf(ss,"Event %lld",d_event->T);
  //txt.DrawTextNDC(0.3,0.92,ss);

  char ss[99];
  sprintf(ss,"Evt: %lld",d_event->Nevt);//std::distance(Levents->begin(),d_event));
  fStat1->SetText(ss);

  sprintf(ss,"Tstamp: %lld",d_event->T);
  fStat2->SetText(ss);
  
  
  cv->Update();

  //Emut->UnLock();

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
