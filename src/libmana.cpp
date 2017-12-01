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

#include "TGMsgBox.h"

//#include <TGColorDialog.h>

#include "libcrs.h"

const double MB = 1024*1024;

extern CRS* crs;
int chanPresent;

EventFrame* EvtFrm;
HistFrame* HiFrm;

ParParDlg *parpar;
CrsParDlg *crspar;
ChanParDlg *chanpar;

const int maxsamp = 16500;// константу 16500 надо будет заменить на переменную

//extern const char* parname;
char* parname;

//FILE* fp=0;
//gzFile fp=0;

char fname[180]="";
char rootname[180]="";

char pr_name[180];
char maintitle[180];

char startdir[180];

const char* ng_name[] = {"gamma","neutrons","tail","other","pileup"};

int idx,idnext;
Long64_t recd;
int nevent,bad_events;
int syncw,type,len,len2,dlen2;

//event:
short int nbits; //ADC bits
short int nw; //samples per word = 2
short int lflag; //last fragment flag
//short int NSamp0; // number of samples in the fragment
short int NSamp; // number of samples recorded in the channel
short int Nch; // channel number
short int id; // block id (=0,1,2)
int evcnt; // event count
int fr_num; //fragment number
int burstcnt; //burst count
int evprev; // previous event count
unsigned int tst=0; // temporary time stamp
int cnst=0; //temporary constant (64 bit timestamp)
int cnst_prev=0; //previous temporary constant (64 bit timestamp)
unsigned int tstamp=0; // time stamp
unsigned int pstamp=0; // time stamp of the previous event
unsigned int istamp=0; // number of tstamp overflows
Long64_t offset64; //istamp offset
Long64_t first64; //first timestamp (zero offset)
Long64_t tstamp64; //64-bit timestamp (corrected for overflows)
Long64_t pstamp64; //64-bit timestamp of the previous event
Long64_t peak64; //64-bit peak time
Long64_t last_peak64[MAX_CH]; //64-bit last peak time (from previous event)
Long64_t start64; // timestamp of nuclotron starts 
//unsigned int tstart=0; // time start (from ch 15)

double tof,tof2,tof3,frame_tof,frame_tof2;
int png1 = -99;
unsigned int pstop1=0;

Long64_t t_ring[MAXRING];
Long64_t t_prev;
unsigned int nring;

//unsigned short data[DSIZE];
int mult=0;
int chan[MAX_CH];
bool new_event=false;
bool beam_on=false;
unsigned short Event[MAX_CH][DSIZE];
double bEvent[MAX_CH][DSIZE]; // background subtracted
double sEvent[MAX_CH][DSIZE]; // smoothed
//double sDeriv[MAX_CH][DSIZE]; // derivative smoothed
int findbeam=0;
int checkpoint=0;

int npeaks[MAX_CH];
int peaks[MAX_CH][DSIZE];
int peaks1[MAX_CH][DSIZE];
int peaks2[MAX_CH][DSIZE];
int peaks0[MAX_CH][DSIZE];
int peak_flag[MAX_CH][DSIZE]; 
// 0 -undetermined; 2 - gamma; 3 - neutron; 4 - tail; 5 - unknown; 6 - NIM;
// 11 - bad "left"; 12 - bad "right"; 13 - true pileup; 14 - false pileup

double evmax[MAX_CH];

double sum[MAX_CH][DSIZE];
double mean[MAX_CH][DSIZE];
double rms[MAX_CH][DSIZE];

int tpeaks;
double *tstarts;
int *t_flag;

event_t evt;

int mult_gam;
//#endif

//char hstnam[100];

MyMainFrame *myM;

Coptions cpar;
Toptions opt;
int debug=0; //=1// for printing debug messages

int *opt_id[MXNUM];

//-------------------------------------
UShort_t ClassToBuf(const char* name, char* var, char* buf) {
  //copies all data members to a buffer, returns size of the buffer
  //buffer should exist and have sufficient size to allocate all data

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

  if (debug&0x2)
    cout << "Save class: " << name << endl;

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
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
	cout << dm->GetName() << " " << len << endl;
    }
  }

  return sz;

}

//------------------------------------

void BufToClass(const char* name, char* var, char* buf, int size) {
  //copies all data members from a buffer, size - size of the buffer
  //buffer should exist. Only data members with matching names are copied

  //cout <<"BufToClass::" << endl;

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
  char clname[100];
  const UShort_t mx=5000;
  char data[mx];
  while (sz<size) {
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>=size) break;
    memcpy(memname,buf+sz,len);
    sz+=len;
    if (sz>=size) break;
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>=size) break;
    memcpy(data,buf+sz,len);
    sz+=len;
    if (sz>=size) break;

    if (strcmp(memname,"class")==0) {
      strcpy(clname,data);
      if (debug&0x2)
	cout << "Read class: " << clname << endl;
      continue;
    }

    dm = (TDataMember*) lst->FindObject(memname);
    if (dm && strcmp(clname,name)==0) {
      len2=dm->GetUnitSize();
      for (int i=0;i<dm->GetArrayDim();i++) {
	len2*=dm->GetMaxIndex(i);
      }
      if (debug&0x2)
	cout << dm->GetName() << " " << len << " " << len2 << endl;
      memcpy(var+dm->GetOffset(),data,TMath::Min(len,len2));
    }
    else {
      //cout << "member not found: " << dm << " " << memname << " " 
      //   << clname << " " << name << endl;
    }

  }

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
  //exit(1); 
}

void segfault_c_handler(int signal, siginfo_t *si, void *arg) {
  printf("Caught segfault %d\n",signal);
  delete myM;
  //exit(1); 
}

/*
void change_gz_file(char* name1, char* name2) {

  //exit(-1);

  gzFile f1 = gzopen(name1,"rb");
  gzFile f2 = gzopen(name2,"wb");

  char buf[1000000];
  Version_t ver;
  UShort_t mod[2];
  
  cout << name1 << " " << name2 << " " << f1 << " " << f2 << endl;

  if (!f1 || !f2) {
    cout << "can't open file" << endl;
    exit(-1);
  }
  
  gzread(f1,mod,sizeof(mod));
  gzread(f1,buf,mod[1]);
  gzread(f1,&ver,sizeof(ver));
  
  gzwrite(f2,mod,sizeof(mod));
  gzwrite(f2,&ver,sizeof(ver));
  gzwrite(f2,buf,mod[1]);

  
  int res=1;
  do {
    res = gzread(f1,buf,sizeof(buf));
    gzwrite(f2,buf,sizeof(buf));
  } while (res>0);

  gzclose(f1);
  gzclose(f2);

  exit(-1);

}
//----------------------------
void change_gz_file2(char* name1, char* name2) {

  gzFile f1 = gzopen(name1,"rb");
  gzFile f2 = gzopen(name2,"wb");

  char buf[1000000];
  Version_t ver;
  UShort_t mod2[2];
  UShort_t mod[8];

  cout << name1 << " " << name2 << " " << f1 << " " << f2 << endl;

  if (!f1 || !f2) {
    cout << "can't open file" << endl;
    exit(-1);
  }
  
  gzread(f1,mod2,sizeof(mod2));
  gzread(f1,&ver,sizeof(ver));
  gzread(f1,&opt,mod2[1]);

  memcpy(&cpar.smooth,   &opt.smooth,   sizeof(cpar.smooth));
  memcpy(&cpar.deadTime, &opt.deadTime, sizeof(cpar.deadTime));
  memcpy(&cpar.preWr,    &opt.preWr,    sizeof(cpar.preWr));
  memcpy(&cpar.durWr,    &opt.durWr,    sizeof(cpar.durWr));
  memcpy(&cpar.kderiv,   &opt.kderiv,   sizeof(cpar.kderiv));
  memcpy(&cpar.threshold,&opt.threshold,sizeof(cpar.threshold));
  memcpy(&cpar.adcGain,  &opt.adcGain,  sizeof(cpar.adcGain));
  memcpy(&cpar.acdc,     &opt.acdc,     sizeof(cpar.acdc));
  memcpy(&cpar.inv,      &opt.inv,      sizeof(cpar.inv));
  memcpy(&cpar.forcewr,  &opt.forcewr,  sizeof(cpar.forcewr));
  memcpy(&cpar.enabl,    &opt.enabl,    sizeof(cpar.enabl));
  memcpy(&cpar.chtype,   &opt.chtype,   sizeof(cpar.chtype));

  cout << "ver: " << ver << " " << opt.durWr[0] << endl;
  cout << "ver2: " << ver << " " << cpar.durWr[0] << endl;


  memset(mod,0,sizeof(mod));
  mod[0]=mod2[0];
  mod[1]=sizeof(cpar);
  mod[2]=TClass::GetClass("Coptions")->GetClassVersion();
  mod[3]=sizeof(opt);
  mod[4]=TClass::GetClass("Toptions")->GetClassVersion();


  cout << sizeof(Coptions) << " "
       << mod[0] << " "
       << mod[1] << " "
       << mod[2] << " "
       << mod[3] << " "
       << mod[4] << endl;

  gzwrite(f2,mod,sizeof(mod));
  gzwrite(f2,&cpar,sizeof(cpar));
  gzwrite(f2,&opt,sizeof(opt));

  
  int res=1;
  do {
    res = gzread(f1,buf,sizeof(buf));
    gzwrite(f2,buf,sizeof(buf));
  } while (res>0);

  gzclose(f1);
  gzclose(f2);

  exit(-1);

}
*/

int main(int argc, char **argv)
{

  /*
  TList* lst = TClass::GetClass("Coptions")->GetListOfDataMembers();
  //lst->ls();

  char buf[100000];

  Coptions* par1 = new Coptions();
  Coptions* par2 = new Coptions();
  Toptions* opt1 = new Toptions();
  Toptions* opt2 = new Toptions();

  par2->adcGain[11]=7;
  opt2->channels[11]=255;

  cout << "-------------" << endl;
  cout << memcmp(par1,par2,sizeof(*par1)) << " " << sizeof(*par1) << endl;
  cout << ClassToBuf("Coptions",(char*) par1, buf) << endl;
  BufToClass("Coptions",(char*) par2, buf, sizeof(buf));
  cout << memcmp(par1,par2,sizeof(*par1)) << " " << sizeof(*par1) << endl;
  cout << "-------------" << endl;

  exit(1);

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
    if (dm->GetDataType()) {
      cout << dm->GetName() << " " << dm->GetOffset() << " " 
	   << strlen(dm->GetName()) << " " 
	   << dm->GetDataType()->Size() << " " 
	   << dm->GetDataType()->Size()
	   << endl;
      //dm->Dump();
      //dm->Inspect();
    }
  }

  dm = (TDataMember*) lst->FindObject("acdc");
  void* pp = &cpar;
  pp+=dm->GetOffset();
  cout << pp << " " << dm->GetArrayDim() << " " << dm->GetMaxIndex(1) << endl;

  cout << &cpar << " " << &cpar.acdc << " " << endl;

  exit(1);
  
  */

  //cpar.ver=TClass::GetClass("Coptions")->GetClassVersion();

  // if (argc<3) {
  //   cout << "need two arguments" << endl;
  //   return 1;
  // }
  // change_gz_file2(argv[1],argv[2]);


  char s_name[200], dir[100], name[100], ext[100];
  //char* pname = (char*) parname;

  bool batch=false;

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












  cout << "sizeof(TDatime): " << sizeof(TDatime) << endl;
  cout << "sizeof(Toptions): " << sizeof(Toptions) << endl;
  cout << "sizeof(opt): " << sizeof(opt) << endl;
  

  crs = new CRS();
  crs->Detect_device();

  parname = (char*)"romana.par";
  gzFile ff = gzopen(parname,"rb");
  crs->ReadParGz(ff,parname,0,1,1);
  gzclose(ff);

#ifdef LINUX
  if (getcwd(startdir,100)) {}
#else
  _getcwd(startdir,100);
#endif
  //cout << "startdir: " << startdir << endl;

  strcpy(pr_name,argv[0]);

  strcpy(maintitle,pr_name);

  int argnn=1;

  //strcpy(fname," ");
  if (argc > 1) {

    if (!strcmp(argv[argnn],"run")) {
      batch=true;
      argnn++;
      //parname= (char*) "init.par";
    }

    //    cout << "argnn: " << argnn << " " << argc << endl;

    if (argnn<argc) {
      crs->DoFopen(argv[argnn],0);

      //strcat(maintitle," ");
      //strcat(maintitle,argv[argnn]);

      /*
      strcpy(crs->Fname,argv[argnn]);
      
      strcpy(fname,argv[argnn]);
      printf("%s\n",fname);
      */
      argnn++;
    }

    if (argnn<argc) {
      //parname = argv[argnn];
      gzFile ff = gzopen(argv[argnn],"rb");
      crs->ReadParGz(ff,argv[argnn],0,1,1);
      gzclose(ff);
      crs->DoReset();
      //pname=argv[argnn];
    }

  }

  greset();
  int nf = 0;//YK - ?? Buffer->NewFile();

  if (batch && nf) {
    return -1;
  }
  else if (batch) {

    //readinit(pname);
    //YK - ????
    //gzFile ff = gzopen(pname,"rb");
    //crs->ReadParGz(ff,1,1);
    //gzclose(ff);
    //YK - ????

    //opt.num_events=0;
    allevents();

    SplitFilename (string(fname),dir,name,ext);
    strcat(dir,"save/");
#ifdef LINUX
    mkdir(dir,0755);
#else
    _mkdir(dir);
#endif
    strcpy(s_name,dir);
    strcat(s_name,name);
    strcat(s_name,".root");
    cout << s_name << endl;
    saveroot(s_name);

    return 0;
  }

  TApplication theApp("App",&argc,argv);
  example();

  //gSystem->Sleep(100);
  //crs->Dummy_trd();

  //EvtFrm->StartThread();
  //gClient->GetColorByName("yellow", yellow);
  theApp.Run();
  //return 0;

  //fclose(fp);
  //gzclose(fp);

  //printf("%d buffers\n",nbuf);
  printf("%lld bytes\n",recd*2);
  printf("%d events\n",nevent);
  printf("%d bad events\n",bad_events);

  //HRPUT(0,(char*)"spectra.hbook",(char*)" ");

  //memcpy(buf2,buf[0],BSIZE);

}

void SplitFilename (string str, char *folder, char *name)
{
  size_t found;
  //cout << "Splitting: " << str << endl;
  found=str.find_last_of("/\\");
  //cout << " folder: " << str.substr(0,found) << endl;
  //cout << " file: " << str.substr(found+1) << endl;
  //printf("%s %s\n",str.substr(0,found+1).c_str(),str.substr(found+1).c_str());

  strcpy(folder,str.substr(0,found+1).c_str());
  strcpy(name,str.substr(found+1).c_str());

}

void SplitFilename (string str, char *folder, char *name, char* ext)
{

  SplitFilename(str, folder, name);
  string fname(name);
  size_t found = fname.find_last_of(".");
  strcpy(name,fname.substr(0,found).c_str());
  strcpy(ext,fname.substr(found+1).c_str());
}

void saveroot(char *name) {

  TFile * tf = new TFile(name,"RECREATE");

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
	//printf("saveroot: %d %s\n",col,h->GetName());
	if (col==0) {
	  h->SetLineColor(50);
	}
	//tf->WriteTObject(obj);
	h->Write();
	h->SetLineColor(col);
      }
    }
  }

  opt.Nevt=nevent;
  opt.Tof=tof;
  cpar.Write();
  opt.Write();

  cout << "Histograms and parameters are saved in file: " << name << endl;

  tf->Close();
}

void readroot(char *name) {

  //char nam[100];

  //cout << opt.channels[0] << endl;

  gROOT->cd();
  TList *list = gDirectory->GetList();

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
	  //cout << obj2 << endl;
	  TDirectory* dir = obj2->GetDirectory();
	  obj->Copy(*obj2);
	  obj->Delete();
	  obj2->SetDirectory(dir);
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

void greset() { //global reset
  //int i,j;

  //YK new_hist();
  //YK set_hist_attr();

  //buf=buf2+BOFFSET;
  //signed_buf=(short*) buf;

  //if (buf) delete[] buf;
  //buf = new unsigned short[opt.rBSIZE];

  //nEvents=0;

  beam_on=false;
  //nbuf=0;
  recd=0;
  nevent=-1;
  //nbuf=0;
  evprev=-9;
  //  devent=0;
  bad_events=0;
  idx=0;
  //r_buf=0;
  istamp=0;
  tstamp=0;
  pstamp=0;
  cnst_prev=0;

  tof=0;
  //opt.Tof=0;
  new_event=false;
  //dstart=-999;
  start64=(Long64_t) (-100*1e8);
  first64=0;
  offset64=0;
  //pdstart=-999;
  nring=0;
  memset(t_ring,0,sizeof(t_ring));

  for (int i=0;i<MAX_CH;i++)
    last_peak64[i]=0;

  //printf("reset: %d %d\n",tstamp,pstamp);
}

void readpar_root(const char* pname)
{
  TFile *f2 = new TFile(pname,"READ");

  cpar.Read("Coptions");
  opt.Read("Toptions");

  greset();

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

void fill_sEvent(int i)
{
  evmax[i]=0;

  for (int j=0;j<NSamp;j++) {
    if (Event[i][j]>evmax[i]) {
      evmax[i]=Event[i][j];
    }
    sEvent[i][j]=Event[i][j];
  }

}

void smooth(int n, int i)
{
  int k,ll;

  evmax[i]=0;
  memset(sEvent[i],0,NSamp*sizeof(double));

  for (int j=0;j<NSamp;j++) {
    if (Event[i][j]>evmax[i]) {
      evmax[i]=Event[i][j];
    }
    ll=0;
    for (int j1=-n;j1<=n;j1++) {
      k=j+j1;
      if (k>=0 && k<NSamp) {
	sEvent[i][j]+=Event[i][k];
	ll++;
      }
    }
    sEvent[i][j]/=ll;

    //if (j>0) {
    //sDeriv[i][j]=sEvent[i][j]-sEvent[i][j-1];
    //}
    //sEvent[i][j]-=bkgr[i];
  }

  /*
    for (j=0;j<nsamp-1;j++) {
    bEvent[i][j]=sEvent[i][j+1]-sEvent[i][j];
    }
  */
}

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

  bRun = false;

  fMenuBar = new TGMenuBar(this, 35, 50, kHorizontalFrame);

  fMenuFile = new TGPopupMenu(gClient->GetRoot());

  fMenuFile->AddEntry("Read Parameters", M_READINIT);
  fMenuFile->AddEntry("Save Parameters", M_SAVEINIT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Read ROOT file", M_READROOT);
  fMenuFile->AddEntry("Save ROOT file", M_SAVEROOT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Browser\tCtrl+B", M_FILE_BROWSE);
  fMenuFile->AddEntry("New Canvas\tCtrl+N", M_FILE_NEWCANVAS);

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
  font->Print();

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



  TGTextButton *fOpen = new TGTextButton(fGr2,"Open");
  fOpen->SetFont(tfont,false);
  fOpen->Resize(butx,buty);
  fOpen->ChangeOptions(fOpen->GetOptions() | kFixedSize);
  fOpen->ChangeBackground(fBluevio);
  fOpen->Connect("Clicked()","MainFrame",this,"DoOpen()");
  fGr2->AddFrame(fOpen, l_But);

  TGTextButton *fReset2 = new TGTextButton(fGr2,"Reset");
  fReset2->SetFont(tfont,false);
  fReset2->Resize(butx,buty);
  fReset2->ChangeOptions(fReset2->GetOptions() | kFixedSize);
  fReset2->ChangeBackground(fCyan);
  fReset2->Connect("Clicked()","MainFrame",this,"DoReset()");
  //fReset2->Connect("Clicked()","CRS",crs,"Reset()");
  fGr2->AddFrame(fReset2, l_But);

  fAna = new TGTextButton(fGr2,"Analyze");
  fAna->SetFont(tfont,false);
  fAna->Resize(butx,buty);
  fAna->ChangeOptions(fAna->GetOptions() | kFixedSize);
  fAna->ChangeBackground(fGreen);
  fAna->Connect("Clicked()","MainFrame",this,"DoAna()");
  fGr2->AddFrame(fAna, l_But);

  TGTextButton* f1b = new TGTextButton(fGr2,"1 buf");
  f1b->SetFont(tfont,false);
  f1b->Resize(butx,buty);
  f1b->ChangeOptions(f1b->GetOptions() | kFixedSize);
  f1b->ChangeBackground(fGreen);
  f1b->Connect("Clicked()","MainFrame",this,"Do1buf()");
  fGr2->AddFrame(f1b, l_But);

  //exit(-1);

  //crs = new CRS();
  //crs->Detect_device();


  //opt.threshold[0]=50;
  //opt.threshold[1]=50;

  //cout << "module: " << crs->module << " " << chanPresent << endl;
  //exit(-1);

  // if (crs->module==2) parname=(char*)"crs2.par";
  // else if (crs->module==32) parname=(char*)"crs32.par";
  // else parname=(char*)"romana.par";

  /*
  parname = (char*)"romana.par";
  //readinit(parname);
  //cout << "1 ReadParGz(ff,1,1);" << endl;
  gzFile ff = gzopen(parname,"rb");
  crs->ReadParGz(ff,parname,0,1,1);
  //cout << "2 ReadParGz(ff,1,1);" << endl;
  gzclose(ff);
  */




  fTab = new TGTab(hframe1, 300, 300);
  //TGLayoutHints *fL5 = new TGLayoutHints(kLHintsTop | kLHintsExpandX |
  //                                        kLHintsExpandY, 2, 2, 5, 1);
  hframe1->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX |
					    kLHintsExpandY,3,3,2,2));

  fTab->Connect("Selected(Int_t)", "MainFrame", this, "DoTab(Int_t)");

  int ntab=0;

  TGCompositeFrame *tab1 = fTab->AddTab("Parameters");
  TGCompositeFrame* fr1 = new TGCompositeFrame(tab1, 10, 10, kHorizontalFrame);
  tab1->AddFrame(fr1, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,2,2));
  parpar = new ParParDlg(fr1, 600, 500);
  //parpar->Update();
  fr1->AddFrame(parpar, 
		new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  ntab++;

  TGCompositeFrame *tab2 = fTab->AddTab("DAQ");
  TGCompositeFrame* fr2 = new TGCompositeFrame(tab2, 10, 10, kHorizontalFrame);
  tab2->AddFrame(fr2, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,2,2));
  crspar = new CrsParDlg(fr2, 600, 500);
  crspar->Make_crspar(fr2, 600, 210);
  fr2->AddFrame(crspar,
		new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  ntab++;

  //crspar->Update();

  //cout << "threshold0: " << opt.threshold[0] << endl;
  
  TGCompositeFrame *tab3 = fTab->AddTab("Channels");
  TGCompositeFrame* fr3 = new TGCompositeFrame(tab3, 10, 10, kHorizontalFrame);
  tab3->AddFrame(fr3, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,2,2));

  chanpar = new ChanParDlg(fr3, 600, 500);
  chanpar->Make_chanpar(fr3, 600, 210);
  fr3->AddFrame(chanpar, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  ntab++;


  TGCompositeFrame *tab4 = fTab->AddTab("Events");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");
  EvtFrm = new EventFrame(tab4, 600, 500,ntab);
  tab4->AddFrame(EvtFrm, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  ntab++;


  TGCompositeFrame *tab5 = fTab->AddTab("Histograms");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");
  HiFrm = new HistFrame(tab5, 800, 500,ntab);
  tab5->AddFrame(HiFrm, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  ntab++;


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

  fNb = new TGTextButton(hfr1,"N buf");
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

  const int fwid=160;

  const char* txtlab[n_stat] = {"Start","Time","Events","Ev/sec","Events2","Buffers","MB in","MB/sec","MB out"};

  const char* st_tip[n_stat] = {
    "Acquisition start",
    "Acquisition/Analysis Time",
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

  // Map all subwindows of main frame
  MapSubwindows();
  // Initialize the layout algorithm
  Resize(GetDefaultSize());
  // Map main frame
  MapWindow();

  Move(-100,-100);

  for (int j=0;j<8;j++) {
    fLeg[j] = new TLegend(0.70,0.80,0.99,0.99);
    //sprintf(txt,"dum%d",j);
    //fHist[j] = new TH2F(txt,"",1000,0.,25.,100.,0.,25.);
    //fHist[j]->SetStats(kFALSE);
    //fGr[j] = new TGraph(nn,xx,yy);
    //fHS[j] = new THStack(txt,"");
  }
  //fHS[2]->SetBit(TH1::kCanRebin);

  /*
    fHS[0]->SetTitle("Amplitude");
    fHS[1]->SetTitle("Width_Energy");
    fHS[2]->SetTitle("Long Time");
    fHS[3]->SetTitle("Energy");
    fHS[4]->SetTitle("Width");
    fHS[5]->SetTitle("Time");
  */


  //hlist = new TList();
  //Make_hist();

  //DoDraw2();
  //crs->Dummy_trd();

}

MainFrame::~MainFrame() {

  //cout << "end: module: " << crs->module << endl;

  if (crs->b_acq && crs->module) {
    crs->DoStartStop();
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

void MainFrame::SetTitle(char* fname) {
  strcpy(maintitle,pr_name);
  strcat(maintitle," ");
  strcat(maintitle,fname);
  SetWindowName(maintitle);
}

void MainFrame::DoStartStop() {

  //cout << gROOT->FindObject("Start") << endl;

  if (crs->b_acq) {
    fStart->ChangeBackground(fGreen);
    fStart->SetText("Start");
    //crs->b_stop=false;
    //crs->Show();
    crs->DoStartStop();
  }
  else {
    if (TestFile()) {
      fStart->ChangeBackground(fRed);
      fStart->SetText("Stop");
      crs->DoStartStop();
    }
    //crs->b_stop=true;
  }

}

void MainFrame::DoOpen() {

  if (bRun) return;

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

  //int fType;
  //cout << "fstart" << endl;
  //new MFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
  //cout << "fstop" << endl;

  if (fi.fFilename != NULL) {
    //printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
    //dir = fi.fIniDir;

    crs->DoFopen(fi.fFilename,1);//1 - read toptions

    parpar->Update();
    crspar->Update();
    chanpar->Update();

    /*
    strcpy(fname,fi.fFilename);
    printf("TGFile: %s\n",fname);

    strcpy(maintitle,pr_name);
    strcat(maintitle," ");
    strcat(maintitle,fname);

    SetWindowName(maintitle);
    cnst_prev=0;
    Buffer->NewFile();
    fBar->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
    UpdateStatus();
    */

  }

}
/*
void MainFrame::DoAna() {

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  //cout << "DoAna" << endl;

  if (crs->b_fana) {
    fAna->ChangeBackground(fGreen);
    fAna->SetText("Analyse");
    crs->b_fana=false;
  }
  else {
    fAna->ChangeBackground(fRed);
    fAna->SetText("Stop");
    crs->b_fana=true;
    crs->FAnalyze();
  }

  //cout << "mainframe::doana: " << endl;
  
  //crs->DoFAna();
}
*/
void MainFrame::DoAna() {

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    fAna->ChangeBackground(fGreen);
    fAna->SetText("Analyse");
    gSystem->Sleep(100);
    crs->b_fana=false;
    crs->b_stop=true;
  }
  else { //start analysis
    if (TestFile()) {
      fAna->ChangeBackground(fRed);
      fAna->SetText("Pause");
      crs->b_fana=true;
      crs->b_stop=false;

      crs->FAnalyze();

      fAna->ChangeBackground(fGreen);
      fAna->SetText("Analyse");
      crs->b_fana=false;
      crs->b_stop=true;
    }
  }

  //cout << "mainframe::doana: " << endl;
  
  //crs->DoFAna();
}

/*
void MainFrame::Do1buf() {

  if (crs->b_stop) {
    //cout << "Do1buf" << endl;
    crs->Do1Buf();
    //crs->b_stop=false;
    crs->Show();
    EvtFrm->fCanvas->GetCanvas()->SetEditable(true);
    //crs->b_stop=true;
  }

}
*/

void MainFrame::Do1buf() {

  //cout << "DoNbuf" << endl;

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  crs->b_fana=true;
  crs->b_stop=false;
  crs->DoNBuf(1);
  crs->b_fana=false;
  crs->b_stop=true;
    
}

void MainFrame::DoNbuf() {

  //cout << "DoNbuf" << endl;

  if (!crs->f_read) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    fAna->ChangeBackground(fGreen);
    fAna->SetText("Analyse");
    fNb->ChangeBackground(fGreen);
    gSystem->Sleep(100);
    crs->b_fana=false;
    crs->b_stop=true;
  }
  else { //start analysis of n buffers
    //cout << "donbuf1" << endl;
    fAna->ChangeBackground(fRed);
    fAna->SetText("Pause");
    fNb->ChangeBackground(fRed);
    crs->b_fana=true;
    crs->b_stop=false;
    crs->DoNBuf(opt.num_buf);
    //gSystem->Sleep(1000);
    //cout << "donbuf2" << endl;
    //DoNbuf();
    fAna->ChangeBackground(fGreen);
    fAna->SetText("Analyse");
    fNb->ChangeBackground(fGreen);
    crs->b_fana=false;
    crs->b_stop=true;
    //cout << "donbuf3" << endl;
    
  }

}

void MainFrame::DoRWinit(EFileDialogMode nn) {

  if (bRun) return;

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

  new TGFileDialog(gClient->GetRoot(), this, nn, &fi);

  if (fi.fFilename != NULL) {
    //printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
    //dir = fi.fIniDir;

    strcpy(pname,fi.fFilename);
    printf("TGFile: %s\n",pname);

    if (nn==kFDOpen) {
      //readinit(pname);
      gzFile ff = gzopen(pname,"rb");
      crs->ReadParGz(ff,pname,0,1,1);
      gzclose(ff);

      parpar->Update();
      crspar->Update();
      chanpar->Update();

      //delete fPar;
      //fPar = new MParDlg(gClient->GetRoot(), fMain, "Parameters");
      //fPar->Map();
      //if (fChan!=NULL) {
      //delete fChan;
      //fChan = new MChanDlg(gClient->GetRoot(), fMain, "Channels");
      //fChan->Map();
      //}
    }
    else {
      //saveinit(pname);
#ifdef LINUX
      if (chdir(startdir)) {}
#else
      _chdir(startdir);
#endif
      gzFile ff = gzopen(pname,"wb");
      if (ff) {
	crs->SaveParGz(ff);
	gzclose(ff);
      }
      else {
	cout << "Can't open file: " << pname << endl;
      }
    }
    //newfile();
  }

}

void MainFrame::DoReadRoot() {

  if (bRun) return;

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
    printf("xxxx TGFile: %s\n",rootname);

    readpar_root(rootname);
    //reset();
    //new_hist();
    readroot(rootname);

    parpar->Update();
    crspar->Update();
    chanpar->Update();
    HiFrm->Update();

    nevent=opt.Nevt;
    tof=opt.Tof;

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

}

void MainFrame::DoReset() {

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

  if (!crs->b_stop) return;

  greset();

  crs->DoReset();
  HiFrm->DoReset();

  parpar->Update();
  crspar->Update();
  chanpar->Update();

  //HiFrm->Update();

  //Buffer->NewFile();

  //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
  UpdateStatus();
  //DoDraw();

}

void MainFrame::DoClear() {

  if (bRun) return;

  clear_hist();

  //DoDraw();

}

/*
  void MainFrame::AddStack(int i, TH1* hh) {

  if (fHS[i]->GetHists()) {
  if (!(fHS[i]->GetHists()->FindObject(hh))) {
  fHS[i]->Add(hh);
  }
  }
  else {
  fHS[i]->Add(hh);
  }

  }
*/

void MainFrame::InitCanvas(int nn) {

  if (nn!=nPads) {

    //fEcanvas->Clear();
    TCanvas *fPad = fEcanvas->GetCanvas();
    fPad->Clear();

    if (nn==6) {
      fPad->Divide(3,2);
    }
    else if (nn==8) {
      fPad->Divide(4,2);
    }

    fPad->SetFillColor(kGray);
    for (int i=1;i<=nn;i++) {
      ((TPad*)fPad->GetPad(i))->SetFillColor(kWhite);
      if (opt.b_logy)
	((TPad*)fPad->GetPad(i))->SetLogy(1);
      else
	((TPad*)fPad->GetPad(i))->SetLogy(0);
    }

    ((TPad*)fPad->GetPad(2))->SetLogz(1);

    nPads=nn;

  }

}

/*
  void MainFrame::DoCheckGcut() {
  //printf("CheckDraw\n");
  opt.b_gcut = ! opt.b_gcut;
  DoDraw();
  }
*/

void MainFrame::DoCheckOsc() {
  //printf("CheckDraw\n");
  opt.b_osc = ! opt.b_osc;
  //DoDraw2();

  //malloc_stats();

}

void MainFrame::DoCheckLeg() {
  //printf("CheckDraw\n");
  opt.b_leg = ! opt.b_leg;
  //DoDraw();
}

void MainFrame::DoCheckLogY() {
  //printf("CheckDraw\n");
  //for (int jj=0;jj<6;jj++) {
  opt.b_logy = ! opt.b_logy;
  //}

  TCanvas *fPad = fEcanvas->GetCanvas();

  for (int i=1;i<=nPads;i++) {
    if (opt.b_logy) {
      ((TPad*)fPad->GetPad(i))->SetLogy(1);
    }
    else {
      ((TPad*)fPad->GetPad(i))->SetLogy(0);
    }
  }

  //DoDraw();
}

void MainFrame::DoCheckTime() {
  opt.b_time = ! opt.b_time;
  //DoDraw();
}

void MainFrame::UpdateStatus() {

  static Long64_t bytes1=0;
  static Long64_t nevents1=0;
  static double t1=0;
  double mb_rate,ev_rate;

  char txt[100];
  //time_t tt = opt.F_start.GetSec();

  time_t tt = (opt.F_start+788907600000)*0.001;
  struct tm *ptm = localtime(&tt);
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


  fStat[0]->SetText(txt,kFALSE);
  fStat[1]->SetText(TGString::Format("%0.1f",opt.T_acq),1);
  fStat[2]->SetText(TGString::Format("%lld",crs->nevents),kFALSE);
  fStat[3]->SetText(TGString::Format("%0.3f",ev_rate),kFALSE);
  fStat[4]->SetText(TGString::Format("%lld",crs->nevents2),kFALSE);
  fStat[5]->SetText(TGString::Format("%lld",crs->nbuffers),kFALSE);
  fStat[6]->SetText(TGString::Format("%0.2f",crs->totalbytes/MB),kFALSE);
  fStat[7]->SetText(TGString::Format("%0.2f",mb_rate),kFALSE);
  fStat[8]->SetText(TGString::Format("%0.2f",crs->writtenbytes/MB),kFALSE);

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

void MainFrame::DoSetNumBuf() {

  if (bRun) return;

  opt.num_buf=(int) n_buffers->GetNumber();
  //printf("test %d\n",gg);
}

void MainFrame::DoStop() {
  bRun=false;
  //DoDraw();
}
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
  gzFile ff = gzopen(parname,"wb");
  if (ff) {
    crs->SaveParGz(ff);
    gzclose(ff);
  }
  else {
    cout << "Can't open file: " << parname << endl;
  }

  printf("%lld bytes\n",recd*2);
  printf("%d events\n",nevent);
  printf("%d bad events\n",bad_events);

  delete this;
  //gApplication->Terminate(0);
}

void MainFrame::DoSave() {

  if (bRun) return;

  const char *dnd_types[] = {
    "root files",     "*.root",
    0,               0
  };

  //int i=0;
  char s_name[100];
  //int len;
  //int nbins;

  //double x,y0,y1,y2,y3,y4,y5;

  char dir[100], name[100], ext[100];
  TGFileInfo fi;

  //string str(fname);
  //string str2 ("c:\\windows\\winhelp.exe");

  SplitFilename (string(fname),dir,name,ext);

  //printf("split: %s %s %s\n",dir,name,ext);
  //SplitFilename (str2);
  //exit(-1);

  strcat(dir,"save/");
  //strcpy(s_name,dir);
  //strcat(s_name,name);
  strcpy(s_name,name);
  strcat(s_name,".root");

  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);
  fi.fFilename  = StrDup(s_name);

  //TGFileDialog *gfsave = 
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);

  //cout << gfsave << endl;

  if (fi.fFilename == NULL) {
    return;
  }

  saveroot(fi.fFilename);

}

void MainFrame::DoGcut(int nn) {
  const char* cname[3] = {"gamma","neutrons","tail"};
  cout << "gcut " << nn << endl;

  TPad *c1 = (TPad*) fEcanvas->GetCanvas()->GetPad(2);
  c1->WaitPrimitive("CUTG","CutG");
  //TCutG *cut1 = new TCutG((TCutG)gPad->GetPrimitive("CUTG"));


  opt.gcut[nn] = new TCutG(*(TCutG*)gROOT->GetListOfSpecials()->FindObject("CUTG"));
  //TCutG *cut2 = new TCutG(*cut1);
  opt.gcut[nn]->SetName(cname[nn]);

  if (nn==0) {
    opt.gcut[nn]->SetLineColor(2);
  }
  else if (nn==1) {
    opt.gcut[nn]->SetLineColor(1);
  }
  else if (nn==2) {
    opt.gcut[nn]->SetLineColor(4);
  }
  c1->cd();
  opt.gcut[nn]->Print();
  opt.gcut[nn]->Draw();

  //TCutG *cut1 = new TCutG( (TCutG) gPad->GetPrimitive("CUTG"));
  //TCutG* cut = (TCutG*)gPad->GetPrimitive("CUTG");

}


/*
  void MainFrame::MakeEvents() {

  cout << "MakeEvents: " << crs->npulses << endl;

  }
*/

void MainFrame::DoTab(Int_t num) {
  //cout << "DoTab: " << num << endl;
  TGTab *tab = (TGTab*) gTQSender;

  TString name = TString(tab->GetCurrentTab()->GetString());

  opt.seltab = tab->GetCurrent();

  //cout << "dotab: " << tab->GetCurrent() << " " 
  //     << tab->GetCurrentTab()->GetString() << endl;
  //cout << "dotab: " << name.EqualTo("Parameters",TString::kIgnoreCase) << endl;


  if (name.EqualTo("Parameters",TString::kIgnoreCase)) {
    cout << "DoTab1: " << name << endl;
    //cout << "Raw_wr: " << opt.raw_write << endl;
    parpar->Update();
  }
  else if (name.EqualTo("DAQ",TString::kIgnoreCase)) {
    cout << "DoTab2: " << name << endl;
    //cout << "Raw_wr: " << opt.raw_write << endl;
    crspar->Update();
  }
  else if (name.EqualTo("Channels",TString::kIgnoreCase)) {
    cout << "DoTab3: " << name << endl;
    chanpar->Update();
  }
  else if (name.EqualTo("Events",TString::kIgnoreCase)) {
    cout << "DoTab4: " << name << endl;
    if (crs->b_stop)
      EvtFrm->DrawEvent2();
  }
  else if (name.EqualTo("Histograms",TString::kIgnoreCase)) {
    cout << "DoTab5: " << name << endl;
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

void MainFrame::HandleMenu(MENU_COM menu_id)
{

  char command[128];
  int status;

  if (bRun) return;

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
  case M_FILE_NEWCANVAS:
    gROOT->MakeDefCanvas();
    break;

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

    strcpy(command,"sevince ");
    strcat(command,"help.pdf &");
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
  //EMsgBoxIcon icontype = kMBIconStop;
  //EMsgBoxIcon icontype = kMBIconExclamation;
  //EMsgBoxIcon icontype = kMBIconQuestion;
  EMsgBoxIcon icontype = kMBIconAsterisk;
  Int_t buttons = kMBOk|kMBCancel;
  const char* msg_exists = "Output file already exists.\nIt will be overwritten.\nPress OK if you want to continue?";

  struct stat buffer;

  if ((opt.raw_write && !stat(opt.fname_raw, &buffer)) ||
      (opt.dec_write && !stat(opt.fname_dec, &buffer))) {
    Int_t retval;
    new TGMsgBox(gClient->GetRoot(), this,
		 "File exists",
		 msg_exists, icontype, buttons, &retval);
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

void MainFrame::exec3event(Int_t event, Int_t x, Int_t y, TObject *selected)
{
  TCanvas *c = (TCanvas *) gTQSender;
  printf("Canvas %s: event=%d, x=%d, y=%d, selected=%s\n", c->GetName(),
	 event, x, y, selected->GetName());
  //   printf("Canvas %s: event=%d, x=%d, y=%d \n", c->GetName(),
  //     event, x, y);
}

void mkstart() {

  tpeaks=0;

  for (int i=0;i<mult;i++) {
    int ch = chan[i];
    if (ch==opt.start_ch) {
      tpeaks = npeaks[ch];
      tstarts = mean[ch];
      t_flag = peak_flag[ch];

      //cout << "TOF:: " << nevent << " " << i << " " << chan[i] << " " << npeaks[ch]<< endl;

    }
  }
}

int getmax(TH1F* hist[]) {

  int j,jmax=-1;
  double max=0;
  double max2;

  for (j=0;j<MAX_CH;j++) {
    if (hist[j]->GetEntries()>0 && opt.channels[j]!=ch_off2 && 
	opt.color[j]!=0) {
      max2 = hist[j]->GetMaximum();
      if (max2 > max) {
	max = max2;
	jmax=j;
      }
    }
  }

  return jmax;
}

void fitpeak(TH1* hh, double ww) {

  int imax = hh->GetMaximumBin();
  double max2 = hh->GetBinContent(imax)*0.5;
  if (max2 < 10) return;

  int i2=1;

  for (int i=imax;i<hh->GetNbinsX();i++) {
    if (hh->GetBinContent(i)<max2) {
      i2=i-imax;
      break;
    }
  }

  double x1=hh->GetBinCenter(imax-i2*ww);
  double x2=hh->GetBinCenter(imax+i2*ww);

  hh->Fit("gaus","","",x1,x2);

  cout << imax << " " << x1 << " " << x2 << endl;
  gPad->Update();
  double y1=gPad->GetUymin();
  double y2=gPad->GetUymax();
  TLine* ll=new TLine(x1,y1,x1,y2);
  ll->SetLineColor(3);
  ll->Draw();
  ll->DrawLine(x2,y1,x2,y2);

  double mean = hh->GetFunction("gaus")->GetParameter(1);
  double sig = hh->GetFunction("gaus")->GetParameter(2);

  double fwhm=sig*2.35/mean;
  char txt[100];
  sprintf(txt,"R=%6.3f",fwhm);

  TText tt;
  tt.DrawTextNDC(0.5,0.7,txt);

}

void allevents() {

  /*
  if (!Buffer->gzF || !Buffer->r_buf) {
    return;
  }
  */
}



//______________________________________________________________________________

/*
  void MainFrame::HandleHelp()
  {

  if (bRun) return;

  cout << "test" << endl;

  char command[128];

  strcpy(command,"evince ");
  strcat(command,"help.pdf");
  int status = system( command );

  cout << status << endl;

  }
*/
//______________________________________________________________________________


void example() {
  // Popup the GUI...
  myM=0;
  myM = new MyMainFrame(gClient->GetRoot(),800,600);
  //HiFrm->Update();
  //myM->Move(-100,-100);
}

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

void dumpevent()
{
  int i;

  //ftmp=fopen("event.dat","w");

  //fprintf(ftmp,"%d\n",nevent);
  printf("Event: %d %d %d\n",nevent,dlen2,NSamp);
  for (i=0;i<NSamp;i++) {
    //fprintf(ftmp,"%d %d\n",i,data[i]);
    //printf("%d %d %f\n",i,data[i],sdata[i]);
  }
  //close(ftmp);
}

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
