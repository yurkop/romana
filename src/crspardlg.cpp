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

//colors of ch_types
namespace CP {
  //Float_t RGB[MAX_TP][3] =
  Float_t RGB[][3] =
    {
      {1,1,0}, //0
      {0,1,1},
      {1,0.4,1},
      {0.4,0.4,1},
      {1,0.8,0.8}, //4
      {0,1,0},
      //{1,0.7,0.4},
      {0.2,0.8,0.6},
      {1,1,1}//8
    };
}

extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern AnaParDlg *anapar;
extern DspParDlg *pikpar;

const int ncrspar=15;

const int tlen1[ncrspar+1]={26,60,24,25,24,24,21,45,40,40,25,25,36,45,75,80};
const char* tlab1[ncrspar+1]={"Ch","Type","on","Inv","AC","pls","hS","Dt","Pre","Len","G","Trg","Drv","Thr","Pulse/sec","BadPulse"};
const char* ttip1[ncrspar+1]={
  "Channel number",
  "Channel type",
  "On/Off",
  "Inversion",
  "AC coupling",
  "Send/don't send pulse data",
  "Hardware smoothing: Smooth=2^hS",
  "Dead time - no new trigger on the current channel within dead time from the old trigger",
  "Number of samples before the trigger",
  "Total length of the pulse in samples",
  "Additional Gain",
  "Trigget type: 0 - threshold crossing of pulse; 1 - threshold crossing of derivative;\n2 - maximum of derivative; 3 - rise of derivative;",
  "Parameter of derivative: S(i) - S(i-Drv)",
  "Trigger threshold",
  "Pulse rate",
  "Number of Bad pulses"
};

const int n_apar=11;
const int tlen2[n_apar]={26,60,24,25,35,35,35,38,38,38,38};
const char* tlab2[n_apar]={"Ch","Type","St","sS","Delay","dT","Pile","E0","E1","E2","Bc"};
const char* ttip2[n_apar]={
  "Channel number",
  "Channel type",
  "Start channel - used for making TOF start\nif there are many start channels in the event, the earliest is used",
  "Software smoothing",
  "Time delay in samples (can be negative or positive)",
  "Dead-time window \nsubsequent peaks within this window are ignored",
  "Pileup window \nmultiple peaks within this window are marked as pileup",
  //"Timing mode (in 1st derivative):\n0 - threshold crossing (Pos);\n1 - left minimum (T1);\n2 - right minimum;\n3 - maximum in 1st derivative",
  "Energy calibration 0: [0]+[1]*x+[2]*x^2",
  "Energy calibration 1: [0]+[1]*x+[2]*x^2",
  "Energy calibration 2: [0]+[1]*x+[2]*x^2",
  "Baseline correction"
};


const int n_dpar=14;
const int tlen3[n_dpar]={26,60,24,26,32,40,40,40,42,42,35,35,35,35};
const char* tlab3[n_dpar]={"Ch","Type","dsp","sTg","Drv","Thr","Base1","Base2","Peak1","Peak2","T1","T2","W1","W2"};
const char* ttip3[n_dpar]={
  "Channel number",
  "Channel type",
  "Checked - use hardware pulse analysis (DSP)\nUnchecked - use software pulse analysis",
  "Software trigget type:\n0 - hreshold crossing of pulse;\n1 - threshold crossing of derivative;\n2 - maximum of derivative;\n3 - rise of derivative;\n4 - fall of derivative;\n5 - threshold crossing of derivative, use 2nd deriv for timing;\n-1 - use hardware trigger",
  "Drv>0 - trigger on differential S(i) - S(i-Drv)",
  "Trigger threshold",
  "Baseline start, relative to peak Pos (negative)",
  "Baseline end, relative to peak Pos (negative), included",
  "Peak start, relative to peak Pos (usually negative)",
  "Peak end, relative to peak Pos (usually positive), included",
  "Timing window start, included (usually negative)",
  "Timing window end, included (usually positive), included",
  "Width window start",
  "Width window end, included",
};

char ttip_g[NGRP][100];

int* tlen7;
char** ttip7;

// const char* types[]={"NaI","BGO","Si 1","Si 2","Stilb","Demon","HPGe",
// 		     "NIM","Other","Copy",""};

//-----------
// const int nchtype=9;
// Int_t combotype[nchtype];

extern Common* com;
extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  fDock = new TGDockableFrame(this);
  AddFrame(fDock, com->LayEE0);
  //fDock->SetWindowName("Events");  
  fDock->SetFixedSize(kFALSE);

  // for (int i=0;i<nchtype;i++) {
  //   combotype[i]=i;
  // }

  nfld=0;

  Int_t cc;
  for (int i=0;i<=MAX_TP;i++) {
    cc=TColor::GetColor(TColor::RGB2Pixel(CP::RGB[i][0],CP::RGB[i][1],CP::RGB[i][2]));
    tcol[i]=gROOT->GetColor(cc)->GetPixel();
  }

  //other - while color
  //cc=TColor::GetColor(TColor::RGB2Pixel(1,1,1));
  //tcol[MAX_TP]=gROOT->GetColor(cc)->GetPixel();

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
  // cout << pp.data << " " << opt.Base1[0] << " ";
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
  //Int_t id = te->WidgetId();

  DoChk();

  //cout << "chkwrite: " << Plist.size() << " " << id << endl;

  //return;

  // pmap pp;
  // pp = Plist[id+1];
  // TGTextButton *but = (TGTextButton*) pp.field;
  // pp = Plist[id+2];
  // TGTextEntry *te2 = (TGTextEntry*) pp.field;

  Bool_t state = (Bool_t) te->GetState();      

  if (state) {
    te->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
  }
  else {
    te->ChangeBackground(gROOT->GetColor(18)->GetPixel());
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

void ParDlg::DoCombo(bool cp) {

  TGComboBox *te = (TGComboBox*) gTQSender;
  Int_t id = te->WidgetId();

  int sel = te->GetSelected();

  pmap pp = Plist[id-1];

  int nline = id/nfld;

  if (nline < MAX_CH) {

    if (sel-1==MAX_TP+1) { //Copy
      int old=*(Int_t*) pp.data;

      //if (old!=MAX_TP) {
      const char* name = te->GetListBox()->GetEntry(old)->GetTitle();
      //cout << "Copy: " << sel << " " << old << " " << nline << " " << nline%MAX_TP << endl;
      if (!TString(name).EqualTo("Other",TString::kIgnoreCase)) {
	crspar->CopyParLine(-old,nline);
	anapar->CopyParLine(-old,nline);
	pikpar->CopyParLine(-old,nline);
      }
      else {//copy other to last
	crspar->CopyParLine(-(MAX_TP-1),nline);
	anapar->CopyParLine(-(MAX_TP-1),nline);
	pikpar->CopyParLine(-(MAX_TP-1),nline);
      }

      te->Select(old);
      return;
    }

    SetCombo(pp,sel);
    if (cp) {
      crspar->CopyParLine(sel,nline);
      anapar->CopyParLine(sel,nline);
      pikpar->CopyParLine(sel,nline);
    }

  }

  if (pp.all==1) {
    //cout << "all: " << nfld << " " << sel << endl;
    if (nfld && sel<MAX_TP+2) {
      int kk = (id-1)%nfld;
      for (int i=0;i<MAX_CH;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetCombo(p2,te->GetSelected());
	TGComboBox *te2 = (TGComboBox*) p2.field;
	te2->Select(te->GetSelected(),false);

	crspar->CopyParLine(sel,i);
	anapar->CopyParLine(sel,i);
	pikpar->CopyParLine(sel,i);
      }
    }
  }

  //cout << "DoCombo chtype: " << opt.chtype[0] << " " << id << " " << cp << endl;

}

/*
void ParDlg::DoCombo2(Event_t* evt) {

  if (evt->fType==2 && evt->fCode==65) {

  TGComboBox *te = (TGComboBox*) gTQSender;
  Int_t id = te->WidgetId();

  int sel = te->GetSelected();
  pmap pp = Plist[id-1];
  int nline = id/nfld;

  if (nline < MAX_CH) {

    if (sel-1==MAX_TP+1) { //Copy
      int old=*(Int_t*) pp.data;

      //if (old!=MAX_TP) {
      const char* name = te->GetListBox()->GetEntry(old)->GetTitle();
      cout << "Copy: " << sel << " " << old << " " << nline << " " << nline%MAX_TP << endl;
      if (!TString(name).EqualTo("Other",TString::kIgnoreCase)) {
	crspar->CopyParLine(-old,nline);
	anapar->CopyParLine(-old,nline);
	pikpar->CopyParLine(-old,nline);
      }
      else {//copy other to last
	crspar->CopyParLine(-(MAX_TP-1),nline);
	anapar->CopyParLine(-(MAX_TP-1),nline);
	pikpar->CopyParLine(-(MAX_TP-1),nline);
      }

      te->Select(old);
      return;
    }

    SetCombo(pp,sel);
    crspar->CopyParLine(sel,nline);
    anapar->CopyParLine(sel,nline);
    pikpar->CopyParLine(sel,nline);

  }

  if (pp.all==1) {
    //cout << "all: " << nfld << " " << sel << endl;
    if (nfld && sel<MAX_TP+2) {
      int kk = (id-1)%nfld;
      for (int i=0;i<MAX_CH;i++) {
	pmap p2 = Plist[i*nfld+kk];
	SetCombo(p2,te->GetSelected());
	TGComboBox *te2 = (TGComboBox*) p2.field;
	te2->Select(te->GetSelected(),false);

	crspar->CopyParLine(sel,i);
	anapar->CopyParLine(sel,i);
	pikpar->CopyParLine(sel,i);
      }
    }
  }

  cout << "DoCombo chtype: " << opt.chtype[0] << " " << id << endl;

}
*/
void ParDlg::DoCombo2(Event_t* evt) {

  //TGComboBox *te = (TGComboBox*) gTQSender;
  //Int_t id = te->WidgetId();

  //cout << "DoCombo2: " << id << " " << evt->fType << " " << evt->fCode << " " << te->GetSelected() << endl;
  if (evt->fType==14) {
    if (evt->fCode==65) {
      //cout << "DoCombo7: " << id << " " << evt->fType << " " << evt->fCode << " " << te->GetSelected() << endl;
      DoCombo(false);
    }
    else {
      DoCombo(true);
    }
  }
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

  pmap pp = Plist[id-1];

  SetTxt(pp,te->GetText());

  /*
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
  }
  */
}

void ParDlg::DoTypes() {

  TGTextEntry *te = (TGTextEntry*) gTQSender;
  //Int_t id = te->WidgetId();

  //cout << "DoTypes: " << id << endl;
  DoTxt();

  //pmap pp = Plist[id-1];

  int i = TString(te->GetName())(0,1).String().Atoi();
  int i0=i-1;
  if (i0==0) i0=-1;
  for (int j=0;j<=MAX_CH;j++) {

    int sel = fCombo[j]->GetListBox()->GetSelected();
    //cout << "Dotypes: " << j << " " << i << " " << opt.ch_name[i-1] << " " << sel << endl;
    fCombo[j]->RemoveEntry(i);
    fCombo[j]->InsertEntry(opt.ch_name[i-1],i,i0);
    if (sel==i)
      fCombo[j]->Select(sel,false);
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
  else if (sel<MAX_TP+1) { //normal copy from group to current ch
    for (int j=1;j<nfld;j++) {
      CopyField((MAX_CH+sel)*nfld+j,line*nfld+j);
    }
    clab[line]->ChangeBackground(tcol[sel-1]);
    cframe[line]->ChangeBackground(tcol[sel-1]);
  }
  else if (sel==MAX_TP+1) { //other - just change color
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
      if (bb) {
	te->ChangeBackground(gROOT->GetColor(kPink-9)->GetPixel());
      }
      else {
	te->ChangeBackground(gROOT->GetColor(18)->GetPixel());
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
    int sel = *(Int_t*) pp->data;


    for (int i=0;i<MAX_TP-1;i++) {
      TGTextLBEntry* ent=
	(TGTextLBEntry*)te->GetListBox()->GetEntry(i+1);
      ent->SetText(new TGString(opt.ch_name[i]));
      //ent->SetTitle(opt.ch_name[i]);
    }
    te->Layout();

    //cout << "cmb2: " << sel << " " << line << " " << tcol[sel-1] << endl;
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
  //MapSubwindows();
  //Layout();
  //Rebuild();
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
    //cout << "rebuild: " << i << " " << opt.Nchan << endl;
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

  fDock->SetWindowName("Parameters");  

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,com->LayEE0);

  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);

  hor = new TGSplitFrame(fcont1,10,10);
  fcont1->AddFrame(hor,com->LayEE0);
  hor->VSplit(420);
  hor->SetWRatio(0.5);
  //hor->SplitVertical();
  ver1 = hor->GetFirst();
  ver2 = hor->GetSecond();

  //ver1->ChangeOptions(ver1->GetOptions()|kFixedWidth);

  AddFiles(ver1);
  AddOpt(ver1);
  AddLogic(ver1);
  AddAna(ver1);
  AddHist(ver2);

}
void ParParDlg::AddChk(TGGroupFrame* frame, const char* txt, Bool_t* opt_chk,
		       Int_t* compr) {
  int id;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(frame,10,10);
  frame->AddFrame(hframe1,com->LayLT0);

  id = Plist.size()+1;
  TGCheckButton *fchk = new TGCheckButton(hframe1, txt, id);
  fchk->SetName(txt);
  fchk->ChangeOptions(fchk->GetOptions()|kFixedWidth);
  fchk->SetWidth(230);

  hframe1->AddFrame(fchk,com->LayCC1);
  DoMap(fchk,opt_chk,p_chk,0);
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");

  //fchk->SetState(kButtonDown,false);

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hframe1, *compr, 2, id, k_int, 
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   0,9);
  hframe1->AddFrame(fNum1,com->LayCC1);
  DoMap(fNum1->GetNumberEntry(),compr,p_inum,0);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoNum()");

  fNum1->GetNumberEntry()->SetToolTipText("Compression factor [0-9]: 0 - no compression (fast); 9- maximum compression (slow)");
  TGLabel* fLabel = new TGLabel(hframe1, "compr.");
  hframe1->AddFrame(fLabel,com->LayCC1);

}

void ParParDlg::AddFiles(TGCompositeFrame* frame) {
  int id;
  char txt[99];

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Files", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, com->LayET3);





  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe1,com->LayLT0);

  TGLabel* fLabel = new TGLabel(hframe1, "Filename:");
  hframe1->AddFrame(fLabel,com->LayCC1);


  id = Plist.size()+1;
  TGTextButton *fbut = new TGTextButton(hframe1,"Select...",id);
  hframe1->AddFrame(fbut, com->LayCC1);
  DoMap(fbut,opt.Filename,p_open,0);
  fbut->Connect("Clicked()", "ParDlg", this, "DoOpen()");

  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe2,com->LayLT0);

  //strcpy(opt.fname_raw,"raw32.gz");
  id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(hframe2,opt.Filename, id);
  tt->SetDefaultSize(380,20);
  tt->SetMaxLength(198);
  //tt->SetWidth(590);
  //tt->SetState(false);
  hframe2->AddFrame(tt,com->LayCC0);
  DoMap(tt,opt.Filename,p_txt,0);
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");


  AddChk(fF6,"Write raw data [Filename].raw",&opt.raw_write,&opt.raw_compr);
  AddChk(fF6,"Write decoded data [Filename].dec",&opt.dec_write,&opt.dec_compr);
  AddChk(fF6,"Write root histograms [Filename].root",&opt.root_write,&opt.root_compr);



  TGCheckButton *fchk;
  TGHorizontalFrame *hframe3 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe3,com->LayLT0);

  id = Plist.size()+1;
  sprintf(txt,"Decode");
  fchk = new TGCheckButton(hframe3, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Decode raw data");
  hframe3->AddFrame(fchk,com->LayCC1);
  DoMap(fchk,&opt.decode,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");


  id = Plist.size()+1;
  sprintf(txt,"CheckDSP");
  fchk = new TGCheckButton(hframe3, txt, id);
  fchk->SetName(txt);
  fchk->SetToolTipText("Compare software pulse analysis vs DSP data");
  hframe3->AddFrame(fchk,com->LayCC1);
  DoMap(fchk,&opt.checkdsp,p_chk,0);
  fchk->Connect("Clicked()", "ParDlg", this, "DoChk()");
  //fchk->Connect("Clicked()", "ParDlg", this, "DoChkWrite()");


}

void ParParDlg::AddOpt(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, com->LayET3);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Number of used channels";
  tip2= "Number of threads (1 - no multithreading)";
  label="Number of channels/Number of threads";
  AddLine_opt(fF6,ww,&opt.Nchan,&opt.nthreads,tip1,tip2,label,k_int,k_int,1,MAX_CH,1,8);

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
  AddLine_opt(fF6,ww,&opt.usb_size,&opt.rbuf_size,tip1,tip2,label,k_int,k_int,
	      1,20000,1,64000,(char*) "DoNum_SetBuf()");
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
  frame->AddFrame(fF6, com->LayET3);

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

  tip1= "";
  tip2= "Main trigegr condition (cut).\nThis condition is used for selecting events which are written as decoded events\nSee Histograms->Cuts for making conditions\nUse this cut number as a main trigger condition.\nIf set to zero - write all events.";
  label="Main trigger";
  AddLine_opt(fF6,ww,NULL,&opt.maintrig,tip1,tip2,label,k_int,k_int,
	      0,0,0,MAXCUTS);

  /*
    tip1= "This condition is used for selecting events which are written as decoded events\nSee Histograms->Cuts for making conditions\nUse arithmetic/logic operations on existing cuts or leave it empty to record all events\nPress Enter or Check button to check if the syntaxis is correct";
    label="Main Trigger conditions";
    ww=150;
    AddLine_txt(fF6,ww,opt.maintrig, tip1, label);
  */

  fF6->Resize();

}

void ParParDlg::AddAna(TGCompositeFrame* frame) {
  
  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "NTOF Analysis", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, com->LayET3);

  tip1= "Ntof period (mks) (ignored if set to zero)";
  tip2= "Ntof start channel";
  label="Ntof period / start channel";
  AddLine_opt(fF6,ww,&opt.ntof_period,&opt.start_ch,tip1,tip2,label,k_r1,k_int,
	      0,1e9,0,255);

  tip1= "";
  tip2= "Dead time for the start input of the CRS (in 5ns samples)";
  label="Ntof dead time";
  AddLine_opt(fF6,ww,NULL,&cpar.DTW,tip1,tip2,label,k_r1,k_int,
	      0,0,1,2e9);

  tip1= "Ntof Flight path (in meters) for Ntof-Energy conversion";
  tip2= "Ntof Time offset (in mks) for Ntof-Energy conversion";
  label="Ntof Flpath / NTOF Zero";
  AddLine_opt(fF6,ww,&opt.Flpath,&opt.TofZero,tip1,tip2,label,k_r3,k_r3,
	      0,1e9,-1e9,1e9);

  fF6->Resize();

}

void ParParDlg::AddHist(TGCompositeFrame* frame2) {
  
  //int ww=70;
  
  //TGLabel* fLabel = new TGLabel(frame, "---  Histograms  ---");
  //frame->AddFrame(fLabel, com->LayET2);


  frame1d = new TGGroupFrame(frame2, "1D Histograms", kVerticalFrame);
  frame1d->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame1d, com->LayET3);

  // 2 column, n rows
  //frame->SetLayoutManager(new TGMatrixLayout(frame, 0, 3, 7));



  tip1= "Total aqcuisition time, in seconds";
  label="Time";
  AddLine_hist(frame1d,&opt.h_time,tip1,label);

  tip1= "Area of the pulse or energy, calibrated (see Channels->EM for calibration)";
  label="Area";
  AddLine_hist(frame1d,&opt.h_area,tip1,label);

  tip1= "Area w/o background, not calibrated";
  label="Area0";
  AddLine_hist(frame1d,&opt.h_area0,tip1,label);

  tip1= "Base line, not calibrated";
  label="Base";
  AddLine_hist(frame1d,&opt.h_base,tip1,label);

  tip1= "Slope1 (baseline)";
  label="Slope1";
  AddLine_hist(frame1d,&opt.h_slope1,tip1,label);

  tip1= "Slope2 (peak)";
  label="Slope2";
  AddLine_hist(frame1d,&opt.h_slope2,tip1,label);

  tip1= "Maximal pulse height (in channels)";
  label="Height";
  AddLine_hist(frame1d,&opt.h_hei,tip1,label);

  tip1= "Time of flight (relative to the starts - see Channels->St), in ns";
  label="Tof";
  AddLine_hist(frame1d,&opt.h_tof,tip1,label);

  tip1= "Neutron time of flight, in mks";
  label="Ntof";
  AddLine_hist(frame1d,&opt.h_ntof,tip1,label);

  tip1= "Neutron energy from NTOF, in eV";
  label="Etof";
  AddLine_hist(frame1d,&opt.h_etof,tip1,label);

  tip1= "Neutron wavelength from NTOF, in A";
  label="Ltof";
  AddLine_hist(frame1d,&opt.h_ltof,tip1,label);

  tip1= "Pulse period (distance between two consecutive pulses), in mks";
  label="Period";
  AddLine_hist(frame1d,&opt.h_per,tip1,label);

  tip1= "Pulse width";
  label="Width";
  AddLine_hist(frame1d,&opt.h_width,tip1,label);

  tip1= "Pulse width2";
  label="Width2";
  AddLine_hist(frame1d,&opt.h_width2,tip1,label);

  // tip1= "Pulse width3";
  // label="Width3";
  // AddLine_hist(frame1d,&opt.h_width3,tip1,label);

  tip1= "Average pulse shape";
  label="Mean_pulses";
  AddLine_mean(frame1d,&opt.h_pulse,tip1,label);

  tip1= "Derivative of Average pulse shape";
  label="Mean_deriv";
  AddLine_mean(frame1d,&opt.h_deriv,tip1,label);


  frame2d = new TGGroupFrame(frame2, "2D Histograms", kVerticalFrame);
  frame2d->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame2d, com->LayET3);


  TGHorizontalFrame* h2fr = new TGHorizontalFrame(frame2d);
  frame2d->AddFrame(h2fr,com->LayLT0);
  TGComboBox* cmb1=new TGComboBox(h2fr,0);
  h2fr->AddFrame(cmb1,com->LayLE0);
  cmb1->AddEntry("test1", 0);
  cmb1->Resize(60, 20);
  TGNumberEntry* fNum1 =
    new TGNumberEntry(h2fr, 0, 0, 111, k_int,
		      TGNumberFormat::kNEAAnyNumber,
		      TGNumberFormat::kNELLimitMinMax,0,MAX_CH-1);

  fNum1->SetWidth(45);
  h2fr->AddFrame(fNum1,com->LayLT2);

  TGComboBox* cmb2=new TGComboBox(h2fr,1);
  h2fr->AddFrame(cmb2,com->LayLE0);
  cmb2->AddEntry("test2", 1);
  cmb2->Resize(60, 20);

  TGNumberEntry* fNum2 =
    new TGNumberEntry(h2fr, 0, 0, 112, k_int,
		      TGNumberFormat::kNEAAnyNumber,
		      TGNumberFormat::kNELLimitMinMax,0,MAX_CH-1);

  fNum2->SetWidth(45);
  h2fr->AddFrame(fNum2,com->LayLT2);

  TGTextButton *fbut = new TGTextButton(h2fr,"Add",0);
  h2fr->AddFrame(fbut, com->LayCC1);
  fbut->Connect("Clicked()", "ParParDlg", this, "Add2d()");




  tip1= "2-dimensional histogram (area0-area1), calibrated (see Channels->EM for calibration)";
  label="A0A1";
  AddLine_hist(frame2d,&opt.h_a0a1,tip1,label);

  tip1= "2-dimensional histogram (Area_Base)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Base";
  AddLine_2d(frame2d,&opt.h_area_base,tip1,label);

  tip1= "2-dimensional histogram (Area_Slope1)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Sl1";
  AddLine_2d(frame2d,&opt.h_area_sl1,tip1,label);

  tip1= "2-dimensional histogram (Area_Slope2)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Sl2";
  AddLine_2d(frame2d,&opt.h_area_sl2,tip1,label);

  tip1= "2-dimensional histogram (Slope1-Slope2)\nMin Max are taken from the corresponding 1d histograms";
  label="Slope_12";
  AddLine_2d(frame2d,&opt.h_slope_12,tip1,label);

  tip1= "2-dimensional histogram (Area_Time)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Time";
  AddLine_2d(frame2d,&opt.h_area_time,tip1,label);

  tip1= "2-dimensional histogram (Area_Width)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width";
  AddLine_2d(frame2d,&opt.h_area_width,tip1,label);

  tip1= "2-dimensional histogram (Area_Width2)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width2";
  AddLine_2d(frame2d,&opt.h_area_width2,tip1,label);

  tip1= "2-dimensional histogram (Profilometer)";
  label="Prof";
  AddLine_prof(frame2d,&opt.h_prof,tip1,label);

  // tip1= "2-dimensional histogram (Area_Width3)\nMin Max are taken from the corresponding 1d histograms";
  // label="Area_Width3";
  // AddLine_2d(frame2d,&opt.h_area_width3,tip1,label);

  // tip1= "2-dimensional histogram (Width_12)\nMin Max are taken from the corresponding 1d histograms";
  // label="Width_12";
  // AddLine_2d(frame,&opt.h_width_12,tip1,label);

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

void ParParDlg::Add2d() {
  cout << "Add2d: " << endl;
  tip1= "2-dimensional histogram (Area_Width)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width4";
  AddLine_2d(frame2d,&opt.h_area_width,tip1,label);
  MapSubwindows();
  Layout();
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
    hfr1->AddFrame(fNum1,com->LayLT4);
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,com->LayLT4);
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
    hfr1->AddFrame(fNum2,com->LayLT4);
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,com->LayLT4);
  }

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,com->LayLT4);

}

/*
  void ParParDlg::AddLine_txt(TGGroupFrame* frame, int width, char* opt_fname, 
  const char* tip1, const char* label)
  {

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;

  id = Plist.size()+1;
  tTrig = new TGTextEntry(hfr1, opt_fname, id);
  tTrig->SetWidth(width);
  tTrig->SetMaxLength(20);

  DoMap(tTrig,opt_fname,p_txt,0);
  tTrig->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");
  tTrig->Connect("ReturnPressed()", "ParParDlg", this, "CheckFormula()");

  tTrig->SetToolTipText(tip1);
  hfr1->AddFrame(tTrig,com->LayLT4);
  
  TGTextButton* but = new TGTextButton(hfr1,"Check",7);
  but->Connect("Clicked()", "ParParDlg", this, "CheckFormula()");
  but->SetToolTipText(tip1);
  hfr1->AddFrame(but, com->LayET3);

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,com->LayLT4);

  }
*/

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
  hfr1->AddFrame(chk_hist,com->LayCC1);
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
  hfr1->AddFrame(fNum1,com->LayLT2);

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
  hfr1->AddFrame(fNum2,com->LayLT2);

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
  hfr1->AddFrame(fNum3,com->LayLT2);


  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,com->LayLT2);

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
  hfr1->AddFrame(chk_hist,com->LayCC1);
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
  hfr1->AddFrame(fNum1,com->LayLT2);

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
  hfr1->AddFrame(fNum1,com->LayLT2);

  //hint
  TGLabel* tt = new TGLabel(hfr1,"");
  //TGTextEntry* tt = new TGTextEntry(hfr1," ", 0);
  //tt->SetToolTipText("Min Max are taken from the corresponding 1d histograms");
  tt->ChangeOptions(tt->GetOptions()|kFixedWidth);
  tt->SetWidth(ww);
  hfr1->AddFrame(tt,com->LayLT2);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,com->LayLT2);
}

void ParParDlg::AddLine_prof(TGGroupFrame* frame, Hdef* hd,
			   const char* tip, const char* label) {

  double ww=90;
  //char name[20];

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  //double zz;
  int id;
  //int id0;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  //sprintf(name,"b_hist%d",id);
  //chk_hist->SetName(name);
  DoMap(chk_hist,&hd->b,p_chk,0);
  chk_hist->Connect("Clicked()", "ParParDlg", this, "DoCheckPulse()");
  hfr1->AddFrame(chk_hist,com->LayCC1);
  //id0=id;

  //hint
  TGLabel* tt = new TGLabel(hfr1,"");
  //TGTextEntry* tt = new TGTextEntry(hfr1," ", 0);
  //tt->SetToolTipText("Min Max are taken from the corresponding 1d histograms");
  tt->ChangeOptions(tt->GetOptions()|kFixedWidth);
  tt->SetWidth(ww);
  hfr1->AddFrame(tt,com->LayLT2);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,com->LayLT6);
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
  hfr1->AddFrame(chk_hist,com->LayCC1);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,com->LayLT5);

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
  //cout << pp.data << " " << opt.Base1[0] << " ";
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

/*
  void ParParDlg::CheckFormula() {
  //cout << "Check: " << opt.maintrig << endl;
  Pixel_t color;

  int ires = crs->Set_Trigger();
  if (ires==0) {// no formula
  gClient->GetColorByName("white", color);
  tTrig->SetBackgroundColor(color);
  }
  else if (ires==1) {// bad formula
  gClient->GetColorByName("red", color);
  tTrig->SetBackgroundColor(color);
  }
  else { //good formula
  gClient->GetColorByName("green", color);
  tTrig->SetBackgroundColor(color);
  }

  }
*/

void ParParDlg::Update() {
  ParDlg::Update();
  MapSubwindows();
  Layout();
}

//------ ChanParDlg -------

ChanParDlg::ChanParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{

  fDock->SetWindowName("Analysis");

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  TGCanvas *fCanvas = new TGCanvas(fMain,10,10);
  TGCompositeFrame *fcont = new TGCompositeFrame(fCanvas->GetViewPort(), 
						 100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont);
  fCanvas->SetScrolling(TGCanvas::kCanvasScrollHorizontal);
  fMain->AddFrame(fCanvas,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
					    kLHintsExpandX, 0, 0, 1, 1));


  //AddHeader();
  head_frame = new TGHorizontalFrame(fcont,10,10);
  fcont->AddFrame(head_frame,com->LayLT0);

  // Hsplitter
  TGVerticalFrame *vFrame = new TGVerticalFrame(fcont, 10, 10);
  fcont->AddFrame(vFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));
  TGHorizontalFrame *fH1 = new TGHorizontalFrame(vFrame, 10, 320);
  TGHorizontalFrame *fH2 = new TGHorizontalFrame(vFrame, 10, 205, kFixedHeight);
  vFrame->AddFrame(fH1, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY));
  TGHSplitter *hsplitter = new TGHSplitter(vFrame,2,2);
  hsplitter->SetFrame(fH2, kFALSE);
  vFrame->AddFrame(hsplitter, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  vFrame->AddFrame(fH2, new TGLayoutHints(kLHintsBottom | kLHintsExpandX));   




  fCanvas1 = new TGCanvas(fH1,10,10);
  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);
  fCanvas1->SetScrolling(TGCanvas::kCanvasScrollVertical);
  //fCanvas1->SetVsbPosition(100);

  //AddHeader();
  //head_frame = new TGHorizontalFrame(fcont1,10,10);
  //fcont1->AddFrame(head_frame,com->LayLT0);


  fH1->AddFrame(fCanvas1,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
					   kLHintsExpandX, 0, 0, 1, 1));


  fCanvas2 = new TGCanvas(fH2,10,10);
  fcont2 = new TGCompositeFrame(fCanvas2->GetViewPort(), 
  				100, 100, kVerticalFrame);
  fCanvas2->SetContainer(fcont2);
  fCanvas2->SetScrolling(TGCanvas::kCanvasScrollVertical);
  fH2->AddFrame(fCanvas2,new TGLayoutHints(kLHintsTop | kLHintsExpandY | 
					   kLHintsExpandX, 0, 0, 1, 1));

}

void ChanParDlg::AddChCombo(int i, int &id, int &kk, int &all) {
  //start AddChCombo
  char txt[255];

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
  cframe[i]->AddFrame(clab[i],com->LayCC0a);
  kk++;

  if (!nfld && Plist.size()) {
    nfld=Plist.size();
  }

  id = Plist.size()+1;


  if (i<=MAX_CH) {
    fCombo[i]=new TGComboBox(cframe[i],id);
    //fCombo->SetToolTipText(ttip1[kk]);
    cframe[i]->AddFrame(fCombo[i],com->LayCC0);
    kk++;

    for (int j = 0; j < MAX_TP-1; j++) {
      fCombo[i]->AddEntry(opt.ch_name[j], j+1);
    }
    fCombo[i]->AddEntry("Other", MAX_TP+1);

    fCombo[i]->Resize(tlen1[1], 20);

    if (i==MAX_CH) {
      fCombo[i]->AddEntry(" ", MAX_TP+2);
    }
    else {
      fCombo[i]->AddEntry("Copy", MAX_TP+2);
    }

    //if (i<=MAX_CH) {
    DoChanMap(fCombo[i],&opt.chtype[i],p_cmb,all,0,0);
    //fCombo[i]->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");
    fCombo[i]->Connect("ProcessedEvent(Event_t*)", "ParDlg", this, "DoCombo2(Event_t*)");
  }
  else {
    int i7=i-MAX_CH-1;
    TGTextEntry* tgtxt=new TGTextEntry(cframe[i], opt.ch_name[i7],id);
    tgtxt->SetName(TString::Format("%02dtxt",i7+1).Data());
    //tgtxt->SetHeight(20);
    tgtxt->SetWidth(tlen1[1]);
    tgtxt->SetMaxLength(5);
    //tgtxt->SetToolTipText(ttip1[kk]);
    //tgtxt->SetState(false);
    cframe[i]->AddFrame(tgtxt,com->LayCC0a);
    kk++;

    DoMap(tgtxt,opt.ch_name[i7],p_txt,0);
    tgtxt->Connect("TextChanged(char*)", "ParDlg", this, "DoTypes()");
    cframe[i]->SetBackgroundColor(tcol[i7]);
    clab[i]->SetBackgroundColor(tcol[i7]);
  }

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
  fNum->SetAlignment(kTextLeft);
  fNum->SetWidth(tlen7[kk]);
  fNum->SetHeight(20);
  fNum->Connect("TextChanged(char*)", "ChanParDlg", this, "DoChanNum()");
  fNum->SetToolTipText(ttip7[kk]);
  hframe1->AddFrame(fNum,com->LayCC0);

}

//------ CrsParDlg -------

CrsParDlg::CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
  fDock->SetWindowName("DAQ");  
  trig=0;
}

void CrsParDlg::Make_crspar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  for (int i=0;i<MAX_CH;i++) {
    AddLine_crs(i,fcont1);
    //cout << "crs: addLine1: " << Plist.size() << endl; 
  }

  AddLine_crs(MAX_CH,fcont2);

  for (int i=1;i<MAX_TP;i++) {
    AddLine_crs(MAX_CH+i,fcont2);
  }

  if (crs->module==2) {
    TGHorizontalFrame *hforce = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(hforce,com->LayLT0);

    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce, "", id);
    hforce->AddFrame(fforce,com->LayCC1);
    DoChanMap((TGWidget*)fforce,&cpar.forcewr,p_chk,0,0,0);
    fforce->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
    
    TGLabel* lforce = new TGLabel(hforce, "  Force_write all channels");
    hforce->AddFrame(lforce,com->LayLT0);
  }

}

void CrsParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,com->LayLT0);

  TGTextEntry* tt[ncrspar+1];

  for (int i=0;i<=ncrspar;i++) {
    // if (!strcmp(tlab1[i],"Trg") && crs->module<33) {
    //   continue;
    // }
    tt[i]=new TGTextEntry(head_frame, tlab1[i]);
    tt[i]->SetWidth(tlen1[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(ttip1[i]);
    tt[i]->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt[i],com->LayCC0);
  }

}

void CrsParDlg::AddLine_crs(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],com->LayLT0);

  // Pixel_t yellow,green;
  // gClient->GetColorByName("yellow", yellow);
  // gClient->GetColorByName("green", green);
  //cframe[i]->ChangeBackground(yellow);
  //cframe[i]->SetForegroundColor(green);

  AddChCombo(i,id,kk,all);

  id = Plist.size()+1;
  TGCheckButton *f_en = new TGCheckButton(cframe[i], "", id);
  DoChanMap(f_en,&cpar.enabl[i],p_chk,all,1,i);
  f_en->SetToolTipText(ttip1[kk]);
  f_en->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  cframe[i]->AddFrame(f_en,com->LayCC1);
  kk++;

  id = Plist.size()+1;
  TGCheckButton *f_inv = new TGCheckButton(cframe[i], "", id);
  DoChanMap(f_inv,&cpar.inv[i],p_chk,all,1,i);
  f_inv->SetToolTipText(ttip1[kk]);
  f_inv->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  cframe[i]->AddFrame(f_inv,com->LayCC1);
  kk++;

  id = Plist.size()+1;
  TGCheckButton *f_acdc = new TGCheckButton(cframe[i], "", id);
  DoChanMap(f_acdc,&cpar.acdc[i],p_chk,all,1,i);
  f_acdc->SetToolTipText(ttip1[kk]);
  f_acdc->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  cframe[i]->AddFrame(f_acdc,com->LayCC1);
  kk++;

  id = Plist.size()+1;
  TGCheckButton *f_pls = new TGCheckButton(cframe[i], "", id);
  DoChanMap(f_pls,&cpar.pls[i],p_chk,all,1,i);
  f_pls->SetToolTipText(ttip1[kk]);
  f_pls->Connect("Clicked()", "CrsParDlg", this, "DoCheck()");
  cframe[i]->AddFrame(f_pls,com->LayCC1);
  kk++;

  AddNumCrs(i,kk++,all,cframe[i],"smooth",&cpar.smooth[i]);
  AddNumCrs(i,kk++,all,cframe[i],"dt"    ,&cpar.deadTime[i]);
  AddNumCrs(i,kk++,all,cframe[i],"pre"   ,&cpar.preWr[i]);
  AddNumCrs(i,kk++,all,cframe[i],"len"   ,&cpar.durWr[i]);
  if (crs->module==2) 
    AddNumCrs(i,kk++,1,cframe[i],"gain"  ,&cpar.adcGain[i]);
  else
    AddNumCrs(i,kk++,all,cframe[i],"gain"  ,&cpar.adcGain[i]);

  // if (crs->module>=33)
  //   AddNumCrs(i,kk++,all,cframe[i],"trig" ,&cpar.trg[i]);
  // else
  //   kk++;
  AddNumCrs(i,kk++,all,cframe[i],"trig" ,&cpar.trg[i]);

  AddNumCrs(i,kk++,all,cframe[i],"deriv" ,&cpar.kderiv[i],&opt.Drv[i]);
  AddNumCrs(i,kk++,all,cframe[i],"thresh",&cpar.threshold[i],&opt.Thr[i]);


  if (i<=MAX_CH) {
    fStat2[i] = new TGTextEntry(cframe[i], "");
    fStat2[i]->ChangeOptions(fStat2[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat2[i]->SetState(false);
    fStat2[i]->SetToolTipText(ttip1[kk]);

    fStat2[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fStat2[i]->SetBackgroundColor(col);
    fStat2[i]->SetText(0);
    cframe[i]->AddFrame(fStat2[i],com->LayLT5);
  }
  kk++;

  if (i<=MAX_CH) {
    fStat3[i] = new TGTextEntry(cframe[i], "");
    fStat3[i]->ChangeOptions(fStat3[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat3[i]->SetState(false);
    fStat3[i]->SetToolTipText(ttip1[kk]);

    fStat3[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fStat3[i]->SetBackgroundColor(col);
    fStat3[i]->SetText(0);
    cframe[i]->AddFrame(fStat3[i],com->LayLT5);
  }
  /*
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
    cframe[i]->AddFrame(fStatBad[i],com->LayLT5);
    }
  */

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

  hframe1->AddFrame(fNum,com->LayCC0);

}

/*
  void CrsParDlg::ResetStatus() {

  TGString txt="0";

  for (int i=0;i<opt.Nchan;i++) {
  fStat[i]->SetText(txt);
  fStatBad[i]->SetText(txt);
  }

  fStat[MAX_CH]->SetText(txt);
  fStatBad[MAX_CH]->SetText(txt);

  }
*/
void CrsParDlg::UpdateStatus(int rst) {

  //static Long64_t allpulses2;
  //static Long64_t allpulses3;
  static Long64_t allbad;
  static double t1;
  static Long64_t npulses2o[MAX_CH];
  static Long64_t npulses3o[MAX_CH];
  static double rate2[MAX_CH];
  static double rate_all2;
  static double rate3[MAX_CH];
  static double rate_all3;

  if (rst) {
    //allpulses2=0;
    allbad=0;
    t1=0;
    memset(npulses2o,0,sizeof(npulses2o));
    memset(npulses3o,0,sizeof(npulses3o));
    memset(rate2,0,sizeof(rate2));
    //rate_all2=0;
    memset(rate3,0,sizeof(rate3));
    //rate_all3=0;
  }

  TGString txt;

  double dt = opt.T_acq - t1;

  //cout << "DT: " << dt << endl;
  allbad=0;
  rate_all2=0;
  rate_all3=0;
  if (dt>0.1) {
    for (int i=0;i<opt.Nchan;i++) {
      rate2[i] = (crs->npulses2[i]-npulses2o[i])/dt;
      npulses2o[i]=crs->npulses2[i];
      rate3[i] = (crs->npulses3[i]-npulses3o[i])/dt;
      npulses3o[i]=crs->npulses3[i];

      rate_all2+=rate2[i];
      rate_all3+=rate3[i];
      allbad+=crs->npulses_bad[i];
    }
    //rate_all2 = (crs->npulses-allpulses2)/dt;
    //allpulses2=crs->npulses;
    //rate_all3 = (crs->npulses-allpulses2)/dt;
    //allpulses2=crs->npulses;

    t1=opt.T_acq;
  }

  for (int i=0;i<opt.Nchan;i++) {
    txt.Form("%0.0f",rate2[i]);
    fStat2[i]->SetText(txt);
    txt.Form("%0.0f",rate3[i]);
    fStat3[i]->SetText(txt);
    //txt.Form("%d",crs->npulses_bad[i]);
    //fStatBad[i]->SetText(txt);
  }
  txt.Form("%0.0f",rate_all2);
  fStat2[MAX_CH]->SetText(txt);
  txt.Form("%0.0f",rate_all3);
  fStat3[MAX_CH]->SetText(txt);
  //txt.Form("%lld",allbad);
  //fStatBad[MAX_CH]->SetText(txt);

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

  if (pp.cmd && crs->b_acq && !trig) {
    crs->Command2(4,0,0,0);
    crs->SetPar();
    gzFile ff = gzopen("tmp.par","wb");
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
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

  if (pp.cmd && crs->b_acq && !trig) {
    crs->Command2(4,0,0,0);
    crs->SetPar();
    crs->Command2(3,0,0,0);
  }
#endif

}

// void CrsParDlg::Update() {
//   ParDlg::Update();
//   MapSubwindows();
//   Layout();
// }

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

  for (int i=1;i<MAX_TP;i++) {
    AddLine_Ana(MAX_CH+i,fcont2);
  }


}

void AnaParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,com->LayLT0);

  TGTextEntry* tt;

  for (int i=0;i<n_apar;i++) {
    tt=new TGTextEntry(head_frame, tlab2[i]);
    tt->SetWidth(tlen2[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip2[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,com->LayCC0);
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
    head_frame->AddFrame(tt,com->LayCC0);
  }

}

void AnaParDlg::AddLine_Ana(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],com->LayLT0);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

  AddChCombo(i,id,kk,all);

  id = Plist.size()+1;
  TGCheckButton *fst = new TGCheckButton(cframe[i], "", id);
  DoChanMap(fst,&opt.St[i],p_chk,all,0,0);
  fst->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fst->SetToolTipText(ttip2[kk]);
  cframe[i]->AddFrame(fst,com->LayCC1);
  kk++;

  // id = Plist.size()+1;
  // TGCheckButton *fmt = new TGCheckButton(cframe[i], "", id);
  // DoChanMap(fmt,&opt.Mrk[i],p_chk,all,0,0);
  // fmt->Connect("Clicked()", "ParDlg", this, "DoChk()");
  // fmt->SetToolTipText(ttip2[kk]);
  // cframe[i]->AddFrame(fmt,com->LayCC1);
  // kk++;


  tlen7 = (int*) tlen2;
  ttip7 = (char**) ttip2;

  AddNumChan(i,kk++,all,cframe[i],&opt.sS[i],0,99,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Delay[i],-999,999,p_fnum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.Base1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.Base2[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.Peak1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.Peak2[i],-999,3070,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.dT[i],0,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Pile[i],0,9999,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.timing[i],0,3,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.twin1[i],-99,99,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.twin2[i],-99,99,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.wwin1[i],-999,3070,p_inum);
  //AddNumChan(i,kk++,all,cframe[i],&opt.wwin2[i],-999,3070,p_inum);

  AddNumChan(i,kk++,all,cframe[i],&opt.E0[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.E1[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.E2[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Bc[i],-1e99,1e99,p_fnum);

  for (int j=0;j<NGRP;j++) {
    id = Plist.size()+1;
    TGCheckButton *gg = new TGCheckButton(cframe[i], "", id);
    DoChanMap(gg,&opt.Grp[i][j],p_chk,all,0,0);
    gg->Connect("Clicked()", "ParDlg", this, "DoChk()");
    gg->SetToolTipText(ttip_g[j]);
    cframe[i]->AddFrame(gg,com->LayCC1);
    kk++;
  }

}

//------ DspParDlg -------

DspParDlg::DspParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
  fDock->SetWindowName("Peaks");  
}

void DspParDlg::Make_DspPar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader();

  for (int i=0;i<MAX_CH;i++) {
    AddLine_Dsp(i,fcont1);
  }

  AddLine_Dsp(MAX_CH,fcont2);

  for (int i=1;i<MAX_TP;i++) {
    AddLine_Dsp(MAX_CH+i,fcont2);
  }


}

void DspParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,com->LayLT0);

  TGTextEntry* tt;

  for (int i=0;i<n_dpar;i++) {
    tt=new TGTextEntry(head_frame, tlab3[i]);
    tt->SetWidth(tlen3[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip3[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,com->LayCC0);
  }

}

void DspParDlg::AddLine_Dsp(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],com->LayLT0);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

  AddChCombo(i,id,kk,all);

  id = Plist.size()+1;
  TGCheckButton *fdsp = new TGCheckButton(cframe[i], "", id);
  DoChanMap(fdsp,&opt.dsp[i],p_chk,all,0,0);
  fdsp->Connect("Clicked()", "ParDlg", this, "DoChk()");
  fdsp->SetToolTipText(ttip2[kk]);
  cframe[i]->AddFrame(fdsp,com->LayCC1);
  kk++;

  // id = Plist.size()+1;
  // TGCheckButton *fpls = new TGCheckButton(cframe[i], "", id);
  // DoChanMap(fpls,&cpar.pls[i],p_chk,all,0,0);
  // fpls->Connect("Clicked()", "ParDlg", this, "DoChk()");
  // fpls->SetToolTipText(ttip2[kk]);
  // cframe[i]->AddFrame(fpls,com->LayCC1);
  // kk++;

  tlen7 = (int*) tlen3;
  ttip7 = (char**) ttip3;

  int amax=1023;
  if (crs->type_ch[i]==1)
    amax=511;

  //cout << "module: " << crs->module << " " << i << " " << crs->type_ch[i] << " " << amax << endl;

  AddNumChan(i,kk++,all,cframe[i],&opt.sTg[i],-1,5,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Drv[i],1,999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Thr[i],0,9999,p_inum);

  AddNumChan(i,kk++,all,cframe[i],&opt.Base1[i],-1024,amax,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Base2[i],-1024,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Peak1[i],-1024,amax,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Peak2[i],-1024,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.T1[i],-1024,amax,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.T2[i],-1024,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.W1[i],-1024,amax,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.W2[i],-1024,9999,p_inum);
}

//-------------------------
ErrFrame::ErrFrame(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{

  errflag=0;
  fDock->SetWindowName("Errors");
  /*
    TGLayoutHints* fLay1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

    fDock = new TGDockableFrame(this);
    AddFrame(fDock, fLay1);
    fDock->SetWindowName("Events");  
    //fDock->SetBackgroundColor(com->fRed);
    fDock->SetFixedSize(kFALSE);
  */
  
  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetBackgroundColor(com->fRed10);
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));
  
  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,com->LayEE0);
  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);

  const char* elab[MAX_ERR] = {
    "Bad buf start:",
    "Bad channel:",
    "Channel mismatch:",
    "Bad frmt:",
    "Zero data"
  };
  for (int i=0;i<MAX_ERR;i++) {
    TGHorizontalFrame* cframe = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(cframe,com->LayLT0);
    TGLabel* lb = new TGLabel(cframe,elab[i]);
    lb->SetTextJustify(kTextLeft);
    lb->ChangeOptions(lb->GetOptions()|kFixedSize);
    lb->Resize(120,20);
    cframe->AddFrame(lb,com->LayLT5);
    fErr[i] = new TGTextEntry(cframe, "");
    fErr[i]->ChangeOptions(fErr[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fErr[i]->SetState(false);
    //fErr[i]->SetMargins(10,0,0,0);
    //fErr[i]->SetTextJustify(kTextLeft|kTextCenterY);

    //////fErr[i]->SetToolTipText(ttip1[kk]);

    fErr[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fErr[i]->SetBackgroundColor(col);
    fErr[i]->SetText(0);
    //fbar[i]->SetParts(parts, nparts);
    //fBar2->SetParts(nparts);
    //fErr[i]->Draw3DCorner(kFALSE);
    //fbar[i]->DrawBorder();
    cframe->AddFrame(fErr[i],com->LayLT5);
  }

}

ErrFrame::~ErrFrame() {
}

void ErrFrame::ErrUpdate() {
  TGString txt;
  for (int i=0;i<MAX_ERR;i++) {
    txt.Form("%lld",crs->errors[i]);
    fErr[i]->SetText(txt);
    if (crs->errors[i]) {
      errflag=1;
      // if (errflag==0)
      // 	errflag=1;
      // else if (errfla==1)
      // 	errflag=2;
    }
  }
}

