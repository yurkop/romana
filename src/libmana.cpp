//#define LONGSTAMP 1

#ifndef LINUX
#include <direct.h>
#endif

#include <signal.h>
#include <malloc.h>

#include "romana.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>

//#include <TTree.h>
#include <TClass.h>
#include <TH1.h>
//#include <TH2.h>
#include <TApplication.h>
#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TColor.h>
#include <TBrowser.h>

#include <TPolyMarker.h>
#include <TF1.h>
#include <TText.h>
#include <TString.h>

#include <TGResourcePool.h>
#include <TGDockableFrame.h>

#include <TDataMember.h>
#include "TThread.h"
#include "TImage.h"

//#include <TGColorDialog.h>

#include "libcrs.h"

const double MB = 1024*1024;

extern CRS* crs;

EventFrame* EvtFrm;
HistFrame* HiFrm;
HClass* hcl;

ParParDlg *parpar;
CrsParDlg *crspar;
ChanParDlg *chanpar;

//const int maxsamp = 16500;// константу 16500 надо будет заменить на переменную

char* parname=(char*)"romana.par";;
char* parname2=0;
char* datfname=0;




char startdir[200];
char pr_name[200];
char maintitle[200];
char rootname[200]="";

struct stat statbuffer;
const char* msg_exists = "Output file already exists. It will be overwritten.\nPress OK if you want to continue?";


MyMainFrame *myM;

Coptions cpar;
Toptions opt;
int debug=0; //=1 or 2 or 6// for printing debug messages

//int *opt_id[MXNUM];

/*
  void *handle_dum(void* ptr)
  {

  gSystem->Sleep(1000);
  cout << myM << endl;
  cout << HiFrm << endl;
  HiFrm->Emit("Upd()");
  //HiFrm->ReDraw();
  ////cout << "Dum: " << endl;

  return NULL;

  }
*/

//void printhlist(int n);

//-------------------------------------
UShort_t ClassToBuf(const char* name, const char* varname, char* var, char* buf) {
  //copies all data members to a buffer, returns size of the buffer
  //buffer should exist and have sufficient size to allocate all data

  //cout << "ClassToBuf: " << name << endl;
  TList* lst = TClass::GetClass(name)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << name << " doesn't exist" << endl;
    return 0;
  }

  Int_t sz=0;
  Short_t len=0;

  char cname[100];
  strcpy(cname,"class");

  len = strlen(cname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,cname,len);
  sz+=len;

  len = strlen(name)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,name,len);
  sz+=len;

  strcpy(cname,"var");
  len = strlen(cname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,cname,len);
  sz+=len;

  len = strlen(varname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,varname,len);
  sz+=len;

  if (debug&0x2)
    cout << "Save class: " << name << endl;

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
    //if (debug&0x2) {
    // if (!dm->GetDataType()) {
    //   cout << "member: " << dm->GetName() << " " << dm->GetDataType() << " " << dm->GetClass()->GetName() << endl;
    // }
    //}
    if (dm->GetDataType()) {
      len = strlen(dm->GetName())+1;
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,dm->GetName(),len);
      sz+=len;
      len=dm->GetUnitSize();
      for (int i=0;i<dm->GetArrayDim();i++) {
	len*=dm->GetMaxIndex(i);
      }
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,var+dm->GetOffset(),len);
      sz+=len;
      if (debug&0x2)
	cout << dm->GetName() << " " << len << " " << sz << endl;
    }
  }

  if (debug&0x2)
    cout << "size: " << sz << endl;
  return sz;

}

//------------------------------------

void BufToClass(const char* name, const char* varname, char* var, char* buf, int size) {
  //copies all data members from a buffer, size - size of the buffer
  //buffer should exist. Only data members with matching names are copied

  if (debug&0x2)
    cout <<"BufToClass:: " << size << endl;

  TList* lst = TClass::GetClass(name)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << name << " doesn't exist" << endl;
    return;
  }

  Int_t sz=0;
  UShort_t len=0;
  UShort_t len2=0;
  //TIter nextd(lst);
  TDataMember *dm;
  char memname[100];
  char clname[100]; //class name
  char vname[100]; //class var name
  const UShort_t mx=50000;
  char data[mx];

  if (TString(name).EqualTo("Toptions",TString::kIgnoreCase)) {
    strcpy(vname,"opt");
  }
  else if (TString(name).EqualTo("Coptions",TString::kIgnoreCase)) {
    strcpy(vname,"cpar");
  }
  else {
    strcpy(vname,"");
  }

  while (sz<size) {
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>size) {
      cout << "br1: " << endl;
      break;
    }
    memcpy(memname,buf+sz,len);
    sz+=len;
    if (sz>size) {
      cout << "br2: " << endl;
      break;
    }
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>size) {
      cout << "br3: " << endl;
      break;
    }
    memcpy(data,buf+sz,len);
    sz+=len;
    if (sz>size) {
      cout << "br4: " << sz << " " << size << endl;
      break;
    }

    if (strcmp(memname,"class")==0) {
      strcpy(clname,data);
      if (debug&0x2)
	if (strcmp(clname,name)==0) { //the same class
	  cout << "Read class: " << clname << " " << name << endl;
	}
      continue;
    }

    if (strcmp(memname,"var")==0) {
      strcpy(vname,data);
      if (debug&0x2)
	if (strcmp(vname,varname)==0) { //the same class
	  cout << "Read var: " << vname << " " << varname << endl;
	}
      continue;
    }

    if (!strcmp(clname,name) && !strcmp(vname,varname)) { //the same class & var
      dm = (TDataMember*) lst->FindObject(memname);
      if (dm) {
	len2=dm->GetUnitSize();
	for (int i=0;i<dm->GetArrayDim();i++) {
	  len2*=dm->GetMaxIndex(i);
	}
	if (debug&0x4)
	  cout << dm->GetName() << " " << len << " " << sz << endl;
	memcpy(var+dm->GetOffset(),data,TMath::Min(len,len2));
      }
      else {
	if (debug&0x2)
	  cout << "class member not found: " << dm << " " << memname << " " 
	       << clname << " " << name << endl;
      }
    }

  }

  if (debug&0x2)
    cout << "len: " << len << " " << sz << endl;
  

}
//--------------------------------

void out_of_memory(void)
{
  std::cerr << "Out of memory. Please go out and buy some more." << std::endl;
  exit(-1);
}

void ctrl_c_handler(int s){
  printf("Caught signal %d\n",s);
  delete myM;
  exit(1); 
}

void segfault_c_handler(int signal, siginfo_t *si, void *arg) {
  printf("Caught segfault %d\n",signal);
  delete myM;
  exit(-1);
  //exit(1); 
}

int main(int argc, char **argv)
{

  string s_name, dir, name, ext;

  //bool batch=false;

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = ctrl_c_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);





  // void segfault_sigaction(int signal, siginfo_t *si, void *arg)
  // {
  //     printf("Caught segfault at address %p\n", si->si_addr);
  //     exit(0);
  // }

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = segfault_c_handler;
  sa.sa_flags   = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);

  // cout << "sizeof(TDatime): " << sizeof(TDatime) << endl;
  // cout << "sizeof(Toptions): " << sizeof(Toptions) << endl;
  // cout << "sizeof(opt): " << sizeof(opt) << endl;
  
  hcl = new HClass();
  crs = new CRS();

  // #ifdef CYUSB
  //   crs->Detect_device();
  // #endif

#ifdef LINUX
  if (getcwd(startdir,150)) {
    strcat(startdir,"/");
  }
#else
  _getcwd(startdir,150);
  strcat(startdir,"\\");
#endif
  //cout << "startdir: " << startdir << endl;

  cout << "----------------------------------------------" << endl;
  cout << "Usage: ./romana.x [filename] [+parname] [-p parname] [-b]" << endl;
  cout << "filename - read data and parameters from filename" << endl;
  cout << "+parname or" << endl;
  cout << "-p parname - read parameters from parname, parameters of filename are ignored" << endl;
  cout << "-b - analyze file in batch mode (without gui) and exit" << endl;
  cout << "----------------------------------------------" << endl;

  strcpy(pr_name,argv[0]);
  strcpy(maintitle,pr_name);

  //parname = (char*)"romana.par";
  gzFile ff = gzopen(parname,"rb");
  if (!ff) {
    cout << "Can't open par file: " << parname << endl;
  }
  else {
    crs->ReadParGz(ff,parname,0,1,1);
    gzclose(ff);
  }

  //process command line parameters
  if (argc > 1) {
    int argnn=1;
    while (argnn<argc) {
      // cout << "argnn: " << argc << " " << argnn << " " << argv[argnn] << " "
      // 	   << argv[argnn]+1 << endl;
      char cc = argv[argnn][0];
      if (cc=='-') { //control character
	char pp = argv[argnn][1];
	switch (pp) {
	case 'b':
	case 'B':
	  crs->batch=true;
	  break;
	case 'p':
	case 'P':
	  argnn++;
	  //cout << argnn+1 << " " << argc << endl;
	  if (argnn<argc) {
	    parname2 = argv[argnn];
	  }
	  break;
	default:
	  break;
	}
      }
      else if (cc=='+') { //read file of parameters
	parname2 = argv[argnn]+1;
      }
      else { //read file
	datfname = argv[argnn];
      }
      argnn++;
    }
  }

  //if (parname2) cout << "parname2: " << parname2 << endl;
  //if (datfname) cout << "datfname: " << datfname << endl;
  //exit(1);

  int rdpar=1;
  if (parname2) {
    gzFile ff = gzopen(parname2,"rb");
    if (!ff) {
      cout << "Can't open par file: " << parname2 << endl;
    }
    else {
      crs->ReadParGz(ff,parname2,0,1,1);
      gzclose(ff);
      rdpar=0; // if parname2 is OK -> don't read par from file
    }
  }

  //cout << "class: " << TClass::GetClass("hdef")->GetNdata() << endl;
  //cout << "tof_max: " << opt.h_tof.max << endl;
  //opt.h_tof.max=100;

  if (datfname) {
    crs->DoFopen(datfname,rdpar); //read file and parameters from it
  }
  else {
    datfname=(char*)"";
  }

  //cout << "strlen: " << strlen(datfname) << endl;
  //cout << "gStyle1: " << gStyle << endl;
  //hcl->Make_hist();
  //cout << "gStyle2: " << gStyle << endl;

  EvtFrm = 0;
  if (crs->batch) {
    if (strlen(datfname)==0)
      return 0;

    SplitFilename (string(datfname),dir,name,ext);
    dir = TString(startdir);
    //cout << "Root_dir: " << dir << endl;
    dir.append("Root/");
#ifdef LINUX
    mkdir(dir.c_str(),0755);
#else
    _mkdir(dir.c_str());
#endif
    s_name = dir;
    s_name.append(name);
    s_name.append(".root");
    strcpy(opt.fname_root,s_name.c_str());

    hcl->Make_hist();
    //cout << "batch0: " << endl;

    crs->b_fana=true;
    crs->b_stop=false;

    crs->FAnalyze2(false);

    crs->b_fana=false;
    crs->b_stop=true;

    //allevents();
    //cout << "batch99: " << endl;

    cout << opt.fname_root << endl;
    saveroot(opt.fname_root);

    return 0;
  }


#ifdef CYUSB
  crs->Detect_device();
#endif

  TApplication theApp("App",&argc,argv);
  //example();

  myM=0;
  myM = new MyMainFrame(gClient->GetRoot(),800,600);

  //gSystem->Sleep(100);
  //crs->Dummy_trd();

  //EvtFrm->StartThread();
  //gClient->GetColorByName("yellow", yellow);
  //cout << "Init end2" << endl;
  theApp.Run();
  //cout << "Init end3" << endl;
  //return 0;

  //fclose(fp);
  //gzclose(fp);

  /*
  //printf("%d buffers\n",nbuf);
  printf("%lld bytes\n",recd*2);
  printf("%d events\n",nevent);
  printf("%d bad events\n",bad_events);
  */
  //HRPUT(0,(char*)"spectra.hbook",(char*)" ");

  //memcpy(buf2,buf[0],BSIZE);

}

void SplitFilename (string str, string &folder, string &name)
{
  size_t found;
  found=str.find_last_of("/\\");

  folder = str.substr(0,found+1);
  name = str.substr(found+1);
  //cout << "spl1: " << str << ";" << folder << ";" << name << endl;
  //strcpy(folder,str.substr(0,found+1).c_str());
  //strcpy(name,str.substr(found+1).c_str());

}

void SplitFilename (string str, string &folder, string &name, string &ext)
{

  SplitFilename(str, folder, name);
  string fname(name);
  size_t found = fname.find_last_of(".");
  //cout << "spl22: " << found << " " << string::npos << endl;
  //return;
  name = fname.substr(0,found);
  if (found!=string::npos) {
    ext = fname.substr(found);
  }
  //cout << "spl2: " << str << ";" << folder << ";" << name << ";" << ext << endl;
  //strcpy(name,fname.substr(0,found).c_str());
  //strcpy(ext,fname.substr(found+1).c_str());
}

void saveroot(const char *name) {

  TFile * tf = new TFile(name,"RECREATE");

  if (!tf->IsOpen()) {
    cout << "Can't open file: " << name << endl;
    return;
  }
  //cout << "saveroot11: " << name << " " << tf << " " << tf->IsOpen() << endl;

  int col;

  gROOT->cd();

  TIter next(gDirectory->GetList());

  tf->cd();

  TH1F *h;
  TH2F *h2;
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (obj->InheritsFrom(TH2::Class())) {
      h2=(TH2F*) obj;
      //h2->Print();
      if (h2->GetEntries() > 0) {
	//printf("saveroot2: %s\n",h2->GetName());
	h2->Write();
      }
    }
    else if (obj->InheritsFrom(TH1::Class())) {
      h=(TH1F*) obj;
      //h->Print();
      if (h->GetEntries() > 0) {
	col=h->GetLineColor();
	//printf("saveroot1: %d %s\n",col,h->GetName());
	if (col==0) {
	  h->SetLineColor(50);
	}
	//tf->WriteTObject(obj);
	h->Write();
	h->SetLineColor(col);
      }
    }
  }

  //opt.Nevt=nevent;
  //opt.Tof=tof;
  cpar.Write();
  opt.Write();

  cout << "Histograms and parameters are saved in file: " << name << endl;

  tf->Close();
}

void readroot(char *name) {

  //char nam[100];

  //cout << opt.channels[0] << endl;

  gROOT->cd();
  //TList *list = gDirectory->GetList();
  TList *list = hcl->hist_list;

  //list->ls();

  TFile *tf = new TFile(name,"READ");

  TIter next(tf->GetListOfKeys());

  TKey *key;
  TObject *obj;
  TH1* obj2;
  while ((key = (TKey*)next())) {
    //key->Print();

    //cout << key->GetClassName() << endl;
    if (strcmp(key->GetClassName(),"Toptions")) {
      obj=key->ReadObj();
      if (obj->InheritsFrom(TH1::Class())) {
	//obj->Print();
	obj2 = (TH1*) list->FindObject(obj->GetName());
	if (obj2) {
	  //printf("%d\n",obj2);
	  // cout << "readroot1: " << obj2 << " " << obj2->GetName() << " "
	  //      << ((TH1*) obj2)->Integral() << endl;
	  TDirectory* dir = obj2->GetDirectory();
	  obj->Copy(*obj2);
	  obj->Delete();
	  obj2->SetDirectory(dir);
	  // cout << "readroot2: " << obj2 << " " << obj2->GetName() << " "
	  //      << ((TH1*) obj2)->Integral() << endl;
	}
      }
    }

  }

  //opt.Read("Toptions");

  tf->Close();

  //strcpy(maintitle,pr_name);
  //strcat(maintitle," ");
  //strcat(maintitle,name);

  //cout << opt.channels[0] << endl;
}

void clear_hist() {

  //memset(f_long_t,0,sizeof(f_long_t));

  TIter next(gDirectory->GetList());

  TH1 *h;
  //TH2F *h2;
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (obj->InheritsFrom(TH1::Class())) {
      h=(TH1*) obj;
      h->Reset();
    }
  }

}

void readpar_root(const char* pname)
{
  TFile *f2 = new TFile(pname,"READ");

  cpar.Read("Coptions");
  opt.Read("Toptions");

  f2->Close();
  delete f2;
}

/*
  void savepar_root(const char* pname)
  {

  #ifdef LINUX
  if (chdir(startdir)) {}
  #else
  _chdir(startdir);
  #endif

  TFile *f2 = new TFile(pname,"RECREATE");

  cpar.Write();
  opt.Write();

  f2->Close();
  delete f2;

  }
*/

void swap_bytes(unsigned short* buf)
{
  //int i,j;
  unsigned char *buf1;
  unsigned char tmp;

  buf1 = (unsigned char*) buf;

  tmp = *(buf1);
  *(buf1) = *(buf1+1);
  *(buf1+1) = tmp;
}

short int bits(int n, int i1, int i2)
{
  int ll=i2-i1+1;
  unsigned int mask = ((1 << ll)-1);
  //unsigned int mask = (0xFFFF << (32-i2)) >> (32-i2);
  short int r = (n>>i1) & mask;
  //printf("%x %d %d %d %x %d\n",n,i1,i2,ll,mask,r);
  //r = r >> i1;
  return r;
}

Bool_t getbit(int n, int bit) {
  return n & (1<<bit);
}
void setbit(int &n, int bit, int set) {
  if (set) {
    n |= 1<<bit;
  }
  else {
    n = n & ~(1<<bit);
  }
}
/*
//----- MFileDialog ----------------

//MFileDialog::MFileDialog(int &fType, const TGWindow* p, const TGWindow* main, EFileDialogMode dlg_type, TGFileInfo* file_info):TGFileDialog(p,main,dlg_type,file_info) {
MFileDialog::MFileDialog(const TGWindow* p, const TGWindow* main, EFileDialogMode dlg_type, TGFileInfo* file_info):TGFileDialog(p,main,dlg_type,file_info) {
cout << "finside: " << p << " " << main << " " << dlg_type << " " << file_info << endl;
}
*/

//----- MainFrame ----------------

MainFrame::MainFrame(const TGWindow *p,UInt_t w,UInt_t h)
  : TGMainFrame(p,w,h) {
  // Create a main frame

  //cout << "gStyle: " << gStyle << endl;
  gStyle->SetOptStat(kFALSE);
  gStyle->SetPalette(1,0);
  gStyle->SetTitleFontSize(0.07);
  gStyle->SetTitleSize(0.05,"xyz");
  gStyle->SetTitleOffset(0.95,"xy"); 
  gStyle->SetLabelSize(0.05,"xyz");
  gStyle->SetNdivisions(606,"xyz");
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetPadRightMargin(0.1);
  //gStyle->SetPadBottomMargin(0.15);
  //gStyle->SetPadTopMargin(0.05);
  

  /*
    fDNDTypeList = new Atom_t[3];
    fDNDTypeList[0] = gVirtualX->InternAtom("application/root", kFALSE);
    fDNDTypeList[1] = gVirtualX->InternAtom("text/uri-list", kFALSE);
    fDNDTypeList[2] = 0;
    if (gDNDManager) delete gDNDManager;
    gDNDManager = new TGDNDManager(this, fDNDTypeList);
  */


  //int nn=2;
  //double xx[nn];
  //double yy[nn];

  //fEv=NULL;

  TGLayoutHints* l_Gr = new TGLayoutHints(kLHintsCenterX|kLHintsTop,1,1,20,2);
  //TGLayoutHints* l_But = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,0,0,5,5);
  TGLayoutHints* l_But = new TGLayoutHints(kLHintsExpandX | kLHintsTop,0,0,5,5);
  TGLayoutHints* lay2 = new TGLayoutHints(kLHintsLeft | kLHintsTop,1,1,1,1);

  Lay11 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1);
  Lay12 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,3,3);
  //bRun = false;

  fMenuBar = new TGMenuBar(this, 35, 50, kHorizontalFrame);

  fMenuFile = new TGPopupMenu(gClient->GetRoot());

  fMenuFile->AddEntry("Read Parameters", M_READINIT);
  fMenuFile->AddEntry("Save Parameters", M_SAVEINIT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Read ROOT file", M_READROOT);
  fMenuFile->AddEntry("Save ROOT file", M_SAVEROOT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Export...", M_EXPORT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Browser\tCtrl+B", M_FILE_BROWSE);
  //fMenuFile->AddEntry("New Canvas\tCtrl+N", M_FILE_NEWCANVAS);

  //fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
  //fMenuFile->AddEntry("&Save", M_FILE_SAVE);

  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  //fMenuFile->AddPopup("&Cascaded menus", fCascadeMenu);
  fMenuFile->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");


  fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("Display Help file", M_HELP);
  //fMenuHelp->AddEntry("Test", M_TEST);
  fMenuHelp->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  fMenuBar->AddPopup("&File", fMenuFile, 
		     new TGLayoutHints(kLHintsTop|kLHintsLeft,0, 4, 0, 0));

  /*
    fMenuBar->AddPopup("&Options", fMenuOptions, 
    new TGLayoutHints(kLHintsTop|kLHintsLeft));

    fMenuBar->AddPopup("Histograms", fMenuHist, 
    new TGLayoutHints(kLHintsTop|kLHintsLeft));

    fMenuBar->AddPopup("Analysis", fMenuAna, 
    new TGLayoutHints(kLHintsTop|kLHintsLeft));
  */
  
  fMenuBar->AddPopup("&Help", fMenuHelp,
		     new TGLayoutHints(kLHintsTop|kLHintsRight));

  AddFrame(fMenuBar, 
	   new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 5));

  //fcanvas=NULL;
  //fAna=NULL;

  //fMain = new TGMainFrame(p,w,h);

  // Create a vertical frame for everything
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this,10,10);
  AddFrame(hframe1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  // Create a left vertical frame with buttons
  TGVerticalFrame *vframe1 = new TGVerticalFrame(hframe1,10,10);
  hframe1->AddFrame(vframe1, new TGLayoutHints(kLHintsLeft | kLHintsExpandY,2,2,2,2));

  TGFontPool *pool = gClient->GetFontPool();

  //pool->Print();

  // family , size (minus value - in pixels, positive value - in points), weight, slant
  // kFontWeightNormal, kFontSlantRoman are defined in TGFont.h
  //font = pool->GetFont("helvetica", -18, kFontWeightMedium, kFontSlantRoman);
  //const TGFont *font = pool->GetFont("helvetica", -18, kFontWeightBold, kFontSlantRoman);
  //const TGFont *font = pool->GetFont("helvetica", -18, 4, kFontSlantRoman);
  const TGFont *font = pool->GetFont("-*-helvetica-bold-r-*-*-18-*-*-*-*-*-iso8859-1",true);

  //const TGFont *font = gClient->GetFont("-*-arial-normal-r-*-*-20-*-*-*-*-*-*-*");

  //cout << "Font: " << font << endl;
  //font->Print();

  if (!font)
    font = gClient->GetResourcePool()->GetDefaultFont();
  FontStruct_t tfont = font->GetFontStruct();

  //cout << font << endl;

  const int butx=80,buty=40;
  //ULong_t fGreen;
  //ULong_t fRed;
  //ULong_t fCyan;
  //ULong_t fBluevio;

  gClient->GetColorByName("green", fGreen);
  gClient->GetColorByName("red", fRed);
  gClient->GetColorByName("cyan", fCyan);
  gClient->GetColorByName("BlueViolet",fBluevio);

  fBluevio=TColor::RGB2Pixel(255,114,86);
  Pixel_t fBlue = TColor::RGB2Pixel(135,92,231);;

  //cout << "fBluevio: " << fBluevio << " " << TColor::GetColor(fBluevio) << endl;

  //gROOT->GetListOfColors()->ls();

  TGGroupFrame* fGr1 = new TGGroupFrame(vframe1, "Acquisition", kVerticalFrame);
  fGr1->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  vframe1->AddFrame(fGr1, l_Gr);

  fStart = new TGTextButton(fGr1,"Start");
  //fStart->SetToggleButton(true);

  fStart->SetFont(tfont,false);
  fStart->Resize(butx,buty);
  fStart->ChangeOptions(fStart->GetOptions() | kFixedSize);
  //fStart->SetStyle("modern");

  fStart->ChangeBackground(fGreen);

  fStart->Connect("Clicked()","MainFrame",this,"DoStartStop()");
  fGr1->AddFrame(fStart, l_But);

  TGTextButton *fReset = new TGTextButton(fGr1,"Reset");
  fReset->SetFont(tfont,false);

  fReset->SetTextJustify(kTextCenterX);

  fReset->Resize(butx,buty);
  fReset->ChangeOptions(fStart->GetOptions() | kFixedSize);
  fReset->ChangeBackground(fCyan);

  fReset->Connect("Clicked()","MainFrame",this,"DoReset()");
  fGr1->AddFrame(fReset, l_But);

  TGGroupFrame* fGr2 = new TGGroupFrame(vframe1, "Analysis", kVerticalFrame);
  fGr2->SetTitlePos(TGGroupFrame::kCenter);
  vframe1->AddFrame(fGr2, l_Gr);



  TGTextButton *fOpen = new TGTextButton(fGr2,new TGHotString("&Open"));
  fOpen->SetFont(tfont,false);
  fOpen->Resize(butx,buty);
  fOpen->ChangeOptions(fOpen->GetOptions() | kFixedSize);
  fOpen->ChangeBackground(fBluevio);
  fOpen->Connect("Clicked()","MainFrame",this,"DoOpen()");
  fGr2->AddFrame(fOpen, l_But);

  TGTextButton *fClose = new TGTextButton(fGr2,new TGHotString("&Close"));
  fClose->SetFont(tfont,false);
  fClose->Resize(butx,buty);
  fClose->ChangeOptions(fClose->GetOptions() | kFixedSize);
  fClose->ChangeBackground(fBlue);
  fClose->Connect("Clicked()","MainFrame",this,"DoClose()");
  fGr2->AddFrame(fClose, l_But);

  TGTextButton *fReset2 = new TGTextButton(fGr2,new TGHotString("&Reset"));
  fReset2->SetFont(tfont,false);
  fReset2->Resize(butx,buty);
  fReset2->ChangeOptions(fReset2->GetOptions() | kFixedSize);
  fReset2->ChangeBackground(fCyan);
  fReset2->Connect("Clicked()","MainFrame",this,"DoReset()");
  //fReset2->Connect("Clicked()","CRS",crs,"Reset()");
  fGr2->AddFrame(fReset2, l_But);

  fAna = new TGTextButton(fGr2,"&Analyze");
  fAna->SetFont(tfont,false);
  fAna->Resize(butx,buty);
  fAna->ChangeOptions(fAna->GetOptions() | kFixedSize);
  fAna->ChangeBackground(fGreen);
  fAna->Connect("Clicked()","MainFrame",this,"DoAna()");
  fGr2->AddFrame(fAna, l_But);

  TGTextButton* f1b = new TGTextButton(fGr2,new TGHotString("&1 buf"));
  f1b->SetFont(tfont,false);
  f1b->Resize(butx,buty);
  f1b->ChangeOptions(f1b->GetOptions() | kFixedSize);
  f1b->ChangeBackground(fGreen);
  f1b->Connect("Clicked()","MainFrame",this,"Do1buf()");
  fGr2->AddFrame(f1b, l_But);


  fTab = new TGTab(hframe1, 300, 300);
  //TGLayoutHints *fL5 = new TGLayoutHints(kLHintsTop | kLHintsExpandX |
  //                                        kLHintsExpandY, 2, 2, 5, 1);
  hframe1->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX |
					    kLHintsExpandY,3,3,2,2));

  fTab->Connect("Selected(Int_t)", "MainFrame", this, "DoTab(Int_t)");


  //fremake=false;

  //cout << "tab1: " << endl;
  tabfr[0] = fTab->AddTab("Parameters");
  tabfr[1] = fTab->AddTab("DAQ");
  tabfr[2] = fTab->AddTab("Channels");
  tabfr[3] = fTab->AddTab("Events");
  tabfr[4] = fTab->AddTab("Histograms");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");

  //parpar = new ParParDlg(tabfr[0], 600, 500);
  //parpar->Update();
  //tabfr[0]->AddFrame(parpar, Lay11);

  MakeTabs();
  //fremake=true;

  //MakeTabs();

  if (crs->Fmode) {
    crspar->AllEnabled(false);
  }


  //cout << "hifrm3: " << endl;


  //TGCompositeFrame *tab6 = fTab->AddTab("Histograms2");
  //TGColorPick* tg = new TGColorPick(tab6);
  //tab6->AddFrame(tg, new TGLayoutHints(kLHintsExpandX| kLHintsExpandY,
  //					     2,2,2,2));
  //ntab++;

  if (!crs->module) {
    //TGTabElement *tabdaq = fTab->GetTabTab("DAQ");
    //tabdaq->SetEnabled(false);
    fStart->SetEnabled(false);
    fReset->SetEnabled(false);

    opt.raw_write=false;
    //parpar->Update();
    TGCheckButton *te = (TGCheckButton*) parpar->FindWidget(&opt.raw_write);
    if (te) 
      te->SetEnabled(false);
  }

  TGHorizontalFrame *hfr1 = new TGHorizontalFrame(fGr2);
  fGr2->AddFrame(hfr1, l_But);

  int id;
  id = parpar->Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(hfr1, 0, 0, id,
					   TGNumberFormat::kNESInteger,
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   1,100000);
  parpar->DoMap(fNum1->GetNumberEntry(),&opt.num_buf,p_inum,0);
  fNum1->Resize(65, fNum1->GetDefaultHeight());
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  hfr1->AddFrame(fNum1,lay2);

  fNb = new TGTextButton(hfr1,new TGHotString("&N buf"));
  //fNb->SetFont(tfont,false);
  fNb->Resize(35,22);
  fNb->ChangeOptions(fNb->GetOptions() | kFixedSize);
  fNb->ChangeBackground(fGreen);
  fNb->Connect("Clicked()","MainFrame",this,"DoNbuf()");
  hfr1->AddFrame(fNb, lay2);

  parpar->Update();

  //HiFrm->DoReset();
  //HiFrm->Update();

  //HiFrm->Update();
  fTab->SetTab(opt.seltab);

  TGLayoutHints* fbHints = 
    new TGLayoutHints(kLHintsTop |kLHintsExpandX,0,0,0,0);

  tfont=gClient->GetResourcePool()->GetStatusFont()->GetFontStruct();
  //gClient->GetResourcePool()->GetStatusFont()->Print();
  /*

    font = pool->GetFont("*helvetica*", -11);
    font->Print();
    tfont = 0;

    if (font) {
    tfont = font->GetFontStruct();
    }
  */

  TGLayoutHints* fL11 = new TGLayoutHints(kLHintsLeft,1,1,0,0);
  TGLayoutHints* fL12 = new TGLayoutHints(kLHintsExpandX,1,1,0,0);

  TGHorizontalFrame *fStatFrame1 = new TGHorizontalFrame(this,10,10);
  TGHorizontalFrame *fStatFrame2 = new TGHorizontalFrame(this,10,10);
  AddFrame(fStatFrame1, fbHints);
  AddFrame(fStatFrame2, fbHints);

  const int fwid=120;

  const char* txtlab[n_stat] = {"Start","AcqTime","Events","Ev/sec","Events2","Buffers","MB in","MB/sec","MB out"};

  const char* st_tip[n_stat] = {
    "Acquisition start",
    //"Total Acquisition Time",
    "Acquisition Time",
    "Total number of events received",
    "Event rate received (in Hz)",
    "Total number of events analyzed",
    "Number of buffers received",
    "Megabytes received",
    "Megabytes per second",
    "Megabytes saved"
  };

  TGTextEntry* fLab[n_stat];
  //TGTextEntry* fStat[10];
  for (int i=0;i<n_stat;i++) {
    fLab[i] = new TGTextEntry(fStatFrame1, txtlab[i]);
    fStat[i] = new TGTextEntry(fStatFrame2, " ");
    if (tfont) {
      fLab[i]->SetFont(tfont,false);
      fStat[i]->SetFont(tfont,false);
    }
    fLab[i]->SetHeight(18);
    fLab[i]->SetState(false);
    fLab[i]->ChangeOptions(fLab[i]->GetOptions()|kSunkenFrame|kFixedHeight);
    fLab[i]->SetToolTipText(st_tip[i]);

    fStat[i]->SetHeight(18);
    fStat[i]->SetState(false);
    fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kSunkenFrame|kFixedHeight);
    fStat[i]->SetToolTipText(st_tip[i]);


    if (i==0) {
      fLab[i]->SetWidth(fwid);
      fLab[i]->ChangeOptions(fLab[i]->GetOptions()|kFixedWidth);
      fStatFrame1->AddFrame(fLab[i],fL11);
      fStat[i]->SetWidth(fwid);
      fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kFixedWidth);
      fStatFrame2->AddFrame(fStat[i],fL11);
    }
    else {
      fStatFrame1->AddFrame(fLab[i],fL12);
      fStatFrame2->AddFrame(fStat[i],fL12);
    }

  }
  
  //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
  //UpdateStatus();

  UpdateStatus();

  // Set a name to the main frame
  //SetWindowName(maintitle);

  SetTitle(crs->Fname);
  //SetWMSizeHints(800,600,10000,10000,1,1);

  //Rebuild();
  // Map all subwindows of main frame
  //ChangeOptions(GetOptions()|kFitWidth|kFitHeight);

  MapSubwindows();
  // Initialize the layout algorithm
  Rebuild();
  //Resize(GetDefaultSize());
  // Map main frame
  MapWindow();

  //cout << "Init end" << endl;
  //Rebuild();

  Move(-100,-100);

  //cout << "2222" << endl;
}

MainFrame::~MainFrame() {

  //cout << "end: module: " << crs->module << endl;

  if (crs->b_acq && crs->module) {
    DoStartStop();
    gSystem->Sleep(300);
  }

  if (crs->b_fana) {
    DoAna();
    gSystem->Sleep(300);
  }

  delete crs;
  delete EvtFrm;

  // Clean up used widgets: frames, buttons, layouthints
  //printf("end\n");
  Cleanup();
  //DoExit();
  //delete fMain;
  gApplication->Terminate(0);
}

void MainFrame::Rebuild() {

  //cout << "main::Rebuild: " << endl;

  EvtFrm->Rebuild();
  crspar->Rebuild();
  chanpar->Rebuild();
  Resize(GetDefaultSize());
  Layout();

}

void MainFrame::MakeTabs() {

  int ntab=0;

  //cout << "tab0: " << endl;

  parpar = new ParParDlg(tabfr[0], 600, 500);
  parpar->Update();
  tabfr[0]->AddFrame(parpar, Lay11);
  ntab++;

  //cout << "tab2: " << endl;
  crspar = new CrsParDlg(tabfr[1], 600, 500);
  crspar->Make_crspar(tabfr[1], 600, 210);
  tabfr[1]->AddFrame(crspar, Lay12);
  ntab++;
  crspar->Update();
  //cout << "tab3: " << endl;

  chanpar = new ChanParDlg(tabfr[2], 600, 500);
  chanpar->Make_chanpar(tabfr[2], 600, 210);
  tabfr[2]->AddFrame(chanpar, Lay12);
  ntab++;
  chanpar->Update();

  EvtFrm = new EventFrame(tabfr[3], 600, 500,ntab);
  tabfr[3]->AddFrame(EvtFrm, Lay11);
  ntab++;

  HiFrm = new HistFrame(tabfr[4], 800, 500,ntab);
  HiFrm->HiReset();
  tabfr[4]->AddFrame(HiFrm, Lay11);
  ntab++;

  local_nch=opt.Nchan;

  // MapSubwindows();
  // Resize(GetDefaultSize());
  // MapWindow();
}

void MainFrame::SetTitle(char* fname) {
  strcpy(maintitle,pr_name);
  strcat(maintitle," ");
  strcat(maintitle,fname);
  SetWindowName(maintitle);
}

void MainFrame::DoStartStop() {

  //cout << "Dostartstop: " << gROOT->FindObject("Start") << endl;

#ifdef CYUSB
  if (crs->b_acq) { //STOP is pressed here
    fStart->ChangeBackground(fGreen);
    fStart->SetText("Start");
    //crs->b_stop=false;
    //crs->Show();
    crs->DoStartStop();

    if (opt.root_write) {
      saveroot(opt.fname_root);
    }

  }
  else { // START is pressed here
    if (TestFile()) {
      //ParLock();
      fStart->ChangeBackground(fRed);
      fStart->SetText("Stop");
      crs->DoStartStop();
      //cout << "Start7: " << endl;


      fStart->ChangeBackground(fGreen);
      fStart->SetText("Start");

      crs->b_stop=true;
      crs->b_fana=false;
      crs->b_acq=false;
      crs->b_run=0;

      if (opt.root_write) {
	saveroot(opt.fname_root);
      }

    }
    //crs->b_stop=true;
  }
#endif

}

void MainFrame::DoOpen() {

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "all files",     "*",
    "adcm raw files",     "run*.dat",
    "crs raw files",     "raw*.gz",
    //"crs32 files",     "*.32gz",
    0,               0
  };

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {
    crs->DoFopen(fi.fFilename,1);//1 - read toptions

    parpar->Update();
    crspar->Update();
    chanpar->Update();

  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::DoClose() {
  cout << "DoClose: " << endl;

  if (!crs->b_stop) return;

  if (crs->f_read) {
    gzclose(crs->f_read);
    crs->f_read=0;
  }
  if (crs->Fbuf2) {
    delete[] crs->Fbuf2;
    crs->Fbuf2=0;
    crs->Fbuf=0;
  }

  myM->SetTitle((char*)"");
  crspar->AllEnabled(true);

  parpar->Update();
  crspar->Update();
  chanpar->Update();

}

void MainFrame::DoAna() {

  //cout << "DoAna: " << gROOT->FindObject("Start") << endl;

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    crs->b_fana=false;
    crs->b_stop=true;
    gSystem->Sleep(100);
    //fAna->ChangeBackground(fGreen);
    fAna->ChangeText("&Analyze");
    //fAna->SetText(hAna);

    if (opt.root_write) {
      saveroot(opt.fname_root);
    }

  }
  else { //start analysis
    if (TestFile()) {
      //cout << "hAna: " << hAna->GetString() << " " << hPause->GetString() << endl;
      //fAna->SetText(new TGHotString("&Pause"));
      fAna->ChangeText("P&ause");
      //fAna->ChangeBackground(fRed);
      crs->b_fana=true;
      crs->b_stop=false;

      crs->FAnalyze2(true);

      //fAna->SetText(new TGHotString("&Analyze"));
      fAna->ChangeText("&Analyze");
      //fAna->ChangeBackground(fGreen);
      crs->b_fana=false;
      crs->b_stop=true;

      if (opt.root_write) {
	saveroot(opt.fname_root);
      }

    }
  }

  //cout << "mainframe::doana: " << endl;
  
  //crs->DoFAna();
}

void MainFrame::Do1buf() {

  //cout << "DoNbuf" << endl;

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  if (TestFile()) {
    crs->b_fana=true;
    crs->b_stop=false;
    crs->DoNBuf2(1);
    crs->b_fana=false;
    crs->b_stop=true;
  }  

}

void MainFrame::DoNbuf() {

  //cout << "DoNbuf" << endl;

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    fAna->ChangeBackground(fGreen);
    fAna->SetText("&Analyse");
    fNb->ChangeBackground(fGreen);
    gSystem->Sleep(100);
    crs->b_fana=false;
    crs->b_stop=true;
  }
  else { //start analysis of n buffers
    if (TestFile()) {
      fAna->ChangeBackground(fRed);
      fAna->SetText("P&ause");
      fNb->ChangeBackground(fRed);
      crs->b_fana=true;
      crs->b_stop=false;
      crs->DoNBuf2(opt.num_buf);
      fAna->ChangeBackground(fGreen);
      fAna->SetText("&Analyse");
      fNb->ChangeBackground(fGreen);
      crs->b_fana=false;
      crs->b_stop=true;
    }
  }

}

// void MainFrame::ParLock() {
//   cout << "ParLock: " << endl;
//   crspar->SelectEnabled(false,"pre");
// }

// void MainFrame::ParUnLock() {
//   cout << "ParUnLock: " << endl;
// }

void MainFrame::DoRWinit(EFileDialogMode nn) {

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "par files",     "*.par",
    "All files",     "*",
    0,               0
  };

  char pname[200];

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {
    strcpy(pname,fi.fFilename);
    printf("TGFile: %s\n",pname);

    if (nn==kFDOpen) {
      //readinit(pname);
      gzFile ff = gzopen(pname,"rb");
      if (!ff) {
	cout << "Can't open par file: " << pname << endl;
      }
      else {
	crs->ReadParGz(ff,pname,0,1,1);
	gzclose(ff);
      }

      parpar->Update();
      crspar->Update();
      chanpar->Update();

    }
    else { //Save pars
      Int_t retval=kMBOk;
      if (!stat(fi.fFilename, &statbuffer)) {
	new TGMsgBox(gClient->GetRoot(), this,
		     "File exists",
		     msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
      }

      if (retval == kMBOk) {
	//saveinit(pname);
	gzFile ff = gzopen(pname,"wb");
	if (ff) {
	  crs->SaveParGz(ff);
	  gzclose(ff);
	}
	else {
	  cout << "Can't open file: " << pname << endl;
	}
      }
    } //else (save Pars)
    //newfile();
  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::DoReadRoot() {

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "par files",     "*.root",
    "All files",     "*",
    0,               0
  };

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {

    //rootname=new char[200];

    strcpy(rootname,fi.fFilename);
    //printf("xxxx TGFile: %s\n",rootname);

    readpar_root(rootname);
    //reset();
    //new_hist();

    readroot(rootname);

    parpar->Update();
    crspar->Update();
    chanpar->Update();
    HiFrm->Update();

    //nevent=opt.Nevt;
    //tof=opt.Tof;

    //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
    UpdateStatus();

    //if (fPar!=NULL) {
    //delete fPar;
    //fPar = new MParDlg(gClient->GetRoot(), fMain, "Parameters");
    //fPar->Map();
    //}
    //if (fChan!=NULL) {
    //delete fChan;
    //fChan = new MChanDlg(gClient->GetRoot(), fMain, "Channels");
    //fChan->Map();
    //}

    //DoDraw();

  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::Export() {
  const char *dnd_types[] = {
    "pdf files",     "*.pdf",
    "png files",     "*.png",
    "jpg files",     "*.jpg",
    "gif files",     "*.gif",
    "all files",     "*",
    0,               0
  };

  //TString name = TString(fTab->GetCurrentTab()->GetString());
  //if (name.EqualTo("DAQ",TString::kIgnoreCase))

  TCanvas* cv=0;
  TString name = TString(fTab->GetCurrentTab()->GetString());
  if (name.EqualTo("Events",TString::kIgnoreCase)) {
    cv=EvtFrm->fCanvas->GetCanvas();
  }
  else if (name.EqualTo("Histograms",TString::kIgnoreCase)) {
    cv=HiFrm->fEc->GetCanvas();
  }
  else {
    return;
  }

  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  //fi.fIniDir    = StrDup(".");
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename) {
    Int_t retval=kMBOk;
    if (!stat(fi.fFilename, &statbuffer)) {
      new TGMsgBox(gClient->GetRoot(), this,
		   "File exists",
		   msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    }

    if (retval == kMBOk) {
      cout << "OK: " << fi.fFilename <<  " " << TImage::GetImageFileTypeFromFilename(fi.fFilename) << endl;
      //TGCompositeFrame*	frame = fTab->GetCurrentContainer();
      //frame->SaveAs(fi.fFilename);
      cv->SaveAs(fi.fFilename);
    }
  }
  
}

void MainFrame::DoReset() {

  //printhlist(4);
  /*
    opt.enabl[0]=false;
    int tmp,max;
    opt.GetPar("thresh",crs->module,0,tmp,tmp,max);
    //cout << "Off: " << (int) chan << " " << max << endl;
    crs->Command2(4,0,0,0);
    crs->Command32(2,0,6,0);
    crs->Command32(2,0,7,max);
    crs->Command2(3,0,0,0);

    return;
  */
  
  // if (HiFrm)
  //   cout << "DoReset_main: " << HiFrm->h_time[1]->GetName() << endl;

  //if (HiFrm)
  //cout << "DoReset_main: " << HiFrm->hlist->GetSize() << endl;

  if (!crs->b_stop) return;

  crs->DoReset();

  if (local_nch!=opt.Nchan) {
    Rebuild();

    //Resize(GetDefaultSize());
    //MapSubwindows();
    //MapWindow();  
    //Layout();
    local_nch=opt.Nchan;
  }

  //else {
  HiFrm->HiReset();
  parpar->Update();
  crspar->Update();
  chanpar->Update();
  //}

  UpdateStatus();

}

void MainFrame::UpdateStatus() {

  int ii=0;

  static Long64_t bytes1=0;
  static Long64_t nevents1=0;
  static double t1=0;
  double mb_rate,ev_rate;

  char txt[100];
  //time_t tt = opt.F_start.GetSec();

  //cout << "T_acq2: " << opt.T_acq << " " << crs->Tstart64 << endl;
  // if (opt.Tstop && opt.T_acq>opt.Tstop) {
  //   DoStartStop();
  // }

  time_t tt = (opt.F_start+788907600000)*0.001;
  //struct tm *ptm = localtime(&tt);
  struct tm *ptm = gmtime(&tt);
  strftime(txt,sizeof(txt),"%F %T",ptm);

  double dt = opt.T_acq - t1;

  if (dt>0.1) {
    mb_rate = (crs->totalbytes-bytes1)/MB/dt;
    ev_rate = (crs->nevents-nevents1)/dt;
  }
  else {
    mb_rate=0;
    ev_rate=0;
  }

  bytes1=crs->totalbytes;
  nevents1=crs->nevents;
  t1=opt.T_acq;


  fStat[ii++]->SetText(txt,kFALSE);
  //fStat[ii++]->SetText(TGString::Format("%0.1f",crs->F_acq),1);
  fStat[ii++]->SetText(TGString::Format("%0.1f",opt.T_acq),1);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.3f",ev_rate),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents2),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nbuffers),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->totalbytes/MB),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",mb_rate),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->writtenbytes/MB),kFALSE);

  //cout << txt << endl;
  //return;
  /*
    fBar1->SetText(txt,0);
    fBar1->SetText(TGString::Format("%0.2f",opt.T_acq),1);
    fBar1->SetText(TGString::Format("%lld",crs->nevents),2);
    fBar1->SetText(TGString::Format("%lld",crs->nevents2),3);
    fBar1->SetText(TGString::Format("%lld",crs->npulses),4);
    fBar1->SetText(TGString::Format("%lld",crs->nbuffers),5);
    fBar1->SetText(TGString::Format("%0.2f",crs->totalbytes/MB),6);
    fBar1->SetText(TGString::Format("%0.2f",crs->mb_rate),7);
    fBar1->SetText(TGString::Format("%0.2f",crs->writtenbytes/MB),8);
  */
  //cout << "Updatestatus2: " << endl;

}

// void MainFrame::DoSetNumBuf() {

//   if (bRun) return;

//   opt.num_buf=(int) n_buffers->GetNumber();
//   printf("test %d\n",opt.num_buf);
// }

void MainFrame::DoExit() {
  //int i;
  //double it[4];
  //double sum;

  cout << "DoExit" << endl;

  //saveinit(parname);
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
  //parname = (char*)"romana.par";
  gzFile ff = gzopen(parname,"wb");
  if (ff) {
    crs->SaveParGz(ff);
    gzclose(ff);
  }
  else {
    cout << "Can't open file: " << parname << endl;
  }

  // printf("%lld bytes\n",recd*2);
  // printf("%d events\n",nevent);
  // printf("%d bad events\n",bad_events);

  delete this;
  //gApplication->Terminate(0);
}

void MainFrame::DoSave() {

  //cout << "Dosave: " << endl;
  //cout << "Dosave: " << datfname << endl;

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "root files",     "*.root",
    "all files",      "*",
    0,               0
  };

  //char s_name[100];

  string s_name, dir, name, ext;
  TGFileInfo fi;

  s_name = string();

//   SplitFilename (string(datfname),dir,name,ext);
//   dir.append("Root/");
// #ifdef LINUX
//   mkdir(dir.c_str(),0755);
// #else
//   _mkdir(dir.c_str());
// #endif

  //s_name = dir;
  s_name.append(name);
  s_name.append(".root");

  //strcat(dir,"save/");
  //strcpy(s_name,name);
  //strcat(s_name,".root");

  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir.c_str());
  fi.fFilename  = StrDup(s_name.c_str());

  //TGFileDialog *gfsave = 
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename) {
    Int_t retval=kMBOk;
    if (!stat(fi.fFilename, &statbuffer)) {
      new TGMsgBox(gClient->GetRoot(), this,
		   "File exists",
		   msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    }

    if (retval == kMBOk) {
      saveroot(fi.fFilename);
    }
  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::DoTab(Int_t num) {
  //cout << "DoTab: " << num << endl;
  TGTab *tab = (TGTab*) gTQSender;

  TString name = TString(tab->GetCurrentTab()->GetString());

  opt.seltab = tab->GetCurrent();

  //cout << "dotab: " << tab->GetCurrent() << " " 
  //     << tab->GetCurrentTab()->GetString() << endl;
  //cout << "dotab: " << name.EqualTo("Parameters",TString::kIgnoreCase) << endl;


  if (name.EqualTo("Parameters",TString::kIgnoreCase)) {
    //cout << "DoTab1: " << name << endl;
    parpar->Update();
  }
  else if (name.EqualTo("DAQ",TString::kIgnoreCase)) {
    //cout << "DoTab2: " << name << endl;
    crspar->Update();
  }
  else if (name.EqualTo("Channels",TString::kIgnoreCase)) {
    //cout << "DoTab3: " << name << endl;
    chanpar->Update();
  }
  else if (name.EqualTo("Events",TString::kIgnoreCase)) {
    //cout << "DoTab4: " << name << endl;
    if (crs->b_stop)
      EvtFrm->DrawEvent2();
  }
  else if (name.EqualTo("Histograms",TString::kIgnoreCase)) {
    //cout << "DoTab5: " << name << endl;
    if (!crs->b_acq)
      HiFrm->Update();
    //HiFrm->ReDraw();
  }
}

void MainFrame::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected)
{
  //  Writes the event status in the status bar parts

  //const char *text0, *text1, *text3;
  //char text2[50];
  //char ttt[100];
  int nn;

  double x1,x2,y1,y2;

  TVirtualPad *fp = fEcanvas->GetCanvas()->GetSelectedPad();
  if (fp) {
    nn = fp->GetNumber();
    //TVirtualPad *fc1 = gPad->GetCanvas()->cd(1);;
    //TVirtualPad *fc2 = gPad->GetCanvas()->cd(2);;
    TVirtualPad *fc = gPad->GetCanvas()->cd(nn);

    //cout << fc << " " << fp << endl;

    //fc-> AbsCoordinates(true);
    x1=fc->PixeltoX(px);
    x2=fc->AbsPixeltoX(px);
    y1=fc->PixeltoY(py);
    y2=fc->AbsPixeltoY(py);
    printf("R: %d %d %d %f %f %f %f\n",nn,px,py,x1,y1,x2,y2);
  }

  //gPad->GetRangeAxis(x1,y1,x2,y2);

  //printf("A: %f %f %f %f\n",x1,y1,x2,y2);

  //cout << gPad->GetUxmin() << " " << gPad->GetUxmax() << endl; 

  //gPad-> AbsCoordinates(true);

  //x1=gPad->PixeltoX(px);
  //y1=gPad->PixeltoY(py);

  /*

    text0 = selected->GetTitle();
    //fBar->SetText(text0,0);
    //SetStatusText(text0,0);
    text1 = selected->GetName();
    //SetStatusText(text1,1);
    if (event == kKeyPress)
    sprintf(text2, "%c", (char) px);
    else
    sprintf(text2, "%d :   %d,%d %f %f", nn, px, py, x1,y1);
    //SetStatusText(text2,2);
    text3 = selected->GetObjectInfo(px,py);
    //SetStatusText(text3,3);

    cout << text1 << " " << text2 << " " << text3 << endl;

  */
}

void MainFrame::DoCross() {

  static bool bcross=false;

  bcross = !bcross;

  cout << "bcross: " << bcross << endl;

  if (bcross) {
    fEcanvas->GetCanvas()->
      Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MainFrame",
	      this, "EventInfo(Int_t,Int_t,Int_t,TObject*)");
  }
  else {
    fEcanvas->GetCanvas()->
      Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)");
  }

}

void MainFrame::HandleMenu(Int_t menu_id)
{

  char command[128];
  int status;

  if (!crs->b_stop) return;

  //cout << menu_id << endl;

  // Handle menu events.

  // TRootHelpDialog *hd;
  // static TString dir(".");
  // TGFileInfo fi;
  // fi.fFileTypes = dnd_types;
  // fi.fIniDir    = StrDup(dir);

  switch (menu_id) {

  case M_READINIT:
    DoRWinit(kFDOpen);
    break;
  case M_SAVEINIT:
    DoRWinit(kFDSave);
    break;
  case M_SAVEROOT:
    DoSave();
    break;
  case M_READROOT:
    DoReadRoot();
    break;
  case M_FILE_BROWSE:
    new TBrowser();
    break;
  case M_EXPORT:
    Export();
    break;
    // case M_FILE_NEWCANVAS:
    //   gROOT->MakeDefCanvas();
    //   break;

    //case M_PARAM:
    //if (fPar!=NULL) {
    //delete fPar;
    //}
    //fPar = new MParDlg(gClient->GetRoot(), fMain, "Parameters");
    //fPar->Map();
    //break;

    //case M_CHANNELS:

    //if (fChan==NULL) {
    //fChan = new MChanDlg(gClient->GetRoot(), fMain, "Channels");
    //fChan->Map();
    //}

    //break;

  case M_HELP:

    strcpy(command,"xdg-open ");
    strcat(command,"src/help.pdf");
    status = system( command );
    if (status == -1) {
      cout << "Return value of system(command): " << status << endl;
    }

    break;

  case M_FILE_EXIT:
    DoExit();   // terminate theApp no need to use SendCloseMessage()
    break;
  }
}

bool MainFrame::TestFile() {

  if (!crs->juststarted) return true;

  //EMsgBoxIcon icontype = kMBIconStop;
  //EMsgBoxIcon icontype = kMBIconExclamation;
  //EMsgBoxIcon icontype = kMBIconQuestion;
  //EMsgBoxIcon icontype = kMBIconAsterisk;
  //Int_t buttons = kMBOk|kMBCancel;

  if ((opt.raw_write && !stat(opt.fname_raw, &statbuffer)) ||
      (opt.dec_write && !stat(opt.fname_dec, &statbuffer)) ||
      (opt.root_write && !stat(opt.fname_root, &statbuffer))) {
    Int_t retval;
    //new ColorMsgBox(gClient->GetRoot(), this,
    new TGMsgBox(gClient->GetRoot(), this,
		 "File exists",
		 msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    if (retval == kMBOk) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return true;
  }

}

//______________________________________________________________________________

/*
  void MainFrame::HandleHelp()
  {

  if (!crs->b_stop) return;

  cout << "test" << endl;

  char command[128];

  strcpy(command,"evince ");
  strcat(command,"help.pdf");
  int status = system( command );

  cout << status << endl;

  }
*/
//______________________________________________________________________________

/*
  void example() {
  // Popup the GUI...

  //gSystem->Sleep(1000);
  //HiFrm->Update();
  //myM->Move(-100,-100);
  }
*/

/*
  void dumpbuf(int nn)
  {
  int i;

  unsigned int *ibuf= (unsigned int*) buf;

  //ftmp=fopen("buf.dat","w");
  //printf("%llx \n",(recd-Buffer->r_buf+idx)*2);
  for (i=0;i<nn;i++) {
  printf("dump: %d %d %d %x\n",i,buf[2*i],buf[2*i+1],ibuf[i]);
  }
  //close(ftmp);
  }
*/

//-----------------------------
//#############################################
// END
//
//
//
//
//
//

struct MyClass {
  MyClass() {std::cout <<"MyClass constructed\n";}
  ~MyClass() {std::cout <<"MyClass destroyed\n";}
};

void pointer_test() {
  MyClass * pt;
  int size = 3;

  pt = new MyClass[size];
  delete[] pt;

}
//-------------------------
ColorMsgBox::ColorMsgBox(const TGWindow *p, const TGWindow *main,
			 const char *title, const char *msg, EMsgBoxIcon icon,
			 Int_t buttons, Int_t *ret_code)
  : TGTransientFrame(p, main, 10, 10)
{
  UInt_t width, height;

  Pixel_t fBluevio;
  fBluevio=TColor::RGB2Pixel(255,114,86);

  cout << "ColorBox: " << endl;

  TGLayoutHints* fL1 = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 3, 3, 0, 0);
  TGLayoutHints* fL2 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);
  TGLayoutHints* fL4 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft | kLHintsExpandX,
					 4, 2, 2, 2);
 
  TGHorizontalFrame* fButtonFrame = new TGHorizontalFrame(this, 100, 20, kFixedWidth);
  AddFrame(fButtonFrame, fL2);
  TGVerticalFrame* fLabelFrame = new TGVerticalFrame(this, 60, 20);
  AddFrame(fLabelFrame, fL2);

  TGTextButton* fOK = new TGTextButton(fButtonFrame, new TGHotString("&OK"), 1);
  //fOK->Associate(this);
  fButtonFrame->AddFrame(fOK, fL1);
  //width = TMath::Max(width, fOK->GetDefaultWidth());
  TGTextButton* fCancel = new TGTextButton(fButtonFrame, new TGHotString("&Cancel"), 2);
  //fCancel->Associate(this);
  fButtonFrame->AddFrame(fCancel, fL1);
  //    width = TMath::Max(width, fCancel->GetDefaultWidth()); ++nb;

  //width = TMath::Max(width, fOK->GetDefaultWidth());


  TGLabel *label;
  label = new TGLabel(fLabelFrame, msg);
  label->SetTextJustify(kTextCenterX);

  this->SetBackgroundColor(fBluevio);

  fLabelFrame->AddFrame(label, fL4);

  MapSubwindows();

  width  = GetDefaultWidth();
  height = GetDefaultHeight();

  Resize(width, height);

  // position relative to the parent's window

  CenterOnParent();

  // make the message box non-resizable

  SetWMSize(width, height);
  SetWMSizeHints(width, height, width, height, 0, 0);

  // set names

  SetWindowName(title);
  SetIconName(title);
  SetClassHints("MsgBox", "MsgBox");

  SetMWMHints(kMWMDecorAll | kMWMDecorResizeH  | kMWMDecorMaximize |
	      kMWMDecorMinimize | kMWMDecorMenu,
	      kMWMFuncAll  | kMWMFuncResize    | kMWMFuncMaximize |
	      kMWMFuncMinimize,
	      kMWMInputModeless);

  MapRaised();

  fClient->WaitFor(this);
}
//-------------------------
ColorMsgBox::~ColorMsgBox()
{
}


//------------------------------

TGMatrixLayout2::TGMatrixLayout2(TGCompositeFrame *main, UInt_t r, UInt_t c,
				 Int_t s, Int_t h)
{
  // TGMatrixLayout2 constructor.

  fMain    = main;
  fList    = fMain->GetList();
  fSep     = s;
  fHints   = h;
  fRows    = r;
  fColumns = c;
}

//______________________________________________________________________________
void TGMatrixLayout2::Layout()
{
  // Make a matrix layout of all frames in the list.
  //cout << "MatrixLayout1: " << endl;


  TGFrameElement *ptr;
  TGDimension csize, maxsize(0,0);
  Int_t bw = fMain->GetBorderWidth();
  Int_t x = fSep+2, y = fSep + bw;
  UInt_t rowcount = fRows, colcount = fColumns;
  fModified = kFALSE;

  TIter next(fList);
  while ((ptr = (TGFrameElement *) next())) {
    if (ptr->fState & kIsVisible) {
      csize = ptr->fFrame->GetDefaultSize();
      maxsize.fWidth  = TMath::Max(maxsize.fWidth, csize.fWidth);
      maxsize.fHeight = TMath::Max(maxsize.fHeight, csize.fHeight);
    }
  }

  next.Reset();
  int nn=0;
  while ((ptr = (TGFrameElement *) next())) {
    //cout << "Layout11: " << nn << " " << ptr->fState << " " << kIsVisible << endl;
    if (ptr->fState & kIsVisible) {
      nn++;
      //cout << "Layout12: " << nn << " " << x << " " << y << endl;
      //ptr->fFrame->Move(x, y);
      ptr->fFrame->MoveResize(x, y, csize.fWidth, csize.fHeight);
      fModified = fModified || (ptr->fFrame->GetX() != x) || 
	(ptr->fFrame->GetY() != y);

      ptr->fFrame->Layout();

      if (fColumns == 0) {
	y += maxsize.fHeight + fSep;
	rowcount--;
	if (rowcount <= 0) {
	  rowcount = fRows;
	  y = fSep + bw; x += maxsize.fWidth + fSep;
	}
      } else if (fRows == 0) {
	x += maxsize.fWidth + fSep;
	colcount--;
	if (colcount <= 0) {
	  colcount = fColumns;
	  x = fSep; y += maxsize.fHeight + fSep;
	}
      } else {
	x += maxsize.fWidth + fSep;
	colcount--;
	if (colcount <= 0) {
	  rowcount--;
	  if (rowcount <= 0) return;
	  else {
	    colcount = fColumns;
	    x = fSep; y += maxsize.fHeight + fSep;
	  }
	}
      }
    }
  }
  //cout << "MatrixLayout2: " << nn << endl;

}

//______________________________________________________________________________
TGDimension TGMatrixLayout2::GetDefaultSize() const
{
  // Return default dimension of the matrix layout.


  TGFrameElement *ptr;
  TGDimension     size, csize, maxsize(0,0);
  Int_t           count = 0;
  Int_t           bw = fMain->GetBorderWidth();

  TIter next(fList);
  while ((ptr = (TGFrameElement *) next())) {
    if (ptr->fState & kIsVisible) {
      count++;
      csize = ptr->fFrame->GetDefaultSize();
      maxsize.fWidth  = TMath::Max(maxsize.fWidth, csize.fWidth);
      maxsize.fHeight = TMath::Max(maxsize.fHeight, csize.fHeight);
    }
  }
  Int_t rows=0;
  Int_t cols=0;

  if (fRows == 0) {
    rows = (count % fColumns) ? (count / fColumns + 1) : (count / fColumns);
    size.fWidth  = fColumns * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = rows * (maxsize.fHeight + fSep) + fSep + bw;
  } else if (fColumns == 0) {
    cols = (count % fRows) ? (count / fRows + 1) : (count / fRows);
    size.fWidth  = cols * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  } else {
    size.fWidth  = fColumns * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  }

  size.fWidth+=5;
  size.fHeight+=10;
  // int sz2 = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  // cout << "GetDefaultSize: " << size.fWidth << " " << size.fHeight
  //      << " " << count << " " << rows << " " << cols
  //      << " " << sz2 << " " << fSep << " " << bw << endl;
  return size;
}
// __________________________________________________________________________
void TGMatrixLayout2::SavePrimitive(ostream &out, Option_t *)
{

  // Save matrix layout manager as a C++ statement(s) on output stream

  out << "new TGMatrixLayout2(" << fMain->GetName() << ","
      << fRows << ","
      << fColumns << ","
      << fSep << ","
      << fHints <<")";

}
