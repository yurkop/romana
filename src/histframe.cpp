//----- HistFrame ----------------
#include "romana.h"

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

TLine gline[2];
TLine cline;

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
//extern BufClass* Buffer;

extern Common* com;
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

//TGLayoutHints* fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
//TGLayoutHints* fLay2 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
//TGLayoutHints* fLay3 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,0);
//TGLayoutHints* fLay4 = new TGLayoutHints(kLHintsCenterX|kLHintsTop,0,0,0,0);
TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,5,5,0,0);
//TGLayoutHints* fLay51 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,0,0,0);
//TGLayoutHints* fLay52 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,0,5,0,0);
//TGLayoutHints* fLay53 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,0,0);
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,2);
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
//TGLayoutHints* fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
//TGLayoutHints* fLay7 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,3,0,0,0);
//TGLayoutHints* fLay8 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,0,0,0,0);
//TGLayoutHints* fLay8a = new TGLayoutHints(kLHintsExpandX,0,0,0,0);
TGLayoutHints* fLay9 = new TGLayoutHints(kLHintsCenterX|kLHintsBottom,0,0,0,0);

//------------------------------

/*
Bool_t MECanvas::HandleDNDDrop(TDNDData *data) {
  //TRootEmbeddedCanvas(data);
  cout << "DND" << endl;
  return TRootEmbeddedCanvas::HandleDNDDrop(data);
}
*/

// void printhlist(int n) {
//   if (!HiFrm) return;
//   if (!HiFrm->hlist) return;
//   cout << "printhlist+: " << n << endl;
//   TIter next(HiFrm->hlist);
//   TObject* obj;
//   while ( (obj=(TObject*)next()) ) {
//     HMap* map=(HMap*) obj;
//     cout << "hl map: " << map << endl;
//     cout << "hl name: " << map->GetName() << endl;
//   }
//   cout << "printhlist-: " << n << endl;
// }
//------------------------------

void DynamicExec()
{
  //static int nn;
  //nn++;
  //static Double_t xx[MAX_PCUTS],yy[MAX_PCUTS];
  //static TLine gline[2];
  static TPolyLine pl;
  static HMap* map=0; // hmap of histogram, on which cut is set
  //TObject* obj=0;
  static int hdim=0;

  int ev = gPad->GetEvent();
  int px = gPad->GetEventX();
  int py = gPad->GetEventY();
  
  Double_t xx = gPad->AbsPixeltoX(px);
  Double_t yy = gPad->AbsPixeltoY(py);

  Double_t y1 = gPad->GetUymin();
  Double_t y2 = gPad->GetUymax();
  //cout << "cutg: " << hdim << " " << HiFrm->np_gcut << " " << ev << " " << xx << " " << yy << endl;

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
      map=0;
      for (int i=1;i<=HiFrm->ndiv;i++) {
	if (gPad == HiFrm->fEc->GetCanvas()->GetPad(i)) {
	  np=i;
	  break;
	}
      }
      if (np) {
	map=(HMap*) HiFrm->padmap[np-1];
	//cout << "hist found: " << map << endl;
	//cout << "hist found: " << map->hst->GetName() << endl;
      }
      if (!map) {
	cout << "hist not found: " << endl;
	HiFrm->in_gcut=false;
	HiFrm->Update();
      }
      hdim=map->hst->GetDimension();
      //cout << "dimension: " << hdim << endl;
      pl.SetPolyLine(0);
      // if (hdim==1) {
      // 	gline[0]->SetLineColor(2);
      // 	gline[1]->SetLineColor(2);
      // }
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
      //cout << "np: " << np << endl;
      //gline[np] = TLine(xx,y1,xx,y2);
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
    //cout << "pl2: " << HiFrm->np_gcut << " " << pl.Size() << endl;
    HiFrm->AddCutG(&pl,map);
  }
  
}

//----- HistFrame ----------------
HistFrame::HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  gline[0].SetLineColor(2);
  gline[1].SetLineColor(2);

  hlist=new TList();
  //hstack=new THStack();
  st_list=new TList();
  st_list->SetOwner(true);
  st_plot = new TH1F("st_plot","",1000,0.,1000.);

  st_plot->SetBit(TH1::kNoStats);
  st_plot->SetBit(kCanDelete,0);
  st_plot->SetDirectory(0);

  //hstack->SetHistogram(hplot);
  ntab=nt;

  changed=false;
  started=true;

  memset(wrk_check,1,sizeof(wrk_check));
  wrk_check_MT=1;

  //Frames.....

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc
  TGHorizontalFrame      *fHor3; //for cuts

  TGVerticalFrame        *fVer0; //canvas && hslider

  //TGVerticalFrame *fVer[NR];
  //TGLabel *flab[NR];

  //TGShutterItem *item;
  //TGCompositeFrame *container;

  TGTextButton* but;

  fDock = new TGDockableFrame(this);
  AddFrame(fDock, com->LayEE0);
  fDock->SetWindowName("Histograms/Cuts");
  fDock->SetFixedSize(kFALSE);

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));

  fHor1 = new TGHorizontalFrame(fMain, 10, 10);
  fHor2 = new TGHorizontalFrame(fMain, 10, 10);

  fMain->AddFrame(fHor1, com->LayEE0);
  fMain->AddFrame(fHor2, com->LayET0);

  fVer0 = new TGVerticalFrame(fHor1, 10, 10);
  fHor1->AddFrame(fVer0, com->LayEE0);

  //fEc = new TRootEmbeddedCanvas("Hcanvas", fHor1, 10, 10);
  //fHor1->AddFrame(fEc, com->LayEE0);
  fEc = new TRootEmbeddedCanvas("Hcanvas", fVer0, 10, 10);
  fVer0->AddFrame(fEc, com->LayEE0);

  fHslider = new TGDoubleHSlider(fVer0, 10, kDoubleScaleBoth,0);
  fHslider->SetRange(0,1);
  fHslider->SetPosition(0,1);
  fVer0->AddFrame(fHslider, com->LayET0);
  fHslider->Connect("PositionChanged()", "HistFrame", 
   		    this, "DoSlider()");

  fVslider = new TGDoubleVSlider(fHor1, 10, kDoubleScaleBoth,1);
  fVslider->SetRange(0,1);
  fVslider->SetPosition(0,1);
  //fVslider->SetWidth(10);
  fHor1->AddFrame(fVslider, com->LayLE0);
  fVslider->Connect("PositionChanged()", "HistFrame", 
   		    this, "DoSlider()");


  TGTab* fTab = new TGTab(fHor1);
  fHor1->AddFrame(fTab, com->LayLE1);
  TGCompositeFrame* tab1 = fTab->AddTab(" Histograms ");
  TGCompositeFrame* tab2 = fTab->AddTab(" Cuts ");
  gCanvas = new TGCanvas(tab1, 150, 100);
  tab1->AddFrame(gCanvas, com->LayLE0);
  gCanvas2 = new TGCanvas(tab2, 150, 100);
  tab2->AddFrame(gCanvas2, com->LayLE0);

  fListTree = new TGListTree(gCanvas, kVerticalFrame);
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");
  fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "DoClick(TGListTreeItem*,Int_t)");

  fCutTree = new TGListTree(gCanvas2, kVerticalFrame);
  fCutTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "CutClick(TGListTreeItem*,Int_t)");
  
  fHor3 = new TGHorizontalFrame(tab2, 10, 10);
  tab2->AddFrame(fHor3, com->LayET0);


  but = new TGTextButton(fHor3,"Add",1);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor3->AddFrame(but, com->LayLC1);

  but = new TGTextButton(fHor3,"Cancel",2);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor3->AddFrame(but, com->LayLC1);

  but = new TGTextButton(fHor3,"Clear",4);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor3->AddFrame(but, com->LayLC1);

  const char* ttip = "Formula for the condition.\nUse standard C and root operators and functions\nFormula turns red in case of an error\nUse [0] [1] [2] ... for other cuts in the formula\nExamples:\n [0] && [1] - cut0 \"and\" cut1\n [2] || [3] - cut2 \"or\" cut3\n ![3] - not cut3";

  tab2->AddFrame(new TGLabel(tab2, "--Formula--"), com->LayET0);
  TGHorizontalFrame *fHor7 = new TGHorizontalFrame(tab2, 10, 10);
  tab2->AddFrame(fHor7, com->LayCB0);

  but = new TGTextButton(fHor7,"Form",7);
  but->Connect("Clicked()", "HistFrame", this, "AddFormula()");
  but->SetToolTipText(ttip);
  fHor7->AddFrame(but, com->LayLT0);
  
  tForm    = new TGTextEntry(fHor7,"",8);;
  tForm->SetWidth(110);
  tForm->SetToolTipText(ttip);
  tForm->Connect("ReturnPressed()", "HistFrame", this, "AddFormula()");
  fHor7->AddFrame(tForm,com->LayET0);

  const char* rlab[2] = {"Stack/N","X/Y:"};
  Rb[0] = new TGRadioButton(fHor2,rlab[0],1);
  fHor2->AddFrame(Rb[0], com->LayLC2);
  Rb[0]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  Rb[0]->SetToolTipText("Draw all histograms in WORK* folders as a stack");
  
  chknorm = new TGCheckButton(fHor2,"",11);
  chknorm->Connect("Clicked()", "HistFrame", this, "DoNorm()");
  fHor2->AddFrame(chknorm, com->LayLC2);
  chknorm->SetToolTipText("Normalize stacked histograms");

  Rb[1] = new TGRadioButton(fHor2,rlab[1],2);
  fHor2->AddFrame(Rb[1], com->LayLC1);
  Rb[1]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  Rb[1]->SetToolTipText("Draw histograms in separate subwindows");

  const char* ttip2[] = {"Horizontal divisions","Vertical divisions"};

  TGNumberEntry* fNum[2];
  int div[2] = {opt.xdiv,opt.ydiv};
  for (int i=0;i<2;i++) {
    fNum[i] = new TGNumberEntry(fHor2, div[i], 2, i+10,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEAAnyNumber,
				TGNumberFormat::kNELLimitMinMax,
				1,16);
    fNum[i]->GetNumberEntry()->Connect("TextChanged(char*)", "HistFrame", this, "DoNum()");
    fNum[i]->GetNumberEntry()->SetToolTipText(ttip2[i]);
  }

  fHor2->AddFrame(fNum[0], com->LayLC2);
  fHor2->AddFrame(fNum[1], com->LayLC3);

  but = new TGTextButton(fHor2," < ",1);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, com->LayLC1);
  but->SetToolTipText("One histogram backward");

  but = new TGTextButton(fHor2," > ",2);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, com->LayLC1);
  but->SetToolTipText("One histogram forward");

  but = new TGTextButton(fHor2," << ",3);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, com->LayLC1);
  but->SetToolTipText("N histograms backward");

  but = new TGTextButton(fHor2," >> ",4);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, com->LayLC1);
  but->SetToolTipText("N histograms forward");

  //TGCheckButton* chk;
  chklog = new TGCheckButton(fHor2,"Log",11);
  chklog->Connect("Clicked()", "HistFrame", this, "DoLog()");
  fHor2->AddFrame(chklog, com->LayLC1);
  chklog->SetToolTipText("Log scale");

  chkstat = new TGCheckButton(fHor2,"Stat",12);
  chkstat->Connect("Clicked()", "HistFrame", this, "DoStat()");
  fHor2->AddFrame(chkstat, com->LayLC1);
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
   fHor2->AddFrame(cuts, com->LayLC1);
  */

  /*
  fHor2->AddFrame(new TGLabel(fHor2, "Cuts:"), com->LayLC1);

  but = new TGTextButton(fHor2,"Add",1);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, com->LayLC1);

  but = new TGTextButton(fHor2,"Cancel",2);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, com->LayLC1);

  //but = new TGTextButton(fHor2,"Show",3);
  //but->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  //fHor2->AddFrame(but, com->LayLC1);

  but = new TGTextButton(fHor2,"Clear",4);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor2->AddFrame(but, com->LayLC1);
  */

  //TGCheckButton* chk;
  chkgcuts = new TGCheckButton(fHor2,"Cuts",5);
  chkgcuts->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  fHor2->AddFrame(chkgcuts, com->LayLC1);
  chkgcuts->SetToolTipText("Draw cuts");

  but = new TGTextButton(fHor2,"Peaks",4);
  but->Connect("Clicked()", "HistFrame", this, "DoPeaks()");
  fHor2->AddFrame(but, com->LayLC1);
  but->SetToolTipText("Find peaks in all visible windows (works only in X/Y mode");

  //DoReset();

  //cout << "histfr7: " << endl;
  //Update();
}

HistFrame::~HistFrame()
{
  //cout << "~HistFrame()" << endl;
}

/*
void NameTitle(char* name, char* title, int i, int cc, 
			  const char* nm, const char* axis) {
  if (cc) {
    sprintf(name,"%s_%02d_cut%d",nm,i,cc);
    sprintf(title,"%s_%02d_cut%d%s_cut%d",nm,i,cc,axis,cc);
  }
  else {
    sprintf(name,"%s_%02d",nm,i);
    sprintf(title,"%s_%02d%s",nm,i,axis);
  }
}
*/

TGListTreeItem* HistFrame::Item_Ltree(TGListTreeItem* parent, const char* string, void* userData, const TGPicture *open, const TGPicture *closed) {

  TGListTreeItem *item;
  if (userData) { //single item
    item=fListTree->AddItem(parent, string, userData, open, closed, true);
    //item->SetDNDSource(kTRUE);
    HMap* map = (HMap*) userData;
    fListTree->CheckItem(item,*map->chk);
  }
  else { //folder
    HMap* map = new HMap(string,NULL,NULL,NULL,NULL);
    hcl->dir_list->Add(map);
    item=fListTree->AddItem(parent, string, map, open, closed, true);
    //item->SetDNDTarget(kTRUE);
  }
  return item;

}

void HistFrame::Make_Ltree() {

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
  //TGListTreeItem *item;
  //int col[MAXCUTS];
  //memset(col,2,sizeof(col));

  //YKYKYK hcl->Make_cuts();

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
  // if (crs->b_maintrig) {
  //   sprintf(cutname,"WORK_MT");
  //   iWork_MT = Item_Ltree(iroot, cutname,0,0,0);
  // }
  
  next.Reset();
  while ( (obj=(TObject*)next()) ) {
    map = (HMap*) obj;
    TString title = TString(map->GetTitle());
    if (!fListTree->FindChildByName(0,title)) {
      idir = Item_Ltree(iroot, title,0,0,0);
    }
    if (map->hst->InheritsFrom(TH2::Class()))
      pic=pic_2d;
    else
      pic=pic_1d;

    Item_Ltree(idir, map->GetName(), map, pic, pic);

    if (*map->wrk) {
      Item_Ltree(iWork, map->GetName(), map, pic, pic);

      //make clones of _work_ histograms and their hmaps
      Clone_Ltree(map);
    }
  }

  fListTree->CheckAllChildren(iWork,wrk_check[0]);
  for (int cc=1;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      fListTree->CheckAllChildren(iWork_cut[cc],wrk_check[cc+1]);
    }
  }
  // if (crs->b_maintrig) {
  //   fListTree->CheckAllChildren(iWork_MT,wrk_check_MT);
  // }
  
  // TIter next2(hcl->dir_list);
  // TObject* obj2=0;
  // while ( (obj2=(TObject*)next2()) ) {
  //   cout << "Make_Ltree: " << obj2->GetName() << endl;
  // }

  //fill fCutTree
  for (int i=1;i<opt.ncuts;i++) {
    //cout << "Make_Ltree1: " << opt.ncuts << " " << i << " " << opt.pcuts[i] << endl;
    if (opt.pcuts[i]) {
      TCutG* gcut = hcl->cutG[i];
      //cout << "gcut: " << gcut << " " << gcut->GetName() << endl;

      if (opt.pcuts[i]==1) //formula
	pic=pic_f;
      else if (opt.pcuts[i]==2) //1d
	pic=pic_c1;
      else //2d
	pic=pic_c2;

      if (gcut->GetN() > 1) {
	sprintf(name,"[%s] %s",gcut->GetName(),gcut->GetTitle());
	item = fCutTree->AddItem(0, name, gcut, pic, pic, false);
	//fCutTree->AddItem(item, gcut->GetTitle(), pic, pic, false);
	for (int i=0;i<gcut->GetN();i++) {
	  sprintf(name,"%0.2f; %0.2f",gcut->GetX()[i],gcut->GetY()[i]);
	  fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
	}
      }
      else {
	sprintf(name,"[%s] %s",gcut->GetName(),"formula");
	item = fCutTree->AddItem(0, name, gcut, pic, pic, false);
	sprintf(name,"%s",opt.cut_form[i]);
	fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
      }
      //cout << "AddCutg: " << i+1 << " " << gcut->GetName() << " " << opt.pcuts[i] << endl;
    }
    else { //if opt.pcuts[i]
      
    }
  }

}
/*
void HistFrame::Delete_work_item(TGListTreeItem *item)
{
  HMap* map = (HMap*) item->GetUserData();

  for (int i=0;i<opt.ncuts;i++) {
    HMap* mcut = map->h_cuts[i];

  }
  
}
*/
void HistFrame::Clear_Ltree()
{
  //cout << "hlistclear: " << hlist << " " << hlist->GetSize() << endl;
  //printhlist(1);

  hlist->Clear();

  //cout << "clearltree: " << endl;
  TGListTreeItem *idir;
  //clear fListTree
  idir = fListTree->GetFirstItem();
  int ii=0;
  while (idir) {
    if (TString(idir->GetText()).Contains("work_mt",TString::kIgnoreCase)) {
      wrk_check_MT=idir->IsChecked();
    }
    else if (TString(idir->GetText()).Contains("work",TString::kIgnoreCase)) {
      wrk_check[ii]=idir->IsChecked();
      ii++;
    }
    fListTree->DeleteChildren(idir);
    fListTree->DeleteItem(idir);
    idir = idir->GetNextSibling();
  }

  //cout << "clearcut: " << endl;
  //clear fCutTree
  idir = fCutTree->GetFirstItem();
  while (idir) {
    fCutTree->DeleteChildren(idir);
    fCutTree->DeleteItem(idir);
    idir = idir->GetNextSibling();
  }
  
}

TGListTreeItem* HistFrame::FindItem(TGListTree* lTree, const char* name) {
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

void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  if (!crs->b_stop)
    return;

  char hname[100];
  char hname2[100]; 
  TGListTreeItem* item2;

   //clear all work* if middle button is clicked on WORK
  if (but==2 && TString(item->GetText()).EqualTo("work",TString::kIgnoreCase)) {
    //cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

    hlist->Clear();

    item2 = item->GetFirstChild();
    while (item2) {
      //cout << "dell: " << item2->GetText() << endl;
      HMap* map = (HMap*) item2->GetUserData();
      *map->wrk=false;
      hcl->Remove_Clones(map);
      item2 = item2->GetNextSibling();
    }
    
    fListTree->DeleteChildren(iWork);
    for (int cc=1;cc<opt.ncuts;cc++) {
      if (opt.pcuts[cc]) {
	fListTree->DeleteChildren(iWork_cut[cc]);
      }
    }
    // if (crs->b_maintrig) {
    //   fListTree->DeleteChildren(iWork_MT);
    // }

  }

  if (item->GetParent() && (but==2 || but==3)) {
    if (TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
      // если клик мышкой (2 или 3) на гистограмму в папке work*,
      // удаляем ее из всех work*

      hlist->Clear();
      //remove items
      //cout << "work or work_cut*: " << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname2,hh->GetName());
      char* str = strstr(hname2,"_cut");
      char* str2 = strstr(hname2,"_MT");
      if (str) {
	strncpy(hname,hname2,str-hname2);
	hname[str-hname2] = '\0';   // null character manually added
      }
      else if (str2) {
	strncpy(hname,hname2,str2-hname2);
	hname[str2-hname2] = '\0';   // null character manually added
      }
      else {
	strcpy(hname,hname2);
      }

      //cout << "work1: " << item << " " << hname << endl;
      item2 = fListTree->FindChildByName(iWork,hname);

      if (item2) {
	HMap* map = (HMap*) item2->GetUserData();
	*map->wrk=false;
	fListTree->DeleteItem(item2);
	hcl->Remove_Clones(map);
      }

      //cout << "work2: " << item << " " << hname << endl;
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
    else { //not work*
      // add items
      //cout << "nowork: " << item << endl;

      TGListTreeItem *item2 =
	fListTree->FindChildByData(iWork, item->GetUserData());
      if (!item2) {
	//cout << "Item2: " << item2 << endl;

	TObject* hh = (TObject*) item->GetUserData();
	strcpy(hname,hh->GetName());
	const TGPicture *pic = item->GetPicture();
	Item_Ltree(iWork, hh->GetName(), hh, pic, pic);

	HMap* map = (HMap*) item->GetUserData();
	*map->wrk=true;

	hcl->Clone_Hist(map);
	Clone_Ltree(map);
      }
    }
  } //if (item->GetParent() && (but==2 || but==3))
  
  //cout << "Upd1: " << endl;
  if (crs->b_stop)
    Update();
  else
    changed=true;
  //cout << "Upd2: " << endl;

}

void HistFrame::Clone_Ltree(HMap* map) {
  //char cutname[99];
  //char name[99],htitle[99];
  TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");
  TGPicture *pic=0;

  if (*map->wrk) {
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
	  *map->chk = item->IsChecked();
	}
      }
    }
    else { //folder
      if (!TString(item->GetText()).Contains("work",TString::kIgnoreCase)) { //not work* folder
	TGListTreeItem *item2 = item->GetFirstChild();
	while (item2) {
	  map = (HMap*) item2->GetUserData();
	  if (map) {
	    *map->chk = item->IsChecked();
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

void HistFrame::DoNorm()
{
  TGCheckButton *te = (TGCheckButton*) gTQSender;

  opt.b_norm = (Bool_t)te->GetState();
  
  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::DoLog()
{
  TGCheckButton *te = (TGCheckButton*) gTQSender;

  opt.b_logy = (Bool_t)te->GetState();
  
  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::DoStat()
{
  TGCheckButton *te = (TGCheckButton*) gTQSender;

  opt.b_stat = (Bool_t)te->GetState();
  
  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::ShowCutG()
{
  TGCheckButton *te = (TGCheckButton*) gTQSender;

  opt.b_gcuts = (Bool_t)te->GetState();
  
  if (crs->b_stop)
    Update();
  else
    changed=true;
}

void HistFrame::DoKey(TGListTreeItem* entry, UInt_t keysym) {
  cout << "key: " << entry << " " << keysym << endl;
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

void HistFrame::DoNum()
{
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  if (id==10) {
    opt.xdiv=te->GetNumber();
  }
  else {
    opt.ydiv=te->GetNumber();
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

void HistFrame::AddCutG(TPolyLine *pl, TObject* hobj) {

  if (opt.b_stack)
    return;

  // check if the histogram exists
  if (!hobj) {
    cout << "AddCutg: histogram not found: " << endl;
    return;
  }

  in_gcut=false;
  TCanvas* cv=fEc->GetCanvas();
  //cv->SetCrosshair(false);
  for (int i=1;i<=ndiv;i++) {
    TPad* pad = (TPad*) cv->GetPad(i);
    if (pad) {
      pad->GetListOfExecs()->Clear();
    }
  }

  MakeCutG(pl,hobj);
  Update();
  //ShowCutG(gcut);

  // HMap* map = (HMap*) hobj;
  // for (int i=0;i<MAXCUTS;i++) {
  //   cout << "index: " << i << " " << (int) map->cut_index[i]
  // 	 << " " << (int) opt.cut_h2d[0*MAXCUTS+i] << endl;
  // }

}

void HistFrame::MakeCutG(TPolyLine *pl, TObject* hobj) {
  HMap *map;

  int icut=-1;
  for (int i=1;i<opt.ncuts;i++) {
    if (opt.pcuts[i]==0) {
      icut=i;
      break;
    }
  }
  if (icut<0) {
    if (opt.ncuts) {
      icut=opt.ncuts;
      opt.ncuts++;
    }
    else {
      icut=1;
      opt.ncuts=2;
    }
  }
  if (icut>=MAXCUTS) {
    cout << "Too many cuts: " << icut << endl;
    return;
  }

  cout << "ncuts: " << icut << " " << opt.ncuts << endl;

  TGListTreeItem *idir = fListTree->GetFirstItem();
  if (strcmp(idir->GetText(),"WORK")) {
    cout << "first item is not WORK" << endl;
    return;
  }

  map = (HMap*) hobj;

  //1. add current cut to the cut_index of hmap
  //(map->cut_index) ?? check it 
  //cout << "make1: " << endl;
  setbit(*(map->cut_index),icut,1);

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
      setbit(*(map->cut_index),icut,0);
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

void HistFrame::DoCutG() {

  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId();
  Int_t retval;

  if (id==1) {
    if (opt.ncuts>=MAXCUTS-1) {
      const char* msg = "Too manu cuts. Delete them first";
      new TGMsgBox(gClient->GetRoot(), this,
	       "Graphical cut",
	       msg, kMBIconAsterisk, kMBDismiss, &retval);
      return;
    }
    in_gcut=true;
    np_gcut=0;
  }
  else {
    in_gcut=false;
  }

  Update();
  //return;
  
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

    int icut=-1;
    for (int i=1;i<opt.ncuts;i++) {
      if (opt.pcuts[i]==0) {
	icut=i;
	break;
      }
    }
    if (icut<0) {
      if (opt.ncuts) {
	icut=opt.ncuts;
	opt.ncuts++;
      }
      else {
	icut=1;
	opt.ncuts=2;
      }
    }
    if (icut>=MAXCUTS) {
      cout << "Too many cuts: " << icut << endl;
      return;
    }

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
    *(map->cut_index)=0;
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

void HistFrame::DoPeaks()
{

  TSpectrum spec;
  
  int nn=1;
  int ii=0;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (ii>=opt.icheck) {
      if (!fEc->GetCanvas()->GetPad(nn)) break;
      fEc->GetCanvas()->cd(nn);
      TH1 *hh = ((HMap*) obj)->hst;
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
    ii++;
  }
  //fEc->GetCanvas()->SetEditable(true);
  fEc->GetCanvas()->Update();
  //fEc->GetCanvas()->SetEditable(false);

}

void HistFrame::HiReset()
{

  //cout << "HiReset1: " << endl;
  //printhlist(2);
  //return;
  
  if (!crs->b_stop) return;

  in_gcut=false;

  TCanvas *cv=fEc->GetCanvas();
  //cv->SetEditable(true);
  cv->Clear();
  //cv->SetEditable(false);

  //cout << "Make_hist():: " << endl;
  Clear_Ltree();
  //cout << "Make_hist2():: " << endl;
  hcl->Make_hist();
  //cout << "Make_hist3():: " << endl;
  Make_Ltree();
  //cout << "Make_hist()::2 " << endl;

  Update();

  cv->Draw();
  cv->Update();

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
  chklog->SetState((EButtonState) opt.b_logy);
  chkstat->SetState((EButtonState) opt.b_stat);
  chkgcuts->SetState((EButtonState) opt.b_gcuts);

  //cout << "HiFrm::Update()2a" << endl;
  //hlist->ls();
  hlist->Clear();
  //cout << "HiFrm::Update()2b" << endl;

  //Hmut.UnLock();
  //return;

  if (opt.b_stack) {
    DrawStack();
  }
  else {
    DrawHist();
  }
  
  //hlist->Print();

  Hmut.UnLock();
  //cout << "hist_list: " << endl;
  //hcl->hist_list->ls();
  //cout << "hist_list: " << hcl->hist_list << " " << hcl->hist_list->GetSize() << endl;


  // TIter next(hlist);
  // TObject* obj;
  // while ( (obj=(TObject*)next()) ) {
  //   HMap* map=(HMap*) obj;
  //   cout << "hlist_update: " << map << " " << map->GetName() << endl;
  //   //cout << "hlist2: " << map->GetName() << endl;
  // }

  //cout << "HiFrm::Update()3" << endl;

}

void HistFrame::DrawStack() {

  double x1=1e99,x2=-1e99,y1=1e99,y2=-1e99;
  //create hstack
  //delete hstack;
  //hstack=new THStack();

  //hstack->GetHists()->Clear();
  st_list->Clear();
  int cc=0;
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (TString(idir->GetText()).Contains("WORK")) {
    //cout << "Work: " << idir->GetText() << endl;
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	if (item->IsChecked()) {
	  HMap* map=(HMap*) item->GetUserData();
	  char name[100];
	  TH1 *hh = (TH1*) map->hst->Clone();
	  sprintf(name,"%s ",hh->GetName());
	  hh->SetName(name);
	  //cout << "cols: " << hcols[cc%nhcols] << endl;
	  if (hh->GetDimension()==1) {
	    hh->SetLineColor(hcols[cc%nhcols]);
	    if (opt.b_norm) {
	      double sc = hh->GetNbinsX()/hh->Integral();
	      hh->Scale(sc);
	    }
	    st_list->Add(hh);
	    //hstack->Add(hh);
	    x1 = TMath::Min(x1,hh->GetXaxis()->GetXmin());
	    x2 = TMath::Max(x2,hh->GetXaxis()->GetXmax());
	    //cout << "x1: " << hh->GetXaxis()->GetXmin() << hh->GetName() << endl;
	    cc++;
	  }
	  //hlist->Add((TObject*)item->GetUserData());
	}
      }
      item = item->GetNextSibling();
    }
    idir = idir->GetNextSibling();
  }

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  cv->Divide(1,1);
  cv->SetLogy(opt.b_logy);

  //cout << "x12: " << x1 << " " << x2 << endl;

  //hstack->Draw("nostack");

  //set x-axis

  st_plot->SetBins(1000,x1,x2);
  double a1,a2;
  X_Slider(st_plot,a1,a2);

  //cout << "x12: " << x1 << " " << x2 << " " << a1 << " " << a2 << endl;
  //determine y-axis ranges

  TIter next(st_list);
  TObject* obj=0;
  while ( (obj=(TObject*)next()) ) {
    TH1* hh = (TH1*) obj;
    GetHMinMax(hh,a1,a2,y1,y2);
  }

  Y_Slider(st_plot,a1,a2,y1,y2);

  //cout << "y12: " << y1 << " " << y2 << " " << a1 << " " << a2 << endl;

  st_plot->Draw("axis");
  next.Reset();
  obj=0;
  while ( (obj=(TObject*)next()) ) {
    TH1* hh = (TH1*) obj;
    hh->Draw("samehist");
  }

  //cout << "Vslider: " << b1 << " " << b2 << " " << y1 << " " << y2 << endl;
  //hstack->Draw("nostack hist");
  /*
  if (opt.b_norm) {
    TIter next(hstack->GetHists());
    TObject* obj;
    while ( (obj=(TObject*)next()) ) {
      TH1* hh=(TH1*) obj;
      double sc = hh->GetNbinsX()/hh->Integral();
      hh->Scale(sc);
    }
    //cout << "DrawStack: " << endl;
    //hstack->Draw("nostack");  
  }
  */
  if (opt.b_stat) {
    TLegend* leg = new TLegend(0.65,0.75,0.95,0.95);
    next.Reset();
    obj=0;
    while ( (obj=(TObject*)next()) ) {
      TH1* hh = (TH1*) obj;
      //if (strlen(hh->GetTitle()))      mes = hist->GetTitle();
      //else if (strlen(hist->GetName()))  mes = hist->GetName();
      //else                               mes = hist->ClassName();
      leg->AddEntry(hh, hh->GetName(), "lpf" );
      //cout << "GetName: " << hh->GetName() << " " << hh->GetTitle() << endl;
      leg->Draw();
    }
    //cv->BuildLegend(0.30,0.75,0.60,0.95);
  }
  cv->Update();

}

void HistFrame::DrawHist() {

  //cout << "DrawHist1:" << endl;
  //create hlist
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	//YK - don't know why this is needed

	// HMap* map = (HMap*) item->GetUserData();
	// if (!TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
	//   *map->chk = item->IsChecked();
	// }
	if (item->IsChecked()) {
	  hlist->Add((TObject*)item->GetUserData());
	  //cout << "hlist_add: " << item->GetUserData() << " " << item->GetText() << endl;
	}
      }
      item = item->GetNextSibling();
    }
    idir = idir->GetNextSibling();
  }

  if (opt.icheck > hlist->GetSize()-ndiv)
    opt.icheck=hlist->GetSize()-ndiv;
  if (opt.icheck < 0)
    opt.icheck=0;

  //draw hists
  HMap* map;

  //cout << "Drawstat: " << gStyle->GetOptStat() << endl;

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  memset(padmap,0,sizeof(padmap));
  double mg=0.001;
  if (opt.xdiv+opt.ydiv>16)
    mg=0;
  if (opt.b_stack)
    cv->Divide(1,1);
  else {
    cv->Divide(opt.xdiv,opt.ydiv,mg,mg);
  }

  //cout << "DrawHist2: " << hlist->GetSize() << endl;
  int nn=1;
  int ii=0;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    //cout << "DrawHist22: " << obj << endl;
    if (ii>=opt.icheck) {
      cv->cd(nn);
      map=(HMap*) obj;
      TH1 *hh = map->hst;
      hh->UseCurrentStyle();
      double a1,a2,y1=1e99,y2=-1e99;
      X_Slider(hh,a1,a2);
      Y_Slider(hh,a1,a2,y1,y2);
      if (hh->GetDimension()==2) {
	gPad->SetLogz(opt.b_logy);
	hh->Draw("zcol");
      }
      else {
	gPad->SetLogy(opt.b_logy);
	//cout << "Vslider: " << v1 << " " << v2 << " " << y1 << " " << y2 << " " << b1 << " " << b2 << endl;
	hh->Draw();
      }
      padmap[nn-1]=obj;

      //draw cuts
      if (opt.b_gcuts && opt.ncuts) {
	TLegend leg = TLegend(0.7,0.8,0.99,0.99);
	leg.SetMargin(0.5);

	bool found=false;
	for (int j=1;j<opt.ncuts;j++) {

	  if (getbit(*(map->cut_index),j)) {
	    if (hcl->cutG[j]) {
	      found=true;
	      if (opt.pcuts[j]>1) { //not formula
		if (opt.pcuts[j]==2) {
		  cline.SetLineColor(hcl->cutG[j]->GetLineColor());
		  gPad->Update();
		  Double_t xx;
		  Double_t y1 = gPad->GetUymin();
		  Double_t y2 = gPad->GetUymax();

		  if (gPad->GetLogy()) {
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

      nn++;
      if (nn>ndiv)
	break;
    }
    ii++;
  }
  cv->Update();

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
  }
  
}

/*
void HistFrame::DrawHist()
{

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  cv->Divide(xdiv,ydiv);
  //cv->Update();
  //cout <<"dr1b: " << fEc << " " << fEc->GetCanvas() << endl;
  //cout <<"dr2: " << hlist << endl;
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

  //cout << "ReDraw: " << fEc->GetCanvas() << endl;

  TCanvas *cv=fEc->GetCanvas();

  if (changed || started || opt.b_stack) {
    //fEc->GetCanvas()->Draw();
    //fEc->GetCanvas()->Update();
    //cout << "changed" << endl;
    Update();
    changed=false;
    started=false;
  }
  else {
    //cout << "unchanged1" << endl;
    //cout << "unchanged2: " << fEc->GetCanvas() << endl;
    //DrawHist();
    //fEc->GetCanvas()->GetListOfPrimitives()->ls();


    //fEc->GetCanvas()->Draw();

    for (int i=0;i<ndiv;i++) {
      cv->cd(i+1);
      //gPad->Draw();
      //cout << "Modified1: " << i << endl;
      gPad->Modified(1);
      //gPad->Update();
    }

    //cout << "unchanged3" << endl;
    fEc->GetCanvas()->Update();
    //cout << "unchanged4" << endl;
  }

}

// void HistFrame::DataDropped(TGListTreeItem *, TDNDData *data)
// {
//   cout << "YK dropped" << endl;
// }
