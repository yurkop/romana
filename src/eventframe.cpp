//----- EventFrame ----------------
#include "eventframe.h"
#include "romana.h"
#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
#include <TMarker.h>
#include <TMutex.h>
//#include <TROOT.h>

#include <TRandom.h>

TLine ln;
TMarker mk;

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
Int_t gcol[MAX_CH];
Int_t fcol[MAX_CH];
//                     1                        11 
// = {1,2,3,4,6,7,8,9,406,426,596,606,636,796,816,856,
//    //17                    23
//    906,415,511,623,742,805,392,259,835,901,
//    //27
//    259,491,635,819,785,513
//};

const char* mgr_name[3] = {"pulse","1deriv","2deriv"};
const char* mgr_title[3] = {"pulse;samples","1 deriv;samples",
			    "2 deriv;samples"};

//extern Coptions cpar;
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

TMutex Emut;
TMutex Emut2;

void doXline(Float_t xx, Float_t y1, Float_t y2, int col, int type) {
  ln.SetLineColor(col);
  ln.SetLineStyle(type);
  ln.DrawLine(xx,y1,xx,y2);
}

void doYline(Float_t yy, Float_t x1, Float_t x2, int col, int type) {
  ln.SetLineColor(col);
  ln.SetLineStyle(type);
  ln.DrawLine(x1,yy,x2,yy);
  //cout << "doyline: " << x1 << " " << x2 << " " << yy << " " << col << endl;
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
    gcol[i]=gROOT->GetColor(chcol[i])->GetPixel();
    fcol[i]=0;
    if (i==0 || i==3 || i==21 || i==25 || i==29)
       fcol[i]=0xffffff;
  }


  mk.SetMarkerStyle(3);
  mk.SetMarkerColor(2);

  Pevents = &Tevents;
  //Pevents = &crs->Levents.back();
  d_event = Pevents->begin();
  //cout << "d_event: " << &(*d_event) << endl;
  //d_event = new EventClass();
  //Emut = new TMutex();
  //Emut2 = new TMutex();

  for (int i=0;i<3;i++) {
    fHist[i] = new TH1F();
    fHist[i]->SetDirectory(0);
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
  fLay8 = new TGLayoutHints(kLHintsExpandX,0,0,0,0);
  fLay9 = new TGLayoutHints(kLHintsExpandY,0,0,0,0);

  //Frames.....

  TGVerticalFrame        *fVer0; //contains canvas, Hslider, fHor1
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

  TGHorizontalFrame* fHor2 = new TGHorizontalFrame(fVer0, 10, 10);
  fVer0->AddFrame(fHor2, fLay1);

  fCanvas = new TRootEmbeddedCanvas("Events",fHor2,w,h);
  fHor2->AddFrame(fCanvas, fLay1);

  fHslider = new TGDoubleHSlider(fVer0, 10, kDoubleScaleBoth,0);
  fHslider->SetRange(0,1);
  fHslider->SetPosition(0,1);
  fVer0->AddFrame(fHslider, fLay8);
  fHslider->Connect("PositionChanged()", "EventFrame", 
		    this, "DoSlider()");

  fVslider = new TGDoubleVSlider(fHor2, 10, kDoubleScaleBoth,1);
  fVslider->SetRange(0,1);
  fVslider->SetPosition(0,1);
  fHor2->AddFrame(fVslider, fLay9);
  fVslider->Connect("PositionChanged()", "EventFrame", 
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

  fDeriv[0] = new TGCheckButton(fVer_d, "Pulse", 0);
  fDeriv[1] = new TGCheckButton(fVer_d, " ' ", 1);
  fDeriv[2] = new TGCheckButton(fVer_d, " ' ' ", 2);
  for (int i=0;i<3;i++) {
    fDeriv[i]->SetState((EButtonState) opt.b_deriv[i]);
    fDeriv[i]->Connect("Clicked()","EventFrame",this,"DoChkDeriv()");
    fVer_d->AddFrame(fDeriv[i], fLay4);
  }
  //fVer_d->AddFrame(fDeriv[1], fLay4);
  fDeriv[0]->SetToolTipText("Show pulse");
  fDeriv[1]->SetToolTipText("Show 1st derivative");
  fDeriv[2]->SetToolTipText("Show 2nd derivative");

  fPeak[0] = new TGCheckButton(fVer_d, "Peaks", 0);
  fPeak[1] = new TGCheckButton(fVer_d, "Pos", 1);
  fPeak[2] = new TGCheckButton(fVer_d, "Time", 2);
  //fPeak[3] = new TGCheckButton(fVer_d, "Time\"", 3);
  fPeak[3] = new TGCheckButton(fVer_d, "Wpeak", 3);
  fPeak[4] = new TGCheckButton(fVer_d, "WBkgr", 4);
  fPeak[5] = new TGCheckButton(fVer_d, "WTime", 5);
  //fPeak[6] = new TGCheckButton(fVer_d, "BRight", 6);
  //fPeak[7] = new TGCheckButton(fVer_d, "TLeft", 7);
  //fPeak[8] = new TGCheckButton(fVer_d, "TRight", 8);
  fPeak[6] = new TGCheckButton(fVer_d, "*TStart", 6);
  fPeak[7] = new TGCheckButton(fVer_d, "Thresh", 7);

  //int cc;
  //cc=gROOT->GetColor(2)->GetPixel();
  fPeak[1]->SetForegroundColor(gROOT->GetColor(2)->GetPixel());
  fPeak[2]->SetForegroundColor(gROOT->GetColor(3)->GetPixel());
  fPeak[3]->SetForegroundColor(gROOT->GetColor(4)->GetPixel());
  fPeak[4]->SetForegroundColor(gROOT->GetColor(6)->GetPixel());
  //fPeak[5]->SetForegroundColor(gROOT->GetColor(1)->GetPixel());
  fPeak[6]->SetForegroundColor(gROOT->GetColor(2)->GetPixel());
  //fPeak[7]->SetForegroundColor(gROOT->GetColor(3)->GetPixel());

  
  fPeak[0]->SetToolTipText("Show peaks");
  fPeak[1]->SetToolTipText("Peak Position (maximum in 1st derivative)");
  fPeak[2]->SetToolTipText("Exact time from 1st derivative");
  //fPeak[3]->SetToolTipText("Exact time from 2nd derivative");
  fPeak[3]->SetToolTipText("Window for peak integration (left lower than right)");
  fPeak[4]->SetToolTipText("Window for background integration (left lower than right)");
  fPeak[5]->SetToolTipText("Window for time integration (left lower than right),\nplotted only in 1st derivative");
  // fPeak[3]->SetToolTipText("Left border of the peak integration");
  // fPeak[4]->SetToolTipText("Right border of the peak integration");
  // fPeak[5]->SetToolTipText("Left border of the background integration");
  // fPeak[6]->SetToolTipText("Right border of the background integration");
  // fPeak[7]->SetToolTipText("Left border of the time integration");
  // fPeak[8]->SetToolTipText("Right border of the time integration");
  fPeak[6]->SetToolTipText("Time start marker");
  fPeak[7]->SetToolTipText("Threshold, plotted only in 1st derivative)");

  for (int i=0;i<8;i++) {
    fPeak[i]->SetState((EButtonState) opt.b_peak[i]);
    fPeak[i]->Connect("Clicked()","EventFrame",this,"DoChkPeak()");
    fVer_d->AddFrame(fPeak[i], fLay4);
  }


  fVer_ch = new TGVerticalFrame(fHor_st, 10, 10);
  fHor_st->AddFrame(fVer_ch, fLay4);

  TGLabel* fLabel2 = new TGLabel(fVer_ch, "  Channels        ");
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

	//TGLabel* flab = new TGLabel(fHor1, ss);
	//flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
	//flab->SetWidth(16);
	//fHor1->AddFrame(flab, fLay6);

	fChn[k] = new TGCheckButton(fHor1, ss, k);
	fChn[k]->ChangeOptions(fChn[k]->GetOptions()|kFixedWidth);
	fChn[k]->SetWidth(36);
	fHor1->AddFrame(fChn[k], fLay6);
	fChn[k]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");

	// int col=gROOT->GetColor(chcol[k])->GetPixel();
	// int fcol=0;
	// if (k==0 || k==3 || k==21 || k==25 || k==29)
	//   fcol=0xffffff;
	
	//printf("Color: %d %d %x %x\n",k,chcol[k],col,fcol);

	//flab->SetBackgroundColor(col);
	//flab->SetForegroundColor(fcol);
	//fChn[k]->SetForegroundColor(fcol);

	//fChn[k]->SetBackgroundColor(col);
	//fChn[k]->SetForegroundColor(col);
	//cout << "bkg: " << fChn[k]->GetBackground() << endl;
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


  TGHorizontalFrame* fHor[nstat];
  TGTextEntry* st_lab[nstat];

  const char* st_nam[nstat]={"Evt:","Mul:","Tst:"};
  const char* st_tip[nstat]={"Event number","Multiplicity","Time stamp"};

  for (int i=0;i<nstat;i++) {
    fHor[i] = new TGHorizontalFrame(fVer_st, 10, 10);
    st_lab[i]= new TGTextEntry(fHor[i], st_nam[i]);
    fStat[i]=new TGStatusBar(fHor[i],10,10);
    fStat[i]->Draw3DCorner(kFALSE);

    st_lab[i]->SetState(false);
    //st_lab[i]->ChangeOptions(st_lab[i]->GetOptions()|kFixedWidth);
    st_lab[i]->ChangeOptions(kFixedWidth);
    st_lab[i]->SetWidth(24);
    st_lab[i]->SetToolTipText(st_tip[i]);
    //fStat[i]->SetToolTipText(st_tip[i]);

    fVer_st->AddFrame(fHor[i],fLay8);
    fHor[i]->AddFrame(st_lab[i],fLay9);
    fHor[i]->AddFrame(fStat[i],fLay3);
  }

  // fStat[1]=new TGStatusBar(fVer_st,10,10);
  // fVer_st->AddFrame(fStat[0],fLay3);
  // fVer_st->AddFrame(fStat[1],fLay3);
  // fStat[0]->Draw3DCorner(kFALSE);
  // fStat[1]->Draw3DCorner(kFALSE);


  //trd=0;
  //cout << "fVer0->Opt: "<<fVer0->GetOptions() << endl;

  //cout << "fCanvas: " << fCanvas << endl;

  for (int i=0;i<MAX_CH;i++) {
    for (int j=0;j<3;j++) {
      //sprintf(ss,"gr_%d_%d",i,j);
      Gr[j][i]=new TGraph(10);
      //Gr[j][i]->SetNameTitle(ss,";Time(ns);");
      Gr[j][i]->SetLineColor(chcol[i]);
      //Gr[j][i]->SetMarkerColor(chcol[i]);
      Gr[j][i]->SetMarkerColor(1);
      Gr[j][i]->SetMarkerStyle(20);
      Gr[j][i]->SetMarkerSize(0.5);
    }
  }  

  //mgr[0]=new TMultiGraph();
  //mgr[1]=new TMultiGraph();
  //mgr[2]=new TMultiGraph();

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
  //delete Emut2;
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
  if (!crs->b_acq) {
    d_event = Pevents->begin();
    DrawEvent2();
  }
}

void EventFrame::Last() {
  if (!crs->b_acq) {
    d_event = --Pevents->end();
    DrawEvent2();
  }
}

void EventFrame::Plus1() {
  if (!crs->b_acq) {
    ++d_event;
    if (d_event==Pevents->end()) {
      --d_event;
    }
    DrawEvent2();
  }
}

void EventFrame::Minus1() {
  if (!crs->b_acq) {
    if (d_event!=Pevents->begin())
      --d_event;
    DrawEvent2();
  }
}

void EventFrame::PlusN() {
  if (!crs->b_acq) {
    for (int i=0;i<opt.num_events;i++) {
      ++d_event;
      if (d_event==Pevents->end()) {
	--d_event;
	break;
      }
    }
    DrawEvent2();
  }
}

void EventFrame::MinusN() {
  if (!crs->b_acq) {
    for (int i=0;i<opt.num_events;i++) {
      if (d_event!=Pevents->begin()) {
	--d_event;
      }
      else {
	break;
      }
    }
    DrawEvent2();
  }
}

void EventFrame::Clear()
{
  fCanvas->GetCanvas()->Clear();
}


void EventFrame::DoSlider() {

  /*
  TGDoubleSlider *btn = (TGDoubleSlider *) gTQSender;
  Int_t id = btn->WidgetId();

  float x1,x2;
  btn->GetPosition(x1,x2);

  cout << id << endl;
  printf("DosLider: %f %f\n",x1,x2);

  //float dx=gx2-gx1;
  //float dx1=dx*x1;
  //float dx2=dx*x2;

  */
  //printf("DosLider: %d\n",nEvents);
  if (!crs->b_acq)
    ReDraw();

}

void EventFrame::DoChkDeriv() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  //cout << "DoDeriv: " << id << endl;

  opt.b_deriv[id] = !opt.b_deriv[id];
  btn->SetState((EButtonState) opt.b_deriv[id]);

  if (!crs->b_acq) {
    DrawEvent2();
  }

}

void EventFrame::DoChkPeak() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  opt.b_peak[id] = !opt.b_peak[id];

  btn->SetState((EButtonState) opt.b_peak[id]);

  if (id==0) {
    for (int i=1;i<8;i++) {
      opt.b_peak[i] = opt.b_peak[id];
      fPeak[i]->SetState((EButtonState) opt.b_peak[id]);
    }
  }
  else {
    if (opt.b_peak[id]) {
      opt.b_peak[0] = true;
      fPeak[0]->SetState((EButtonState) 1);      
    }
  }

  if (!crs->b_acq)
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

  if (!crs->b_acq)
    ReDraw();

}

void EventFrame::FillGraph(int dr) {
  //Float_t dat[40000];
  //Float_t *pdat=0;

  UInt_t ch[MAX_CH];

  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    PulseClass *pulse = &d_event->pulses.at(i);
    ch[i]= pulse->Chan;

    Gr[dr][i]->Set(pulse->sData.size());
    Gr[dr][i]->SetLineColor(chcol[ch[i]]);
    Gr[dr][i]->SetMarkerColor(chcol[ch[i]]);

    double dt=pulse->Tstamp64 - d_event->T - crs->Pre[ch[i]];
    //double dt=pulse->Tstamp64 - d_event->T - cpar.preWr[ch[i]];
    //double dt=pulse->Tstamp64 - d_event->T;

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

    gx1[i]=(dt-1);
    gx2[i]=(pulse->sData.size()+dt);
    gy1[dr][i]=1e99;
    gy2[dr][i]=-1e99;

    if (dr==0) { //main pulse
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][i]->GetX()[j]=(j+dt);
	Gr[dr][i]->GetY()[j]=pulse->sData[j];

	if (Gr[dr][i]->GetY()[j]<gy1[dr][i])
	  gy1[dr][i]=Gr[dr][i]->GetY()[j];
	if (Gr[dr][i]->GetY()[j]>gy2[dr][i])
	  gy2[dr][i]=Gr[dr][i]->GetY()[j];

	//xx[j]=dt+j;
	//yy[j]=pulse->sData[j];
	// cout << "GR: " << dt << " " << Gr[dr][ch[i]]->GetN() << " "
	//      << ch[i] << " " << dt+j << " "
	//      << pulse->sData[j] << " " 
	//      << Gr[dr][ch[i]]->GetX()[j] << " " 
	//      << Gr[dr][ch[i]]->GetY()[j] << endl; 
	  
      }
    }
    else if (dr==1) { //1st derivaive

      Int_t kk=opt.kdrv[ch[i]];
      if (kk<1 || kk>=(Int_t)pulse->sData.size()) kk=1;

      //dat = new Float_t[pulse->sData.size()];
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][i]->GetX()[j]=(j+dt);
	if (j<kk)
	  Gr[dr][i]->GetY()[j]=pulse->sData[j]-pulse->sData[0];
	//dat[j]=0;
	else
	  Gr[dr][i]->GetY()[j]=pulse->sData[j]-pulse->sData[j-kk];

	if (Gr[dr][i]->GetY()[j]<gy1[dr][i])
	  gy1[dr][i]=Gr[dr][i]->GetY()[j];
	if (Gr[dr][i]->GetY()[j]>gy2[dr][i])
	  gy2[dr][i]=Gr[dr][i]->GetY()[j];

      }
    }
    else if (dr==2) { //2nd derivative
      for (Int_t j=0;j<(Int_t)pulse->sData.size();j++) {
	Gr[dr][i]->GetX()[j]=(j+dt);
	if (j<2)
	  Gr[dr][i]->GetY()[j]=0;
	//dat[j]=0;
	else
	  Gr[dr][i]->GetY()[j]=
	    pulse->sData[j]-2*pulse->sData[j-1]+pulse->sData[j-2];

	if (Gr[dr][i]->GetY()[j]<gy1[dr][i])
	  gy1[dr][i]=Gr[dr][i]->GetY()[j];
	if (Gr[dr][i]->GetY()[j]>gy2[dr][i])
	  gy2[dr][i]=Gr[dr][i]->GetY()[j];

      }
    }
  } 
}

void EventFrame::SetRanges(int dr) {

  //if (mgr[dr]->GetListOfGraphs())
  //  mgr[dr]->GetListOfGraphs()->Clear();

  //delete mgr[dr];
  //mgr[dr]=new TMultiGraph(mgr_name[dr],mgr_title[dr]);

  mx1=1e99;
  mx2=-1e99;
  my1[dr]=1e99;
  my2[dr]=-1e99;

  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    UInt_t ch= d_event->pulses.at(i).Chan;
    if (fChn[ch]->IsOn()) {
      if (gx1[i]<mx1) mx1=gx1[i];
      if (gx2[i]>mx2) mx2=gx2[i];
      if (gy1[dr][i]<my1[dr]) my1[dr]=gy1[dr][i];
      if (gy2[dr][i]>my2[dr]) my2[dr]=gy2[dr][i];

      //cout << "Graph1: " << dr << " " << mx1 << " " << mx2 << " " 
      //   << my1[dr] << " " << my2[dr] << endl;

      //mgr[dr]->Add(Gr[dr][ch]);
    }
  } 

  double dy=my2[dr]-my1[dr];
  my1[dr]-=dy*0.05;
  my2[dr]+=dy*0.05;

}

void EventFrame::DrawEvent2() {

  cout << "draw0:" << endl;
  //return;
  Emut2.Lock();

  TCanvas *cv=fCanvas->GetCanvas();
  cv->Clear();

  cout << "draw1:" << endl;

  //cv->Update();
  //int nnn=0;
  //if (Pevents) nnn=Pevents->size();
  //printf("DrawEvent0: %p %d %p %d\n",Pevents,nnn,crs->Pevents,crs->Pevents.size());

  if (Pevents->empty()) {
    txt.DrawTextNDC(0.2,0.7,"Empty event");
    cv->Update();
    cout << "draw1a:" << endl;
    Emut2.UnLock();
    return;
  }

  //cout << "draw1a:" << endl;
  ndiv=0;

  if (d_event->pulses.empty()) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    Emut2.UnLock();
    return;
  }

  //cout << "draw1b: " << d_event->pulses.size() << endl;
  ULong64_t mask=0;
  ULong64_t one=1;
  
  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    UInt_t ch= d_event->pulses.at(i).Chan;
    mask|=one<<ch;
  }

  //printf("mask: %llx\n",mask);
  
  for (int i=0;i<MAX_CH;i++) {
    if (mask&(one<<i)) {
      fChn[i]->SetBackgroundColor(gcol[i]);
      fChn[i]->SetForegroundColor(fcol[i]);
    }
    else {
      fChn[i]->SetBackgroundColor(15263976);
      fChn[i]->SetForegroundColor(0);
    }
  }

  // for (UInt_t i=0;i<d_event->pulses.size();i++) {
  //   UInt_t ch= d_event->pulses.at(i).Chan;
  //   fChn[ch]->SetBackgroundColor(gcol[ch]);
  //   fChn[ch]->SetForegroundColor(fcol[ch]);
  // }

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

  //cout << "draw2:" << endl;

  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {
      FillGraph(i);
      ndiv++;
    }
  }

  //cout << "draw3:" << endl;

  //Emut2.UnLock();
  //return;

  //cout << "hst: " << hst[0]->GetNhists() << endl;
  //cout << "mgr: " << mgr[0]->GetHistogram() << endl;
  //TGraph* gr = (TGraph*) mgr[0]->GetListOfGraphs()->First();
  //cout << gr->GetN() << " " << gr->GetX()[0] << " " << gr->GetX()[1] << endl;
  
  cv->Divide(1,ndiv);

  ReDraw();

  //cout << "Draw1: " << endl;
  cout << "Draw2: " << d_event->T << endl;
  //Emut2.UnLock();
  //return;

  char ss[99];

  sprintf(ss,"%lld",d_event->Nevt);
  fStat[0]->SetText(ss);

  sprintf(ss,"%ld",d_event->pulses.size());
  fStat[1]->SetText(ss);

  sprintf(ss,"%lld",d_event->T);
  fStat[2]->SetText(ss);
  
  
  cv->Update();

  Emut2.UnLock();

} //DrawEvent2

void EventFrame::DrawPeaks(int dr, PulseClass* pulse, double y1,double y2) {

  //gPad->Update();
  //double ux1=gPad->GetUxmin();
  //double ux2=gPad->GetUxmax();
  //double uy1=gPad->GetUymin();
  //double uy2=gPad->GetUymax();

  //for (UInt_t i=0;i<d_event->pulses.size();i++) {
  //PulseClass *pulse = &d_event->pulses.at(i);

  double dy=y2-y1;
  if (dr>0) y1=0;
  
  UInt_t ch= pulse->Chan;
  if (fChn[ch]->IsOn()) {
    double dt=pulse->Tstamp64 - d_event->T - crs->Pre[ch];
    //double dt=pulse->Tstamp64 - d_event->T - cpar.preWr[ch];
    //double dt=pulse->Tstamp64 - d_event->T;

    //int dt=d_event->pulses.at(i).Tstamp64 - d_event->T;
    //cout << "Dt: " << i << " " << d_event->T << " " << dt
    // << " " << d_event->pulses.size() << " " << hst[dr]->GetNhists() << endl;
    //UInt_t ch= pulse->Chan;

    for (UInt_t j=0;j<pulse->Peaks.size();j++) {
      peak_type *pk = &pulse->Peaks[j];
      if (fPeak[1]->IsOn()) // Pos
	doXline(pk->Pos+dt,y1,y2-dy*0.3,2,1);
      if (fPeak[2]->IsOn()) // Time
	doXline(pk->Time+dt,y2-dy*0.2,y2,3,1);
      if (dr==0 && fPeak[3]->IsOn()) { // Wpeak
	doXline(pk->P1+dt,y1,y2-dy*0.2,4,2);
	doXline(pk->P2+dt,y1,y2-dy*0.1,4,2);
      }
      if (dr==0 && fPeak[4]->IsOn()) { // WBkgr
	doXline(pk->B1+dt,y1,y2-dy*0.2,6,3);
	doXline(pk->B2+dt,y1,y2-dy*0.1,6,3);
      }
      if (dr==1 && fPeak[5]->IsOn()) { //WTime
	doXline(pk->T3+dt,y1,y2-dy*0.2,1,2);
	doXline(pk->T4+dt,y1,y2-dy*0.1,1,2);
      }
      //cout <<"DrawPeaksT2: " << pk->Time+dt << " " << dt << " " 
      //   << pulse->Tstamp64 << " " << d_event->T << endl;
    }
  }
    //}
}

//void EventFrame::DrawEvent() {
//} //DrawEvent

void EventFrame::DoGraph(int ndiv, int dd) {

} //DoGraph

void EventFrame::ReDraw() {

  Emut.Lock();
  //cv->Update();
  
  //cout << "Redr0: " << endl;

  int nn=1;
  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {

      //TCanvas *cv=fCanvas->GetCanvas();
      //cout << "Redr0: " << cv << endl;
      //Emut.UnLock();
      //return;

      fCanvas->GetCanvas()->cd(nn++);

      //cout << "Redr1: " << i << endl;

      SetRanges(i);


      if (mx1>1e98) {
	gPad->Clear();
	continue;
      }


      //cout << "Redr2: " << i << endl;

      double dx=mx2-mx1;
      double dy=my2[i]-my1[i];

      Float_t h1,h2;

      fHslider->GetPosition(h1,h2);
      double x1=mx1+dx*h1;
      double x2=mx2-dx*(1-h2);
      fVslider->GetPosition(h1,h2);
      double y1=my1[i]+dy*(1-h2);
      double y2=my2[i]-dy*h1;

      //printf("DosLider: %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f %0.2f\n",
      //     h1,h2,y1,y2,mx1,mx2,dx);

      //cout << "Redraw5: " << i << fHist[i] << endl;
      delete fHist[i];
      //cout << "Redraw6: " << i << fHist[i] << endl;
      fHist[i] = new TH1F(mgr_name[i],mgr_title[i],int(dx),x1,x2);
      fHist[i]->SetDirectory(0);
      fHist[i]->SetMinimum(y1);
      fHist[i]->SetMaximum(y2);
      fHist[i]->Draw("axis");

      //cout << "Redraw7: " << i << fHist[i] << endl;

      doYline(0,x1,x2,4,2);
      //cout <<"DrawPeaks: " << d_event->T0 << endl;
      if (fPeak[6]->IsOn()) { //T0
	mk.DrawMarker(d_event->T0,y2-dy*0.1);
	//doXline(d_event->T0,y1,y2,5,1);
      }

      for (UInt_t j=0;j<d_event->pulses.size();j++) {
	PulseClass *pulse = &d_event->pulses.at(j);
	//UInt_t ch= d_event->pulses.at(j).Chan;
	if (fChn[pulse->Chan]->IsOn()) {
	  //cout << "Gr: " << i << " " << j << " " << int(pulse->Chan) << " "
	  //     << Gr[i][j]->GetLineColor() << " " << Gr[i][j]->GetN() << endl;
	  Gr[i][j]->Draw("lp");
	  //Gr[i][pulse->Chan]->Draw("lp");
	  DrawPeaks(i,pulse,y1,y2);
	  if (i==1 && fPeak[7]->IsOn()) //threshold
	    doYline(opt.thresh[pulse->Chan],gx1[j],
		    gx2[j],chcol[pulse->Chan],2);
	}
      }

      //cout << "MinMax: " << mgr[i]->GetXaxis()->GetXmin() << " " 
      //     << mgr[i]->GetXaxis()->GetXmax() << endl;

      // TH1* mh = mgr[i]->GetHistogram();
      // if (mh) {
      // 	cout << "MinMax: " << mh->GetXaxis()->GetXmin() << " " 
      // 	     << mh->GetXaxis()->GetXmax() << endl;
      // }

      //mgr[i]->Draw("alp");

      // cout << "mgr3: " << i << " " << nn << " " << fPeak[0]->IsOn() << " "
      //    << mgr[i]->GetHistogram()<< endl;

      //   << mgr[i]->GetHistogram()->GetMaximum() << endl;

      // if (mgr[i]->GetHistogram() && fPeak[0]->IsOn()) {
      // 	DrawPeaks(mgr[i]->GetHistogram()->GetMinimum(),
      // 	      mgr[i]->GetHistogram()->GetMaximum());
      // }
    }
  }

  //cout << "rdr77: " << endl;
  fCanvas->GetCanvas()->Update();

  //cout << "rdr78: " << endl;
  Emut.UnLock();

}
