// #include "crspardlg.h"
// #include "libcrs.h"
// #include "libmana.h"
// #include "toptions.h"
#include "romana.h"

#include "TGFileDialog.h"
#include <TGSplitter.h>
#include <TG3DLine.h>
#include <TSystem.h>
//#include "romana.h"
#include <TColor.h>

//const char* t_raw = "Write raw data";

namespace CP {
  Float_t RGB[ADDCH][3] =
    {
      {1,1,0}, //0
      {0,1,1},
      {1,0.4,1},
      {0.4,0.4,1},
      {1,0.8,0.8}, //4
      {0,1,0},
      {1,0.7,0.4},
      {0.2,0.8,0.6},
      {1,1,1}//8
    };
}

extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern AnaParDlg *anapar;
extern PikParDlg *pikpar;

const int ncrspar=14;

const int tlen1[ncrspar+1]={26,60,24,25,24,21,45,40,40,25,25,36,45,75,80};
const char* tlab1[ncrspar+1]={"Ch","Type","on","Inv","AC","hS","Dt","Pre","Len","G","Trg","Drv","Thr","Pulse/sec","BadPulse"};
const char* ttip1[ncrspar+1]={
  "Channel number",
  "Channel type",
  "On/Off",
  "Inversion",
  "AC coupling",
  "Hardware smoothing",
  "Dead time - no new trigger on the current channel within dead time from the old trigger",
  "Number of samples before the trigger",
  "Total length of the pulse in samples",
  "Additional Gain",
  "Trigget type: 0 - pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative",
  "Parameter of derivative: S(i) - S(i-Drv). 0 means trigger on the signal.",
  "Trigger threshold",
  "Pulse rate",
  "Number of Bad pulses"
};

const int n_apar=14;
const int tlen2[n_apar]={26,60,24,24,24,25,32,40,35,35,20,42,42,42};
const char* tlab2[n_apar]={"Ch","Type","dsp","St","Mt","sS","Drv","Thr","dT","Pile","Tm","EM","Elim1","Elim2"};
const char* ttip2[n_apar]={
  "Channel number",
  "Channel type",
  "Use Digital Signal Processing (DSP) data instead of raw data",
  "Start channel - used for making TOF start\nif there are many start channels in the event, the earliest is used",
  "Marked channel - use this channel for making Mtof spectra",
  "Software smoothing",
  "Drv>0 - trigger on differential S(i) - S(i-Drv)",
  "Trigger threshold",
  "Dead-time window \nsubsequent peaks within this window are ignored",
  "Pileup window \nmultiple peaks within this window are marked as pileup",
  "Timing mode (in 1st derivative):\n0 - threshold crossing (Pos);\n1 - left minimum (T1);\n2 - right minimum;\n3 - maximum in 1st derivative",
  "Energy multiplier",
  "Low energy limit (not working yet)",
  "High energy limit (not working yet)"
};

const int n_ppar=10;
const int tlen3[n_ppar]={26,60,40,40,42,42,35,35,35,35};
const char* tlab3[n_ppar]={"Ch","Type","Base1","Base2","Peak1","Peak2","T1","T2","W1","W2"};
const char* ttip3[n_ppar]={
  "Channel number",
  "Channel type",
  "Baseline start, relative to peak Pos (negative)",
  "Baseline end, relative to peak Pos (negative), excluded",
  "Peak start, relative to peak Pos (usually negative)",
  "Peak end, relative to peak Pos (usually positive), excluded",
  "Timing window start, included (usually negative, if 99 - use T1)",
  "Timing window end, excluded (usually positive, if 99 - use T2)",
  "Width window start",
  "Width window end, excluded",
};

char ttip_g[NGRP][100];

int* tlen7;
char** ttip7;

const char* types[ADDCH+2]={"NaI","BGO","Si 1","Si 2","Stilb","Demon","HPGe",
			    "NIM","Other","Copy",""};

//-----------
const int nchtype=9;
Int_t combotype[nchtype];

extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;
//extern int chanPresent;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;

// TGLayoutHints* fL0;
// TGLayoutHints* fL1;
// TGLayoutHints* fL2;
// TGLayoutHints* fL3;
// TGLayoutHints* fL4;
// TGLayoutHints* fL5;
// TGLayoutHints* fL6;
// TGLayoutHints* fLexp;
TGLayoutHints* fL0 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
TGLayoutHints* fL0a = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 1, 1);
TGLayoutHints* fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
TGLayoutHints* fL2 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
TGLayoutHints* fL2a = new TGLayoutHints(kLHintsLeft | kLHintsBottom,0,0,5,0);
TGLayoutHints* fL3 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 4, 4, 0, 0);
TGLayoutHints* fL4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 5, 5);
TGLayoutHints* fL5 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 2, 2);
TGLayoutHints* fL6 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2, 2, 0, 0);
TGLayoutHints* fL7 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 11, 1, 1, 1);
TGLayoutHints* fL8 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 1, 1, 1);
TGLayoutHints* fL8a = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 1, 1, 0);
TGLayoutHints* fLexp = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  for (int i=0;i<nchtype;i++) {
    combotype[i]=i;
  }

  nfld=0;

  for (int i=0;i<ADDCH;i++) {
    Int_t cc=TColor::GetColor(TColor::RGB2Pixel(CP::RGB[i][0],CP::RGB[i][1],CP::RGB[i][2]));
    tcol[i]=gROOT->GetColor(cc)->GetPixel();
  }

}
ParDlg::~ParDlg() {
  cout << "~ParDlg: " << this << endl;
  //CleanUp();
}

void ParDlg::DoMap(TGWidget* f, void* d, P_Def t, int all, void* d2) {
  pmap pp;
  pp.field = (TGWidget*) f;
  pp.data = d;
  pp.data2= d2;
  pp.type = t;
  pp.all=all;
  //cout << "DoMap1: " << f << " " << d << " " << t << endl;
  Plist.push_back(pp);
  //cout << "DoMap2: " << f << " " << d << " " << t << endl;
}

void ParDlg::SetNum(pmap pp, Double_t num) {
  //cout << "setnum: " << pp.data2 << endl;
  if (pp.type==p_fnum) {
    *(Float_t*) pp.data = num;
    if (pp.data2) *(Float_t*) pp.data2 = num;
    //cout << "setpar1: " << *(Float_t*) pp.data << endl;
  }
  else if (pp.type==p_inum) {
    *(Int_t*) pp.data = num;
    if (pp.data2) *(Int_t*) pp.data2 = num;
    //cout << "setpar2: " << *(Int_t*) pp.data << endl;
  }
  else {
    cout << "(SetNum) Wrong type: " << pp.type << endl;
  }
}

void ParDlg::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  // cout << "DoNum: " << id << " ";
  // cout << *(Int_t*) pp.data << " ";
  // cout << pp.data << " " << opt.bkg1[0] << " ";
  // cout << (Int_t) pp.all << endl;

  //cout << "Donum: " << te->GetName() << endl;
  
  SetNum(pp,te->GetNumber());

  //return;
  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld;
      //cout << "donum: " << kk << " " << te->GetNumber() << endl;
      for (int i=0;i<MAX_CH;i++) {
	if (pp.all==1 || opt.chtype[i]==pp.all-1) {
	  pmap p2 = Plist[i*nfld+kk];
	  TGNumberEntryField *te2 = (TGNumberEntryField*) p2.field;
	  double min = te2->GetNumMin();
	  double max = te2->GetNumMax();
	  double fnum=te->GetNumber();
	  //cout << "donum: " << i << " " << min << " " << max << " " << fnum << endl;
	  if (fnum<min) fnum=min;
	  if (fnum>max) fnum=max;	  
	  te2->SetNumber(fnum);
	  SetNum(p2,fnum);
	}
      }
    }
  }

  //cout << "Donum2: " << te->GetName() << endl;
  //cout << "long_bins: " << opt.long_bins << endl;
  
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

  //te->SetState(kButtonDisabled);

  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<MAX_CH;i++) {
	if (pp.all==1 || opt.chtype[i]==pp.all-1) {
	  pmap p2 = Plist[i*nfld+kk];
	  SetChk(p2,te->GetState());
	  TGCheckButton *te2 = (TGCheckButton*) p2.field;
	  te2->SetState(te->GetState());
	  te2->Clicked();
	}
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<opt.Nchan;i++) {
    //   if (pp.all==1 || opt.chtype[i]==pp.all-1) {
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

  //te->SetState(kButtonDisabled);
  //te2->SetEnabled(state);
  //but->SetEnabled(state);

}

void ParDlg::DoOpen() {

  const char *dnd_types[] = {
    "all files",     "*",
    ".raw files",     "*.raw",
    ".dec files",     "*.dec",
    ".root files",     "*.root",
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

  if (nline < MAX_CH) {
    // cout << "DoCombo: " << id << " " << nline << " " << (int) pp.all 
    // 	 << " " << nfld << " " << *(Int_t*) pp.data
    // 	 << " " << sel << " " << (opt.Nchan+sel)*nfld
    // 	 << endl;
    // cout << this << " " << crspar << " " << anapar << endl;

    if (sel==ADDCH+1) {
      int old=*(Int_t*) pp.data;

      crspar->CopyParLine(-old,nline);
      anapar->CopyParLine(-old,nline);
      pikpar->CopyParLine(-old,nline);


      te->Select(old);
      return;
    }

    SetCombo(pp,sel);
    crspar->CopyParLine(sel,nline);
    anapar->CopyParLine(sel,nline);
    pikpar->CopyParLine(sel,nline);

  }

  if (pp.all==1) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<MAX_CH;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetCombo(p2,te->GetSelected());
	TGComboBox *te2 = (TGComboBox*) p2.field;
	te2->Select(te->GetSelected(),false);

	crspar->CopyParLine(sel,i);
	anapar->CopyParLine(sel,i);
	pikpar->CopyParLine(sel,i);

	// if (sel<ADDCH) {
	//   for (int j=1;j<nfld;j++) {
	//     CopyField((opt.Nchan+sel)*nfld+j,i*nfld+kk+j);
	//   }
	// }
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<opt.Nchan;i++) {
    //   pmap p2 = Plist[i*nfld+kk];
    //   SetCombo(p2,te->GetSelected());
    //   TGComboBox *te2 = (TGComboBox*) p2.field;
    //   te2->Select(te->GetSelected(),false);

    //   if (sel<ADDCH) {
    // 	for (int j=1;j<nfld;j++) {
    // 	  CopyField((opt.Nchan+sel)*nfld+j,i*nfld+kk+j);
    // 	}
    //   }
    // }
  }

  //cout << "DoCombo chtype: " << opt.chtype[0] << " " << id << endl;

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

  const char *r_ext[] = {".raw",".dec",".root"};
  //const int len[] = {4,4,5};
  string dir, name, ext;

  TGTextEntry *te = (TGTextEntry*) gTQSender;
  Int_t id = te->WidgetId();
  Int_t i2=-1;
  for (int i=0;i<3;i++) {
    if (id==id_write[i]) {
      i2=i;
      break;
    }
  }

  //int sel = te->GetSelected();

  pmap pp = Plist[id-1];

  //cout << "DoTxt: " << id << " " << i2 << " " << te->GetText() << endl;
  //cout << r_ext[i2] << endl;

  if (i2>=0) {
    string ss(te->GetText());
    SplitFilename (ss,dir,name,ext);

    if (!TString(ext).EqualTo(r_ext[i2],TString::kIgnoreCase)) {
      ss=dir;
      ss.append(name);
      ss.append(r_ext[i2]);
    }

    int pos = te->GetCursorPosition();
    te->SetText(ss.c_str(),false);
    //te->SetCursorPosition(ss.length()-len[i2]);
    te->SetCursorPosition(pos);
    //cout << ss.length() << endl;
  }

  SetTxt(pp,te->GetText());

  if (pp.all==1) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<MAX_CH;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetTxt(p2,te->GetText());
	TGTextEntry *te2 = (TGTextEntry*) p2.field;
	te2->SetText(te->GetText());      
      }
    }
    // int kk;
    // (nfld ? (kk=(id-1)%nfld) : (kk=0));
    // for (int i=0;i<opt.Nchan;i++) {
    //   pmap p2 = Plist[i*nfld+kk];
    //   SetTxt(p2,te->GetText());
    //   TGTextEntry *te2 = (TGTextEntry*) p2.field;
    //   te2->SetText(te->GetText());      
    // }
  }

}

void ParDlg::CopyParLine(int sel, int line) {
  if (sel<0) { //inverse copy - from current ch to group
    //cout << "CopyParLine: " << line << " " << MAX_CH-sel << endl;
    //return;
    for (int j=1;j<nfld;j++) {
      CopyField(line*nfld+j,(MAX_CH-sel)*nfld+j);
    }
  }
  else if (sel<ADDCH) { //normal copy from group to current ch
    for (int j=1;j<nfld;j++) {
      CopyField((MAX_CH+sel)*nfld+j,line*nfld+j);
    }
    clab[line]->ChangeBackground(tcol[sel-1]);
    cframe[line]->ChangeBackground(tcol[sel-1]);
  }
  else if (sel==ADDCH) { //other - just change color
    clab[line]->ChangeBackground(tcol[sel-1]);
    cframe[line]->ChangeBackground(tcol[sel-1]);
  }
}
 
void ParDlg::CopyField(int from, int to) {

  pmap* p1 = &Plist[from];
  pmap* p2 = &Plist[to];

  //cout << "copyfield: " << p1 << " " << p2 << endl;
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
  //cout << "copyfield2: " << p1 << " " << p2 << endl;
}

void ParDlg::UpdateField(int nn) {

  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;
  pmap* pp = &Plist[nn];
  
  TQObject* tq = (TQObject*) pp->field;
  tq->BlockAllSignals(true);

  //cout << "updatefield0: " << nn << endl;
  //cout << "updatefield: " << nn << " " << pp->type << " " << p_chk << " " << p_cmb << " " << gROOT << endl;

  switch (pp->type) {
    case p_inum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      Int_t *dat = (Int_t*) pp->data;
      if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      	// cout << "IUpdMax: " << te->WidgetId() << " " << te->GetNumLimits()
	//      << " " << lim
	//      << " " << te->GetNumMax() << " " << *dat << endl;
      	*dat = te->GetNumMax();
      }
      if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      	//cout << "IUpdMin: " << te->GetNumMin() << " " << *dat << endl;
      	*dat = te->GetNumMin();
      }
      te->SetNumber(*dat);
    }
      break;
    case p_fnum: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      Float_t *dat = (Float_t*) pp->data;
      if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      	cout << "FUpdMax: " << te->GetNumMax() << " " << *dat << endl;
      	*dat = te->GetNumMax();
      }
      if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      	//cout << "FUpdMin: " << te->GetNumMin() << " " << *dat << endl;
      	*dat = te->GetNumMin();
      }
      te->SetNumber(*dat);

      if (TString(te->GetName()).EqualTo("Tstop",TString::kIgnoreCase)) {
	//bool ww = 
	//cout << "Upd: " << te->GetName() << " " << nn << endl;
	if (opt.Tstop) {
	  te->ChangeBackground(gROOT->GetColor(kYellow)->GetPixel());
	}
	else {
	  te->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
	}
      }
      //te->SetNumber(*(Float_t*) pp->data);
    }
      break;
    case p_chk: {
      TGCheckButton *te = (TGCheckButton*) pp->field;
      Bool_t bb = *(Bool_t*) pp->data;
      EButtonState st = te->GetState();
      te->SetState((EButtonState) bb);
      if (st==kButtonDisabled) {
	te->SetEnabled(false);
      }

      TString str = TString(te->GetName());
      if (str.Contains("write",TString::kIgnoreCase)) {
	TGTextButton *but = (TGTextButton*) (pp+2)->field;
	TGTextEntry *te2 = (TGTextEntry*) (pp+3)->field;	

	//cout << "updatefield2: " << te << " " << str << " " << but << " " << te2 << endl;

	if (bb) {
	  //cout << "updatefield4: " << te << " " << str << endl;
	  but->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
	  te2->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
	}
	else {
	  //cout << "updatefield5: " << te << " " << str << " " << gROOT->GetColor(18) << endl;
	  but->ChangeBackground(gROOT->GetColor(18)->GetPixel());
	  //cout << "updatefield6: " << te << " " << str << endl;
	  te2->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
	  //cout << "updatefield7: " << te << " " << str << endl;
	}
      }
      if (str.Contains("b_hist",TString::kIgnoreCase)) {
	for (int i=0;i<3;i++) {
	  pp = &Plist[nn+i+1];
	  //TGWidget* wg = (TGWidget*) pp->field;
	  TGNumberEntryField *te2 = (TGNumberEntryField*) pp->field;
	  //cout << "upd: " << i << " " << te2->GetName() << " " << st << endl;
	  te2->SetState(bb);
	}
      }
    }
      break;
    case p_cmb: {
      //cout << "cmb1: " << endl;
      TGComboBox *te = (TGComboBox*) pp->field;
      int line = nn/nfld;
      int sel = *(ChDef*) pp->data;
      //cout << "cmb2: " << sel << " " << line << endl;
      if (line<MAX_CH) {
	clab[line]->ChangeBackground(tcol[sel-1]);
	cframe[line]->ChangeBackground(tcol[sel-1]);
      }
      //cout << "cmb3: " << tcol[sel-1] << endl;
      te->Select(sel,false);
      //cout << "cmb4: " << tcol[sel-1] << endl;
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

  tq->BlockAllSignals(false);

}

void ParDlg::Update() {
  //cout << "update1: " << Plist.size() << endl;
  for (UInt_t i=0;i<Plist.size();i++) {
    UpdateField(i);
  }
  //cout << "update2: " << endl;
}

void ParDlg::EnableField(int nn, bool state) {

  pmap* pp = &Plist[nn];
  
  //TQObject* tq = (TQObject*) pp->field;
  //tq->BlockAllSignals(true);

  switch (pp->type) {
  case p_inum:
  case p_fnum:
  case p_txt: {
    TGTextEntry *te = (TGTextEntry*) pp->field;
    //cout << "enable1: " << te->GetName() << " " << te->GetTitle() << endl;
    te->SetEnabled(state);
  }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) pp->field;
    //cout << "enable2: " << te->GetName() << " " << te->GetTitle() << endl;
    // if (state)
    //   te->SetState(kButtonUp);
    // else
    te->SetEnabled(state);
    //cout << "p_chk: " << te->GetState() << endl;
  }
    break;
  case p_cmb: {
    TGComboBox *te = (TGComboBox*) pp->field;
    //cout << "enable3: " << te->GetName() << " " << te->GetTitle() << endl;
    te->SetEnabled(state);
  }
    break;
  default: ;
  } //switch

  //tq->BlockAllSignals(false);

}

void ParDlg::AllEnabled(bool state) {

  for (UInt_t i=0;i<Plist.size();i++) {
    // pmap* pp = &Plist[i];
    // TGFrame *te = (TGFrame*) pp->field;
    // cout << te << endl;
    // if (te)
    //   cout << "enable: " << te->GetName() << endl;
    EnableField(i,state);
  }
}

void ParDlg::SelectEnabled(bool state, const char* text) {

  for (UInt_t i=0;i<Plist.size();i++) {

     pmap* pp = &Plist[i];
     switch (pp->type) {
     case p_inum:
     case p_fnum:
     case p_txt: {
       TGTextEntry *te = (TGTextEntry*) pp->field;
       //cout << "enable3: " << te->GetName() << " " << te->GetTitle() << endl;
       if (TString(te->GetName()).Contains(text)) {
	 //cout << "enable1: " << te->GetName() << " " << te->GetTitle() << endl;
	 te->SetEnabled(state);
       }
     }
       break;
     default: ;
     } //switch
  }
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

void ParDlg::Rebuild() {
  for (int i=0;i<MAX_CH;i++) {
    if (i<opt.Nchan) {
      fcont1->ShowFrame(cframe[i]);
    }
    else {
      fcont1->HideFrame(cframe[i]);
    }
  }
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

  id_usb=-1;
  //id_tstop=-1;

  fCanvas = new TGCanvas(this,w,h);
  AddFrame(fCanvas,fLexp);

  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);

  /*
  //opt.chk_raw=true;
  //opt.chk_dec=false;
  AddWrite("Write raw data",&opt.raw_write,&opt.raw_compr,opt.fname_raw);
  id_write[0]=Plist.size();
  //cout << "raw: " << Plist.size()+1 << endl;

  TGCheckButton *fchk;
  int id;
  char txt[99];
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe1,fL1);

  id = Plist.size()+1;
  sprintf(txt,"Decode");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.decode,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  id = Plist.size()+1;
  sprintf(txt,"Analyze");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.analyze,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  AddWrite("Write decoded data",&opt.dec_write,&opt.dec_compr,opt.fname_dec);
  id_write[1]=Plist.size();
  //cout << "dec: " << Plist.size()+1 << endl;

  AddWrite("Write root histograms",&opt.root_write,&opt.root_compr,opt.fname_root);
  id_write[2]=Plist.size();
  //cout << "root: " << Plist.size()+1 << endl;
  */

  hor = new TGSplitFrame(fcont1,10,10);
  fcont1->AddFrame(hor,fLexp);
  hor->VSplit(420);
  ver1 = hor->GetFirst();
  ver2 = hor->GetSecond();

  //ver1->ChangeOptions(ver1->GetOptions()|kFixedWidth);

  AddFiles(ver1);
  AddOpt(ver1);
  AddLogic(ver1);
  AddAna(ver1);
  AddHist(ver2);

}

void ParParDlg::AddWrite(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
			 Int_t* compr, char* opt_fname) {
  int id;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(frame,10,10);
  frame->AddFrame(hframe1,fL1);

  id = Plist.size()+1;
  TGCheckButton *fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,opt_chk,p_chk,0);
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  //fchk->SetState(kButtonDown,false);

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

  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(frame,10,10);
  frame->AddFrame(hframe2,fL1);

  //strcpy(opt.fname_raw,"raw32.gz");
  id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(hframe2,(char*)opt_fname, id);
  tt->SetDefaultSize(380,20);
  tt->SetMaxLength(98);
  //tt->SetWidth(590);
  //tt->SetState(false);
  hframe2->AddFrame(tt,fL0);
  DoMap(tt,opt_fname,p_txt,0);
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");
}

void ParParDlg::AddFiles(TGCompositeFrame* frame) {
  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Files", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  AddWrite(fF6,"Write raw data",&opt.raw_write,&opt.raw_compr,opt.fname_raw);
  id_write[0]=Plist.size();
  //cout << "raw: " << Plist.size()+1 << endl;

  TGCheckButton *fchk;
  int id;
  char txt[99];
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe1,fL1);

  id = Plist.size()+1;
  sprintf(txt,"Decode");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Decode raw data");
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.decode,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  /*
  id = Plist.size()+1;
  sprintf(txt,"Analyze");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Analyze and use raw decoded data");
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.analyze,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  id = Plist.size()+1;
  sprintf(txt,"DSP");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Use Digital Signal Processing data");
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.dsp,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");
  */

  id = Plist.size()+1;
  sprintf(txt,"CheckDSP");
  fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Compare raw decoded data vs DSP data");
  hframe1->AddFrame(fchk,fL3);
  DoMap(fchk,&opt.checkdsp,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  AddWrite(fF6,"Write decoded data",&opt.dec_write,&opt.dec_compr,opt.fname_dec);
  id_write[1]=Plist.size();
  //cout << "dec: " << Plist.size()+1 << endl;

  AddWrite(fF6,"Write root histograms",&opt.root_write,&opt.root_compr,opt.fname_root);
  id_write[2]=Plist.size();
  //cout << "root: " << Plist.size()+1 << endl;

}

void ParParDlg::AddOpt(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "";
  tip2= "Number of used channels";
  label="Number of channels";
  AddLine_opt(fF6,ww,NULL,&opt.Nchan,tip1,tip2,label,k_int,k_int,1,MAX_CH,1,MAX_CH);

  tip1= "Analysis start (in sec) - only for analyzing files";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine_opt(fF6,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_r0,k_r0);

  TGTextEntry *te = (TGTextEntry*) Plist.back().field;
  te->SetName("Tstop");
  //cout << "tstop: " << te->GetName() << endl;
  //id_tstop = Plist.size();

  tip1= "";
  tip2= "Delay between drawing events (in msec)";
  label="DrawEvent delay";
  AddLine_opt(fF6,ww,NULL,&opt.tsleep,tip1,tip2,label,k_int,k_int,100,10000,100,10000);

  tip1= "Size of the USB buffer in kilobytes (1024 is OK)";
  tip2= "Size of the READ buffer in kilobytes (as large as possible for faster speed)";
  label="USB/READ buffer size";
  AddLine_opt(fF6,ww,&opt.usb_size,&opt.rbuf_size,tip1,tip2,label,k_int,k_int,1,2048,
	   1,1e5,(char*) "DoNum_SetBuf()");
  id_usb = Plist.size()-1;
  //cout << "usbbuf: " << id_usb << endl;

  tip1= "Maximal size of the event list:\nNumber of events available for viewing in the Events Tab";
  tip2= "Event lag:\nMaximal number of events which may come delayed from another channel";
  label="Event_list size / Event lag";
  AddLine_opt(fF6,ww,&opt.ev_max,&opt.ev_min,tip1,tip2,label,k_int,k_int,1,1000000,1,1000000);

  fF6->Resize();

}

void ParParDlg::AddLogic(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Event Logic", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Coincidence window for making events (in samples)";
  tip2= "Veto window (in samples): \nsubsequent pulses from the same channel coming within this window are ignored";
  label="Coincidence (smp), veto (smp)";
  AddLine_opt(fF6,ww,&opt.tgate,&opt.tveto,tip1,tip2,label,k_int,k_int,0,1000,0,1000);

  tip1= "Minimal multiplicity";
  tip2= "Maximal multiplicity";
  label="Multiplicity (min, max)";
  AddLine_opt(fF6,ww,&opt.mult1,&opt.mult2,tip1,tip2,label,k_int,k_int,
	      1,MAX_CH,1,MAX_CH);


  tip1= "This condition is used for selecting events which are written as decoded events\nSee Histograms->Cuts for making conditions\nUse arithmetic/logic operations on existing cuts or leave it empty to record all events\nPress Enter or Check button to check if the syntaxis is correct";
  label="Main Trigger conditions";
  ww=150;
  AddLine_txt(fF6,ww,opt.maintrig, tip1, label);


  fF6->Resize();

}

void ParParDlg::AddAna(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "MTOF Analysis", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, fL6);

  tip1= "Mtof period (mks) (ignored if set to zero)";
  tip2= "Mtof start channel";
  label="Mtof period / start channel";
  AddLine_opt(fF6,ww,&opt.mtof_period,&opt.start_ch,tip1,tip2,label,k_r1,k_int,
	      0,1e9,0,MAX_CH-1);

  tip1= "Mtof Flight path (in meters) for Mtof-Energy conversion";
  tip2= "Mtof Time offset (in mks) for Mtof-Energy conversion";
  label="Mtof Flpath / MTOF Zero";
  AddLine_opt(fF6,ww,&opt.Flpath,&opt.TofZero,tip1,tip2,label,k_r3,k_r3,
	      0,1e9,-1e9,1e9);

  fF6->Resize();

}

void ParParDlg::AddHist(TGCompositeFrame* frame2) {
  
  //int ww=70;
  
  //TGLabel* fLabel = new TGLabel(frame, "---  Histograms  ---");
  //frame->AddFrame(fLabel, fL5);


  TGGroupFrame* frame = new TGGroupFrame(frame2, "1D Histograms", kVerticalFrame);
  frame->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame, fL6);

   // 2 column, n rows
   //frame->SetLayoutManager(new TGMatrixLayout(frame, 0, 3, 7));



  tip1= "Total aqcuisition time, in seconds";
  label="Time";
  AddLine_hist(frame,&opt.h_time,tip1,label);

  tip1= "Area of the pulse or energy, calibrated (see Channels->EM for calibration)";
  label="Area";
  AddLine_hist(frame,&opt.h_area,tip1,label);

  tip1= "Area w/o background, calibrated (see Channels->EM for calibration)";
  label="Area0";
  AddLine_hist(frame,&opt.h_area0,tip1,label);

  tip1= "Base line, calibrated (see Channels->EM for calibration)";
  label="Base";
  AddLine_hist(frame,&opt.h_base,tip1,label);

  tip1= "Maximal pulse height (in channels)";
  label="Height";
  AddLine_hist(frame,&opt.h_hei,tip1,label);

  tip1= "Time of flight (relative to the starts - see Channels->St), in ns";
  label="Tof";
  AddLine_hist(frame,&opt.h_tof,tip1,label);

  tip1= "Time of flight with multiplicity, in mks";
  label="Mtof";
  AddLine_hist(frame,&opt.h_mtof,tip1,label);

  tip1= "Neutron energy from MTOF, in eV";
  label="Etof";
  AddLine_hist(frame,&opt.h_etof,tip1,label);

  tip1= "Pulse period (distance between two consecutive pulses), in mks";
  label="Period";
  AddLine_hist(frame,&opt.h_per,tip1,label);

  tip1= "Average pulse shapes for all channels";
  label="Mean_pulses";
  AddLine_mean(frame,&opt.h_pulse,tip1,label);


  frame = new TGGroupFrame(frame2, "2D Histograms", kVerticalFrame);
  frame->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame, fL6);

  tip1= "2-dimensional histogram (area0-area1), calibrated (see Channels->EM for calibration)";
  label="A0A1";
  AddLine_hist(frame,&opt.h_a0a1,tip1,label);

  tip1= "2-dimensional histogram (Area-Base)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Base";
  AddLine_2d(frame,&opt.h_area_base,tip1,label);

  /*
  tip1= "Bins per channel for Width";
  tip2= "Length of Width (in channels)";
  label="Width";
  AddLine_opt(frame,ww,&opt.rms_bins,&opt.rms_max,tip1,tip2,label,k_r0);

  tip1= "Bins per nanosecond for TOF";
  tip2= "Length of TOF (in nanoseconds)";
  label="TOF";
  AddLine_opt(frame,ww,&opt.tof_bins,&opt.tof_max,tip1,tip2,label,k_r0);
  */

}

void ParParDlg::AddLine_opt(TGGroupFrame* frame, int width, void *x1, void *x2, 
		      const char* tip1, const char* tip2, const char* label,
		      TGNumberFormat::EStyle style1, 
		      TGNumberFormat::EStyle style2, 
			 //TGNumberFormat::EAttribute attr, 
			 double min1, double max1,
			 double min2, double max2, char* connect)
{

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //hfr1->SetBackgroundColor(yellow);
  frame->AddFrame(hfr1);

  if (connect==NULL) {
    connect = (char*) "DoParNum()";
  }

  //double zz;
  int id;
  P_Def pdef1,pdef2;

  if (style1==k_int) {
    pdef1=p_inum;
  }
  else {
    pdef1=p_fnum;
  }

  if (style2==k_int) {
    pdef2=p_inum;
  }
  else {
    pdef2=p_fnum;
  }

  TGNumberFormat::ELimit limits;

  limits = TGNumberFormat::kNELNoLimits;
  if (max1!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  if (x1!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, style1, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min1,max1);
    DoMap(fNum1->GetNumberEntry(),x1,pdef1,0);
    fNum1->GetNumberEntry()->SetToolTipText(tip1);
    fNum1->SetWidth(width);
    //fNum1->Resize(width, fNum1->GetDefaultHeight());
    fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParParDlg", this, connect);
    hfr1->AddFrame(fNum1,fL7);
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,fL7);
  }
  
  limits = TGNumberFormat::kNELNoLimits;
  if (max2!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  if (x2!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum2 = new TGNumberEntry(hfr1, 0, 0, id, style2, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min2,max2);
    DoMap(fNum2->GetNumberEntry(),x2,pdef2,0);
    fNum2->GetNumberEntry()->SetToolTipText(tip2);
    fNum2->SetWidth(width);
    //fNum2->Resize(width, fNum2->GetDefaultHeight());
    fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParParDlg", this, connect);
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

void ParParDlg::AddLine_txt(TGGroupFrame* frame, int width, char* opt_fname, 
			    const char* tip1, const char* label)
{

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;

  id = Plist.size()+1;
  tTrig = new TGTextEntry(hfr1,(char*)opt_fname, id);
  tTrig->SetWidth(width);
  tTrig->SetMaxLength(20);

  DoMap(tTrig,opt_fname,p_txt,0);
  tTrig->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");
  tTrig->Connect("ReturnPressed()", "ParParDlg", this, "CheckFormula()");

  tTrig->SetToolTipText(tip1);
  hfr1->AddFrame(tTrig,fL7);
  
  TGTextButton* but = new TGTextButton(hfr1,"Check",7);
  but->Connect("Clicked()", "ParParDlg", this, "CheckFormula()");
  but->SetToolTipText(tip1);
  hfr1->AddFrame(but, fL6);

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,fL7);

}

void ParParDlg::AddLine_hist(TGGroupFrame* frame, Hdef* hd,
		  const char* tip, const char* label) {

  double ww1=50;
  double ww=70;
  char name[20];

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  //double zz;
  int id;
  //int id0;

  TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  sprintf(name,"b_hist%d",id);
  chk_hist->SetName(name);
  DoMap(chk_hist,&hd->b,p_chk,0);
  chk_hist->Connect("Clicked()", "ParParDlg", this, "DoCheckHist()");
  hfr1->AddFrame(chk_hist,fL3);
  //id0=id;

  //cout << "hist: " << id << " " << chk_hist->GetName() << " " << chk_hist->GetTitle() << endl;
  //nbins
  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),&hd->bins,p_fnum,0);
  fNum1->GetNumberEntry()->SetToolTipText("Number of bins per channel");
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum1,fL8a);

  //xlow
  id = Plist.size()+1;
  TGNumberEntry* fNum2 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum2->GetNumberEntry(),&hd->min,p_fnum,0);
  fNum2->GetNumberEntry()->SetToolTipText("Low edge");
  fNum2->SetWidth(ww);
  fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum2,fL8a);

  //xup
  id = Plist.size()+1;
  TGNumberEntry* fNum3 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum3->GetNumberEntry(),&hd->max,p_fnum,0);
  fNum3->GetNumberEntry()->SetToolTipText("Upper edge");
  fNum3->SetWidth(ww);
  fNum3->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum3,fL8a);


  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,fL8a);

}

void ParParDlg::AddLine_2d(TGGroupFrame* frame, Hdef* hd,
		  const char* tip, const char* label) {

  double ww1=50;
  double ww=90;
  //char name[20];

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  //double zz;
  int id;
  //int id0;

  //TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  //sprintf(name,"b_hist%d",id);
  //chk_hist->SetName(name);
  DoMap(chk_hist,&hd->b,p_chk,0);
  chk_hist->Connect("Clicked()", "ParParDlg", this, "DoCheckPulse()");
  hfr1->AddFrame(chk_hist,fL3);
  //id0=id;

  //nbins (x-axis)
  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),&hd->bins,p_fnum,0);
  fNum1->GetNumberEntry()->SetToolTipText("Number of bins per channel on X-axis");
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				     "DoNum()");
  hfr1->AddFrame(fNum1,fL8a);

  //nbins2 (y-axis)
  id = Plist.size()+1;
  fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
  					   TGNumberFormat::kNEAAnyNumber,
  					   lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),&hd->bins2,p_fnum,0);
  fNum1->GetNumberEntry()->SetToolTipText("Number of bins per channel on Y-axis");
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
  				     "DoNum()");
  hfr1->AddFrame(fNum1,fL8a);

  //hint
  TGLabel* tt = new TGLabel(hfr1,"");
  //TGTextEntry* tt = new TGTextEntry(hfr1," ", 0);
  //tt->SetToolTipText("Min Max are taken from the corresponding 1d histograms");
  tt->ChangeOptions(tt->GetOptions()|kFixedWidth);
  tt->SetWidth(ww);
  hfr1->AddFrame(tt,fL8a);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,fL8a);

}

void ParParDlg::AddLine_mean(TGGroupFrame* frame, Hdef* hd,
			 const char* tip, const char* label)
{
  char name[20];

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  sprintf(name,"b_pulse%d",id);
  chk_hist->SetName(name);
  DoMap(chk_hist,&hd->b,p_chk,0);
  chk_hist->Connect("Clicked()", "ParParDlg", this, "DoCheckPulse()");
  hfr1->AddFrame(chk_hist,fL3);

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

void ParParDlg::DoParNum() {
  ParDlg::DoNum();
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  //Int_t id = te->WidgetId();
  //cout << "Donum: " << id << " " << te->GetName() << endl;
  if (TString(te->GetName()).EqualTo("Tstop",TString::kIgnoreCase)) {
    if (opt.Tstop) {
      te->ChangeBackground(gROOT->GetColor(kYellow)->GetPixel());
    }
    else {
      te->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
    }
  }
}

void ParParDlg::DoNum_SetBuf() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  //cout << "DoNum_SetBuf: " << te->GetNumMax() << " " << id << " " << id_usb << endl;
  //cout << "DoNum: ";
  //cout << *(Int_t*) pp.data << " ";
  //cout << pp.data << " " << opt.bkg1[0] << " ";
  //cout << (Int_t) pp.all << endl;

  if (te->GetNumber() > te->GetNumMax()) {
    cout << "value out of range" << endl;
    return;
  }

  SetNum(pp,te->GetNumber());

  //if (te->GetNumMax() < 3000) { //this is USB buffer
  if (id==id_usb) { //this is USB buffer  
    //#ifdef CYUSB
    //crs->b_usbbuf=true;
    // if (crs->module && !crs->b_acq) {
    //   crs->Free_Transfer();
    //   gSystem->Sleep(50);
    //   crs->Init_Transfer();
    // }
    //#endif
  }
  else { //this is READ buffer
    int bsize=opt.rbuf_size*1024;
    int boffset=0;
    //if (crs->Fmode==1) {
    if (crs->module==1) {
      bsize+=4096;
      boffset+=4096;
    }
    crs->DoReset();
    /*
    if (crs->Fbuf2) delete[] crs->Fbuf2;
    crs->Fbuf2 = new UChar_t[bsize];
    crs->Fbuf = crs->Fbuf2+boffset;
    memset(crs->Fbuf2,0,boffset);
    */
  }

}

void ParParDlg::DoCheckHist() {

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();

  DoChk();

  //cout << "DoCheckHist: " << Plist.size() << " " << id << " " << opt.b_time << endl;

  Bool_t state = (Bool_t) te->GetState();      
  pmap *pp;
  for (int i=0;i<3;i++) {
    pp = &Plist[id+i];
    TGNumberEntryField *te2 = (TGNumberEntryField*) pp->field;
    //cout << i << " " << te2->GetNumber() << endl;
    te2->SetState(state);
  }
  HiFrm->HiReset();
}

void ParParDlg::DoCheckPulse() {
  DoChk();
  HiFrm->HiReset();
}

void ParParDlg::CheckFormula() {
  //cout << "Check: " << opt.maintrig << endl;
  Pixel_t color;

  int ires = crs->Set_Trigger();
  if (ires==1) {// bad formula
    gClient->GetColorByName("red", color);
    tTrig->SetBackgroundColor(color);
  }
  else { //good formula
    gClient->GetColorByName("green", color);
    tTrig->SetBackgroundColor(color);
  }

}

void ParParDlg::Update() {
  ParDlg::Update();
}

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
  TGHorizontalFrame *fH1 = new TGHorizontalFrame(vFrame, 10, 320);
  TGHorizontalFrame *fH2 = new TGHorizontalFrame(vFrame, 10, 205, kFixedHeight);
  vFrame->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));
  TGHSplitter *hsplitter = new TGHSplitter(vFrame,2,2);
  hsplitter->SetFrame(fH2, kFALSE);
  vFrame->AddFrame(hsplitter, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  vFrame->AddFrame(fH2, new TGLayoutHints(kLHintsBottom | kLHintsExpandX));   




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

void ChanParDlg::DoChanMap(TGWidget *f, void *d, P_Def t, int all,
		       byte cmd, byte chan, void* d2) {
  ParDlg::DoMap(f,d,t,all);
  Plist.back().cmd=cmd;
  Plist.back().chan=chan;
  Plist.back().data2= d2;
}

void ChanParDlg::DoChanNum() {
  ParDlg::DoNum();

  // TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  // Int_t id = te->WidgetId();

  // pmap pp = Plist[id-1];

  //cout << "ch_donum: " << pp.data2 << endl;

// #ifdef CYUSB
//   printf("cmd77: %d %d %d\n",pp.cmd,pp.chan,*(Int_t*)pp.data);
//   if (pp.cmd && crs->b_acq) {
//     crs->Command2(4,0,0,0);
//     printf("cmd: %d %d %d\n",pp.cmd,pp.chan,*(Int_t*)pp.data);
//     //crs->Command_crs(pp.cmd,pp.chan,*(Int_t*)pp.data);
//     crs->Command_crs(pp.cmd,pp.chan);
//     crs->Command2(3,0,0,0);
//   }
// #endif

}

//void ChanParDlg::DoCheck() {
//ParDlg::DoChk();
//}

void ChanParDlg::AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
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

  DoChanMap(fNum,apar,ptype, all,0,0);
  fNum->SetWidth(tlen7[kk]);
  fNum->SetHeight(20);
  fNum->Connect("TextChanged(char*)", "ChanParDlg", this, "DoChanNum()");
  fNum->SetToolTipText(ttip7[kk]);
  hframe1->AddFrame(fNum,fL0);

}

//------ CrsParDlg -------

CrsParDlg::CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
  trig=0;
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

  for (int i=0;i<MAX_CH;i++) {
    AddLine_crs(i,fcont1);
    //cout << "crs: addLine1: " << Plist.size() << endl; 
  }

  AddLine_crs(MAX_CH,fcont2);

  for (int i=1;i<ADDCH;i++) {
    AddLine_crs(MAX_CH+i,fcont2);
    //cout << "crs2: addLine1: " << Plist.size() << endl; 
  }

  if (crs->module==2) {
    TGHorizontalFrame *hforce = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(hforce,fL1);

    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce, "", id);
    hforce->AddFrame(fforce,fL3);
    DoChanMap((TGWidget*)fforce,&cpar.forcewr,p_chk,0,0,0);
    fforce->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
    
    TGLabel* lforce = new TGLabel(hforce, "  Force_write all channels");
    hforce->AddFrame(lforce,fL1);
  }

}

void CrsParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,fL1);

  TGTextEntry* tt[ncrspar+1];

  for (int i=0;i<=ncrspar;i++) {
    if (!strcmp(tlab1[i],"Trg") && crs->module<33) {
      continue;
    }
    tt[i]=new TGTextEntry(head_frame, tlab1[i]);
    tt[i]->SetWidth(tlen1[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(ttip1[i]);
    tt[i]->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt[i],fL0);
  }

}

void CrsParDlg::AddLine_crs(int i, TGCompositeFrame* fcont1) {
  char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],fL1);

  // Pixel_t yellow,green;
  // gClient->GetColorByName("yellow", yellow);
  // gClient->GetColorByName("green", green);
  //cframe[i]->ChangeBackground(yellow);
  //cframe[i]->SetForegroundColor(green);

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

  clab[i]=new TGTextEntry(cframe[i], txt);
  clab[i]->SetHeight(20);
  clab[i]->SetWidth(tlen1[0]);
  clab[i]->SetToolTipText(ttip1[kk]);
  clab[i]->SetState(false);
  cframe[i]->AddFrame(clab[i],fL0a);
  kk++;

  // if (i%2) {
  //   cframe[i]->SetBackgroundColor(yellow);
  //   clab[i]->SetBackgroundColor(yellow);
  // }
  // else {
  //   cframe[i]->SetBackgroundColor(green);
  //   clab[i]->SetBackgroundColor(green);
  // }

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
    //cout << "nfld: " << Plist.size() << " " << nfld << endl;
    //start=false;
  }

  id = Plist.size()+1;
  TGComboBox* fCombo=new TGComboBox(cframe[i],id);
  //fCombo->SetToolTipText(ttip1[kk]);
  //TGComboBox* fCombo=new TGComboBox(cframe[i],id,kHorizontalFrame|kSunkenFrame|kDoubleBorder, yellow);
  // fCombo->SetBackgroundColor(yellow);
  // fCombo->SetForegroundColor(green);
  cframe[i]->AddFrame(fCombo,fL0);
  kk++;

  for (int j = 0; j < ADDCH; j++) {
    fCombo->AddEntry(types[j], j+1);
  }

  fCombo->Resize(tlen1[1], 20);

  if (i==MAX_CH) {
    fCombo->AddEntry(types[ADDCH+1], ADDCH+2);
  }
  else {
    fCombo->AddEntry(types[ADDCH], ADDCH+1);
  }

  if (i<=MAX_CH) {
    DoChanMap(fCombo,&opt.chtype[i],p_cmb,all,0,0);
  }
  else {
    DoChanMap(fCombo,&combotype[i],p_cmb,all,0,0);
    fCombo->Select(i-MAX_CH,false);
    fCombo->SetEnabled(false);
    //cout << "tcol: " << i-MAX_CH << " " << tcol[i-MAX_CH] << endl;
    cframe[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);
    clab[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);    
  }

  // cout << fCombo->GetSelectedEntry() << endl; //->SetBackgroundColor(yellow);
  // if (fCombo->GetSelectedEntry()) {
  //   fCombo->GetSelectedEntry()->SetBackgroundColor(yellow);
  //   fCombo->GetSelectedEntry()->SetForegroundColor(yellow);
  // }
  
  //DoMap(fCombo,&opt.chtype[i],p_cmb,all,0,0);

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");

  id = Plist.size()+1;
  TGCheckButton *f_en = new TGCheckButton(cframe[i], "", id);
  TGCheckButton *f_inv = new TGCheckButton(cframe[i], "", id+1);
  TGCheckButton *f_acdc = new TGCheckButton(cframe[i], "", id+2);
  DoChanMap(f_en,&cpar.enabl[i],p_chk,all,1,i);
  DoChanMap(f_inv,&cpar.inv[i],p_chk,all,2,i);
  DoChanMap(f_acdc,&cpar.acdc[i],p_chk,all,3,i);

  f_en->SetToolTipText(ttip1[kk]);
  f_inv->SetToolTipText(ttip1[kk+1]);
  f_acdc->SetToolTipText(ttip1[kk+2]);
  f_en->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  f_inv->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  f_acdc->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");

  //f_inv->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_acdc->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_inv->SetWidth(25);
  //f_acdc->SetWidth(25);

  cframe[i]->AddFrame(f_en,fL3);
  cframe[i]->AddFrame(f_inv,fL3);
  cframe[i]->AddFrame(f_acdc,fL3);
  kk+=3;

  AddNumCrs(i,kk++,all,cframe[i],"smooth",&cpar.smooth[i]);
  AddNumCrs(i,kk++,all,cframe[i],"dt"    ,&cpar.deadTime[i]);
  AddNumCrs(i,kk++,all,cframe[i],"pre"   ,&cpar.preWr[i]);
  AddNumCrs(i,kk++,all,cframe[i],"len"   ,&cpar.durWr[i]);
  if (crs->module==2) 
    AddNumCrs(i,kk++,1,cframe[i],"gain"  ,&cpar.adcGain[i]);
  else
    AddNumCrs(i,kk++,all,cframe[i],"gain"  ,&cpar.adcGain[i]);

  if (crs->module>=33)
    AddNumCrs(i,kk++,all,cframe[i],"trig" ,&cpar.trg[i]);
  else
    kk++;

  AddNumCrs(i,kk++,all,cframe[i],"deriv" ,&cpar.kderiv[i],&opt.kdrv[i]);
  AddNumCrs(i,kk++,all,cframe[i],"thresh",&cpar.threshold[i],&opt.thresh[i]);


  if (i<=MAX_CH) {
    fStat[i] = new TGTextEntry(cframe[i], "");
    fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat[i]->SetState(false);
    //fStat[i]->SetMargins(10,0,0,0);
    //fStat[i]->SetTextJustify(kTextLeft|kTextCenterY);
    fStat[i]->SetToolTipText(ttip1[kk]);

    fStat[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fStat[i]->SetBackgroundColor(col);
    fStat[i]->SetText(0);
    //fbar[i]->SetParts(parts, nparts);
    //fBar2->SetParts(nparts);
    //fStat[i]->Draw3DCorner(kFALSE);
    //fbar[i]->DrawBorder();
    cframe[i]->AddFrame(fStat[i],fL8);
  }
  kk++;

  if (i<=MAX_CH) {
    fStatBad[i] = new TGTextEntry(cframe[i], "");
    fStatBad[i]->ChangeOptions(fStatBad[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStatBad[i]->SetState(false);
    fStatBad[i]->SetToolTipText(ttip1[kk]);

    fStatBad[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fStatBad[i]->SetBackgroundColor(col);
    fStatBad[i]->SetText(0);
    cframe[i]->AddFrame(fStatBad[i],fL8);
  }


}

void CrsParDlg::AddNumCrs(int i, int kk, int all, TGHorizontalFrame *hframe1,
			 const char* name, void* apar, void* apar2) {  //const char* name) {

  int par, min, max;

  cpar.GetPar(name,crs->module,i,crs->type_ch[i],par,min,max);
  //cout << "getpar1: " << i << " " << min << " " << max << endl;

  TGNumberFormat::ELimit limits;

  limits = TGNumberFormat::kNELLimitMinMax;
  if (max==0) {
    limits = TGNumberFormat::kNELNoLimits;
  }

  int id = Plist.size()+1;
  TGNumberEntryField* fNum =
    new TGNumberEntryField(hframe1, id, par, TGNumberFormat::kNESInteger,
			   TGNumberFormat::kNEAAnyNumber,
			   limits,min,max);

  char ss[100];
  sprintf(ss,"%s%d",name,id);
  fNum->SetName(ss);
  // if (apar == &cpar.preWr[1]) {
  //   cout << "prewr[0]: " << id << " " << fNum->GetNumLimits() << " " << min << " " << max << endl;
  // }
  DoChanMap(fNum,apar,p_inum, all,kk-1,i,apar2);
  
  //fNum->SetName(name);
  fNum->SetToolTipText(ttip1[kk]);
  fNum->SetWidth(tlen1[kk]);
  fNum->SetHeight(20);

  fNum->Connect("TextChanged(char*)", "CrsParDlg", this, "DoCrsNum()");

  hframe1->AddFrame(fNum,fL0);

}

void CrsParDlg::ResetStatus() {

  TGString txt="0";

  for (int i=0;i<opt.Nchan;i++) {
      fStat[i]->SetText(txt);
      fStatBad[i]->SetText(txt);
  }

  fStat[MAX_CH]->SetText(txt);
  fStatBad[MAX_CH]->SetText(txt);

}
void CrsParDlg::UpdateStatus() {

  static Long64_t allpulses;
  static Long64_t allbad;
  static Int_t npulses3[MAX_CH];
  static double t1;
  double rate;

  TGString txt;

  double dt = opt.T_acq - t1;

  //cout << "DT: " << dt << endl;
  allbad=0;
  if (dt>0) {
    for (int i=0;i<opt.Nchan;i++) {
      //if (crs->npulses2[i]) {
      rate = (crs->npulses2[i]-npulses3[i])/dt;
	//if (rate>0) {
      txt.Form("%0.0f",rate);
      fStat[i]->SetText(txt);
      txt.Form("%d",crs->npulses_bad[i]);
      fStatBad[i]->SetText(txt);
	  //cout << i << " " << crs->npulses2[i] << " " << rate << " " << dt << endl;
	  //}
      npulses3[i]=crs->npulses2[i];
      allbad+=crs->npulses_bad[i];
	//}
    }
    //if (crs->npulses) {
    rate = (crs->npulses-allpulses)/dt;
    //if (rate>0) {
    txt.Form("%0.0f",rate);
    fStat[MAX_CH]->SetText(txt);
    txt.Form("%lld",allbad);
    fStatBad[MAX_CH]->SetText(txt);
    //cout << i << " " << crs->npulses2[i] << " " << rate << " " << dt << endl;
    //}
    allpulses=crs->npulses;
    //}
  }
  t1=opt.T_acq;

}

void CrsParDlg::DoCrsNum() {
  trig++;
  ParDlg::DoNum();
  trig--;

  //cout << "CrsParDlg::DoNum()" << endl;
#ifdef CYUSB
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  pmap pp = Plist[id-1];

  //cout << "ch_donum: " << id << " " << pp.data2 << " " << trig << endl;

  //printf("cmd77: %d %d %d\n",pp.cmd,pp.chan,*(Int_t*)pp.data);
  if (pp.cmd && crs->b_acq && !trig) {
    crs->Command2(4,0,0,0);
    //printf("cmd: %d %d %d\n",pp.cmd,pp.chan,*(Int_t*)pp.data);
    //crs->Command_crs(pp.cmd,pp.chan);
    crs->SetPar();
    crs->Command2(3,0,0,0);
  }
#endif

}

void CrsParDlg::DoCheck() {
  trig++;
  ParDlg::DoChk();
  trig--;

#ifdef CYUSB
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  pmap pp = Plist[id-1];

  //printf("chk77: %d %d %d\n",pp.cmd,pp.chan,*(Bool_t*)pp.data);
  if (pp.cmd && crs->b_acq && !trig) {
    crs->Command2(4,0,0,0);
    //printf("chk: %d %d %d\n",pp.cmd,pp.chan,*(Bool_t*)pp.data);
    //crs->Command_crs(pp.cmd,pp.chan);
    crs->SetPar();
    crs->Command2(3,0,0,0);
  }
#endif

}


//------ AnaParDlg -------

AnaParDlg::AnaParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
}

void AnaParDlg::Make_AnaPar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  for (int i=0;i<MAX_CH;i++) {
    AddLine_Ana(i,fcont1);
  }

  AddLine_Ana(MAX_CH,fcont2);

  for (int i=1;i<ADDCH;i++) {
    AddLine_Ana(MAX_CH+i,fcont2);
  }


}

void AnaParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,fL1);

  TGTextEntry* tt;

  for (int i=0;i<n_apar;i++) {
    tt=new TGTextEntry(head_frame, tlab2[i]);
    tt->SetWidth(tlen2[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip2[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,fL0);
  }

  char ss[8];
  //char tip[100];
  for (int j=0;j<NGRP;j++) {
    sprintf(ss,"g%d",j+1);
    sprintf(ttip_g[j],"Use channel for group histograms *_%s",ss);
    tt=new TGTextEntry(head_frame, ss);
    tt->SetWidth(24);
    tt->SetState(false);
    tt->SetToolTipText(ttip_g[j]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,fL0);
  }

}

void AnaParDlg::AddLine_Ana(int i, TGCompositeFrame* fcont1) {
  char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],fL1);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

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

  clab[i]=new TGTextEntry(cframe[i], txt);
  clab[i]->SetHeight(20);
  clab[i]->SetWidth(tlen2[0]);
  clab[i]->SetToolTipText(ttip2[kk]);
  clab[i]->SetState(false);
  cframe[i]->AddFrame(clab[i],fL0a);
  kk++;

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
    //cout << "nfld: " << Plist.size() << " " << nfld << endl;
    //start=false;
  }

  id = Plist.size()+1;


  TGComboBox* fCombo=new TGComboBox(cframe[i],id);
  cframe[i]->AddFrame(fCombo,fL0);
  kk++;

  for (int j = 0; j < ADDCH; j++) {
    fCombo->AddEntry(types[j], j+1);
  }

  fCombo->Resize(tlen2[1], 20);

  if (i==MAX_CH) {
    fCombo->AddEntry(types[ADDCH+1], ADDCH+2);
  }
  else {
    fCombo->AddEntry(types[ADDCH], ADDCH+1);
  }

  if (i<=MAX_CH) {
    DoChanMap(fCombo,&opt.chtype[i],p_cmb,all,0,0);
  }
  else {
    DoChanMap(fCombo,&combotype[i],p_cmb,all,0,0);
    fCombo->Select(i-MAX_CH,false);
    fCombo->SetEnabled(false);
    //cout << "tcol: " << i-MAX_CH << " " << tcol[i-MAX_CH] << endl;
    cframe[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);
    clab[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);
  }

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");

  id = Plist.size()+1;
  TGCheckButton *fdsp = new TGCheckButton(cframe[i], "", id);
  DoChanMap(fdsp,&opt.dsp[i],p_chk,all,0,0);
  fdsp->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fdsp->SetToolTipText(ttip2[kk]);
  cframe[i]->AddFrame(fdsp,fL3);
  kk++;

  id = Plist.size()+1;
  TGCheckButton *fst = new TGCheckButton(cframe[i], "", id);
  DoChanMap(fst,&opt.Start[i],p_chk,all,0,0);
  fst->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fst->SetToolTipText(ttip2[kk]);
  cframe[i]->AddFrame(fst,fL3);
  kk++;

  id = Plist.size()+1;
  TGCheckButton *fmt = new TGCheckButton(cframe[i], "", id);
  DoChanMap(fmt,&opt.Mrk[i],p_chk,all,0,0);
  fmt->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fmt->SetToolTipText(ttip2[kk]);
  cframe[i]->AddFrame(fmt,fL3);
  kk++;


  tlen7 = (int*) tlen2;
  ttip7 = (char**) ttip2;

  AddNumChan(i,kk++,all,cframe[i],&opt.nsmoo[i],0,99,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.kdrv[i],1,999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.thresh[i],0,9999,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.bkg1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.bkg2[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.peak1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.peak2[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.deadT[i],0,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.pile[i],0,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.timing[i],0,3,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.twin1[i],-99,99,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.twin2[i],-99,99,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.wwin1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.wwin2[i],-999,3070,p_inum);

  AddNumChan(i,kk++,all,cframe[i],&opt.emult[i],0,99999,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.elim1[i],0,99999,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.elim2[i],0,99999,p_fnum);

  for (int j=0;j<NGRP;j++) {
    id = Plist.size()+1;
    TGCheckButton *gg = new TGCheckButton(cframe[i], "", id);
    DoChanMap(gg,&opt.Grp[i][j],p_chk,all,0,0);
    gg->Connect("Clicked()", "ParDlg", this, "DoChk()");
    gg->SetToolTipText(ttip_g[j]);
    cframe[i]->AddFrame(gg,fL3);
    kk++;
  }

}

//------ PikParDlg -------

PikParDlg::PikParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
}

void PikParDlg::Make_PikPar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  for (int i=0;i<MAX_CH;i++) {
    AddLine_Pik(i,fcont1);
  }

  AddLine_Pik(MAX_CH,fcont2);

  for (int i=1;i<ADDCH;i++) {
    AddLine_Pik(MAX_CH+i,fcont2);
  }


}

void PikParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,fL1);

  TGTextEntry* tt;

  for (int i=0;i<n_ppar;i++) {
    tt=new TGTextEntry(head_frame, tlab3[i]);
    tt->SetWidth(tlen3[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip3[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,fL0);
  }

}

void PikParDlg::AddLine_Pik(int i, TGCompositeFrame* fcont1) {
  char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],fL1);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

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

  clab[i]=new TGTextEntry(cframe[i], txt);
  clab[i]->SetHeight(20);
  clab[i]->SetWidth(tlen2[0]);
  clab[i]->SetToolTipText(ttip2[kk]);
  clab[i]->SetState(false);
  cframe[i]->AddFrame(clab[i],fL0a);
  kk++;

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
    //cout << "nfld: " << Plist.size() << " " << nfld << endl;
    //start=false;
  }

  id = Plist.size()+1;


  TGComboBox* fCombo=new TGComboBox(cframe[i],id);
  cframe[i]->AddFrame(fCombo,fL0);
  kk++;

  for (int j = 0; j < ADDCH; j++) {
    fCombo->AddEntry(types[j], j+1);
  }

  fCombo->Resize(tlen2[1], 20);

  if (i==MAX_CH) {
    fCombo->AddEntry(types[ADDCH+1], ADDCH+2);
  }
  else {
    fCombo->AddEntry(types[ADDCH], ADDCH+1);
  }

  if (i<=MAX_CH) {
    DoChanMap(fCombo,&opt.chtype[i],p_cmb,all,0,0);
  }
  else {
    DoChanMap(fCombo,&combotype[i],p_cmb,all,0,0);
    fCombo->Select(i-MAX_CH,false);
    fCombo->SetEnabled(false);
    //cout << "tcol: " << i-MAX_CH << " " << tcol[i-MAX_CH] << endl;
    cframe[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);
    clab[i]->SetBackgroundColor(tcol[i-MAX_CH-1]);
  }

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");


  tlen7 = (int*) tlen3;
  ttip7 = (char**) ttip3;

  AddNumChan(i,kk++,all,cframe[i],&opt.bkg1[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.bkg2[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.peak1[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.peak2[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.twin1[i],-99,99,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.twin2[i],-99,99,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.wwin1[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.wwin2[i],-999,3070,p_inum);
}
