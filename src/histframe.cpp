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

extern TH1F *hrms;
extern TH1F *htdc_a[MAX_CH]; //absolute tof (start=0)

extern HistFrame* EvtFrm;

//TText txt;

//TMutex *Emut;

TGLayoutHints* fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);
TGLayoutHints* fLay2 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
TGLayoutHints* fLay3 = new TGLayoutHints(kLHintsLeft|kLHintsTop,10,10,0,0);
TGLayoutHints* fLay4 = new TGLayoutHints(kLHintsCenterX|kLHintsTop,0,0,0,0);
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

  const char* tRad[NR]={"1x1","2x2","3x2","4x2"};

  //cout << "HistFrame: " << gDNDManager << endl;

  ntab=nt;

  char ss[100];

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
  //fEc = new MECanvas("Hcanvas", fHor1, 10, 10);
  fHor1->AddFrame(fEc, fLay1);

  //TGVertical3DLine *separator1 = new TGVertical3DLine(fHor1);
  //fHor1->AddFrame(separator1, fLay2);

  fCanvas = new TGCanvas(fHor1, 150, 100);
  fHor1->AddFrame(fCanvas, fLay7);
  fListTree = new TGListTree(fCanvas, kHorizontalFrame);

  //fListTree->Associate(this);
  //fEc->SetDNDTarget(kTRUE);


  Make_hist();

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


  //fListTree->Connect("Clicked(TGListTreeItem*,Int_t)","HistFrame",this,
  //		     "DoClick(TGListTreeItem*,Int_t)");


  
  fListTree->SetCheckMode(TGListTree::kRecursive);
  fListTree->Connect("Checked(TObject*, Bool_t)","HistFrame",this,"DoCheck(TObject*, Bool_t)");

  //void Clicked(TGListTreeItem* entry, Int_t btn, UInt_t mask, Int_t x, Int_t y)


  //fListTree->Connect("DoubleClicked(TGListTreeItem*,Int_t)","HistFrame",this,
  //		     "DoDblClick(TGListTreeItem*,Int_t)");

   //fListTree->Connect("KeyPressed(TGListTreeItem*, UInt_t, UInt_t)","HistFrame",
   //		     this,"DoKey(TGListTreeItem*, UInt_t)");
  

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
  Rb[sel]->Clicked();

  hlist=new TList();
  //Update();
}

HistFrame::~HistFrame()
{
  cout << "~HistFrame()" << endl;
}


void HistFrame::Make_hist() {

  //char title[100];
  char nam[100];


  const TGPicture *pic = gClient->GetPicture("h1_t.xpm");
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir;
  TGListTreeItem *item;

  //iroot = fListTree->AddItem(0, "All",0,0,true);

  idir = fListTree->AddItem(iroot, "Amplitude",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(nam,"ampl_%02d",i);
    h_ampl[i]=new TH1F(nam,nam,opt.sum_max*opt.sum_bins,0.,opt.sum_max);
    item = fListTree->AddItem(idir, nam, h_ampl[i], pic, pic,true);
    item->CheckItem(false);
  }
  idir->CheckItem(false);
  
  idir = fListTree->AddItem(iroot, "Height",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(nam,"height_%02d",i);
    h_height[i]=new TH1F(nam,nam,opt.sum_max*opt.sum_bins,0.,opt.sum_max);
    item = fListTree->AddItem(idir, nam, h_height[i], pic, pic,true);
    item->CheckItem(false);
  }
  idir->CheckItem(false);
  
  idir = fListTree->AddItem(iroot, "Time",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(nam,"time_%02d",i);
    h_time[i]=new TH1F(nam,nam,opt.long_max*opt.long_bins,0.,opt.long_max);
    item = fListTree->AddItem(idir, nam, h_time[i], pic, pic,true);
    item->CheckItem(false);
  }
  idir->CheckItem(false);
  
  idir = fListTree->AddItem(iroot, "TOF",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(nam,"tof_%02d",i);
    h_tof[i]=new TH1F(nam,nam,opt.tof_max*opt.tof_bins,0.,opt.tof_max);
    item = fListTree->AddItem(idir, nam, h_tof[i], pic, pic,true);
    item->CheckItem(false);
  }
  idir->CheckItem(false);

}

void HistFrame::FillHist(EventClass* evt) {
  //cout << "fillhist" << endl;
  double DT = opt.period*1e-9;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    int ch = evt->pulses[i].Chan;
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];
      double tt = evt->pulses[i].Tstamp64 + pk->Pos;
      //cout << "FilHist: " << ch << " " << tt*DT << endl;
      h_time[ch]->Fill(tt*DT);
      h_ampl[ch]->Fill(pk->Area*opt.emult[ch]);
      h_height[ch]->Fill(pk->Height);
    }
  }
}

/*
void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  cout << "DoClick: " << item << " " << but << endl;
  if (but==1) {
    if (item->IsOpen()) {
      fListTree->CloseItem(item);
    }
    else {
      fListTree->OpenItem(item);
    }
  }
}
*/

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

void HistFrame::DoCheck(TObject* obj, Bool_t check)
{
  cout << "DoCheck: " << obj << " " << check << endl;
  Update();
}

//void HistFrame::DoKey(TGListTreeItem* entry, UInt_t keysym) {
//  cout << "key: " << entry << " " << keysym << endl;
//}

void HistFrame::DoRadio()
{
  TGButton *btn = (TGButton *) gTQSender;
  Int_t id = btn->WidgetId()-1;
  //int prev = (id+NR-1)%NR;

  cout << "doradio: " << id << endl;

  for (int i=0;i<NR;i++) {
    if (i==id)
      Rb[i]->SetState(kButtonDown);
    else
      Rb[i]->SetState(kButtonUp);
  }

  opt.sel_hdiv=id;
  switch (id) {
  case 0:
    xdiv=1;
    ydiv=1;
    break;
  case 1:
    xdiv=2;
    ydiv=2;
    break;
  case 2:
    xdiv=3;
    ydiv=2;
    break;
  case 3:
    xdiv=4;
    ydiv=2;
    break;
  }

  fEc->GetCanvas()->Clear();
  fEc->GetCanvas()->Divide(xdiv,ydiv);
  
  /*
  int j;

  int ii=id/MAX_R;
  int k=id%MAX_R;

  Pixel_t bkg=fLabel[0]->GetBackground();

  if (ii>=0 && ii<MAX_CH) {
    for (j=ii*MAX_R;j<ii*MAX_R+MAX_R;j++) {
      if (j!=id) {
	fR[j]->SetState(kButtonUp);
	fR[j]->ChangeBackground(bkg);
      }
    }
    //YK opt.channels[i]=(ChannelDef) k;
    if (k!=MAX_R-1) {
      fR[id]->ChangeBackground(fColor[ii]->GetColor());
      //fR[id]->ChangeBackground(gROOT->GetColor(opt.color[i])->GetPixel());
    }
  }
  else if (ii==MAX_CH) {
    for (int i=0;i<=MAX_CH;i++) {
      for (j=i*MAX_R;j<i*MAX_R+MAX_R;j++) {
	if (j%MAX_R!=k) {
	  fR[j]->SetState(kButtonUp);
	  fR[j]->ChangeBackground(bkg);
	}
	else {
	  fR[j]->SetState(kButtonDown);
	  if (j%MAX_R!=MAX_R-1) {
	    fR[j]->ChangeBackground(fColor[i]->GetColor());
	    //fR[j]->ChangeBackground(gROOT->GetColor(opt.color[i])->GetPixel());
	  }
	}
      }
      //YK opt.channels[i]=(ChannelDef) k;
    }
  }
  else{
    printf("DoRadio: wrong id: %d\n",id);
  }
  */
}

void HistFrame::Update()
{
  hlist->Clear();
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    //cout << "Dir: " << idir->GetText() << " : " << idir->GetUserData() << endl;
    if (idir->IsChecked()) {
      if (idir->GetUserData()) {
	//TH1* hh = (TH1*) idir->GetUserData();
	hlist->Add((TObject*)idir->GetUserData());
      }
      TGListTreeItem *item = idir->GetFirstChild();
      while (item) {
	if (item->IsChecked() && item->GetUserData()) {
	  //TH1* hh = (TH1*) item->GetUserData();
	  hlist->Add((TObject*)item->GetUserData());
	}
	//cout << item->GetText() << " : " << item->GetUserData() << endl;
	item = item->GetNextSibling();
      }
    }
    idir = idir->GetNextSibling();
  }

  //fListTree->GetChecked(hlist);
  cout << "hlist: " << hlist->GetSize() << endl;

  /*
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    TH1 *hh = (TH1*) obj;
    //cout << "Checked: " << hh->GetName() << endl;
  }
  */
  
}

void HistFrame::DrawHist()
{
  int nn=1;
  TIter next(hlist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    cout << "DrawHist: " << fEc->GetCanvas()->GetPad(nn) << endl;
    if (!fEc->GetCanvas()->GetPad(nn)) break;
    fEc->GetCanvas()->cd(nn);
    TH1 *hh = (TH1*) obj;
    hh->Draw();
    nn++;
    //cout << "Checked: " << hh->GetName() << endl;
  }
  fEc->GetCanvas()->Update();
}
