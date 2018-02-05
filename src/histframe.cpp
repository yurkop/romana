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

//TLine gline[2];

/*
int Ch_Gamma_X[8]={ 1, 2, 3, 4, 5, 6, 7, 8};
int Ch_Gamma_Y[8]={16,15,14,13,12,11,10, 9};
int Ch_Alpha_X[8]={25,29,26,30,27,31,28,32};
int Ch_Alpha_Y[8]={24,20,23,19,22,18,21,17};
*/

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
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,2);
//TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft,20,2,2,2);
//TGLayoutHints* fLay6 = new TGLayoutHints(kLHintsLeft,0,0,0,0);
TGLayoutHints* fLay7 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,3,0,0,0);
TGLayoutHints* fLay8 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,0,0,0,0);

//------------------------------

/*
Bool_t MECanvas::HandleDNDDrop(TDNDData *data) {
  //TRootEmbeddedCanvas(data);
  cout << "DND" << endl;
  return TRootEmbeddedCanvas::HandleDNDDrop(data);
}
*/

//------------------------------

void DynamicExec()
{
  //static int nn;
  //nn++;
  //static Double_t xx[MAX_PCUTS],yy[MAX_PCUTS];
  static TLine gline[2];
  static TPolyLine pl;
  static HMap* map=0; // hmap of histogram, on which cut is set
  //TObject* obj=0;
  static int hdim=0;

  int ev = gPad->GetEvent();
  int px = gPad->GetEventX();
  int py = gPad->GetEventY();
  
  Double_t x1 = gPad->AbsPixeltoX(px);
  Double_t y1 = gPad->AbsPixeltoY(py);

  //cout << "cutg: " << hdim << " " << HiFrm->np_gcut << " " << ev << " " << x1 << " " << y1 << endl;

  if (ev==51 && hdim==2 && HiFrm->np_gcut) { //mouse movement
    gline[0].SetX2(x1);
    gline[0].SetY2(y1);
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
	cout << "hist found: " << map << endl;
	cout << "hist found: " << map->hst->GetName() << endl;
      }
      if (!map) {
	cout << "hist not found: " << endl;
	HiFrm->in_gcut=false;
	HiFrm->Update();
      }
      hdim=map->hst->GetDimension();
      cout << "dimension: " << hdim << endl;
      pl.SetPolyLine(0);
    } //first click

    if (hdim==2) {
      HiFrm->np_gcut=pl.SetNextPoint(x1,y1)+1;
      if (HiFrm->np_gcut>1) {
	pl.Draw();
      }
      if (HiFrm->np_gcut>=MAX_PCUTS-1) {
	HiFrm->np_gcut=pl.SetNextPoint(pl.GetX()[0],pl.GetY()[0])+1;
	HiFrm->AddCutG(&pl,map);
	return;
      }

      gline[0].SetX1(x1);
      gline[0].SetY1(y1);
    } //if hdim==2
    else if (hdim==1) {
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

  const char* tRad[NR]={"1x1","2x2","3x2","4x2","4x4","8x4","8x8"};

  xdiv=1;
  ydiv=1;
  ndiv=1;

  hlist=new TList();
  //hlist2.clear();

  //Hmut = new TMutex();
  
  //cout << "HistFrame: " << gDNDManager << endl;

  ntab=nt;

  changed=false;
  started=true;
  //char ss[100];


  //Frames.....

  TGHorizontalFrame      *fHor1; //contains canvas and list of histograms
  TGHorizontalFrame      *fHor2; //contains buttons etc

  TGVerticalFrame *fVer[NR];
  TGLabel *flab[NR];

  TGShutterItem *item;
  TGCompositeFrame *container;

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

  //fCanvas->AddFrame(new TGLabel(fCanvas, "--Histograms--"), fLay5);
  //TGCanvas *gCanvas2 = new TGCanvas(fV1, 150, 150);
  //fV1->AddFrame(gCanvas2, fLay8);

  //new TGLabel(gCanvas2, "--Cuts--");

  //fListTree=NULL;
  //if (fListTree) delete fListTree;

  fListTree = new TGListTree(gCanvas, kVerticalFrame);
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");
  fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "DoClick(TGListTreeItem*,Int_t)");

  //fListTree->AddRoot("Histograms");


  //cout << "make_ltree1: " << endl;
  Make_Ltree();
  //cout << "make_ltree2: " << endl;

  fListTree->Connect("DataDropped(TGListTreeItem*, TDNDData*)", "HistFrame",
                    this, "DataDropped(TGListTreeItem*,TDNDData*)");

  //fListTree->Connect("KeyPressed(TGListTreeItem*, UInt_t, UInt_t)","HistFrame",
  //		     this,"DoKey(TGListTreeItem*, UInt_t)");

  //TGListTree* fListTree2 = new TGListTree(gCanvas, kHorizontalFrame);

  fCutTree = new TGListTree(gCanvas2, kVerticalFrame);
  

  for (int i=0;i<NR;i++) {
    fVer[i] = new TGVerticalFrame(fHor2, 10, 10);
    flab[i] = new TGLabel(fVer[i], tRad[i]);
    Rb[i]= new TGRadioButton(fVer[i],"",i+1);
    Rb[i]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
    fHor2->AddFrame(fVer[i], fLay3);
    fVer[i]->AddFrame(Rb[i], fLay4);
    fVer[i]->AddFrame(flab[i], fLay4);
  }

  // cout << "1111c7" << endl;
  // int sel = abs(opt.sel_hdiv)%NR;
  // SelectDiv(sel);
  // Rb[sel]->Clicked();
  // cout << "1111c8" << endl;

  TGTextButton* but;

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

  but = new TGTextButton(fHor2,"Show",3);
  but->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2,"Clear",4);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor2->AddFrame(but, fLay5);

  //DoReset();

  //Update();
}

HistFrame::~HistFrame()
{
  cout << "~HistFrame()" << endl;
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
  if (userData) {
    item=fListTree->AddItem(parent, string, userData, open, closed, true);
    item->SetDNDSource(kTRUE);
    HMap* map = (HMap*) userData;
    fListTree->CheckItem(item,*map->chk);
  }
  else {
    item=fListTree->AddItem(parent, string, open, closed, true);
    item->SetDNDTarget(kTRUE);
  }
  return item;

}

void HistFrame::Make_Ltree() {

  TObject* obj=0;
  TIter next(hcl->hilist);

  char cuttitle[MAXCUTS][99];
  char cutname[99];
  char name[99],htitle[99];
  TGPicture *pic_2d = (TGPicture*) gClient->GetPicture("marker25.xpm");
  TGPicture *pic_xy = (TGPicture*) gClient->GetPicture("marker.xpm");
  TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");
  TGPicture *pic=0;
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir=0;
  TGListTreeItem *item=0;
  //TGListTreeItem *item;
  int col[MAXCUTS];
  memset(col,2,sizeof(col));

  //fListTree->AddRoot("Histograms");
  iWork = Item_Ltree(iroot, "WORK",0,0,0);
  for (int cc=0;cc<opt.ncuts;cc++) {
    sprintf(cutname,"WORK_cut%d",cc+1);
    iWork_cut[cc] = Item_Ltree(iroot, cutname,0,0,0);
  }

  next.Reset();
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;
    TString title = TString(map->GetTitle());
    //cout << "Title: " << title << " " << endl;
    if (!fListTree->FindChildByName(0,title)) {
      idir = Item_Ltree(iroot, title,0,0,0);
    }
    if (map->hst->InheritsFrom(TH2::Class()))
      pic=pic2;
    else
      pic=pic1;
    Item_Ltree(idir, map->GetName(), map, pic, pic);
    //cout << "map: " << map->hst->GetName() << " " << *map->wrk);
    if (*map->wrk) {
      Item_Ltree(iWork, map->GetName(), map, pic, pic);

      //make clones of _work_ histograms and their hmaps
      Clone_Hist(map);
      // TH1* h0 = (TH1*) map->hst;
      // for (int cc=0;cc<opt.ncuts;cc++) {
      // 	sprintf(name,"%s_%d",h0->GetName(),cc+1);
      // 	sprintf(htitle,"%s_%d",h0->GetTitle(),cc+1);
      // 	//cout << "clone: " << name << " " << htitle << endl;
      // 	TH1* hcut = (TH1*) h0->Clone();
      // 	hcut->SetNameTitle(name,htitle);
      // 	HMap* mcut = new HMap(cutname,hcut,map->chk,&hcl->wfalse,0);
      // 	// add this map to the list h_cuts
      // 	map->h_cuts[cc]=mcut;
      // 	Item_Ltree(iWork_cut[cc], mcut->GetName(), mcut, pic, pic);
      // }
    }
    //determine cut colors
    for (int i=0;i<MAXCUTS;i++) {
      col[i]=*(map->cut_index)[i];
      if (col[i]==0) {
	break;
      }
      col[i]+=1;
      sprintf(cuttitle[i],"%s",map->GetName());
    }
  }


  //make gcuts and fill fCutTree
  for (int i=0;i<opt.ncuts;i++) {
    //make gcut
    TCutG* gcut = hcl->cutG[i];
    if (gcut) delete gcut;
    sprintf(cutname,"cut%d",i+1);
    gcut = new TCutG(cutname,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    gcut->SetTitle(cuttitle[i]);
    gcut->SetLineColor(col[i]);
    //gcut->Print();

    //fill fCutTree
    item = fCutTree->AddItem(0, gcut->GetName(), gcut, pic_2d, pic_2d, false);
    fCutTree->AddItem(item, gcut->GetTitle(), gcut, pic2, pic2, false);
    for (int i=0;i<gcut->GetN();i++) {
      sprintf(name,"%0.2f; %0.2f",gcut->GetX()[i],gcut->GetY()[i]);
      fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
    }
    cout << "AddCutg: " << i+1 << " " << gcut->GetName() << " " << opt.pcuts[i] << endl;

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
  hlist->Clear();

  TGListTreeItem *idir;
  //clear fListTree
  idir = fListTree->GetFirstItem();
  while (idir) {
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

  cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

  //return;
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
      }

      //cout << "work2: " << item << endl;
      for (int cc=0;cc<opt.ncuts;cc++) {
	sprintf(hname2,"%s_cut%d",hname,cc+1);
	item2 = fListTree->FindChildByName(iWork_cut[cc],hname2);
	//cout << "cc: " << cc << " " << hname2 << " " << item2 << endl;
	if (item2)
	  fListTree->DeleteItem(item2);
      }
    }
    else {
      // add items
      //cout << "nowork: " << item << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname,hh->GetName());
      const TGPicture *pic = item->GetPicture();
      Item_Ltree(iWork, hh->GetName(), hh, pic, pic);

      HMap* map = (HMap*) item->GetUserData();
      *map->wrk=true;

      Clone_Hist(map);
      // for (int cc=0;cc<opt.ncuts;cc++) {
      // 	sprintf(hname2,"%s_cut%d",hname,cc+1);
      // 	hh = gROOT->FindObject(hname2);
      // 	//cout << "cc: " << cc << " " << hname2 << " " << hh << endl;
      // 	Item_Ltree(iWork_cut[cc], hh->GetName(), hh, pic, pic);
      // }
    }
  }
  
  if (crs->b_stop)
    Update();
  else
    changed=true;

}

void HistFrame::Clone_Hist(HMap* map) {
  char cutname[99];
  char name[99],htitle[99];
  TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");
  TGPicture *pic=0;

  if (map->hst->InheritsFrom(TH2::Class()))
    pic=pic2;
  else
    pic=pic1;

  TH1* h0 = (TH1*) map->hst;
  for (int cc=0;cc<opt.ncuts;cc++) {
    sprintf(cutname,"WORK_cut%d",cc+1);
    sprintf(name,"%s_cut%d",h0->GetName(),cc+1);
    sprintf(htitle,"%s_cut%d",h0->GetTitle(),cc+1);
    //cout << "clone: " << name << " " << htitle << endl;
    TH1* hcut = (TH1*) h0->Clone();
    hcut->SetNameTitle(name,htitle);
    HMap* mcut = new HMap(cutname,hcut,map->chk,&hcl->wfalse,0);
    // add this map to the list h_cuts
    map->h_cuts[cc]=mcut;
    Item_Ltree(iWork_cut[cc], mcut->GetName(), mcut, pic, pic);
  }
}

/*
void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  if (!crs->b_stop)
    return;

  char hname[100];
  char hname2[100]; 
  TGListTreeItem* item2;

  cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

  //return;
  if (item->GetParent() && (but==2 || but==3)) {
    if (TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
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
      }
    }
    else {
      //cout << "nowork: " << item << endl;
      HMap* map = (HMap*) item->GetUserData();
      *map->wrk=true;
    }
  }
  

  Clear_Ltree();
  Make_Ltree();
  //Update();

}
*/

void HistFrame::DoCheck(TObject* obj, Bool_t check)
{
  //cout << "DoCheck: " << obj << " " << check << endl;
  /*
  if (!obj) {
    TGListTreeItem *idir = fListTree->GetFirstItem();
    while (idir) {
      cout << "Dir: " << idir->GetText()<< " " << idir->IsChecked() << endl;
      // TGListTreeItem *item = idir->GetFirstChild();
      // while (item) {
      // 	cout << item->GetText()<< endl;
      // 	item = item->GetNextSibling();
      // }    
      idir = idir->GetNextSibling();
    }
    
  }
  else {
    cout << "item1: " << obj->GetName() << endl;
  }
  */

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

void HistFrame::DoKey(TGListTreeItem* entry, UInt_t keysym) {
  cout << "key: " << entry << " " << keysym << endl;
}

void HistFrame::SelectDiv(int nn)
{
  switch (nn) {
  case 0:
    ndiv=1;
    xdiv=1;
    ydiv=1;
    break;
  case 1:
    ndiv=4;
    xdiv=2;
    ydiv=2;
    break;
  case 2:
    ndiv=6;
    xdiv=3;
    ydiv=2;
    break;
  case 3:
    ndiv=8;
    xdiv=4;
    ydiv=2;
    break;
  case 4:
    ndiv=16;
    xdiv=4;
    ydiv=4;
    break;
  case 5:
    ndiv=32;
    xdiv=8;
    ydiv=4;
    break;
  case 6:
    ndiv=64;
    xdiv=8;
    ydiv=8;
    break;
  default:
    ndiv=1;
    xdiv=1;
    ydiv=1;
  }

}

void HistFrame::DoRadio()
{
  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId()-1;

  //int prev = (id+NR-1)%NR;
  //cout << "doradio: " << id << endl;

  for (int i=0;i<NR;i++) {
    if (i==id)
      Rb[i]->SetState(kButtonDown);
    else
      Rb[i]->SetState(kButtonUp);
  }

  opt.sel_hdiv=id;
  if (crs->b_stop)
    Update();
  else
    changed=true;

  //SelectDiv(id);

}

void HistFrame::DoButton()
{
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

  // check if the histogram exists
  if (!hobj) {
    cout << "AddCutg: histogram not found: " << endl;
    return;
  }

  //opt.ncuts++;

  in_gcut=false;
  TCanvas* cv=fEc->GetCanvas();
  //cv->SetCrosshair(false);
  for (int i=1;i<=ndiv;i++) {
    TPad* pad = (TPad*) cv->GetPad(i);
    if (pad) {
      pad->GetListOfExecs()->Clear();
    }
  }

  MakeCutG(opt.ncuts,pl,hobj);
  Update();
  //ShowCutG(gcut);

  HMap* map = (HMap*) hobj;
  for (int i=0;i<MAXCUTS;i++) {
    cout << "index: " << i << " " << (int)*(map->cut_index)[i]
	 << " " << (int) opt.cut_h2d[0][i] << endl;
  }

}

void HistFrame::MakeCutG(int icut, TPolyLine *pl, TObject* hobj) {
  //TGPicture *pic_2d = (TGPicture*) gClient->GetPicture("marker25.xpm");
  //TGPicture *pic_xy = (TGPicture*) gClient->GetPicture("marker.xpm");
  //TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  //TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");

  char name[99];//,title[99];
  char cutname[99];
  HMap *map;
  //HMap *map2;
  //HMap *map3;
  //TH1* hh;
  TGListTreeItem* item;

  if (icut<0 || icut>=MAXCUTS) {
    cout << "Wrong icut: " << icut << endl;
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
  for (int i=0;i<MAXCUTS;i++) {
    if (*(map->cut_index)[i]==0) {
      *(map->cut_index[i])=icut+1;
      break;
    }
  }
  // for (int i=0;i<MAXCUTS;i++) {
  //   cout << "index: " << i << " " << (int)*(map->cut_index)[i] << endl;
  // }

  //1a fill opt.** variables
  opt.pcuts[icut] = pl->Size();
  //cout << "pl: " << endl;
  //pl->Print();
  for (int i=0;i<pl->Size();i++) {
    opt.gcut[icut][0][i]=pl->GetX()[i];
    opt.gcut[icut][1][i]=pl->GetY()[i];
  }

  /*
  //1b. make gcut, set linecolor
  //?? change pl to opt.***
  TCutG* gcut = hcl->cutG[icut];
  if (gcut) delete gcut;
  sprintf(cutname,"cut%d",icut+1);
  gcut = new TCutG(cutname,pl->GetN(),pl->GetX(),pl->GetY());
  //gcut->SetTitle("h2dcut");
  gcut->SetLineColor(2);
  gcut->Print();
  
  //2. add gcut to fCutTree
  //cout << "AddCutg: " << icut << " " << fCutTree->GetFirstItem() << endl;
  item = fCutTree->AddItem(0, gcut->GetName(), gcut, pic_2d, pic_2d, false);
  fCutTree->AddItem(item, gcut->GetTitle(), gcut, pic2, pic2, false);
  for (int i=0;i<gcut->GetN();i++) {
    sprintf(name,"%0.2f; %0.2f",gcut->GetX()[i],gcut->GetY()[i]);
    fCutTree->AddItem(item, name, pic_xy, pic_xy, false);
  }
  cout << "AddCutg: " << icut+1 << " " << fCutTree->GetFirstItem() << " "
       << map->hst->GetName() << endl;
  */


  //4. make copies of all histograms which are in the iWork directory
  //?? move it to the Make_Lhist
  // item = idir->GetFirstChild();
  // while (item) {
  //   map2 = (HMap*) item->GetUserData();
  //   TH1* h0 = (TH1*) map2->hst;
  //   sprintf(name,"%s_%d",h0->GetName(),icut+1);
  //   sprintf(title,"%s_%d",h0->GetTitle(),icut+1);
  //   cout << "clone: " << name << " " << title << endl;
  //   TH1* hcut = (TH1*) h0->Clone();
  //   hcut->SetNameTitle(name,title);
  //   HMap* map3 = new HMap(cutname,hcut,map->chk,&hcl->wfalse,0);
  //   map2->h_cuts[icut]=map3;
  //   item = item->GetNextSibling();
  // }
  
  opt.ncuts++;
  //5. remake Ltree
  Clear_Ltree();
  Make_Ltree();
  //Update();
  return;

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
  
  TObject* obj=0;
  TPad* pad=0;
  const char* msg;// = "Gcut Test";

  TCanvas* cv = fEc->GetCanvas();
  cv->SetCrosshair(true);
  //pad = (TPad*) fEc->GetCanvas()->GetPad(1);

  //pad->cd();
  //cout << "gPad1: " << gPad << " " << pad << endl;
  //gPad->WaitPrimitive("CUTG","CutG");
  //pad->SetCrosshair(false);

  //cv->AddExec("dynamic","DynamicExec()");
  for (int i=1;i<=ndiv;i++) {
    TPad* pad = (TPad*) cv->GetPad(i);
    pad->AddExec("dynamic","DynamicExec()");    
  }

  return;

}

void HistFrame::ShowCutG()
{
  TLegend leg = TLegend(0.7,0.8,0.99,0.99);
  leg.SetMargin(0.5);
  int nn=0;

  cout << "show..." << endl;

  TCanvas* cv=fEc->GetCanvas();
  for (int i=1;i<=ndiv;i++) {
    cv->cd(i);
    HMap* map=(HMap*) padmap[i-1];
    cout << "map: " << i << " " << map << endl;
    if (padmap[i-1]==0)
      break;
    bool found=false;
    leg.Clear();
    for (int j=0;j<MAXCUTS;j++) {
      nn=*(map->cut_index[j]);
      if (nn<=0 || nn>=MAXCUTS)
      	break;
      cout << "showcut: " << i << " " << j << " " << nn << endl;

      // if (hcl->cutG[nn]) {
      // found=true;
      // cout << "showcut: " << i << " " << j << " " << nn << " " << hcl->cutG[nn] << endl;
      //hcl->cutG[nn]->Draw("same");
      //leg.AddEntry(hcl->cutG[i],hcl->cutG[i]->GetName(),"l");
      //}
    }
    if (found) {
      leg.Draw("same");
    }
  }
  cv->Update();
}

void HistFrame::ClearCutG()
{
  cout << "Clear_cuts: " << endl;

  // fV1->RemoveFrame(clab);
  // fV1->RemoveFrame(gCanvas2);
  // //delete clab;
  // //delete gCanvas2;
  // fV1->Resize(GetDefaultSize());
  // fV1->MapSubwindows();
  // fV1->MapWindow();

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

  //return;
  
  if (!crs->b_stop) return;

  in_gcut=false;

  TCanvas *cv=fEc->GetCanvas();
  //cv->SetEditable(true);
  cv->Clear();
  //cv->SetEditable(false);

  //cout << "hst::DoReset: " << endl;
  //cv->ls();
  
  for (int i=0;i<MAXCUTS;i++) {
    if (hcl->cutG[i]) {
      delete hcl->cutG[i];
      hcl->cutG[i]=0;
    }
  }

  for (int i=0;i<opt.ncuts;i++) {
    char ss[64];
    sprintf(ss,"cut%d",i+1);
    hcl->cutG[i] = new TCutG(ss,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    hcl->cutG[i]->SetLineColor(i+2);
  }

  //hcl->NewBins();
  //cout << "Make_hist():: " << endl;
  Clear_Ltree();
  hcl->Make_hist();
  Make_Ltree();
  //cout << "Make_hist()::2 " << endl;

  /*
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      fListTree->DeleteItem(item);
      item = item->GetNextSibling();
    }
    //if (idir->GetUserData()) {
      fListTree->DeleteItem(idir);
      //}
    idir = idir->GetNextSibling();
  }

  //delete fListTree;
  
  delete_hist();
  Make_hist();
  */
  //cout << "update1" << endl;

  int sel = abs(opt.sel_hdiv)%NR;
  SelectDiv(sel);
  Rb[sel]->Clicked();

  Update();

  cv->Draw();
  cv->Update();

}

void HistFrame::Update()
{
  //cout << "in_gcut: " << in_gcut << " " << opt.b_logy << " " << chklog << endl;

  //cout << "HiFrm::Update()" << endl;

  Hmut.Lock();
  int sel = abs(opt.sel_hdiv)%NR;
  SelectDiv(sel);
  chklog->SetState((EButtonState) opt.b_logy);

  hlist->Clear();

  //Hmut.UnLock();
  //return;
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    // if (idir->IsChecked() && idir->GetUserData()) {
    //   hlist->Add(((HMap*)idir->GetUserData())->hst);
    //   //hlist->Add((TObject*)idir->GetUserData());
    // }
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->GetUserData()) {
	HMap* map = (HMap*) item->GetUserData();
	if (!TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
	  *map->chk = item->IsChecked();
	}
	if (item->IsChecked()) {
	  hlist->Add((TObject*)item->GetUserData());
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

  // TString* ss[1000000];
  // for (int i=0;i<1000000;i++) {
  //   ss[i] = new TString("asdasdasdf");
  // }
  // for (int i=0;i<1000000;i++) {
  //   delete ss[i];
  // }

  DrawHist();
  
  TCanvas* cv=fEc->GetCanvas();
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
  //hlist->Print();

  Hmut.UnLock();
  //cout << "HiFrm::Update()2" << endl;
}

void HistFrame::DrawHist()
{
  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  memset(padmap,0,sizeof(padmap));
  cv->Divide(xdiv,ydiv);
  int nn=1;
  int ii=0;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (ii>=opt.icheck) {
      cv->cd(nn);
      TH1 *hh = ((HMap*) obj)->hst;
      if (hh->GetDimension()==2) {
	if (opt.b_logy)
	  gPad->SetLogz(1);
	else
	  gPad->SetLogz(0);
	hh->Draw("zcol");
      }
      else {
	if (opt.b_logy)
	  gPad->SetLogy(1);
	else
	  gPad->SetLogy(0);
	hh->Draw();
      }
      padmap[nn-1]=obj;
      nn++;
      if (nn>ndiv)
	break;
    }
    ii++;
  }
  cv->Update();
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

  if (changed || started) {
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
      gPad->Modified(1);
      //gPad->Update();
    }



    //cout << "unchanged3" << endl;
    fEc->GetCanvas()->Update();
    //cout << "unchanged4" << endl;
  }

}

void HistFrame::DataDropped(TGListTreeItem *, TDNDData *data)
{
  cout << "YK dropped" << endl;
}
