#include "crspardlg.h"
#include "libcrs.h"
#include "toptions.h"
#include "TGFileDialog.h"
#include <TGSplitter.h>
#include <TG3DLine.h>
#include <TSystem.h>
#include "romana.h"
#include <TColor.h>

//const char* t_raw = "Write raw data";

extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern ChanParDlg *chanpar;

const int ncrspar=12;

const int tlen[ncrspar]={26,60,24,25,24,21,45,40,40,21,36,37};
const char* tlab[ncrspar]={"Ch","Type","on","Inv","AC","hS","Dt","Pre","Len","G","Drv","Thr"};
const char* tip[ncrspar]={
  "Channel number",
  "Channel type",
  "On/Off",
  "Inversion",
  "AC coupling",
  "Hardware smoothing",
  "Dead time - no new trigger on the current channel within dead time from the old trigger",
  "Number of samples before the trigger",
  "Total length of the pulse in samples",
  "0 - trigger on the pulse; Drv>0 - trigger on differential S(i) - S(i-n)",
  "Additional Gain",
  "Trigger threshold"};

const int nchpar=16;
const int tlen2[nchpar]={26,60,26,25,35,35,42,42,35,35,20,35,35,35,42,42};
const char* tlab2[nchpar]={"Ch","Type","St","sS","Bkg1","Bkg2","Peak1","Peak2","dT","Pile","Tm","T1","T2","EM","ELim1","Elim2"};
const char* tip2[nchpar]={
  "Channel number",
  "Channel type",
  "Start channel - used for making TOF/MTOF start\nif there are many start channels in the event, the earliest is used",
  "Software smoothing",
  "Background start, relative to peak Pos (negative)",
  "Background end, relative to peak Pos (negative)",
  "Peak start, relative to peak Pos (usually negative)",
  "Peak end, relative to peak Pos (usually positive)",
  "Dead-time window \nsubsequent peaks within this window are ignored",
  "Pileup window \nall peaks within this window are marked as pileup",
  "Timing mode: 0 - first derivative (T2); 1 - second derivative (Pos)",
  "Timing window start (usually negative, if 99 - use T1)",
  "Timing window end (usually positive, if 99 - use T2)",
  "Energy multiplier",
  "Low energy limit",
  "High energy limit"
};


const char* types[ADDCH+1]={"NaI","BGO","Si 1","Si 2","Stilb","Demon","HPGe",
			    "NIM","Other",""};

//-----------
const int nchtype=9;

extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern int chanPresent;
extern MyMainFrame *myM;

// TGLayoutHints* fL0;
// TGLayoutHints* fL1;
// TGLayoutHints* fL2;
// TGLayoutHints* fL3;
// TGLayoutHints* fL4;
// TGLayoutHints* fL5;
// TGLayoutHints* fL6;
// TGLayoutHints* fLexp;
TGLayoutHints* fL0 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
TGLayoutHints* fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
TGLayoutHints* fL2 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
TGLayoutHints* fL2a = new TGLayoutHints(kLHintsLeft | kLHintsBottom,0,0,5,0);
TGLayoutHints* fL3 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 4, 4, 0, 0);
TGLayoutHints* fL4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 5, 5);
TGLayoutHints* fL5 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 2, 2);
TGLayoutHints* fL6 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2, 2, 2, 2);
TGLayoutHints* fL7 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 11, 1, 2, 2);
TGLayoutHints* fL8 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 1, 1, 1);
TGLayoutHints* fLexp = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  nfld=0;

  // fL0 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
  // fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  // fL2 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
  // fL3 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 4, 4, 0, 0);
  // fL4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 5, 5);
  // fL5 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 2, 2);
  // fL6 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2, 2, 2, 2);

  // fLexp = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

}

void ParDlg::DoMap(TGWidget* f, void* d, P_Def t, int all) {
  pmap pp;
  pp.field = (TGWidget*) f;
  pp.data = d;
  pp.type = t;
  pp.all=all;
  //cout << "DoMap1: " << f << " " << d << " " << t << endl;
  Plist.push_back(pp);
  //cout << "DoMap2: " << f << " " << d << " " << t << endl;
}

void ParDlg::SetNum(pmap pp, Double_t num) {
  if (pp.type==p_fnum) {
    *(Float_t*) pp.data = num;
    //cout << "setpar1: " << *(Float_t*) pp.data << endl;
  }
  else if (pp.type==p_inum) {
    *(Int_t*) pp.data = num;
    //cout << "setpar2: " << *(Int_t*) pp.data << endl;
  }
  else {
    cout << "(DoNum) Wrong type: " << pp.type << endl;
  }
}

void ParDlg::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  // cout << "DoNum: ";
  // cout << *(Int_t*) pp.data << " ";
  // cout << pp.data << " " << opt.bkg1[0] << " ";
  // cout << (Int_t) pp.all << endl;

  SetNum(pp,te->GetNumber());

  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<chanPresent;i++) {
	if (pp.all==1 || cpar.chtype[i]==pp.all-1) {
	  pmap p2 = Plist[i*nfld+kk];
	  SetNum(p2,te->GetNumber());
	  TGNumberEntryField *te2 = (TGNumberEntryField*) p2.field;
	  te2->SetNumber(te->GetNumber());
	}
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<chanPresent;i++) {
    //   if (pp.all==1 || cpar.chtype[i]==pp.all-1) {
    // 	pmap p2 = Plist[i*nfld+kk];
    // 	SetNum(p2,te->GetNumber());
    // 	TGNumberEntryField *te2 = (TGNumberEntryField*) p2.field;
    // 	te2->SetNumber(te->GetNumber());
    //   }
    // }
  }

  //cout << "long_bins: " << opt.long_bins << endl;
  
}

void ParDlg::DoNum_SetBuf() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  //cout << "DoNum: ";
  //cout << *(Int_t*) pp.data << " ";
  //cout << pp.data << " " << opt.bkg1[0] << " ";
  //cout << (Int_t) pp.all << endl;

  SetNum(pp,te->GetNumber());

  if (crs->module && !crs->b_acq) {
    crs->Free_Transfer();
    gSystem->Sleep(50);
    crs->Init_Transfer();
  }

}

void ParDlg::SetChk(pmap pp, Bool_t num) {
  if (pp.type==p_chk) {
      *(Bool_t*) pp.data = (Bool_t) num;
  }
  else {
    cout << "(DoChk) Wrong type: " << (int) pp.type << endl;
  }
}

void ParDlg::DoChk() {

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();
  pmap pp = Plist[id-1];

  /*
  cout << "DoChk: ";
  cout << id << " ";
  cout << *(Bool_t*) pp.data << " ";
  cout << (Int_t) pp.type << " ";
  cout << te->GetState() << endl;
  */

  SetChk(pp,(Bool_t)te->GetState());

  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<chanPresent;i++) {
	if (pp.all==1 || cpar.chtype[i]==pp.all-1) {
	  pmap p2 = Plist[i*nfld+kk];
	  SetChk(p2,te->GetState());
	  TGCheckButton *te2 = (TGCheckButton*) p2.field;
	  te2->SetState(te->GetState());      
	}
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<chanPresent;i++) {
    //   if (pp.all==1 || cpar.chtype[i]==pp.all-1) {
    // 	pmap p2 = Plist[i*nfld+kk];
    // 	SetChk(p2,te->GetState());
    // 	TGCheckButton *te2 = (TGCheckButton*) p2.field;
    // 	te2->SetState(te->GetState());      
    //   }
    // }
  }

}

void ParDlg::DoChkWrite() {

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();

  DoChk();

  //cout << "chkwrite: " << Plist.size() << " " << id << endl;

  //return;

  pmap pp;
  pp = Plist[id+1];
  TGTextButton *but = (TGTextButton*) pp.field;
  pp = Plist[id+2];
  TGTextEntry *te2 = (TGTextEntry*) pp.field;

  Bool_t state = (Bool_t) te->GetState();      

  if (state) {
    but->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
    te2->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
  }
  else {
    but->ChangeBackground(gROOT->GetColor(18)->GetPixel());
    te2->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
  }

  //te2->SetEnabled(state);
  //but->SetEnabled(state);

}

void ParDlg::DoOpen() {

  const char *dnd_types[] = {
    "all files",     "*",
    ".gz files",     "*.gz",
    0,               0
  };

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {
    pmap pp = Plist[id];

    //cout << "DoOpen: " << id << " " << fi.fFilename << endl;
    //cout << "DoOpen: " << fi.fIniDir << endl;
     
    SetTxt(pp,fi.fFilename);

    TGTextEntry *te2 = (TGTextEntry*) pp.field;
    te2->SetText(fi.fFilename);      

  }

}

void ParDlg::SetCombo(pmap pp, Int_t num) {
  if (pp.type==p_cmb) {
    *(Int_t*) pp.data = num;
    
    //cout << "DoCombo: " << num << endl;
  }
  else {
    cout << "(DoCombo) Wrong type: " << (int)pp.type << endl;
  }
}

void ParDlg::DoCombo() {

  TGComboBox *te = (TGComboBox*) gTQSender;
  Int_t id = te->WidgetId();

  int sel = te->GetSelected();

  pmap pp = Plist[id-1];

  int nline = id/nfld;

  // cout << "DoCombo: " << id << " " << nline << " " << (int) pp.all 
  //      << " " << nfld
  //      << " " << sel << " " << (chanPresent+sel)*nfld
  //      << endl;
  // cout << this << " " << crspar << " " << chanpar << endl;

  SetCombo(pp,sel);

  crspar->CopyParLine(sel,nline);
  chanpar->CopyParLine(sel,nline);

  // if (sel<ADDCH) {
  //   for (int j=1;j<nfld;j++) {
  //     CopyField((chanPresent+sel)*nfld+j,id-1+j);
  //   }
  // }
  
  if (pp.all==1) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<chanPresent;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetCombo(p2,te->GetSelected());
	TGComboBox *te2 = (TGComboBox*) p2.field;
	te2->Select(te->GetSelected(),false);

	crspar->CopyParLine(sel,i);
	chanpar->CopyParLine(sel,i);

	// if (sel<ADDCH) {
	//   for (int j=1;j<nfld;j++) {
	//     CopyField((chanPresent+sel)*nfld+j,i*nfld+kk+j);
	//   }
	// }
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<chanPresent;i++) {
    //   pmap p2 = Plist[i*nfld+kk];
    //   SetCombo(p2,te->GetSelected());
    //   TGComboBox *te2 = (TGComboBox*) p2.field;
    //   te2->Select(te->GetSelected(),false);

    //   if (sel<ADDCH) {
    // 	for (int j=1;j<nfld;j++) {
    // 	  CopyField((chanPresent+sel)*nfld+j,i*nfld+kk+j);
    // 	}
    //   }
    // }
  }

  //cout << "DoCombo chtype: " << cpar.chtype[0] << " " << id << endl;

}

void ParDlg::SetTxt(pmap pp, const char* txt) {
  if (pp.type==p_txt) {
    strcpy((char*) pp.data,txt);
  }
  else {
    cout << "(DoTxt) Wrong type: " << (int)pp.type << endl;
  }
}

void ParDlg::DoTxt() {

  TGTextEntry *te = (TGTextEntry*) gTQSender;
  Int_t id = te->WidgetId();

  //int sel = te->GetSelected();

  pmap pp = Plist[id-1];

  //cout << "DoTxt: " << id << " " << kk << " " << sel << endl;
  
  SetTxt(pp,te->GetText());

  if (pp.all==1) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<chanPresent;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetTxt(p2,te->GetText());
	TGTextEntry *te2 = (TGTextEntry*) p2.field;
	te2->SetText(te->GetText());      
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<chanPresent;i++) {
    //   pmap p2 = Plist[i*nfld+kk];
    //   SetTxt(p2,te->GetText());
    //   TGTextEntry *te2 = (TGTextEntry*) p2.field;
    //   te2->SetText(te->GetText());      
    // }
  }

}

void ParDlg::CopyParLine(int sel, int line) {
  if (sel<ADDCH) {
    for (int j=1;j<nfld;j++) {
      CopyField((chanPresent+sel)*nfld+j,line*nfld+j);
    }
  }
}
 
void ParDlg::CopyField(int from, int to) {

  pmap* p1 = &Plist[from];
  pmap* p2 = &Plist[to];

  if (p1->type!=p2->type) {
    cout << "CopyField bad type: "
	 << (int) p1->type << " " << (int) p2->type << endl; 
  }
  
  switch (p1->type) {
  case p_inum: {
    TGNumberEntryField *te = (TGNumberEntryField*) p2->field;
    te->SetNumber(*(Int_t*) p1->data);
    *(Int_t*) p2->data = *(Int_t*) p1->data; }
    break;
  case p_fnum: {
    TGNumberEntryField *te = (TGNumberEntryField*) p2->field;
    te->SetNumber(*(Float_t*) p1->data);
    *(Float_t*) p2->data = *(Float_t*) p1->data; }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) p2->field;
    Bool_t bb = *(Bool_t*) p1->data;
    te->SetState((EButtonState) bb);
    *(Bool_t*) p2->data = *(Bool_t*) p1->data; }
    break;
  default:
    cout << "unknown pp->type: " << p1->type << endl;
  } //switch  
}

void ParDlg::UpdateField(int nn) {

  pmap* pp = &Plist[nn];
  
  TQObject* tq = (TQObject*) pp->field;
  tq->BlockAllSignals(true);

  switch (pp->type) {
    case p_inum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      //te->BlockSignals(true);
      te->SetNumber(*(Int_t*) pp->data);
      //te->BlockSignals(false);
    }
      break;
    case p_fnum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      //te->BlockSignals(true);
      te->SetNumber(*(Float_t*) pp->data);
      //te->BlockSignals(false);
    }
      break;
    case p_chk: {
      TGCheckButton *te = (TGCheckButton*) pp->field;
      //te->BlockSignals(true);
      Bool_t bb = *(Bool_t*) pp->data;
      te->SetState((EButtonState) bb);
      TString str = TString(te->GetName());
      if (str.Contains("write",TString::kIgnoreCase)) {
	TGTextButton *but = (TGTextButton*) (pp+2)->field;
	TGTextEntry *te2 = (TGTextEntry*) (pp+3)->field;	

	if (bb) {
	  but->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
	  te2->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
	}
	else {
	  but->ChangeBackground(gROOT->GetColor(18)->GetPixel());
	  te2->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
	}
	//te2->SetEnabled(bb);
	//but->SetEnabled(bb);
      }
      //te->BlockSignals(false);
    }
      break;
    case p_cmb: {
      TGComboBox *te = (TGComboBox*) pp->field;
      //te->BlockSignals(true);
      //cout << "Combo2: " << nn << " " << *(ChDef*) pp->data << endl;
      te->Select(*(ChDef*) pp->data,false);
      //te->BlockSignals(false);
    }
      break;
    case p_txt: {
      TGTextEntry *te = (TGTextEntry*) pp->field;
      //te->BlockSignals(true);
      te->SetText((char*) pp->data);
      //te->BlockSignals(false);
    }
      break;
    case p_open:
      break;
    default:
      cout << "unknown pp->type: " << pp->type << endl;
    } //switch

  tq->BlockAllSignals(false);

}

void ParDlg::Update() {

  //cout << "smooth39: " << cpar.smooth[39] << " " << Plist.size() << endl;
  //cout << "smooth1: " << cpar.smooth[0] << " " << Plist.size() << endl;
  for (UInt_t i=0;i<Plist.size();i++) {
    //cout << "smooth3: " << cpar.smooth[0] << " " << i << endl;
    UpdateField(i);
    //cout << "smooth4: " << cpar.smooth[0] << " " << i << endl;
  }
  //cout << "smooth2: " << cpar.smooth[0] << endl;
}

TGWidget *ParDlg::FindWidget(void* p) {
  //finds widget using address of asigned parameter
  for (std::vector<pmap>::iterator pp = Plist.begin();
       pp != Plist.end(); pp++) {
    if (pp->data == p) {
      return pp->field;
    }
  }
  return 0;
}


//------ ParParDlg -------

ParParDlg::ParParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
  k_int=TGNumberFormat::kNESInteger;
  k_r0=TGNumberFormat::kNESReal;
  k_r1=TGNumberFormat::kNESRealOne;
  k_r2=TGNumberFormat::kNESRealTwo;
  k_r3=TGNumberFormat::kNESRealThree;

  //TGNumberFormat::EAttribute k_nneg=TGNumberFormat::kNEANonNegative;
  //TGNumberFormat::EAttribute k_any=TGNumberFormat::kNEAAnyNumber;


  fCanvas = new TGCanvas(this,w,h);
  AddFrame(fCanvas,fLexp);

  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);

  //opt.chk_raw=true;
  //opt.chk_dec=false;
  AddWrite("Write raw data",&opt.raw_write,&opt.raw_compr,opt.fname_raw);

  int id;
  char txt[99];
  sprintf(txt,"Decode");
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe1,fL1);
  id = Plist.size()+1;
  TGCheckButton *fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.decode,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  AddWrite("Write decoded data",&opt.dec_write,&opt.dec_compr,opt.fname_dec);

  hor = new TGSplitFrame(fcont1,10,10);
  fcont1->AddFrame(hor,fLexp);
  hor->VSplit(380);
  ver1 = hor->GetFirst();
  ver2 = hor->GetSecond();

  AddOpt(ver1);
  AddAna(ver1);
  AddHist(ver2);

}

void ParParDlg::AddWrite(const char* txt, Bool_t* opt_chk, Int_t* compr,
			 char* opt_fname) {
  int id;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe1,fL1);

  id = Plist.size()+1;
  TGCheckButton *fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,opt_chk,p_chk,0);
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hframe1, *compr, 2, id, k_int, 
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   0,9);
  hframe1->AddFrame(fNum1,fL3);
  DoMap(fNum1->GetNumberEntry(),compr,p_inum,0);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoNum()");

  TGLabel* fLabel = new TGLabel(hframe1, "compr.level");
  hframe1->AddFrame(fLabel,fL3);


  id = Plist.size()+1;
  TGTextButton *fbut = new TGTextButton(hframe1,"Select...",id);
  hframe1->AddFrame(fbut, fL3);
  DoMap(fbut,opt_fname,p_open,0);
  fbut->Connect("Clicked()", "ParDlg", this, "DoOpen()");

  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe2,fL1);

  //strcpy(opt.fname_raw,"raw32.gz");
  id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(hframe2,(char*)opt_fname, id);;
  tt->SetWidth(590);
  //tt->SetState(false);
  hframe2->AddFrame(tt,fL0);
  DoMap(tt,opt_fname,p_txt,0);
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");
}

void ParParDlg::AddOpt(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Analysis start (in sec)";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine2(fF6,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_int);

  tip1= "Delay between drawing events (in msec)";
  tip2= "";
  label="DrawEvent delay";
  AddLine2(fF6,ww,NULL,&opt.tsleep,tip1,tip2,label,k_int,100,10000);

  tip1= "Size of the USB buffer in kilobytes";
  tip2= "";
  label="USB buffer size";
  AddLine2(fF6,ww,NULL,&opt.buf_size,tip1,tip2,label,k_int,1,2048,
	   (char*) "DoNum_SetBuf()");

  tip1= "Minimal size of the event list";
  tip2= "Maximal size of the event list";
  label="Event_list size";
  AddLine2(fF6,ww,&opt.ev_min,&opt.ev_max,tip1,tip2,label,k_int);


  fF6->Resize();

}

void ParParDlg::AddAna(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Analysis", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Coincidence window for making events";
  tip2= "Period of digitizer";
  label="Coincidence, period (ns)";
  AddLine2(fF6,ww,&opt.tgate,&opt.period,tip1,tip2,label,k_int);

  tip1= "Minimal multiplicity";
  tip2= "Maximal multiplicity";
  label="Multiplicity (min, max)";
  AddLine2(fF6,ww,&opt.mult1,&opt.mult2,tip1,tip2,label,k_int,1,MAX_CH);

  fF6->Resize();

}

void ParParDlg::AddHist(TGCompositeFrame* frame2) {
  
  //int ww=70;
  
  //TGLabel* fLabel = new TGLabel(frame, "---  Histograms  ---");
  //frame->AddFrame(fLabel, fL5);


  TGGroupFrame* frame = new TGGroupFrame(frame2, "Histograms", kVerticalFrame);
  frame->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame, fL6);

   // 2 column, n rows
   //frame->SetLayoutManager(new TGMatrixLayout(frame, 0, 3, 7));

  tip1= "Total qcuisition time";
  label="Time";
  AddLine3(frame,&opt.time_bins,&opt.time_min,&opt.time_max,tip1,label);

  tip1= "Time of flight (relative to the starts - see Channels->St)";
  label="TOF";
  AddLine3(frame,&opt.tof_bins,&opt.tof_min,&opt.tof_max,tip1,label);

  tip1= "Time of flight with multiplicity";
  label="M_TOF";
  AddLine3(frame,&opt.mtof_bins,&opt.mtof_min,&opt.mtof_max,tip1,label);

  tip1= "Amplitude or energy, calibrated (see Channels->EM for calibration)";
  label="Amplitude";
  AddLine3(frame,&opt.amp_bins,&opt.amp_min,&opt.amp_max,tip1,label);

  tip1= "Pulse height (in channels)";
  label="Height";
  AddLine3(frame,&opt.hei_bins,&opt.hei_min,&opt.hei_max,tip1,label);

  /*
  tip1= "Bins per channel for Width";
  tip2= "Length of Width (in channels)";
  label="Width";
  AddLine2(frame,ww,&opt.rms_bins,&opt.rms_max,tip1,tip2,label,k_r0);

  tip1= "Bins per nanosecond for TOF";
  tip2= "Length of TOF (in nanoseconds)";
  label="TOF";
  AddLine2(frame,ww,&opt.tof_bins,&opt.tof_max,tip1,tip2,label,k_r0);
  */

}

/*
void ParParDlg::AddLine2(TGCompositeFrame* frame, int width, void *x1, void *x2, 
		      const char* tip1, const char* tip2, const char* label,
		      TGNumberFormat::EStyle style, 
			 //TGNumberFormat::EAttribute attr, 
		      double min, double max)
{
  //double zz;
  int id;

  TGHorizontalFrame* fHor = new TGHorizontalFrame(frame, 10, 10);
  frame->AddFrame(fHor, fL1);

  TGNumberFormat::ELimit limits = TGNumberFormat::kNELNoLimits;
  if (max!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(fHor, 0, 0, id, style, 
					   TGNumberFormat::kNEAAnyNumber,
			     limits,min,max);
  DoMap(fNum1->GetNumberEntry(),x1,p_fnum,0);
  fNum1->GetNumberEntry()->SetToolTipText(tip1);
  fNum1->SetWidth(width);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, "DoNum()");
  fHor->AddFrame(fNum1, fL3);

  if (x2!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum2 = new TGNumberEntry(fHor, 0, 0, id, style, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min,max);
    DoMap(fNum2->GetNumberEntry(),x2,p_fnum,0);
    fNum2->GetNumberEntry()->SetToolTipText(tip2);
    fNum2->SetWidth(width);
    fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, "DoNum()");
    fHor->AddFrame(fNum2, fL1);
  }
  else {
    TGLabel* fskip = new TGLabel(fHor, "");
    fskip->SetWidth(width);
    fHor->AddFrame(fskip, fL3);
  }
  
  TGLabel* fLabel = new TGLabel(fHor, label);
  fHor->AddFrame(fLabel, fL3);

}
*/

void ParParDlg::AddLine2(TGGroupFrame* frame, int width, void *x1, void *x2, 
		      const char* tip1, const char* tip2, const char* label,
		      TGNumberFormat::EStyle style, 
			 //TGNumberFormat::EAttribute attr, 
			 double min, double max, char* connect)
{

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  if (connect==NULL) {
    connect = (char*) "DoNum()";
  }

  //double zz;
  int id;
  P_Def pdef;

  if (style==k_int) {
    pdef=p_inum;
  }
  else {
    pdef=p_fnum;
  }

  TGNumberFormat::ELimit limits = TGNumberFormat::kNELNoLimits;
  if (max!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  if (x1!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, style, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min,max);
    DoMap(fNum1->GetNumberEntry(),x1,pdef,0);
    fNum1->GetNumberEntry()->SetToolTipText(tip1);
    fNum1->SetWidth(width);
    //fNum1->Resize(width, fNum1->GetDefaultHeight());
    fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, connect);
    hfr1->AddFrame(fNum1,fL7);
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,fL7);
  }
  
  if (x2!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum2 = new TGNumberEntry(hfr1, 0, 0, id, style, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min,max);
    DoMap(fNum2->GetNumberEntry(),x2,pdef,0);
    fNum2->GetNumberEntry()->SetToolTipText(tip2);
    fNum2->SetWidth(width);
    //fNum2->Resize(width, fNum2->GetDefaultHeight());
    fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, connect);
    hfr1->AddFrame(fNum2,fL7);
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,fL7);
  }

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,fL7);

}

void ParParDlg::AddLine3(TGGroupFrame* frame, Float_t *x1,
			 Float_t *x2, Float_t *x3, 
			 const char* tip, const char* label)
{
  double ww=70;
  
  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  //double zz;
  int id;

  TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //nbins
  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),x1,p_fnum,0);
  fNum1->GetNumberEntry()->SetToolTipText("Number of Bins");
  fNum1->SetWidth(ww);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum1,fL8);

  //xlow
  id = Plist.size()+1;
  TGNumberEntry* fNum2 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum2->GetNumberEntry(),x2,p_fnum,0);
  fNum2->GetNumberEntry()->SetToolTipText("Low edge");
  fNum2->SetWidth(ww);
  fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum2,fL8);

  //xup
  id = Plist.size()+1;
  TGNumberEntry* fNum3 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum3->GetNumberEntry(),x3,p_fnum,0);
  fNum3->GetNumberEntry()->SetToolTipText("Upper edge");
  fNum3->SetWidth(ww);
  fNum3->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum3,fL8);


  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,fL8);

}

/*
void ParParDlg::Update() {

  for (std::vector<pmap>::iterator pp = Plist.begin();
       pp != Plist.end(); ++pp) {
    //cout << (int) pp->type << endl;
    switch (pp->type) {
    case p_inum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      te->SetNumber(*(Int_t*) pp->data);
    }
      break;
    case p_fnum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      te->SetNumber(*(Float_t*) pp->data);
    }
      break;
    case p_chk: {
      TGCheckButton *te = (TGCheckButton*) pp->field;
      Bool_t bb = *(Bool_t*) pp->data;
      te->SetState((EButtonState) bb);
      TString str = TString(te->GetName());
      if (str.Contains("write",TString::kIgnoreCase)) {
	TGTextButton *but = (TGTextButton*) (pp+1)->field;
	TGTextEntry *te2 = (TGTextEntry*) (pp+2)->field;	
	te2->SetEnabled(bb);
	but->SetEnabled(bb);
      }
    }
      break;
    case p_cmb: {
      TGComboBox *te = (TGComboBox*) pp->field;
      te->Select(*(ChDef*) pp->data,false);
    }
      break;
    case p_txt: {
      TGTextEntry *te = (TGTextEntry*) pp->field;
      te->SetText((char*) pp->data);
    }
      break;
    case p_open:
      break;
    default:
      cout << "unknown pp->type: " << pp->type << endl;
    } //switch
  } //for

}
*/
//------ ChanParDlg -------

ChanParDlg::ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
  //AddHeader();
  head_frame = new TGHorizontalFrame(this,10,10);
  AddFrame(head_frame,fL1);

  // Hsplitter
  TGVerticalFrame *vFrame = new TGVerticalFrame(this, 10, 10);
  AddFrame(vFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));
  TGHorizontalFrame *fH1 = new TGHorizontalFrame(vFrame, 10, 320, kFixedHeight);
  TGHorizontalFrame *fH2 = new TGHorizontalFrame(vFrame, 10, 10);
  vFrame->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  TGHSplitter *hsplitter = new TGHSplitter(vFrame,2,2);
  hsplitter->SetFrame(fH1, kTRUE);
  vFrame->AddFrame(hsplitter, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  vFrame->AddFrame(fH2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));   




  fCanvas = new TGCanvas(fH1,10,10);
  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);
  fH1->AddFrame(fCanvas,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
                                          kLHintsExpandX, 0, 0, 1, 1));


  fCanvas2 = new TGCanvas(fH2,10,10);
  fcont2 = new TGCompositeFrame(fCanvas2->GetViewPort(), 
  				100, 100, kVerticalFrame);
  fCanvas2->SetContainer(fcont2);
  fH2->AddFrame(fCanvas2,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
                                             kLHintsExpandX, 0, 0, 1, 1));

}

void ChanParDlg::Make_chanpar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  for (int i=0;i<chanPresent;i++) {
    AddLine1(i,fcont1);
  }

  AddLine1(MAX_CH,fcont2);

  for (int i=1;i<ADDCH;i++) {
    AddLine1(MAX_CH+i,fcont2);
  }


}

void ChanParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,fL1);

  TGTextEntry* tt[nchpar];

  for (int i=0;i<nchpar;i++) {
    tt[i]=new TGTextEntry(head_frame, tlab2[i]);
    tt[i]->SetWidth(tlen2[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(tip2[i]);
    tt[i]->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt[i],fL0);
  }

}

void ChanParDlg::AddLine1(int i, TGCompositeFrame* fcont1) {
  char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe1,fL1);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //hframe1->ChangeBackground(yellow);

  if (i<MAX_CH)
    sprintf(txt,"%2d  ",i);
  else if (i==MAX_CH) {
    sprintf(txt,"all");
    all=1;
  }
  else {
    sprintf(txt," ");
    all=i-MAX_CH+1;
  }

  TGTextEntry* lab=new TGTextEntry(hframe1, txt);
  lab->SetWidth(tlen2[0]);
  lab->SetState(false);
  hframe1->AddFrame(lab,fL0);
  kk++;

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
    //cout << "nfld: " << Plist.size() << " " << nfld << endl;
    //start=false;
  }

  id = Plist.size()+1;
  TGComboBox* fCombo=new TGComboBox(hframe1,id);
  hframe1->AddFrame(fCombo,fL0);
  kk++;

  for (int j = 0; j < ADDCH; j++) {
    fCombo->AddEntry(types[j], j+1);
  }

  fCombo->Resize(tlen2[1], 20);

  if (i==MAX_CH) {
    fCombo->AddEntry(types[ADDCH], ADDCH+1);
  }

  if (i<=MAX_CH)
    ;//fCombo->Select(1);
  else {
    fCombo->Select(i-MAX_CH,false);
    fCombo->SetEnabled(false);
  }

  DoMap(fCombo,&cpar.chtype[i],p_cmb,all,0,0);

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");


  id = Plist.size()+1;
  TGCheckButton *fst = new TGCheckButton(hframe1, "", id);
  DoMap(fst,&opt.Start[i],p_chk,all,0,0);

  fst->Connect("Clicked()", "ChanParDlg", this, "DoChk()");

  hframe1->AddFrame(fst,fL3);
  kk++;



  AddNum2(i,kk++,all,hframe1,&opt.nsmoo[i],0,99,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.bkg1[i],-999,999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.bkg2[i],-999,999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.peak1[i],-999,16500,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.peak2[i],-999,16500,p_inum);
  AddNum2(i,kk++,all,hframe1,&cpar.deadTime[i],0,9999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.pile[i],0,9999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.timing[i],0,1,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.twin1[i],-99,99,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.twin2[i],-99,99,p_inum);

  AddNum2(i,kk++,all,hframe1,&opt.emult[i],0,99999,p_fnum);
  AddNum2(i,kk++,all,hframe1,&opt.elim1[i],0,99999,p_fnum);
  AddNum2(i,kk++,all,hframe1,&opt.elim2[i],0,99999,p_fnum);
}

void ChanParDlg::AddNum1(int i, int kk, int all, TGHorizontalFrame *hframe1,
		       const char* name, void* apar) {  //const char* name) {

  int par, min, max;

  cpar.GetPar(name,crs->module,i,par,min,max);

  int id = Plist.size()+1;
  TGNumberEntryField* fNum =
    new TGNumberEntryField(hframe1, id, par, TGNumberFormat::kNESInteger,
			   TGNumberFormat::kNEAAnyNumber,
			   TGNumberFormat::kNELLimitMinMax,min,max);

  DoMap(fNum,apar,p_inum, all,kk-1,i);
  
  fNum->SetWidth(tlen[kk]);

  fNum->Connect("TextChanged(char*)", "ChanParDlg", this, "DoNum()");

  hframe1->AddFrame(fNum,fL0);

}

void ChanParDlg::AddNum2(int i, int kk, int all, TGHorizontalFrame *hframe1,
			void* apar, double min, double max, P_Def ptype) {

  //opt.GetPar(name,crs->module,i,par,min,max);

  int id = Plist.size()+1;

  TGNumberFormat::EStyle style;
  if (ptype==p_fnum) {
    style=TGNumberFormat::kNESReal;
  }
  else {
    style=TGNumberFormat::kNESInteger;
  }

  TGNumberEntryField* fNum =
    new TGNumberEntryField(hframe1, id, 0, style,
			   TGNumberFormat::kNEAAnyNumber,
			   TGNumberFormat::kNELLimitMinMax,min,max);

  DoMap(fNum,apar,ptype, all,0,0);
  fNum->SetWidth(tlen2[kk]);
  fNum->Connect("TextChanged(char*)", "ChanParDlg", this, "DoNum()");
  hframe1->AddFrame(fNum,fL0);

}

void ChanParDlg::DoMap(TGWidget *f, void *d, P_Def t, int all,
		      byte cmd, byte chan) {
  ParDlg::DoMap(f,d,t,all);
  Plist.back().cmd=cmd;
  Plist.back().chan=chan;
}

void ChanParDlg::DoNum() {
  ParDlg::DoNum();

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  if (pp.cmd && crs->b_acq) {
    crs->Command2(4,0,0,0);
    printf("cmd: %d %d %d\n",pp.cmd,pp.chan,*(Int_t*)pp.data);
    crs->Command_crs(pp.cmd,pp.chan,*(Int_t*)pp.data);
    crs->Command2(3,0,0,0);
  }

}

void ChanParDlg::DoChk() {
  ParDlg::DoChk();
}

//------ ChanParDlg -------

CrsParDlg::CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
}

void CrsParDlg::Make_crspar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  /*
  AddHeader1();

  // Hsplitter
  TGVerticalFrame *vFrame = new TGVerticalFrame(this, 10, 10);
  AddFrame(vFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));
  TGHorizontalFrame *fH1 = new TGHorizontalFrame(vFrame, 10, 320, kFixedHeight);
  TGHorizontalFrame *fH2 = new TGHorizontalFrame(vFrame, 10, 10);
  vFrame->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  TGHSplitter *hsplitter = new TGHSplitter(vFrame,2,2);
  hsplitter->SetFrame(fH1, kTRUE);
  vFrame->AddFrame(hsplitter, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  vFrame->AddFrame(fH2, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));   


  fCanvas = new TGCanvas(fH1,10,10);
  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);
  fH1->AddFrame(fCanvas,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
                                          kLHintsExpandX, 0, 0, 1, 1));

  fCanvas2 = new TGCanvas(fH2,10,10);
  fcont2 = new TGCompositeFrame(fCanvas2->GetViewPort(), 
  				100, 100, kVerticalFrame);
  fCanvas2->SetContainer(fcont2);
  fH2->AddFrame(fCanvas2,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
                                             kLHintsExpandX, 0, 0, 1, 1));

  */

  for (int i=0;i<chanPresent;i++) {
    AddLine0(i,fcont1);
    //cout << "crs: addLine1: " << Plist.size() << endl; 
  }

  AddLine0(MAX_CH,fcont2);

  for (int i=1;i<ADDCH;i++) {
    AddLine0(MAX_CH+i,fcont2);
    //cout << "crs2: addLine1: " << Plist.size() << endl; 
  }

  if (crs->module==2) {
    TGHorizontalFrame *hforce = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(hforce,fL1);

    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce, "", id);
    hforce->AddFrame(fforce,fL3);
    DoMap((TGWidget*)fforce,&cpar.forcewr,p_chk,0,0,0);
    fforce->Connect("Clicked()", "ParDlg", this, "DoChk()");
    
    TGLabel* lforce = new TGLabel(hforce, "  Force_write all channels");
    hforce->AddFrame(lforce,fL1);
  }

}

void CrsParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,fL1);

  TGTextEntry* tt[ncrspar];

  for (int i=0;i<ncrspar;i++) {
    tt[i]=new TGTextEntry(head_frame, tlab[i]);
    tt[i]->SetWidth(tlen[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(tip[i]);
    tt[i]->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt[i],fL0);
  }

}

void CrsParDlg::AddLine0(int i, TGCompositeFrame* fcont1) {
  char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe1,fL1);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //hframe1->ChangeBackground(yellow);

  if (i<MAX_CH)
    sprintf(txt,"%2d  ",i);
  else if (i==MAX_CH) {
    sprintf(txt,"all");
    all=1;
  }
  else {
    sprintf(txt," ");
    all=i-MAX_CH+1;
  }

  TGTextEntry* lab=new TGTextEntry(hframe1, txt);
  lab->SetWidth(tlen[0]);
  lab->SetState(false);
  hframe1->AddFrame(lab,fL0);
  kk++;

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
    //cout << "nfld: " << Plist.size() << " " << nfld << endl;
    //start=false;
  }

  id = Plist.size()+1;
  TGComboBox* fCombo=new TGComboBox(hframe1,id);
  hframe1->AddFrame(fCombo,fL0);
  kk++;

  for (int j = 0; j < ADDCH; j++) {
    fCombo->AddEntry(types[j], j+1);
  }

  fCombo->Resize(tlen[1], 20);

  if (i==MAX_CH) {
    fCombo->AddEntry(types[ADDCH], ADDCH+1);
  }

  if (i<=MAX_CH)
    ;//fCombo->Select(1);
  else {
    fCombo->Select(i-MAX_CH,false);
    fCombo->SetEnabled(false);
  }

  DoMap(fCombo,&cpar.chtype[i],p_cmb,all,0,0);

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");

  id = Plist.size()+1;
  TGCheckButton *f_en = new TGCheckButton(hframe1, "", id);
  TGCheckButton *f_inv = new TGCheckButton(hframe1, "", id+1);
  TGCheckButton *f_acdc = new TGCheckButton(hframe1, "", id+2);
  DoMap(f_en,&cpar.enabl[i],p_chk,all,1,i);
  DoMap(f_inv,&cpar.inv[i],p_chk,all,2,i);
  DoMap(f_acdc,&cpar.acdc[i],p_chk,all,3,i);

  f_en->Connect("Clicked()", "ChanParDlg", this, "DoChk()");
  f_inv->Connect("Clicked()", "ChanParDlg", this, "DoChk()");
  f_acdc->Connect("Clicked()", "ChanParDlg", this, "DoChk()");

  //f_inv->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_acdc->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_inv->SetWidth(25);
  //f_acdc->SetWidth(25);

  hframe1->AddFrame(f_en,fL3);
  hframe1->AddFrame(f_inv,fL3);
  hframe1->AddFrame(f_acdc,fL3);
  kk+=3;

  AddNum1(i,kk++,all,hframe1,"smooth",&cpar.smooth[i]);
  AddNum1(i,kk++,all,hframe1,"dt"    ,&cpar.deadTime[i]);
  AddNum1(i,kk++,all,hframe1,"pre"   ,&cpar.preWr[i]);
  AddNum1(i,kk++,all,hframe1,"len"   ,&cpar.durWr[i]);
  if (crs->module==2) 
    AddNum1(i,kk++,1,hframe1,"gain"  ,&cpar.adcGain[i]);
  else
    AddNum1(i,kk++,all,hframe1,"gain"  ,&cpar.adcGain[i]);
  AddNum1(i,kk++,all,hframe1,"deriv" ,&cpar.kderiv[i]);
  AddNum1(i,kk++,all,hframe1,"thresh",&cpar.threshold[i]);

  if (i<MAX_CH) {
    fStat[i] = new TGLabel(hframe1, "");
    fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat[i]->SetMargins(10,0,0,0);
    fStat[i]->SetTextJustify(kTextLeft|kTextCenterY);

    fStat[i]->Resize(70,21);
    int col=gROOT->GetColor(19)->GetPixel();
    fStat[i]->SetBackgroundColor(col);
    fStat[i]->SetText(0);
    //fbar[i]->SetParts(parts, nparts);
    //fBar2->SetParts(nparts);
    //fStat[i]->Draw3DCorner(kFALSE);
    //fbar[i]->DrawBorder();
    hframe1->AddFrame(fStat[i],fL8);
  }

}
