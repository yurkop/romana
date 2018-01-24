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
//#include <TROOT.h>


#include <TRandom.h>

//TLine ln1;

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

extern HistFrame* EvtFrm;

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
TGLayoutHints* fLay7 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY,5,0,0,0);

//------------------------------

/*
Bool_t MECanvas::HandleDNDDrop(TDNDData *data) {
  //TRootEmbeddedCanvas(data);
  cout << "DND" << endl;
  return TRootEmbeddedCanvas::HandleDNDDrop(data);
}
*/

//------------------------------

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
  //TGHorizontalFrame      *fHor3; //contains buttons etc

  TGVerticalFrame *fVer[NR];
  TGLabel *flab[NR];

  fHor1 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor1, fLay1);

  fHor2 = new TGHorizontalFrame(this, 10, 10);
  AddFrame(fHor2, fLay2);

  //fHor3 = new TGHorizontalFrame(this, 10, 10);
  //AddFrame(fHor3, fLay3);

  // fHor2->AddFrame(flab, fLay4);

  // TGRadioButton* rb1= new TGRadioButton(fHor3,"",1);
  // TGRadioButton* rb2= new TGRadioButton(fHor3,"",2);
  // TGRadioButton* rb3= new TGRadioButton(fHor3,"",3);
  // TGRadioButton* rb4= new TGRadioButton(fHor3,"",4);

  // fHor3->AddFrame(rb1, fLay5);
  // fHor3->AddFrame(rb2, fLay5);
  // fHor3->AddFrame(rb3, fLay5);
  // fHor3->AddFrame(rb4, fLay5);

  fEc = new TRootEmbeddedCanvas("Hcanvas", fHor1, 10, 10);
  //fEc->GetCanvas()->SetEditable(false);
  //fEc = new MECanvas("Hcanvas", fHor1, 10, 10);
  fHor1->AddFrame(fEc, fLay1);

  //TGVertical3DLine *separator1 = new TGVertical3DLine(fHor1);
  //fHor1->AddFrame(separator1, fLay2);

  fCanvas = new TGCanvas(fHor1, 150, 100);
  fHor1->AddFrame(fCanvas, fLay7);


  //fListTree=NULL;
  //if (fListTree) delete fListTree;

  fListTree = new TGListTree(fCanvas, kHorizontalFrame);
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");
  fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
   		     "DoClick(TGListTreeItem*,Int_t)");



  //fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","TGListTree",fListTree,
  //		     "DoubleClicked(TGListTreeItem*,Int_t)");

  //fListTree->Associate(this);
  //fEc->SetDNDTarget(kTRUE);

  //Make_hist();
  //exit(-1);
  cout << "Make_Ltree1: " << endl;
  Make_Ltree();
  cout << "Make_Ltree2: " << endl;
  //Clear_tree();
  //Make_tree();

  fListTree->Connect("DataDropped(TGListTreeItem*, TDNDData*)", "HistFrame",
                    this, "DataDropped(TGListTreeItem*,TDNDData*)");

  //NewBins();

  /*
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    cout << "Dir: " << idir->GetText()<< endl;
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      cout << item->GetText()<< endl;
      item = item->GetNextSibling();
    }    
    idir = idir->GetNextSibling();
  }
  */



  
  //void Clicked(TGListTreeItem* entry, Int_t btn, UInt_t mask, Int_t x, Int_t y)


  //fListTree->Connect("DoubleClicked(TGListTreeItem*,Int_t)","HistFrame",this,
  //		     "DoDblClick(TGListTreeItem*,Int_t)");

  fListTree->Connect("KeyPressed(TGListTreeItem*, UInt_t, UInt_t)","HistFrame",
   		     this,"DoKey(TGListTreeItem*, UInt_t)");
  

  for (int i=0;i<NR;i++) {
    fVer[i] = new TGVerticalFrame(fHor2, 10, 10);
    flab[i] = new TGLabel(fVer[i], tRad[i]);
    Rb[i]= new TGRadioButton(fVer[i],"",i+1);
    Rb[i]->Connect("Clicked()", "HistFrame", this, "DoRadio()");
    fHor2->AddFrame(fVer[i], fLay3);
    fVer[i]->AddFrame(Rb[i], fLay4);
    fVer[i]->AddFrame(flab[i], fLay4);
  }

  int sel = abs(opt.sel_hdiv)%NR;
  SelectDiv(sel);
  Rb[sel]->Clicked();

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

  TGCheckButton* chk;
  chk = new TGCheckButton(fHor2,"Log",11);
  chk->Connect("Clicked()", "HistFrame", this, "DoLog()");
  fHor2->AddFrame(chk, fLay5);

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
  but->Connect("Clicked()", "HistFrame", this, "AddCutG()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2,"Show",2);
  but->Connect("Clicked()", "HistFrame", this, "ShowCutG()");
  fHor2->AddFrame(but, fLay5);

  but = new TGTextButton(fHor2,"Clear",3);
  but->Connect("Clicked()", "HistFrame", this, "ClearCutG()");
  fHor2->AddFrame(but, fLay5);

  DoReset();

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

  char ss[64];
  TGPicture *pic1 = (TGPicture*) gClient->GetPicture("h1_t.xpm");
  TGPicture *pic2 = (TGPicture*) gClient->GetPicture("h2_t.xpm");
  TGPicture *pic=0;
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir=0;
  //TGListTreeItem *item;

  iWork = Item_Ltree(iroot, "WORK",0,0,0);
  for (int cc=0;cc<opt.ncuts;cc++) {
    sprintf(ss,"WORK_cut%d",cc+1);
    iWork_cut[cc] = Item_Ltree(iroot, ss,0,0,0);
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
    }
  }

}

/*
void HistFrame::Make_Ltree() {

  char ss[64];
  const TGPicture *pic1 = gClient->GetPicture("h1_t.xpm");
  const TGPicture *pic2 = gClient->GetPicture("h2_t.xpm");
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir;
  //TGListTreeItem *item;

  iWork = Item_Ltree(iroot, "WORK",0,0,0,true);
  Item_Ltree(iWork, hcl->h_ampl[0][0]->GetName(), hcl->h_ampl[0][0], pic1, pic1,true);
  Item_Ltree(iWork, hcl->h_ampl[1][0]->GetName(), hcl->h_ampl[1][0], pic1, pic1,true);
  Item_Ltree(iWork, hcl->h_mtof[2][0]->GetName(), hcl->h_mtof[2][0], pic1, pic1,true);
  Item_Ltree(iWork, hcl->h_2d[0]->GetName(),      hcl->h_2d[0],      pic2, pic2,true);

  for (int cc=0;cc<opt.ncuts;cc++) {
    sprintf(ss,"WORK_cut%d",cc+1);
    iWork_cut[cc] = Item_Ltree(iroot, ss,0,0,0,true);
    Item_Ltree(iWork_cut[cc], hcl->h_ampl[0][cc+1]->GetName(), hcl->h_ampl[0][cc+1], pic1, pic1,true);
    Item_Ltree(iWork_cut[cc], hcl->h_ampl[1][cc+1]->GetName(), hcl->h_ampl[1][cc+1], pic1, pic1,true);
    Item_Ltree(iWork_cut[cc], hcl->h_mtof[2][cc+1]->GetName(), hcl->h_mtof[2][cc+1], pic1, pic1,true);
    Item_Ltree(iWork_cut[cc], hcl->h_2d[cc+1]->GetName(),      hcl->h_2d[cc+1],      pic2, pic2,true);

  }

  idir = Item_Ltree(iroot, "Amplitude",0,0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    Item_Ltree(idir, hcl->h_ampl[i][0]->GetName(), hcl->h_ampl[i][0], pic1, pic1,true,opt.s_amp[i]);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

  idir = Item_Ltree(iroot, "Height",0,0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    Item_Ltree(idir, hcl->h_height[i][0]->GetName(), hcl->h_height[i][0], pic1, pic1,true,opt.s_hei[i]);
  }

  idir = Item_Ltree(iroot, "Time",0,0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    Item_Ltree(idir, hcl->h_time[i][0]->GetName(), hcl->h_time[i][0], pic1, pic1,true,opt.s_time[i]);
  }
  
  idir = Item_Ltree(iroot, "TOF",0,0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    Item_Ltree(idir, hcl->h_tof[i][0]->GetName(), hcl->h_tof[i][0], pic1, pic1,true,opt.s_tof[i]);
  }

  idir = Item_Ltree(iroot, "MTOF",0,0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    Item_Ltree(idir, hcl->h_mtof[i][0]->GetName(), hcl->h_mtof[i][0], pic1, pic1,true,opt.s_mtof[i]);
  }


  // idir = Item_Ltree(iroot, "Profile strip",0,0,true);
  // for (int i=0; i<64; i++){
  //   Item_Ltree(idir, h2_prof_strip[i], pic1, pic1,true);
  // }

  // idir = Item_Ltree(iroot, "Profile real",0,0,true);
  // for (int i=0; i<64; i++){
  //   Item_Ltree(idir, name, h2_prof_real[i], pic1, pic1,true);
  // }

  
  idir = Item_Ltree(iroot, "2d",0,0,0,true);
  Item_Ltree(idir, hcl->h_2d[0]->GetName(), hcl->h_2d[0], pic2, pic2,true);

  //fListTree->Sort(idir);

}
*/


/*
void HistFrame::Reset_hist() {

  for (int i=0;i<MAX_CH;i++) {
    h_ampl[i]->   SetBins(opt.sum_max*opt.sum_bins,0.,opt.sum_max);
    h_height[i]-> SetBins(opt.sum_max*opt.sum_bins,0.,opt.sum_max);
    h_time[i]->   SetBins(opt.long_max*opt.long_bins,0.,opt.long_max);
    h_tof[i]->    SetBins(opt.tof_max*opt.tof_bins,0.,opt.tof_max);
  }

}
*/

void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  char hname[100];
  char hname2[100]; 
  TGListTreeItem* item2;

  cout << "DoClick: " << item->GetText() << " " << item->GetParent() << " " << but << endl;

  //return;
  if (item->GetParent() && (but==2 || but==3)) {
    if (TString(item->GetParent()->GetText()).Contains("work",TString::kIgnoreCase)) {
      //cout << "work: " << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname2,hh->GetName());
      char* str = strstr(hname2,"_cut");
      if (str) {
	strncpy(hname,hname2,str-hname2);
	hname[str-hname2] = '\0';   /* null character manually added */
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
      //cout << "nowork: " << item << endl;
      TObject* hh = (TObject*) item->GetUserData();
      strcpy(hname,hh->GetName());
      const TGPicture *pic = item->GetPicture();
      Item_Ltree(iWork, hh->GetName(), hh, pic, pic);

      HMap* map = (HMap*) item->GetUserData();
      *map->wrk=true;

      for (int cc=0;cc<opt.ncuts;cc++) {
	sprintf(hname2,"%s_cut%d",hname,cc+1);
	hh = gROOT->FindObject(hname2);
	//cout << "cc: " << cc << " " << hname2 << " " << hh << endl;
	Item_Ltree(iWork_cut[cc], hh->GetName(), hh, pic, pic);
      }
    }
  }
  
  //fListTree->DoubleClicked(item,but);

  /*
  if (but==1) {
    if (item->IsOpen()) {
      fListTree->CloseItem(item);
    }
    else {
      fListTree->OpenItem(item);
    }
  }
  */
}

/*
void HistFrame::DoClick(TGListTreeItem* entry, Int_t btn, UInt_t mask, Int_t x, Int_t y)
{
  cout << "DoClick2: " << entry << " " << btn << " " << mask << " "
       << entry->IsChecked() << endl;
  //entry->SetColor(TColor::GetColor(myM->fBluevio));

  //return;

  if (mask) {
    entry->Toggle();
    //entry->CheckAllChildren(entry->IsChecked());
    //Update();
  }
  
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

void HistFrame::AddCutG()
{
  TObject* obj=0;
  TPad* pad;
  const char* msg;// = "Gcut Test";
  Int_t retval;

  for (int i=1;i<=ndiv;i++) {
    pad = (TPad*) fEc->GetCanvas()->GetPad(i);
    obj = pad->GetPrimitive("h2d");
    if (obj)
      break;
    //cout << obj << endl;
    //TList* list = pad->GetL
  }

  if (opt.ncuts>=MAXCUTS-1) {
    msg = "Too manu cuts. Delete them first";
    obj=0;
  }
  else {
    if (obj) {
      msg = "Use your mouse to create a graphical cut on the h2d histogram\nUse double click to finish";
    }
    else {
      msg = "Error: Graphical cut is only possible with h2d histogram\nUse Histograms menu on the right to make it visible";
    }
  }

  new TGMsgBox(gClient->GetRoot(), this,
	       "Graphical cut",
	       msg, kMBIconAsterisk, kMBDismiss, &retval);

  if (!obj) return;
  //cout << "test: " << obj << endl;

  pad->cd();
  pad->SetCrosshair(true);
  pad->WaitPrimitive("CUTG","CutG");
  pad->SetCrosshair(false);

  TCutG* cc= (TCutG*)gROOT->GetListOfSpecials()->FindObject("CUTG");

  int np = cc->GetN();

  if (np>9) {
    new TGMsgBox(gClient->GetRoot(), this,
		 "Graphical cut",
		 "Too many points", kMBIconAsterisk, kMBDismiss, &retval);
    delete cc;
    return;
  }
  if (np<=3) {
    new TGMsgBox(gClient->GetRoot(), this,
		 "Graphical cut",
		 "Too few points", kMBIconAsterisk, kMBDismiss, &retval);
    delete cc;
    return;
  }
  //cout << "cc: " << cc->GetN() << endl;
  int nc=opt.ncuts;

  hcl->cutG[nc] = new TCutG(*cc);
  delete cc;
  hcl->cutG[nc]->SetLineColor(nc+2);

  char ss[64];
  sprintf(ss,"cut%d",nc+1);
  hcl->cutG[nc]->SetName(ss);

  pad->cd();

  opt.pcuts[nc]=np;
  for (int i=0;i<np;i++) {
    opt.gcut[nc][0][i] = hcl->cutG[nc]->GetX()[i];
    opt.gcut[nc][1][i] = hcl->cutG[nc]->GetY()[i];
  }

  //opt.gcut[0]->Print();
  hcl->cutG[nc]->Draw();

  opt.ncuts++;
  //Make_hist(opt.ncuts);
  Clear_Ltree();
  Make_Ltree();

  fEc->GetCanvas()->Update();
}

void HistFrame::ShowCutG()
{

  //cout << "show..." << endl;

  TObject* obj=0;
  TPad* pad;
  bool found=false;

  for (int i=1;i<=ndiv;i++) {
    pad = (TPad*) fEc->GetCanvas()->GetPad(i);
    TList* list = pad->GetListOfPrimitives();
    TIter next(list);
    while ( (obj=(TObject*)next()) ) {
      TString name = TString(((TNamed*) obj)->GetName());
      //const char* name = ((TNamed*) obj)->GetName();
      //cout << name << " " << endl;
      //cout << name << " " << name.Contains("h2d",TString::kIgnoreCase) << endl;
      if (name.Contains("h2d",TString::kIgnoreCase)) {
	found=true;
      }
    }
    
    //obj = pad->GetPrimitive("h2d");
    if (found)
      break;
    //cout << obj << endl;
    //TList* list = pad->GetL
  }

  if (!found || opt.ncuts==0)
    return;

  TLegend *leg = new TLegend(0.7,0.8,0.99,0.99);
  leg->SetMargin(0.5);
  pad->cd();
  //opt.gcut[0]->Print();
  for (int i=0;i<opt.ncuts;i++) {
    if (hcl->cutG[i]) {
      cout << "cut: " << i << " " << hcl->cutG[i]->GetName() << endl;
      hcl->cutG[i]->Draw("same");
      leg->AddEntry(hcl->cutG[i],hcl->cutG[i]->GetName(),"l");
    }
  }

  leg->Draw();
  fEc->GetCanvas()->Update();
}

void HistFrame::ClearCutG()
{
  cout << "Clear_cuts: " << endl;

  Clear_Ltree();

  //opt.gcut[0]->Print();
  for (int i=0;i<opt.ncuts;i++) {
    delete hcl->cutG[i];
    hcl->cutG[i]=0;
  }

  /*
  for (int cc=1;cc<MAXCUTS;cc++) {
    for (int i=0;i<MAX_CH;i++) {
      delete h_ampl[i][cc];
      delete h_height[i][cc];
      delete h_time[i][cc];
      delete h_tof[i][cc];
      delete h_mtof[i][cc];

      h_ampl[i][cc]=0;
      h_height[i][cc]=0;
      h_time[i][cc]=0;
      h_tof[i][cc]=0;
      h_mtof[i][cc]=0;
    }
    delete h_2d[cc];
    h_2d[cc]=0;
  }
  */

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
    ii++;
  }
  //fEc->GetCanvas()->SetEditable(true);
  fEc->GetCanvas()->Update();
  //fEc->GetCanvas()->SetEditable(false);

}

void HistFrame::Clear_Ltree()
{
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
}

void HistFrame::DoReset()
{

  //return;
  
  if (!crs->b_stop) return;

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

  hcl->NewBins();

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
  Update();

  cv->Draw();
  cv->Update();

}

void HistFrame::Update()
{
  //cout << "HiFrm::Update()" << endl;

  Hmut.Lock();
  int sel = abs(opt.sel_hdiv)%NR;
  SelectDiv(sel);

  hlist->Clear();
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
	  hlist->Add(((HMap*)item->GetUserData())->hst);
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

  Hmut.UnLock();
}

void HistFrame::DrawHist()
{

  TCanvas *cv=fEc->GetCanvas();
  cv->Clear();
  cv->Divide(xdiv,ydiv);
  int nn=1;
  int ii=0;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (ii>=opt.icheck) {
      cv->cd(nn);
      TH1 *hh = (TH1*) obj;
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
