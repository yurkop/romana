//----- EventFrame ----------------
#include "romana.h"

#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
#include <TMarker.h>
#include <TMutex.h>
#include <TSystem.h>
//#include <TROOT.h>
#include <TGTableLayout.h>

#include <TRandom.h>

namespace EF {
  Float_t RGB[32][3] = 
    {
      {0,0,0},//0
      {1,0,0},
      {0,1,0},
      {0,0,1},
      {1,0,1},
      {0,1,1}, //5
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
}

Long64_t markt[10];

TLine ln;
TMarker mk;

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

extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
//extern BufClass* Buffer;

extern CRS* crs;
extern ParParDlg *parpar;
extern EventFrame* EvtFrm;

extern ULong_t fGreen;
extern ULong_t fRed;
extern ULong_t fCyan;

Float_t xgr[DSIZE]; // x coordinate for graph
Float_t ygr[DSIZE]; // y coordinate for graph

char hname[3][MAX_CH+1][20]; //+all
 
TText txt;

//TMutex Emut;
//TMutex Emut2;

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
   //:TGCompositeFrame(p,w,h,kVerticalFrame)
{

  Float_t rr[3];
  for (int i=0;i<MAX_CH;i++) {
    for (int j=0;j<3;j++) {
      rr[j]=EF::RGB[i%32][j];
    }
    //chcol[i]=TColor::GetColor(TColor::RGB2Pixel(EF::RGB[i][0],EF::RGB[i][1],EF::RGB[i][2]));
    chcol[i]=TColor::GetColor(TColor::RGB2Pixel(rr[0],rr[1],rr[2]));
    gcol[i]=gROOT->GetColor(chcol[i])->GetPixel();
    // fcol[i]=0;
    // if (i==0 || i==3 || i==21 || i==25 || i==29)
    //    fcol[i]=0xffffff;

    //cout << i << " " << gcol[i] << endl;

  }
  //exit(1);

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
  
  fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
  fLay2 = new TGLayoutHints(kLHintsExpandY,2,2,2,2);
  fLay3 = new TGLayoutHints(kLHintsExpandX,2,2,2,2);
  fLay4 = new TGLayoutHints(kLHintsLeft,2,2,2,2);
  fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
  fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
  fLay7 = new TGLayoutHints(kLHintsCenterX,0,0,0,0);
  fLay8 = new TGLayoutHints(kLHintsExpandX,0,0,0,0);
  fLay9 = new TGLayoutHints(kLHintsExpandY,0,0,0,0);

  //fLay16 = new TGLayoutHints(kLHintsLeft,0,2,0,0);

  //Frames.....

  fDock = new TGDockableFrame(this);
  AddFrame(fDock, fLay1);
  fDock->SetWindowName("Events");  
  fDock->SetFixedSize(kFALSE);
  fDock->Connect("Docked()","EventFrame",this,"Rebuild()");
  fDock->Connect("Undocked()","EventFrame",this,"DoUndock()");

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  fVer0 = new TGVerticalFrame(fMain, 10, 10);
  separator1 = new TGVertical3DLine(fMain);
  fVer_st = new TGVerticalFrame(fMain, 10, 10);

  fMain->AddFrame(fVer0, fLay1);
  fMain->AddFrame(separator1, fLay2);
  fMain->AddFrame(fVer_st, fLay4);

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

  fFirst = new TGTextButton(fHor_but,"First");
  fFirst->SetToolTipText("Go to the first event in the list");
  fFirst->Connect("Clicked()","EventFrame",this,"First()");
  fHor_but->AddFrame(fFirst, fLay5);

  fLast = new TGTextButton(fHor_but,"Last");
  fLast->SetToolTipText("Go to the last event in the list");
  fLast->Connect("Clicked()","EventFrame",this,"Last()");
  fHor_but->AddFrame(fLast, fLay4);

  fmOne = new TGTextButton(fHor_but," -1 ");
  fmOne->SetToolTipText("One event backward");
  fmOne->Connect("Clicked()","EventFrame",this,"Minus1()");
  fHor_but->AddFrame(fmOne, fLay4);

  fOne = new TGTextButton(fHor_but," +1 ");
  fOne->SetToolTipText("One event forward");
  fOne->Connect("Clicked()","EventFrame",this,"Plus1()");
  fHor_but->AddFrame(fOne, fLay4);

  fmNev = new TGTextButton(fHor_but," -N ");
  fmNev->SetToolTipText("N events backward");
  fmNev->Connect("Clicked()","EventFrame",this,"MinusN()");
  fHor_but->AddFrame(fmNev, fLay4);

  fNev = new TGTextButton(fHor_but," +N ");
  fNev->SetToolTipText("N events forward");
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

  const char* ttip = "Checkpoint: search backward for an event which satisfies the following condition";
  fChk1 = new TGTextButton(fHor_but," S-: ",1);
  fChk1->SetToolTipText(ttip);
  fChk1->Connect("Clicked()","EventFrame",this,"DoCheckPoint()");
  fHor_but->AddFrame(fChk1, fLay4);

  ttip = "Checkpoint: search forward for an event which satisfies the following condition";
  fChk2 = new TGTextButton(fHor_but," S+: ",2);
  fChk2->SetToolTipText(ttip);
  fChk2->Connect("Clicked()","EventFrame",this,"DoCheckPoint()");
  fHor_but->AddFrame(fChk2, fLay4);

  ttip = "Formula for the condition.\nUse standard C and root operators and functions\nFormula turns red in case of an error\n[0] - channel number;\n[1] - Tstamp;\n[2] - time (sec);\n[3] - multiplicity;\n[4] - Area;\n[5] - Base;\n[6] - tof";
  //cout << "formula: " << opt.formula << endl;
  tEnt = new TGTextEntry(fHor_but,opt.formula,0);;
  tEnt->SetWidth(100);
  tEnt->SetToolTipText(ttip);
  //tt->SetState(false);
  fHor_but->AddFrame(tEnt,fLay4);
  tEnt->Connect("TextChanged(char*)", "EventFrame", this, "DoFormula()");

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
  fPeak[3] = new TGCheckButton(fVer_d, "WBase", 4);
  fPeak[4] = new TGCheckButton(fVer_d, "Wpeak", 3);
  fPeak[5] = new TGCheckButton(fVer_d, "WTime", 5);
  fPeak[6] = new TGCheckButton(fVer_d, "WWidth", 6);
  fPeak[7] = new TGCheckButton(fVer_d, "*TStart", 7);
  fPeak[8] = new TGCheckButton(fVer_d, "Thresh", 8);

  fPeak[9] = new TGCheckButton(fVer_d, "Val", 9);

  //int cc;
  //cc=gROOT->GetColor(2)->GetPixel();
  fPeak[1]->SetForegroundColor(gROOT->GetColor(2)->GetPixel());
  fPeak[2]->SetForegroundColor(gROOT->GetColor(3)->GetPixel());
  fPeak[3]->SetForegroundColor(gROOT->GetColor(6)->GetPixel()); //Wbase
  fPeak[4]->SetForegroundColor(gROOT->GetColor(1)->GetPixel()); //Wpeak
  fPeak[5]->SetForegroundColor(gROOT->GetColor(3)->GetPixel()); //Wtime
  fPeak[6]->SetForegroundColor(gROOT->GetColor(4)->GetPixel()); //Wwidth
  fPeak[7]->SetForegroundColor(gROOT->GetColor(2)->GetPixel()); //Tstart


  fPeak[0]->SetToolTipText("Show peaks");
  fPeak[1]->SetToolTipText("Peak Position (maximum in 1st derivative)");
  fPeak[2]->SetToolTipText("Exact time from 1st derivative");
  //fPeak[3]->SetToolTipText("Exact time from 2nd derivative");
  fPeak[3]->SetToolTipText("Window for baseline integration");
  fPeak[4]->SetToolTipText("Window for peak integration");
  fPeak[5]->SetToolTipText("Window for time integration (left,right),\nplotted only in 1st derivative");
  fPeak[6]->SetToolTipText("Window for width integration (left,right),\nplotted only in pulse");
  fPeak[7]->SetToolTipText("Time start marker (earliest Time)");
  //fPeak[8]->SetToolTipText("Threshold, plotted only in 1st derivative)");
  fPeak[8]->SetToolTipText("Threshold)");

  fPeak[9]->SetToolTipText("Print peak values");

  for (int i=0;i<10;i++) {
    fPeak[i]->SetState((EButtonState) opt.b_peak[i]);
    fPeak[i]->Connect("Clicked()","EventFrame",this,"DoChkPeak()");
    fVer_d->AddFrame(fPeak[i], fLay4);
  }


  fVer_ch = new TGVerticalFrame(fHor_st);
  fHor_st->AddFrame(fVer_ch, fLay4);

  AddCh();

  TGHorizontalFrame* fHor1 = new TGHorizontalFrame(fVer_ch, 10, 10);
  fVer_ch->AddFrame(fHor1, fLay7);
  fChn[MAX_CH] = new TGCheckButton(fHor1, "all", MAX_CH);
  fHor1->AddFrame(fChn[MAX_CH], fLay6);
  fChn[MAX_CH]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");
  fChn[MAX_CH]->SetState(kButtonDown);

  TGHorizontalFrame* fHor[nstat];
  TGTextEntry* st_lab[nstat];

  const char* st_nam[nstat]={"E","T","M","S"};
  const char* st_tip[nstat]={"Event number","Time stamp","Multiplicity","State"};

  TGFontPool *pool = gClient->GetFontPool();
  const TGFont *font = pool->GetFont("misc", -10, kFontWeightNormal, kFontSlantRoman);
  //font->Print();
  FontStruct_t tfont = 0;

  if (font) {
    tfont = font->GetFontStruct();
  }

  for (int i=0;i<nstat-1;i++) {
    fHor[i] = new TGHorizontalFrame(fVer_st, 10, 10);
    fVer_st->AddFrame(fHor[i],fLay8);
    fHor[i]->ChangeOptions(kFixedWidth);
    fHor[i]->SetWidth(100);
  }

  for (int i=0;i<nstat;i++) {
    int j=i;
    if (i==nstat-1) j=i-1;
    //fHor[i] = new TGHorizontalFrame(fVer_st, 10, 10);
    st_lab[i]= new TGTextEntry(fHor[j], st_nam[i]);

    //fStat[i]=new TGStatusBar(fHor[j],10,10);
    //fStat[i]->Draw3DCorner(kFALSE);
    fStat[i] = new TGTextEntry(fHor[j], "");
    if (tfont) {
      fStat[i]->SetFont(tfont,false);
    }
    fStat[i]->SetState(false);
    fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kSunkenFrame);

    st_lab[i]->SetState(false);
    //st_lab[i]->ChangeOptions(st_lab[i]->GetOptions()|kFixedWidth);
    st_lab[i]->ChangeOptions(kFixedWidth);
    st_lab[i]->SetWidth(16);
    st_lab[i]->SetToolTipText(st_tip[i]);
    fStat[i]->SetToolTipText(st_tip[i]);

    //fVer_st->AddFrame(fHor[j],fLay8);
    fHor[j]->AddFrame(st_lab[i],fLay9);
    fHor[j]->AddFrame(fStat[i],fLay3);
  }

  // TGLayoutHints *fLay10 = new TGLayoutHints(kLHintsExpandX,2,2,0,0);
  // for (int i=0;i<4;i++) {
  //   fStat2[i]=new TGStatusBar(fVer_st,10,10);
  //   fStat2[i]->Draw3DCorner(kFALSE);
  //   fVer_st->AddFrame(fStat2[i],fLay10);
  // }

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

  //cout << "Compile1: " << opt.formula << endl;
  //strcpy(opt.formula,"0");
  //formula = new TFormula("formula",opt.formula);
  formula = new TFormula();
  formula->SetName("evtformula");
  //cout << "Compile2: " << formula->GetName() << endl;

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
  //cout << "~EventFrame()" << endl;
}


// void EventFrame::StartThread()
// {
//   trd = new TThread("trd", trd_handle, (void*) 0);
//   trd->Run();
// }

// void EventFrame::CloseWindow()
// {
//   delete this;
// }








// void EventFrame::Rebuild() {
//   cout << "Event::Rebuild" << endl;

//   fGroupCh->Cleanup();
//   //delete fGroupCh;

// }






/*
void EventFrame::AddCh() {
  char ss[100];

  fVer_ch = new TGVerticalFrame(fHor_st, 10, 10);
  fHor_st->AddFrame(fVer_ch, fLay4);

  TGLabel* fLabel2 = new TGLabel(fVer_ch, "  Channels        ");
  fVer_ch->AddFrame(fLabel2, fLay4);

  fHor_ch = new TGHorizontalFrame(fVer_ch, 10, 10);
  fVer_ch->AddFrame(fHor_ch, fLay1);

  for (int i=0;i<MAX_CH;i++) {
    fChn[i] = NULL;
  }

  fVer_ch2 = new TGVerticalFrame(fHor_ch, 10, 10);
  fHor_ch->AddFrame(fVer_ch2, fLay1);

  int ny = MAX_CH;
  if (ny>16) ny=16;

  int nclmn=(MAX_CH-1)/ny+1;

  // for (int m=1;m<=64;m++) {
  //   int nn=(m-1)/ny+1;
  //   cout << m << " " << nn << endl;
  // }
  // exit(1);

  
  for (int i=0;i<ny;i++) {
    fHorCh[i] = new TGHorizontalFrame(fVer_ch2, 10, 10);
    fVer_ch2->AddFrame(fHorCh[i], fLay6);

    for (int j=0;j<nclmn;j++) {
      int k=j*ny+i;

      if (k<opt.Nchan) {
	sprintf(ss,"%d",k);

	fChn[k] = new TGCheckButton(fHorCh[i]," ", k);
	fChn[k]->ChangeOptions(fChn[k]->GetOptions()|kFixedSize);
	fChn[k]->SetWidth(16);
	fChn[k]->SetHeight(20);
	fHorCh[i]->AddFrame(fChn[k], fLay6);
	fChn[k]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");

	TGLabel* flab = new TGLabel(fHorCh[i], ss);
	flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
	flab->SetWidth(16);
	fHorCh[i]->AddFrame(flab, fLay6);

	// int col=gROOT->GetColor(chcol[k])->GetPixel();
	// int fcol=0;
	// if (k==0 || k==3 || k==21 || k==25 || k==29)
	//   fcol=0xffffff;
	
	//printf("Color: %d %d %x %x\n",k,chcol[k],col,fcol);



	//flab->SetBackgroundColor(gcol[k]);
	//flab->SetForegroundColor(gcol[k]);
	//fChn[k]->SetForegroundColor(fcol);

	fChn[k]->SetBackgroundColor(gcol[k]);
	//fChn[k]->SetForegroundColor(gcol[k]);
	//cout << "bkg: " << fChn[k]->GetBackground() << endl;



	fChn[k]->SetState(kButtonDown);

      } //if
    } //for j
  } //for i

  TGHorizontalFrame* fHor1 = new TGHorizontalFrame(fVer_ch2, 10, 10);
  fVer_ch2->AddFrame(fHor1, fLay7);

  fChn[MAX_CH] = new TGCheckButton(fHor1, "all", MAX_CH);
  fHor1->AddFrame(fChn[MAX_CH], fLay6);
  fChn[MAX_CH]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");
  fChn[MAX_CH]->SetState(kButtonDown);
  //TGLabel* flab = new TGLabel(fHor1, "all");
  //flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
  //flab->SetWidth(16);
  //fHor1->AddFrame(flab, fLay6);

}
*/

void EventFrame::AddCh() {
  char ss[100];

  //fGroupCh = new TGGroupFrame(fVer_ch, "Channels", kVerticalFrame);
  fGroupCh = new TGGroupFrame(fVer_ch, "Channels");
  fGroupCh->SetTitlePos(TGGroupFrame::kCenter); // centered
  fVer_ch->AddFrame(fGroupCh, fLay4);

  // 2 column, n rows
  //fGroupCh->SetLayoutManager(new TGVerticalLayout(fGroupCh));
  fGroupCh->SetLayoutManager(new TGMatrixLayout2(fGroupCh, 16, 0, 1));
  //fGroupCh->SetLayoutManager(new TGTileLayout(fGroupCh, 5));
  //fGroupCh->SetLayoutManager(new TGTableLayout(fGroupCh, 16, 0, kFALSE, 2, kLHintsLeft));

  for (int i=0;i<MAX_CH;i++) {
    frCh[i] = new TGHorizontalFrame(fGroupCh);
    fGroupCh->AddFrame(frCh[i]);

    sprintf(ss,"%d",i);
    fChn[i] = new TGCheckButton(frCh[i],"", i);
    fChn[i]->ChangeOptions(fChn[i]->GetOptions()|kFixedSize);
    fChn[i]->SetWidth(16);
    fChn[i]->SetHeight(20);
    fChn[i]->SetBackgroundColor(gcol[i]);
    fChn[i]->SetState(kButtonDown);

    frCh[i]->AddFrame(fChn[i],fLay6);

    TGLabel* flab = new TGLabel(frCh[i], ss);
    flab->ChangeOptions(flab->GetOptions()|kFixedWidth);
    flab->SetWidth(16);
    frCh[i]->AddFrame(flab, fLay6);

    fChn[i]->Connect("Clicked()","EventFrame",this,"DoPulseOff()");
  }  

  //fGroupCh->HideFrame(frCh[1]);
  //ShPtr(1);

}

void EventFrame::Rebuild() {
  //cout << "Event::Rebuild: " << opt.Nchan << " " << fDock->GetUndocked() << endl;

  //fGroupCh->RemoveAll();
  //fGroupCh->Cleanup();

  for (int i=0;i<MAX_CH;i++) {
    //fGroupCh->AddFrame(frCh[i]);
    if (i<opt.Nchan) {
      //fGroupCh->MapWindow();
      fGroupCh->ShowFrame(frCh[i]);
    }
    else {
      //fGroupCh->UnmapWindow();
      fGroupCh->HideFrame(frCh[i]);
    }
  }

  //fGroupCh->HideFrame(frCh[1]);

  //fGroupCh->MapSubwindows();
  //fGroupCh->Resize(GetDefaultSize());
  //fGroupCh->MapWindow();
  //fGroupCh->Layout();
}

void EventFrame::DoUndock() {
  //cout << "Event::DoUndock: " << opt.Nchan << endl;
  Rebuild();
  // if (fDock->GetUndocked()) {
  //   fDock->GetUndocked()->
  //     ChangeOptions((fDock->GetUndocked()->GetOptions() &
  // 		     ~kTransientFrame) | kMainFrame);
  // }
}

void EventFrame::ShPtr(int zz) {
  //fGroupCh->HideFrame(frCh[1]);
  TList* fList = fGroupCh->GetList();
  TGFrameElement *ptr;
  TIter next(fList);
  int nn=0;
  while ((ptr = (TGFrameElement *) next())) {
    cout << zz << "Layout:" << nn << " " << ptr->fState << " " << kIsVisible << endl;
    nn++;
  }
}

void EventFrame::DoReset() {
  Pevents = &Tevents;
  d_event = Pevents->begin();
}

/*
void EventFrame::DoColor() {
  TGColorDialog* tg = new TGColorDialog();
}
*/

void EventFrame::First() {
  if (crs->b_stop) {
    d_event = Pevents->begin();
    DrawEvent2();
  }
}

void EventFrame::Last() {
  if (crs->b_stop) {
    d_event = --Pevents->end();
    DrawEvent2();
  }
}

void EventFrame::Plus1() {
  if (crs->b_stop) {
    ++d_event;
    if (d_event==Pevents->end()) {
      --d_event;
    }
    DrawEvent2();
  }
}

void EventFrame::Minus1() {
  if (crs->b_stop) {
    if (d_event!=Pevents->begin())
      --d_event;
    DrawEvent2();
  }
}

void EventFrame::PlusN() {
  if (crs->b_stop) {
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
  if (crs->b_stop) {
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

void EventFrame::DoCheckPoint() {
  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  Pixel_t color;
  Double_t par[7];
  formula->SetTitle(opt.formula);
  formula->Clear();
  int ires = formula->Compile();
  //formula is not valid
  if (ires) {
    gClient->GetColorByName("red", color);
    tEnt->SetBackgroundColor(color);
  }
  //formula is valid
  else {
    gClient->GetColorByName("white", color);
    tEnt->SetBackgroundColor(color);
    double res = 0;

    std::list<EventClass>::iterator ievt;
    // prepare search backward
    if (id==1) {
      ievt = Pevents->begin();
      if (d_event==Pevents->begin())
	return;
      else
	--d_event;
    }
    //prepare seach forward
    else {
      ievt = --Pevents->end();
      ++d_event;
      if (d_event==Pevents->end()) {
	--d_event;
	return;
      }
    }
    while (d_event!=ievt) {
      //cout << d_event->Nevt << " " << d_event->T << endl;
      par[3]=d_event->pulses.size();
      par[2]=(d_event->Tstmp-crs->Tstart64)*crs->period*1e-9;
      par[1]=d_event->Tstmp;

      for (UInt_t i=0;i<d_event->pulses.size();i++) {
	int ch = d_event->pulses[i].Chan;
	par[0]=ch;

	for (UInt_t j=0;j<d_event->pulses[i].Peaks.size();j++) {
	  peak_type* pk = &d_event->pulses[i].Peaks[j];
	  par[4]=pk->Area;
	  par[5]=pk->Base;
	  //double dt = d_event->pulses[i].Tstamp64 - d_event->Tstmp;
	  //double tt = pk->Time - d_event->T0 + dt;
	  double tt = pk->Time - d_event->T0;

	  par[6]=tt*crs->period;
	  res = formula->EvalPar(0,par);
	  //cout << "DoCheck: " << id << " " << opt.formula << " " << d_event->Nevt << " " << d_event->TT << " " << " " << par[4] << " " << res << endl;
	  if (res) {
	    //d_event = d_event;
	    DrawEvent2();
	    return;
	  }
	}
      }
      if (id==1) {
	--d_event;
      }
      else {
	++d_event;
      }
    } //while

    DrawEvent2();
  } //else
}

void EventFrame::DoFormula() {
  TGTextEntry *te = (TGTextEntry*) gTQSender;
  //Int_t id = te->WidgetId();
  
  strcpy(opt.formula,te->GetText());
  //cout << "DoFormula: " << te->GetText() << " " << opt.formula << endl;
}


// void EventFrame::Clear()
// {
//   fCanvas->GetCanvas()->Clear();
// }


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
  if (crs->b_stop) {
    ReDraw();
  }

}

void EventFrame::DoChkDeriv() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  //cout << "DoDeriv: " << id << endl;

  opt.b_deriv[id] = !opt.b_deriv[id];
  btn->SetState((EButtonState) opt.b_deriv[id]);

  if (crs->b_stop) {
    DrawEvent2();
  }

}

void EventFrame::DoChkPeak() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  opt.b_peak[id] = !opt.b_peak[id];

  btn->SetState((EButtonState) opt.b_peak[id]);

  if (id==0) {
    for (int i=1;i<10;i++) {
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

  if (crs->b_stop)
    ReDraw();

}

void EventFrame::DoPulseOff() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  if (id==MAX_CH) {
    for (int i=0;i<opt.Nchan;i++) {
      fChn[i]->SetState((EButtonState) fChn[id]->IsOn());
    }
  }
  //cout << "DoPulseOff: " << id << " " << fChn[id]->IsOn() << endl;

  if (crs->b_stop)
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

    if (Gr[dr][i]->GetN()==0) {
      gx1[i]=0;
      gx2[i]=1;
      gy1[dr][i]=0;
      gy2[dr][i]=1;
      continue;
    }
    //cout << "sData: " << i << " " << pulse->sData.size() << " " << Gr[dr][i]->GetN() << endl;

    double dt=pulse->Tstamp64 - d_event->Tstmp - cpar.preWr[ch[i]];

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
      }
    }
    else if (dr==1) { //1st derivaive

      Int_t kk=opt.Drv[ch[i]];
      if (kk<1 || kk>=(Int_t)pulse->sData.size()) kk=1;

      //cout << "kk=" << kk << " " << ch[i] << " " << opt.Drv[ch[i]] << endl;
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

  //cout << "psize: " << d_event->pulses.size() << endl;
  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    UInt_t ch= d_event->pulses.at(i).Chan;
    if (fChn[ch]->IsOn()) {
      if (gx1[i]<mx1) mx1=gx1[i];
      if (gx2[i]>mx2) mx2=gx2[i];
      if (gy1[dr][i]<my1[dr]) my1[dr]=gy1[dr][i];
      if (gy2[dr][i]>my2[dr]) my2[dr]=gy2[dr][i];

      //cout << "Graph1: " << dr << " " << mx1 << " " << mx2 << " " 
      //<< my1[dr] << " " << my2[dr] << endl;

      //mgr[dr]->Add(Gr[dr][ch]);
    }
  } 

  double dy=my2[dr]-my1[dr];
  my1[dr]-=dy*0.05;
  my2[dr]+=dy*0.05;

}

void EventFrame::DrawEvent2() {

  //cout << "draw0:" << " " << Pevents->empty() << endl;
  //return;
  //Emut2.Lock();

  TCanvas *cv=fCanvas->GetCanvas();
  //cv->SetEditable(true);
  cv->Clear();
  
  //cout << "draw1: " << Pevents << " " << &crs->Levents.back() << endl;
  //cout << "draw111: " << Pevents->empty() << endl;

  //cv->Update();
  //int nnn=0;
  //if (Pevents) nnn=Pevents->size();
  //printf("DrawEvent0: %p %d %p %d\n",Pevents,nnn,crs->Pevents,crs->Pevents.size());

  if (Pevents->empty()) {
    return;
  }
  /*
  if (Pevents->empty()) {
    txt.DrawTextNDC(0.2,0.7,"Empty event");
    cv->Update();
    //cout << "draw1a:" << endl;
    //Emut2.UnLock();
    return;
  }
  */
  //cout << "draw1a:" << endl;
  if (d_event->pulses.empty()) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    //Emut2.UnLock();
    return;
  }

  //cout << "draw1b: " << d_event->pulses.size() << endl;
  ULong64_t mask=0;
  ULong64_t one=1;
  
  for (UInt_t i=0;i<d_event->pulses.size();i++) {
    UInt_t ch= d_event->pulses.at(i).Chan;
    mask|=one<<ch;
  }


  markt[0]=gSystem->Now();

  for (int i=0;i<opt.Nchan;i++) {
    Pixel_t cc;
    if (mask&(one<<i)) {
      cc = gcol[i];
    }
    else {
      cc = 15263976;
    }
    if (cc!=fChn[i]->GetBackground())
      fChn[i]->ChangeBackground(cc);

    //fChn[i]->ChangeBackground(gcol[i]);
  }

  markt[1]=gSystem->Now();

  ndiv=0;

  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {
      FillGraph(i);
      ndiv++;
    }
  }

  markt[2]=gSystem->Now();

  cv->Divide(1,ndiv);

  markt[3]=gSystem->Now();

  ReDraw();

  markt[4]=gSystem->Now();
  char ss[99];

  sprintf(ss,"%lld",d_event->Nevt);
  fStat[0]->SetText(ss);

  sprintf(ss,"%lld",d_event->Tstmp);
  fStat[1]->SetText(ss);

  sprintf(ss,"%ld",d_event->pulses.size());
  fStat[2]->SetText(ss);

  sprintf(ss,"%d",d_event->State);
  fStat[3]->SetText(ss);

  // for (int i=0;i<4;i++) {
  //   sprintf(ss,"%d: %lld",i+1,markt[i+1]-markt[i]);
  //   fStat2[i]->SetText(ss);
  // }


  cv->Update();

  //cv->SetEditable(false);
  //Emut2.UnLock();

} //DrawEvent2

void EventFrame::DrawPeaks(int dr, PulseClass* pulse, double y1,double y2) {

  double dy=y2-y1;
  if (dr>0) y1=0;
  
  UInt_t ch= pulse->Chan;
  if (fChn[ch]->IsOn()) {
    double dt=pulse->Tstamp64 - d_event->Tstmp - cpar.preWr[ch];

    for (UInt_t j=0;j<pulse->Peaks.size();j++) {
      peak_type *pk = &pulse->Peaks[j];
      //cout << "drawpeak: " << (int) pulse->Chan << " " << pk->Pos << " " << pk->Time << " " << dt << endl;

      if (fPeak[1]->IsOn()) // Pos
	doXline(pk->Pos+dt,y1,y2-dy*0.3,2,1);
      if (fPeak[2]->IsOn()) {// Time
	//double dt2=pulse->Tstamp64 - d_event->Tstmp - cpar.preWr[ch];
	//doXline(pk->Time+dt+cpar.preWr[ch],y2-dy*0.2,y2,3,1);
	doXline(pk->Time,y2-dy*0.2,y2,3,1);
      }
      if (dr==0 && fPeak[4]->IsOn()) { // Wpeak
	doXline(pk->P1+dt,y1,y2-dy*0.2,1,2);
	doXline(pk->P2-1+dt,y1,y2-dy*0.1,1,2);
      }
      if (dr==0 && fPeak[3]->IsOn()) { // WBkgr
	doXline(pk->B1+dt,y1,y2-dy*0.2,6,3);
	doXline(pk->B2-1+dt,y1,y2-dy*0.1,6,3);
      }
      if (dr==1 && fPeak[5]->IsOn()) { //WTime
	doXline(pk->T3+dt,y1,y2-dy*0.2,3,2);
	doXline(pk->T4-1+dt,y1,y2-dy*0.1,3,2);
      }
      if (dr==0 && fPeak[6]->IsOn()) { //WWidth
	doXline(pk->T5+dt,y1,y2-dy*0.2,4,4);
	doXline(pk->T6-1+dt,y1,y2-dy*0.1,4,4);
      }
      //cout <<"DrawPeaksT2: " << pk->Time+dt << " " << dt << " " 
      //   << pulse->Tstamp64 << " " << d_event->T << endl;
    }
  }

}

//void EventFrame::DrawEvent() {
//} //DrawEvent

void EventFrame::DoGraph(int ndiv, int dd) {

} //DoGraph

void EventFrame::ReDraw() {

  //Emut.Lock();
  //cv->Update();

  TText tt;

  if (Pevents->empty()) {
    return;
  }

  TCanvas *cv=fCanvas->GetCanvas();
  //cout << "Redr0: " << endl;
  /*
  if (Pevents->empty()) {
    txt.DrawTextNDC(0.2,0.7,"Empty event");
    cv->Update();
    //cout << "draw1a:" << endl;
    //Emut2.UnLock();
    return;
  }
  */
  //cout << "draw1a:" << endl;
  if (d_event->pulses.empty()) {
    //TText tt;
    txt.DrawTextNDC(0.2,0.7,"No pulses in this event");
    cv->Update();
    //Emut2.UnLock();
    return;
  }

  int nn=1;
  for (int i=0;i<3;i++) {
    if (opt.b_deriv[i]) {

      int tx=0,ty=0;
      int ny=d_event->pulses.size();
      double dd = 0.74*2.0/ny;

      cv->cd(nn++);

      SetRanges(i);

      //cout << "mx1: " << i << " " << mx1 << endl;

      if (mx1>1e98) {
	gPad->Clear();
	continue;
      }

      double dx=mx2-mx1;
      double dy=my2[i]-my1[i];

      Float_t h1,h2;

      fHslider->GetPosition(h1,h2);
      double x1=mx1+dx*h1;
      double x2=mx2-dx*(1-h2);
      fVslider->GetPosition(h1,h2);
      double y1=my1[i]+dy*(1-h2);
      double y2=my2[i]-dy*h1;

      delete fHist[i];

      fHist[i] = new TH1F(mgr_name[i],mgr_title[i],int(dx),x1,x2);
      fHist[i]->SetDirectory(0);
      fHist[i]->SetMinimum(y1);
      fHist[i]->SetMaximum(y2);
      fHist[i]->Draw("axis");

      doYline(0,x1,x2,4,2);
      if (fPeak[7]->IsOn()) { //T0
	mk.DrawMarker(d_event->T0,y2-dy*0.1);
      }

      for (UInt_t j=0;j<d_event->pulses.size();j++) {
	PulseClass *pulse = &d_event->pulses.at(j);
	//UInt_t ch= d_event->pulses.at(j).Chan;
	if (fChn[pulse->Chan]->IsOn()) {
	  if (Gr[i][j]->GetN()) { //draw graph
	    Gr[i][j]->Draw("lp");
	    DrawPeaks(i,pulse,y1,y2);
	    int ithr=(opt.strg[pulse->Chan]!=0);
	    if (i==ithr && fPeak[8]->IsOn()) //threshold
	      doYline(opt.Thr[pulse->Chan],gx1[j],
		      gx2[j],chcol[pulse->Chan],2);
	  }
	  if (fPeak[9]->IsOn()) { //draw text
	    if (pulse->Peaks.size()) {
	      char ss[256];
	      peak_type *pk = &pulse->Peaks.back();
	      sprintf(ss,"%d A=%0.1f T=%0.1f W=%0.1f W3=%f",
		      pulse->Chan,pk->Area,pk->Time,pk->Width,pk->Width3);
	      //tt.SetBBoxX1(0);
	      //tt.SetBBoxX2(100);
	      //tt.SetBBoxY1(0);
	      //tt.SetBBoxY2(20);

	      tt.SetTextAlign(0);
	      tt.SetTextSize(0.03);
	      tt.SetTextColor(chcol[pulse->Chan]);
	      dd=0.74*2.0/32;

	      // cout << "txt: " << j << " " << tx << " " << ty << " "
	      // 	 << 0.1+tx*0.5 << " " << 0.8-ty*dd
	      // 	 << endl;
	      tt.DrawTextNDC(0.17+tx*0.35,0.85-ty*dd,ss);
	      tx++;
	      if (tx>1) {
		tx=0;
		ty++;
	      }
	    }
	  }
	}
      } //for (UInt_t j=0;j<d_event->pulses.size();j++) {

      // for (UInt_t j=0;j<32;j++) {
      // 	PulseClass *pulse = &d_event->pulses.at(0);
      // 	pulse->Chan=j;
      // }

    } // if (opt.b_deriv[i]) {
  } //for i

  cv->Update();

  //Emut.UnLock();

}
