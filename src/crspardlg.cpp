#include "romana.h"

#include "TGFileDialog.h"
#include <TSystem.h>
#include <TColor.h>
#include <TGToolTip.h>
#include <TVirtualX.h>

extern std::list<VarClass> varlist;


extern Pixel_t fWhite;
extern Pixel_t fGrey;
extern Pixel_t fYellow;
extern Pixel_t fGreen;
extern Pixel_t fRed;
extern Pixel_t fCyan;
extern Pixel_t fMagenta;
extern Pixel_t fOrng;
extern Pixel_t fBlue;
extern Pixel_t fRed10;

extern Pixel_t fCol[7];// = [1..7] {fYellow,fGreen,fRed,fCyan,fOrng,fBlue,fRed10};

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

extern HClass* hcl;
extern ParParDlg *parpar;
extern ChanParDlg *chanpar;

const char* ttip_type = "Channel type:\nOther - dummy type\nCopy - copy from channel to group\nSwap - first select swap, then change parameter(s), then change to new type";

vector<const char*> ptip = {
  "Channel number",
  "On/Off",
  "Select",
  ttip_type,
  "Inversion",
  "AC coupling\nFor CRS-128 grouped by 4 channels",
  "Send/don't send pulse data (waveforms)",
  "Send/don't send DSP data",
  "Only for coincidence scheme:\nRate divider (0 - don't write reduced data)",
  "Only for coincidence scheme:\nChannel belongs to coincidence group 1",
  "Only for coincidence scheme:\nChannel belongs to coincidence group 2",
  "Hardware smoothing. It is recommended to set it to the power of 2 to avoid reduction of amplitude.",
  "Hardware delay (in samples)",
  "Dead time - no new trigger on the current channel within dead time from the old trigger",
  "Number of samples before the trigger",
  "Total length of the pulse in samples",
  "Additional Gain\nFor CRS-8/16 and CRS-128 grouped by 4 channels",
  "Trigger type:\n"
  "0 - threshold crossing of pulse;\n"
  "1 - threshold crossing of derivative;\n"
  "2 - maximum of derivative;\n"
  "3 - rise of derivative;\n"
  "4 - fall of derivative;\n"
  //"5 - fall of 2nd derivative, use 2nd deriv for timing;\n"
  "5 - not used;\n"
  "6 - fall of derivative, zero crossing\n"
  "Not all types are available for all devices",
  "Parameter of derivative: S(i) - S(i-Drv)",
  "Trigger threshold",
  "Trigger lower threshold (for Trg 3-6)",
  "Start channel - used as start in Time spectra\nif there are many start channels in the event, the earliest is used",
  "Master/slave channel:\nEvents containing only slave channels are rejected\nEach event must contain at least one master channel",
  "Checked - use hardware pulse analysis (DSP)\nUnchecked - use software pulse analysis",
  "Checked - write pulses in Dec",
  "Software delay in ns (can be negative or positive)",
  // "Dead-time window \nsubsequent peaks within this window are ignored",
  // "Pileup window \nmultiple peaks within this window are marked as pileup",
  "Energy calibration type: 0 - no calibration; 1 - linear; 2 - parabola; 3 - spline",
  "Energy calibration 0: E0+E1*x",
  "Energy calibration 1: E0+E1*x",
  "Energy calibration 2: E0+E1*x+E2*x*x",
  "Pole-Zero correction",
  //"Baseline correction",
  "Use channel for group histograms *_g1",
  "Use channel for group histograms *_g2",
  "Use channel for group histograms *_g3",
  "Use channel for group histograms *_g4",
  "Software smoothing. If negative - imitates hS (data are truncated to integer etc)",
  "Software trigget type:\n"
  "0 - hreshold crossing of pulse;\n"
  "1 - threshold crossing of derivative;\n"
  "2 - maximum of derivative;\n"
  "3 - rise of derivative, LT (lower threshold) crossing;\n"
  "4 - fall of derivative;\n"
  //"5 - fall of 2nd derivative, use 2nd deriv for timing;\n"
  "5 - not used;\n"
  "6 - fall of derivative, LT (lower threshold) crossing;\n"
  "7 - CFD, zero crosing;\n"
  "-1 - use hardware trigger",
  "Software parameter of derivative: S(i) - S(i-Drv)",
  "Software trigger threshold",
  "Analysis method:\n0 - standard;\n1 - area from 1st derivative between T1 and T2; no base subtraction\n2 - base slope subtraction (for HPGe)\n3 - base slope2 instead of slope1 (using W1 & W2) + slope2 subtraction (for HPGe)\n  for Mt=3 RMS2 is not calculated; Width=Pos-Time in pulse mode",
  "Baseline start, relative to peak Pos (usually negative, included)",
  "Baseline end, relative to peak Pos (usually negative, included)",
  "Peak start, relative to peak Pos (usually negative, included)",
  "Peak end, relative to peak Pos (usually positive, included)",
  "CFD delay in samples",
  "CFD fraction x10",
  "Timing window start (usually negative, included)",
  //"CFD delay [delay=abs(T1)]",
  "Timing window end (usually positive, included)",
  //"CFD fraction x10",
  "Width window start (included)",
  "Width window end (included)",
  "Pulse rate (software)",
  "Pulse rate (hardware)",
  "Number of bad pulses"
};

const int PHeight = 20;
const int NFLD = ptip.size();

//#define mask0 "TN AtWHBSsRrpD C"
#define mask0 mask_e " " mask_p " DC"
const char* tip_dec=
  "Mask for decoder format (type 81 or higher)\n"
  mask0" (spaces are ignored)\n"
  "\n"
  "T - TimeStamp + State/Polarization\n"
  //"P - Polarization(state)\n"
  "N - Number of pulses in event + "
  "Event length (in 8-byte words) + "
  "Event number\n"
  "\n"
  "A - Peak area\n"
  "t - Peak time relative to Timestamp\n"
  "W - Peak width\n"
  "H - Peak height\n"
  "B - Baseline\n"
  "S - Slope (baseline)\n"
  "s - Slope (peak)\n"
  "R - RMS (baseline)\n"
  "r - RMS (peak)\n"
  "p - Pileup\n"
  "\n"
  "D - Pulse data (oscillogram)\n"
  "C - Hardware counter";


//char ttip_g[NGRP][100];

//int* tlen7;
//char** ttip7;


//extern Common* com;
extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  //:TGCompositeFrame(p,w,h,kVerticalFrame)
  :TGCompositeFrame(p,w,h)
{

  k_int=TGNumberFormat::kNESInteger;
  k_r0=TGNumberFormat::kNESReal;
  k_r1=TGNumberFormat::kNESRealOne;
  k_r2=TGNumberFormat::kNESRealTwo;
  k_chk=TGNumberFormat::kNESMDayYear;
  k_lab=TGNumberFormat::kNESDayMYear;
  k_hex=TGNumberFormat::kNESHex;
  
  LayCC0   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,0,0);
  LayCC0a  = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,1,1);
  LayCC1   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 4, 4, 0, 0);
  //LayCC1a   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 4, 4, 4, 4);
  LayCC2   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 10, 0, 0, 0);
  LayET0   = new TGLayoutHints(kLHintsExpandX|kLHintsTop, 0, 0, 0, 0);
  LayET1   = new TGLayoutHints(kLHintsExpandX|kLHintsTop, 5, 5, 5, 5);
  LayLC1   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 1, 6, 1, 1);
  LayLC2   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 1, 1, 1, 1);
  LayLT0   = new TGLayoutHints(kLHintsLeft|kLHintsTop);
  LayLT1   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 0, 5, 0);
  LayLT1a  = new TGLayoutHints(kLHintsLeft|kLHintsTop, 1, 1, 5, 0);
  LayLT1b  = new TGLayoutHints(kLHintsLeft|kLHintsTop, 0, 0, 5, 5);
  LayLT2   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 0);
  LayLT3   = new TGLayoutHints(kLHintsLeft|kLHintsTop,1,1,1,1);
  LayLT4   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 6, 1, 1, 1);
  LayLT4a  = new TGLayoutHints(kLHintsLeft|kLHintsTop, 6, 11, 1, 1);
  LayLT5   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 1);
  LayLT6   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 50, 1, 20, -10);
  LayLT7   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 0, 5, 0);

  LayLE0   = new TGLayoutHints(kLHintsLeft|kLHintsExpandY);
  LayEE0   = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY);
  LayEE1   = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,0,0,1,1);

  LayRT0   = new TGLayoutHints(kLHintsRight|kLHintsTop, 0, 0, 0, 0);

  LayTrig   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 2, 0, 10, 10);

  //LayL2  = new TGLayoutHints(kLHintsLeft,2,2,2,2);
  //SetCleanup(kDeepCleanup);
  //jtrig=0;
  //notbuilt=true;
  //pmax=0;

  fDock = new TGDockableFrame(this);

  fDock->SetFixedSize(kFALSE);

  nfld=0;

  Int_t cc;
  for (int i=0;i<MAX_TP+3;i++) {
    cc=TColor::GetColor(TColor::RGB2Pixel(CP::RGB[i][0],CP::RGB[i][1],CP::RGB[i][2]));
    tcol[i]=gROOT->GetColor(cc)->GetPixel();
  }
  memset(hparl,0,sizeof(hparl));
}
ParDlg::~ParDlg() {
  //cout << "~ParDlg: " << this << endl;
  //CleanUp();
}

void ParDlg::DoMap(TGFrame* f, void* d, P_Def t, int all, UInt_t cmd, void* d2, UShort_t off, UChar_t step) {
  Pmap pp;
  pp.field = (TGFrame*) f;
  pp.data = d;
  pp.data2 = d2;
  pp.off = off;
  pp.step = step;
  pp.type = t;
  pp.all= (UChar_t) all;
  pp.cmd=cmd;
  Plist.push_back(pp);
  Clist.push_back(0);
}

bool ParDlg::Chk_all(int all, int i) {
  // проверяет состояние opt.chkall

  if (all==1) { //all
    if (opt.chkall==0) { // * (star)
      return (i<opt.Nchan) && opt.star[i];
    }
    else if (opt.chkall==1) { //all
      return i<opt.Nchan;
    }
    else { //ALL
      return true;
    }
  }
  else if (all>1) { //chtype
    return i<opt.Nchan && opt.chtype[i]==all-1;
  }
  else
    return false;
}

void ParDlg::DoAct(int id, UShort_t off, Double_t fnum, bool dq) {
  Pmap* pp = &Plist[id-1];
  int act = (pp->cmd>>4)&0xF;

  //prnt("ss d d f d ds;",BBLU,"act:",id,off,fnum,dq,act,RST);

  switch (act) {
  case 1:
    if (crs->b_stop)
      crs->DoReset();
    break;

  case 2:
    if (crs->b_stop)
      HiFrm->HiReset();
    break;

  case 3:
    if (pp->data==&opt.b_fpeaks && opt.b_fpeaks) {
      HiFrm->pkprint=true;
    }

    if (crs->b_stop)
      HiFrm->HiUpdate();
    else
      HiFrm->changed=true;

    break;
    /*
  case 4:
    if (nfld && (crs->module==22)) {
      int ll = (id-1)/nfld; //line number
      if (ll<pmax) {
	if (&cpar.Drv[ll]==pp->data) { //Drv
	  if (cpar.Drv[ll])
	    cpar.Trg[ll]=1;
	  else
	    cpar.Trg[ll]=0;
	  UpdateField(id-2);
	}
	else { //Trg
	  if (cpar.Trg[ll])
	    cpar.Drv[ll]=1;
	  else
	    cpar.Drv[ll]=0;
	  UpdateField(id);
	}
      }
    }
    break;
    */
  case 5: {//group4
    bool ff=false;
    // crs->module=41;
    if (pp->data==&cpar.AC && crs->module==45) {
      //cout << "act5: " << pp->data << " " << &cpar.AC << endl;
      ff=true;
    }
    if (ff || crs->module==44 || crs->module==54) { //CRS-8 or CRS-128
      int l4 = off/4*4; //group4

      for (int i=l4;i<l4+4;i++) { //MAX_CH+MAX_TP+1
	if (i<MAX_CH) {
	  if (pp->type == p_chk)
	    SetChk(*pp,i,fnum);
	  else
	    SetNum(*pp,i,fnum);
	}
      }
      UpdateColumn(id);
      
    }
    break;
  }
  case 6: { //проверка Len кратно 3 или 4
    //cout << "act6: " << endl;

    //int kk = (id-1)%nfld; //column number
    //здесь должно быть off вместо id?
    //int ll = (id-1)/nfld; //line number

    int l2 = cpar.ChkLen(off,crs->module);
    if (l2-cpar.Len[off]==1)
      cpar.Len[off]-=4;
    //prnt("ss d d ds;",BBLU,"Act:",off,cpar.Len[off],l2,RST);

    UpdateField(id-1);

    break;
  }
  case 7:
    myM->fTimer->SetTime(opt.tsleep);
    break;
  case 8:
    chanpar->Update();
    break;
  } //switch

  DoColor(pp,(Float_t) fnum);

#ifdef CYUSB
  int cmd = pp->cmd & 1;
  if (dq && cmd && crs->b_acq) {// && !jtrig) {
    crs->Command2(4,0,0,0);
    /*
    prnt("sss;",BYEL,"Sleep 1300",RST);
    gSystem->Sleep(1300); //1300 - проблема 3 в АК-32 устраняется
    crs->Cancel_all(7);
    prnt("sss;",BYEL,"Sleep 1300",RST);
    gSystem->Sleep(1300); //1300 - проблема 3 в АК-32 устраняется

    crs->Init_Transfer();
    prnt("sss;",BYEL,"Sleep 3300",RST);
    gSystem->Sleep(3300); //1300 - проблема 3 в АК-32 устраняется

    crs->Submit_all(crs->ntrans);
    prnt("sss;",BYEL,"Sleep 3300",RST);
    gSystem->Sleep(3300); //1300 - проблема 3 в АК-32 устраняется

    //crs->Command32(8,0,0,0); //сброс сч./буф.
    //crs->Command32(9,0,0,0); //сброс времени
    */
    crs->SetPar();
    gzFile ff = gzopen("last.par","wb");
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
    if (crs->module==22) {//CRS2
      myM->UpdateTimer(1);
      //myM->UpdateStatus(1);
      chanpar->UpdateStatus(1);
    }
    /*
    gSystem->Sleep(1300); //1300 - проблема 3 в АК-32 устраняется
    //crs->Submit_all(crs->ntrans);
    gSystem->Sleep(1300); //1300 - проблема 3 в АК-32 устраняется
    */
    crs->Command2(3,0,0,0);
  }
#endif
}

void ParDlg::SetNum(Pmap pp, UShort_t off, Double_t num) {
  switch (pp.type) {
  case p_fnum:
  case p_fnum2: {
    *((Float_t*)pp.data+off) = num;
    if (pp.data2) *((Float_t*)pp.data2+off) = num;
    break;
  }
  case p_inum: {
    *((Int_t*)pp.data+off) = num;
    if (pp.data2) *((Int_t*)pp.data2+off) = num;
    break;
  }
  default:
    cout << "(SetNum) Wrong type: " << pp.type << endl;
  }

  // cout << "SetNum: " << num << " "
  //      << *(Int_t*)pp.data << " "
  //      << *((Int_t*)pp.data+off) << " "
  //      << off << endl;

}

void ParDlg::DoNum() {

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  Pmap pp = Plist[id-1];

  SetNum(pp,pp.off,te->GetNumber());
  //int pos = te->GetCursorPosition();

  if (pp.all>0) {
    for (int i=0;i<MAX_CHTP;i++) { //MAX_CH+MAX_TP+1
      if (Chk_all(pp.all,i)) {
	SetNum(pp,i,te->GetNumber());
	int act = (pp.cmd>>4)&0xF;
	if (act)
	  DoAct(id,i,te->GetNumber(),0);
      }
    }

    UpdateColumn(id);

  }
  //te->SetCursorPosition(pos);
}

void ParDlg::DoDaqNum() {
  //jtrig++;
  ParDlg::DoNum();
  //jtrig--;

  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();

  DoAct(id,Plist[id-1].off,te->GetNumber(),1);

}

void ParDlg::SetChk(Pmap pp, UShort_t off, Bool_t num) {
  //cout << "setchk: " << pp.off << " " << off << " " << (int) pp.step << endl;
  if (pp.type==p_chk) {
    *((Bool_t*)pp.data+off) = num;
    if (pp.data2) *((Bool_t*)pp.data2+off) = num;
  }
  else {
    cout << "(DoChk) Wrong type: " << (int) pp.type << endl;
  }
}

void ParDlg::DoChk(Bool_t on) {

  Int_t id = ((TGCheckButton*) gTQSender)->WidgetId();
  Pmap pp = Plist[id-1];

  // if (pp.data==&cpar.on) {
  //   crs->chan_changed=true;
  // }

  SetChk(pp,pp.off,on);
  UpdateField(id-1);

  if (pp.all>0) {
    for (int i=0;i<MAX_CHTP;i++) { //MAX_CH+MAX_TP+1
      if (Chk_all(pp.all,i)) {
	SetChk(pp,i*pp.step,on);
	int act = (pp.cmd>>4)&0xF;
	if (act)
	  DoAct(id,i,on,0);
      }
    }
    UpdateColumn(id);
  }

}

void ParDlg::DoDaqChk(Bool_t on) {
  //jtrig++;
  ParDlg::DoChk(on);
  //jtrig--;

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();

  DoAct(id,Plist[id-1].off,on,1);
}

void ParDlg::DoCheckHist(Bool_t on) {
  if (!crs->b_stop) return;

  //jtrig++;
  DoChk(on);
  //jtrig--;

  //if (!jtrig) {
  HiFrm->HiReset();
  //}
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

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {
    Pmap pp = Plist[id];

    SetTxt(pp,fi.fFilename);

    TGTextEntry *te2 = (TGTextEntry*) pp.field;
    te2->SetText(fi.fFilename);      

  }

}

void ParDlg::SetCombo(Pmap pp, UShort_t off, Int_t num) {
  if (pp.type==p_cmb) {
    *((Int_t*)pp.data+off) = num;
  }
  else {
    cout << "(DoCombo) Wrong type: " << (int)pp.type << endl;
  }
  // cout << "combo: " << num << " " << *((Int_t*)pp.data+off)
  //      << " " << off << " " << opt.chtype[1]
  //      << endl;
}

void ParDlg::DoCombo() {

  TGComboBox *te = (TGComboBox*) gTQSender;
  Int_t id = te->WidgetId();

  int sel = te->GetSelected(); //sel starts from 1

  Pmap* pp = &Plist[id-1];

  int nline = id/nfld; // номер строки виджета
  int index = pp->off; // index number of *data

  // bool cp=false; //copy from chtype to channel
  int old_type=opt.chtype[index];

  //prnt("ss d d d d d ds;",BRED,"comb:",id,sel,index,nline,pp->off,pp->all,RST);

  if (pp->all==0) { //normal channels
    if (sel!=MAX_TP+2) { // sel!=Copy -> normal sel
      SetCombo(*pp,index,sel);
      if (old_type!=MAX_TP+3) { //if old type is not swap, then copy one line
	CopyParLine(sel,index,nline);
      }
      else { // swap -> just change background
	ColorLine(nline,tcol[sel-1]);
      }
    }

    else { //if (sel==MAX_TP+2) { // sel==Copy
      if (old_type<=MAX_TP) { // normal chtype, exclude "other"
	// inverse copy (channel to chtype)
	CopyParLine(-old_type,index,nline);
      }
      te->Select(old_type,false);
    }
  }
  // "All" group
  else { //pp.all==1
    SetCombo(*pp,index,sel);
    if (sel<=MAX_TP+1) { //normal chtype & other: select and copy line
      for (int i=0;i<MAX_CHTP;i++) { //MAX_CH+MAX_TP+1
	if (Chk_all(pp->all,i)) {
	  SetCombo(*pp,i,sel);
	  CopyParLine(sel,i,nline);
	}
      } //for
    } // if sel<=MAX_TP
  } //if pp.all

  Update();
}


void ParDlg::SetTxt(Pmap pp, const char* txt) {
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

  Pmap pp = Plist[id-1];

  SetTxt(pp,te->GetText());
  //UpdateField(id-1);
  DoAct(id,Plist[id-1].off,0,0);
}

void ParDlg::DoOneType(int i) {

  int i0=i-1;
  if (i0==0) i0=-1;

  for (int j=0;j<=MAX_CH;j++) {
    if (j<chanpar->nrows || j==MAX_CH) {
      int sel = fCombo[j]->GetListBox()->GetSelected();
      fCombo[j]->RemoveEntry(i);
      fCombo[j]->InsertEntry(opt.ch_name[i-1],i,i0);
      if (sel==i)
	fCombo[j]->Select(sel,false);
    }
  }
}

void ParDlg::DoTypes() {

  TGTextEntry *te = (TGTextEntry*) gTQSender;

  DoTxt();

  int i = TString(te->GetName())(0,1).String().Atoi();

  DoOneType(i);
}

void ParDlg::DoAll() {

  opt.chkall = (opt.chkall+1)%3;
  UpdateField(cbut_id-1);

}

void ParDlg::ColorLine(int line, ULong_t col) {
  // здесь line - номер строки виджета
  if (line==MAX_CH)
    cbut->ChangeBackground(col);
  else
    clab[line]->ChangeBackground(col);

  for (int i=0;i<3;i++)
    if (hparl[i][line])
      hparl[i][line]->ChangeBackground(col);
}

void ParDlg::CopyParLine(int sel, int index, int line) {
  //sel  -  индекс в комбо = chtype
  //index - индекс переменной
  //line - номер строки виджета

  if (sel<0) { //inverse copy - from current ch to group
    //cout << "copyp: " << sel << " " << index << " " << line << endl;
    for (int j=0;j<nfld;j++) {
      Pmap* pp = &Plist[j];
      int a = index*pp->step; //from
      int b = (MAX_CH-sel)*pp->step; //to
      CopyField(pp,a,b);
    }
    //ColorLine(line,tcol[-sel-1]);
  }
  else if (sel<=MAX_TP) { //normal copy from group to current ch
    for (int j=0;j<nfld;j++) {
      //prnt("ss d d d ds;",BYEL,"CPL:",j,sel,index,line,RST);
      Pmap* pp = &Plist[j];
      int b = index*pp->step; //to
      int a = (MAX_CH+sel)*pp->step; //from
      CopyField(pp,a,b);
    }
    if (line<chanpar->nrows) {
      ColorLine(line,tcol[sel-1]);
    }
  }
  else { //if (sel>MAX_TP) { //other,swap - just change color
    if (line<chanpar->nrows) {
      ColorLine(line,tcol[MAX_TP]);
    }
  }
}

void ParDlg::CopyField(Pmap* pp, int from, int to) {

  //skip wrong types
  switch (pp->type) {
  case p_inum:
  case p_fnum:
  case p_fnum2:
  case p_chk:
    break;
  default:
    return;
  }

  // if (pp->type!=p_inum && pp->type!=p_fnum && pp->type!=p_chk) {
  //   return;
  // }
	
  switch (pp->type) {
  case p_inum:
    *((Int_t*)pp->data+to) = *((Int_t*)pp->data+from);
    break;
  case p_fnum:
  case p_fnum2:
    *((Float_t*)pp->data+to) = *((Float_t*)pp->data+from);
    break;
  case p_chk:
    *((Bool_t*)pp->data+to) = *((Bool_t*)pp->data+from);
    break;
  // case p_stat:
  //   break;
  default:
    cout << "CopyField: unknown pp->type: " << pp->type << endl;
  } //switch  
} //CopyField

void ParDlg::DoColor(Pmap* pp, Float_t val) {
  //change color, if needed
  int col=(pp->cmd & 0xF)>>1;
  if (col) {
    switch (pp->type) {
    case p_txt: {
      TGTextEntry *te = (TGTextEntry*) pp->field;

      if (pp->data==&opt.dec_mask) { //проверяем маску для dec
	if (crs->MakeDecMask())
	  te->ChangeBackground(fWhite);
	else
	  te->ChangeBackground(fCol[col-1]);
      }
      break;
    }
    case p_inum:
    case p_fnum:
    case p_fnum2: {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      if (te->IsEnabled()) {
	bool vv = (val!=0);
	if (te->GetToolTip()->GetText()->
	    Contains("rebin",TString::kIgnoreCase)) {
	  vv = val>1;
	}
	if (vv) {
	  te->ChangeBackground(fCol[col-1]);
	}
	else {
	  te->ChangeBackground(fWhite);
	}
      }
      break;
    }
    case p_chk: {
      TGCheckButton *te = (TGCheckButton*) pp->field;
      if (val) {
	te->ChangeBackground(fCol[col-1]);
      }
      else {
	te->ChangeBackground(fGrey);
      }
      break;
    }
    default:;
    }
  }

}

void ParDlg::UpdateField(int nn) {
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;
  Pmap* pp = &Plist[nn];

  TQObject* tq = (TQObject*) pp->field;
  tq->BlockAllSignals(true);

  Float_t val=0;
  Bool_t bb;

  int act = (pp->cmd>>4)&0xF;

  switch (pp->type) {
  case p_inum: {
    TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
    Int_t *dat = ((Int_t*)pp->data) + pp->off;
    val=*dat;
    if (act==6) { //проверка Len кратно 3 или 4
      //int ll = (nn)/nfld; //line number
      //if (ll<pmax) {
      cpar.Len[pp->off]=cpar.ChkLen(pp->off,crs->module);
      //}
    }
    if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      *dat = te->GetNumMax();
    }
    if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      *dat = te->GetNumMin();
    }
    te->SetNumber(*dat);
  }
    break;
  case p_fnum:
  case p_fnum2: {
    TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
    Float_t *dat = ((Float_t*)pp->data) + pp->off;
    val = *dat;
    if (te->GetNumLimits()==lim && *dat > te->GetNumMax()) {
      *dat = te->GetNumMax();
    }
    if (te->GetNumLimits()==lim && *dat < te->GetNumMin()) {
      *dat = te->GetNumMin();
    }
    te->SetNumber(*dat);
  }
    break;
  case p_chk: {
    TGCheckButton *te = (TGCheckButton*) pp->field;
    EButtonState st = te->GetState();
    te->SetEnabled();
    bb = *((Bool_t*)pp->data+pp->off);
    val=bb;
    te->SetState((EButtonState) bb);
    if (st==kButtonDisabled) {
      te->SetEnabled(false);
    }
    //cout << "p_chk: " << bb << endl;
  }
    break;
  case p_cmb: {
    TGComboBox *te = (TGComboBox*) pp->field;
    int line = nn/nfld;
    int sel = *((Int_t*)pp->data+pp->off);
    // update group names in combo
    for (int i=0;i<MAX_TP;i++) { //ZZ
      TGTextLBEntry* ent=
	(TGTextLBEntry*)te->GetListBox()->GetEntry(i+1); //ZZ
      ent->SetText(new TGString(opt.ch_name[i]));
    }
    te->Layout();

    if (line<chanpar->nrows) {
      ColorLine(line,tcol[sel-1]);
    }
    te->Select(sel,false);
  }
    break;
  case p_txt: {
    TGTextEntry *te = (TGTextEntry*) pp->field;
    te->SetText((char*) pp->data);
  }
    break;
  case p_but:
    if (pp->all) {
      TGTextButton *te = (TGTextButton*) pp->field;
      if (opt.chkall==0) {
	te->SetText("*");
	ColorLine(MAX_CH,fMagenta);
      }
      else if (opt.chkall==1) {
	te->SetText("all");
	ColorLine(MAX_CH,fGreen);
      }
      else {
	te->SetText("ALL");
	ColorLine(MAX_CH,fBlue);
      }
    }
    break;
  case p_chn: {
    TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
    te->SetNumber(pp->off);
  }
    break;
  case p_stat:
    if (tq) {
      Double_t *dat = ((Double_t*)pp->data) + pp->off;
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      if (*dat) {
	te->SetNumber(*dat);
      }
      else {
	te->SetText("");
      }
    }
    break;
  default:
    cout << "UpdateField: unknown pp->type: " << pp->type << endl;
  } //switch

  DoColor(pp,val);

  //disble some fields, if needed

  UInt_t bit9=(pp->cmd & 0x200);
  if (bit9) { //disble fields not existing in certain devices

    if ((pp->data==&cpar.Smpl || pp->data==&cpar.F24)
	&& (crs->Fmode!=1 || crs->module<41)) {
      EnableField(nn,false);
    }
    else if ((pp->data==&cpar.St_Per)
	&& (crs->Fmode!=1 || crs->module<35)) {
      EnableField(nn,false);
    }
    else if ((pp->data==&cpar.DTW)
	&& crs->Fmode!=1) {
      EnableField(nn,false);
    }
    else if (pp->data==&opt.ntof_period) {
      EnableField(nn,false);
    }

  }

  UInt_t bit10=(pp->cmd & 0x400);

  if (bit10) {
    //EnableField(nn,opt.b_ntof);
  }

  //TGNumberEntryField *wg = (TGNumberEntryField*) pp->field;
  //cout << "bit0: " << wg->IsEnabled() << " " << pp->cmd << endl;

  tq->BlockAllSignals(false);

}

void ParDlg::UpdateColumn(int id) {
  int kk = (id-1)%nfld; //column number
  int nn = chanpar->nrows+MAX_TP+1;
  for (int i=0;i<nn;i++) {
    // cout << "clmn: " << i*nfld+kk << endl;
    UpdateField(i*nfld+kk);
  }
}

void ParDlg::Update() {
  for (UInt_t i=0;i<Plist.size();i++) {
    UpdateField(i);
  }
}

void ParDlg::EnableField(int nn, bool state) {

  Pmap* pp = &Plist[nn];
	
  //TQObject* tq = (TQObject*) pp->field;
  //tq->BlockAllSignals(true);
  switch (pp->type) {
  case p_inum:
  case p_fnum:
  case p_fnum2:
    {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      TString str = TString(te->GetParent()->GetName());
      if (str.Contains("NumberEntry")) {
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
  case p_but: {
    TGButton *te = (TGButton*) pp->field;
    te->SetEnabled(state);
  }
    break;
  default: ;
  } //switch

  //tq->BlockAllSignals(false);
}

void ParDlg::AllEnabled(bool state) {

  for (UInt_t i=0;i<Plist.size();i++) {
    EnableField(i,state);
  }
}

void ParDlg::DaqDisable() {

  for (UInt_t i=0;i<Plist.size();i++) {
    Pmap* pp = &Plist[i];
    if (pp->cmd & 0x100) {
      TGCheckButton* wg = (TGCheckButton*) pp->field;
      Clist[i]=wg->IsEnabled();
      // const char* col=KGRN;
      // if (pp->type == p_chk)
      // 	col = BRED;
      // prnt("ss d d d ds;",col,"daqdis: ",i,pp->type,pp->cmd,(int)wg->IsEnabled(),RST);
      EnableField(i,false);
      //prnt("ss d d d ds;",col,"daqdis2: ",i,pp->type,pp->cmd,(int)wg->IsEnabled(),RST);
    }
  }
}

void ParDlg::DaqEnable() {

  for (UInt_t i=0;i<Plist.size();i++) {
    Pmap* pp = &Plist[i];
    if (pp->cmd & 0x100) {
      EnableField(i,Clist[i]);
    }
  }
}

TGFrame *ParDlg::FindWidget(void* p) {
  //finds widget using address of asigned parameter
  // !!!!!!! pp->off is not implemented here
  for (piter pp = Plist.begin(); pp != Plist.end(); pp++) {
    if (pp->data == p) {
      return pp->field;
    }
  }
  return 0;
}

void ParDlg::Check_opt(TGHorizontalFrame *hfr1, int width, void* x1,
			  const char* tip1, UInt_t cmd1, const char* cname) {

  int id;

  if (x1!=NULL) {
    id = Plist.size()+1;
    TGCheckButton *fchk = new TGCheckButton(hfr1, cname, id);
    DoMap(fchk,x1,p_chk,0,cmd1);
    fchk->SetToolTipText(tip1);

    if (width) {
      fchk->ChangeOptions(fchk->GetOptions()|kFixedWidth);
      fchk->SetWidth(width);
    }

    // int wd = fchk->GetWidth();
    // int x1 = (width-wd)/2+10;
    // int x2 = 7+width-x1-wd; //7 - from laylt4

    //cout << "wd: " << width << " " << wd << " " << x1 << " " << x2 << endl;



    // hfr1->AddFrame(fchk,
    // 		   new TGLayoutHints(kLHintsLeft|kLHintsCenterY,x1,x2,1,1));



    //fchk->SetWidth(52);
    hfr1->AddFrame(fchk,LayLT2);
    //fchk->SetWidth(width);
    fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  }
  else {
    TGLabel* fskip = new TGLabel(hfr1, "");
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    hfr1->AddFrame(fskip,LayLT4);
  }  
}

void ParDlg::Num_opt(TGHorizontalFrame *hfr1, int width, void* x1, void* x1a,
			const char* tip1, TGNumberFormat::EStyle style1,
			double min1, double max1, UInt_t cmd1,
			TGLayoutHints* Lay) {

  int id;
  P_Def pdef1;

  TGNumberEntry* fNum1;
  TGNumberEntryField* fNum2;
  TGLabel* fskip;
  
  if (style1==k_int || style1==k_hex) {
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

  //if (x1!=NULL) {
  if (style1!=k_lab) {
    id = Plist.size()+1;
    if (width>0) {
      fNum1 = new TGNumberEntry(hfr1, 0, 0, id, style1, 
				TGNumberFormat::kNEAAnyNumber,
				limits,min1,max1);
      fNum1->SetWidth(width);
      hfr1->AddFrame(fNum1,Lay);
      fNum2=fNum1->GetNumberEntry();
    }
    else {
      width=-width;
      fNum2 = new TGNumberEntryField(hfr1, id, 0, style1,
				     TGNumberFormat::kNEAAnyNumber,
				     limits,min1,max1);
      fNum2->SetWidth(width);
      hfr1->AddFrame(fNum2,Lay);
    }

    DoMap(fNum2,x1,pdef1,0,cmd1,x1a);
    fNum2->SetToolTipText(tip1);
    fNum2->Connect("TextChanged(char*)", "ParParDlg", this, "DoDaqNum()");
  }
  else {
    fskip = new TGLabel(hfr1, (char*) x1);
    fskip->ChangeOptions(fskip->GetOptions()|kFixedWidth);
    fskip->SetWidth(width);
    hfr1->AddFrame(fskip,Lay);
  }  
}

void ParDlg::AddLine_opt(TGCompositeFrame* frame, int width,
			 void *x1, void *x2, 
			 const char* tip1, const char* tip2,
			 const char* label,
			 TGNumberFormat::EStyle style1, 
			 TGNumberFormat::EStyle style2,
			 double min1, double max1,
			 double min2, double max2, UInt_t cmd1, UInt_t cmd2,
			 TGLayoutHints* Lay1, TGLayoutHints* Lay2) {

  if (!Lay1) Lay1 = LayLT4;
  if (!Lay2) Lay2 = LayLT4;

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);

  frame->AddFrame(hfr1);

  if (style1==k_chk)
    Check_opt(hfr1,width,x1,tip1,cmd1,"Strig");
  else
    Num_opt(hfr1,width,x1,0,tip1,style1,min1,max1,cmd1,Lay1);

  Num_opt(hfr1,width,x2,0,tip2,style2,min2,max2,cmd2,Lay2);

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,LayLT4);

}

void ParDlg::AddLine_1opt(TGCompositeFrame* frame, int width, void *x1,
			  void *x1a, const char* tip1, const char* label,
			  TGNumberFormat::EStyle style1, 
			  double min1, double max1,
			  UInt_t cmd1, TGLayoutHints* Lay1) {
  if (!Lay1) Lay1 = LayLT4;

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  if (style1==k_chk)
    Check_opt(hfr1,width,x1,tip1,cmd1,"");
  else
    Num_opt(hfr1,width,x1,x1a,tip1,style1,min1,max1,cmd1,Lay1);

  TGLabel* fLabel = new TGLabel(hfr1, label);
  hfr1->AddFrame(fLabel,LayLT4);
}

//------ TrigFrame -------
TrigFrame::TrigFrame(TGGroupFrame *p, int opt)
  :TGHorizontalFrame(p)
{
  const char* tip1 = "Hardware trigger type";
  const char* tip2[3] = {
			 "Trigger on Discriminator (normal trigger)",
			 "Trigger on START channel",
			 "Use hardware coincidence scheme",
  };
  //const char* label[3] = {"Normal","START","Hardw.Coinc."};
  const char* label0[3] = {"D","S","Hw.Coin."};
  const char* label1[3] = {"Discr.","START","Hw.Coinc."};
  const char** label = label0;

  TGLayoutHints* LayLC1 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 1, 0, 1, 1);

  //TGHorizontalFrame *hfr = new TGHorizontalFrame(frame);
  p->AddFrame(this,new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 0, 5, 5));

  if (opt) {
    TGTextEntry* fLabel=new TGTextEntry(this, "Trigger:");
    fLabel->ChangeOptions(fLabel->GetOptions()|kRaisedFrame);
    fLabel->SetState(false);
    fLabel->SetToolTipText(tip1);

    //TGLabel* fLabel = new TGLabel(this, "Trigger:");
    this->AddFrame(fLabel,LayLC1);
    label = label1;
  }

  for (int i=0;i<3;i++) {
    fchkTrig[i] = new TGCheckButton(this, label[i], i);
    fchkTrig[i]->SetToolTipText(tip2[i]);
    fchkTrig[i]->Connect("Clicked()", "TrigFrame", this, "DoCheckTrigger()");
    this->AddFrame(fchkTrig[i],LayLC1);
  }
}

void TrigFrame::DoCheckTrigger() {

  TGCheckButton *te = (TGCheckButton*) gTQSender;
  Int_t id = te->WidgetId();

  cpar.Trigger = id;
  
  parpar->tTrig->UpdateTrigger();
  chanpar->tTrig->UpdateTrigger();

#ifdef CYUSB
  if (crs->b_acq) {// && !jtrig) {
    crs->Command2(4,0,0,0);
    crs->SetPar();
    gzFile ff = gzopen("last.par","wb");
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
    crs->Command2(3,0,0,0);
  }
#endif

}

void TrigFrame::UpdateTrigger() {

  Pixel_t col[3] = {fGrey,fGreen,fMagenta};
  Pixel_t fcol;
  for (int i=0;i<3;i++) {
    int state = (cpar.Trigger==i);
    fchkTrig[i]->SetState((EButtonState) state);
    if (state)
      fcol=col[i];
    else
      fcol=col[0];
    fchkTrig[i]->ChangeBackground(fcol);
  }
  this->ChangeBackground(col[cpar.Trigger]);

  if (chanpar) {
    chanpar->cGrp->ChangeBackground(col[cpar.Trigger]);
  }
}

//------ ParParDlg -------

ParParDlg::ParParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
  //soft_list.clear();
  //hard_list.clear();

  AddFrame(fDock, LayEE0);
  fDock->SetWindowName("Parameters");  

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));

  AddFileName(fMain);

  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,LayEE0);

  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				1, 1, kVerticalFrame);
  fCanvas1->SetContainer(fcont1);
  fCanvas1->GetViewPort()->SetCleanup(kDeepCleanup);

  TGHorizontalFrame *hF1 = new TGHorizontalFrame(fcont1);
  fcont1->AddFrame(hF1,LayEE0);

  TGVerticalFrame *fV1 = new TGVerticalFrame(hF1, 1, 1, kFixedWidth|kSunkenFrame);

  hF1->AddFrame(fV1,new TGLayoutHints(kLHintsLeft |kLHintsExpandY));


  TGVSplitter *vsplitter = new TGVSplitter(hF1,2,h);
  vsplitter->ChangeOptions(vsplitter->GetOptions()|kFixedSize);
  vsplitter->SetFrame(fV1, kTRUE);
  hF1->AddFrame(vsplitter, new TGLayoutHints(kLHintsLeft | kLHintsTop));

  int ww = AddFiles(fV1);
  ww = TMath::Max(ww,AddOpt(fV1));
  ww = TMath::Max(ww,AddNtof(fV1));
  ww = TMath::Max(ww,AddLogic(fV1));

  fV1->Resize(ww+10,1);

  TGVerticalFrame *fV2 = new TGVerticalFrame(hF1, 1, 1, kSunkenFrame);
  hF1->AddFrame(fV2,new TGLayoutHints(kLHintsExpandX|kLHintsExpandY));
  AddExpert(fV2);

  if (crs->module==17) {
    //#ifdef SIMUL
    AddSimul(fV2);
    //#endif
  }

  // for (UInt_t i=0;i<Plist.size();i++) {
  //   Pmap* pp = &Plist[i];
  //   const char* col=KGRN;
  //   if (pp->cmd & 1)
  //     col = BRED;
  //   prnt("ss d d d ds;",col,"parpar: ",i,pp->type,pp->all,pp->cmd,RST);
  // }

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
  fchk->SetWidth(220);

  hframe1->AddFrame(fchk,LayCC1);
  DoMap(fchk,opt_chk,p_chk,0,0x100|(7<<1));
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");

  //fchk->SetState(kButtonDown,false);

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hframe1, *compr, 2, id, k_int, 
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   0,9);
  hframe1->AddFrame(fNum1,LayCC1);
  DoMap(fNum1->GetNumberEntry(),compr,p_inum,0,0x100);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");

  fNum1->GetNumberEntry()->SetToolTipText("Compression factor [0-9]: 0 - no compression (fast); 9- maximum compression (slow)");
  TGLabel* fLabel = new TGLabel(hframe1, "compr.");
  hframe1->AddFrame(fLabel,LayCC1);

  //fProc
  if (rflag) {
    /*
    id = Plist.size()+1;
    TGCheckButton *fchk2 = new TGCheckButton(hframe1, "fProc", id);
    fchk2->SetToolTipText("input raw: Checked - write processed events; unchecked - write direct raw stream\ninput dec: Checked - reanalyse .dec file with new coincidence conditions");
    //fchk2->SetName(txt);
    //fchk2->ChangeOptions(fchk2->GetOptions()|kFixedWidth);
    //fchk2->SetWidth(230);

    hframe1->AddFrame(fchk2,LayCC1);
    DoMap(fchk2,rflag,p_chk,0,7<<1);
    fchk2->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
    */
  }

}

void ParParDlg::AddFileName(TGCompositeFrame* frame) {
  int id;

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hframe1,LayLT7);

  TGLabel* fLabel = new TGLabel(hframe1, "Filename:");
  hframe1->AddFrame(fLabel,LayCC1);

  //id = Plist.size()+1;
  TGTextButton *fbut = new TGTextButton(hframe1,"Select...",0);
  hframe1->AddFrame(fbut, LayCC1);
  //DoMap(fbut,opt.Filename,p_but,0,0x100);
  fbut->Connect("Clicked()", "ParDlg", this, "DoOpen()");

  id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(frame,opt.Filename, id);
  //tt->SetDefaultSize(380,20);
  tt->SetMaxLength(sizeof(opt.Filename)-1);
  frame->AddFrame(tt,LayET1);
  DoMap(tt,opt.Filename,p_txt,0,0x100);
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");

}

int ParParDlg::AddFiles(TGCompositeFrame* frame) {
  int id;
  char txt[99];

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Files", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);

  AddChk(fF6,"Write raw data [Filename].raw",&opt.raw_write,&opt.raw_compr,0);
  AddChk(fF6,"Write decoded data [Filename].dec",&opt.dec_write,&opt.dec_compr,0);
  AddChk(fF6,"Write root histograms [Filename].root",&opt.root_write,&opt.root_compr,0);

  TGCheckButton *fchk;
  TGHorizontalFrame *hframe3 = new TGHorizontalFrame(fF6,10,10);
  fF6->AddFrame(hframe3,LayLT0);

  id = Plist.size()+1;
  sprintf(txt,"DirectRaw");
  fchk = new TGCheckButton(hframe3, txt, id);
  // fchk->SetName(txt);
  fchk->SetToolTipText("Don't decode raw data: write direct raw stream");
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.directraw,p_chk,0,0x100|(6<<1));
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");

  id = Plist.size()+1;
  sprintf(txt,"CheckDSP");
  fchk = new TGCheckButton(hframe3, txt, id);
  // fchk->SetName(txt);
  fchk->SetToolTipText("Compare software pulse analysis vs DSP data");
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.checkdsp,p_chk,0,0x100|(5<<1));
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoChk(Bool_t)");

  id = Plist.size()+1;
  fchk = new TGCheckButton(hframe3, "fProc", id);
  fchk->SetToolTipText("input raw: Checked - write processed events; unchecked - write direct raw stream\ninput dec: Checked - reanalyse .dec file with new coincidence conditions");
  //fchk2->SetName(txt);
  //fchk2->ChangeOptions(fchk2->GetOptions()|kFixedWidth);
  //fchk2->SetWidth(230);
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.fProc,p_chk,0,0x100|(7<<1));
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");

  id = Plist.size()+1;
  fchk = new TGCheckButton(hframe3, "fTxt", id);
  fchk->SetToolTipText("Checked - write events in text file [Filename].txt. Workd only in single-threaded mode.");
  hframe3->AddFrame(fchk,LayCC1);
  DoMap(fchk,&opt.fTxt,p_chk,0,0x100|(2<<1));
  fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");

  fF6->Resize();
  return fF6->GetDefaultWidth();
} //AddFiles

int ParParDlg::AddOpt(TGCompositeFrame* frame) {

  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);

  // 2 column, n rows
  //fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

  tip1= "Number of used channels";
  tip2= "Number of threads (1 - no multithreading)";
  label="Number of channels/ threads";
  AddLine_opt(fF6,ww,&opt.Nchan,&opt.nthreads,tip1,tip2,label,k_int,k_int,1,MAX_CH,1,CRS::MAXTHREADS,0x100,0x100);

  tip1= "Analysis start (in sec) - only for analyzing files";
  tip2= "Analysis/acquisition stop (in sec)";
  label="Tstart / Tstop";
  AddLine_opt(fF6,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_r0,k_r0,0,0,0,0,3<<1,1<<1);

  tip1= "";
  tip2= "Delay between drawing events (in msec)";
  label="DrawEvent delay";
  AddLine_opt(fF6,ww,NULL,&opt.tsleep,tip1,tip2,label,k_lab,k_int,100,10000,500,10000,0,7<<4);

  tip1= "Size of the USB buffer in kilobytes (1024 is OK)\n"
    "Small for low count rate; large for high count rate";
  tip2= "Size of the READ buffer in kilobytes\n"
    "As large as possible for faster speed, but may run out of memory";
  label="USB/READ buffer size";
  AddLine_opt(fF6,ww,&opt.usb_size,&opt.rbuf_size,tip1,tip2,label,k_int,k_int,
	      1,2048,1,64000,0x100,0x100|(1<<4));

  tip1= "Maximal size of the event list:\nNumber of events available for viewing in the Events Tab";
  tip2= "Event lag:\nMaximal expected number of lagged events (see Errors/Event lag exceeded\nMust be at least 1)";
  label="Event_list size / Event lag";
  AddLine_opt(fF6,ww,&opt.ev_max,&opt.ev_min,tip1,tip2,label,k_int,k_int,1,1000000,1,1000000);

  tip1= "[CRS-8/16] Sampling rate, MHz (0: 100, 1: 50, 2: 25 .. 14: ~0.006)";
  tip2= "[CRS-8/16] Soft start (imitator) period (sec):\n"
    "(0->use LEMO START input)\n"
    "If non-zero, LEMO START input is blocked\n"
    "For CRS-8/128:  1: 0.08, 2: 0.17, 3: 0.33, 4: 0.67, 5: 1.34, 6: 2.68, 7: 5.36, 8: 10.7\n"
    "For AK-32:           1: 0.05, 2: 0.11, 3: 0.21, 4: 0.43, 5: 0.86, 6: 1.72, 7: 3.44, 8: 6.87";
  label="Sampling Rate / Start period";
  // cout << "Smpl: " << cpar.Smpl << endl;
  AddLine_opt(fF6,ww,&cpar.Smpl,&cpar.St_Per,tip1,tip2,label,k_int,k_int,0,14,0,8,0x200|(2<<1)|1,0x200|(6<<1)|1);


  // tip1= "[CRS-8/16] Force trigger on all active channels from START signal.\n"
  //   "Normal trigger is disabled";
  // tip2= "START input channel dead time (in samples). All channels are inhibited during START dead time";
  // label="START trigger/START dead time";
  // AddLine_opt(fF6,ww,&cpar.St_trig,&cpar.DTW,tip1,tip2,label,k_chk,k_int,
  // 	      0,0,1,2e9,0x200|(3<<1)|1,0x200|1);

  tip1="24-bit data format (only for CRS-8/16, CRS-128, AK-32)";
  tip2= "START input channel dead time (in samples). All channels are inhibited during START dead time";
  label="F24/START dead time";
  AddLine_opt(fF6,ww,&cpar.F24,&cpar.DTW,tip1,tip2,label,k_int,k_int,
	      0,1,1,2e9,0x200|(5<<1)|1,0x200|1);


  // if (crs->module<41 || crs->module>70) {
  //   int id = Plist.size()-2;
  //   EnableField(id,false);
  //   EnableField(id-1,false);
  //   EnableField(id-2,false);
  //   if (crs->module==35) EnableField(id-1,true);
  // }

  fF6->Resize();
  return fF6->GetDefaultWidth();

}

int ParParDlg::AddNtof(TGCompositeFrame* frame) {

  int ww=70;








  TGGroupFrame* fF6 = new TGGroupFrame(frame, "NTOF Analysis", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);





  //b_ntof:

  // TGHorizontalFrame *hfr1 = new TGHorizontalFrame(fF6);
  // fF6->AddFrame(hfr1);

  // int id = Plist.size()+1;
  // TGCheckButton* fchk = new TGCheckButton(hfr1, "on/off", id);
  // DoMap(fchk,&opt.b_ntof,p_chk,0,0x100|2<<1);
  // fchk->SetToolTipText("b_ntof: Enable/disable NTOF analysis");
  // //fchk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  // fchk->Connect("Toggled(Bool_t)", "ParParDlg", this, "DoCheckNtof(Bool_t)");
  // hfr1->AddFrame(fchk,LayCC1);







  tip1= "Ntof period (mks) (should be always zero if unsure why it's needed)";
  tip2= "Ntof start channel (255 for START input)";
  label="Ntof period / start channel";
  opt.ntof_period = 0;
  AddLine_opt(fF6,ww,&opt.ntof_period,&opt.start_ch,tip1,tip2,label,k_r1,k_int,
	      0,1e9,0,255,0x200|0x400,0x400);

  tip1= "Ntof Flight path (in meters) for Ntof-Energy conversion";
  tip2= "Ntof Time offset (in mks) for Ntof-Energy conversion";
  label="Ntof Flpath / Ntof Zero";
  AddLine_opt(fF6,ww,&opt.Flpath,&opt.TofZero,tip1,tip2,label,k_r0,k_r0,
	      0,1e9,-1e9,1e9,0x400,0x400);

  fF6->Resize();
  return fF6->GetDefaultWidth();

}

int ParParDlg::AddLogic(TGCompositeFrame* frame) {

  int ww=70;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Event Logic", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);

  //AddTrigger(fF6);
  tTrig = new TrigFrame(fF6,1);

  tip1= "Coincidence window for making events (in samples)";
  tip2= "Veto window (in samples): \nsubsequent pulses from the same channel coming within this window are ignored";
  label="Coincidence (smp), veto (smp)";
  AddLine_opt(fF6,ww,&opt.tgate,&opt.tveto,tip1,tip2,label,k_int,k_int,0,10000,0,1000);

  tip1= "Minimal multiplicity";
  tip2= "Maximal multiplicity";
  label="Multiplicity (min, max)";
  AddLine_opt(fF6,ww,&opt.mult1,&opt.mult2,tip1,tip2,label,k_int,k_int,
	      1,9999,1,9999);

  tip1= "";
  tip2= "Main trigegr condition (cut).\nThis condition is used for selecting events which are written as decoded events\nSee \"Plots\" for making conditions\nUse this cut number as a main trigger condition.\nIf set to zero - write all events.";
  label="Main trigger";
  AddLine_opt(fF6,ww,NULL,&opt.maintrig,tip1,tip2,label,k_lab,k_int,
	      0,0,0,MAXCUTS);

  fF6->Resize();

  // soft_list.push_back(FindWidget(&opt.tgate));
  // soft_list.push_back(FindWidget(&opt.tveto));
  // soft_list.push_back(FindWidget(&opt.mult1));
  // soft_list.push_back(FindWidget(&opt.mult2));
  // soft_list.push_back(FindWidget(&opt.maintrig));

  return fF6->GetDefaultWidth();

}

void ParParDlg::FakeTxt() {

  TGTextEntry *te = (TGTextEntry*) gTQSender;
  te->SetText(mask0);

}

void ParParDlg::AddLine_dec_format(TGCompositeFrame* frame, int width) {

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);
  TGTextEntry* tv = new TGTextEntry(hfr1,mask0);
  //TGTextView* tv = new TGTextView(hfr1);
  //tv->AddLine("test");
  hfr1->AddFrame(tv,LayLT4);
  tv->SetWidth(width);

  //tv->SetState(false);
  //tv->SetEditable(false);
  //tv->SetEditDisabled(kEditDisable);
  //tv->SetEnabled(false);
  tv->Connect("TextChanged(char*)", "ParParDlg", this, "FakeTxt()");
  tv->SetToolTipText(tip_dec);

  hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1);

  int id = Plist.size()+1;
  TGTextEntry* tt = new TGTextEntry(hfr1,opt.dec_mask,id);
  tt->SetWidth(width);
  tt->SetToolTipText(tip_dec);
  hfr1->AddFrame(tt,LayLT4);
  DoMap(tt,opt.dec_mask,p_txt,0,0x100|(3<<1));
  tt->Connect("TextChanged(char*)", "ParDlg", this, "DoTxt()");
  //tt->Connect("TextChanged(char*)", "ParParDlg", this, "DoDecFormat()");
  
  TGLabel* fLabel = new TGLabel(hfr1, "Dec mask");
  hfr1->AddFrame(fLabel,LayLT4);

}

int ParParDlg::AddExpert(TGCompositeFrame* frame) {

  int ww=70;
  const char *tip1, *label;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Expert", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);

  tip1= "Decoded data format";
  label="Dec format";
  AddLine_1opt(fF6,ww,&opt.dec_format,0,tip1,label,k_int,79,81);

  AddLine_dec_format(fF6,150);

  tip1=
    "Hex bitwise raw data transfer mask:\n"
    "default value: FFFF\n\n"

    "Meaning of the bits:\n"
    "0 - timestamp (must be always set to 1)\n"
    "1 - spin/counter\n"
    "2 - pulse data 11 bit\n"
    "3 - pulse data 16 bit\n"
    "4 - C A (11 bit)\n"
    "5 - RX QX (11 bit)\n"
    "6 - AY H\n"
    "7 - \n"
    "8 - RX C (16 bit)\n"
    "9 - coinc.counter A (16 bit)\n"
    "10 - OVF.counter QX (16 bit)\n"
    "11 - Counter\n"
    "12 - OVF"
    ;
  label="Raw mask";
  AddLine_1opt(fF6,ww,&cpar.RMask,0,tip1,label,k_hex,0,0xFFFF);

  /*
  tip1= "";
  label="Bitmask for START";
  AddLine_1opt(fF6,ww,cpar.coinc_w,0,tip1,label,k_int,1,1023);

  tip1= "";
  label="Bitmask for discriminator";
  AddLine_1opt(fF6,ww,cpar.coinc_w,0,tip1,label,k_int,1,1023);

  tip1= "";
  label="Bitmask for coincidences/RD";
  AddLine_1opt(fF6,ww,cpar.coinc_w,0,tip1,label,k_int,1,1023);

  tip1= "";
  label="Repeated triggering type";
  AddLine_1opt(fF6,ww,cpar.coinc_w,0,tip1,label,k_int,1,1023);

  tip1= "";
  label="Type 3 discriminator length";
  AddLine_1opt(fF6,ww,cpar.coinc_w,0,tip1,label,k_int,1,1023);
  */

  // tip1= "";
  // label="Thr2: low threshold for trigger type 3,4";
  // AddLine_1opt(fF6,ww,&cpar.Thr2,&opt.sThr2,tip1,label,k_int,-1000,1000);

  tip1= "ADCM period in nsec: 10 for ADCM32; 16 for ADCM64";
  label="adcm period";
  AddLine_1opt(fF6,ww,&opt.adcm_period,0,tip1,label,k_r0,1,1000);

  tip1= "Add random number when filling all histograms";
  label="Add random";
  AddLine_1opt(fF6,ww,&opt.addrandom,0,tip1,label,k_chk,1,1000);

  tip1= "Max number of rows in Channels tab";
  label="Nrows";
  AddLine_1opt(fF6,ww,&opt.Nrows,0,tip1,label,k_int,2,64);


  fF6->Resize();
  //fF6->ChangeBackground(fRed);

  return fF6->GetDefaultWidth();

}

int ParParDlg::AddSimul(TGCompositeFrame* frame) {

  int ww=70;
  const char *tip1, *label;

  TGGroupFrame* fF6 = new TGGroupFrame(frame, "Simul", kVerticalFrame);
  fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  frame->AddFrame(fF6, LayLT1);

  tip1= "";

  label= "use 'romana 17' to start in simulation mode\n"
    "'N buf' simulates [nbuf x Event_list_size] events.\n"
    "Set coincidence window & peak parameters\n"
    ;
  TGLabel* fLabel = new TGLabel(fF6, label);
  fF6->AddFrame(fLabel,LayLT4);
  


  label= "opt.Period in ns";
  AddLine_1opt(fF6,ww,&opt.SimSim[0],0,label,label,k_r0,1,99999);

  label="cpar.Pre in smp";
  AddLine_1opt(fF6,ww,&opt.SimSim[1],0,label,label,k_r0,0,99);

  label="cpar.Len in smp";
  AddLine_1opt(fF6,ww,&opt.SimSim[2],0,label,label,k_r0,0,9999);

  label= "Pulse type: 0 - gauss; 1 - RC; 2 - Fourier";
  AddLine_1opt(fF6,ww,&opt.SimSim[3],0,label,label,k_r0,0,2);

  label= "Par0";
  tip1= "Pulse type0: Pulse Amplitude\nPulse type1: Pulse Amplitude\nPulse type2: Fourier 1st period in samples";
  AddLine_1opt(fF6,ww,&opt.SimSim[4],0,tip1,label,k_r1,0,99999);

  label= "Par1";
  tip1="Pulse type0: Sigma in ns\nPulse type1: RC_Width in ns\nPulse type2: Fourier 2nd period in samples (0 - skip)";
  AddLine_1opt(fF6,ww,&opt.SimSim[5],0,tip1,label,k_r1,0,99);

  label= "Par2";
  tip1="Pulse type0: --\nPulse type1: Pulse RC in ns\nPulse type2: Fourier 3rd period in samples (0 - skip)";
  AddLine_1opt(fF6,ww,&opt.SimSim[6],0,tip1,label,k_r1,0,99);


  label= "Pos min in ns";
  tip1= "Pos: position of peak relative to discriminator (Pre)";
  AddLine_1opt(fF6,ww,&opt.SimSim[7],0,tip1,label,k_r0,-99999,99999);

  label= "Pos spread in ns";
  tip1= "Pos: position of peak relative to discriminator (Pre)";
  AddLine_1opt(fF6,ww,&opt.SimSim[8],0,tip1,label,k_r0,0,99999);

  label= "Coincidence window in ns";
  tip1= "Coincidence window in ns = Max time spread between two pulses.\n"
    "Must be smaller than 'Coincidence' in samples (opt.tgate).";
  AddLine_1opt(fF6,ww,&opt.SimSim[9],0,tip1,label,k_r0,0,99999);

  label="Time delta";
  tip1= "difference between Simul and Pos (in ns)";
  AddLine_1opt(fF6,ww,&opt.SimSim[10],0,tip1,label,k_r1,-99,99);

  label="Noise";
  tip1= "amplitude of gaussian noise added to the signal";
  AddLine_1opt(fF6,ww,&opt.SimSim[11],0,tip1,label,k_r1,0,999);

  // label="Simul delta";
  // tip1= "difference between Simul and Pos (in ns)";
  // AddLine_1opt(fF6,ww,&opt.SimSim[9],tip1,label,k_r1,-99,99);


  // label="CFD delay";
  // AddLine_1opt(fF6,ww,&opt.SimSim[10],label,label,k_r0,-99,99);

  // label="CFD fraction";
  // AddLine_1opt(fF6,ww,&opt.SimSim[11],label,label,k_r1,-99,99);

  //label="sS";
  //AddLine_1opt(fF6,ww,&opt.sS[MAX_CH],tip1,label,k_int,-99,99);

  fF6->Resize();
  //fF6->ChangeBackground(fRed);

  return fF6->GetDefaultWidth();

}


void ParParDlg::DoCheckNtof(Bool_t on) {

  DoDaqChk(on);
  Update();

}


void ParParDlg::Update() {
  ParDlg::Update();
  tTrig->UpdateTrigger();
  MapSubwindows();
  Layout();
}

// void ParParDlg::UpdateLL(wlist &llist, Bool_t state) {
//   for (wlist::iterator it = llist.begin(); it != llist.end(); ++it) {
//     if ((*it)->InheritsFrom("TGButton")) {
//       ((TGButton*)(*it))->SetEnabled(state);
//     }
//     else {
//       ((TGTextEntry*)(*it))->SetEnabled(state);
//     }
//   }
// }

//------ HistParDlg -------

HistParDlg::HistParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
  k_int=TGNumberFormat::kNESInteger;
  k_r0=TGNumberFormat::kNESReal;
  k_r1=TGNumberFormat::kNESRealOne;

  k_chk=TGNumberFormat::kNESMDayYear;

  AddFrame(fDock, LayLE0);
  fDock->SetWindowName("Histograms");  

  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  fCanvas1 = new TGCanvas(fMain,w,h);
  //fMain->AddFrame(fCanvas1,LayEE0);
  fMain->AddFrame(fCanvas1,LayEE0);

  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
   				10, 10);
  fCanvas1->SetContainer(fcont1);

  AddHist(fcont1);
  AddHist_2d();

  fDock->Resize(fcont1->GetDefaultWidth()+20,0);

}

void HistParDlg::AddHist(TGCompositeFrame* frame2) {

  hcl->md_pulse=0;

  //cout << "AddHist: " << endl;
  TGGroupFrame* f1 = new TGGroupFrame(frame2,"1D Histograms",kHorizontalFrame);
  f1->SetTitlePos(TGGroupFrame::kCenter);
  frame2->AddFrame(f1, LayLT0);

  //left frame
  frame1d[0] = new TGVerticalFrame(f1);
  f1->AddFrame(frame1d[0], LayLT0);

  TGVertical3DLine* separator1 = new TGVertical3DLine(f1);
  f1->AddFrame(separator1,LayLE0);

  //right frame
  frame1d[1] = new TGVerticalFrame(f1);
  f1->AddFrame(frame1d[1], LayLT0);

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame1d[1]);
  TGHorizontalFrame *hfr2 = new TGHorizontalFrame(frame1d[1]);

  int n1=0;
  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->hnum < 50) n1++;
  }
  n1=(n1+2)/2;

  //cout << "Mlist.size: " << hcl->Mlist.size() << " " << n1 << endl;
  //bool hleft=true;

  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    //if (it->hnum==22) hleft=false; //hwrate -> right
    if (it->hnum < 50) { // 1d hist
      if (n1>0)
	AddLine_hist(frame1d[0],&*it);
      else
	AddLine_hist(frame1d[1],&*it);
      n1--;
    }
    else if (it->hnum == 51) { //mean pulses
      frame1d[1]->AddFrame(hfr1);
      AddLine_mean(hfr1,&*it);
      hcl->md_pulse=&*it;
    }
    else if (it->hnum == 52) { //mean deriv
      AddLine_mean(hfr1,&*it);
    }
    else if (it->hnum == 53) { //mean FFT
      AddLine_mean(hfr1,&*it);


      //checkbutton Double/Float
      if (hcl->md_pulse) {
	int id = Plist.size()+1;
	TGCheckButton *chk_hist2 = new TGCheckButton(hfr1, "", id);
	DoMap(chk_hist2,&hcl->md_pulse->hd->htp,p_chk,0,0x100|(2<<1)|(2<<4));
	chk_hist2->SetToolTipText("Double/Float");
	chk_hist2->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
	hfr1->AddFrame(chk_hist2,LayLT2);
      }


    }
    else if (it->hnum == 61) { //profilometer
      frame1d[1]->AddFrame(hfr2);
      AddLine_prof(hfr2,&*it);
    }
    else if (it->hnum == 62) //prof_int
      AddLine_prof_int(hfr2,&*it);
  }

#ifdef YUMO
  TGHorizontalFrame* hh = new TGHorizontalFrame(frame2);
  frame2->AddFrame(hh, LayLT0);

  frame2d = new TGGroupFrame(hh, "2D Histograms", kVerticalFrame);
  hh->AddFrame(frame2d, LayLT0);

  mdef_iter it1 = hcl->Find_Mdef(71);
  mdef_iter it2 = hcl->Find_Mdef(72);
  mdef_iter it3 = hcl->Find_Mdef(73);
  if (it1!=hcl->Mlist.end() && it2!=hcl->Mlist.end()) {
    TGGroupFrame* frameYumo = new TGGroupFrame(hh, "YUMO", kVerticalFrame);
    frameYumo->SetTitlePos(TGGroupFrame::kCenter);
    hh->AddFrame(frameYumo, LayLT0);

    Add_yumo(frameYumo,&*it1,&*it2,&*it3);
  }
#else
  frame2d = new TGGroupFrame(frame2, "2D Histograms", kVerticalFrame);
  frame2->AddFrame(frame2d, LayLT0);
#endif

  frame2d->SetTitlePos(TGGroupFrame::kCenter);

  TGHorizontalFrame* h2fr = new TGHorizontalFrame(frame2d);
  h2fr->ChangeOptions(h2fr->GetOptions()|kFixedWidth);
  h2fr->Resize(HFRAME_WIDTH,0);
  frame2d->AddFrame(h2fr,LayLT0);

  cmb1=new TGComboBox(h2fr,0);
  h2fr->AddFrame(cmb1,LayLC2);

  cmb2=new TGComboBox(h2fr,1);
  h2fr->AddFrame(cmb2,LayLC2);

  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->hnum<49) {
      if (it->hnum!=21 && it->hnum!=22) //Rate и HWRate не могут быть по X
	cmb1->AddEntry(it->name.Data(), it->hnum);
      if (it->hnum!=22) //HWRate не может быть и по Y
      cmb2->AddEntry(it->name.Data(), it->hnum);
    }
  }

  cmb1->Select(1,false);
  cmb2->Select(1,false);
  cmb1->Resize(80, 20);
  cmb2->Resize(80, 20);

  TGTextButton *fbut = new TGTextButton(h2fr," Add ",0);
  fbut->SetToolTipText("Add new type of 2d histogram. Low and Upper edge on\nX and Y axes are taken from the corresponding 1d histogram.");
  h2fr->AddFrame(fbut, LayRT0);
  fbut->Connect("Clicked()", "HistParDlg", this, "Add2d()");

}

void HistParDlg::AddHist_2d() {

  //cout << "AddHist_2d: " << endl;
  //сначала удаляем все строки с 2d, потом добавляем из Mlist, потом Layout


  //удаляем
  TGHorizontalFrame* fr = (TGHorizontalFrame*) list2d.First();
  while (fr) {
    //cout << "fr: " << fr << " "<<fr->GetName()<<" "<< fr->GetTitle() << endl;

    TGFrameElement *el;
    TIter next(fr->GetList());
    while ((el = (TGFrameElement *) next())) {
      fr->HideFrame(el->fFrame);
      fr->RemoveFrame(el->fFrame);
      //delete el->fFrame;
    }

    frame2d->HideFrame(fr);
    frame2d->RemoveFrame(fr);

    //TGHorizontalFrame* fr2 = fr;
    fr = (TGHorizontalFrame*) list2d.After(fr);
  }

  list2d.Clear();

  //добавляем
  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->is2d()) { // 2d hist
      AddLine_2d(frame2d,&*it);
    }
  }

  MapSubwindows();
  //myM->Resize(GetDefaultSize());
  Layout();
  MapWindow();
}

/*
void HistParDlg::RemHist_2d(TGCompositeFrame* frame2) {

  cout << "AddHist_2d: " << endl;
  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->hnum > 100) { // 2d hist
      AddLine_2d(frame2d,&*it);
    }
  }

}
*/

void HistParDlg::AddLine_hist(TGCompositeFrame* frame, Mdef* md) {
  double ww1=50;
  double ww=70;

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  //hfr1->ChangeOptions(hfr1->GetOptions()|kFixedWidth);
  //hfr1->Resize(HFRAME_WIDTH,0);
  frame->AddFrame(hfr1);

  int id;
  //int id0;

  TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&md->hd->b,p_chk,0,0x100);
  chk_hist->SetToolTipText("on/off");
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayLT2);








  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0.0001,10000);
  DoMap(fNum1->GetNumberEntry(),&md->hd->bins,p_fnum,0,0x100|(2<<4));
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
  DoMap(fNum2->GetNumberEntry(),&md->hd->min,p_fnum,0,0x100|(2<<4));
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
  DoMap(fNum3->GetNumberEntry(),&md->hd->max,p_fnum,0,0x100|(2<<4));
  fNum3->GetNumberEntry()->SetToolTipText("Upper edge");
  fNum3->SetWidth(ww);
  fNum3->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum3,LayLT2);

  //rebin
  id = Plist.size()+1;
  TGNumberEntry* fNum4 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAPositive,
					   lim,1,1000);
  DoMap(fNum4->GetNumberEntry(),&md->hd->rb,p_inum,0,4<<1|3<<4); //cyan + Update
  fNum4->GetNumberEntry()->SetToolTipText("Rebin (only for drawing)");
  fNum4->SetWidth(ww1);
  fNum4->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum4,LayLT2);




  //checkbutton Double/Float
  id = Plist.size()+1;
  TGCheckButton *chk_hist2 = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist2,&md->hd->htp,p_chk,0,0x100|(2<<1)|(2<<4));
  chk_hist2->SetToolTipText("Double/Float");
  chk_hist2->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  hfr1->AddFrame(chk_hist2,LayLT2);




  TGTextEntry *fLabel=new TGTextEntry(hfr1, md->name.Data());
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(md->tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT2);
}

void HistParDlg::AddLine_mean(TGHorizontalFrame *hfr1, Mdef* md) {
  char name[20];

  int id;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  sprintf(name,"b_pulse%d",id);
  chk_hist->SetName(name);
  DoMap(chk_hist,&md->hd->b,p_chk,0,0x100);
  chk_hist->SetToolTipText("on/off");
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);

  TString lab = "Mean_"+md->name;
  
  TGTextEntry *fLabel=new TGTextEntry(hfr1, lab.Data());
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(md->tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT5);

}

void HistParDlg::Add_prof_num(TGHorizontalFrame *hfr1, void *nnn, Int_t max,
			     P_Def pp, UInt_t cmd, const char* tip) {
  double ww1=26;
  int id;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  id = Plist.size()+1;
  TGNumberEntryField* fNum1 =
    new TGNumberEntryField(hfr1, id, 0, k_int,
			   TGNumberFormat::kNEAAnyNumber,lim,1,max);
  DoMap(fNum1,nnn,pp,0,cmd);
  fNum1->SetToolTipText(tip);
  fNum1->SetWidth(ww1);
  fNum1->Connect("TextChanged(char*)", "ParDlg", this,
		 "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT3);
}

void HistParDlg::AddLine_prof(TGHorizontalFrame *hfr1, Mdef* md) {
  int id;

  //main checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&md->hd->b,p_chk,0,0x100|(4<<4));
  chk_hist->SetToolTipText("on/off");
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayCC1);
  //id0=id;

  Add_prof_num(hfr1,&opt.prof_nx,16,p_inum,0x100|(2<<4),"Number of X-strips from ING-27");
  Add_prof_num(hfr1,&opt.prof_ny,16,p_inum,0x100|(2<<4),"Number of Y-strips from ING-27");
  Add_prof_num(hfr1,&md->hd->bins,64,p_fnum,0x100|(2<<4),"Number of X-bins in profilometer histograms");
  Add_prof_num(hfr1,&md->hd->bins2,64,p_fnum,0x100|(2<<4),"Number of Y-bins in profilometer histograms");
  Add_prof_num(hfr1,&md->hd->rb,64,p_inum,4<<1|3<<4,"Rebin X (only for drawing)");
  Add_prof_num(hfr1,&md->hd->rb2,64,p_inum,4<<1|3<<4,"Rebin Y (only for drawing)");

	
  TGTextEntry *fLabel=new TGTextEntry(hfr1, md->name.Data());
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(md->tip);
  fLabel->SetAlignment(kTextCenterY);

  //TGLabel* fLabel = new TGLabel(hfr1, label);
  //fLabel->SetToolTipText(tip);
  hfr1->AddFrame(fLabel,LayLT4);

}

void HistParDlg::AddLine_prof_int(TGHorizontalFrame *hfr1, Mdef* md) {
  int id;

  //1d checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_1d = new TGCheckButton(hfr1, md->name.Data(), id);
  chk_1d->SetToolTipText(md->tip);
  DoMap(chk_1d,&opt.h_prof_int.b,p_chk,0,0x100);
  chk_1d->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_1d,LayCC1);
  //id0=id;

}

#ifdef YUMO
void HistParDlg::Add_yumo(TGGroupFrame* frame, Mdef* md1, Mdef* md2, Mdef* md3) {

  TGHorizontalFrame *hfr1;
  TGTextEntry *fLabel;

  // 1d hist
  hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1,LayLT1b);
  Check_opt(hfr1,0,&md1->hd->b,"on/off",0x100|(2<<4),"");
  Num_opt(hfr1,40,&md1->hd->bins,0,"Number of bins per channel",
	  k_r0,0.01,10,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md1->hd->min,0,"Low edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md1->hd->max,0,"Upper edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,40,&md1->hd->rb,0,"Rebin (only for drawing)",
	  k_int,1,1000,4<<1|3<<4,LayLT2);
  Check_opt(hfr1,0,&md1->hd->htp,"Double/Float",0x100|(2<<1)|(2<<4),"");

  fLabel=new TGTextEntry(hfr1, "YUMO 1d");
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText("1d YUMO histograms");
  hfr1->AddFrame(fLabel,LayLT2);


  // 2d hist
  hfr1 = new TGHorizontalFrame(frame);
  //hfr1->ChangeOptions(hfr1->GetOptions()|kFixedWidth);
  //hfr1->Resize(HFRAME_WIDTH,0);
  frame->AddFrame(hfr1,LayLT1b);

  Check_opt(hfr1,0,&md2->hd->b,"on/off",0x100|(2<<4),"");
  Num_opt(hfr1,40,&md2->hd->bins,0,"Number of bins per channel on X and Y axis",
	  k_r0,0.01,10,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md2->hd->min,0,"Low edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md2->hd->max,0,"Upper edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,40,&md2->hd->rb,&md2->hd->rb2,"Rebin X/Y (only for drawing)",
	  k_int,1,1000,4<<1|3<<4,LayLT2);
  Check_opt(hfr1,0,&md2->hd->htp,"Double/Float",0x100|(2<<1)|(2<<4),"");

  fLabel=new TGTextEntry(hfr1, "YUMO 2d");
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText("2d YUMO histograms");
  //fLabel->SetAlignment(kTextCenterY);
  hfr1->AddFrame(fLabel,LayLT2);


  // 3d hist
  hfr1 = new TGHorizontalFrame(frame);
  //hfr1->ChangeOptions(hfr1->GetOptions()|kFixedWidth);
  //hfr1->Resize(HFRAME_WIDTH,0);
  frame->AddFrame(hfr1,LayLT1b);

  Check_opt(hfr1,0,&md3->hd->b,"on/off",0x100|(2<<4),"");
  Num_opt(hfr1,40,&md3->hd->bins,0,"Number of bins per channel on X and Y axis",
	  k_r0,0.01,10,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md3->hd->min,0,"Low edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,50,&md3->hd->max,0,"Upper edge",
	  k_r0,0,0,0x100|(2<<4),LayLT2);
  Num_opt(hfr1,40,&md3->hd->rb,&md3->hd->rb2,"Rebin X/Y (only for drawing)",
	  k_int,1,1000,4<<1|3<<4,LayLT2);
  Check_opt(hfr1,0,&md3->hd->htp,"Double/Float",0x100|(2<<1)|(2<<4),"");

  fLabel=new TGTextEntry(hfr1, "YUMO 3d");
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText("3d YUMO histograms");
  //fLabel->SetAlignment(kTextCenterY);
  hfr1->AddFrame(fLabel,LayLT2);


  // x1x2y1y2
  hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1,LayLT1b);
  Num_opt(hfr1,40,&opt.yumo_x1,0,"X1 channel",k_int,0,31,0x100,LayLT0);
  Num_opt(hfr1,40,&opt.yumo_x2,0,"X2 channel",k_int,0,31,0x100,LayLT0);
  Num_opt(hfr1,40,&opt.yumo_y1,0,"Y1 channel",k_int,0,31,0x100,LayLT0);
  Num_opt(hfr1,40,&opt.yumo_y2,0,"Y2 channel",k_int,0,31,0x100,LayLT0);

  fLabel=new TGTextEntry(hfr1, "X1 X2 Y1 Y2 channels");
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText("cathode channels");
  hfr1->AddFrame(fLabel,LayLT2);


  // peak window
  hfr1 = new TGHorizontalFrame(frame);
  frame->AddFrame(hfr1,LayLT1b);
  Num_opt(hfr1,50,&opt.yumo_peak1,0,"left border of the peak",
	  k_r0,0,0,0x100,LayLT0);
  Num_opt(hfr1,50,&opt.yumo_peak2,0,"right border of the peak",
	  k_r0,0,0,0x100,LayLT0);

  fLabel=new TGTextEntry(hfr1, "Peak Window");
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText("Peak window in x1+x2 & y1+y2 spectra");
  hfr1->AddFrame(fLabel,LayLT2);

} //Add_yumo
#endif

void HistParDlg::AddLine_2d(TGGroupFrame* frame, Mdef* md) {
  int id1 = md->hnum/100;
  int id2 = md->hnum%100;

  int col=0;
  double ww1=50,ww2=40;
  double min2=0.0001,
    max2=10000;
  char *tip11, *tip22;

  if (id1==id2) { //AXAY
    col=2<<1; //зеленый
    min2=1;
    max2=opt.Nchan; //MAX_AXAY-1;
    tip11= (char*) "Number of bins per channel on X and Y axis";
    tip22= (char*) "Maximal channel number";
  }
  else { //normal 2d
    tip11= (char*) "Number of bins per channel on X-axis";
    tip22= (char*) "Number of bins per channel on Y-axis";
  }

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(frame);
  hfr1->ChangeOptions(hfr1->GetOptions()|kFixedWidth);
  hfr1->Resize(HFRAME_WIDTH,0);
  frame->AddFrame(hfr1);

  hfr1->SetCleanup(kDeepCleanup);
  
  int id;
  //int id0;

  //TGNumberFormat::ELimit nolim = TGNumberFormat::kNELNoLimits;
  TGNumberFormat::ELimit lim = TGNumberFormat::kNELLimitMinMax;

  //checkbutton
  id = Plist.size()+1;
  TGCheckButton *chk_hist = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist,&md->hd->b,p_chk,0,0x100|(2<<4));
  chk_hist->SetToolTipText("on/off");
  chk_hist->Connect("Toggled(Bool_t)", "ParDlg", this, "DoCheckHist(Bool_t)");
  hfr1->AddFrame(chk_hist,LayLT2);
  //id0=id;

  //nbins (x-axis)
  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0, 
					   TGNumberFormat::kNEAAnyNumber,
					   lim,0.0001,10000);
  DoMap(fNum1->GetNumberEntry(),&md->hd->bins,p_fnum,0,0x100|(2<<4));
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
  DoMap(fNum1->GetNumberEntry(),&md->hd->bins2,p_fnum,0,0x100|(2<<4)|col);
  fNum1->GetNumberEntry()->SetToolTipText(tip22);
  fNum1->SetWidth(ww1);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  //rebin (x-axis)
  id = Plist.size()+1;
  fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0,
			    TGNumberFormat::kNEAAnyNumber,
			    lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),&md->hd->rb,p_inum,0,4<<1|3<<4);//cyan + Update
  fNum1->GetNumberEntry()->SetToolTipText("Rebin X (only for drawing)");
  fNum1->SetWidth(ww2);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  //rebin (y-axis)
  id = Plist.size()+1;
  fNum1 = new TGNumberEntry(hfr1, 0, 0, id, k_r0,
			    TGNumberFormat::kNEAAnyNumber,
			    lim,0,1000);
  DoMap(fNum1->GetNumberEntry(),&md->hd->rb2,p_inum,0,4<<1|3<<4);//cyan + Update
  fNum1->GetNumberEntry()->SetToolTipText("Rebin Y (only for drawing)");
  fNum1->SetWidth(ww2);
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this,
				   "DoDaqNum()");
  hfr1->AddFrame(fNum1,LayLT2);

  // - just to take space
  // TGLabel* tt = new TGLabel(hfr1,"");
  // tt->ChangeOptions(tt->GetOptions()|kFixedWidth);
  // tt->SetWidth(ww);
  // hfr1->AddFrame(tt,LayLT2);

  //checkbutton Double/Float
  id = Plist.size()+1;
  TGCheckButton *chk_hist2 = new TGCheckButton(hfr1, "", id);
  DoMap(chk_hist2,&md->hd->htp,p_chk,0,0x100|(2<<1)|(2<<4));
  chk_hist2->SetToolTipText("Double/Float");
  chk_hist2->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  hfr1->AddFrame(chk_hist2,LayLT2);

  TGTextEntry *fLabel=new TGTextEntry(hfr1, md->name.Data());
  //fLabel->SetWidth();
  fLabel->SetState(false);
  fLabel->ChangeOptions(0);
  fLabel->SetToolTipText(md->tip);
  fLabel->SetAlignment(kTextCenterY);

  hfr1->AddFrame(fLabel,LayLT2);

  // cout << "d2: " << hfr1 << " " << hfr1->GetName() << endl;
  // hfr1->SetName(TString(md->hnum).Data());
  // hfr1->SetName("sdfsdf");
  // cout << "d2: " << hfr1->GetName() << endl;

  // TGFrameElement* d3 = (TGFrameElement*) frame2d->GetList()->First();
  // while (d3) {
  //   cout << "d33: " << d3 << " " << d3->GetName() << endl;
  //   d3 = (TGFrameElement*)frame2d->GetList()->After(d3);
  // }

  
  TGTextButton *fbut = new TGTextButton(hfr1,"Remove",md->hnum);
  fbut->SetToolTipText("Remove this type of 2d histograms");
  hfr1->AddFrame(fbut, LayRT0);
  fbut->Connect("Clicked()", "HistParDlg", this, "Rem2d()");

  // TGHorizontalFrame* parent = (TGHorizontalFrame*) fbut->GetParent();
  // cout << "Rem2d: " << id << " " << parent << endl;

  list2d.Add(hfr1);
  //ParDlg::Update();
  Update();
}

void HistParDlg::Add2d() {
  //cout << "Add2d: " << cmb1->GetSelected() << " " << cmb2->GetSelected() << endl;

  int id1 = cmb1->GetSelected();
  int id2 = cmb2->GetSelected();

  if (hcl->Find_Mdef(100*id1+id2)!=hcl->Mlist.end()) { //already exists
    return;
  }

  //cout << "Add2d_1: " << id1 << " " << id2 << endl;

  Mdef* md = hcl->Add_h2(id1,id2);

  if (md) {
    AddHist_2d();
  }





  /*
  //cout << "Add2d_2: " << id1 << " " << id2 << " " << md << endl;

  if (md) {
    AddLine_2d(frame2d,md);
  }
  //cout << "Add2d_3: " << id1 << " " << id2 << " " << md << endl;
  MapSubwindows();
  Layout();

  //cout << "Add2d_4: " << id1 << " " << id2 << " " << md << endl;

  // cout << endl;
  // for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
  //   if (it->hnum>=100) {
  //     cout << "2d: " << it->hnum << " " << it->name << endl;
  //   }
  // }
  */

}

void HistParDlg::Rem2d() {
  TGButton *btn = (TGButton *) gTQSender;
  int id = btn->WidgetId();

  mdef_iter it = hcl->Find_Mdef(id);
  if (it!=hcl->Mlist.end()) {
    //delete it->hd;
    hcl->Mlist.erase(it);
  }

  AddHist_2d();
  //return;


  
  /*
  myM->MapSubwindows();
  //myM->Resize(GetDefaultSize());
  myM->Layout();
  myM->MapWindow();



  MapSubwindows();
  //myM->Resize(GetDefaultSize());
  Layout();
  MapWindow();

  */




  HiFrm->HiReset();
}

void HistParDlg::Update() {
  //cout << "HistParDlg::Update()" << endl;
  ParDlg::Update();
  //AddHist_2d();

  if (myM) {
    myM->MapSubwindows();
    myM->Layout();
    myM->MapWindow();
  }

  // MapSubwindows();
  // Layout();
  // MapWindow();

}

//------ ChanParDlg -------

ChanParDlg::ChanParDlg(const TGWindow *p,UInt_t wdth,UInt_t h)
  :ParDlg(p,wdth,h)
{

  nrows = TMath::Min(opt.Nrows,opt.Nchan);
  tTrig=0;

  AddFrame(fDock, LayEE0);
  //fMain - контейнер от fDock
  //fCnv[3] - три TGCanvas внутри fMain. fCnv[1] с горизонтальным скроллом
  //vfr[3] - контейнеры от fCnv

  //vfr[2] содержит еще tTrig 


  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGHorizontalLayout(fMain));

  TGCompositeFrame *vfr[3];

  for (int i=0;i<3;i++) {
    // начальные размеры 0,0. Окончательно устанавливаются в конце Build
    //fCnv[i] = new TGCanvas(fcont,0,0);
    fCnv[i] = new TGCanvas(fMain,0,0);
    vfr[i] = new TGCompositeFrame(fCnv[i]->GetViewPort(), 0, 0, kVerticalFrame);
    fCnv[i]->SetContainer(vfr[i]);
  }
  fMain->AddFrame(fCnv[0],LayLE0);
  fMain->AddFrame(fCnv[1],LayEE0);
  fMain->AddFrame(fCnv[2],LayLE0);

  fCnv[0]->SetScrolling(TGCanvas::kCanvasNoScroll);
  fCnv[1]->SetScrolling(TGCanvas::kCanvasScrollHorizontal);
  fCnv[2]->SetScrolling(TGCanvas::kCanvasNoScroll);

  hscroll=fCnv[1]->GetHScrollbar();
  //hscroll->SetRange(20+1,1);

  vscroll = new TGVScrollBar(fMain);
  fMain->AddFrame(vscroll,LayLE0);

  int range = opt.Nchan-nrows;
  int page = round(range/6.0)+1;
  //cout << "range: " << range << " " << page << endl;

  vscroll->SetRange(range+page,page);
  oldscroll = TMath::Max(opt.ScrollPos,0);
  vscroll->SetPosition(oldscroll);

  for (int i=0;i<3;i++) {
    head_frame[i] = new TGHorizontalFrame(vfr[i]);
    vfr[i]->AddFrame(head_frame[i]);

    for (int j=0;j<nrows;j++) {
      hparl[i][j] = new TGHorizontalFrame(vfr[i]);
      vfr[i]->AddFrame(hparl[i][j]);
    }

    hsep[i] = new TGHorizontal3DLine(vfr[i]);
    vfr[i]->AddFrame(hsep[i], LayLT1b);

    if (crs->module==22) {
      hforce[i] = new TGHorizontalFrame(vfr[i]);
      vfr[i]->AddFrame(hforce[i]);
      hforce[i]->SetHeight(PHeight);
      hforce[i]->ChangeOptions(hforce[i]->GetOptions()|kFixedHeight);
    }

    for (int j=MAX_CH;j<MAX_CHTP;j++) {
      hparl[i][j] = new TGHorizontalFrame(vfr[i]);
      vfr[i]->AddFrame(hparl[i][j]);
    }
  }



  // YKWheel
  //vscroll->Connect("PositionChanged(Int_t)", "ChanParDlg", this, "DoScroll()");
  fMain->Connect("ProcessedEvent(Event_t*)", "ChanParDlg", this,
		 "HandleMouseWheel(Event_t*)");
  gVirtualX->GrabButton(fMain->GetId(), kButton4, kAnyModifier,
			kButtonPressMask,
			kNone, kNone);
  gVirtualX->GrabButton(fMain->GetId(), kButton5, kAnyModifier,
			kButtonPressMask,
			kNone, kNone);
  //YKWheel

  fDock->SetWindowName("Channels");  
}

void ChanParDlg::Build() {

  //MakeVarList(1,1);

  //int wsize[NFLD+3];
  //memset(wsize,0,sizeof(wsize));

  for (int jj=-1;jj<MAX_CHTP;jj++) {
    BuildColumns(jj);
  }

  if (NFLD!=nfld) {
    prnt("ss d ds;",BRED,"NFLD constant is not set properly:",NFLD,nfld,RST);
    exit(-1);
  }

  // for (int i=0;i<nfld;i++) {
  //   cout << "Plist: " << i << " " << Plist[nfld+i].off << " " << (int) Plist[nfld+i].step << endl;
  // }

  for (int i=0;i<3;i++) {
    int dw = hparl[i][0]->GetDefaultWidth();
    hsep[i]->SetWidth(dw);
  }

  //pmax=nrows;

  //устанавливаем размеры fCnv[]
  fCnv[0]->SetWidth(head_frame[0]->GetDefaultWidth()+2);
  fCnv[2]->SetWidth(head_frame[2]->GetDefaultWidth()+4);



  // --- Coincidence scheme
  const char *tip1="", *tip2="", *label;
  int ww=40;

  TGCompositeFrame *vfr = (TGCompositeFrame*) fCnv[2]->GetViewPort()->GetContainer();

  cGrp = new TGGroupFrame(vfr, "Coincidence scheme", kVerticalFrame);
  vfr->AddFrame(cGrp, LayTrig);

  cGrp->SetTitlePos(TGGroupFrame::kCenter);

  AddLine_opt(cGrp,ww,(void*)"C1",(void*)"C2",tip1,tip2,"",
	      k_lab,k_lab,0,0,0,0,0,0,LayLC1,LayLC2);

  tip1= "Width of coincidence window for Group 1 (in samples)";
  tip2= "Width of coincidence window for Group 2 (in samples)";
  label="Windows";
  AddLine_opt(cGrp,-ww,cpar.coinc_w,cpar.coinc_w+1,tip1,tip2,label,k_int,k_int,
       1,1023,1,1023,1,1,LayLC1,LayLC2);

  tip1= "Minimal multiplicity for Group 1";
  tip2= "Minimal multiplicity for Group 2";
  label="Min mult ";
  AddLine_opt(cGrp,-ww,cpar.mult_w1,cpar.mult_w1+1,tip1,tip2,label,k_int,k_int,
	      0,255,0,255,1,1,LayLC1,LayLC2);

  tip1= "Maximal multiplicity for Group 1";
  tip2= "Maximal multiplicity for Group 2";
  label="Max mult";
  AddLine_opt(cGrp,-ww,cpar.mult_w2,cpar.mult_w2+1,tip1,tip2,label,k_int,k_int,
	      0,255,0,255,1,1,LayLC1,LayLC2);

  TGLabel* tLab = new TGLabel(cGrp,"Trigger: ");
  tLab->ChangeOptions(tLab->GetOptions()|kFixedWidth);
  tLab->SetWidth(130);
  cGrp->AddFrame(tLab,LayLT1a);

  tTrig = new TrigFrame(cGrp,0);
  // --- Coincidence scheme



  //В конце добавляем fforce для ЦРС2
  if (crs->module==22) {
    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce[0], "", id);
    hforce[0]->AddFrame(fforce,LayLT2);
    DoMap((TGFrame*)fforce,&cpar.forcewr,p_chk,0,1);
    fforce->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
    fforce->SetToolTipText("Record both channels simultaneously");
    TGLabel* lforce = new TGLabel(hforce[0], "Force_write");
    hforce[0]->AddFrame(lforce,LayLT2);
  }

  DoScroll(opt.ScrollPos);
}

void ChanParDlg::BuildColumns(int jj) {

  char txt[64];
  //int cmd;

  int kk=0;
  AddColumn(jj,kk++,0,p_chn,26,0,0,0,"Ch",0,0,0);
  AddColumn(jj,kk++,0,p_chk,24,1,0,0,"on",cpar.on,0,1);
  AddColumn(jj,kk++,0,p_chk,24,0,0,0,"*",opt.star,0,0);
  AddColumn(jj,kk++,0,p_cmb,70,0,0,0,"Type",0,0,1);

  
  AddColumn(jj,kk++,1,p_chk,25,1,0,0,"Inv",cpar.Inv,0,1);
  AddColumn(jj,kk++,1,p_chk,24,1,0,0,"AC",cpar.AC,0,1|(5<<4));
  AddColumn(jj,kk++,1,p_chk,24,1,0,0,"pls",cpar.pls,0,1);
  AddColumn(jj,kk++,1,p_chk,24,0,0,0,"dsp",opt.dsp,opt.Dsp,1|(8<<4));
  AddColumn(jj,kk++,1,p_inum,34,1,0,0,"RD",cpar.RD,0,1);
  AddColumn(jj,kk++,1,p_chk,24,1,0,0,"C1",cpar.group,0,1,21);
  AddColumn(jj,kk++,1,p_chk,24,1,0,0,"C2",cpar.group,0,1,22);
  AddColumn(jj,kk++,1,p_inum,30,1,0,0,"hS",cpar.hS,0,1);
  AddColumn(jj,kk++,1,p_inum,36,1,0,0,"hD",cpar.hD,0,1);
  AddColumn(jj,kk++,1,p_inum,40,1,0,0,"Dt",cpar.Dt,0,1);
  AddColumn(jj,kk++,1,p_inum,36,1,0,0,"Pre",cpar.Pre,0,1);
  AddColumn(jj,kk++,1,p_inum,40,1,0,0,"Len",cpar.Len,0,1|(6<<4));
  AddColumn(jj,kk++,1,p_inum,21,1,0,0,"G",cpar.G,0,1|(5<<4));
  AddColumn(jj,kk++,1,p_inum,21,1,0,0,"Trg",cpar.Trg,0,1);
  AddColumn(jj,kk++,1,p_inum,36,1,0,0,"Drv",cpar.Drv,opt.sDrv,1|(8<<4));
  AddColumn(jj,kk++,1,p_inum,40,1,0,0,"Thr",cpar.Thr,opt.sThr,1|(8<<4));
  AddColumn(jj,kk++,1,p_inum,40,1,0,0,"LT",cpar.LT,0,1);

  //Analysis
  AddColumn(jj,kk++,1,p_chk,24,0,0,0,"St",opt.St);
  AddColumn(jj,kk++,1,p_chk,24,0,0,0,"Ms",opt.Ms);
  AddColumn(jj,kk++,1,p_chk,24,0,0,0,"Dsp",opt.Dsp,0,0);
  AddColumn(jj,kk++,1,p_chk,24,0,0,0,"Pls",opt.Pls,0,0);
  AddColumn(jj,kk++,1,p_fnum2,45,0,-9999,9999,"sD",opt.sD);
  //AddColumn(jj,kk++,1,p_inum,35,0,0,9999,"dTm",opt.dTm);
  //AddColumn(jj,kk++,1,p_inum,35,0,0,9999,"Pile",opt.Pile);
  AddColumn(jj,kk++,1,p_inum,20,0,0,2,"C",opt.calibr_t);
  AddColumn(jj,kk++,1,p_fnum,50,0,-1e99,1e99,"E0",opt.E0);
  AddColumn(jj,kk++,1,p_fnum,60,0,-1e99,1e99,"E1",opt.E1);
  AddColumn(jj,kk++,1,p_fnum,50,0,-1e99,1e99,"E2",opt.E2);
  AddColumn(jj,kk++,1,p_inum,40,0,-99999,99999,"Pz",opt.Pz);
  //AddColumn(jj,kk++,1,p_fnum,40,0,-1e99,1e99,"Bc",opt.Bc);
  for (int i=1;i<=4;i++) {
    sprintf(txt,"g%d",i);
    AddColumn(jj,kk++,1,p_chk,24,0,0,0,txt,opt.Grp,0,0,40+i);
  }


  double amax=-2e101;
  //Peaks
  AddColumn(jj,kk++,1,p_inum,25,0,-999,999,"sS",opt.sS);
  AddColumn(jj,kk++,1,p_inum,26,0,-1,7,"sTg",opt.sTg);
  AddColumn(jj,kk++,1,p_inum,32,0,1,1023,"sDrv",opt.sDrv);
  AddColumn(jj,kk++,1,p_inum,40,0,0,65565,"sThr",opt.sThr);
  AddColumn(jj,kk++,1,p_inum,20,0,0,3,"Mt",opt.Mt);
  AddColumn(jj,kk++,1,p_inum,30,0,1,999,"DD",opt.DD);
  AddColumn(jj,kk++,1,p_inum,30,0,1,9,"FF",opt.FF);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,amax,"B1",opt.B1);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,9999,"B2",opt.B2);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,amax,"P1",opt.P1);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,9999,"P2",opt.P2);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,amax,"T1",opt.T1);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,9999,"T2",opt.T2);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,amax,"W1",opt.W1);
  AddColumn(jj,kk++,1,p_inum,40,0,-1024,9999,"W2",opt.W2);

  AddColumn(jj,kk++,2,p_stat,60,0,0,0,"P/sec (sw)",crs->rate_soft);
  AddColumn(jj,kk++,2,p_stat,60,0,0,0,"P/sec (hw)",crs->rate_hard);
  AddColumn(jj,kk++,2,p_stat,60,0,0,0,"BadPls",crs->npulses_bad);

  nfld=kk;
  //cout << "kk: " << jj << " " << kk << " " << nfld << endl;
}

void ChanParDlg::
AddColumn(int jj, int kk, int ii, P_Def pdef,
	  int wd, int daq, double min, double max, const char* pname,
	  void* apar, void* apar2, UInt_t cmd, int s2) {

  //jj - nr of line; kk - nr of column; ii - nr of frame
  //s2 = step*10 + first+1; - only for p_chk

  if (jj==-1) {

    //header
    TGTextEntry* tt=new TGTextEntry(head_frame[ii], pname);
    tt->SetWidth(wd);
    tt->SetState(false);
    tt->SetToolTipText(ptip.at(kk));
    tt->SetAlignment(kTextCenterX);
    head_frame[ii]->AddFrame(tt,LayCC0);
    return;
  }

  //main loop

  int all=0;
  if (jj>=nrows && jj<MAX_CH)
    return;
  if (jj>=MAX_CH) {
    all=jj-MAX_CH+1;
    //cmd = cmd & 1; //reset higher bits
  }

  UShort_t off = jj;
  UChar_t step = 1;
  int first=0;

  if (s2) {
    first = s2%10-1;
    step = s2/10;
    off = jj*step;
  }
  char* ap = (char*)apar+first*sizeof(Bool_t);

  double amax = max;
  if (max<-1e101) { //B1, Peak1, T1, W1
    //cout << "max!!!: " << jj << " " << kk << " " << pname << endl;
    amax = 1023;
    if (crs->crs_ch[jj]==1 || crs->crs_ch[jj]==2)
      amax=511;
  }

  switch (pdef) {
  case p_chn:
    AddChan(jj,kk,wd,all,hparl[ii][jj]);
    break;
  case p_cmb:
    AddCombo(jj,wd,all,hparl[ii][jj]);
    break;
  case p_chk:
    AddChkPar(kk,wd,all,daq,hparl[ii][jj],ap,off,cmd,apar2,step);
    break;
  case p_inum:
  case p_fnum:
  case p_fnum2:
    AddNumPar(jj,kk,wd,all,daq,pdef,min,amax,hparl[ii][jj],pname,
	      apar,off,cmd,apar2);
    break;
  case p_stat:
    {
      //TGTextEntry** fS = (TGTextEntry**) apar2;

      if (jj<=MAX_CH) {
	AddStatDaq(jj,kk,wd,apar,hparl[ii][jj]);
	//AddStatDaq(kk,wd,*(fS+jj),hparl[ii][jj]);
      }
      else {
	DoMap(0,0,p_stat,0,0,0,0);
      }
    }
    break;
  default:
    ;
  }

}

void ChanParDlg::AddChan(int j, int kk, int wd, int all,
			 TGHorizontalFrame *hfr, UShort_t off) {

  // char txt[255];
  // if (j<MAX_CH)
  //   sprintf(txt,"%2d",j);
  // else {
  //   sprintf(txt," ");
  // }

  P_Def pdef = p_chn;
  int id = Plist.size()+1;
  if (j!=MAX_CH) { //clab: channel Nr or text
    if (j<MAX_CH) { //Chan
      clab[j]=new TGNumberEntryField(hfr, id, j, TGNumberFormat::kNESInteger,
				   TGNumberFormat::kNEAAnyNumber);
      clab[j]->SetToolTipText(ptip.at(kk));
    }
    else { //text
      clab[j]=new TGTextEntry(hfr, " ", id);
      //clab[j]->SetBackgroundColor(tcol[all-2]);
      pdef=p_txt;
    }

    DoMap(clab[j],0,pdef,all,0,0,j);
    clab[j]->SetHeight(PHeight);
    clab[j]->SetWidth(wd);
    clab[j]->SetState(false);
    hfr->AddFrame(clab[j],LayCC0a);
    if (all)
      clab[j]->SetBackgroundColor(tcol[all-2]);
  }
  else { //cbut
    cbut_id = id;
    cbut = new TGTextButton(hfr, "ZZ", cbut_id);
    cbut->ChangeOptions(cbut->GetOptions()|kFixedWidth);
    cbut->SetHeight(PHeight);
    cbut->SetWidth(wd);
    //cbut->SetTextJustify(kTextBottom);
    cbut->SetToolTipText("* - change selected channels;\nall - change all channels;\nALL - change all channels and groups.");
    cbut->SetDown(false);
    cbut->Connect("Clicked()", "ParDlg", this, "DoAll()");
    DoMap(cbut,&opt.chkall,p_but,1,0);
    hfr->AddFrame(cbut,LayCC0a);
  }

}

void ChanParDlg::AddCombo(int j, int wd, int all, TGHorizontalFrame *hfr) {
  if (j<=MAX_CH) { //combo
    int id = Plist.size()+1;

    fCombo[j]=new TGComboBox(hfr,id);
    //fCombo[j]->SetToolTipText(ttip_type);
    hfr->AddFrame(fCombo[j],LayCC0);
    fCombo[j]->Resize(wd, PHeight);

    for (int m = 0; m < MAX_TP; m++) {
      fCombo[j]->AddEntry(opt.ch_name[m], m+1);
    }
    fCombo[j]->AddEntry("Other", MAX_TP+1);

    if (j!=MAX_CH) {
      fCombo[j]->AddEntry("Copy", MAX_TP+2);
      fCombo[j]->AddEntry("Swap", MAX_TP+3);
    }
    DoMap(fCombo[j],opt.chtype,p_cmb,all,0,0,j);
    fCombo[j]->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");
  } //if (i<=MAX_CH)
  else { //j>MAX_CH -> Chname
    int id=Plist.size()+1;
    TGTextEntry* tgtxt=new TGTextEntry(hfr," ",id);
    tgtxt->SetWidth(wd);
    tgtxt->SetMaxLength(sizeof(opt.ch_name[0])-1);
    hfr->AddFrame(tgtxt,LayCC0);//,LayCC0a);

    tgtxt->SetName(TString::Format("%02dtxt",all-1).Data());
    tgtxt->SetText(opt.ch_name[all-2],false);
    tgtxt->Connect("TextChanged(char*)", "ParDlg", this, "DoTypes()");
    //tgtxt->SetBackgroundColor(tcol[all-2]);
    //hfr->SetBackgroundColor(tcol[all-2]);
    for (int i=0;i<3;i++) {
      hparl[i][j]->SetBackgroundColor(tcol[all-2]);
    }
    DoMap(tgtxt,opt.ch_name[all-2],p_txt,0);
  }
}

void ChanParDlg::AddChkPar(int kk, int wd, int all, int daq, TGHorizontalFrame *hfr, void* apar, UShort_t off, UInt_t cmd, void* apar2, UChar_t step) {

  int id = Plist.size()+1;
  TGCheckButton *f_chk = new TGCheckButton(hfr, "", id);
  DoMap(f_chk,apar,p_chk,all,cmd,apar2,off,step);
  f_chk->SetToolTipText(ptip.at(kk));
  //f_chk->ChangeOptions(f_chk->GetOptions()|kFixedWidth);
  //f_chk->SetWidth(wd-10);
  // f_chk->SetHeight(PHeight);
  f_chk->Connect("Toggled(Bool_t)", "ParDlg", this, "DoDaqChk(Bool_t)");
  hfr->AddFrame(f_chk,LayCC1);
}

void ChanParDlg::AddNumPar(int j, int kk, int wd, int all, int daq, P_Def pdef, double min, double max, TGHorizontalFrame *hfr, const char* name, void* apar, UShort_t off, UInt_t cmd, void* apar2) {

  int /*par, */imin, imax;

  if (daq) {
    //cpar.GetPar(name,crs->module,j,cpar.crs_ch[j],par,imin,imax);
    cpar.GetParm(name,j,apar,imin,imax);
    min=imin;
    max=imax;
  }

  TGNumberFormat::ELimit limits = TGNumberFormat::kNELLimitMinMax;
  // if (max==-1) {
  //   limits = TGNumberFormat::kNELNoLimits;
  // }

  int id = Plist.size()+1;

  //ETextJustification al=kTextRight;;
  TGNumberFormat::EStyle style=TGNumberFormat::kNESInteger;
  switch (pdef) {
  case p_fnum:
    style=TGNumberFormat::kNESReal;
    //al = kTextLeft;
    break;
  case p_fnum2:
    style=TGNumberFormat::kNESRealTwo;
    //al = kTextLeft;
    break;
  default:
    ;
  }

  TGNumberEntryField* fNum =
    new TGNumberEntryField(hfr, id, 0, style,
			   TGNumberFormat::kNEAAnyNumber,
			   limits,min,max);

  char ss[100];
  sprintf(ss,"%s%d",name,id);
  fNum->SetName(ss);
  DoMap(fNum,apar,pdef,all,cmd,apar2,off);
  fNum->SetToolTipText(ptip.at(kk));
  fNum->SetWidth(wd);
  fNum->SetHeight(PHeight);
  //fNum->SetAlignment(al);
  fNum->SetAlignment(kTextRight);
  //fNum->SetAlignment(kTextLeft);
  fNum->Connect("TextChanged(char*)", "ParDlg", this, "DoDaqNum()");
  hfr->AddFrame(fNum,LayCC0a);

}

void ChanParDlg::AddStatDaq(int jj,int kk,int wd,
			    void* apar,TGHorizontalFrame* hfr) {
  //int col;

  int id = Plist.size()+1;
  TGNumberEntryField* fNum =
    new TGNumberEntryField(hfr, id, 0, TGNumberFormat::kNESRealOne,
			   TGNumberFormat::kNEAAnyNumber,
			   TGNumberFormat::kNELNoLimits);

  fNum->SetAlignment(kTextLeft);
  fNum->Resize(wd,PHeight);
  fNum->SetToolTipText(ptip.at(kk));
  fNum->SetState(false);

  DoMap(fNum,apar,p_stat,0,0,0,jj);
  hfr->AddFrame(fNum,LayCC0a);




  /*
  fStat = new TGTextEntry(hfr, "");
  fStat->ChangeOptions(fStat->GetOptions()|kFixedSize|kSunkenFrame);

  fStat->SetState(false);
  fStat->SetToolTipText(ptip.at(kk));

  fStat->Resize(wd,PHeight);
  col=gROOT->GetColor(19)->GetPixel();
  fStat->SetBackgroundColor(col);
  fStat->SetText(0);


  // char txt[100];
  // static int j;
  // j++;
  // sprintf(txt,"t %d",j);
  // fStat->SetText(txt);


  hfr->AddFrame(fStat,LayCC0a);
  */
}

void ChanParDlg::Update() {
  ParDlg::Update();
  if (tTrig)
    tTrig->UpdateTrigger();

  //prnt("ss d ds;",BGRN,"upd:",cpar.hS[0],cpar.hS[MAX_CH+1],RST);

  //MapSubwindows();
  //Layout();
}

void ChanParDlg::DoScroll(int pos) {  
  //Update();
  //cout << "wheel1: " << vscroll->GetPosition() << " " << pos << endl;

  for (int i=0;i<nrows;i++) {
    int row = i+pos;
    if (row<0) row=0;
    if (row>=opt.Nchan) row=opt.Nchan-1;
    for (int j=0;j<nfld;j++) {
      int rr = i*nfld+j;
      Pmap* pp = &Plist[rr];
      // if (j==3 || j==9 || j==10) {
      // 	TGFrame* wg = (TGFrame*) pp->field;
      // 	cout << "scr: "<<i << " " << j << " " << rr << " " << nfld << " "
      // 	     <<wg->GetName()<<" "<<pp->off<<" "<<(int)pp->step<<endl;
      // }


      pp->off=row*pp->step;
      UpdateField(rr);
    }
  }

  // for (UInt_t i=0;i<Plist.size();i++) {
  //   Pmap* pp = &Plist[i];
  //   TGFrame* wg = (TGFrame*) pp->field;
  //   cout << "scr: " << i << " " << nfld << " " << wg->GetName() << endl;
  // }


  
}

void ChanParDlg::HandleMouseWheel(Event_t *event) {
   // Handle mouse wheel to scroll.

  //prnt("ss d d d d d d ds;",BGRN,"Scroll:",event->fType,event->fCode,event->fState,hscroll->GetPosition(),hscroll->GetRange(),hscroll->GetPageSize(), hscroll->GetRange()-hscroll->GetPageSize(),RST);

  if (event->fState==1) { //shift
    int pos = hscroll->GetPosition();
    int sz = hscroll->GetPageSize()/20;
    if (event->fCode == 4) //up
      pos+=sz;
    else if (event->fCode == 5) //down
      pos-=sz;
    pos = TMath::Max(pos, 0);
    hscroll->SetPosition(pos);
    return;
  }

  int kk=0;
  if (event->fCode == 4) //up
    kk=-1;
  else if (event->fCode == 5) //down
    kk=1;

  if (kk) {
    int pos = vscroll->GetPosition()+kk;
    pos = TMath::Max(pos, 0);
    vscroll->SetPosition(pos);
  }

  opt.ScrollPos = vscroll->GetPosition();
  if (opt.ScrollPos != oldscroll) {
    //int scrl = opt.ScrollPos - oldscroll;

    // if (opt.ScrollPos<0 || nrows+opt.ScrollPos>=opt.Nchan) {
    //   prnt("ss d d d d ds;",BRED,"Scroll:",event->fType,opt.ScrollPos,scrl,vscroll->GetRange(),vscroll->GetPageSize(),RST);
    //   scrl=0;
    // }
    // else {
    //   prnt("ss d d d d ds;",BGRN,"Scroll:",event->fType,opt.ScrollPos,scrl,vscroll->GetRange(),vscroll->GetPageSize(),RST);
    // }

    DoScroll(opt.ScrollPos);
    //Update();
    oldscroll=opt.ScrollPos;
  }
}

void ChanParDlg::UpdateStatus(int rst) {

  //static Long64_t allbad;
  static double t1;
  //static Long64_t npulses2o[MAX_CH];
  //static Long64_t npulses3o[MAX_CH];
  //static double rate2[MAX_CH];
  //static double rate_all2;
  //static double rate3[MAX_CH];
  //static double rate_all3;

  if (rst) {
    crs->npulses_bad[MAX_CH]=0;
    t1=0;
    opt.T_acq=0;
    memset(crs->npulses2o,0,sizeof(crs->npulses2o));
    //memset(npulses3o,0,sizeof(npulses3o));
    memset(crs->rate_soft,0,sizeof(crs->rate_soft));
    //memset(rate3,0,sizeof(rate3));
  }

  TGString txt;

  double dt = opt.T_acq - t1;

  if (dt>0.1) {
    crs->rate_soft[MAX_CH]=0;
    crs->rate_hard[MAX_CH]=0;

    for (int i=0;i<opt.Nchan;i++) {
      if (cpar.on[i]) { //only for active channels
	crs->rate_soft[i] = (crs->npulses2[i]-crs->npulses2o[i])/dt;
	// prnt("ss d f l ls;",BBLU,"rate:",i,crs->rate_soft[i],crs->npulses2[i],
	//      crs->npulses2o[i],RST);
	crs->npulses2o[i]=crs->npulses2[i];

	crs->rate_soft[MAX_CH]+=crs->rate_soft[i];
	crs->rate_hard[MAX_CH]+=crs->rate_hard[i];

	crs->npulses_bad[MAX_CH]+=crs->npulses_bad[i];
      }
    }
    t1=opt.T_acq;
  }

  for (UInt_t i=0;i<Plist.size();i++) {
    if (Plist[i].type == p_stat)
      UpdateField(i);
  }

  /*
  for (int i=0;i<pmax;i++) {
    txt.Form("%0.0f",rate2[i]);
    fStat2[i]->SetText(txt);
    txt.Form("%0.0f",crs->rate_hard[i]);
    fStat3[i]->SetText(txt);
    txt.Form("%d",crs->npulses_bad[i]);
    fStatBad[i]->SetText(txt);
  }

  txt.Form("%0.0f",rate_all2);
  fStat2[MAX_CH]->SetText(txt);
  txt.Form("%0.0f",rate_all3);
  fStat3[MAX_CH]->SetText(txt);
  txt.Form("%lld",allbad);
  fStatBad[MAX_CH]->SetText(txt);
  */
  //cout << "Updatestatus3: " << crs->rate_soft[0] << endl;
}

//-------------------------
ErrFrame::ErrFrame(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{

  errflag=0;

  AddFrame(fDock, LayLE0);
  fDock->SetWindowName("Errors");
	
  TGCompositeFrame *fMain=fDock->GetContainer();
  fMain->SetLayoutManager(new TGVerticalLayout(fMain));
	
  fCanvas1 = new TGCanvas(fMain,w,h);
  fMain->AddFrame(fCanvas1,LayEE0);
  fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
				1, 1, kVerticalFrame);
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

    fErr[i]->Resize(70,20);
    int col=gROOT->GetColor(19)->GetPixel();
    fErr[i]->SetBackgroundColor(col);
    fErr[i]->SetText(0);
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

  for (int i=0;i<MAX_ERR;i++) {
    txt.Form("%lld",crs->errors[i]);
    fErr[i]->SetText(txt);
    if (crs->errors[i] && !errflag) {
      errflag=1;
      TGTabElement* tab6 = myM->fTab->GetTabTab("Errors");
      tab6->SetBackgroundColor(fRed);
      tab6->Resize();
      tab6->Layout();
    }
  }
}
