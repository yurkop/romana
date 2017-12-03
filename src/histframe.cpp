//----- HistFrame ----------------
#include "romana.h"

#include <TColor.h>
#include <TCanvas.h>
#include <TText.h>
#include <TStyle.h>
#include <TLine.h>
#include <TSpectrum.h>
#include <TMutex.h>
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
TGLayoutHints* fLay5 = new TGLayoutHints(kLHintsLeft|kLHintsTop,5,5,0,0);
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


  Make_hist();
  NewBins();


  
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

  but = new TGTextButton(fHor2,"Peaks",4);
  but->Connect("Clicked()", "HistFrame", this, "DoPeaks()");
  fHor2->AddFrame(but, fLay5);

  //Update();
}

HistFrame::~HistFrame()
{
  cout << "~HistFrame()" << endl;
}


void HistFrame::Make_hist() {

  //char title[100];
  char name[100];
  char title[100];

  const TGPicture *pic = gClient->GetPicture("h1_t.xpm");
  TGListTreeItem *iroot=0;
  TGListTreeItem *idir;
  //TGListTreeItem *item;

  //gStyle->SetNdivisions(606,"xyz");
  

  //iroot = fListTree->AddItem(0, "All",0,0,true);

  idir = fListTree->AddItem(iroot, "Amplitude",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(name,"ampl_%02d",i);
    sprintf(title,"ampl_%02d;Channel;Counts",i);
    int nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
    h_ampl[i]=new TH1F(name,title,nn,opt.amp_min,opt.amp_max);
    fListTree->AddItem(idir, name, h_ampl[i], pic, pic,true);
    //cout << i << " " << ((TObject*)idir->GetUserData())->TestBit(kCanDelete);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

  idir = fListTree->AddItem(iroot, "Height",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(name,"height_%02d",i);
    sprintf(title,"height_%02d;Channel;Counts",i);
    int nn=opt.hei_bins*(opt.hei_max-opt.hei_min);
    h_height[i]=new TH1F(name,title,nn,opt.hei_min,opt.hei_max);
    fListTree->AddItem(idir, name, h_height[i], pic, pic,true);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

  idir = fListTree->AddItem(iroot, "Time",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(name,"time_%02d",i);
    sprintf(title,"time_%02d;T(sec);Counts",i);
    int nn=opt.time_bins*(opt.time_max-opt.time_min);
    h_time[i]=new TH1F(name,title,nn,opt.time_min,opt.time_max);

    /*
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,0,0)
    h_time[i]->CanExtendAllAxes();
#else
    h_time[i]->SetBit(TH1::kCanRebin);
#endif
    */

    fListTree->AddItem(idir, name, h_time[i], pic, pic,true);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);
  
  idir = fListTree->AddItem(iroot, "TOF",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(name,"tof_%02d",i);
    sprintf(title,"tof_%02d;t(ns);Counts",i);
    int nn=opt.tof_bins*(opt.tof_max-opt.tof_min);
    h_tof[i]=new TH1F(name,title,nn,opt.tof_min,opt.tof_max);
    fListTree->AddItem(idir, name, h_tof[i], pic, pic,true);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

  idir = fListTree->AddItem(iroot, "MTOF",0,0,true);
  for (int i=0;i<MAX_CH;i++) {
    sprintf(name,"mtof_%02d",i);
    sprintf(title,"mtof_%02d;t(mks);Counts",i);
    int nn=opt.mtof_bins*(opt.mtof_max-opt.mtof_min);
    h_mtof[i]=new TH1F(name,title,nn,opt.mtof_min,opt.mtof_max);
    fListTree->AddItem(idir, name, h_mtof[i], pic, pic,true);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

  idir = fListTree->AddItem(iroot, "Profile strip",0,0,true);
  for (int i=0; i<64; i++){
    sprintf(name,"Profile_strip%02d",i);
    sprintf(title,"Profile_strip%02d;X_strip;Y_strip",i);
    h2_prof_strip[i] = new TH2F(name,title,8,0,8,8,0,8);
    //h2_prof_strip[i]->GetXaxis()->CenterTitle();
    //h2_prof_strip[i]->GetYaxis()->CenterTitle();
    fListTree->AddItem(idir, name, h2_prof_strip[i], pic, pic,true);
  }

  idir = fListTree->AddItem(iroot, "Profile real",0,0,true);
  for (int i=0; i<64; i++){
    sprintf(name,"profile_real%02d",i);
    sprintf(title,"Profile_real%02d;X (mm);Y (mm)",i);
    h2_prof_real[i] = new TH2F(name,title,8,0,120,8,0,120); 
    fListTree->AddItem(idir, name, h2_prof_real[i], pic, pic,true);
  }

  idir = fListTree->AddItem(iroot, "2d",0,0,true);
  for (int i=0;i<1;i++) {
    sprintf(name,"h2d_%02d",i);
    sprintf(title,"h2d_%02d;Channel;Counts",i);
    int nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
    h_2d[i]=new TH2F(name,title,nn,opt.amp_min,opt.amp_max,
		     nn,opt.amp_min,opt.amp_max);
    fListTree->AddItem(idir, name, h_2d[i], pic, pic,true);
    //cout << i << " " << ((TObject*)idir->GetUserData())->TestBit(kCanDelete);
    //item->CheckItem(false);
  }
  //idir->CheckItem(false);

}

void HistFrame::NewBins() {

  int nn;

  for (int i=0;i<MAX_CH;i++) {
    nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
    h_ampl[i]->SetBins(nn,opt.amp_min,opt.amp_max);
    h_ampl[i]->Reset();
    
    nn=opt.hei_bins*(opt.hei_max-opt.hei_min);
    h_height[i]->SetBins(nn,opt.hei_min,opt.hei_max);
    h_height[i]->Reset();
    
    nn=opt.time_bins*(opt.time_max-opt.time_min);
    h_time[i]->SetBins(nn,opt.time_min,opt.time_max);
    h_time[i]->Reset();
    
    nn=opt.tof_bins*(opt.tof_max-opt.tof_min);
    h_tof[i]->SetBins(nn,opt.tof_min,opt.tof_max);
    h_tof[i]->Reset();

    nn=opt.mtof_bins*(opt.mtof_max-opt.mtof_min);
    h_mtof[i]->SetBins(nn,opt.mtof_min,opt.mtof_max);
    h_mtof[i]->Reset();
  }

  for (int i=0; i<1; i++) {
    nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
    h_2d[i]->SetBins(nn,opt.amp_min,opt.amp_max,
		     nn,opt.amp_min,opt.amp_max);
    h_2d[i]->Reset();
  }

  for (int i=0; i<64; i++) {
    h2_prof_strip[i]->Reset();
    h2_prof_real[i]->Reset();
  }

}

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

/*
void HistFrame::FillHist(EventClass* evt) {
  double DT = crs->period*1e-9;
  //int ch[MAX_CH];
  double tt;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    int ch = evt->pulses[i].Chan;
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];

      h_ampl[ch]->Fill(pk->Area*opt.emult[ch]);
      h_height[ch]->Fill(pk->Height);

      tt = evt->pulses[i].Tstamp64 + crs->Tstart64;
      tt += pk->Pos;
      h_time[ch]->Fill(tt*DT);

      double dt = evt->pulses[i].Tstamp64 - evt->T;
      //tt = pk->Time - cpar.preWr[ch] - evt->T0 + dt;
      tt = pk->Time - crs->Pre[ch] - evt->T0 + dt;
      // cout << "TOF: " << crs->nevents << " " << i << " "
      // 	   << ch << " " << pk->Time << " " << evt->T0 << " "
      // 	   << dt << " " << tt << endl;
      h_tof[ch]->Fill(tt*crs->period);

    }

    if (crs->Tstart0>0) {
      tt = evt->T - crs->Tstart0;
      h_mtof[evt->pulses.size()]->Fill(tt*crs->period/1000);
    }
    if (ch==opt.start_ch) {
      crs->Tstart0 = evt->T;
    }
  }

  if (evt->pulses.size()>=2) {
    double amp[2];
    int nn=0;
    for (UInt_t i=0;i<evt->pulses.size();i++) {
      int ch = evt->pulses[i].Chan;
      for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
	peak_type* pk = &evt->pulses[i].Peaks[j];

	if (ch==0 && j==0) {
	  amp[0]=pk->Area*opt.emult[ch];
	  nn++;
	}
	if (ch==1 && j==0) {
	  amp[1]=pk->Area*opt.emult[ch];
	  nn++;
	}

      }
    }
    if (nn==2) {
      h_2d[0]->Fill(amp[0],amp[1]);
    }
  }

  int ax=0,ay=0,px=0,py=0;
  if (evt->pulses.size()==4) {
    for (UInt_t i=0;i<evt->pulses.size();i++) {
      int ch = evt->pulses[i].Chan;
      if (ch<8) { //prof_x
	px=PROF::prof_ch[ch];
      }
      else if (ch<16) { //prof y
	py=PROF::prof_ch[ch];
      }
      else if (ch<24) { //alpha y
	ay=PROF::prof_ch[ch];
      }
      else { //alpha x
	ax=PROF::prof_ch[ch];
      }
    }

    int ch_alpha = ax + ay*8;

    //cout << "prof: " << crs->nevents << " " << ch_alpha << endl;

    h2_prof_strip[ch_alpha]->Fill(px,py);
    h2_prof_real[ch_alpha]->Fill(px*15,py*15);    
  }

}
*/

void HistFrame::DoClick(TGListTreeItem* item,Int_t but)
{
  //cout << "DoClick: " << item << " " << but << endl;
  //fListTree->DoubleClicked(item,but);

  if (but==1) {
    if (item->IsOpen()) {
      fListTree->CloseItem(item);
    }
    else {
      fListTree->OpenItem(item);
    }
  }

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

//void HistFrame::DoKey(TGListTreeItem* entry, UInt_t keysym) {
//  cout << "key: " << entry << " " << keysym << endl;
//}

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
      Float_t* peaks = spec.GetPositionX();
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
  
  NewBins();

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

  Hmut.Lock();
  int sel = abs(opt.sel_hdiv)%NR;
  SelectDiv(sel);

  hlist->Clear();
  TGListTreeItem *idir = fListTree->GetFirstItem();
  while (idir) {
    if (idir->IsChecked() && idir->GetUserData()) {
      hlist->Add((TObject*)idir->GetUserData());
    }
    TGListTreeItem *item = idir->GetFirstChild();
    while (item) {
      if (item->IsChecked() && item->GetUserData()) {
	hlist->Add((TObject*)item->GetUserData());
      }
      item = item->GetNextSibling();
    }
    idir = idir->GetNextSibling();
  }

  //fListTree->GetChecked(hlist);
  //cout << "hlist: " << hlist->GetSize() << endl;

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

  if (changed) {
    //fEc->GetCanvas()->Draw();
    //fEc->GetCanvas()->Update();
    //cout << "changed" << endl;
    Update();
    changed=false;
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
