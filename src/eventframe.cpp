//----- EventFrame ----------------
#include "eventframe.h"
#include "romana.h"
#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
//#include <TROOT.h>


#include <TRandom.h>

TLine ln1;

extern Toptions opt;
extern MyMainFrame *myM;
extern BufClass* Buffer;

extern CRS* crs;

Float_t xgr[DSIZE]; // x coordinate for graph
Float_t ygr[DSIZE]; // y coordinate for graph

char hname[3][MAX_CH+1][20]; //+all
 
TText txt;

TMutex *Emut;

void doXline(TLine* ln, Float_t xx, Float_t y1, Float_t y2, int col, int type) {
  ln->SetLineColor(col);
  ln->SetLineStyle(type);
  ln->DrawLine(xx,y1,xx,y2);
}

EventFrame::EventFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kHorizontalFrame)
{

  Emut = new TMutex();

  for (int i=0;i<3;i++) {
    for (int j=0;j<MAX_CH;j++) {
      sprintf(hname[i][j],"ch_%02d_%01d",j,i);
    }
  }
  
  ntab=nt;
  // build widgets

  //bprint=false;

  //cout << "EventFrame: " << p->GetName() << endl;
  
  char ss[100];

  fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
  fLay2 = new TGLayoutHints(kLHintsExpandY,2,2,2,2);
  fLay3 = new TGLayoutHints(kLHintsExpandX,2,2,2,2);
  fLay4 = new TGLayoutHints(kLHintsLeft,2,2,2,2);
  fLay5 = new TGLayoutHints(kLHintsLeft,10,2,2,2);
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

  fHslider->Connect("PositionChanged()", "EventFrame", 
		    this, "DoSlider()");

  fHor_but = new TGHorizontalFrame(fVer0, 10, 10);
  fVer0->AddFrame(fHor_but, fLay4);

  freset = new TGTextButton(fHor_but,"Reset");
  freset->Connect("Clicked()","EventFrame",this,"DoReset()");
  fHor_but->AddFrame(freset, fLay4);

  f1buf = new TGTextButton(fHor_but,"1 buf");
  f1buf->Connect("Clicked()","EventFrame",this,"Do1buf()");
  fHor_but->AddFrame(f1buf, fLay4);

  fNbuf = new TGTextButton(fHor_but,"N buf");
  fNbuf->Connect("Clicked()","EventFrame",this,"DoNbuf()");
  fHor_but->AddFrame(fNbuf, fLay4);

  //fHor_but->AddSeparator();

  fFirst = new TGTextButton(fHor_but,"First");
  fFirst->Connect("Clicked()","EventFrame",this,"First()");
  fHor_but->AddFrame(fFirst, fLay5);

  fmNev = new TGTextButton(fHor_but," -10 ");
  fmNev->Connect("Clicked()","EventFrame",this,"MinusN()");
  fHor_but->AddFrame(fmNev, fLay4);

  fmOne = new TGTextButton(fHor_but," -1 ");
  fmOne->Connect("Clicked()","EventFrame",this,"Minus1()");
  fHor_but->AddFrame(fmOne, fLay4);

  fOne = new TGTextButton(fHor_but," +1 ");
  fOne->Connect("Clicked()","EventFrame",this,"Plus1()");
  fHor_but->AddFrame(fOne, fLay4);

  fNev = new TGTextButton(fHor_but," +10 ");
  fNev->Connect("Clicked()","EventFrame",this,"PlusN()");
  fHor_but->AddFrame(fNev, fLay4);

  fLast = new TGTextButton(fHor_but,"Last");
  fLast->Connect("Clicked()","EventFrame",this,"Last()");
  fHor_but->AddFrame(fLast, fLay4);

  fDeriv[0] = new TGCheckButton(fVer_d, "'", 0);
  fDeriv[1] = new TGCheckButton(fVer_d, "\"", 1);
  fDeriv[0]->SetState((EButtonState) opt.b_deriv[0]);
  fDeriv[1]->SetState((EButtonState) opt.b_deriv[1]);
  fDeriv[0]->Connect("Clicked()","EventFrame",this,"DoChkDeriv()");
  fDeriv[1]->Connect("Clicked()","EventFrame",this,"DoChkDeriv()");
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
    fPeak[i]->Connect("Clicked()","EventFrame",this,"DoChkPeak()");
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
	fChn[k]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");
	fChn[k]->SetState(kButtonDisabled);
	fVer[i]->AddFrame(fChn[k], fLay6);
      }
    }
  }


  fStat=new TGStatusBar(fVer_st,10,10);
  fVer_st->AddFrame(fStat,fLay3);


  trd=0;
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

  StartThread();
  
}

EventFrame::~EventFrame()
{

  //cout << "~Emut" << endl;
  delete Emut;
  //gSystem->Sleep(100);

  if (trd) {
    trd->Delete();
  }

  //cout << "~trd" << endl;
  //gSystem->Sleep(100);

  cout << "~EventFrame()" << endl;
  //fMain->DeleteWindow();

  //myM->fEv=NULL;
}


extern EventFrame* EvtFrm;

void *trd_handle(void* ptr)
{
  //static int nn;

  //TEllipse * el1 = new TEllipse(0.25,0.25,.10,.20);
  //el1->SetFillColor(6);
  //el1->SetFillStyle(3008);

  while (true) {
    //nn++;
    //cout << "trd2: " << nn << " " << myM->fTab->GetCurrent() << " " <<
    //EvtFrm->ntab << endl; 
    if (crs->b_acq && myM && myM->fTab->GetCurrent()==EvtFrm->ntab) {
      //cout << "trd2: " << nn << " " << myM->fTab->GetCurrent() << endl; 

      /*
	TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
	//el1->DrawEllipse();
	double xx=gRandom->Rndm();
	double yy=gRandom->Rndm();
	//cv->Update();
	//cv->cd();
	cv->Clear();
	TText tt;
	tt.DrawTextNDC(xx,yy,"No pulses");
	cv->Update();
      */

      //TSytem::IgnoreSignal();

      /*
	for (int i=0;i<15;i++) {
	gSystem->IgnoreSignal((ESignals)i);
	}
      */

      /*
	kSigBus,
	kSigSegmentationViolation,
	kSigSystem,
	kSigPipe,
	kSigIllegalInstruction,
	kSigQuit,
	kSigInterrupt,
	kSigWindowChanged,
	kSigAlarm,
	kSigChild,
	kSigUrgent,
	kSigFloatingException,
	kSigTermination,
	kSigUser1,
	kSigUser2
      */

      //cout << "Block: " << EvtFrm->BlockAllSignals(true) << endl;

      //opt.b_deriv[1] = !opt.b_deriv[1];

      //Emut.Lock();




      EvtFrm->DrawEvent2();
      //Emut.UnLock();


      //Emut.UnLock();

      //gSystem->Sleep(50);

      //TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
      //cv->Update();


    }
    else {
      //cout << "trd1: " << nn << " " << myM->fTab->GetCurrent() << endl;
    }

    //cout << "Block: " << EvtFrm->BlockAllSignals(false) << endl;

    gSystem->Sleep(opt.tsleep);

  }

  return 0;

}


void EventFrame::StartThread()
{
  trd = new TThread("trd", trd_handle, (void*) 0);
  trd->Run();
}

void EventFrame::CloseWindow()
{
  delete this;
}

void EventFrame::DoReset() {

  myM->DoReset();

}

void EventFrame::Do1buf() {

  myM->Do1buf();

}

void EventFrame::DoNbuf() {

  myM->DoNbuf();

}

void EventFrame::Plus1() {
  ievent++;
  if (ievent >= Buffer->nev) {
    ievent=Buffer->nev-1;
  }
  DrawEvent();
}

void EventFrame::Minus1() {
  ievent--;
  if (ievent < 0) {
    ievent=0;
  }
  DrawEvent();
}

void EventFrame::PlusN() {
  ievent+=10;
  if (ievent >= Buffer->nev) {
    ievent=Buffer->nev-1;
  }
  DrawEvent();
}

void EventFrame::MinusN() {
  ievent-=10;
  if (ievent < 0) {
    ievent=0;
  }
  DrawEvent();
}

void EventFrame::First() {
  ievent=0;
  DrawEvent();
}

void EventFrame::Last() {
  ievent=Buffer->nev-1;
  DrawEvent();
}

void EventFrame::Clear()
{
  fCanvas->GetCanvas()->Clear();
}

void EventFrame::DoSlider() {

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

void EventFrame::DoChkDeriv() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  //cout << "DoDeriv: " << id << endl;

  opt.b_deriv[id] = !opt.b_deriv[id];
  btn->SetState((EButtonState) opt.b_deriv[id]);

  //DrawEvent();

}

void EventFrame::DoChkPeak() {

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

void EventFrame::DoPulseOff() {

  //TGButton *btn = (TGButton *) gTQSender;
  //Int_t id = btn->WidgetId();
  //cout << "DoPulseOff: " << id << " " << fChn[id]->IsOn() << endl;

  ReDraw();

}

void EventFrame::FillHist(std::list<EventClass1>::reverse_iterator evt, int dr) {
  Float_t dat[40000];
  Float_t *pdat=0;

  TText txt;
  
  //const char* hs_name[3] = {"hst0","hst1","hst2"};
  
  if (hst[dr]) {
    delete hst[dr];
  }
  hst[dr]=new THStack("hst0","");

  //BADhst[dr]->Clear();
  //if (!dr)
  //hst[dr]->SetTitle(ss);
  
  for (UInt_t i=0;i<evt->pulses.size();i++) {
    int dt=evt->pulses.at(i).Tstamp64 - evt->T;
    //cout << "Dt: " << i << " " << evt->T << " " << dt
    // << " " << evt->pulses.size() << " " << hst[dr]->GetNhists() << endl;
    PulseClass2 *pulse = &evt->pulses.at(i);
    UInt_t ch= pulse->Chan;

    if (dr==0) {
      pdat=&pulse->sData[0];
    }
    else if (dr==1) {

      UInt_t kk=opt.kderiv[ch];
      if (kk<1 || kk>=pulse->sData.size()) kk=1;

      //dat = new Float_t[pulse->sData.size()];
      for (UInt_t j=0;j<pulse->sData.size();j++) {
	if (j<kk)
	  dat[j]=pulse->sData[j]-pulse->sData[0];
	//dat[j]=0;
	else
	  dat[j]=pulse->sData[j]-pulse->sData[j-kk];
      }
      pdat=dat;
    }
    else if (dr==2) {
      //dat = new Float_t[pulse->sData.size()];
      dat[0]=0;
      dat[1]=0;
      for (UInt_t j=2;j<pulse->sData.size();j++) {
	dat[j]=pulse->sData[j]-2*pulse->sData[j-1]+pulse->sData[j-2];
      }
      pdat=dat;
    }

    if (hh[dr][ch]) hh[dr][ch]->Delete();
    hh[dr][ch] = new TH1F(hname[dr][pulse->Chan],"",
    			pulse->sData.size(),dt,dt+pulse->sData.size());
    hh[dr][ch]->SetDirectory(0);
    //TAxis* X = hh[dr][ch]->GetXaxis();
    hh[dr][ch]->SetLineColor(ch+1);
    //X->Set(pulse->sData.size(),dt,dt+pulse->sData.size());
    //hh[dr][ch]->Rebuild();


    //hh[dr][ch]->Set(pulse->sData.size(),pdat);

    for (UInt_t j=0;j<pulse->sData.size();j++) {
      hh[dr][ch]->SetBinContent(j+1,pdat[j]);
    }

    hst[dr]->Add(hh[dr][ch]);
    //TH1F* hi = (TH1F*) hh[dr][ch]->Clone();
    //hst[dr]->Add(hi);

    //if (dr) delete[] dat;
  } 
}

void EventFrame::DrawEvent2() {

  Emut->Lock();
  //crs->b_pevent=false;
  //EventClass1* d_event;
  if (crs->Levents.empty()) {
    Emut->UnLock();
    return;
  }

  std::list<EventClass1>::reverse_iterator evt;
  UInt_t nn=0;
  for (evt=crs->Levents.rbegin();evt!=crs->Levents.rend();evt++) {
    nn++;
    if (nn>2) break;
  }

  //printf("Draw1:\n");

  TCanvas *cv=fCanvas->GetCanvas();

  cv->Clear();

  ndiv=1;

  if (evt->pulses.empty()) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    Emut->UnLock();
    return;
  }

  //printf("Draw22: %lld\n", evt->T);

  FillHist(evt,0);

  if (opt.b_deriv[0]) {
    FillHist(evt,1);
    ndiv++;
  }    
  if (opt.b_deriv[1]) {
    FillHist(evt,2);
    ndiv++;
  }


  //cout << "hst: " << hst[0]->GetNhists() << endl;

  cv->Divide(1,ndiv);
  for (int j=0;j<ndiv;j++) {
    cv->cd(j+1);
    hst[j]->Draw("nostack");
    DrawPeaks(evt,hst[j]->GetMinimum("nostack"),
	      hst[j]->GetMaximum("nostack")*1.05);
  }

  //cv->cd(1);
  //char ss[99];
  //sprintf(ss,"Event %lld",evt->T);
  //txt.DrawTextNDC(0.3,0.92,ss);

  char ss[99];
  sprintf(ss,"Tstamp: %lld",evt->T);
  fStat->SetText(ss);
  
  
  cv->Update();

  Emut->UnLock();

} //DrawEvent2

void EventFrame::DrawPeaks(std::list<EventClass1>::reverse_iterator evt,
			   double y1,double y2) {

  //gPad->Update();
  //double ux1=gPad->GetUxmin();
  //double ux2=gPad->GetUxmax();
  //double uy1=gPad->GetUymin();
  //double uy2=gPad->GetUymax();

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    int dt=evt->pulses.at(i).Tstamp64 - evt->T;
    //cout << "Dt: " << i << " " << evt->T << " " << dt
    // << " " << evt->pulses.size() << " " << hst[dr]->GetNhists() << endl;
    PulseClass2 *pulse = &evt->pulses.at(i);
    //UInt_t ch= pulse->Chan;
    for (UInt_t j=0;j<pulse->Peaks.size();j++) {
      peak_type *pk = &pulse->Peaks[j];
      doXline(&ln1,pk->Pos+dt,y1,y2,2,1);
      doXline(&ln1,pk->T1+dt,y1,y2,3,2);
      doXline(&ln1,pk->T2+dt,y1,y2,4,3);
      //cout <<"DrawPeaksT2: " << pk->T2+dt << endl;
    }
  }
  //cout <<"DrawPeaks: " << evt->T0 << endl;
  if (evt->T0>0.1) {
    doXline(&ln1,evt->T0,y1,y2,7,1);
  }

}

void EventFrame::DrawEvent() {

  //opt.b_deriv[id] = !opt.b_deriv[id];

  cout << "b_deriv0: " << fDeriv[0]->GetState() << endl;
  cout << "b_deriv1: " << fDeriv[1]->GetState() << endl;

  opt.b_deriv[0] = fDeriv[0]->GetState();
  opt.b_deriv[1] = fDeriv[1]->GetState();

  //char ss[100];

  //cout << "Draw: " << crs->npulses << " " << crs->iBP << " " << crs->iEV << endl;

  //cout << this << endl;
  //return;

  //printf("Draw: %lld %d\n", crs->npulses, crs->iBP);

  //return;
  /*
    for (int i=0;i<MAX_CH;i++) {
    cout << "fChn: " << i << endl;
    cout << "fChn: " << fChn[i] << endl;
    fChn[i]->SetState(kButtonDisabled);
    }

    return;
  */

  printf("Draw1:\n");

  TCanvas *cv=fCanvas->GetCanvas();
  //cout << "cv: " << cv << endl;

  cv->Clear();

  ndiv=0;

  /*
    if (!Buffer->nev) {
    //cout << "No Events" << endl;
    TText tt;
    tt.DrawTextNDC(0.2,0.7,"No events in this buffer");
    cv->Update();
    return;
    }
  */

  d_event = crs->BEvents+crs->iEV;
  //event=Buffer->Bevents+ievent;

  //sprintf(ss,"Buffer: %d, Event: %d, Tstamp: %lld",
  //	  Buffer->nEbuf-1,ievent,event->Tstamp64);
  //fMain->SetWindowName(ss);

  if (d_event->nEpulses==0) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    return;
  }

  //cout << "Drawevent: " << event->nEpulses << endl;
  //return;

  //sprintf(ss,"Ev: %u Pk: %u Tst: %u", nevent, nPk, tstamp);
  //cout << "nevent2: " << nevent << " " << tstamp << endl;

  printf("Draw2: %d\n", d_event->nEpulses);
  gx1= 1e99;
  gx2= -1e99;
  DoGraph(ndiv,0);
  if (opt.b_deriv[0]) {
    ndiv++;
    DoGraph(ndiv,1);
  }
  if (opt.b_deriv[1]) {
    ndiv++;
    DoGraph(ndiv,2);
  }

  cv->Divide(1,ndiv+1);

  printf("Draw3: %d\n", d_event->nEpulses);

  ReDraw();

  printf("Draw4: %d\n", d_event->nEpulses);

} //DrawEvent

void EventFrame::DoGraph(int ndiv, int dd) {

  char ss[100];
  const char* title[] = {"Pulse","1st deriv","2nd deriv"};

  gy1[ndiv]= 1e99;
  gy2[ndiv]= -1e99;

  //NGr=0;

  //cout << "DoGraph 1 ##############: " << event->nEpulses << endl;

  divtype[ndiv]= dd;

  for (int i=0;i<d_event->nEpulses;i++) {

    PulseClass *pulse = d_event->Epulses[i];
    if (pulse->Nsamp==0) {
      Gr[ndiv][i]= TGraph(0);
      //Gr[ndiv][i].Set(0);
      continue;
    }
    int chn=pulse->Chan;

    //printf("DoGraph: %d %d %d\n",dd,i,pulse->Nsamp);
    //cout << pulse->sData << endl;

    switch (dd) {
    case 0:

      fChn[chn]->SetState(kButtonDown);
      for (int j=0;j<pulse->Nsamp;j++) {
	//ygr[j]=pulse->sData[j]-pulse->sData[0];
	ygr[j]=pulse->sData[j];
	xgr[j]=j+pulse->tdif;
      }
      Gr[ndiv][i]= TGraph (pulse->Nsamp, xgr, ygr);
      sprintf(ss,"ch %d",chn);
      //}
      break;
    case 1:
      //else {
      ygr[0]=0;
      xgr[0]=pulse->tdif;
      for (int j=1;j<pulse->Nsamp;j++) {
	ygr[j]=pulse->sData[j]-pulse->sData[j-1];
	xgr[j]=j+pulse->tdif;
      }
      Gr[ndiv][i]= TGraph (pulse->Nsamp, xgr, ygr);
      sprintf(ss,"ch %d deriv",chn);
      //}
      break;
    case 2:
      ygr[0]=0;
      ygr[1]=0;
      xgr[0]=pulse->tdif;
      xgr[1]=1+pulse->tdif;
      for (int j=2;j<pulse->Nsamp;j++) {
	ygr[j]=pulse->sData[j]-2*pulse->sData[j-1]+pulse->sData[j-2];
	xgr[j]=j+pulse->tdif;
      }
      Gr[ndiv][i]= TGraph (pulse->Nsamp, xgr, ygr);
      sprintf(ss,"ch %d deriv2",chn);
      //}
      break;
    }
    int col=opt.color[chn];
    cout << "Color: " << i << " " << col << endl;
    Gr[ndiv][i].SetLineColor(col);
    Gr[ndiv][i].SetMarkerStyle(20);
    Gr[ndiv][i].SetMarkerSize(0.5);
    Gr[ndiv][i].SetMarkerColor(col);
    //Gr[ndiv][i]->SetFillColor(0);
    //Gr[ndiv][i]->SetLineWidth(3);
    //Gr[ndiv][i].SetTitle(ss);

    //printf("Chan: %d %d %d\n",i,chn,col);
    //delete gr1;

    double x1,x2,y1,y2;
    Gr[ndiv][i].ComputeRange(x1,y1,x2,y2);
    if (x1-1<gx1) gx1=x1-1;
    if (x2+1>gx2) gx2=x2+1;
    if (y1<gy1[ndiv]) gy1[ndiv]=y1;
    if (y2>gy2[ndiv]) gy2[ndiv]=y2;

    //printf("MAXmin: %d %f %f %f %f\n",i, x1,x2,y1,y2);

    //NGr++;

  } //for
  //malloc_stats();

  //printf("MAXmin: %f %f %f %f\n",gx1,gx2,gy1,gy2);

  gy1[ndiv]*=1.2;
  gy2[ndiv]*=1.2;

  sprintf(ss,"fPaint%d",dd);
  fPaint[ndiv]=TH2F(ss,title[dd],100,gx1,gx2,100,gy1[ndiv],gy2[ndiv]);
  fPaint[ndiv].SetBit(TH1::kNoStats);
  fPaint[ndiv].SetDirectory(0);

  //fPaint[ndiv].SetTitleSize(0.91);
  gStyle->SetTitleFontSize(0.07);

  fPaint[ndiv].SetLabelSize(0.05,"X");
  fPaint[ndiv].SetLabelSize(0.05,"Y");

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

void EventFrame::ReDraw() {

  //EventClass* event=Buffer->Bevents+ievent;

  if (d_event->nEpulses==0) {
    return;
  }

  TCanvas *cv=fCanvas->GetCanvas();

  //ndiv=cv->GetListOfPrimitives()->GetEntries();
  //cout << "Pads: " << ndiv << endl;


  TLine lpk  = TLine();
  TLine lb1  = TLine();
  TLine lb2  = TLine();
  TLine lthr = TLine();

  float x1,x2;
  fHslider->GetPosition(x1,x2);

  //printf("ReDraw: %d %d\n",ndiv, ievent);
  //cout << event << endl;

  float dx=gx2-gx1;
  float dx1=dx*x1;
  float dx2=dx*x2;

  for (int n=0;n<ndiv+1;n++) {
    fPaint[n].GetXaxis()->SetRangeUser(gx1+dx1,gx1+dx2);
  }

  for (int n=0;n<ndiv+1;n++) {
    cv->cd(n+1);
    fPaint[n].Draw("AXIS");
    gPad->Update();

    double ux1=gPad->GetUxmin();
    double ux2=gPad->GetUxmax();
    double uy1=gPad->GetUymin();
    double uy2=gPad->GetUymax();

    //printf("ReDraw: %f %f\n",uy1,uy2);

    for (int i=0;i<d_event->nEpulses;i++) {
      if (Gr[n][i].GetN()) {
	PulseClass *pulse = d_event->Epulses[i];
	int chn=pulse->Chan;

	if (fChn[chn]->IsOn()) {
	  Gr[n][i].Draw("LP");
	  if (opt.b_peak[0]) {

	    int col=opt.color[chn];
	    //lpk.SetLineColor(col);
	    for (int j=0;j<pulse->Npeaks;j++) {
	      if (opt.b_peak[1]) {//Pos
		doXline(&lpk,pulse->Peaks[j].Pos+pulse->tdif,uy1,uy2,col,1);
	      }
	      if (opt.b_peak[2] && divtype[n]==1) {//Time
		doXline(&lpk,pulse->Peaks[j].Time+pulse->tdif,uy1,uy2,col,2);
	      }
	      if (opt.b_peak[3] && divtype[n]==2) {//Time2
		doXline(&lpk,pulse->Peaks[j].Time2+pulse->tdif,uy1,uy2,col,2);
	      }
	      if (opt.b_peak[4]) {//left
		//doXline(&lpk,pulse->Peaks[j].Time2+pulse->tdif,uy1,uy2,col,1);
	      }
	    }
	    if (opt.b_peak[8] && divtype[n]==1) {
	      int col=opt.color[chn];
	      lthr.SetLineColor(col);
	      lthr.SetLineStyle(3);
	      //lthr.DrawLine(ux1,opt.ng_thresh,ux2,opt.ng_thresh);
	    }

	  } //if opt.b_peak[0]
	}
      }
    } //for i
  }

  cv->Update();
  
}
