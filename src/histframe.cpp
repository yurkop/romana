//----- HistFrame ----------------
#include "romana.h"
//#include "peditor.h"

#include <sstream>

#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
#include <TSpectrum.h>
#include <TMutex.h>
#include <TGButtonGroup.h>
#include <TGShutter.h>
//#include <TROOT.h>
#include <TRandom.h>
#include <TMath.h>
#include <TF1.h>

TLine gline[2];
TLine cline;

extern int debug;
/*
int Ch_Gamma_X[8]={ 1, 2, 3, 4, 5, 6, 7, 8};
int Ch_Gamma_Y[8]={16,15,14,13,12,11,10, 9};
int Ch_Alpha_X[8]={25,29,26,30,27,31,28,32};
int Ch_Alpha_Y[8]={24,20,23,19,22,18,21,17};
*/

const short nhcols=16;
const short hcols[nhcols] = {1,2,3,4,5,6,7,22,33,41,46,38,30,24,29,40};

extern Toptions opt;
//extern Coptions cpar;
extern MyMainFrame *myM;
extern EventFrame* EvtFrm;
//extern BufClass* Buffer;

//extern Common* com;
extern CRS* crs;
extern ParParDlg *parpar;
extern HClass* hcl;

extern ULong_t fGreen;
extern ULong_t fRed;
extern ULong_t fCyan;

//extern TH1F *hrms;
//extern TH1F *htdc_a[MAX_CH]; //absolute tof (start=0)

extern HistFrame* HiFrm;

//TText txt;

TMutex Hmut;

TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,5,5,0,0);
TGLayoutHints* fLay9 = new TGLayoutHints(kLHintsCenterX|kLHintsBottom,0,0,0,0);

//------------------------------

void DynamicExec()
{
  static TPolyLine pl;
  static HMap* map=0; // hmap of histogram, on which cut is set
  //TObject* obj=0;
  static int hdim=0;

  int ev = gPad->GetEvent();
  int px = gPad->GetEventX();
  int py = gPad->GetEventY();
  
  Double_t xx = gPad->AbsPixeltoX(px);
  Double_t yy = gPad->AbsPixeltoY(py);

  if (gPad->GetLogx()) {
    xx=pow(10,xx);
  }
  if (gPad->GetLogy()) {
    yy=pow(10,yy);
  }

  Double_t y1 = gPad->GetUymin();
  Double_t y2 = gPad->GetUymax();

  TString ss = TString::Format("%0.7g   %0.7g",xx,yy);
  //ss += xx;
  //ss += "  ";
  //ss += yy;
  HiFrm->fStatus->SetText(ss);

  if (ev==51 && hdim==2 && HiFrm->np_gcut) { //mouse movement
    gline[0].SetX2(xx);
    gline[0].SetY2(yy);
    gline[0].Draw();
    HiFrm->fEc->GetCanvas()->SetCrosshair(false);
    gPad->Update();
  }

  if(ev==1 || ev==12) { //left click (also middle click)
    if (HiFrm->np_gcut==0) {//first click
      int np=0;
      if (opt.b_stack) { //DrawStack
	map = HiFrm->pad_map[0];
      }
      else { //DrawHist
	np = gPad->GetNumber()-1;
	map = HiFrm->pad_map[np];
      }
      if (!map || np >= (int)HiFrm->pad_map.size()) {
	cout << "hist not found for pad " << np << endl;
	HiFrm->in_gcut=0;
	HiFrm->Update();
	return;
      }
      hdim=map->hst->GetDimension();
      pl.SetPolyLine(0);
    } //first click

    if (hdim==2) {
      HiFrm->np_gcut=pl.SetNextPoint(xx,yy)+1;
      if (HiFrm->np_gcut>1) {
	pl.Draw();
      }
      if (HiFrm->np_gcut>=MAX_PCUTS-1) {
	HiFrm->np_gcut=pl.SetNextPoint(pl.GetX()[0],pl.GetY()[0])+1;
	HiFrm->AddCutG(&pl,map);
	return;
      }

      gline[0].SetX1(xx);
      gline[0].SetY1(yy);
    } //if hdim==2
    else if (hdim==1) {
      int np=pl.SetNextPoint(xx,yy);
      HiFrm->np_gcut=np+1;
      if (np<=1) {
	gline[np].SetX1(xx);
	gline[np].SetX2(xx);
	gline[np].SetY1(y1);
	gline[np].SetY2(y2);
	gline[np].Draw();
      }
      if (np>=1) {
      	HiFrm->AddCutG(&pl,map);
      	return;
      }
    }
  } //if(ev==1 || ev==12)
  
  if (ev==61 || ev==12) { //middle click or double click
    HiFrm->np_gcut=pl.SetNextPoint(pl.GetX()[0],pl.GetY()[0])+1;
    HiFrm->AddCutG(&pl,map);
  }
  
} //DynamicExec()

Bool_t empty_roi(Hdef* hd, int i) {
  return hd->roi[i][0]==0 && hd->roi[i][1]==0;
}

//----- HistFrame ----------------
HistFrame::HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  ttxt.SetTextAlign(22);
  ttxt.SetTextSize(0.04);
  b_adj=false;

  //TGLayoutHints* LayCB0   = new TGLayoutHints(kLHintsCenterX|kLHintsBottom,0,0,0,0);
  TGLayoutHints* LayET0  = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
  //TGLayoutHints* LayLT0   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 0,0,0,0);
  TGLayoutHints* LayLE0   = new TGLayoutHints(kLHintsLeft|kLHintsExpandY);
  TGLayoutHints* LayLE1  = new TGLayoutHints(kLHintsLeft|kLHintsExpandY,3,0,0,0);
  TGLayoutHints* LayLC1  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,1,1,0,0);
  TGLayoutHints* LayLC2  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,0,0,0);
  TGLayoutHints* LayLC3  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,0,5,0,0);
  //TGLayoutHints* LayLC4  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,1,1,7,7);
  TGLayoutHints* LayEE0 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY);

  TGLayoutHints* LayRC0 = new TGLayoutHints(kLHintsRight|kLHintsCenterY);


  
  gline[0].SetLineColor(2);
  gline[1].SetLineColor(2);

  hmap_chklist=new TList();
  //hstack=new THStack();
  //st_hlist=new TList();
  //st_hlist->SetOwner(true);
  st_plot = new TH1F("st_plot","",1000,0.,1000.);
  //memset(pad_hist,0,sizeof(pad_hist));

  st_plot->SetBit(TH1::kNoStats);
  st_plot->SetBit(kCanDelete,0);
  st_plot->SetDirectory(0);

  //hstack->SetHistogram(hplot);
  ntab=nt;

  changed=false;
  started=true;

  memset(wrk_check,1,sizeof(wrk_check));
  //wrk_check_MT=1;

  //Frames.....

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc
  TGHorizontalFrame      *fHor3; //for cuts
  TGHorizontalFrame      *fHor4; //for hist buttons
  TGHorizontalFrame      *fHor5; //for ROI


  TGVerticalFrame        *fVer0; //canvas && hslider

  //TGVerticalFrame *fVer[NR];
  //TGLabel *flab[NR];

  //TGShutterItem *item;
  //TGCompositeFrame *container;

  TGTextButton* but;

  fDock = new TGDockableFrame(this);
  AddFrame(fDock, LayEE0);
  fDock->SetWindowName("Plots/Cuts");
  fDock->SetFixedSize(kFALSE);

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));

  fHor1 = new TGHorizontalFrame(fMain, 10, 10);
  fHor2 = new TGHorizontalFrame(fMain, 10, 10);

  fMain->AddFrame(fHor1, LayEE0);
  fMain->AddFrame(fHor2, LayET0);

  fVer0 = new TGVerticalFrame(fHor1, 10, 10);
  fHor1->AddFrame(fVer0, LayEE0);

  //fEc = new TRootEmbeddedCanvas("Hcanvas", fHor1, 10, 10);
  //fHor1->AddFrame(fEc, LayEE0);
  fEc = new TRootEmbeddedCanvas("Hcanvas", fVer0, 10, 10);
  fVer0->AddFrame(fEc, LayEE0);

  fHslider = new TGDoubleHSlider(fVer0, 10, kDoubleScaleBoth,0);
  fHslider->SetRange(0,1);
  fHslider->SetPosition(0,1);
  fVer0->AddFrame(fHslider, LayET0);
  fHslider->Connect("PositionChanged()", "HistFrame", 
   		    this, "DoSlider()");

  fVslider = new TGDoubleVSlider(fHor1, 10, kDoubleScaleBoth,1);
  fVslider->SetRange(0,1);
  fVslider->SetPosition(0,1);
  //fVslider->SetWidth(10);
  fHor1->AddFrame(fVslider, LayLE0);
  fVslider->Connect("PositionChanged()", "HistFrame", 
   		    this, "DoSlider()");


  TGTab* fTab = new TGTab(fHor1);
  fHor1->AddFrame(fTab, LayLE1);
  TGCompositeFrame* tab1 = fTab->AddTab(" Hist ");
  TGCompositeFrame* tab2 = fTab->AddTab(" Cuts ");
  TGCompositeFrame* tab3 = fTab->AddTab(" ROI ");
  gCanvas = new TGCanvas(tab1, 150, 100);
  tab1->AddFrame(gCanvas, LayLE0);
  gCanvas2 = new TGCanvas(tab2, 150, 100);
  tab2->AddFrame(gCanvas2, LayLE0);
  gCanvas3 = new TGCanvas(tab3, 150, 100);
  tab3->AddFrame(gCanvas3, LayLE0);

  fListTree = new TGListTree(gCanvas, kVerticalFrame);
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");
  fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "DoClick(TGListTreeItem*,Int_t)");

  // fListTree->Connect("KeyPressed(TGListTreeItem*, UInt_t, UInt_t)","HistFrame",
  // 		     this,"DoKey(TGListTreeItem*, UInt_t, UInt_t)");

  fHor4 = new TGHorizontalFrame(tab1, 10, 10);
  tab1->AddFrame(fHor4, LayET0);

  TGCheckButton *chkbut = new TGCheckButton(fHor4,"",0);
  chkbut->Connect("Toggled(Bool_t)", "HistFrame", this, "CheckAll(Bool_t)");
  chkbut->SetToolTipText("check/uncheck all");
  fHor4->AddFrame(chkbut, LayLC1);

  but = new TGTextButton(fHor4,"w+",1);
  but->Connect("Clicked()", "HistFrame", this, "CheckWork()");
  but->SetToolTipText("Add all checked histograms to WORK*");
  fHor4->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor4,"w-",2);
  but->Connect("Clicked()", "HistFrame", this, "ClearWork()");
  but->SetToolTipText("Remove all histograms from WORK*");
  fHor4->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor4,"Rst",2);
  but->Connect("Clicked()", "HistFrame", this, "DoRst()");
  but->SetToolTipText("Reset all histograms (works during acquisition/analysis)");
  but->ChangeBackground(fCyan);
  fHor4->AddFrame(but, LayLC1);

  fHor4 = new TGHorizontalFrame(tab1, 10, 10);
  tab1->AddFrame(fHor4, LayET0);

  int id = parpar->Plist.size()+1;
  TGLabel* fLab = new TGLabel(fHor4, "Draw:");
  fHor4->AddFrame(fLab,LayLC1);
  TGTextEntry* tOpt    = new TGTextEntry(fHor4,"",id);;
  fHor4->AddFrame(tOpt,LayET0);
  //tForm->SetWidth(110);
  tOpt->SetMaxLength(sizeof(opt.drawopt)-1);
  tOpt->SetToolTipText("Draw options for 1d and 2d, separated by space\n(see ROOT manual for drawing histograms)");
  parpar->DoMap(tOpt,opt.drawopt,p_txt,0,3<<4);
  tOpt->Connect("TextChanged(char*)", "ParDlg", parpar, "DoTxt()");
  //tOpt->Connect("ReturnPressed()", "HistFrame", this, "AddFormula()");

  // TGNumberEntryField* fN =
  //   new TGNumberEntryField(fHor4, 0, 7, TGNumberFormat::kNESInteger,
  // 			   TGNumberFormat::kNEAPositive,
  // 			   TGNumberFormat::kNELLimitMinMax,
  // 			   1,1000);
  // fN->SetToolTipText("Rebin all checked histograms (only for drawing)");
  // fN->SetWidth(36);
  // fHor4->AddFrame(fN, LayLC1);

  //-- Cuts

  fCutTree = new TGListTree(gCanvas2, kVerticalFrame);
  fCutTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "CutClick(TGListTreeItem*,Int_t)");
  
  fHor3 = new TGHorizontalFrame(tab2, 10, 10);
  tab2->AddFrame(fHor3, LayET0);


  but = new TGTextButton(fHor3,"Add",1);
  but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Add a cut for existing histogram");
  fHor3->AddFrame(but, LayLC1);

  //but = new TGTextButton(fHor3,"Cancel",9);
  but = new TGTextButton(fHor3," X ",9);
  but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Cancel/interrupt adding a cut");
  fHor3->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor3,"Del",3);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  but->SetToolTipText("Delete all cuts. Use right mouse button on a cut to delete it");
  fHor3->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor3,"Edit",4);
  but->Connect("Clicked()", "HistFrame", this, "EditCutG()");
  but->SetToolTipText("Edit all cuts");
  fHor3->AddFrame(but, LayLC1);

  // but = new TGTextButton(fHor3,"Sort",4);
  // //but->Connect("Clicked()", "HistFrame", this, "EditCutG()");
  // but->SetToolTipText("Sort all cuts");
  // fHor3->AddFrame(but, LayLC1);

  const char* ttip = "Formula for the cut.\nUse standard C and root operators and functions\nFormula turns red in case of an error\nUse [0] [1] [2] ... for other cuts in the formula\nExamples:\n [0] && [1] - cut0 \"and\" cut1\n [2] || [3] - cut2 \"or\" cut3\n ![3] - not cut3";

  //tab2->AddFrame(new TGLabel(tab2, "--Formula--"), LayET0);

  //TGHorizontalFrame *fHor7 = new TGHorizontalFrame(tab2, 10, 10);
  //tab2->AddFrame(fHor7, LayCB0);

  // but = new TGTextButton(fHor7,"Form",7);
  // but->Connect("Clicked()", "HistFrame", this, "AddFormula()");
  // but->SetToolTipText(ttip);
  // fHor7->AddFrame(but, LayLT0);
  
  tForm    = new TGTextEntry(tab2,"",8);;
  //tForm->SetWidth(110);
  tForm->SetMaxLength(sizeof(opt.cut_form[0])-1);
  tForm->SetToolTipText(ttip);
  tForm->Connect("ReturnPressed()", "HistFrame", this, "AddFormula()");
  tab2->AddFrame(tForm,LayET0);

  //-- ROI

  fRoiTree = new TGListTree(gCanvas3, kVerticalFrame);
  fRoiTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "RoiClick(TGListTreeItem*,Int_t)");

  fHor5 = new TGHorizontalFrame(tab3, 10, 10);
  tab3->AddFrame(fHor5, LayET0);

  but = new TGTextButton(fHor5,"Add",2);
  but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Add ROI for existing histogram(s)");
  fHor5->AddFrame(but, LayLC1);

  //but = new TGTextButton(fHor5,"Cancel",9);
  but = new TGTextButton(fHor5," X ",9);
  but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Cancel/interrupt adding ROI");
  fHor5->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor5,"Del",9);
  //but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Delete all ROI (not implemented). Use right mouse button on a ROI to delete it");
  fHor5->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor5,"Edit",9);
  //but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Edit all ROI (not implemented)");
  fHor5->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor5,"Sort",9);
  //but->Connect("Clicked()", "HistFrame", this, "StartMouse()");
  but->SetToolTipText("Sort all ROI (not implemented)");
  fHor5->AddFrame(but, LayLC1);

  //fStatus = new TGStatusBar(tab3, 50, 10, kRaisedFrame);
  //fStatus->Draw3DCorner(kFALSE);
  //tab3->AddFrame(fStatus, LayET0);
  //AddFrame(fStatusBar, new TGLayoutHints(kLHintsExpandX, 0, 0, 10, 0));

  
  /*
  but = new TGTextButton(fHor5,"Del",3);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  but->SetToolTipText("Delete all ROI. Use right mouse button on a ROI to delete it");
  fHor5->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor5,"Edit",4);
  but->Connect("Clicked()", "HistFrame", this, "EditCutG()");
  but->SetToolTipText("Edit all ROIs");
  fHor5->AddFrame(but, LayLC1);
  */

  //-- main frame

  const char* rlab[2] = {"Stack/N","X/Y:"};
  Rb[0] = new TGRadioButton(fHor2,rlab[0],1);
  fHor2->AddFrame(Rb[0], LayLC2);
  Rb[0]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  Rb[0]->SetToolTipText("Draw all marked histograms as stack");
  
  id = parpar->Plist.size()+1;
  chknorm = new TGCheckButton(fHor2,"",id);
  parpar->DoMap(chknorm,&opt.b_norm,p_chk,0,3<<4);
  chknorm->Connect("Toggled(Bool_t)", "ParDlg", parpar, "DoDaqChk(Bool_t)");
  fHor2->AddFrame(chknorm, LayLC2);
  chknorm->SetToolTipText("Normalize stacked histograms");

  Rb[1] = new TGRadioButton(fHor2,rlab[1],2);
  fHor2->AddFrame(Rb[1], LayLC1);
  Rb[1]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  Rb[1]->SetToolTipText("Draw histograms in separate subwindows");

  const char* ttip2[] = {"Horizontal divisions","Vertical divisions"};

  TGNumberEntry* fNum[2];
  //int div[2] = {opt.xdiv,opt.ydiv};
  int* div[2] = {&opt.xdiv,&opt.ydiv};

  /*
  for (int i=0;i<2;i++) {
    fNum[i] = new TGNumberEntry(fHor2, div[i], 2, i+10,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEAAnyNumber,
				TGNumberFormat::kNELLimitMinMax,
				1,16);
    fNum[i]->GetNumberEntry()->Connect("TextChanged(char*)", "HistFrame", this, "DoNum()");
    fNum[i]->GetNumberEntry()->SetToolTipText(ttip2[i]);
  }
  */

  for (int i=0;i<2;i++) {
    int id = parpar->Plist.size()+1;
    fNum[i] = new TGNumberEntry(fHor2, 0, 2, id,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEAAnyNumber,
				TGNumberFormat::kNELLimitMinMax,
				1,16);
    
    fNum[i]->GetNumberEntry()->SetToolTipText(ttip2[i]);
    parpar->DoMap(fNum[i]->GetNumberEntry(),div[i],p_inum,0,3<<4);
    fNum[i]->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar, "DoDaqNum()");
  }

  fHor2->AddFrame(fNum[0], LayLC2);
  fHor2->AddFrame(fNum[1], LayLC3);

  but = new TGTextButton(fHor2," < ",1);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, LayLC1);
  but->SetToolTipText("One histogram backward");

  but = new TGTextButton(fHor2," > ",2);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, LayLC1);
  but->SetToolTipText("One histogram forward");

  but = new TGTextButton(fHor2," << ",3);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, LayLC1);
  but->SetToolTipText("N histograms backward");

  but = new TGTextButton(fHor2," >> ",4);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, LayLC1);
  but->SetToolTipText("N histograms forward");

  //TGCheckButton* chk;
  id = parpar->Plist.size()+1;
  chklog = new TGCheckButton(fHor2,"Log",id);
  parpar->DoMap(chklog,&opt.b_logy,p_chk,0,3<<4);
  chklog->Connect("Toggled(Bool_t)", "ParDlg", parpar, "DoDaqChk(Bool_t)");
  fHor2->AddFrame(chklog, LayLC1);
  chklog->SetToolTipText("Log scale");

  id = parpar->Plist.size()+1;
  chkstat = new TGCheckButton(fHor2,"Stat",id);
  parpar->DoMap(chkstat,&opt.b_stat,p_chk,0,3<<4);
  chkstat->Connect("Toggled(Bool_t)", "ParDlg", parpar, "DoDaqChk(Bool_t)");
  fHor2->AddFrame(chkstat, LayLC1);
  chkstat->SetToolTipText("Show stats");

  /*
   // control horizontal position of the text
  TGButtonGroup *cuts = new TGButtonGroup(fHor2, "Horizontal Position",kChildFrame|kHorizontalFrame);
  //cuts->SetTitlePos(TGGroupFrame::kCenter);
  cuts->SetBorderDrawn(false);
   new TGTextButton(cuts, "Add", 1);
   new TGTextButton(cuts, "Show", 2);
   new TGTextButton(cuts, "Clear", 3);
   //horizontal->SetButton(kTextCenterX);
   //horizontal->Connect("Pressed(Int_t)", "ButtonWindow", this, "DoHPosition(Int_t)");
   fHor2->AddFrame(cuts, LayLC1);
  */

  /*
  fHor2->AddFrame(new TGLabel(fHor2, "Cuts:"), LayLC1);

  but = new TGTextButton(fHor2,"Add",1);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor2,"Cancel",2);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, LayLC1);

  //but = new TGTextButton(fHor2,"Show",3);
  //but->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  //fHor2->AddFrame(but, LayLC1);

  but = new TGTextButton(fHor2,"Clear",4);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor2->AddFrame(but, LayLC1);
  */

  //TGCheckButton* chk;

  id = parpar->Plist.size()+1;
  chkd = new TGCheckButton(fHor2,"Cuts",id);
  parpar->DoMap(chkd,&opt.b_gcuts,p_chk,0,3<<4);
  chkd->Connect("Toggled(Bool_t)", "ParDlg", parpar, "DoDaqChk(Bool_t)");
  fHor2->AddFrame(chkd, LayLC1);
  chkd->SetToolTipText("Draw cuts");

  id = parpar->Plist.size()+1;
  chkd = new TGCheckButton(fHor2,"Roi",id);
  parpar->DoMap(chkd,&opt.b_roi,p_chk,0,3<<4);
  chkd->Connect("Toggled(Bool_t)", "ParDlg", parpar, "DoDaqChk(Bool_t)");
  fHor2->AddFrame(chkd, LayLC1);
  chkd->SetToolTipText("Draw ROI");

  but = new TGTextButton(fHor2,"Peaks",4);
  but->Connect("Clicked()", "HistFrame", this, "DoPeaks()");
  fHor2->AddFrame(but, LayLC1);
  but->SetToolTipText("Find peaks in all visible windows (works only in X/Y mode");

  fStatus = new TGStatusBar(fHor2, 130, 10, kRaisedFrame);
  fStatus->Draw3DCorner(kFALSE);
  fHor2->AddFrame(fStatus, LayRC0);


  //DoReset();

  //cout << "histfr7: " << endl;
  //Update();
}

HistFrame::~HistFrame()
{
  //cout << "~HistFrame()" << endl;
}

TGListTreeItem* HistFrame::Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open, const TGPicture *closed) {

  HMap* map2 = (HMap*) userData;
  TGListTreeItem *item;
  if (parent) { //single item
    item=fListTree->AddItem(parent, string, userData, open, closed, true);
    //item->SetDNDSource(kTRUE);
    fListTree->CheckItem(item,*(map2->hd->c+map2->nn));
  }
  else { //folder
    Hdef* hd2 = 0;
    if (userData) { // not WORK*
      hd2 = map2->hd;
    }
    HMap* map = new HMap(string,hd2);
    //HMap* map = new HMap(string);
    //cout << "folder: " << map << " " << hd2 << endl;
    hcl->dir_list->Add(map);
    item=fListTree->AddItem(parent, string, map, open, closed, true);
    //item->SetDNDTarget(kTRUE);
  }
  return item;

}

void HistFrame::Make_Ltree() {

  //cout << "Make_Ltree():" << endl;
  // for (int i=0;i<opt.Nchan;i++) {
  //   cout << "chtype: " << i << " " << opt.chtype[i] << endl;
  // }

  TObject* obj=0;
  TIter next(hcl->map_list);

  char cutname[99];
  char name[99];
  TGPicture *pic_1d = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic_2d = (TGPicture*) gClient->GetPicture("h2_t.xpm");

  TGPicture *pic_c2 = (TGPicture*) gClient->GetPicture("marker30.xpm");
  TGPicture *pic_c1 = (TGPicture*) gClient->GetPicture("hor_arrow_cursor.png");
  TGPicture *pic_f = (TGPicture*) gClient->GetPicture("cpp_src_t.xpm");

  TGPicture *pic_xy = (TGPicture*) gClient->GetPicture("marker7.xpm");

  TGPicture *pic=0;
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir=0;
  TGListTreeItem *item=0;
  HMap* map;


  if (hcl->dir_list)
    delete hcl->dir_list;

  hcl->dir_list = new TList();
  hcl->dir_list->SetName("dir_list");
  hcl->dir_list->SetOwner(true);


  iWork = Item_Ltree(iroot, "WORK",0,0,0);
  for (int cc=1;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      sprintf(cutname,"WORK_cut%d",cc);
      iWork_cut[cc] = Item_Ltree(iroot, cutname,0,0,0);
    }
  }

  next.Reset();
  while ( (obj=(TObject*)next()) ) {
    map = (HMap*) obj;
    //if (!map->flg) {
      TString title = TString(map->GetTitle());
      if (!fListTree->FindChildByName(0,title)) {
	//cout << "map: " << map->GetName() << " " << map->GetTitle() << " " << map->hd->rb << endl;
	idir = Item_Ltree(iroot, title,map,0,0);
      }
      if (map->hst->InheritsFrom(TH2::Class()))
	pic=pic_2d;
      else
	pic=pic_1d;

      Item_Ltree(idir, map->GetName(), map, pic, pic);

      if (*(map->hd->w+map->nn)) {
	Item_Ltree(iWork, map->GetName(), map, pic, pic);
	//make clones of _work_ histograms and their hmaps
	Clone_Ltree(map);
      }
      //} //if
  } //while

  fListTree->CheckAllChildren(iWork,wrk_check[0]);
  for (int cc=1;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      fListTree->CheckAllChildren(iWork_cut[cc],wrk_check[cc+1]);
    }
  }

  //fill fCutTree
  for (int i=1;i<opt.ncuts;i++) {
    if (opt.pcuts[i]) {
      TCutG* gcut = hcl->cutG[i];

      if (opt.pcuts[i]==1) //formula
	pic=pic_f;
      else if (opt.pcuts[i]==2) //1d
	pic=pic_c1;
      else //2d
	pic=pic_c2;

      if (gcut->GetN() > 1) {
	sprintf(name,"[%s] %s",gcut->GetName(),gcut->GetTitle());
	item = fCutTree->AddItem(0, name, gcut, pic, pic, false);
	for (int j=0;j<gcut->GetN();j++) {
	  sprintf(name,"%0.2f; %0.2f",gcut->GetX()[j],gcut->GetY()[j]);
	  fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
	}
      }
      else {
	sprintf(name,"[%s] %s",gcut->GetName(),"formula");
	item = fCutTree->AddItem(0, name, gcut, pic, pic, false);
	sprintf(name,"%s",opt.cut_form[i]);
	fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
      }
    }
    else { //if opt.pcuts[i]
      
    }
  }

  //fill fRoiTree
  TIter next2(hcl->dir_list);
  while ( (obj=(TObject*)next2()) ) {
    map = (HMap*) obj;
    if (map->hd) {
      //for (int i=0;i<MAX_CH+NGRP;i++) {
      for (int i=0;i<MAXROI;i++) {
	//cout << "roi1: " << i << " " << map->hd->roi[i][0] << endl;
	//if (map->hd->roi[i][0]==0)
	if (empty_roi(map->hd,i))
	  continue;
	//cout << "roi2: " << i << " " << map->hd->roi[i][0] << endl;
	ItemROI(map,i);
      }
    }
  }




  //CutsToStr();
}

void HistFrame::Clear_Ltree()
{

  hmap_chklist->Clear();

  TGListTreeItem *idir;

  idir = fListTree->GetFirstItem();
  int ii=0;
  while (idir) {
    if (TString(idir->GetText()).Contains("work",TString::kIgnoreCase)) {
      wrk_check[ii]=idir->IsChecked();
      ii++;
    }
    fListTree->DeleteChildren(idir);
    fListTree->DeleteItem(idir);
    idir = idir->GetNextSibling();
  }

  //clear fCutTree
  idir = fCutTree->GetFirstItem();
  while (idir) {
    fCutTree->DeleteChildren(idir);
    fCutTree->DeleteItem(idir);
    idir = idir->GetNextSibling();
  }
  
  //clear fRoiTree
  idir = fRoiTree->GetFirstItem();
  while (idir) {
    fRoiTree->DeleteChildren(idir);
    fRoiTree->DeleteItem(idir);
    idir = idir->GetNextSibling();
  }
  
} //Clear_Ltree()

TGListTreeItem* HistFrame::FindItem(const char* name) {
  TGListTree* lTree = fListTree;;
  TGListTreeItem *idir = lTree->GetFirstItem();
  TGListTreeItem *item = 0;
  while (idir) {
    if ( (item=lTree->FindChildByName(idir,name)) ) {
      return item;
    }
    idir = idir->GetNextSibling();
  }
  return 0;
}

void HistFrame::AddWork(TGListTreeItem *item) {
  TGListTreeItem *item2 =
    fListTree->FindChildByData(iWork, item->GetUserData());
  if (!item2) {
    //cout << "Item2: " << item2 << endl;

    TObject* hh = (TObject*) item->GetUserData();
    //strcpy(hname,hh->GetName());
    const TGPicture *pic = item->GetPicture();
    Item_Ltree(iWork, hh->GetName(), hh, pic, pic);

    HMap* map = (HMap*) item->GetUserData();
    *(map->hd->w+map->nn)=true;

    hcl->Clone_Hist(map);
    Clone_Ltree(map);
  }
}

void HistFrame::ClearWork() {
  hmap_chklist->Clear();

  TGListTreeItem* item2 = iWork->GetFirstChild();
  while (item2) {
    HMap* map = (HMap*) item2->GetUserData();
    *(map->hd->w+map->nn)=false;
    hcl->Remove_Clones(map);
    item2 = item2->GetNextSibling();
  }
    
  fListTree->DeleteChildren(iWork);
  for (int cc=1;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      fListTree->DeleteChildren(iWork_cut[cc]);
    }
  }
}

void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  if (!crs->b_stop)
    return;

  char hname[80];
  char hname2[100]; 
  TGListTreeItem* item2;

   //clear all work* if middle button is clicked on WORK
  if (but==2 && TString(item->GetText()).EqualTo("work",TString::kIgnoreCase)) {
    //cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

    ClearWork();

    /*
    hmap_chklist->Clear();

    item2 = item->GetFirstChild();
    while (item2) {
      //cout << "dell: " << item2->GetText() << endl;
      HMap* map = (HMap*) item2->GetUserData();
      *(map->hd->w+map->nn)=false;
      hcl->Remove_Clones(map);
      item2 = item2->GetNextSibling();
    }
    
    fListTree->DeleteChildren(iWork);
    for (int cc=1;cc<opt.ncuts;cc++) {
      if (opt.pcuts[cc]) {
	fListTree->DeleteChildren(iWork_cut[cc]);
      }
    }
    */
    // if (crs->b_maintrig) {
    //   fListTree->DeleteChildren(iWork_MT);
    // }

  }

  if (item->GetParent() && (but==2 || but==3)) {
    if (TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
      // если клик мышкой (2 или 3) на гистограмму в папке work*,
      // удаляем ее из всех work*

      hmap_chklist->Clear();
      //remove items
      //cout << "work or work_cut*: " << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname2,hh->GetName());
      char* str = strstr(hname2,"_cut");
      //char* str2 = strstr(hname2,"_MT");
      if (str) {
	strncpy(hname,hname2,str-hname2);
	hname[str-hname2] = '\0';   // null character manually added
      }
      // else if (str2) {
      // 	strncpy(hname,hname2,str2-hname2);
      // 	hname[str2-hname2] = '\0';   // null character manually added
      // }
      else {
	strcpy(hname,hname2);
      }

      item2 = fListTree->FindChildByName(iWork,hname);

      if (item2) {
	HMap* map = (HMap*) item2->GetUserData();
	*(map->hd->w+map->nn)=false;
	fListTree->DeleteItem(item2);
	hcl->Remove_Clones(map);
      }

      for (int cc=1;cc<opt.ncuts;cc++) {
	if (opt.pcuts[cc]) {
	  sprintf(hname2,"%s_cut%d",hname,cc);
	  item2 = fListTree->FindChildByName(iWork_cut[cc],hname2);
	  //cout << "cc: " << cc << " " << hname2 << " " << item2 << endl;
	  if (item2)
	    fListTree->DeleteItem(item2);
	}
      }
      // if (crs->b_maintrig) {
      // 	sprintf(hname2,"%s_MT",hname);
      // 	item2 = fListTree->FindChildByName(iWork_MT,hname2);
      // 	if (item2)
      // 	  fListTree->DeleteItem(item2);
      // }
      //cout << "work3: " << item << " " << hname << endl;
    } //work*
    else { //not work* -> добавляем гистограмму в work*
      AddWork(item);
    }
  } //if (item->GetParent() && (but==2 || but==3))
  
  //cout << "Upd1: " << endl;
  if (crs->b_stop)
    Update();
  else
    changed=true;
  //cout << "Upd2: " << endl;

}

void HistFrame::CheckAll(Bool_t on) {
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    fListTree->CheckAllChildren(idir,on);
    idir = idir->GetNextSibling();
  }
  changed=true;
  ReDraw();
}

void HistFrame::CheckWork() {
  //добавляем все помеченные гистограмы в Work 
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	if (item->IsChecked()) {
	  AddWork(item);
	}
      }
      item = item->GetNextSibling();
    }
    idir = idir->GetNextSibling();
  }
  changed=true;
  ReDraw();
}

void HistFrame::Clone_Ltree(HMap* map) {

  //char cutname[99];
  //char name[99],htitle[99];
  TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");
  TGPicture *pic=0;

  if (*(map->hd->w+map->nn)) {
    if (map->hst->InheritsFrom(TH2::Class()))
      pic=pic2;
    else
      pic=pic1;

    for (int i=1;i<opt.ncuts;i++) {
      if (opt.pcuts[i]) {
	HMap* mcut = map->h_cuts[i];
	if (mcut)
	  Item_Ltree(iWork_cut[i], mcut->GetName(), mcut, pic, pic);
      }
    }

    // if (crs->b_maintrig) {
    //   HMap* mcut = map->h_MT;
    //   if (mcut)
    // 	Item_Ltree(iWork_MT, mcut->GetName(), mcut, pic, pic);
    // }

  }
}

void HistFrame::DoCheck(TObject* obj, Bool_t check)
{
  HMap* map;

  TGListTreeItem *item = fListTree->GetFirstItem();
  item = fListTree->FindItemByObj(item,obj);

  // cout << "DoCheck: " << obj << " " << item << " " << obj->GetName() << " " << obj->GetTitle() << endl;
  // if (item) {
  //   cout << "DoCheck: " << item->GetText() << endl;
  //   cout << "DoCheck: " << item->GetParent() << endl;
  //   cout << "DoCheck: " << item->GetUserData() << " " << check << endl;
  // }

  if (item) {
    if (item->GetParent()) { //single item
      if (!TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) { //not in works* folder
	map = (HMap*) item->GetUserData();
	if (map) {
	  *(map->hd->c+map->nn) = item->IsChecked();
	}
      }
    }
    else { //folder
      if (!TString(item->GetText()).Contains("work",TString::kIgnoreCase)) { //not work* folder
	TGListTreeItem *item2 = item->GetFirstChild();
	while (item2) {
	  map = (HMap*) item2->GetUserData();
	  if (map) {
	    *(map->hd->c+map->nn) = item->IsChecked();
	  }
	  item2 = item2->GetNextSibling();
	}
      }
    }
  }

  // TGListTreeItem *iwork = fListTree->GetFirstItem();
  // cout << "check!: " << item->GetText() << " " << check << " " << iwork->IsChecked() << endl;

  // HMap *map2 = (HMap*) hcl->map_list->FindObject(item->GetText());
  // if (map2) {
  //   cout << "map2: " << map2->GetName() << " " << *map2->chk << " " << opt.s_ntof[0] << endl;
  // }

  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::DoKey(TGListTreeItem* entry, UInt_t keysym, UInt_t mask) {
  cout << "key: " << entry << " " << keysym << " " << mask << endl;
}

void HistFrame::DoRadio()
{
  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  if (id==1) {
    Rb[0]->SetState(kButtonDown);
    Rb[1]->SetState(kButtonUp);
    opt.b_stack=true;
  }
  else {
    Rb[0]->SetState(kButtonUp);
    Rb[1]->SetState(kButtonDown);
    opt.b_stack=false;
  }
  
  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::DoButton()
{
  if (opt.b_stack)
    return;

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId()-1;

  //cout << "dobutton: " << id << endl;

  switch (id) {
  case 0:
    opt.icheck--;
    break;
  case 1:
    opt.icheck++;
    break;
  case 2:
    opt.icheck-=ndiv;
    break;
  case 3:
    opt.icheck+=ndiv;
    break;
  }

  //cout << "doradio2: " << id << endl;
  if (crs->b_stop)
    Update();
  else
    changed=true;
  //cout << "doradio3: " << id << endl;

}

void HistFrame::DoSlider() {

  //printf("DosLider: %d\n",nEvents);
  if (crs->b_stop)
    Update();
  else
    changed=true;

}

void HistFrame::GetHMinMax(TH1* hh, double x1, double x2,
		double &y1, double &y2) {
  for (int i=hh->FindFixBin(x1);i<=hh->FindFixBin(x2);i++) {
    double cc = hh->GetBinContent(i);//+hh->GetBinError();
    if (opt.b_logy) {
      if (cc>0)
	y1 = TMath::Min(y1,cc);
    }
    else {
      y1 = TMath::Min(y1,cc);
    }
    y2 = TMath::Max(y2,cc);
  }
}

void HistFrame::X_Slider(TH1* hh, double &a1, double &a2) {
  Float_t h1,h2;
  fHslider->GetPosition(h1,h2);
  if (h2-h1<1) {
    double x1=hh->GetXaxis()->GetXmin();
    double x2=hh->GetXaxis()->GetXmax();
    double rr = x2-x1;
    a1 = x1+rr*h1;
    a2 = x1+rr*h2;
    hh->GetXaxis()->SetRangeUser(a1,a2);
    //cout << "x_slider: " << hh->GetName() << " " << a1 << " " << a2 << endl;
  }
  else {
    a1=hh->GetXaxis()->GetXmin();
    a2=hh->GetXaxis()->GetXmax();
    hh->GetXaxis()->UnZoom();
  }
}

void HistFrame::Y_Slider(TH1* hh, double a1, double a2, double y1, double y2) {
  Float_t h1,h2;
  fVslider->GetPosition(h2,h1);
  h1=1-h1;
  h2=1-h2;
  //double y1,y2;
  if (h2-h1<1 || hh==st_plot) {
    if (hh->GetDimension()==2) {
      y1=hh->GetYaxis()->GetXmin();
      y2=hh->GetYaxis()->GetXmax();
    }
    else if (hh!=st_plot) {
      y1=1e99;
      y2=-1e99;
      GetHMinMax(hh,a1,a2,y1,y2);
      //x1=hh->GetBinContent(hh->GetMinimumBin())*1.05;
      //x2=hh->GetBinContent(hh->GetMaximumBin())*1.05;
    }

    if (opt.b_logy) {
      if (y2>0) {
	y1/=2;
	y2*=2;
      }
      else {
	y1=0.01;
	y2=1;
      }
    }
    else {
      //y1*=1.1;
      y2*=1.1;
      if (y1>0) y1=0;
      else y1*=1.1;
    }

    double rr = y2-y1;
    double a1 = y1+rr*h1;
    double a2 = y1+rr*h2;
    hh->GetYaxis()->SetRangeUser(a1,a2);
    //cout << "Y_sl: " << hh->GetName() << " " << y1 << " " << y2 << " " << a1 << " " << a2 << endl;
  }
  else {
    hh->GetYaxis()->UnZoom();
  }
}

void HistFrame::AddCutG(TPolyLine *pl, HMap* map) {

  // if (opt.b_stack)
  //   return;

  // check if the histogram exists
  if (!map) {
    cout << "AddCutg: histogram not found: " << endl;
    return;
  }

  TCanvas* cv=fEc->GetCanvas();
  //cv->SetCrosshair(false);
  for (int i=0;i<=ndiv;i++) {
    TPad* pad = (TPad*) cv->GetPad(i);
    if (pad) {
      pad->GetListOfExecs()->Clear();
    }
  }

  if (in_gcut==1) { //cut
    MakeCutG(pl,map);
  }
  else if (in_gcut==2) { //roi
    MakeROI(pl,map);
  }
  in_gcut=0;
  Update();

}

int HistFrame::Find_icut() {
  int icut=-1;
  for (int i=1;i<opt.ncuts;i++) {
    if (opt.pcuts[i]==0) { //найден пустой cut внутри
      icut=i;
      return icut;
    }
  }
  //не найден пустой cut, добавляем в конец
  icut=opt.ncuts;
  if (icut==0)
    icut=1;
  if (icut>=MAXCUTS)
    icut=-1;

  return icut;
}

void HistFrame::MakeCutG(TPolyLine *pl, HMap* map) {

  int icut=Find_icut();
  if (icut<0) {
    cout << "Too many cuts: " << icut << endl;
    return;
  }
  //cout << "ncuts: " << icut << " " << opt.ncuts << endl;

  TGListTreeItem *idir = fListTree->GetFirstItem();
  if (strcmp(idir->GetText(),"WORK")) {
    cout << "first item is not WORK" << endl;
    return;
  }

  //1. add current cut to the cut_index of hmap
  //(map->cut_index) ?? check it 
  //cout << "make1: " << endl;
  //setbit(*(map->cut_index),icut,1);
  setbit(*(map->hd->cut+map->nn),icut,1);

  // for (int i=0;i<MAXCUTS;i++) {
  //   if (map->cut_index[i]==0) {
  //     map->cut_index[i]=icut+1;
  //     break;
  //   }
  // }

  //cout << "make2: " << pl->Size() << endl;
  //1a fill opt.** variables
  opt.pcuts[icut] = pl->Size();
  //cout << "pl: " << endl;
  //pl->Print();
  for (int i=0;i<pl->Size();i++) {
    opt.gcut[icut][0][i]=pl->GetX()[i];
    opt.gcut[icut][1][i]=pl->GetY()[i];
  }

  //hcl->cutmap[icut]=map;

  //cout << "make3: " << endl;
  //5. remake Ltree
  Clear_Ltree();
  hcl->Make_cuts();
  Make_Ltree();
  //cout << "make5: " << endl;
  //Update();

}

//void HistFrame::ItemROI(HMap* map, int ich, int iroi) {
//YKYK
void HistFrame::ItemROI(HMap* map, int iroi) {
  char name[99];
  TGPicture *pic = (TGPicture*) gClient->GetPicture("hor_arrow_cursor.png");
  TGPicture *pic_xy = (TGPicture*) gClient->GetPicture("marker7.xpm");
  sprintf(name,"%s_%02d",map->GetTitle(),iroi+1);
  TGListTreeItem *item = fRoiTree->AddItem(0, name, map->hd->roi[iroi], pic, pic, false);
  for (int j=0;j<2;j++) {
    sprintf(name,"%f",map->hd->roi[iroi][j]);
    fRoiTree->AddItem(item, name, pic_xy, pic_xy, false);
  }  
}

void HistFrame::MakeROI(TPolyLine *pl, HMap* map) {

  int iroi;
  //find first empty roi
  for (iroi=0;iroi<MAXROI;iroi++) {
    if (empty_roi(map->hd,iroi))
      break;
  }

  if (iroi>=MAXROI) {
    cout << "Too many ROI: " << iroi << endl;
    return;
  }

  //TGListTreeItem *item=0;
  if (pl->Size()>=2) {
    map->hd->roi[iroi][0]=pl->GetX()[0];
    map->hd->roi[iroi][1]=pl->GetX()[1];

    ItemROI(map,iroi);    
  }

  // cout << "r: " << iroi << " " << map->GetName() << " " << map->GetTitle() << endl;
  // for (int i=0;i<MAXROI;i++) {
  //   cout << "roi: " << i << " " << map->hd->roi[i][0]
  // 	 << " " << map->hd->roi[i][1] << endl;
  // }

}

void HistFrame::CutClick(TGListTreeItem* item,Int_t but) {
  if (!crs->b_stop)
    return;

  if (!(but==2 || but==3)) {
    return;
  }
  TCutG* gcut;

  //cout << "CutClick: " << item->GetText() << " " << item->GetUserData() << " " << item->GetParent() << " " << but << endl;

  gcut = (TCutG*) item->GetUserData();
  if (gcut) {
    TString ss = TString(gcut->GetName());
    //ss.ReplaceAll("cut","");
    //ss.ReplaceAll("[","");
    //ss.ReplaceAll("]","");
    int icut = ss.Atoi();
    //cout << "cutclick2: " << gcut->GetName() << " " << gcut->GetTitle() << " " << ss << " " << icut << endl;

    if (icut<0 && icut >=MAXCUTS) {
      cout << "wrong cut number: " << icut << ". Consider clearing all cuts" << endl;
      return;
    }
    //icut--;

    HMap *map = (HMap*) hcl->map_list->FindObject(gcut->GetTitle());
    //clear cut from cut_index
    if (map) {
      setbit(*(map->hd->cut+map->nn),icut,0);
    }

    fCutTree->DeleteChildren(item);
    fCutTree->DeleteItem(item);

    opt.pcuts[icut]=0;
    memset(opt.gcut[icut],0,sizeof(opt.gcut[icut]));

    Clear_Ltree();
    hcl->Make_cuts();
    Make_Ltree();
    Update();
  
  } // if gcut
}

void HistFrame::RoiClick(TGListTreeItem* item,Int_t but) {
  if (!crs->b_stop)
    return;

  if (!(but==2 || but==3)) {
    return;
  }

  Float_t* roi = (Float_t*) item->GetUserData();

  if (roi) {
    roi[0]=0;
    roi[1]=0;

    fRoiTree->DeleteChildren(item);
    fRoiTree->DeleteItem(item);
  } // if roi

}

void HistFrame::StartMouse() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();

  if (id==1 && !opt.b_stack) { //add Cut
    int icut=Find_icut();
    if (icut<0) {
      const char* msg = "Too manu cuts. Delete them first";
      Int_t retval;
      new TGMsgBox(gClient->GetRoot(), this,
	       "Graphical cut",
	       msg, kMBIconAsterisk, kMBDismiss, &retval);
      return;
    }
    in_gcut=1;
  }
  else if (id==2) { //add ROI
    in_gcut=2;
  }
  else { //cancel
    in_gcut=0;
  }

  Update();
  
}

void HistFrame::AddFormula() {
  Pixel_t color;
  int len = strlen(tForm->GetText());
  //cout << "len: " << len << endl;
  TFormula form = TFormula("form",tForm->GetText());
  int ires = form.Compile();
  //formula is not valid
  if (ires || len>=16) {
    gClient->GetColorByName("red", color);
    tForm->SetBackgroundColor(color);
  }
  //formula is valid
  else {

    int icut=Find_icut();
    if (icut<0) {
      const char* msg = "Too manu cuts. Delete them first";
      Int_t retval;
      new TGMsgBox(gClient->GetRoot(), this,
	       "Graphical cut",
	       msg, kMBIconAsterisk, kMBDismiss, &retval);
      return;
    }
    //cout << "ncuts: " << icut << " " << opt.ncuts << endl;

    gClient->GetColorByName("white", color);
    tForm->SetBackgroundColor(color);
    opt.pcuts[icut]=1;
    opt.gcut[icut][0][0]=0;
    opt.gcut[icut][1][0]=0;
    strcpy(opt.cut_form[icut],tForm->GetText());
    //opt.ncuts++;

    Clear_Ltree();
    hcl->Make_cuts();
    Make_Ltree();
    
  }
}

// void HistFrame::DoFormula() {
//   TGTextEntry *te = (TGTextEntry*) gTQSender;
//   //Int_t id = te->WidgetId();
//   strcpy(opt.cut_form,te->GetText());
//   //cout << "DoFormula: " << te->GetText() << " " << opt.formula << endl;
// }

void HistFrame::ClearCutG()
{
  //cout << "Clear_cuts: " << endl;

  // clear cut_index for all histograms
  TIter next(hcl->map_list);
  TObject* obj=0;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;
    *(map->hd->cut+map->nn)=0;
    //memset(map->cut_index,0,MAXCUTS);
  }
  

  Clear_Ltree();

  //opt.gcut[0]->Print();
  for (int i=1;i<opt.ncuts;i++) {
    delete hcl->cutG[i];
    hcl->cutG[i]=0;
  }

  //Clear_tree();
  opt.ncuts=0;
  memset(opt.pcuts,0,sizeof(opt.pcuts));
  memset(opt.gcut,0,sizeof(opt.gcut));
  fEc->GetCanvas()->Update();

  hcl->Make_cuts();
  Make_Ltree();

}

string HistFrame::CutsToStr()
{
  std::ostringstream oss;

  oss << "#NCUT NP hist" <<endl;
  oss << "#  X Y (NP lines)" <<endl;

  for (int i=1;i<opt.ncuts;i++) {
    if (opt.pcuts[i]) {

      TCutG* gcut = hcl->cutG[i];
      if (gcut->GetN() > 1) {
	oss << gcut->GetName() << " " << gcut->GetN()
	    << " " << gcut->GetTitle() << endl;;
	for (int j=0;j<gcut->GetN();j++) {
	  oss << "  " << gcut->GetX()[j] << " " << gcut->GetY()[j] << endl;
	}
      }
      else {
	oss << gcut->GetName() << " 1 formula " << endl
	    << "  " << opt.cut_form[i] << endl;
      }
    }
  }

  //std::string s = oss.str();
  //std::cout << s;
  return oss.str();
}

void HistFrame::EditCutG()
{
  new PEditor(this, M_EDIT_CUTG, 400, 520);
}

void HistFrame::DoPeaks()
{
  /*
  TSpectrum spec;
  
  int nn=1;
  TIter next(st_hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (!fEc->GetCanvas()->GetPad(nn)) break;
    fEc->GetCanvas()->cd(nn);
    TH1 *hh = (TH1*) obj;
    //cout << "hhh: " << hh->GetTitleSize() << endl;
    //hh->Draw();
    int npk = spec.Search(hh,2,"",0.5);

#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    Double_t* peaks = spec.GetPositionX();
#else
    Float_t* peaks = spec.GetPositionX();
#endif

    for (int j=0;j<npk;j++) {
      int bin = hh->FindFixBin(peaks[j]);
      int k;
      for (k=bin;k>0;k--) {
	if (hh->GetBinContent(k)<spec.GetPositionY()[j]*0.5)
	  break;
      }
      //cout << hh->GetName() << " " << j << " " << peaks[j] << " " << bin
      //     << " " << spec.GetPositionY()[j] << " " << bin-k << endl;
      double sig = (bin-k)*2.0/2.35;

      int width=(bin-k)*5;

      TF1* f1=new TF1("fitf","gaus(0)+pol1(3)",peaks[j]-width,peaks[j]+width);
      //cout << f1->GetNpar() << endl;
      f1->SetParameters(spec.GetPositionY()[j],peaks[j],sig,0,0);

      //f1->Print();
      const char* fitopt="+";
      if (j==0) fitopt="";

      //TF1* fitf=new TF1("fitf","gaus",0,10);
      hh->Fit(f1,fitopt,"",peaks[j]-width,peaks[j]+width);
      //f1->Draw("same");
    }
    nn++;
  }
  //fEc->GetCanvas()->SetEditable(true);
  fEc->GetCanvas()->Update();
  //fEc->GetCanvas()->SetEditable(false);
*/
}

void HistFrame::PeakFit(HMap* map,TH1* hist1,TH1* hist2,int nn,d2vect &d2) {
			   // double &yy, double &er) {

  //cout << "PeakFit: " << map->GetName() << endl;
  //yy - найденное положение пика в каналах
  //er - ошибка yy
  int iroi = ee_calib[nn][0];

  int b1 = hist2->FindFixBin(map->hd->roi[iroi][0]);
  int b2 = hist2->FindFixBin(map->hd->roi[iroi][1]);
  double x1 = hist1->GetBinCenter(b1);
  double x2 = hist1->GetBinCenter(b2);
  double y1 = hist1->GetBinContent(b1);
  double y2 = hist1->GetBinContent(b2);
  double ymax = hist1->GetBinContent(hist1->GetMaximumBin());
  //double bkg = (y1+y2)*0.5;
  double bkg = TMath::Min(y1,y2);

  //d2vect dd;

  /*
  cout << "peak: " << iroi;
  for (UInt_t i=2;i<ee_calib[nn].size();i+=2) {
    cout << " " << ee_calib[nn][i-1] << " " << ee_calib[nn][i];
  }
  cout << endl;
  return;
  */
  int np; //number of parameters
  TString str;
  dvect fpar;
  fpar.push_back(bkg);
  fpar.push_back(0);
  if (bpol==2) {
    str="pol2(0)";
    np=3;
    fpar.push_back(0);
  }
  else {
    str="pol1(0)";
    np=2;
  }

  for (UInt_t i=2;i<ee_calib[nn].size();i+=2) {
    double ff=ee_calib[nn][i];
    double imax;
    double sig;
    double max=-9999999999;

    if (ff>0 && ff<1) { //fixed peak
      imax = x1+(x2-x1)*ff;
      sig = fitsig;
      int bin = hist1->FindFixBin(imax);
      max=hist1->GetBinContent(bin);
      //cout << "imax: " << imax << " " << bin << " " << max << " " << bkg << " " << nofit << endl;
    }
    else { //search
      for (int j=b1;j<=b2;j++) {
	double zz = hist1->GetBinContent(j)-bkg;
	if (zz>max) {
	  max=zz;
	  imax=hist1->GetBinCenter(j);
	}
	if (zz<max/2) {
	  sig=hist1->GetBinCenter(j)-imax;
	  break;
	}
      }

      cout << "srch: " << map->hst->GetName() << " " << iroi
	   << " " << b1 << " " << b2
	   << " " << imax << " " << max << " " << sig
	   << endl;
    } //else (search)  

    fpar.push_back(max-bkg); //ampl
    fpar.push_back(imax); //pos
    fpar.push_back(sig); //sig
    str+=TString::Format("+gaus(%d)",np);
    np+=3;
    d2[1].push_back(ee_calib[nn][i-1]);

  } //for i

  // cout << str << " " << np << " " << d2.size() << " " << fpar.size() << endl;
  // for (UInt_t i=0;i<fpar.size();i++) {
  //   cout << " " << fpar[i];
  // }
  // cout << endl;

  TF1* f1=new TF1("fitf",str,x1,x2);
  f1->SetParameters(fpar.data());
  if (!nofit)
    hist1->Fit(f1,"NQ","",x1,x2);

  hist1->GetListOfFunctions()->Add((TF1*) f1->Clone());
  hist1->GetListOfFunctions()->Add(new TLine(x1,y1,x1,ymax));
  hist1->GetListOfFunctions()->Add(new TLine(x2,y2,x2,ymax));

  int npk = (np-2)/3;
  int nbg = np-npk*3;

  // cout << "pmap: " << map->GetName() << endl;
  // for (int i=0;i<f1->GetNpar();i++) {
  //   cout << " " << f1->GetParameter(i);
  // }
  // cout << endl;

  for (int i=0;i<npk;i++) {
    d2[0].push_back(f1->GetParameter(nbg+i*3+1));
    d2[2].push_back(f1->GetParError(nbg+i*3+1));
  }
  //yy = f1->GetParameter(1);
  //er = f1->GetParError(1);

  //cout << "npk: " << nn << " " << npk << " " << nbg << " " << endl;

}

void HistFrame::DelMaps(TGListTreeItem *idir) {
  if (idir) {
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	HMap* map = (HMap*) item->GetUserData();
	//cout << "map1: " << map->GetName() << " " << map->hd << endl;
	if (map) {
	  // if (!hd) {
	  //   hd=map->hd;
	  // }
	  delete map;
	}
      }
      item = item->GetNextSibling();
    }

    fListTree->DeleteChildren(idir);
    fListTree->DeleteItem(idir);
  }
}

void HistFrame::Do_Ecalibr(PopFrame* pop) {

  TString name;

  TGPicture *pic = (TGPicture*) gClient->GetPicture("h1_t.xpm");

  //Toptions &opt = *(gg->g_opt);
  //cout << "Do_ecalibr2: " << endl;
  HMap *map, *map1, *map2;
  Hdef *hd1=0, *hd2=0;
  const char* mapname1 = "Efit";
  const char* mapname2 = "Ecalibr";

  TGListTreeItem *iroot=0;
  TGListTreeItem *idir1, *idir2;

  hmap_chklist->Clear();

  idir1=fListTree->FindChildByName(0,mapname1);
  DelMaps(idir1);
  idir2=fListTree->FindChildByName(0,mapname2);
  DelMaps(idir2);

  if (hd1) delete hd1;
  hd1 = new Hdef();
  if (hd2) delete hd2;
  hd2 = new Hdef();


  idir1 = Item_Ltree(iroot,mapname1,0,0,0);;
  idir2 = Item_Ltree(iroot,mapname2,0,0,0);;

  Make_Hmap_ChkList();

  TIter next(hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    map=(HMap*) obj;
    if (map->hd==&opt.h_area) {

      //map->hst->GetListOfFunctions()->Clear();

      //hist1 - not b_adjusted histogram, fitted
      //hist2 - b_adjusted histogram, only for ROI
      name = mapname1 + TString::Format("_%02d",map->nn);
      TH1* hist1 = (TH1*) map->hst->Clone(name.Data());
      hist1->SetTitle(name);
      OneRebinPreCalibr(map, hist1, false);

      name = "tmp" + TString::Format("_%02d",map->nn);
      TH1* hist2 = (TH1*) map->hst->Clone(name.Data());
      OneRebinPreCalibr(map, hist2, true);

      //hist2->GetListOfFunctions()->Clear();

      hist1->UseCurrentStyle();
      hist2->UseCurrentStyle();
      hist2->SetLineColor(2);

      //double xx[100],yy[100],er[100];
      d2vect d2;
      for (int i=0;i<3;i++) {
	d2.push_back(dvect());
      }
      for (UInt_t i=0;i<ee_calib.size();++i) {
	//int iroi = pl->GetX()[i];
	//xx[i]=pl->GetY()[i];
	//cout << "p1: " << i << endl;
	PeakFit(map,hist1,hist2,i,d2);
	//cout << "p2: " << i << endl;
      }

      //TCanvas* c1 = new TCanvas("c1","c1");
      //hist1->DrawCopy();
      //hist->DrawCopy("same");

      map1 = new HMap(mapname1,hist1,hd1,map->nn);
      Item_Ltree(idir1, map1->GetName(), map1, pic, pic);
      delete hist2;

      // cout << "11: " << map->GetName() << " " << d2[0].size()
      // 	   << " " << d2[1].size()
      // 	   << " " << d2[2].size()
      // 	   << endl;

      // for (UInt_t i=0;i<d2[0].size();i++) {
      // 	cout << i << " " << d2[0][i] << " " << d2[1][i] << " " << d2[2][i] << endl;
      // }
      
      //TGraphErrors* gr = new TGraphErrors(pl->GetN(),yy,xx,er,0);
      TGraphErrors* gr =
	new TGraphErrors(d2[0].size(),d2[0].data(),d2[1].data(),d2[2].data(),0);
      //cout << "11a: " << map->GetName() << endl;
      name = mapname2 + TString::Format("_%02d",map->nn);
      gr->SetNameTitle(name,name);
      gr->SetMarkerStyle(24);
      gr->SetMarkerSize(1);

      //cout << "22: " << map->GetName() << endl;


      TF1* fitf = new TF1("fpol2","pol2");
      fitf->SetParameters(0,1,0);
      fitf->FixParameter(2,0);
      gr->Fit(fitf,"Q");
      fitf->ReleaseParameter(2);
      gr->Fit(fitf,"Q");


      //cout << "33: " << map->GetName() << endl;

      //gr->GetListOfFunctions()->ls();
      //TF1* fitf = gr->GetFunction("pol2");


      for (int j=0;j<3;j++) {
	opt.adj[map->nn][j]=fitf->GetParameter(j);
      }
      
      //cout << "Par: " << fitf->GetParameters() << endl;

      map2 = new HMap(mapname2,gr,hd2,map->nn);
      Item_Ltree(idir2, map2->GetName(), map2, pic, pic);

      //break;

    }
  } //next

} //Do_Ecalibr

//--------------------------------------------
void HistFrame::Do_Tcalibr(PopFrame* pop) {

  TString name;

  TGPicture *pic = (TGPicture*) gClient->GetPicture("h1_t.xpm");

  HMap *map, *map1;
  Hdef *hd1=0;
  const char* mapname1 = "Tfit";

  TGListTreeItem *iroot=0;
  TGListTreeItem *idir1;

  hmap_chklist->Clear();

  idir1=fListTree->FindChildByName(0,mapname1);
  DelMaps(idir1);

  if (hd1) delete hd1;
  hd1 = new Hdef();


  idir1 = Item_Ltree(iroot,mapname1,0,0,0);;
  Make_Hmap_ChkList();

  TIter next(hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    map=(HMap*) obj;
    if (map->hd==&opt.h_time && map->hst->Integral()) {
      name = mapname1 + TString::Format("_%02d",map->nn);
      TH1* hist1 = (TH1*) map->hst->Clone(name.Data());
      hist1->SetTitle(name);
      OneRebinPreCalibr(map, hist1, false);

      hist1->UseCurrentStyle();

      double imax = hist1->GetBinCenter(hist1->GetMaximumBin());
      double ymax = hist1->GetBinContent(hist1->GetMaximumBin());
      double x1 = imax-pop->range;
      double x2 = imax+pop->range;
      double y1 = hist1->GetBinContent(hist1->FindFixBin(x1));
      double y2 = hist1->GetBinContent(hist1->FindFixBin(x2));
      double bkg=(y1+y2)/2;

      // cout << "Tfit: " << name << " " << imax << " " << ymax
      // 	   << " " << x1 << " " << x2 << endl; 

      TF1* f1=new TF1("fitf","pol1(0)+gaus(2)",x1,x2);
      f1->SetParameters(bkg,0,ymax,imax,pop->fwhm/2.35);

      hist1->Fit(f1,"N","",x1,x2);
      pop->sD[map->nn]=f1->GetParameter(3);

      hist1->GetListOfFunctions()->Add((TF1*) f1->Clone());
      hist1->GetListOfFunctions()->Add(new TLine(x1,0,x1,ymax));
      hist1->GetListOfFunctions()->Add(new TLine(x2,0,x2,ymax));
      
      // d2vect d2;
      // for (int i=0;i<3;i++) {
      // 	d2.push_back(dvect());
      // }
      // for (UInt_t i=0;i<ee_calib.size();++i) {
      // 	PeakFit(map,hist1,hist2,i,d2);
      // }

      map1 = new HMap(mapname1,hist1,hd1,map->nn);
      Item_Ltree(idir1, map1->GetName(), map1, pic, pic);


      // for (int j=0;j<3;j++) {
      // 	opt.adj[map->nn][j]=fitf->GetParameter(j);
      // }
      
      // map2 = new HMap(mapname2,gr,hd2,map->nn);
      // Item_Ltree(idir2, map2->GetName(), map2, pic, pic);

      //break;

    }
  } //next

  Update();
} //Do_Tcalibr

/*
void HistFrame::Do_Ecalibr()
{
  double sig = myM->p_pop->fwhm/2.35;
  double width = myM->p_pop->range;
  double* ee = myM->p_pop->ee;
  
  cout << "eee: " << sig << endl;
  //return;

  TSpectrum spec;

  int nch;
  int nn=1;
  TIter next(st_hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (!fEc->GetCanvas()->GetPad(nn)) break;
    fEc->GetCanvas()->cd(nn);
    TH1 *hh = (TH1*) obj;
    string str(hh->GetName());
    size_t found = str.find_last_of("_");
    std::stringstream ss(str.substr(found+1));
    ss >> nch;

    //cout << "hhh: " << hh->GetTitleSize() << endl;
    //hh->Draw();
    int npk = spec.Search(hh,sig,"",0.5);
#if ROOT_VERSION_CODE > ROOT_VERSION(6,0,0)
    Double_t* peaks = spec.GetPositionX();
#else
    Float_t* peaks = spec.GetPositionX();
#endif
    for (int j=0;j<npk;j++) {
      //int bin = hh->FindFixBin(peaks[j]);

      TF1* f1=new TF1("fitf","gaus(0)+pol1(3)",peaks[j]-width,peaks[j]+width);
      //cout << f1->GetNpar() << endl;
      f1->SetParameters(spec.GetPositionY()[j],peaks[j],sig,0,0);

      //f1->Print();
      const char* fitopt="NQ+";
      if (j==0) fitopt="NQ";

      //TF1* fitf=new TF1("fitf","gaus",0,10);
      hh->Fit(f1,fitopt,"",peaks[j]-width,peaks[j]+width);
      f1->Draw("same");
      opt.E1[nch]*=ee[0]/f1->GetParameter(1);
      cout << "hist: " << nch << " " << opt.E1[nch] << " " << hh->GetName() << " " << f1->GetParameter(1) << endl;
    }
    nn++;
  }
  //fEc->GetCanvas()->SetEditable(true);
  fEc->GetCanvas()->Update();
  //fEc->GetCanvas()->SetEditable(false);

}
*/

void HistFrame::DoRst() {

  //cout << "DoRst: " << endl;
  
  TIter next(hcl->allmap_list);

  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;
    if (map->hst->GetEntries() > 0) {
      map->hst->Reset();
    }
  }

  //UpdateStatus(1);

}

void HistFrame::HiReset()
{

  //cout << "HiReset1: " << endl;
  
  if (!crs->b_stop) return;

  in_gcut=0;

  //cout << "Make_hist():: " << endl;
  Clear_Ltree();
  //cout << "Make_hist2():: " << endl;
  hcl->Make_hist();
  //cout << "Make_hist3():: " << endl;
  Make_Ltree();
  //cout << "Make_hist()::2 " << endl;


  TCanvas *cv=fEc->GetCanvas();
  //cv->SetEditable(true);
  cv->Clear();
  //cv->SetEditable(false);
  //cv->Draw();
  cv->Update();

  Update();
  //cout << "HiReset2: " << endl;
}

void HistFrame::Update()
{
  //cout << "in_gcut: " << in_gcut << " " << opt.b_logy << " " << chklog << endl;

  Hmut.Lock();

  if (opt.b_stat) {
    gStyle->SetOptStat(1002211);
  }
  else {
    gStyle->SetOptStat(0);
  }
  

  //xdiv=2;
  //ydiv=2;
  int n2 = (int) opt.b_stack;
  Rb[1-n2]->SetState(kButtonDown);
  Rb[n2]->SetState(kButtonUp);

  if (opt.b_stack)
    ndiv=1;
  else
    ndiv=opt.xdiv*opt.ydiv;

  //int sel = abs(opt.sel_hdiv)%NR;
  //SelectDiv(sel);

  //chknorm->SetState((EButtonState) opt.b_norm);
  //chklog->SetState((EButtonState) opt.b_logy);
  //chkstat->SetState((EButtonState) opt.b_stat);
  //chkgcuts->SetState((EButtonState) opt.b_gcuts);

  //cout << "HiFrm::Update()2a" << endl;
  hmap_chklist->Clear();
  //st_hlist->Clear();
  //cout << "HiFrm::Update()2b" << endl;

  //Hmut.UnLock();
  //return;

  if (opt.b_stack) {
    DrawStack();
  }
  else {
    DrawHist();
  }

  // make cuts if selected
  TCanvas* cv=fEc->GetCanvas();
  if (in_gcut) {
    np_gcut=0;
    cv->SetCrosshair(true);
    // if (opt.b_stack) {
    //   gPad->AddExec("dynamic","DynamicExec()");
    // }
    // else {
    for (int i=0;i<=ndiv;i++) {
      TPad* pad = (TPad*) cv->GetPad(i);
      if (pad)
	pad->AddExec("dynamic","DynamicExec()");
    }
    //}
  }
  else {
    cv->SetCrosshair(false);
    HiFrm->fStatus->SetText("");
  }



  //hmap_chklist->Print();

  Hmut.UnLock();
  //cout << "hist_list: " << endl;
  //hcl->hist_list->ls();
  //cout << "hist_list: " << hcl->hist_list << " " << hcl->hist_list->GetSize() << endl;


  // TIter next(hmap_chklist);
  // TObject* obj;
  // while ( (obj=(TObject*)next()) ) {
  //   HMap* map=(HMap*) obj;
  //   cout << "hmap_list_update: " << map << " " << map->GetName() << endl;
  //   //cout << "hmap_list2: " << map->GetName() << endl;
  // }

  //cout << "HiFrm::Update()3" << endl;

}

void HistFrame::DrawStack() {

  // for (int i=0;i<10;i++) {
  //   cout << "adj: " << adj[i][1] << endl;
  // }
  std::vector<Hdef*> def_list;
  std::vector<Hdef*>::iterator it;
 
  Make_Hmap_ChkList();

  double x1=1e99,x2=-1e99,y1=1e99,y2=-1e99;

  int cc=0;
  //memset(pad_map,0,sizeof(pad_map));
  std::vector <TH1*>::iterator ih;
  for (ih = pad_hist.begin(); ih != pad_hist.end(); ++ih) {
    if (*ih)
      delete *ih;
  }
  pad_hist.clear();
  pad_map.clear();
  TIter next2(hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next2()) ) {
    HMap* map=(HMap*) obj;
    if (map->hst && map->hst->GetDimension()==1) { //only 1d hist

      pad_map.push_back(map);

      it = find (def_list.begin(), def_list.end(), map->hd);
      if (it == def_list.end()) {
	def_list.push_back(map->hd);
      }
	
      TString name(map->hst->GetName());
      name+=' ';

      TH1* hh = (TH1*) map->hst->Clone(name.Data());
      hh->UseCurrentStyle();
      OneRebinPreCalibr(map,hh,b_adj);

      //hh->SetLineColor(EvtFrm->chcol[map->nn]);
      hh->SetLineColor(EvtFrm->chcol[cc%32]);
      cc++;

      if (opt.b_norm && hh->Integral()) {
	double sc = hh->GetNbinsX()/hh->Integral();
	hh->Scale(sc);
      }
      pad_hist.push_back(hh);
      //st_hlist->Add(hh);

      x1 = TMath::Min(x1,hh->GetXaxis()->GetXmin());
      x2 = TMath::Max(x2,hh->GetXaxis()->GetXmax());
    }
  } //while ( (obj=(TObject*)next2()) )

  
  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  cv->Divide(1,1);
  cv->SetLogy(opt.b_logy);

  //if (st_hlist->IsEmpty()) {
  if (pad_hist.empty()) {
    cv->Update();
    return;
  }

  //cout << "x12: " << x1 << " " << x2 << endl;

  //set x-axis
  st_plot->SetBins(1000,x1,x2);
  double a1,a2;
  X_Slider(st_plot,a1,a2);

  //cout << "x12: " << x1 << " " << x2 << " " << a1 << " " << a2 << endl;
  //determine y-axis ranges

  for (ih = pad_hist.begin(); ih != pad_hist.end(); ++ih) {
    if (*ih)
      GetHMinMax(*ih,a1,a2,y1,y2);
  }

  Y_Slider(st_plot,a1,a2,y1,y2);

  //cout << "y12: " << y1 << " " << y2 << " " << a1 << " " << a2 << endl;

  st_plot->Draw("axis");

  for (ih = pad_hist.begin(); ih != pad_hist.end(); ++ih) {
    if (*ih)
      (*ih)->Draw("samehist");
  }

  if (opt.b_stat) {
    TLegend* leg = new TLegend(0.65,0.75,0.95,0.95);
    for (ih = pad_hist.begin(); ih != pad_hist.end(); ++ih) {
      if (*ih) {
	leg->AddEntry((*ih), (*ih)->GetName(), "lpf" );
	leg->Draw();
      }
    }
  }

  // cout << "def_list: ";
  // for (it=def_list.begin(); it!=def_list.end(); ++it) {
  //   cout << *it;
  // }
  // cout << endl;

  if (opt.b_roi) {
    for (it=def_list.begin(); it!=def_list.end(); ++it) {
      DrawRoi(*it,gPad);
    }
  }

  cv->Update();

} //DrawStack

void HistFrame::Make_Hmap_ChkList() {
  hmap_chklist->Clear();
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	if (item->IsChecked()) {
	  hmap_chklist->Add((TObject*)item->GetUserData());
	}
      }
      item = item->GetNextSibling();
    }
    idir = idir->GetNextSibling();
  }
}

void HistFrame::DrawHist() {

  //create hmap_chklist
  Make_Hmap_ChkList();

  if (opt.icheck > hmap_chklist->GetSize()-ndiv)
    opt.icheck=hmap_chklist->GetSize()-ndiv;
  if (opt.icheck < 0)
    opt.icheck=0;

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  //memset(pad_map,0,sizeof(pad_map));

  std::vector <TH1*>::iterator ih;
  for (ih = pad_hist.begin(); ih != pad_hist.end(); ++ih) {
    if (*ih)
      delete *ih;
  }
  pad_hist.clear();
  pad_map.clear();

  double mg=0.001;
  if (opt.xdiv+opt.ydiv>16)
    mg=0;
  cv->Divide(opt.xdiv,opt.ydiv,mg,mg);

  int npad=0;
  int ii=0;

  //draw hists
  HMap* map;

  // create pad_hist from pad_map
  TIter next(hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (ii>=opt.icheck) {
      cv->cd(npad+1);
      map=(HMap*) obj;
      pad_map.push_back(map);
      if (npad+1 != (int)pad_map.size()) {
	cout << "pad_map size doesn't match: " << npad << " " << pad_map.size() << endl;
	return;
      }
      //pad_map[npad]=map;

      TH1* hh=0;
      if (map->hst) { //histogram
	TString name(map->hst->GetName());
	name+=' ';
	//if (pad_hist[npad]) delete pad_hist[npad];
	//pad_hist[npad] = (TH1*) map->hst->Clone(name.Data());
	hh = (TH1*) map->hst->Clone(name.Data());
	hh->UseCurrentStyle();
      }
      else if (map->gr) { //graph
	map->gr->Draw("AP");
      }
      pad_hist.push_back(hh);

      npad++;
      if (npad>=ndiv)
	break;
    }
    ii++;
  } //while ( (obj=(TObject*)next()) )

  // draw pad_hist

  AllRebinDraw();
  cv->Update();

  /*
  // make cuts if selected
  //TCanvas* cv=fEc->GetCanvas();
  if (in_gcut) {
    np_gcut=0;
    cv->SetCrosshair(true);
    for (int i=1;i<=ndiv;i++) {
      TPad* pad = (TPad*) cv->GetPad(i);
      if (pad)
	pad->AddExec("dynamic","DynamicExec()");
    }
  }
  else {
    cv->SetCrosshair(false);
    HiFrm->fStatus->SetText("");
  }
  */
  
} //DrawHist

void HistFrame::OneRebinPreCalibr(HMap* &map, TH1* &hist, bool badj) {
  Float_t *hold, *hnew;

  int nn = map->nn;

  Int_t nx = map->hst->GetNbinsX();
  Int_t rb=map->hd->rb;
  if (rb<1)
    rb=1;
  if (rb>nx)
    rb=nx;

  Int_t newx = nx/rb;

  TAxis* xa = map->hst->GetXaxis();
  Double_t xmin  = xa->GetXmin();
  Double_t xmax  = xa->GetXmax();

  if (map->hst->GetDimension()==1) { //1d hist -> rebin

    TString str = map->GetTitle();

    if (badj && !str.Contains("Efit",TString::kIgnoreCase)) {// map->nn==1) {
      Double_t *xbins = new Double_t[newx+1];
      Double_t dx=(xmax-xmin)/newx;

      for (int i=0;i<newx+1;i++) {
	double x = xmin+i*dx;
	xbins[i]=opt.adj[nn][0]+x*opt.adj[nn][1]+x*x*opt.adj[nn][2];
      }

      hist->SetBins(newx,xbins);
      delete[] xbins;
    }
    else {
      hist->SetBins(newx,xmin,xmax);
    }

    hold = ((TH1F*)map->hst)->GetArray();
    hnew = ((TH1F*)hist)->GetArray();
    memset(hnew,0,((TH1F*)hist)->GetSize()*sizeof(*hnew));

    //hist->Reset();
    for (int i=0;i<nx;i++) {
      int j=i/rb;
      hnew[j+1]+=hold[i+1];
    }
  }
  else { //2d hist

    //prnt("ss ss;",BRED,"hst:",map->hst->GetName(),RST);    

    Int_t ny = map->hst->GetNbinsY();
    Int_t rb2=map->hd->rb2;
    if (rb2<1)
      rb2=1;
    if (rb2>ny)
      rb2=ny;

    //prnt("ss d ds;",BGRN,"ny:",ny,rb2,RST);

    Int_t newy = ny/rb2;

    TAxis* ya = map->hst->GetYaxis();

    //cout << "ya: " << ya << endl;

    Double_t ymin  = ya->GetXmin();
    Double_t ymax  = ya->GetXmax();

    //prnt("ss d f f d f fs;",BRED,"newx:",newx,xmin,xmax,newy,ymin,ymax,RST);
    ((TH2F*) hist)->SetBins(newx,xmin,xmax,newy,ymin,ymax);

    //prnt("ss d f f d f fs;",BBLU,"newx:",newx,xmin,xmax,newy,ymin,ymax,RST);
    hnew = ((TH2F*) hist)->GetArray();
    memset(hnew,0,((TH2F*)hist)->GetSize()*sizeof(*hnew));

    for (int i=0;i<nx;i++) {
      int i2=i/rb;
      for (int j=0;j<ny;j++) {
	int j2=j/rb2;
	hist->AddBinContent(hist->GetBin(i2+1,j2+1),
			    map->hst->GetBinContent(i+1,j+1));
      }
    }
    
    // nx = ((TH2F*) map->hst)->GetSize();
    // hold = ((TH2F*) map->hst)->GetArray();
    // hnew = ((TH2F*) hist)->GetArray();
    // memcpy(hnew,hold,nx*sizeof(Float_t));
  }

  hist->ResetStats();
}

void HistFrame::AllRebinDraw() {

  //cout << "AllRebin: " << endl;
  string dopt[2] = {"","zcol"};
  string temp;
  int nn=0;
  if (opt.drawopt[0]==' ')
    nn=1;

  stringstream ss(opt.drawopt);
  while ((ss>>temp) && nn<2) {
    dopt[nn++] = temp;
  }

  //cout << "dopt: " << dopt[0] << " " << dopt[1] << " " << dopt[0].size() << " " << dopt[1].size() << endl;

  for (int npad = 0;npad<ndiv;npad++) {
    if (npad >= (int)pad_map.size() || !pad_map[npad] || !pad_hist[npad])
      continue;

    OneRebinPreCalibr(pad_map[npad], pad_hist[npad], b_adj);
    //TAxis* ya = pad_map[npad]->hst->GetYaxis();

    double a1,a2,y1=1e99,y2=-1e99;
    X_Slider(pad_hist[npad],a1,a2);
    Y_Slider(pad_hist[npad],a1,a2,y1,y2);

    fEc->GetCanvas()->cd(npad+1);
    if (pad_hist[npad]->GetDimension()==2) {
      gPad->SetLogz(opt.b_logy);
      pad_hist[npad]->Draw(dopt[1].c_str());
    }
    else {
      gPad->SetLogy(opt.b_logy);
      //cout << "Vslider: " << v1 << " " << v2 << " " << y1 << " " << y2 << " " << b1 << " " << b2 << endl;
      pad_hist[npad]->Draw(dopt[0].c_str());

      // cout << "padhist: " << pad_hist[npad]->GetName()
      // 	   << " " << pad_hist[npad]->GetOption()
      // 	   << " " << pad_hist[npad]->GetSumw2N()
      // 	   << endl;

      // TSpectrum ts;
      // TH1* bkg = ts.Background(pad_hist[npad],40);
      // bkg->SetLineColor(2);
      // bkg->Draw("samehist");
    }

    //draw cuts
    if (opt.b_gcuts && opt.ncuts) {
      DrawCuts(npad);
    }
    if (opt.b_roi) {
      DrawRoi(pad_map[npad]->hd,fEc->GetCanvas()->GetPad(npad+1));
    }

  } //for npad
}

void HistFrame::DrawCuts(int npad) {
  HMap* map = pad_map[npad];
  if (!map)
    return;

  TVirtualPad* pad = fEc->GetCanvas()->GetPad(npad+1);
  TLegend leg = TLegend(0.7,0.8,0.99,0.99);
  leg.SetMargin(0.5);

  bool found=false;
  for (int j=1;j<opt.ncuts;j++) {

    if (getbit(*(map->hd->cut+map->nn),j)) {
      if (hcl->cutG[j]) {
	found=true;
	if (opt.pcuts[j]>1) { //not formula
	  if (opt.pcuts[j]==2) {
	    cline.SetLineColor(hcl->cutG[j]->GetLineColor());
	    pad->Update();
	    Double_t xx;
	    Double_t y1 = pad->GetUymin();
	    Double_t y2 = pad->GetUymax();

	    if (pad->GetLogy()) {
	      y1=std::pow(10,y1);
	      y2=std::pow(10,y2);
	    }

	    xx = hcl->cutG[j]->GetX()[0];
	    cline.DrawLine(xx,y1,xx,y2);
	    xx = hcl->cutG[j]->GetX()[1];
	    cline.DrawLine(xx,y1,xx,y2);
	  }
	  else {
	    hcl->cutG[j]->Draw("same");
	  }
	  leg.AddEntry(hcl->cutG[j],hcl->cutG[j]->GetName(),"l");
	}
      }
    }
  }
  if (found) {
    leg.DrawClone("same");
  }
}

void HistFrame::DrawRoi(Hdef* hd, TVirtualPad* pad) {
  //HMap* map = pad_map[npad];
  if (!hd)
    return;
  //TVirtualPad* pad = fEc->GetCanvas()->GetPad(npad+1);

  //TLegend leg = TLegend(0.7,0.8,0.99,0.99);
  //leg.SetMargin(0.5);

  for (int j=0;j<MAXROI;j++) {
    if (empty_roi(hd,j))
      continue;
    //cline.SetLineColor(hcl->cutG[j]->GetLineColor());
    pad->Update();
    Double_t xx;
    Double_t y1 = pad->GetUymin();
    Double_t y2 = pad->GetUymax();

    Double_t y3 = y2*0.96;
    y2 = y1+(y2-y1)*0.93;

    if (pad->GetLogy()) {
      y1=std::pow(10,y1);
      y2=std::pow(10,y2);
      y3=std::pow(10,y3);
    }

    cline.SetLineColor(1);
    xx = hd->roi[j][0];
    cline.DrawLine(xx,y1,xx,y2);
    xx = hd->roi[j][1];
    cline.DrawLine(xx,y1,xx,y2);

    TString ss;
    ss += j+1;
    xx = (hd->roi[j][0] + hd->roi[j][1])/2;
    //cout << "ROI: " << j << " " << ss << " " << ss.Data() << endl;
    ttxt.DrawText(xx,y3,ss.Data());

    //leg.AddEntry(hcl->cutG[j],hcl->cutG[j]->GetName(),"l");
  }
  //if (found) {
  //leg.DrawClone("same");
  //}
}

/*
void HistFrame::DrawHist()
{

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  cv->Divide(xdiv,ydiv);
  //cv->Update();
  //cout <<"dr1b: " << fEc << " " << fEc->GetCanvas() << endl;
  //cout <<"dr2: " << hmap_chklist << endl;
  //return;

  for (int i=0;i<ndiv;i++) {
    cv->cd(i+1);
    //h_ampl[i]->Draw();
  }
  //return;
  //cout <<"dr3:" << endl;
  cv->Update();
}
*/
void HistFrame::ReDraw()
{

  // Update();
  // changed=false;
  // started=false;

  TCanvas *cv=fEc->GetCanvas();

  if (changed || started || opt.b_stack) {
    //cout << "changed: " << cv << endl;
    //fEc->GetCanvas()->Draw();
    //fEc->GetCanvas()->Update();
    //cout << "changed" << endl;
    Update();
    changed=false;
    started=false;
  }
  else {
    //cout << "unchanged: " << cv << endl;
    AllRebinDraw();
    // for (int i=0;i<ndiv;i++) {
    //   //cv->GetPad(i+1)->Modified(1);
    //   cv->cd(i+1);
    //   gPad->Modified(1);
    //   gPad->Draw();


    //   gPad->Update();
    // }
    cv->Update();
  }

}

// void HistFrame::DataDropped(TGListTreeItem *, TDNDData *data)
// {
//   cout << "YK dropped" << endl;
// }
