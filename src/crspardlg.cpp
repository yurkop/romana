#include "crspardlg.h"
#include "libcrs.h"
#include "toptions.h"
#include "TGFileDialog.h"
#include <TGSplitter.h>
#include <TG3DLine.h>
#include <TSystem.h>
#include "romana.h"

//const char* t_raw = "Write raw data";

extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern CrsParDlg *chanpar;

const int ncrspar=12;

const int tlen[ncrspar]={26,60,25,25,25,21,45,40,40,36,21,37};
const char* tlab[ncrspar]={"Ch","Type","on","Inv","AC","hS","Dt","Pre","Len","Drv","G","Thr"};
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
const int tlen2[nchpar]={26,60,25,25,35,35,42,42,35,35,20,35,35,35,42,42};
const char* tlab2[nchpar]={"Ch","Type","St","sS","Bkg1","Bkg2","Peak1","Peak2","dT","Pile","Tm","T1","T2","EM","ELim1","Elim2"};
const char* tip2[nchpar]={
  "Channel number",
  "Channel type",
  "Start channel\nif there are many start channels in the event, the earliest is used",
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
extern Toptions opt;
extern int chanPresent;
extern MyMainFrame *myM;

using namespace std;

ParDlg::ParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :TGCompositeFrame(p,w,h,kVerticalFrame)
{

  nfld=0;

  // fL0 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
  // fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 1, 1);
  // fL2 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
  // fL3 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 4, 4, 1, 1);
  // fL4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 1, 1, 5, 5);
  // fL5 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 1, 1, 2, 2);
  // fL6 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2, 2, 2, 2);

  fL0 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 0, 0);
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  fL2 = new TGLayoutHints(kLHintsLeft | kLHintsExpandY);
  fL3 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 4, 4, 0, 0);
  fL4 = new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 0, 0, 5, 5);
  fL5 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 0, 0, 2, 2);
  fL6 = new TGLayoutHints(kLHintsExpandX | kLHintsTop, 2, 2, 2, 2);

  fLexp = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY);

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
	if (pp.all==1 || opt.chtype[i]==pp.all-1) {
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
    //   if (pp.all==1 || opt.chtype[i]==pp.all-1) {
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
	if (pp.all==1 || opt.chtype[i]==pp.all-1) {
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
  pp = Plist[id];
  TGTextButton *but = (TGTextButton*) pp.field;
  pp = Plist[id+1];
  TGTextEntry *te2 = (TGTextEntry*) pp.field;

  Bool_t state = (Bool_t) te->GetState();      

  te2->SetEnabled(state);
  but->SetEnabled(state);

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

    cout << "DoOpen: " << id << " " << fi.fFilename << endl;
    cout << "DoOpen: " << fi.fIniDir << endl;
     
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

  cout << "DoCombo: " << id << " " << nline << " " << (int) pp.all 
       << " " << nfld
       << " " << sel << " " << (chanPresent+sel)*nfld
       << endl;

  cout << this << " " << crspar << " " << chanpar << endl;

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
}

void ParDlg::Update() {

  for (UInt_t i=0;i<Plist.size();i++) {
    UpdateField(i);
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
  AddWrite("Write raw data",&opt.raw_write,opt.fname_raw);

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

  AddWrite("Write decoded data",&opt.dec_write,opt.fname_dec);

  hor = new TGSplitFrame(fcont1,10,10);
  fcont1->AddFrame(hor,fLexp);
  hor->VSplit(380);
  ver1 = hor->GetFirst();
  ver2 = hor->GetSecond();

  AddPar(ver1);
  AddHist(ver2);

}

void ParParDlg::AddWrite(const char* txt, Bool_t* opt_chk, char* opt_fname) {
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

void ParParDlg::AddPar(TGCompositeFrame* frame) {
  
  int ww=70;

  /*
  TGLabel* fLabel = new TGLabel(frame, "---  Parameters  ---");
  frame->AddFrame(fLabel, fL5);
  frame->AddFrame(new TGHorizontal3DLine(frame), fL5);

  tip1= "Analysis start (in sec)";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine3(frame,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_int);
  
  frame->AddFrame(new TGHorizontal3DLine(frame), fL5);

  tip1= "Analysis start (in sec)";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine3(frame,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_int);

  tip1= "Delay between drawing events (in msec)";
  tip2= "";
  label="DrawEvent delay";
  AddLine3(frame,ww,&opt.tsleep,NULL,tip1,tip2,label,k_int);
  */

   TGGroupFrame* fF6 = new TGGroupFrame(frame, "Options", kVerticalFrame);
   fF6->SetTitlePos(TGGroupFrame::kCenter); // right aligned
   frame->AddFrame(fF6, fL6);

   // 2 column, n rows
   fF6->SetLayoutManager(new TGMatrixLayout(fF6, 0, 3, 7));

   // char buff[100];
   // for (int j = 0; j < 5; j++) {
   //    sprintf(buff, "Module %i", j+1);
   //    fF6->AddFrame(new TGLabel(fF6, new TGHotString(buff)));

   //    TGTextBuffer *tbuf = new TGTextBuffer(10);
   //    tbuf->AddText(0, "0.0");

   //    TGTextEntry  *tent = new TGTextEntry(fF6, tbuf);
   //    tent->Resize(50, tent->GetDefaultHeight());
   //    tent->SetFont("-adobe-courier-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
   //    fF6->AddFrame(tent);
   // }


  tip1= "Analysis start (in sec)";
  tip2= "Analysis stop (in sec)";
  label="Time limits";
  AddLine3(fF6,ww,&opt.Tstart,&opt.Tstop,tip1,tip2,label,k_int);

  tip1= "Delay between drawing events (in msec)";
  tip2= "";
  label="DrawEvent delay";
  AddLine3(fF6,ww,&opt.tsleep,NULL,tip1,tip2,label,k_int,100,10000);

  tip1= "Size of the USB buffer in kilobytes";
  tip2= "";
  label="USB buffer size";
  AddLine3(fF6,ww,&opt.buf_size,NULL,tip1,tip2,label,k_int,1,2048,
	   (char*) "DoNum_SetBuf()");

  tip1= "Minimal size of the event list";
  tip2= "Maximal size of the event list";
  label="Event_list size";
  AddLine3(fF6,ww,&opt.ev_min,&opt.ev_max,tip1,tip2,label,k_int);


  fF6->Resize();



}

void ParParDlg::AddHist(TGCompositeFrame* frame2) {
  
  int ww=70;
  
  //TGLabel* fLabel = new TGLabel(frame, "---  Histograms  ---");
  //frame->AddFrame(fLabel, fL5);


   TGGroupFrame* frame = new TGGroupFrame(frame2, "Histograms", kVerticalFrame);
   frame->SetTitlePos(TGGroupFrame::kCenter); // right aligned
   frame2->AddFrame(frame, fL6);

   // 2 column, n rows
   frame->SetLayoutManager(new TGMatrixLayout(frame, 0, 3, 7));


  tip1= "Initial bins per second for long_tdc";
  tip2= "Initial length of long_tdc (in seconds)";
  label="Long TDC";
  AddLine3(frame,ww,&opt.long_bins,&opt.long_max,tip1,tip2,label,k_r0);
  

  tip1= "Bins per second for tdc";
  tip2= "Length of tdc (in seconds)";
  label="TDC";
  AddLine3(frame,ww,&opt.tdc_bins,&opt.tdc_max,tip1,tip2,label,k_r0);

  tip1= "Bins per mks for M_TOF";
  tip2= "Length of MTof (in mks)";
  label="M_TOF";
  AddLine3(frame,ww,&opt.mtof_bins,&opt.mtof_max,tip1,tip2,label,k_r0);

  tip1= "Bins per channel/MeV for Energy";
  tip2= "Length of Energy (in channels/MeV)";
  label="Energy";
  AddLine3(frame,ww,&opt.sum_bins,&opt.sum_max,tip1,tip2,label,k_r0);

  tip1= "Bins per channel for Width";
  tip2= "Length of Width (in channels)";
  label="Width";
  AddLine3(frame,ww,&opt.rms_bins,&opt.rms_max,tip1,tip2,label,k_r0);

  tip1= "Bins per nanosecond for TOF";
  tip2= "Length of TOF (in nanoseconds)";
  label="TOF";
  AddLine3(frame,ww,&opt.tof_bins,&opt.tof_max,tip1,tip2,label,k_r0);

}

/*
void ParParDlg::AddLine3(TGCompositeFrame* frame, int width, void *x1, void *x2, 
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

void ParParDlg::AddLine3(TGGroupFrame* frame, int width, void *x1, void *x2, 
		      const char* tip1, const char* tip2, const char* label,
		      TGNumberFormat::EStyle style, 
			 //TGNumberFormat::EAttribute attr, 
			 double min, double max, char* connect)
{

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

  TGLabel* fLabel = new TGLabel(frame, label);
  frame->AddFrame(fLabel);

  TGNumberFormat::ELimit limits = TGNumberFormat::kNELNoLimits;
  if (max!=0) {
    limits = TGNumberFormat::kNELLimitMinMax;
  }

  id = Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(frame, 0, 0, id, style, 
					   TGNumberFormat::kNEAAnyNumber,
			     limits,min,max);
  DoMap(fNum1->GetNumberEntry(),x1,pdef,0);
  fNum1->GetNumberEntry()->SetToolTipText(tip1);
  //fNum1->SetWidth(5);
  fNum1->Resize(width, fNum1->GetDefaultHeight());
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, connect);
  frame->AddFrame(fNum1);

  if (x2!=NULL) {
    id = Plist.size()+1;
    TGNumberEntry* fNum2 = new TGNumberEntry(frame, 0, 0, id, style, 
					     TGNumberFormat::kNEAAnyNumber,
					     limits,min,max);
    DoMap(fNum2->GetNumberEntry(),x2,pdef,0);
    fNum2->GetNumberEntry()->SetToolTipText(tip2);
    //fNum2->SetWidth(width);
    fNum2->Resize(width, fNum2->GetDefaultHeight());
    fNum2->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", this, connect);
    frame->AddFrame(fNum2);
  }
  else {
    TGLabel* fskip = new TGLabel(frame, "");
    //fskip->SetWidth(width);
    frame->AddFrame(fskip);
  }
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
//------ CrsParDlg -------

CrsParDlg::CrsParDlg(const TGWindow *p,UInt_t w,UInt_t h)
  :ParDlg(p,w,h)
{
}

void CrsParDlg::Make_crspar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader1();

  fCanvas = new TGCanvas(this,w,h);
  AddFrame(fCanvas,fL2);

  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);

  for (int i=0;i<chanPresent;i++) {
    AddLine1(i);
  }

  //TGHorizontal3DLine* separator1 = new TGHorizontal3DLine(fcont1);
  //TGHorizontal3DLine* separator2 = new TGHorizontal3DLine(fcont1);
  //fcont1->AddFrame(separator1, fL5);

  //TGHorizontalFrame *separ = new TGHorizontalFrame(fcont1,10,10);
  //fcont1->AddFrame(separ,fL4);

  fcont1->AddFrame(new TGHorizontal3DLine(fcont1), fL5);
  AddLine1(MAX_CH);
  //fcont1->AddFrame(new TGHorizontal3DLine(fcont1), fL5);
  //fcont1->AddFrame(separ,fL4);

  for (int i=1;i<ADDCH;i++) {
    AddLine1(MAX_CH+i);
    //fcont1->AddFrame(new TGTextButton(fcont1, "&Test button1", 0), fL1);
  }

  //cout << "module: " << crs->module << endl;

  if (crs->module==2) {
    TGHorizontalFrame *hforce = new TGHorizontalFrame(fcont1,10,10);
    fcont1->AddFrame(hforce,fL1);

    int id = Plist.size()+1;
    TGCheckButton *fforce = new TGCheckButton(hforce, "", id);
    hforce->AddFrame(fforce,fL3);
    DoMap((TGWidget*)fforce,&opt.forcewr,p_chk,0);
    fforce->Connect("Clicked()", "ParDlg", this, "DoChk()");
    
    // pmap pp;
    // pp.field = &fforce;
    // pp.data = &opt.forcewr;
    // pp.type = 2;
    // Plist.push_back(pp);
    TGLabel* lforce = new TGLabel(hforce, "  Force writing all channels");
    hforce->AddFrame(lforce,fL1);
  }

  //cout << "nfld2: " << Plist.size() << " " << nfld << endl;

}

void CrsParDlg::Make_chanpar(const TGWindow *p,UInt_t w,UInt_t h) {

  AddHeader2();

  fCanvas = new TGCanvas(this,w,h);
  AddFrame(fCanvas,fL2);

  fcont1 = new TGCompositeFrame(fCanvas->GetViewPort(), 
				100, 100, kVerticalFrame);
  fCanvas->SetContainer(fcont1);

  for (int i=0;i<chanPresent;i++) {
    AddLine2(i);
  }

  //TGHorizontalFrame *separ = new TGHorizontalFrame(fcont1,10,10);
  //fcont1->AddFrame(separ,fL4);

  fcont1->AddFrame(new TGHorizontal3DLine(fcont1), fL5);
  AddLine2(MAX_CH);
  //fcont1->AddFrame(new TGHorizontal3DLine(fcont1), fL5);

  //fcont1->AddFrame(separ,fL4);

  for (int i=1;i<ADDCH;i++) {
    AddLine2(MAX_CH+i);
  }


}

void CrsParDlg::AddHeader1() {

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  AddFrame(hframe1,fL1);

  TGTextEntry* tt[ncrspar];

  for (int i=0;i<ncrspar;i++) {
    tt[i]=new TGTextEntry(hframe1, tlab[i]);
    tt[i]->SetWidth(tlen[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(tip[i]);
    tt[i]->SetAlignment(kTextCenterX);
    hframe1->AddFrame(tt[i],fL0);
  }

}

void CrsParDlg::AddHeader2() {

  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  AddFrame(hframe1,fL1);

  TGTextEntry* tt[nchpar];

  for (int i=0;i<nchpar;i++) {
    tt[i]=new TGTextEntry(hframe1, tlab2[i]);
    tt[i]->SetWidth(tlen2[i]);
    tt[i]->SetState(false);
    tt[i]->SetToolTipText(tip2[i]);
    tt[i]->SetAlignment(kTextCenterX);
    hframe1->AddFrame(tt[i],fL0);
  }

}

void CrsParDlg::AddLine1(int i) {
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

  DoMap(fCombo,&opt.chtype[i],p_cmb,all);

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");

  id = Plist.size()+1;
  TGCheckButton *f_en = new TGCheckButton(hframe1, "", id);
  TGCheckButton *f_inv = new TGCheckButton(hframe1, "", id+1);
  TGCheckButton *f_acdc = new TGCheckButton(hframe1, "", id+2);
  DoMap(f_en,&opt.enabl[i],p_chk,all);
  DoMap(f_inv,&opt.inv[i],p_chk,all);
  DoMap(f_acdc,&opt.acdc[i],p_chk,all);

  f_en->Connect("Clicked()", "ParDlg", this, "DoChk()");
  f_inv->Connect("Clicked()", "ParDlg", this, "DoChk()");
  f_acdc->Connect("Clicked()", "ParDlg", this, "DoChk()");

  //f_inv->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_acdc->ChangeOptions(facdc->GetOptions()|kFixedSize);
  //f_inv->SetWidth(25);
  //f_acdc->SetWidth(25);

  hframe1->AddFrame(f_en,fL3);
  hframe1->AddFrame(f_inv,fL3);
  hframe1->AddFrame(f_acdc,fL3);
  kk+=3;

  AddNum1(i,kk++,all,hframe1,"smooth",&opt.smooth[i]);
  AddNum1(i,kk++,all,hframe1,"dt"    ,&opt.deadTime[i]);
  AddNum1(i,kk++,all,hframe1,"pre"   ,&opt.preWr[i]);
  AddNum1(i,kk++,all,hframe1,"len"   ,&opt.durWr[i]);
  AddNum1(i,kk++,all,hframe1,"deriv" ,&opt.kderiv[i]);
  AddNum1(i,kk++,all,hframe1,"gain"  ,&opt.adcGain[i]);
  AddNum1(i,kk++,all,hframe1,"thresh",&opt.threshold[i]);

}

void CrsParDlg::AddLine2(int i) {
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

  DoMap(fCombo,&opt.chtype[i],p_cmb,all);

  fCombo->Connect("Selected(Int_t)", "ParDlg", this, "DoCombo()");


  id = Plist.size()+1;
  TGCheckButton *fst = new TGCheckButton(hframe1, "", id);
  DoMap(fst,&opt.Start[i],p_chk,all);

  fst->Connect("Clicked()", "ParDlg", this, "DoChk()");

  hframe1->AddFrame(fst,fL3);
  kk++;



  AddNum2(i,kk++,all,hframe1,&opt.nsmoo[i],0,99,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.bkg1[i],-999,999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.bkg2[i],-999,999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.peak1[i],-999,16500,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.peak2[i],-999,16500,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.deadTime[i],0,9999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.pile[i],0,9999,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.timing[i],0,1,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.twin1[i],-99,99,p_inum);
  AddNum2(i,kk++,all,hframe1,&opt.twin2[i],-99,99,p_inum);

  AddNum2(i,kk++,all,hframe1,&opt.emult[i],0,99999,p_fnum);
  AddNum2(i,kk++,all,hframe1,&opt.elim1[i],0,99999,p_fnum);
  AddNum2(i,kk++,all,hframe1,&opt.elim2[i],0,99999,p_fnum);
}

void CrsParDlg::AddNum1(int i, int kk, int all, TGHorizontalFrame *hframe1,
		       const char* name, void* apar) {  //const char* name) {

  int par, min, max;

  opt.GetPar(name,crs->module,i,par,min,max);

  int id = Plist.size()+1;
  TGNumberEntryField* fNum =
    new TGNumberEntryField(hframe1, id, par, TGNumberFormat::kNESInteger,
			   TGNumberFormat::kNEAAnyNumber,
			   TGNumberFormat::kNELLimitMinMax,min,max);

  DoMap(fNum,apar,p_inum, all);
  
  fNum->SetWidth(tlen[kk]);

  fNum->Connect("TextChanged(char*)", "ParDlg", this, "DoNum()");

  hframe1->AddFrame(fNum,fL0);

}

void CrsParDlg::AddNum2(int i, int kk, int all, TGHorizontalFrame *hframe1,
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

  DoMap(fNum,apar,ptype, all);
  fNum->SetWidth(tlen2[kk]);
  fNum->Connect("TextChanged(char*)", "ParDlg", this, "DoNum()");
  hframe1->AddFrame(fNum,fL0);

}

/*
void CrsParDlg::Update() {

  //cout << "chtype: " << opt.chtype[0] << endl;

  for (std::vector<pmap>::iterator pp = Plist.begin();
       pp != Plist.end(); ++pp) {

      // int i = pp-Plist.begin();
      // if (i==1) {
      // cout << "Update: " << i << " " ;
      // cout << *(Int_t*) pp->data << " " ;
      // cout << opt.bkg1[0];
      // //cout << *(Int_t*) pp->type;
      // cout << endl;
      // }

    //cout << (int) pp->type << endl;
    if (pp->type==p_inum) {
      TGNumberEntryField *te = (TGNumberEntryField*) pp->field;
      te->SetNumber(*(Int_t*) pp->data);
    }
    else if (pp->type==p_chk) {
      TGCheckButton *te = (TGCheckButton*) pp->field;
      Bool_t bb = *(Bool_t*) pp->data;
      te->SetState((EButtonState) bb);
    }
    else if (pp->type==p_cmb) {
      TGComboBox *te = (TGComboBox*) pp->field;
      te->Select(*(ChDef*) pp->data,false);
    }

  }

  //cout << "threshold0: " << opt.threshold[0] << endl;
}
*/

/*
  void CrsParDlg::DoPar(int k, int i, Long_t num) {
  switch (k) {
  case 1:
  opt.smooth[i]=num;
  break;
  case 2:
  opt.deadTime[i]=num;
  break;
  case 3:
  opt.preWr[i]=num;
  break;
  case 4:
  opt.durWr[i]=num;
  break;
  case 5:
  opt.kderiv[i]=num;
  break;
  case 6:
  opt.adcGain[i]=num;
  break;
  case 7:
  opt.threshold[i]=num;
  break;
  }
  }

  void *CrsParDlg::MapPar(int id) {

  int i = id%1000;
  int k = id/1000;

  switch (k) {
  case 1:
  return &opt.smooth[i];
  case 2:
  return &opt.deadTime[i];
  case 3:
  return &opt.preWr[i];
  case 4:
  return &opt.durWr[i];
  case 5:
  return &opt.kderiv[i];
  case 6:
  return &opt.adcGain[i];
  case 7:
  return &opt.threshold[i];
  }
  return NULL;
  }
*/
