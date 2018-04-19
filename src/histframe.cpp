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

TGLayoutHints* fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
TGLayoutHints* fLay2 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
TGLayoutHints* fLay3 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,0);
TGLayoutHints* fLay4 = new TGLayoutHints(kLHintsCenterX|kLHintsTop,0,0,0,0);
TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,5,5,0,0);
TGLayoutHints* fLay51 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,0,0,0);
TGLayoutHints* fLay52 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,0,5,0,0);
TGLayoutHints* fLay53 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,0,0);
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,2);
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
TGLayoutHints* fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
TGLayoutHints* fLay7 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,3,0,0,0);
TGLayoutHints* fLay8 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,0,0,0,0);
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

HistFrame::HistFrame(const TGWindow *p,UInt_t w,UInt_t h, Int_t nt)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  gline[0].SetLineColor(2);
  gline[1].SetLineColor(2);

  //const char* tRad[NR]={"1x1","2x2","3x2","4x2","4x4","8x4","8x8"};

  //opt.xdiv=2;
  //opt.ydiv=2;
  // if (opt.b_stack)
  //   ndiv=1;
  // else
  //   ndiv=opt.xdiv*opt.ydiv;

  hlist=new TList();
  hstack=new THStack();
  //hlist2.clear();

  //Hmut = new TMutex();
  
  //cout << "HistFrame: " << gDNDManager << endl;

  ntab=nt;

  changed=false;
  started=true;

  memset(wrk_check,1,sizeof(wrk_check));
  //char ss[100];


  //Frames.....

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc

  //TGVerticalFrame *fVer[NR];
  //TGLabel *flab[NR];

  TGShutterItem *item;
  TGCompositeFrame *container;

  TGTextButton* but;

  fHor1 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor1, fLay1);

  fHor2 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor2, fLay2);

  fEc = new TRootEmbeddedCanvas("Hcanvas", fHor1, 10, 10);
  fHor1->AddFrame(fEc, fLay1);

  //TGVerticalFrame *fV1 = new TGVerticalFrame(fHor1, 10,10);
  //fHor1->AddFrame(fV1, fLay7);

  TGShutter* fShutter = new TGShutter(fHor1);
  fShutter->SetDefaultSize(150,10);
  fHor1->AddFrame(fShutter, fLay7);

  item = new TGShutterItem(fShutter, new TGHotString("Histograms"), 1);
  container = (TGCompositeFrame *) item->GetContainer();
  fShutter->AddItem(item);
  gCanvas = new TGCanvas(container, 150, 100);
  container->AddFrame(gCanvas, fLay8);

  item = new TGShutterItem(fShutter, new TGHotString("Cuts"), 2);
  container = (TGCompositeFrame *) item->GetContainer();
  fShutter->AddItem(item);
  gCanvas2 = new TGCanvas(container, 150, 100);
  container->AddFrame(gCanvas2, fLay8);

  //TGVerticalFrame *fV2 = new TGVerticalFrame(fHor1, 10, 10);
  //fHor1->AddFrame(fV2, fLay7);
  //clab = new TGLabel(fV1, "--Cuts--");
  //fV1->AddFrame(clab, fLay4);

  container->AddFrame(new TGLabel(container, "--Formula--"), fLay2);
  TGHorizontalFrame *fHor7 = new TGHorizontalFrame(container, 10, 10);
  container->AddFrame(fHor7, fLay9);

  but = new TGTextButton(fHor7,"Add",7);
  but->Connect("Clicked()", "HistFrame", this, "AddFormula()");
  fHor7->AddFrame(but, fLay6);
  
  const char* ttip = "Formula for the condition.\nUse standard C and root operators and functions\nFormula turns red in case of an error\nUse [1] [2] [3] ... for other cuts in the formula\nExamples:\n [1] && [2] - cut1 \"and\" cut2\n [2] || [3] - cut2 \"or\" cut3\n ![3] - not cut3";

  tForm    = new TGTextEntry(fHor7,"",8);;
  tForm->SetWidth(120);
  tForm->SetToolTipText(ttip);
  tForm->Connect("ReturnPressed()", "HistFrame", this, "AddFormula()");
  fHor7->AddFrame(tForm,fLay2);

  //container->AddFrame(new TGLabel(container, "--Histograms--"), fLay9);
  //gCanvas->AddFrame(new TGLabel(gCanvas, "--Histograms--"), fLay5);
  //TGCanvas *gCanvas2 = new TGCanvas(fV1, 150, 150);
  //fV1->AddFrame(gCanvas2, fLay8);


  //fListTree=NULL;
  //if (fListTree) delete fListTree;

  fListTree = new TGListTree(gCanvas, kVerticalFrame);
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");
  fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "DoClick(TGListTreeItem*,Int_t)");

  //fListTree->AddRoot("Histograms");

  //new TGLabel(gCanvas2, "--Cuts--");
  //gCanvas2->AddFrame(new TGLabel(gCanvas2, "--Cuts--"));

  fCutTree = new TGListTree(gCanvas2, kVerticalFrame);
  fCutTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "CutClick(TGListTreeItem*,Int_t)");
  


  //cout << "make_ltree1: " << endl;
  //Make_Ltree();
  //cout << "make_ltree2: " << endl;

  const char* rlab[2] = {"Stack","DX/DY:"};
  for (int i=0;i<2;i++) {
    Rb[i] = new TGRadioButton(fHor2,rlab[i],i+1);
    fHor2->AddFrame(Rb[i], fLay5);
    Rb[i]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  }

  //TGLabel* fLabel1 = new TGLabel(fHor2, "DX/DY:");
  //TGLabel* fLabel2 = new TGLabel(fHor2, "DivY:");

  TGNumberEntry* fNum[2];
  int div[2] = {opt.xdiv,opt.ydiv};
  for (int i=0;i<2;i++) {
    fNum[i] = new TGNumberEntry(fHor2, div[i], 2, i+10,
				TGNumberFormat::kNESInteger,
				TGNumberFormat::kNEAAnyNumber,
				TGNumberFormat::kNELLimitMinMax,
				1,8);
    fNum[i]->GetNumberEntry()->Connect("TextChanged(char*)", "HistFrame", this, "DoNum()");
  }

  fHor2->AddFrame(fNum[0], fLay51);
  fHor2->AddFrame(fNum[1], fLay52);

  //cout << "histframe: " << opt.xdiv << endl;

  // for (int i=0;i<NR;i++) {
  //   fVer[i] = new TGVerticalFrame(fHor2, 10, 10);
  //   flab[i] = new TGLabel(fVer[i], tRad[i]);
  //   Rb[i]= new TGRadioButton(fVer[i],"",i+1);
  //   Rb[i]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
  //   fHor2->AddFrame(fVer[i], fLay8);
  //   fVer[i]->AddFrame(Rb[i], fLay4);
  //   fVer[i]->AddFrame(flab[i], fLay4);
  // }


  but = new TGTextButton(fHor2," < ",1);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2," > ",2);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2," << ",3);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2," >> ",4);
  but->Connect("Clicked()", "HistFrame", this, "DoButton()");
  fHor2->AddFrame(but, fLay5);

  //TGCheckButton* chk;
  chklog = new TGCheckButton(fHor2,"Log",11);
  chklog->Connect("Clicked()", "HistFrame", this, "DoLog()");
  fHor2->AddFrame(chklog, fLay5);

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
   fHor2->AddFrame(cuts, fLay5);
  */

  but = new TGTextButton(fHor2,"Peaks",4);
  but->Connect("Clicked()", "HistFrame", this, "DoPeaks()");
  fHor2->AddFrame(but, fLay5);

  fHor2->AddFrame(new TGLabel(fHor2, "Cuts:"), fLay5);

  but = new TGTextButton(fHor2,"Add",1);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2,"Cancel",2);
  but->Connect("Clicked()", "HistFrame", this, "DoCutG()");
  fHor2->AddFrame(but, fLay5);

  //but = new TGTextButton(fHor2,"Show",3);
  //but->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  //fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2,"Clear",4);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor2->AddFrame(but, fLay5);

  //TGCheckButton* chk;
  chkgcuts = new TGCheckButton(fHor2,"Show",5);
  chkgcuts->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  fHor2->AddFrame(chkgcuts, fLay5);

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
  for (int cc=0;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      sprintf(cutname,"WORK_cut%d",cc+1);
      iWork_cut[cc] = Item_Ltree(iroot, cutname,0,0,0);
    }
  }
  
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
  for (int cc=0;cc<opt.ncuts;cc++) {
    if (opt.pcuts[cc]) {
      fListTree->CheckAllChildren(iWork_cut[cc],wrk_check[cc+1]);
    }
  }
  
  // TIter next2(hcl->dir_list);
  // TObject* obj2=0;
  // while ( (obj2=(TObject*)next2()) ) {
  //   cout << "Make_Ltree: " << obj2->GetName() << endl;
  // }

  //fill fCutTree
  for (int i=0;i<opt.ncuts;i++) {
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
    if (TString(idir->GetText()).Contains("work",TString::kIgnoreCase)) {
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

  //cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

  if (item->GetParent() && (but==2 || but==3)) {
    if (TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
      //remove items
      //cout << "work or work_cut*: " << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname2,hh->GetName());
      char* str = strstr(hname2,"_cut");
      if (str) {
	strncpy(hname,hname2,str-hname2);
	hname[str-hname2] = '\0';   // null character manually added
      }
      else {
	strcpy(hname,hname2);
      }

      //cout << "work1: " << item << endl;
      item2 = fListTree->FindChildByName(iWork,hname);

      if (item2) {
	HMap* map = (HMap*) item2->GetUserData();
	*map->wrk=false;
	fListTree->DeleteItem(item2);
	hcl->Remove_Clones(map);
      }

      //cout << "work2: " << item << endl;
      for (int cc=0;cc<opt.ncuts;cc++) {
	sprintf(hname2,"%s_cut%d",hname,cc+1);
	item2 = fListTree->FindChildByName(iWork_cut[cc],hname2);
	//cout << "cc: " << cc << " " << hname2 << " " << item2 << endl;
	if (item2)
	  fListTree->DeleteItem(item2);
      }
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
  }
  
  if (crs->b_stop)
    Update();
  else
    changed=true;

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

    for (int i=0;i<opt.ncuts;i++) {
      if (opt.pcuts[i]) {
	HMap* mcut = map->h_cuts[i];
	if (mcut)
	  Item_Ltree(iWork_cut[i], mcut->GetName(), mcut, pic, pic);
      }
    }
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
      if (!TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) { //not in work* folder
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
  //   cout << "map2: " << map2->GetName() << " " << *map2->chk << " " << opt.s_mtof[0] << endl;
  // }

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
  for (int i=0;i<opt.ncuts;i++) {
    if (opt.pcuts[i]==0) {
      icut=i;
      break;
    }
  }
  if (icut<0) {
    icut=opt.ncuts;
    opt.ncuts++;
  }
  if (icut>=MAXCUTS) {
    cout << "Too many cuts: " << icut << endl;
    return;
  }

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

    if (icut<1 && icut >MAXCUTS) {
      cout << "wrong cut number: " << icut << ". Consider clearing all cuts" << endl;
      return;
    }
    icut--;

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
  return;
  
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
    for (int i=0;i<opt.ncuts;i++) {
      if (opt.pcuts[i]==0) {
	icut=i;
	break;
      }
    }
    if (icut<0) {
      icut=opt.ncuts;
      opt.ncuts++;
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
  for (int i=0;i<opt.ncuts;i++) {
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

  //cout << "HiFrm::Update()" << endl;

  Hmut.Lock();

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
  chkgcuts->SetState((EButtonState) opt.b_gcuts);

  hlist->Clear();

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
  //cout << "HiFrm::Update()2" << endl;


  // TIter next(hlist);
  // TObject* obj;
  // while ( (obj=(TObject*)next()) ) {
  //   HMap* map=(HMap*) obj;
  //   cout << "hlist_update: " << map << " " << map->GetName() << endl;
  //   //cout << "hlist2: " << map->GetName() << endl;
  // }


}

void HistFrame::DrawStack() {

  //create hstack
  delete hstack;
  hstack=new THStack();
  //hstack->GetHists()->Clear();
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
	  hh->SetLineColor(hcols[cc%nhcols]);
	  //cout << "cols: " << hcols[cc%nhcols] << endl;
	  if (hh->GetDimension()==1) {
	    hstack->Add(hh);
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
  hstack->Draw("nostack");
  cv->BuildLegend(0.65,0.75,0.95,0.95);
  cv->Update();

}

void HistFrame::DrawHist() {

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

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  memset(padmap,0,sizeof(padmap));
  if (opt.b_stack)
    cv->Divide(1,1);
  else
    cv->Divide(opt.xdiv,opt.ydiv);

  int nn=1;
  int ii=0;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (ii>=opt.icheck) {
      cv->cd(nn);
      map=(HMap*) obj;
      TH1 *hh = map->hst;
      if (hh->GetDimension()==2) {
	gPad->SetLogz(opt.b_logy);
	hh->Draw("zcol");
      }
      else {
	gPad->SetLogy(opt.b_logy);
	hh->Draw();
      }
      padmap[nn-1]=obj;

      //draw cuts
      if (opt.b_gcuts && opt.ncuts) {
	TLegend leg = TLegend(0.7,0.8,0.99,0.99);
	leg.SetMargin(0.5);

	//int icut=0;
	bool found=false;
	// cout << "cut_index: " << hh->GetName() << ": ";
	// for (int j=0;j<MAXCUTS;j++) {
	//   cout << (int) map->cut_index[j] << " ";
	// }
	// cout << endl;
	for (int j=0;j<opt.ncuts;j++) {

	  //icut=map->cut_index[j];
	  //if (icut==0 || icut>=MAXCUTS)
	  //break;

	  if (getbit(*(map->cut_index),j)) {
	    if (hcl->cutG[j]) {
	      found=true;
	      //cout << "pcuts: " << icut << " " << opt.pcuts[icut-1] << " " << opt.ncuts << endl;
	      if (opt.pcuts[j]>1) { //not formula
		if (opt.pcuts[j]==2) {
		  cline.SetLineColor(hcl->cutG[j]->GetLineColor());
		  gPad->Update();
		  Double_t xx;
		  Double_t y1 = gPad->GetUymin();
		  Double_t y2 = gPad->GetUymax();

		  xx = hcl->cutG[j]->GetX()[0];
		  cline.DrawLine(xx,y1,xx,y2);
		  xx = hcl->cutG[j]->GetX()[1];
		  cline.DrawLine(xx,y1,xx,y2);
		  //cout << "cline: " << xx << " " << y2 << " " << hcl->cutG[icut-1]->GetLineColor() << endl;
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
