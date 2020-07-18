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

extern Pixel_t fWhite;
extern Pixel_t fYellow;
extern Pixel_t fGreen;
extern Pixel_t fRed;
extern Pixel_t fCyan;
extern Pixel_t fMagenta;
extern Pixel_t fOrng;
extern Pixel_t fBlue;
extern Pixel_t fRed10;

extern Pixel_t fCol[7];// = {fYellow,fGreen,fRed,fRed10,fCyan,fOrng,fBlue};

//colors of ch_types
namespace CP {
  Float_t RGB[MAX_TP+3][3] = {
    {1,1,0}, //0 yellow
    {0,1,1}, //cyan
    {1,0.4,1}, // magenta
    {0.4,0.4,1}, // blue
    {1,0.6,0.0}, // orange
    {0,1,0}, //green
    //{1,0.7,0.4},
    {0.7,0.3,0.1},//brown
    {0.05,0.6,0.05}, //dark green
    {1,1,1}, //other
    {1,1,1}, //copy
    {1,1,1}  //swap
  };
}

extern ParParDlg *parpar;
extern DaqParDlg *daqpar;
extern AnaParDlg *anapar;
extern PikParDlg *pikpar;

const char* tool_type = "Channel type:\nOther - dummy type\nCopy - copy from channel to group\nSwap - first select swap, then change parameter, then change to new type";


const int ndaqpar=20;
const int tlen1[ndaqpar]={24,26,70,24,25,24,24,24,21,36,40,36,36,24,24,36,40,77,77,77};
const char* tlab1[ndaqpar]={"*","Ch","Type","on","Inv","AC","pls","dsp","hS","hD","Dt","Pre","Len","G","Trg","Drv","Thr","Pls/sec (sw)","Pls/sec (hw)","BadPulses"};
const char* ttip1[ndaqpar]={
  "Select",
  "Channel number",
  tool_type,
  "On/Off",
  "Inversion",
  "AC coupling",
  "Send/don't send pulse data",
  "Checked - use hardware pulse analysis (DSP)\nUnchecked - use software pulse analysis",
  "Hardware smoothing: Smooth=2^hS",
  "Hardware delay (in samples)",
  "Dead time - no new trigger on the current channel within dead time from the old trigger",
  "Number of samples before the trigger",
  "Total length of the pulse in samples",
  "Additional Gain",
  "Trigget type:\n0 - threshold crossing of pulse;\n1 - threshold crossing of derivative;\n2 - maximum of derivative;\n3 - rise of derivative;\n4 - fall of derivative (only for CRS-8/16)",
  "Parameter of derivative: S(i) - S(i-Drv) (0 - trigger on pulse)",
  "Trigger threshold",
  "Pulse rate (software)",
  "Pulse rate (hardware)",
  "Number of bad pulses"
};

const int n_apar=12;
const int tlen2[n_apar]={24,26,70,24,25,35,35,35,38,38,38,38};
const char* tlab2[n_apar]={"*","Ch","Type","St","sS","sD","dT","Pile","E0","E1","E2","Bc"};
const char* ttip2[n_apar]={
  "Select",
  "Channel number",
  tool_type,
  "Start channel - used for making TOF start\nif there are many start channels in the event, the earliest is used",
  "Software smoothing",
  "Software delay in ??samples?? (can be negative or positive)",
  "Dead-time window \nsubsequent peaks within this window are ignored",
  "Pileup window \nmultiple peaks within this window are marked as pileup",
  //"Timing mode (in 1st derivative):\n0 - threshold crossing (Pos);\n1 - left minimum (T1);\n2 - right minimum;\n3 - maximum in 1st derivative",
  "Energy calibration 0: [0]+[1]*x+[2]*x^2",
  "Energy calibration 1: [0]+[1]*x+[2]*x^2",
  "Energy calibration 2: [0]+[1]*x+[2]*x^2",
  "Baseline correction"
};


const int n_ppar=15;
const int tlen3[n_ppar]={24,26,70,24,26,32,40,40,40,42,42,40,40,40,40};
const char* tlab3[n_ppar]={"*","Ch","Type","dsp","sTg","Drv","Thr","Base1","Base2","Peak1","Peak2","T1","T2","W1","W2"};
const char* ttip3[n_ppar]={
  "Select",
  "Channel number",
  tool_type,
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

//extern Common* com;
extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  LayCC0   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,0,0);
  LayCC0a  = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,1,1);
  LayCC1   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 4, 4, 0, 0);
  LayET3   = new TGLayoutHints(kLHintsExpandX|kLHintsTop, 2, 2, 0, 0);
  LayLT0   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 0,0,0,0);
  LayLT2   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 0);
  LayLT3   = new TGLayoutHints(kLHintsLeft|kLHintsTop,1,1,1,1);
  LayLT4   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 11, 1, 1, 1);
  LayLC1   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 29, 1, 1, 1);
  LayLT5   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 1);
  LayLE0   = new TGLayoutHints(kLHintsLeft|kLHintsExpandY);
  LayEE0   = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY);




  //SetCleanup(kDeepCleanup);
  jtrig=0;
  notbuilt=true;
  pmax=0;

  fDock = new TGDockableFrame(this);
  AddFrame(fDock, LayEE0);
  //fDock->SetWindowName("Events");  
  fDock->SetFixedSize(kFALSE);

  // for (int i=0;i<nchtype;i++) {
  //   combotype[i]=i;
  // }

  nfld=0;

  Int_t cc;
  for (int i=0;i<MAX_TP+3;i++) {
    cc=TColor::GetColor(TColor::RGB2Pixel(CP::RGB[i][0],CP::RGB[i][1],CP::RGB[i][2]));
    tcol[i]=gROOT->GetColor(cc)->GetPixel();
  }

}
ParDlg::~ParDlg() {
  cout << "~ParDlg: " << this << endl;
  //CleanUp();
}

void ParDlg::DoMap(TGWidget* f, void* d, P_Def t, int all, byte cmd, void* d2) {
  pmap pp;
  pp.field = (TGWidget*) f;
  pp.data = d;
  pp.data2= d2;
  pp.type = t;
  pp.all=all;
  pp.cmd=cmd;
  //cout << "DoMap1: " << f << " " << d << " " << t << endl;
  Plist.push_back(pp);
  //cout << "DoMap2: " << f << " " << d << " " << t << endl;
}

bool ParDlg::Chk_all(int all, int i) {
  if (all==0) { //no action
    return false;
  }
  else if (all==1) { //all
    if (opt.chkall==0) { //*
      return (i<pmax) && opt.star[i];
    }
    else if (opt.chkall==1) { //all
      return i<pmax;
    }
    else { //ALL
      return true;
    }
  }
  else { //chtype
    return i<pmax && opt.chtype[i]==all-1;
  }
}

void ParDlg::SetNum(pmap pp, Double_t num) {
  //cout << "setnum: " << pp.data2 << " " << num << endl;
  if (pp.type==p_fnum) {
    *(Float_t*) pp.data = num;
    if (pp.data2) *(Float_t*) pp.data2 = num;
    //cout << "setpar1: " << *(Float_t*) pp.data << endl;
  }
  else if (pp.type==p_inum) {
    *(Int_t*) pp.data = num;
    if (pp.data2) *(Int_t*) pp.data2 = num;
    //cout << "setnum2: " << *(Int_t*) pp.data << endl;
  }
  else {
    cout << "(SetNum) Wrong type: " << pp.type << endl;
  }
}

void ParDlg::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  pmap pp = Plist[id-1];

  SetNum(pp,te->GetNumber());
  UpdateField(id-1);

  //cout << "Donum: " << id << " " << (int) pp.all << " " << te->GetNumber() << endl;
  //return;
  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld; //column number
      for (int i=0;i<pmax+MAX_TP+1;i++) { //pmax+all+MAX_TP
	if (Chk_all(pp.all,i)) {
	  pmap p2 = Plist[i*nfld+kk];
	  SetNum(p2,te->GetNumber());
	  UpdateField(i*nfld+kk);
	}
      }
    }
  }

}

void ParDlg::DoDaqNum() {
  jtrig++;
  ParDlg::DoNum();
  jtrig--;

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  pmap* pp = &Plist[id-1];

#ifdef CYUSB
  // cout << "DoDaqNum(): " << jtrig << " " << (int)pp->cmd << endl;
  if (pp->cmd && crs->b_acq && !jtrig) {
    crs->Command2(4,0,0,0);
    crs->SetPar();
    gzFile ff = gzopen("last.par","wb");
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
    crs->Command2(3,0,0,0);
  }
#endif

  int act = pp->cmd>>4;
  if (act==1 && crs->b_stop) {
    // cout << "Act_SetBuf: " << endl;
    crs->DoReset();
  }

  if (act==2 && crs->b_stop) {
    // cout << "Act_HiReset: " << endl;
    HiFrm->HiReset();
  }

}

void ParDlg::SetChk(pmap pp, Bool_t num) {
  //cout << "setchk1: " << num << endl;
  if (pp.type==p_chk) {
    *(Bool_t*) pp.data = (Bool_t) num;
  }
  else {
    cout << "(DoChk) Wrong type: " << (int) pp.type << endl;
  }
}

void ParDlg::DoChk(Bool_t on) {

  //TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = ((TGCheckButton*) gTQSender)->WidgetId();
  pmap pp = Plist[id-1];

  SetChk(pp,on);
  UpdateField(id-1);

  //te->SetState(kButtonDisabled);
  //cout << "Dochk: " << id << " " << (int) pp.all << " " << on << endl;

  if (pp.all>0) {
    if (nfld) {
      int kk = (id-1)%nfld;
      for (int i=0;i<pmax+MAX_TP+1;i++) { //pmax+all+MAX_TP
	if (Chk_all(pp.all,i)) {
	  pmap p2 = Plist[i*nfld+kk];
	  SetChk(p2,on);
	  UpdateField(i*nfld+kk);
	}
      }
    }
  }

}

void ParDlg::DoDaqChk(Bool_t on) {
  jtrig++;
  ParDlg::DoChk(on);
  jtrig--;

#ifdef CYUSB
  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();
  pmap pp = Plist[id-1];

  // cout << "DoDaqChk(): " << jtrig << " " << (int)pp.cmd << endl;

  if (pp.cmd && crs->b_acq && !jtrig) {
    crs->Command2(4,0,0,0);
    crs->SetPar();
    gzFile ff = gzopen("last.par","wb");
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
    crs->Command2(3,0,0,0);
  }
#endif

}

void ParDlg::DoCheckHist(Bool_t on) {
  if (!crs->b_stop) return;

  jtrig++;
  DoChk(on);
  jtrig--;

  // cout << "DoCheckHist(): " << jtrig << endl;

  if (!jtrig) {
    HiFrm->HiReset();
  }
}

void ParDlg::DoOpen() {

  const char *dnd_types[] = {
    "all files",     "*",
    // ".raw files",     "*.raw",
    // ".dec files",     "*.dec",
    // ".root files",     "*.root",
    // ".gz files",     "*.gz",
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

  int sel = te->GetSelected(); //sel starts from 1

  pmap pp = Plist[id-1];

  int nline = id/nfld;

  // bool cp=false; //copy from group to channel
  int old_type=opt.chtype[nline];
	
  //cout << "DoCombo: " << sel << " " << nline << " " << pmax << " " << opt.chtype[nline] << endl;

  if (nline < pmax) { //normal channels

    if (sel==MAX_TP+2) { //sel==Copy+1
      int old=*(Int_t*) pp.data;

      if (opt.chtype[nline]<=MAX_TP) { // normal groups
	// cout << "invcopy: " << old << " " << nline << endl;
	// inverse copy (channel to group)
	daqpar->CopyParLine(-old,nline);
	anapar->CopyParLine(-old,nline);
	pikpar->CopyParLine(-old,nline);
      }

      te->Select(old,false);
      return;
    }

    SetCombo(pp,sel);
    if (old_type!=MAX_TP+3) { //if old type is not swap, then copy
      daqpar->CopyParLine(sel,nline);
      anapar->CopyParLine(sel,nline);
      pikpar->CopyParLine(sel,nline);
    }
    else { //if swap, just change background
      clab[nline]->ChangeBackground(tcol[sel-1]);
      cframe[nline]->ChangeBackground(tcol[sel-1]);
    }

  }

  // UpdateField(id-1);

  // "All" group
  if (pp.all==1) {
    //cout << "all: " << nfld << " " << sel << endl;
    int kk = (id-1)%nfld;
    if (nfld) { // не знаю, зачем нужна эта проверка...
      if (sel<=MAX_TP+1) { //normal group & other: select and copy line
	for (int i=0;i<pmax;i++) {
	  if (Chk_all(pp.all,i)) {
	    pmap p2 = Plist[i*nfld+kk];
	    SetCombo(p2,sel);
	    TGComboBox *te2 = (TGComboBox*) p2.field;
	    te2->Select(sel,false);

	    daqpar->CopyParLine(sel,i);
	    anapar->CopyParLine(sel,i);
	    pikpar->CopyParLine(sel,i);
	  }
	} //for
      } // if sel<=MAX_TP
    } //if nfld
  } //if pp.all

  //cout << "DoCombo chtype: " << opt.chtype[0] << " " << id << " " << cp << endl;

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

void ParDlg::DoOneType(int i) {

  int i0=i-1;
  if (i0==0) i0=-1;

  for (int j=0;j<=MAX_CH;j++) {
    if (j<pmax || j==MAX_CH) {
      int sel = fCombo[j]->GetListBox()->GetSelected();
      // cout << "Dotypes: " << j << " " << i << " " << opt.ch_name[i-1] << " " << sel << endl;
      fCombo[j]->RemoveEntry(i);
      fCombo[j]->InsertEntry(opt.ch_name[i-1],i,i0);
      if (sel==i)
	fCombo[j]->Select(sel,false);
    }
  }
//   cout << "DoTypes3: " << id << " " << pmax << endl;
}

void ParDlg::DoTypes() {

  TGTextEntry *te = (TGTextEntry*) gTQSender;

  DoTxt();

  int i = TString(te->GetName())(0,1).String().Atoi();

  daqpar->DoOneType(i);
  anapar->DoOneType(i);
  pikpar->DoOneType(i);
  //   cout << "DoTypes3: " << id << " " << pmax << endl;
}

void ParDlg::DoAll() {

  opt.chkall+=1;
  if (opt.chkall>2) opt.chkall=0;

  Int_t id = ((TGCheckButton*) gTQSender)->WidgetId();
  UpdateField(id-1);

}

void ParDlg::CopyParLine(int sel, int line) {
  // cout << "CopyParLine: " << sel << " " << line << " " << MAX_CH-sel << endl;
  if (sel<0) { //inverse copy - from current ch to group
    //return;
    for (int j=0;j<nfld;j++) {
      int a = line*nfld+j;
      int b = (pmax-sel)*nfld+j;
      CopyField(a,b);
    }
    clab[line]->ChangeBackground(tcol[-sel-1]);
    cframe[line]->ChangeBackground(tcol[-sel-1]);
  }
  else if (sel<=MAX_TP) { //ZZ //normal copy from group to current ch
    // cout << "CopyParLine: " << line << " " << sel << " " << pmax+sel << " " << nfld << endl;
    for (int j=0;j<nfld;j++) {
      int b = line*nfld+j;
      int a = (pmax+sel)*nfld+j;
      CopyField(a,b);
    }
    clab[line]->ChangeBackground(tcol[sel-1]);
    cframe[line]->ChangeBackground(tcol[sel-1]);
  }
  else { //if (sel>MAX_TP) { //other,swap - just change color
    clab[line]->ChangeBackground(tcol[MAX_TP]);
    cframe[line]->ChangeBackground(tcol[MAX_TP]);
  }
}

void ParDlg::CopyField(int from, int to) {

  pmap* p1 = &Plist[from];
  pmap* p2 = &Plist[to];

  // cout << "CopyField: " << from << " " << to << " "
  //      << (int) p1->type << " " << (int) p2->type << " " << p_cmb << endl;

  //skip combo
  if (p1->type==p_cmb || p2->type==p_cmb) {
    return;
  }

  if (p1->type!=p2->type) {
    cout << "CopyField bad type: " << from << " " << to << " "
	 << (int) p1->type << " " << (int) p2->type << endl;
    return;
  }
	
  switch (p1->type) {
  case p_inum: {
    TGNumberEntryField *te = (TGNumberEntryField*) p2->field;
    te->SetNumber(*(Int_t*) p1->data);
    *(Int_t*) p2->data = *(Int_t*) p1->data;
  }
    break;
  case p_fnum: {
    TGNumberEntryField *te = (TGNumberEntryField*) p2->field;
    te->SetNumber(*(Float_t*) p1->data);
    *(Float_t*) p2->data = *(Float_t*) p1->data;
  }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) p2->field;
    Bool_t bb = *(Bool_t*) p1->data;
    te->SetState((EButtonState) bb, true);
    *(Bool_t*) p2->data = *(Bool_t*) p1->data;
  }
    break;
  default:
    cout << "unknown pp->type: " << p1->type << endl;
  } //switch  
  //cout << "copyfield2: " << p1 << " " << p2 << endl;
}

// void ParDlg::NumField1(int nn, bool bb) {
// 	EnableField(nn,bb);
// }

void ParDlg::UpdateField(int nn) {

  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;
  pmap* pp = &Plist[nn];
	
  TQObject* tq = (TQObject*) pp->field;
  tq->BlockAllSignals(true);

  Int_t val=0;
  Bool_t bb;
  //cout << "updatefield0: " << nn << endl;
  //cout << "updatefield: " << nn << " " << pp->type << " " << p_chk << " " << p_cmb << " " << gROOT << endl;

  switch (pp->type) {
  case p_inum: {
    TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
    Int_t *dat = (Int_t*) pp->data;
    val=*dat;
    if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      *dat = te->GetNumMax();
    }
    if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      *dat = te->GetNumMin();
    }
    te->SetNumber(*dat);
  }
    break;
  case p_fnum: {
    TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
    Float_t *dat = (Float_t*) pp->data;
    val = (*dat==0) ? 0 : 1;
    if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      *dat = te->GetNumMax();
    }
    if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      *dat = te->GetNumMin();
    }
    te->SetNumber(*dat);

    // if (TString(te->GetName()).EqualTo("Tstop",TString::kIgnoreCase)) {
    // 	if (opt.Tstop) {
    // 		te->ChangeBackground(gROOT->GetColor(kYellow)->GetPixel());
    // 	}
    // 	else {
    // 		te->ChangeBackground(gROOT->GetColor(kWhite)->GetPixel());
    // 	}
    // }
  }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) pp->field;
    EButtonState st = te->GetState();
    te->SetEnabled();
    bb = *(Bool_t*) pp->data;
    val=bb;
    te->SetState((EButtonState) bb);
    if (st==kButtonDisabled) {
      te->SetEnabled(false);
    }
  }
    break;
  case p_cmb: {
    //cout << "cmb1: " << endl;
    TGComboBox *te = (TGComboBox*) pp->field;
    int line = nn/nfld;
    int sel = *(Int_t*) pp->data;

    // update group names in combo
    for (int i=0;i<MAX_TP;i++) { //ZZ
      TGTextLBEntry* ent=
	(TGTextLBEntry*)te->GetListBox()->GetEntry(i+1); //ZZ
      ent->SetText(new TGString(opt.ch_name[i]));
      //ent->SetTitle(opt.ch_name[i]);
    }
    te->Layout();

    // cout << "cmb2: " << sel << " " << line << " " << tcol[sel-1] << endl;
    if (line<pmax) {
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
    //te->SetText(((TString*)pp->data)->Data());
  }
    break;
  case p_but:
    if (pp->all) {
      TGTextButton *te = (TGTextButton*) pp->field;
      if (opt.chkall==0) {
	te->SetText("*");
	cbut->ChangeBackground(fMagenta);
	cframe[MAX_CH]->ChangeBackground(fMagenta);
      }
      else if (opt.chkall==1) {
	te->SetText("all");
	cbut->ChangeBackground(fGreen);
	cframe[MAX_CH]->ChangeBackground(fGreen);
      }
      else {
	te->SetText("ALL");
	cbut->ChangeBackground(fBlue);
	cframe[MAX_CH]->ChangeBackground(fBlue);
      }
    }
    break;
  default:
    cout << "unknown pp->type: " << pp->type << endl;
  } //switch

  //do some action, if needed
  int act = pp->cmd>>4;
  if (act && crs->b_stop && pp->type==p_chk) {
    // cout << "Act_Chk: " << act << endl;
    switch (act) {
    case 2: //2d hist
    case 3: //1d hist
      for (int i=0;i<act;i++) {
	EnableField(nn+i+1,bb);
      }
      break;
    case 4: {
      for (int i=0;i<4;i++) {
	EnableField(nn+i+1,bb);
      }
      pp = &Plist[nn+5];
      TGCheckButton *te3 = (TGCheckButton*) pp->field;
      te3->SetEnabled(bb);
      break;
    }
    default:;
    }
  }

  //change color, if needed
  int col=(pp->cmd & 0xF)>>1;
  if (col) {
    switch (pp->type) {
    case p_inum:
    case p_fnum: {
      // cout << "c_num: " << col << endl;
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      if (val) {
	te->ChangeBackground(fCol[col-1]);
      }
      else {
	te->ChangeBackground(fWhite);
      }
      break;
    }
    case p_chk: {
      // cout << "c_chk: " << col << endl;
      TGCheckButton *te = (TGCheckButton*) pp->field;
      if (val) {
	te->ChangeBackground(fCol[col-1]);
      }
      else {
	te->ChangeBackground(gROOT->GetColor(18)->GetPixel());
      }
      break;
    }
    default:;
    }
  }


  tq->BlockAllSignals(false);

}

void ParDlg::Update() {
  //cout << "update1: " << Plist.size() << endl;
  for (UInt_t i=0;i<Plist.size();i++) {
    UpdateField(i);
  }
  //cout << "update2: " << Plist.size() << endl;  
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
    {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      TString str = TString(te->GetParent()->GetName());
      //cout << "Field1: " << str << endl;
      if (str.Contains("NumberEntry")) {
	//cout << "Field2: " << str << endl;
	TGNumberEntry *ww = (TGNumberEntry*) te->GetParent();
	ww->SetState(state);
      }
      else {
	te->SetState(state);
      }
    }
    break;
  case p_txt:
    {
      TGTextEntry *te = (TGTextEntry*) pp->field;
      te->SetEnabled(state);
    }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) pp->field;
    te->SetEnabled(state);
  }
    break;
  case p_cmb: {
    TGComboBox *te = (TGComboBox*) pp->field;
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

/*
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
*/

//------ ParParDlg -------

ParParDlg::ParParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
  k_int=TGNumberFormat::kNESInteger;
  k_r0=TGNumberFormat::kNESReal;
  k_r1=TGNumberFormat::kNESRealOne;

  k_mon=TGNumberFormat::kNESMDayYear;

  //TGNumberFormat::EAttribute k_nneg=TGNumberFormat::kNEANonNegative;
  //TGNumberFormat::EAttribute k_any=TGNumberFormat::kNEAAnyNumber;

  //id_tstop=-1;

  fDock->SetWindowName("Parameters");  

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,LayEE0);

  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);

  hor = new TGSplitFrame(fcont1,10,10);
  fcont1->AddFrame(hor,LayEE0);
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
		       Int_t* compr, Bool_t* rflag) {
  int id;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(frame,10,10);
  frame->AddFrame(hframe1,LayLT0);

  id = Plist.size()+1;
  TGCheckButton *fchk = new TGCheckButton(hframe1, txt, id);
  // fchk->SetName(txt);
  fchk->ChangeOptions(fchk->GetOptions()|kFixedWidth);
  fchk->SetWidth(230);

  hframe1->AddFrame(fchk,LayCC1);
  DoMap(fchk,opt_chk,p_chk,0,7<<1);
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");

  //fchk->SetState(kButtonDown,false);

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hframe1, *compr, 2, id, k_int, 
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   0,9);
  hframe1->AddFrame(fNum1,LayCC1);
  DoMap(fNum1->GetNumberEntry(),compr,p_inum,0);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoNum()");

  fNum1->GetNumberEntry()->SetToolTipText("Compression factor [0-9]: 0 - no compression (fast); 9- maximum compression (slow)");
  TGLabel* fLabel = new TGLabel(hframe1, "compr.");
  hframe1->AddFrame(fLabel,LayCC1);

  //raw_flag
  if (rflag) {
    id = Plist.size()+1;
    TGCheckButton *fchk2 = new TGCheckButton(hframe1, "Proc", id);
    fchk2->SetToolTipText("Checked - write processed events; unchecked - write direct raw stream");
    //fchk2->SetName(txt);
    //fchk2->ChangeOptions(fchk2->GetOptions()|kFixedWidth);
    //fchk2->SetWidth(230);

    hframe1->AddFrame(fchk2,LayCC1);
    DoMap(fchk2,rflag,p_chk,0,7<<1);
    fchk2->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  }

}

void ParParDlg::AddFiles(TGCompositeFrame* frame) {
  int id;
  char txt[99];

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Files", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayET3);

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe1,LayLT0);

  TGLabel* fLabel = new TGLabel(hframe1, "Filename:");
  hframe1->AddFrame(fLabel,LayCC1);


  id = Plist.size()+1;
  TGTextButton *fbut = new TGTextButton(hframe1,"Select...",id);
  hframe1->AddFrame(fbut, LayCC1);
  DoMap(fbut,opt.Filename,p_but,0);
  fbut->Connect("Clicked()", "ParDlg", this, "DoOpen()");

  TGHorizontalFrame *hframe2 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe2,LayLT0);

  //strcpy(opt.fname_raw,"raw32.gz");
  id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(hframe2,opt.Filename, id);
  tt->SetDefaultSize(380,20);
  tt->SetMaxLength(sizeof(opt.Filename)-1);
  //tt->SetWidth(590);
  //tt->SetState(false);
  hframe2->AddFrame(tt,LayCC0);
  DoMap(tt,opt.Filename,p_txt,0);
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");


  AddChk(fF6,"Write raw data [Filename].raw",&opt.raw_write,&opt.raw_compr,&opt.raw_flag);
  AddChk(fF6,"Write decoded data [Filename].dec",&opt.dec_write,&opt.dec_compr,0);
  AddChk(fF6,"Write root histograms [Filename].root",&opt.root_write,&opt.root_compr,0);



  TGCheckButton *fchk;
  TGHorizontalFrame *hframe3 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe3,LayLT0);

  id = Plist.size()+1;
  sprintf(txt,"Decode");
  fchk = new TGCheckButton(hframe3, txt, id);
  // fchk->SetName(txt);
  fchk->SetToolTipText("Decode raw data");
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.decode,p_chk,0);
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");

  id = Plist.size()+1;
  sprintf(txt,"CheckDSP");
  fchk = new TGCheckButton(hframe3, txt, id);
  // fchk->SetName(txt);
  fchk->SetToolTipText("Compare software pulse analysis vs DSP data");
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.checkdsp,p_chk,0,5<<1);
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");

}

void ParParDlg::AddOpt(TGCompositeFrame* frame) {

  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayET3);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Number of used channels";
  tip2= "Number of threads (1 - no multithreading)";
  label="Number of channels/Number of threads";
  AddLine_opt(fF6,ww,&opt.Nchan,&opt.nthreads,tip1,tip2,label,k_int,k_int,1,MAX_CH,1,8);

  tip1= "Analysis start (in sec) - only for analyzing files";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine_opt(fF6,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_r0,k_r0,0,0,0,0,3<<1,1<<1);

  // TGTextEntry *te = (TGTextEntry*) Plist.back().field;
  // te->SetName("Tstop");
  //cout << "tstop: " << te->GetName() << endl;
  //id_tstop = Plist.size();

  tip1= "";
  tip2= "Delay between drawing events (in msec)";
  label="DrawEvent delay";
  AddLine_opt(fF6,ww,NULL,&opt.tsleep,tip1,tip2,label,k_int,k_int,100,10000,100,10000);

  tip1= "Size of the USB buffer in kilobytes (1024 is OK)\n"
    "Small for low count rate; large for high count rate";
  tip2= "Size of the READ buffer in kilobytes\n"
    "As large as possible for faster speed, but may run out of memory";
  label="USB/READ buffer size";
  AddLine_opt(fF6,ww,&opt.usb_size,&opt.rbuf_size,tip1,tip2,label,k_int,k_int,
	      1,2048,1,64000,0,1<<4);

  tip1= "Maximal size of the event list:\nNumber of events available for viewing in the Events Tab";
  tip2= "Event lag:\nMaximal number of events which may come delayed from another channel";
  label="Event_list size / Event lag";
  AddLine_opt(fF6,ww,&opt.ev_max,&opt.ev_min,tip1,tip2,label,k_int,k_int,1,1000000,1,1000000);

  tip1= "[CRS-8/16] Sampling rate, MHz (0: 100, 1: 50, 2: 25 .. 14: ~0.006)";
  tip2= "[CRS-8/16] Soft start (imitator) period: (0->use LEMO START input)\n"
    "If non-zero, LEMO START input is blocked\n"
    "1: 0.08, 2: 0.17, 3: 0.33, 4: 0.67, 5: 1.34, 6: 2.68, 7: 5.36, 8: 10.7";
  label="Sampling Rate / Start period";
  // cout << "Smpl: " << cpar.Smpl << endl;
  AddLine_opt(fF6,ww,&cpar.Smpl,&cpar.St_Per,tip1,tip2,label,k_int,k_int,0,14,0,7,2<<1,6<<1);

  tip1= "[CRS-8/16] Force trigger on all active channels from START signal.\n"
    "Normal trigger is disabled";
  tip2= "START input channel dead time (in samples)";
  label="START trigger/START dead time";
  AddLine_opt(fF6,ww,&cpar.St_trig,&cpar.DTW,tip1,tip2,label,k_mon,k_int,
	      0,0,1,2e9,(3<<1)+1,1);


  if (crs->module<41 || crs->module>70) {
    int id = Plist.size()-2;
    EnableField(id,false);
    EnableField(id-1,false);
    EnableField(id-2,false);
  }

  fF6->Resize();

}

void ParParDlg::AddLogic(TGCompositeFrame* frame) {

  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Event Logic", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayET3);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Coincidence window for making events (in samples)";
  tip2= "Veto window (in samples): \nsubsequent pulses from the same channel coming within this window are ignored";
  label="Coincidence (smp), veto (smp)";
  AddLine_opt(fF6,ww,&opt.tgate,&opt.tveto,tip1,tip2,label,k_int,k_int,0,10000,0,1000);

  tip1= "Minimal multiplicity";
  tip2= "Maximal multiplicity";
  label="Multiplicity (min, max)";
  AddLine_opt(fF6,ww,&opt.mult1,&opt.mult2,tip1,tip2,label,k_int,k_int,
	      //1,MAX_CH,1,MAX_CH);
	      1,9999,1,9999);

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
  frame->AddFrame(fF6, LayET3);

  tip1= "Ntof period (mks) (ignored if set to zero)";
  tip2= "Ntof start channel";
  label="Ntof period / start channel";
  AddLine_opt(fF6,ww,&opt.ntof_period,&opt.start_ch,tip1,tip2,label,k_r1,k_int,
	      0,1e9,0,255);

  tip1= "Ntof Flight path (in meters) for Ntof-Energy conversion";
  tip2= "Ntof Time offset (in mks) for Ntof-Energy conversion";
  label="Ntof Flpath / NTOF Zero";
  AddLine_opt(fF6,ww,&opt.Flpath,&opt.TofZero,tip1,tip2,label,k_r0,k_r0,
	      0,1e9,-1e9,1e9);

  fF6->Resize();

}

void ParParDlg::AddHist(TGCompositeFrame* frame2) {

  //int ww=70;

  //TGLabel* fLabel = new TGLabel(frame, "---  Histograms  ---");
  //frame->AddFrame(fLabel, LayET2);


  frame1d = new TGGroupFrame(frame2, "1D Histograms", kVerticalFrame);
  frame1d->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame1d, LayET3);

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

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame1d);
  frame1d->AddFrame(hfr1);

  tip1= "Average pulse shape";
  label="Mean_pulses";
  AddLine_mean(hfr1,&opt.h_pulse,tip1,label);

  tip1= "Derivative of Average pulse shape";
  label="Mean_deriv";
  AddLine_mean(hfr1,&opt.h_deriv,tip1,label);

  tip1= "1d and 2d histograms for Profilometer";
  label="Profilometer";
  AddLine_prof(frame1d,&opt.h_prof,tip1,label);

  // tip1= "2-dimensional histogram (Profilometer)";
  // label="Profilometer";
  // AddLine_prof(frame2d,&opt.h_prof,tip1,label);


  frame2d = new TGGroupFrame(frame2, "2D Histograms", kVerticalFrame);
  frame2d->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame2->AddFrame(frame2d, LayET3);


  TGHorizontalFrame* h2fr = new TGHorizontalFrame(frame2d);
  frame2d->AddFrame(h2fr,LayLT0);
  TGComboBox* cmb1=new TGComboBox(h2fr,0);
  h2fr->AddFrame(cmb1,LayLE0);
  cmb1->AddEntry("test1", 0);
  cmb1->Resize(60, 20);
  TGNumberEntry* fNum1 =
    new TGNumberEntry(h2fr, 0, 0, 111, k_int,
		      TGNumberFormat::kNEAAnyNumber,
		      TGNumberFormat::kNELLimitMinMax,0,MAX_CH-1);

  fNum1->SetWidth(45);
  h2fr->AddFrame(fNum1,LayLT2);

  TGComboBox* cmb2=new TGComboBox(h2fr,1);
  h2fr->AddFrame(cmb2,LayLE0);
  cmb2->AddEntry("test2", 1);
  cmb2->Resize(60, 20);

  TGNumberEntry* fNum2 =
    new TGNumberEntry(h2fr, 0, 0, 112, k_int,
		      TGNumberFormat::kNEAAnyNumber,
		      TGNumberFormat::kNELLimitMinMax,0,MAX_CH-1);

  fNum2->SetWidth(45);
  h2fr->AddFrame(fNum2,LayLT2);

  TGTextButton *fbut = new TGTextButton(h2fr,"Add",0);
  h2fr->AddFrame(fbut, LayCC1);
  fbut->Connect("Clicked()", "ParParDlg", this, "Add2d()");


  // tip1= "2-dimensional histogram (Profilometer)";
  // label="Profilometer";
  // AddLine_prof(frame2d,&opt.h_prof,tip1,label);

  tip1= "2-dimensional histogram AreaX-AreaY)";
  label="AXAY";
  AddLine_2d(frame2d,&opt.h_axay,tip1,label,2);

  tip1= "2-dimensional histogram (Area_Base)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Base";
  AddLine_2d(frame2d,&opt.h_area_base,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Slope1)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Sl1";
  AddLine_2d(frame2d,&opt.h_area_sl1,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Slope2)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Sl2";
  AddLine_2d(frame2d,&opt.h_area_sl2,tip1,label,1);

  tip1= "2-dimensional histogram (Slope1-Slope2)\nMin Max are taken from the corresponding 1d histograms";
  label="Slope_12";
  AddLine_2d(frame2d,&opt.h_slope_12,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Time)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Time";
  AddLine_2d(frame2d,&opt.h_area_time,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Width)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width";
  AddLine_2d(frame2d,&opt.h_area_width,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Width2)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width2";
  AddLine_2d(frame2d,&opt.h_area_width2,tip1,label,1);

  tip1= "2-dimensional histogram (Area_Ntof)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Ntof";
  AddLine_2d(frame2d,&opt.h_area_ntof,tip1,label,1);

  // tip1= "2-dimensional histogram (Area_Width3)\nMin Max are taken from the corresponding 1d histograms";
  // label="Area_Width3";
  // AddLine_2d(frame2d,&opt.h_area_width3,tip1,label,1);

  // tip1= "2-dimensional histogram (Width_12)\nMin Max are taken from the corresponding 1d histograms";
  // label="Width_12";
  // AddLine_2d(frame,&opt.h_width_12,tip1,label,1);

}

void ParParDlg::Add2d() {
  cout << "Add2d: " << endl;
  tip1= "2-dimensional histogram (Area_Width)\nMin Max are taken from the corresponding 1d histograms";
  label="Area_Width4";
  AddLine_2d(frame2d,&opt.h_area_width,tip1,label,1);
  MapSubwindows();
  Layout();
}

void ParParDlg::One_opt(TGHorizontalFrame *hfr1, int width, void* x1,
			const char* tip1, TGNumberFormat::EStyle style1,
			double min1, double max1, byte cmd1) {

  /*
    char* connect;
    byte icmd=cmd1>>4;

    if (icmd==1) {
    connect=(char*) "DoNum_SetBuf()";
    }
    else {
    connect=(char*) "DoDaqNum()";    
    }
  */

  // if (iconnect==0) {
  //   connect = (char*) "DoParNum()";
  // }
  // else if (iconnect==1) {
  //   connect = (char*) "DoNum_SetBuf()";
  // }
  // else if (iconnect==2) {
  //   connect = (char*) "DoParNum_Daq()";
  //   icmd=1;
  // }

  int id;
  P_Def pdef1;

  if (style1==k_int) {
    pdef1=p_inum;
  }
  else {
    pdef1=p_fnum;
  }

  TGNumberFormat::ELimit limits;

  limits = TGNumberFormat::kNELNoLimits;
  if (max1!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  if (x1!=NULL) {
    if (style1==k_mon) { //check button
      id = Plist.size()+1;
      TGCheckButton *fchk = new TGCheckButton(hfr1, "STrig", id);
      DoMap(fchk,x1,p_chk,0,cmd1);
      fchk->SetToolTipText(tip1);
      hfr1->AddFrame(fchk,LayLC1);
      fchk->ChangeOptions(fchk->GetOptions()|kFixedWidth);
      fchk->SetWidth(52);
      fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
    }
    else { //normal number
      id = Plist.size()+1;
      TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, style1, 
					       TGNumberFormat::kNEAAnyNumber,
					       limits,min1,max1);
      DoMap(fNum1->GetNumberEntry(),x1,pdef1,0,cmd1);
      fNum1->GetNumberEntry()->SetToolTipText(tip1);
      fNum1->SetWidth(width);
      //fNum1->Resize(width, fNum1->GetDefaultHeight());
      fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParParDlg", this, "DoDaqNum()");
      hfr1->AddFrame(fNum1,LayLT4);
    }
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    //fskip->Resize(width, fskip->GetDefaultHeight());
    hfr1->AddFrame(fskip,LayLT4);
  }  
}

void ParParDlg::AddLine_opt(TGGroupFrame* frame, int width, void *x1, void *x2, 
			    const char* tip1, const char* tip2,
			    const char* label,
			    TGNumberFormat::EStyle style1, 
			    TGNumberFormat::EStyle style2,
			    double min1, double max1,
			    double min2, double max2, byte cmd1, byte cmd2)
{

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //hfr1->SetBackgroundColor(yellow);
  frame->AddFrame(hfr1);

  One_opt(hfr1,width,x1,tip1,style1,min1,max1,cmd1);
  One_opt(hfr1,width,x2,tip2,style2,min2,max2,cmd2);

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,LayLT4);

}

void ParParDlg::AddLine_hist(TGGroupFrame* frame, Hdef* hd,
			     const char* tip, const char* label) {

  double ww1=50;
  double ww=70;

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;
  //int id0;

  TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&hd->b,p_chk,0,3<<4);
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0.0001,10000);
  DoMap(fNum1->GetNumberEntry(),&hd->bins,p_fnum,0,2<<4);
  fNum1->GetNumberEntry()->SetToolTipText("Number of bins per channel");
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  //xlow
  id = Plist.size()+1;
  TGNumberEntry* fNum2 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum2->GetNumberEntry(),&hd->min,p_fnum,0,2<<4);
  fNum2->GetNumberEntry()->SetToolTipText("Low edge");
  fNum2->SetWidth(ww);
  fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum2,LayLT2);

  //xup
  id = Plist.size()+1;
  TGNumberEntry* fNum3 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   nolim);
  DoMap(fNum3->GetNumberEntry(),&hd->max,p_fnum,0,2<<4);
  fNum3->GetNumberEntry()->SetToolTipText("Upper edge");
  fNum3->SetWidth(ww);
  fNum3->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum3,LayLT2);


  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT2);

}

void ParParDlg::AddLine_2d(TGGroupFrame* frame, Hdef* hd,
			   const char* tip, const char* label, int type) {

  double ww1=50;
  int min2=0.0001;
  int max2=10000;
  char *tip11, *tip22;
  // byte cmd=0;

  if (type==1) { //normal 2d
    tip11= (char*) "Number of bins per channel on X-axis";
    tip22= (char*) "Number of bins per channel on Y-axis";
  }
  else { //AXAY
    ww1=60;
    min2=0;
    max2=opt.Nchan-1;
    // cmd=2<<4;
    tip11= (char*) "Number of bins per channel on X and Y axis";
    tip22= (char*) "Maximal channel number";
  }


  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;
  //int id0;

  //TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&hd->b,p_chk,0,2<<4);
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);
  //id0=id;

  //nbins (x-axis)
  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0.0001,10000);
  DoMap(fNum1->GetNumberEntry(),&hd->bins,p_fnum,0,2<<4);
  fNum1->GetNumberEntry()->SetToolTipText(tip11);
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  //nbins2 (y-axis)
  id = Plist.size()+1;
  fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0,
			    TGNumberFormat::kNEAAnyNumber,
			    lim,min2,max2);
  DoMap(fNum1->GetNumberEntry(),&hd->bins2,p_fnum,0,2<<4);
  fNum1->GetNumberEntry()->SetToolTipText(tip22);
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  // - just to take space
  // TGLabel* tt = new TGLabel(hfr1,"");
  // tt->ChangeOptions(tt->GetOptions()|kFixedWidth);
  // tt->SetWidth(ww);
  // hfr1->AddFrame(tt,LayLT2);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT2);
}

void ParParDlg::Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
			     P_Def pp, const char* tip) {
  double ww1=26;
  int id;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  id = Plist.size()+1;
  TGNumberEntryField* fNum1 = new TGNumberEntryField(hfr1, id, 0, k_int, 
						     TGNumberFormat::kNEAAnyNumber,
						     lim,1,max);
  DoMap(fNum1,nnn,pp,0);
  fNum1->SetToolTipText(tip);
  fNum1->SetWidth(ww1);
  fNum1->Connect("TextChanged(char*)", "ParDlg", this,
		 "DoNum()");
  hfr1->AddFrame(fNum1,LayLT3);
}

void ParParDlg::AddLine_prof(TGGroupFrame* frame, Hdef* hd,
			     const char* tip, const char* label) {

  //double ww1=40;
  //TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id;

  //main checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&hd->b,p_chk,0,4<<4);
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);
  //id0=id;

  Add_prof_num(hfr1,&opt.prof_nx,16,p_inum,"Number of X-strips from ING-27");
  Add_prof_num(hfr1,&opt.prof_ny,16,p_inum,"Number of Y-strips from ING-27");
  Add_prof_num(hfr1,&hd->bins,64,p_fnum,"Number of X-bins in profilometer histograms");
  Add_prof_num(hfr1,&hd->bins2,64,p_fnum,"Number of Y-bins in profilometer histograms");

	
  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT4);


  //1d checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_1d = new TGCheckButton(hfr1, "+1D", id);
  chk_1d->SetToolTipText("Create 1D histograms (only for the new profilometer)");
  DoMap(chk_1d,&opt.h_prof_x.b,p_chk,0);
  chk_1d->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_1d,LayCC1);
  //id0=id;



}

void ParParDlg::AddLine_mean(TGHorizontalFrame *hfr1, Hdef* hd,
			     const char* tip, const char* label)
{
  char name[20];

  int id;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  sprintf(name,"b_pulse%d",id);
  chk_hist->SetName(name);
  DoMap(chk_hist,&hd->b,p_chk,0);
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, label);
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT5);

}

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
  fcont->AddFrame(head_frame,LayLT0);

  // Hsplitter
  TGVerticalFrame *vFrame = new TGVerticalFrame(fcont, 10, 10);
  fcont->AddFrame(vFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));
  TGHorizontalFrame *fH1 = new TGHorizontalFrame(vFrame, 10, 120);
  TGHorizontalFrame *fH2 = new TGHorizontalFrame(vFrame, 10, W2_HEIGHT, kFixedHeight);
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
  //fcont1->AddFrame(head_frame,LayLT0);


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
  char txt[255];

  //set nfld after 1st line is filled
  if (!nfld && Plist.size()) {
    nfld=Plist.size();
  }

  if (i<MAX_CH)
    sprintf(txt,"%2d  ",i);
  else {
    sprintf(txt," ");
    all=i-MAX_CH+1;
  }

  // clab[i]=new TGTextEntry(cframe[i], txt);
  // clab[i]->SetHeight(20);
  // clab[i]->SetWidth(tlen1[kk]);
  // clab[i]->SetToolTipText(ttip1[kk]);
  // clab[i]->SetState(false);
  // cframe[i]->AddFrame(clab[i],LayCC0a);
  // kk++;

  //button for "all"
  if (i==MAX_CH) {
  //if (false) {
    id = Plist.size()+1;
    cbut = new TGTextButton(cframe[i], "ZZ", id);
    DoMap(cbut,&opt.chkall,p_but,1,0);
    //f_chk->SetToolTipText(ttip1[kk]);
    cbut->SetHeight(20);
    cbut->ChangeOptions(cbut->GetOptions()|kFixedWidth);
    cbut->SetWidth(tlen1[kk]+tlen1[kk+1]);
    cbut->SetToolTipText("* - change selected channels;\nall - change all channels;\nALL - change all channels and groups.");
    cbut->SetDown(false);
    cbut->Connect("Clicked()", "ParDlg", this, "DoAll()");
    cframe[i]->AddFrame(cbut,LayCC0);
    kk++;
    kk++;
  }
  else {
    AddChkPar(kk, cframe[i], &opt.star[i], all, ttip1[kk], 0);

    clab[i]=new TGTextEntry(cframe[i], txt);
    clab[i]->SetHeight(20);
    clab[i]->SetWidth(tlen1[kk]);
    clab[i]->SetToolTipText(ttip1[kk]);
    clab[i]->SetState(false);
    cframe[i]->AddFrame(clab[i],LayCC0a);
    kk++;
  }

  id = Plist.size()+1;

  if (i<=MAX_CH) {
    fCombo[i]=new TGComboBox(cframe[i],id);
    //fCombo->SetToolTipText(ttip1[kk]);
    cframe[i]->AddFrame(fCombo[i],LayCC0);
    fCombo[i]->Resize(tlen1[kk], 20);
    kk++;

    for (int j = 0; j < MAX_TP; j++) { //ZZ
      fCombo[i]->AddEntry(opt.ch_name[j], j+1);
    }
    fCombo[i]->AddEntry("Other", MAX_TP+1);

    if (i!=MAX_CH) {
      fCombo[i]->AddEntry("Copy", MAX_TP+2);
      fCombo[i]->AddEntry("Swap", MAX_TP+3);
    }
    // else {
    //   fCombo[i]->AddEntry("Copy", MAX_TP+2);
    //   fCombo[i]->AddEntry(" --- ", MAX_TP+3);
    // }

    //if (i<=MAX_CH) {
    //DoChanMap(fCombo[i],&opt.chtype[i],p_cmb,all,0);
    DoMap(fCombo[i],&opt.chtype[i],p_cmb,all,0);
    fCombo[i]->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");
    // fCombo[i]->Connect("ProcessedEvent(Event_t*)", "ParDlg", this, "DoCombo2(Event_t*)");
  }
  else {
    int i7=i-MAX_CH-1;
    TGTextEntry* tgtxt=new TGTextEntry(cframe[i]," ",id);
    tgtxt->SetWidth(tlen1[kk]);
    tgtxt->SetMaxLength(sizeof(opt.ch_name[0])-1);
    cframe[i]->AddFrame(tgtxt,LayCC0a);
    kk++;

    //cout << "txt: " << i << " " << i7 << " " << opt.ch_name[i7] << endl;
		
    tgtxt->SetName(TString::Format("%02dtxt",i7+1).Data());
    tgtxt->SetText(opt.ch_name[i7],false);
    tgtxt->Connect("TextChanged(char*)", "ParDlg", this, "DoTypes()");
    cframe[i]->SetBackgroundColor(tcol[i7]);
    clab[i]->SetBackgroundColor(tcol[i7]);

    DoMap(tgtxt,opt.ch_name[i7],p_txt,0);

  }

}

void ChanParDlg::AddChkPar(int &kk, TGHorizontalFrame *cframe,
			   Bool_t* dat, int all, const char* ttip, int cmd) {

  int id = Plist.size()+1;
  TGCheckButton *f_chk = new TGCheckButton(cframe, "", id);
  DoMap(f_chk,dat,p_chk,all,cmd);
  //f_chk->SetToolTipText(ttip1[kk]);
  f_chk->SetToolTipText(ttip);
  f_chk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  cframe->AddFrame(f_chk,LayCC1);
  kk++;
}

void ChanParDlg::AddNumChan(int i, int kk, int all, TGHorizontalFrame *hframe1,
			    void* apar, double min, double max, P_Def ptype, byte cmd) {

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

  //DoChanMap(fNum,apar,ptype, all,0);
  DoMap(fNum,apar,ptype, all,cmd);
  fNum->SetAlignment(kTextLeft);
  fNum->SetWidth(tlen7[kk]);
  fNum->SetHeight(20);
  fNum->Connect("TextChanged(char*)", "ParDlg", this, "DoDaqNum()");
  fNum->SetToolTipText(ttip7[kk]);
  hframe1->AddFrame(fNum,LayCC0);

}

//------ DaqParDlg -------

DaqParDlg::DaqParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
  fDock->SetWindowName("DAQ");  
}

void DaqParDlg::Build() {

  notbuilt=false;
  pmax=opt.Nchan;

  // cout << "DAQ: " << opt.Nchan << endl;

  AddHeader();

  for (int i=0;i<pmax;i++) {
    AddLine_daq(i,fcont1);
    //cout << "crs: addLine1: " << Plist.size() << endl; 
  }

  // AddLine_daq(opt.Nchan,fcont2);

  for (int i=0;i<MAX_TP+1;i++) { //ZZ
    AddLine_daq(MAX_CH+i,fcont2);
  }

  // AddLine_daq(opt.Nchan+MAX_TP,fcont2);

  if (crs->module==22) {
    TGHorizontalFrame *hforce = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(hforce,LayLT0);

    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce, "", id);
    hforce->AddFrame(fforce,LayCC1);
    //DoChanMap((TGWidget*)fforce,&cpar.forcewr,p_chk,0,0);
    DoMap((TGWidget*)fforce,&cpar.forcewr,p_chk,0,1);
    fforce->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
		
    TGLabel* lforce = new TGLabel(hforce, "  Force_write all channels");
    hforce->AddFrame(lforce,LayLT0);
  }

}

void DaqParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,LayLT0);

  TGTextEntry* tt[ndaqpar];

  for (int i=0;i<ndaqpar;i++) {
    // if (!strcmp(tlab1[i],"Trg") && crs->module<33) {
    //   continue;
    // }
    tt[i]=new TGTextEntry(head_frame, tlab1[i]);
    tt[i]->SetWidth(tlen1[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(ttip1[i]);
    tt[i]->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt[i],LayCC0);
  }

}

void DaqParDlg::AddLine_daq(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],LayLT0);

  // Pixel_t yellow,green;
  // gClient->GetColorByName("yellow", yellow);
  // gClient->GetColorByName("green", green);
  //cframe[i]->ChangeBackground(yellow);
  //cframe[i]->SetForegroundColor(green);

  AddChCombo(i,id,kk,all);

  AddChkPar(kk, cframe[i], &cpar.enabl[i], all, ttip1[kk], 1);
  AddChkPar(kk, cframe[i], &cpar.inv[i], all, ttip1[kk], 1);
  AddChkPar(kk, cframe[i], &cpar.acdc[i], all, ttip1[kk], 1);
  AddChkPar(kk, cframe[i], &cpar.pls[i], all, ttip1[kk], 1);
  AddChkPar(kk, cframe[i], &opt.dsp[i], all, ttip1[kk], 1);


  AddNumDaq(i,kk++,all,cframe[i],"smooth",&cpar.smooth[i]);
  AddNumDaq(i,kk++,all,cframe[i],"delay" ,&cpar.delay[i]);
  AddNumDaq(i,kk++,all,cframe[i],"dt"    ,&cpar.deadTime[i]);
  AddNumDaq(i,kk++,all,cframe[i],"pre"   ,&cpar.preWr[i]);
  AddNumDaq(i,kk++,all,cframe[i],"len"   ,&cpar.durWr[i]);
  if (crs->module==22) 
    AddNumDaq(i,kk++,1,cframe[i],"gain"  ,&cpar.adcGain[i]);
  else
    AddNumDaq(i,kk++,all,cframe[i],"gain"  ,&cpar.adcGain[i]);

  // if (crs->module>=33)
  //   AddNumDaq(i,kk++,all,cframe[i],"trig" ,&cpar.trg[i]);
  // else
  //   kk++;
  AddNumDaq(i,kk++,all,cframe[i],"trig" ,&cpar.trg[i]);

  AddNumDaq(i,kk++,all,cframe[i],"deriv" ,&cpar.kderiv[i],&opt.Drv[i]);
  AddNumDaq(i,kk++,all,cframe[i],"thresh",&cpar.threshold[i],&opt.Thr[i]);


  if (i<=MAX_CH) {
    AddStat_daq(fStat2[i],cframe[i],ttip1[kk]);kk++;
    AddStat_daq(fStat3[i],cframe[i],ttip1[kk]);kk++;
    AddStat_daq(fStatBad[i],cframe[i],ttip1[kk]);kk++;
    /*
    fStat2[i] = new TGTextEntry(cframe[i], "");
    fStat2[i]->ChangeOptions(fStat2[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat2[i]->SetState(false);
    fStat2[i]->SetToolTipText(ttip1[kk]);

    fStat2[i]->Resize(70,20);
    col=gROOT->GetColor(19)->GetPixel();
    fStat2[i]->SetBackgroundColor(col);
    fStat2[i]->SetText(0);
    cframe[i]->AddFrame(fStat2[i],LayLT5);
    //}
    kk++;

    //if (i<=opt.Nchan) {
    fStat3[i] = new TGTextEntry(cframe[i], "");
    fStat3[i]->ChangeOptions(fStat3[i]->GetOptions()|kFixedSize|kSunkenFrame);

    fStat3[i]->SetState(false);
    fStat3[i]->SetToolTipText(ttip1[kk]);

    fStat3[i]->Resize(70,20);
    col=gROOT->GetColor(19)->GetPixel();
    fStat3[i]->SetBackgroundColor(col);
    fStat3[i]->SetText(0);
    cframe[i]->AddFrame(fStat3[i],LayLT5);
    */
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
    cframe[i]->AddFrame(fStatBad[i],LayLT5);
    }
  */

}

void DaqParDlg::AddNumDaq(int i, int kk, int all, TGHorizontalFrame *hframe1,
			  const char* name, void* apar, void* apar2) {  //const char* name) {

  int par, min, max;

  cpar.GetPar(name,crs->module,i,crs->type_ch[i],par,min,max);
  //cout << "getpar1: " << i << " " << min << " " << max << endl;

  TGNumberFormat::ELimit limits;

  limits = TGNumberFormat::kNELLimitMinMax;
  if (max==-1) {
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
  //DoChanMap(fNum,apar,p_inum, all,kk-1,apar2);
  DoMap(fNum,apar,p_inum, all,1,apar2);
	
  //fNum->SetName(name);
  fNum->SetToolTipText(ttip1[kk]);
  fNum->SetWidth(tlen1[kk]);
  fNum->SetHeight(20);

  fNum->Connect("TextChanged(char*)", "ParDlg", this, "DoDaqNum()");

  hframe1->AddFrame(fNum,LayCC0);

}

void DaqParDlg::AddStat_daq(TGTextEntry* &fStat, TGHorizontalFrame* &cframe,
			    const char* ttip) {
  int col;

  fStat = new TGTextEntry(cframe, "");
  fStat->ChangeOptions(fStat->GetOptions()|kFixedSize|kSunkenFrame);

  fStat->SetState(false);
  fStat->SetToolTipText(ttip);

  fStat->Resize(70,20);
  col=gROOT->GetColor(19)->GetPixel();
  fStat->SetBackgroundColor(col);
  fStat->SetText(0);
  cframe->AddFrame(fStat,LayLT5);

}

void DaqParDlg::UpdateStatus(int rst) {

  //cout << "Updatestatus1: " << pmax << endl;
  static Long64_t allbad;
  static double t1;
  static Long64_t npulses2o[MAX_CH];
  static Long64_t npulses3o[MAX_CH];
  static double rate2[MAX_CH];
  static double rate_all2;
  static double rate3[MAX_CH];
  static double rate_all3;

  if (rst) {
    allbad=0;
    t1=0;
    memset(npulses2o,0,sizeof(npulses2o));
    memset(npulses3o,0,sizeof(npulses3o));
    memset(rate2,0,sizeof(rate2));
    memset(rate3,0,sizeof(rate3));
  }

  TGString txt;

  double dt = opt.T_acq - t1;

  //cout << "DT: " << dt << endl;
  allbad=0;
  rate_all2=0;
  rate_all3=0;
  if (dt>0.1) {
    for (int i=0;i<pmax;i++) {
      rate2[i] = (crs->npulses2[i]-npulses2o[i])/dt;
      npulses2o[i]=crs->npulses2[i];
      rate3[i] = (crs->npulses3[i]-npulses3o[i])/dt;
      npulses3o[i]=crs->npulses3[i];

      rate_all2+=rate2[i];
      rate_all3+=rate3[i];
      allbad+=crs->npulses_bad[i];
    }
    t1=opt.T_acq;
  }

  //cout << "Updatestatus2: " << pmax << endl;
  for (int i=0;i<pmax;i++) {
    txt.Form("%0.0f",rate2[i]);
    fStat2[i]->SetText(txt);
    txt.Form("%0.0f",rate3[i]);
    fStat3[i]->SetText(txt);
    txt.Form("%d",crs->npulses_bad[i]);
    fStatBad[i]->SetText(txt);
  }
  //cout << "Updatestatus2a: " << pmax << endl;
  txt.Form("%0.0f",rate_all2);
  fStat2[MAX_CH]->SetText(txt);
  txt.Form("%0.0f",rate_all3);
  fStat3[MAX_CH]->SetText(txt);
  txt.Form("%lld",allbad);
  fStatBad[MAX_CH]->SetText(txt);

  //cout << "Updatestatus3: " << pmax << endl;
}

// void DaqParDlg::Update() {
//   ParDlg::Update();
//   MapSubwindows();
//   Layout();
// }

//------ AnaParDlg -------

AnaParDlg::AnaParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
}

void AnaParDlg::Build() {

  notbuilt=false;
  pmax=opt.Nchan;

  AddHeader();

  //cout << "ana1: " << endl;
  for (int i=0;i<pmax;i++) {
    AddLine_Ana(i,fcont1);
  }

  // AddLine_Ana(opt.Nchan,fcont2);

  for (int i=0;i<MAX_TP+1;i++) { //ZZ
    AddLine_Ana(MAX_CH+i,fcont2);
  }
  // AddLine_Ana(opt.Nchan+MAX_TP,fcont2);

  //cout << "ana2: " << endl;
}

void AnaParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,LayLT0);

  TGTextEntry* tt;

  for (int i=0;i<n_apar;i++) {
    tt=new TGTextEntry(head_frame, tlab2[i]);
    tt->SetWidth(tlen2[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip2[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,LayCC0);
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
    head_frame->AddFrame(tt,LayCC0);
  }

}

void AnaParDlg::AddLine_Ana(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],LayLT0);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

  AddChCombo(i,id,kk,all);

  AddChkPar(kk, cframe[i], &opt.St[i], all, ttip2[kk]);

  // id = Plist.size()+1;
  // TGCheckButton *fst = new TGCheckButton(cframe[i], "", id);
  // //DoChanMap(fst,&opt.St[i],p_chk,all,0);
  // DoMap(fst,&opt.St[i],p_chk,all,0);
  // fst->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");
  // fst->SetToolTipText(ttip2[kk]);
  // cframe[i]->AddFrame(fst,LayCC1);
  // kk++;

  tlen7 = (int*) tlen2;
  ttip7 = (char**) ttip2;

  AddNumChan(i,kk++,all,cframe[i],&opt.sS[i],0,99,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Delay[i],-999,999,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.dT[i],0,9999,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Pile[i],0,9999,p_inum);

  AddNumChan(i,kk++,all,cframe[i],&opt.E0[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.E1[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.E2[i],-1e99,1e99,p_fnum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Bc[i],-1e99,1e99,p_fnum);

  for (int j=0;j<NGRP;j++) {
    id = Plist.size()+1;
    TGCheckButton *gg = new TGCheckButton(cframe[i], "", id);
    //DoChanMap(gg,&opt.Grp[i][j],p_chk,all,0);
    DoMap(gg,&opt.Grp[i][j],p_chk,all,0);
    gg->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
    gg->SetToolTipText(ttip_g[j]);
    cframe[i]->AddFrame(gg,LayCC1);
    kk++;
  }

}

//------ PikParDlg -------

PikParDlg::PikParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ChanParDlg(p,w,h)
{
  fDock->SetWindowName("Peaks");  
}

void PikParDlg::Build() {

  notbuilt=false;
  pmax=opt.Nchan;

  AddHeader();

  //prtime("DS1--");
  for (int i=0;i<pmax;i++) {
    AddLine_Pik(i,fcont1);
  }

  //prtime("DS2--");
  // AddLine_Pik(opt.Nchan,fcont2);

  //prtime("DS3--");
  for (int i=0;i<MAX_TP+1;i++) { //ZZ
    AddLine_Pik(MAX_CH+i,fcont2);
  }
  // AddLine_Pik(opt.Nchan+MAX_TP,fcont2);
  //prtime("DS4--");


}

void PikParDlg::AddHeader() {

  //TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  //AddFrame(hframe1,LayLT0);

  TGTextEntry* tt;

  for (int i=0;i<n_ppar;i++) {
    tt=new TGTextEntry(head_frame, tlab3[i]);
    tt->SetWidth(tlen3[i]);
    tt->SetState(false);
    tt->SetToolTipText(ttip3[i]);
    tt->SetAlignment(kTextCenterX);
    head_frame->AddFrame(tt,LayCC0);
  }

}

void PikParDlg::AddLine_Pik(int i, TGCompositeFrame* fcont1) {
  //char txt[255];
  int kk=0;
  int all=0;
  int id;

  //static bool start=true;

  cframe[i] = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(cframe[i],LayLT0);

  //Pixel_t yellow;
  //gClient->GetColorByName("yellow", yellow);
  //cframe[i]->ChangeBackground(yellow);

  AddChCombo(i,id,kk,all);

  AddChkPar(kk, cframe[i], &opt.dsp[i], all, ttip3[kk], 1);

  // id = Plist.size()+1;
  // TGCheckButton *fpls = new TGCheckButton(cframe[i], "", id);
  // DoChanMap(fpls,&cpar.pls[i],p_chk,all,0,0);
  // fpls->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");
  // fpls->SetToolTipText(ttip2[kk]);
  // cframe[i]->AddFrame(fpls,LayCC1);
  // kk++;

  tlen7 = (int*) tlen3;
  ttip7 = (char**) ttip3;

  int amax=1023;
  if (crs->type_ch[i]>=1)
    amax=511;

  //cout << "module: " << crs->module << " " << i << " " << crs->type_ch[i] << " " << amax << endl;

  AddNumChan(i,kk++,all,cframe[i],&opt.sTg[i],-1,5,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Drv[i],1,1023,p_inum);
  AddNumChan(i,kk++,all,cframe[i],&opt.Thr[i],0,65565,p_inum);

  AddNumChan(i,kk++,all,cframe[i],&opt.Base1[i],-1024,amax,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.Base2[i],-1024,9999,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.Peak1[i],-1024,amax,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.Peak2[i],-1024,9999,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.T1[i],-1024,amax,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.T2[i],-1024,9999,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.W1[i],-1024,amax,p_inum,1);
  AddNumChan(i,kk++,all,cframe[i],&opt.W2[i],-1024,9999,p_inum,1);
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
    //fDock->SetBackgroundColor(fRed);
    fDock->SetFixedSize(kFALSE);
  */
	
  TGCompositeFrame *fMain=fDock->GetContainer();
  //fMain->SetBackgroundColor(fRed10);
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));
	
  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,LayEE0);
  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);

  for (int i=0;i<MAX_ERR;i++) {
    TGHorizontalFrame* cframe = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(cframe,LayLT0);
    TGLabel* lb = new TGLabel(cframe,crs->errlabel[i].c_str());
    lb->SetTextJustify(kTextLeft);
    lb->ChangeOptions(lb->GetOptions()|kFixedSize);
    lb->Resize(120,20);
    cframe->AddFrame(lb,LayLT5);
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
    cframe->AddFrame(fErr[i],LayLT5);
  }

}

ErrFrame::~ErrFrame() {
}

void ErrFrame::Reset() {
  errflag=0;
  TGTabElement* tab6 = myM->fTab->GetTabTab("Errors");
  tab6->SetBackgroundColor(15263976);
  tab6->Resize();
  tab6->Layout();
  ErrUpdate();
}

void ErrFrame::ErrUpdate() {
  TGString txt;
  //cout << "ErrUpdate: " << endl;

  for (int i=0;i<MAX_ERR;i++) {
    txt.Form("%lld",crs->errors[i]);
    fErr[i]->SetText(txt);
    if (crs->errors[i] && !errflag) {
      errflag=1;
      TGTabElement* tab6 = myM->fTab->GetTabTab("Errors");
      tab6->SetBackgroundColor(fRed);
      tab6->Resize();
      tab6->Layout();
      // if (errflag==0)
      // 	errflag=1;
      // else if (errfla==1)
      // 	errflag=2;
    }
  }
}
