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

Float_t RGB[MAX_CH][3] = {
  {0,0,0},//0
  {1,0,0},
  {0,1,0},
  {0,0,1},
  {0,1,1},
  {1,0,1}, //5
  {0.63,0.67,0.02},
  {0.71,0.11,0.52},
  {0.51,0.95,0.46},
  {0.16,0.71,0.77},
  {0.77,0.96,0.16},//10
  {0.68,0.61,0.98},
  {0.15,0.45,0.74},
  {0.03,0.62,0.07},
  {0.47,0.37,0.50},
  {0.72,0.34,0.04},//15
  {0.51,0.54,0.59},
  {0.52,0.78,0.11},
  {0.01,0.86,0.66},
  {0.78,0.69,0.14},
  {0.91,0.65,0.90},//20
  {0.16,0.30,0.29},
  {0.41,0.69,0.03},
  {0.77,0.22,0.80},
  {0.40,0.37,0.98},
  {0.56,0.06,0.34},//25
  {0.89,0.64,0.79},
  {0.24,0.99,0.28},
  {0.84,0.89,0.10},
  {0.12,0.25,0.18},
  {0.97,0.85,0.74},//30
  {0.55,0.29,0.20}
  
};

Int_t chcol[MAX_CH];
//                     1                        11 
// = {1,2,3,4,6,7,8,9,406,426,596,606,636,796,816,856,
//    //17                    23
//    906,415,511,623,742,805,392,259,835,901,
//    //27
//    259,491,635,819,785,513
//};

const char* mgr_name[3] = {"pulse","1deriv","2deriv"};
const char* mgr_title[3] = {"pulse;Time(ns)","1 deriv;Time(ns)",
			    "2 deriv;Time(ns)"};

extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
//extern BufClass* Buffer;

extern CRS* crs;
extern ParParDlg *parpar;

extern ULong_t fGreen;
extern ULong_t fRed;
extern ULong_t fCyan;

Float_t xgr[DSIZE]; // x coordinate for graph
Float_t ygr[DSIZE]; // y coordinate for graph

char hname[3][MAX_CH+1][20]; //+all
 
TText txt;

//TMutex *Emut;

void doXline(TLine* ln, Float_t xx, Float_t y1, Float_t y2, int col, int type) {
  xx*=opt.period;
  ln->SetLineColor(col);
  ln->SetLineStyle(type);
  ln->DrawLine(xx,y1,xx,y2);
}

EventFrame::EventFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kHorizontalFrame)
{

  // ULong_t fcol;
  // int nn=0;
  // for (int i=0;i<255;i+=80)
  //   for (int j=0;j<255;j+=80)
  //     for (int k=0;k<255;k+=80) {
  // 	if (nn>=MAX_CH)
  // 	  break;
  // 	fcol=TColor::RGB2Pixel(i,j,k);
  // 	chcol[nn]=TColor::GetColor(fcol);
  // 	cout << "chcol: " << nn << " " << chcol[nn] << " " << fcol << endl;
  // 	nn++;
  //     }

  // chcol[0]=TColor::GetColor(TColor::RGB2Pixel(0,0,0));
  // chcol[1]=TColor::GetColor(TColor::RGB2Pixel(255,0,0));
  // chcol[2]=TColor::GetColor(TColor::RGB2Pixel(0,255,0));
  // chcol[3]=TColor::GetColor(TColor::RGB2Pixel(0,0,255));
  // chcol[4]=TColor::GetColor(TColor::RGB2Pixel(255,0,255));
  // chcol[5]=TColor::GetColor(TColor::RGB2Pixel(0,255,255));

  /*
  gRandom->SetSeed(0);
  for (int i=1;i<MAX_CH;i++) {
    RGB[i][0]=gRandom->Rndm();
    RGB[i][1]=gRandom->Rndm();
    RGB[i][2]=gRandom->Rndm();
    printf("%2d {%0.2f,%0.2f,%0.2f},\n",i,RGB[i][0],RGB[i][1],RGB[i][2]);
  }
  */  

  for (int i=0;i<MAX_CH;i++) {
    chcol[i]=TColor::GetColor(TColor::RGB2Pixel(RGB[i][0],RGB[i][1],RGB[i][2]));
  }

  //Levents = &Tevents;
  Levents = &crs->Levents;
  d_event = Levents->begin();
  cout << "d_event: " << &(*d_event) << endl;
  //d_event = new EventClass();
  //Emut = new TMutex();

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
  fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
  fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
  fLay7 = new TGLayoutHints(kLHintsCenterX,0,0,0,0);

  //Frames.....

  TGVerticalFrame        *fVer0; //contains canvas, fHor1
  TGVerticalFrame        *fVer_d; //contains deriv etc
  TGVerticalFrame        *fVer_ch; //contains fHor_ch
  TGHorizontalFrame      *fHor_ch; //contains fVer[..]
  //TGVerticalFrame        *fVer[(MAX_CH-1)/16+1];
  TGVerticalFrame        *fVer1;

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

  /*
  
  //freset = new TGTextButton(fHor_but,"Reset");
  //freset->Connect("Clicked()","EventFrame",this,"DoReset()");
  //fHor_but->AddFrame(freset, fLay4);

  f1buf = new TGTextButton(fHor_but,"1 buf");
  f1buf->Connect("Clicked()","EventFrame",this,"Do1buf()");
  fHor_but->AddFrame(f1buf, fLay4);

  fNbuf = new TGTextButton(fHor_but,"N buf");
  fNbuf->ChangeBackground(fGreen);
  fNbuf->Connect("Clicked()","EventFrame",this,"DoNbuf()");
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
  //fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "EventFrame", this,
  //				   "DoNum()");
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  fHor_but->AddFrame(fNum1,fLay4);

  //cout << "num_buf: " << id << " " << opt.num_buf << endl;

  //fHor_but->AddSeparator();
  */

  fFirst = new TGTextButton(fHor_but,"First");
  fFirst->Connect("Clicked()","EventFrame",this,"First()");
  fHor_but->AddFrame(fFirst, fLay5);

  fLast = new TGTextButton(fHor_but,"Last");
  fLast->Connect("Clicked()","EventFrame",this,"Last()");
  fHor_but->AddFrame(fLast, fLay4);

  fmOne = new TGTextButton(fHor_but," -1 ");
  fmOne->Connect("Clicked()","EventFrame",this,"Minus1()");
  fHor_but->AddFrame(fmOne, fLay4);

  fOne = new TGTextButton(fHor_but," +1 ");
  fOne->Connect("Clicked()","EventFrame",this,"Plus1()");
  fHor_but->AddFrame(fOne, fLay4);

  fmNev = new TGTextButton(fHor_but," -N ");
  fmNev->Connect("Clicked()","EventFrame",this,"MinusN()");
  fHor_but->AddFrame(fmNev, fLay4);

  fNev = new TGTextButton(fHor_but," +N ");
  fNev->Connect("Clicked()","EventFrame",this,"PlusN()");
  fHor_but->AddFrame(fNev, fLay4);

  // TGTextButton* fClr = new TGTextButton(fHor_but,"colors...");
  // fClr->Connect("Clicked()","EventFrame",this,"DoColor()");
  // fHor_but->AddFrame(fClr, fLay4);

  int id;
  id = parpar->Plist.size()+1;
  TGNumberEntry* fNum2 = new TGNumberEntry(fHor_but, 0, 0, id,
					   TGNumberFormat::kNESInteger,
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   1,1000000);
  parpar->DoMap(fNum2->GetNumberEntry(),&opt.num_events,p_inum,0);
  fNum2->Resize(70, fNum2->GetDefaultHeight());
  //fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "EventFrame", this,
  //				   "DoNum()");
  fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  fHor_but->AddFrame(fNum2,fLay4);

  opt.b_deriv[0]=true;

  fDeriv[0] = new TGCheckButton(fVer_d, "'", 1);
  fDeriv[1] = new TGCheckButton(fVer_d, "\"", 2);
  fDeriv[0]->SetState((EButtonState) opt.b_deriv[1]);
  fDeriv[1]->SetState((EButtonState) opt.b_deriv[2]);
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

  fVer1 = new TGVerticalFrame(fHor_ch, 10, 10);
  fHor_ch->AddFrame(fVer1, fLay1);

  int nclmn=2;
  int ny=(MAX_CH+1)/nclmn;

  for (int i=0;i<ny;i++) {
    TGHorizontalFrame* fHor1 = new TGHorizontalFrame(fVer1, 10, 10);
    fVer1->AddFrame(fHor1, fLay6);

    for (int j=0;j<nclmn;j++) {
      int k=j*ny+i;

      if (k<MAX_CH) {
	sprintf(ss,"%d",k);

	TGLabel* flab = new TGLabel(fHor1, ss);
	flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
	flab->SetWidth(16);
	fHor1->AddFrame(flab, fLay6);

	fChn[k] = new TGCheckButton(fHor1, " ", k);
	fHor1->AddFrame(fChn[k], fLay6);
	fChn[k]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");

	int col=gROOT->GetColor(chcol[k])->GetPixel();
	//col=1;
	int fcol=0;
	if (k==0 || k==3 || k==21 || k==25 || k==29)
	  fcol=0xffffff;
	
	//printf("Color: %d %d %x %x\n",k,chcol[k],col,fcol);

	flab->SetBackgroundColor(col);
	flab->SetForegroundColor(fcol);

	//fChn[k]->SetBackgroundColor(col);
	//fChn[k]->SetForegroundColor(col);
	fChn[k]->SetState(kButtonDown);

      } //if
    } //for j
  } //for i

  TGHorizontalFrame* fHor1 = new TGHorizontalFrame(fVer1, 10, 10);
  fVer1->AddFrame(fHor1, fLay7);

  fChn[MAX_CH] = new TGCheckButton(fHor1, "all", MAX_CH);
  fHor1->AddFrame(fChn[MAX_CH], fLay6);
  fChn[MAX_CH]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");
  fChn[MAX_CH]->SetState(kButtonDown);
  //TGLabel* flab = new TGLabel(fHor1, "all");
  //flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
  //flab->SetWidth(16);
  //fHor1->AddFrame(flab, fLay6);


  fStat1=new TGStatusBar(fVer_st,10,10);
  fStat2=new TGStatusBar(fVer_st,10,10);
  fVer_st->AddFrame(fStat1,fLay3);
  fVer_st->AddFrame(fStat2,fLay3);


  //trd=0;
  //cout << "fVer0->Opt: "<<fVer0->GetOptions() << endl;

  //cout << "fCanvas: " << fCanvas << endl;

  for (int i=0;i<MAX_CH;i++) {
    for (int j=0;j<3;j++) {
      //sprintf(ss,"gr_%d_%d",i,j);
      Gr[j][i]=new TGraph(10);
      //Gr[j][i]->SetNameTitle(ss,";Time(ns);");
      Gr[j][i]->SetLineColor(chcol[i]);
      Gr[j][i]->SetMarkerColor(chcol[i]);
      Gr[j][i]->SetMarkerStyle(20);
      Gr[j][i]->SetMarkerSize(0.5);
    }
  }  

  mgr[0]=new TMultiGraph();
  mgr[1]=new TMultiGraph();
  mgr[2]=new TMultiGraph();

  //for (int j=0;j<3;j++) {
  ////sprintf(ss,"mgr_%d",j);
  ////mgr[j]=new TMultiGraph(ss,ss);
  //mgr[j]=0;
  //}

  //StartThread();
  
}

EventFrame::~EventFrame()
{

  //cout << "~Emut" << endl;
  //delete Emut;
  //gSystem->Sleep(100);

  //if (trd) {
  //trd->Delete();
  //}

  //cout << "~trd" << endl;
  //gSystem->Sleep(100);

  cout << "~EventFrame()" << endl;
  //fMain->DeleteWindow();

  //myM->fEv=NULL;
}


extern EventFrame* EvtFrm;

// void EventFrame::StartThread()
// {
//   trd = new TThread("trd", trd_handle, (void*) 0);
//   trd->Run();
// }

// void EventFrame::CloseWindow()
// {
//   delete this;
// }

// void EventFrame::DoReset() {
//   myM->DoReset();
// }

/*

void EventFrame::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  if (id==101) {
    opt.num_buf=te->GetNumber();
    cout << "num_buf: " << te->GetNumber() << endl;
  }
  

}

void EventFrame::Do1buf() {

  crs->Do1Buf();

}

void EventFrame::DoNbuf() {

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

/*
void EventFrame::DoColor() {
  TGColorDialog* tg = new TGColorDialog();
}
*/

void EventFrame::First() {
  d_event = Levents->begin();
  DrawEvent2();
}

void EventFrame::Last() {
  d_event = --Levents->end();
  DrawEvent2();
}

void EventFrame::Plus1() {
  ++d_event;
  if (d_event==Levents->end()) {
    --d_event;
  }
  DrawEvent2();
}

void EventFrame::Minus1() {
  if (d_event!=Levents->begin())
    --d_event;
  DrawEvent2();
}

void EventFrame::PlusN() {
  for (int i=0;i<opt.num_events;i++) {
    ++d_event;
    if (d_event==Levents->end()) {
      --d_event;
      break;
    }
  }
  DrawEvent2();
}

void EventFrame::MinusN() {
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

  DrawEvent2();

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

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  if (id==MAX_CH) {
    for (int i=0;i<MAX_CH;i++) {
      fChn[i]->SetState((EButtonState) fChn[id]->IsOn());
    }
  }
  //cout << "DoPulseOff: " << id << " " << fChn[id]->IsOn() << endl;

  ReDraw();

}

void EventFrame::FillGraph(int dr) {
  //Float_t dat[40000];
  //Float_t *pdat=0;

  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    PulseClass *pulse = &d_event->pulses.at(i);
    UInt_t ch= pulse->Chan;

    Gr[dr][ch]->Set(pulse->sData.size());

    double dt=pulse->Tstamp64 - d_event->T - cpar.preWr[ch];

    // if (d_event->pulses.size()>1) {
    //   int nh=0;
    //   if (hst[dr]->GetHists())
    // 	nh=hst[dr]->GetHists()->GetSize();
    //   cout << "Dt: " << i << " " << (int) pulse->Chan << " "
    // 	   << pulse->Tstamp64 << " " << d_event->T << " " << dt
    // 	   << " " << d_event->pulses.size() << " " << nh
    // 	   << endl;
    // }

    //Double_t *xx = Gr[dr][ch]->GetX();
    //Double_t *yy = Gr[dr][ch]->GetY();

    if (dr==0) { //main pulse
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][ch]->GetX()[j]=(j+dt)*opt.period;
	Gr[dr][ch]->GetY()[j]=pulse->sData[j];
	//xx[j]=dt+j;
	//yy[j]=pulse->sData[j];
	// cout << "GR: " << dt << " " << Gr[dr][ch]->GetN() << " "
	//      << ch << " " << dt+j << " "
	//      << pulse->sData[j] << " " 
	//      << Gr[dr][ch]->GetX()[j] << " " 
	//      << Gr[dr][ch]->GetY()[j] << endl; 
	  
      }
    }
    else if (dr==1) { //1st derivaive

      Int_t kk=cpar.kderiv[ch];
      if (kk<1 || kk>=(Int_t)pulse->sData.size()) kk=1;

      //dat = new Float_t[pulse->sData.size()];
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][ch]->GetX()[j]=(j+dt)*opt.period;
	if (j<kk)
	  Gr[dr][ch]->GetY()[j]=pulse->sData[j]-pulse->sData[0];
	//dat[j]=0;
	else
	  Gr[dr][ch]->GetY()[j]=pulse->sData[j]-pulse->sData[j-kk];
      }
    }
    else if (dr==2) { //2nd derivative
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][ch]->GetX()[j]=(j+dt)*opt.period;
	if (j<2)
	  Gr[dr][ch]->GetY()[j]=0;
	//dat[j]=0;
	else
	  Gr[dr][ch]->GetY()[j]=
	    pulse->sData[j]-2*pulse->sData[j-1]+pulse->sData[j-2];
      }
    }
  } 
}

void EventFrame::FillMgr(int dr) {

  if (mgr[dr]->GetListOfGraphs())
    mgr[dr]->GetListOfGraphs()->Clear();

  delete mgr[dr];
  mgr[dr]=new TMultiGraph(mgr_name[dr],mgr_title[dr]);
  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    UInt_t ch= d_event->pulses.at(i).Chan;
    if (fChn[ch]->IsOn()) {
      mgr[dr]->Add(Gr[dr][ch]);
    }
  } 

}

void EventFrame::DrawEvent2() {

  TCanvas *cv=fCanvas->GetCanvas();
  cv->Clear();

  //int nnn=0;
  //if (Levents) nnn=Levents->size();
  //printf("DrawEvent0: %p %d %p %d\n",Levents,nnn,crs->Levents,crs->Levents.size());

  //Emut->Lock();
  if (Levents->empty()) {
    //Emut->UnLock();
    txt.DrawTextNDC(0.2,0.7,"Empty event");
    cv->Update();
    return;
  }

  //printf("Draw1:\n");

  ndiv=0;

  if (d_event->pulses.empty()) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    //Emut->UnLock();
    return;
  }

  //printf("Draw22: %lld\n", d_event->T);

  // FillHstack(0);
  // if (opt.b_deriv[0]) {
  //   FillHstack(1);
  //   ndiv++;
  // }    
  // if (opt.b_deriv[1]) {
  //   FillHstack(2);
  //   ndiv++;
  // }

  cout << "mgr1: " << mgr[0]->GetListOfGraphs() << " " << opt.b_deriv[0] << endl;

  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {
      FillGraph(i);
      ndiv++;
    }
  }

  //cout << "hst: " << hst[0]->GetNhists() << endl;
  //cout << "mgr: " << mgr[0]->GetHistogram() << endl;
  //TGraph* gr = (TGraph*) mgr[0]->GetListOfGraphs()->First();
  //cout << gr->GetN() << " " << gr->GetX()[0] << " " << gr->GetX()[1] << endl;
  
  cv->Divide(1,ndiv);

  ReDraw();
  // int nn=1;
  // for (int i=0;i<3;i++) {
  //   if (opt.b_deriv[i]) {
  //     cout << "mgr3: " << i << " " << nn << " " << mgr[i] << endl;
  //     cv->cd(nn++);
  //     mgr[i]->Draw("alp");
  //     //DrawPeaks(hst[j]->GetMinimum("nostack"),
  //     //      hst[j]->GetMaximum("nostack")*1.05);
  //   }
  // }

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

void EventFrame::DrawPeaks(double y1,double y2) {

  //gPad->Update();
  //double ux1=gPad->GetUxmin();
  //double ux2=gPad->GetUxmax();
  //double uy1=gPad->GetUymin();
  //double uy2=gPad->GetUymax();

  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    PulseClass *pulse = &d_event->pulses.at(i);
    UInt_t ch= pulse->Chan;
    if (fChn[ch]->IsOn()) {
      double dt=pulse->Tstamp64 - d_event->T - cpar.preWr[ch];
      //int dt=d_event->pulses.at(i).Tstamp64 - d_event->T;
      //cout << "Dt: " << i << " " << d_event->T << " " << dt
      // << " " << d_event->pulses.size() << " " << hst[dr]->GetNhists() << endl;
      //UInt_t ch= pulse->Chan;
      for (UInt_t j=0;j<pulse->Peaks.size();j++) {
	peak_type *pk = &pulse->Peaks[j];
	doXline(&ln1,pk->Pos+dt,y1,y2,2,1);
	doXline(&ln1,pk->T1+dt,y1,y2,3,2);
	doXline(&ln1,pk->T2+dt,y1,y2,4,3);
	cout <<"DrawPeaksT2: " << pk->T2+dt << " " << dt << " " 
	     << pulse->Tstamp64 << " " << d_event->T << endl;
      }
    }
  }
  cout <<"DrawPeaks: " << d_event->T0 << endl;
  if (d_event->T0<99998) {
    doXline(&ln1,d_event->T0,y1,y2,5,1);
  }

}

//void EventFrame::DrawEvent() {
//} //DrawEvent

void EventFrame::DoGraph(int ndiv, int dd) {

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

  //cv->Update();
  
  int nn=1;
  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {
      fCanvas->GetCanvas()->cd(nn++);
      FillMgr(i);
      mgr[i]->Draw("alp");
      cout << "mgr3: " << i << " " << nn << " " << fPeak[0]->IsOn() << " "
         << mgr[i]->GetHistogram()<< endl;
      //   << mgr[i]->GetHistogram()->GetMaximum() << endl;

      if (mgr[i]->GetHistogram() && fPeak[0]->IsOn()) {
	DrawPeaks(mgr[i]->GetHistogram()->GetMinimum(),
	      mgr[i]->GetHistogram()->GetMaximum());
      }
    }
  }

  fCanvas->GetCanvas()->Update();
}
