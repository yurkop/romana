//#define LONGSTAMP 1

#ifndef LINUX
#include <direct.h>
#endif

#include <signal.h>
//#include <malloc.h>

#include "romana.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>

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

Common* com;
EventFrame* EvtFrm;
HistFrame* HiFrm;
ErrFrame* ErrFrm;
HClass* hcl;

ParParDlg *parpar;
CrsParDlg *crspar;
AnaParDlg *anapar;
DspParDlg *pikpar;

//const int maxsamp = 16500;// константу 16500 надо будет заменить на переменную

//TString parname,lastpar;
char* parname=(char*)"romana.par";;
char* parname2=0;
char* datfname=0;

bool b_dec=false,b_root=false,b_force=false;

char startdir[200];
char pr_name[200];
char maintitle[200];
//char rootname[200]="";

struct stat statb;
const char* msg_exists = "Output file already exists.\nPress OK to overwrite it";


//std::list<string> listpar;
//typedef std::list<string>::iterator l_iter;
std::list<TString> listpar;
typedef std::list<TString>::iterator l_iter;

//TList listmap2;

MyMainFrame *myM;

Coptions cpar;
Toptions opt;
int debug=0; //2|4; //=1 or 2 or 6// for printing debug messages

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

void debug_mess(bool cond, const char* mess, double par1, int par2) {
  if (cond) {
    cout << mess << par1;
    if (par2!=-9999)
      cout << " " << par2;
    cout << endl;
  }
}

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
    if (debug&0x2) {
      // if (!dm->GetDataType()) {
      cout << "member: " << dm->GetName() << " " << dm->GetDataType() << " " << dm->GetClass()->GetName() << endl;
      // }
    }
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

  // TList* lst;
  // TIter nextd(lst);
  // TDataMember *dm;
  // while ((dm = (TDataMember *) nextd())) {
  // }

  //= (TList*)lst1->Clone("lst2");
  //cout << "lst: " << lst->GetName() << endl;
  //exit(1);

  Int_t sz=0;
  UShort_t len=0;
  UShort_t len2=0;
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
      TIter nextd(lst);
      while ((dm = (TDataMember *) nextd())) {
	if (!strcmp(dm->GetName(),memname)) {
	  break;
	}
	else {
	  const char* s1 = strchr(dm->GetTitle(),'[');
	  const char* s2 = strchr(dm->GetTitle(),']');
	  if (s1 && s2) {
	    int len=s2-s1-1;
	    char str[999]="";
	    strncpy(str,s1+1,len);
	    // cout << "dm: " << dm << " " << str << " " << len << " " << dm->GetName() << " " << dm->GetTitle() << endl;
	    // exit(-1);
	    if (!strcmp(str,memname)) {
	      cout << "dm2: " << dm << " " << str << " " << len << " " << dm->GetName() << " " << dm->GetTitle() << endl;
	      //exit(-1);
	      break;
	    }
	  }
	}
	//cout << "dm: " << dm << " " << dm->GetName() << " " << dm->GetTitle() << endl;
      }
      // cout << "dm2: " << dm << endl;
      // if (dm) {
      //  	cout << "dm: " << dm << " " << dm->GetName() << " " << dm->GetTitle() << endl;
      // }
      //dm = (TDataMember*) lst->FindObject(memname);
      if (dm) {
	len2=dm->GetUnitSize();
	for (int i=0;i<dm->GetArrayDim();i++) {
	  len2*=dm->GetMaxIndex(i);
	}
	if (debug&0x4)
	  cout << "read: " << dm->GetName() << " " << len << " " << sz << endl;
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

  //common_init();
  //printf("Version: %s\n", VERSION);
  //exit(1);
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
  cout << "Usage: romana [-h] [filename] [-p parname] [-b] [-d] [-r] [-f] " << endl;
  cout << "-h: print usage and exit" << endl;
  cout << "filename: read data and parameters from filename" << endl;
  cout << "-p parname: read parameters from parname, parameters from filename are ignored" << endl;
  cout << "-b: analyze file in batch mode (without gui) and exit" << endl;
  cout << "-d (only in batch mode): create decoded .dec file in subdirectory Dec" << endl;
  cout << "-r (only in batch mode): create .root file in subdirectory Root" << endl;
  cout << "-f (only in batch mode): force overwriting .dec and/or .root files" << endl;
  cout << "Options <Filename>, <Write raw/decoded/root data> are ignored in batch mode" << endl; 
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

  listpar.clear();
  //process command line parameters
  if (argc > 1) {
    for (int i=1;i<argc;i++) {
      //int argnn=1;
      //while (argnn<argc) {
      // cout << "argnn: " << argc << " " << argnn << " " << argv[argnn] << " "
      // 	   << argv[argnn]+1 << endl;
      TString sarg=TString(argv[i]);

      if (sarg[0]=='-') {
	//cout << "sarg: " << i << " " << sarg << " " << (int) sarg[1] << endl;
	switch (sarg[1]) {
	case 'h':
	case 'H':
	  exit(0);
	case 'b':
	case 'B':
	  crs->batch=true;
	  continue;
	case 'd':
	case 'D':
	  b_dec=true;
	  continue;
	case 'r':
	case 'R':
	  b_root=true;
	  continue;
	case 'f':
	case 'F':
	  b_force=true;
	  continue;
	case 'p':
	case 'P':
	  ++i;
	  if (i<argc) {
	    parname2 = argv[i];
	  }
	  //cout << "parname2: " << parname2 << endl;
	  continue;
	default:
	  continue;
	}
      } //if (sarg[0]=='-')
      //else if (sarg.find("=")!=string::npos) {
      else if (sarg.First("=")>=0) {
	listpar.push_back(sarg);
      }
      else {
	datfname = argv[i];
      }
      /*
	char cc = argv[i][0];
	if (cc=='-') { //control character
	char pp = argv[i][1];
	}
	else if (cc=='+') { //read file of parameters
	parname2 = argv[argnn]+1;
	}
	else { //read file
	datfname = argv[argnn];
	}
	argnn++;
      */
    } //for i
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

  //cout << "listpar: " << endl;
  for (l_iter it=listpar.begin(); it!=listpar.end(); ++it) {
    //try {
    TString par, // parameter name
      p0, //parameter with []
      p2, //index
      sdata; //value
    int index;
    Ssiz_t ll,len;
    char sbuf[1024];
    int buflen=0;
    char* var = (char*) &opt;

    ll = it->First("=");
    //p0 = it->substr(0,ll);
    p0 = (*it)(0,ll);
    //sdata=it->substr(ll+1);
    sdata=(*it)(ll+1,it->Length());
    //cout << "p0: " << p0 << " " << sdata << endl;

    ll = p0.First("[");
    len = p0.First("]");
    //cout << "p0: " << " " << ll << " " << len << endl;
    if (ll==kNPOS || len==kNPOS) {
      par=p0;
      index=-1;
    }
    else {
      len=len-ll-1;
      //par=p0.substr(0,ll);
      par=p0(0,ll);
      //p2=p0.substr(ll+1,len);
      p2=p0(ll+1,len);
      index=p2.Atoi();
    }

    //strcpy(opt.Filename,"qwerty");
    //cout << "par: " << *it << " " << par << " " << p0 << " " << index << " " << sdata << endl;

    TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
    TDataMember* dm = (TDataMember*) lst->FindObject(par.Data());
    if (dm) {
      //cout << "dm: " << dm << " " << dm->GetName() << " " << dm->GetTitle() << endl;
      //TDataType* tm = dm->GetDataType();
      TString tp = dm->GetTypeName();
      //cout << "tp: " << tp << " " <<dm->GetArrayDim() << " " << dm->GetMaxIndex(0) << endl;
      int dim=dm->GetArrayDim();
      int maxindex = dm->GetMaxIndex(0);
      int unit = dm->GetUnitSize();
      Long_t off = dm->GetOffset();
      //cout << "off: " << var+off << " " << &opt.Filename << endl;
      if (tp.Contains("int",TString::kIgnoreCase)) {
	int d=sdata.Atoi();
	buflen=sizeof(int);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("bool",TString::kIgnoreCase)) {
	bool d=sdata.Atoi();
	buflen=sizeof(bool);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("float",TString::kIgnoreCase)) {
	float d=sdata.Atof();
	buflen=sizeof(float);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("char",TString::kIgnoreCase)) {
	buflen=sdata.Length();
	memset(var+off,0,maxindex);
	memcpy(var+off,sdata.Data(),TMath::Min(buflen,maxindex));
	//cout << "char: " << sdata.Data() << " " << buflen << " " << maxindex << endl;
	continue;
      }
      else {
	cout << "Error: unknown type of parameter: "
	     << par << " " << tp << endl;
	continue;
      }

      if (dim==0) { //only one parameter
	memcpy(var+off,sbuf,TMath::Min(buflen,unit));
      }
      else if (dim==1) { //array [text was copied in the if("char") ]
	if (index<0) {
	  for (int j=0;j<maxindex;j++) {
	    memcpy(var+off+j*unit,sbuf,TMath::Min(buflen,unit));
	  }
	}
	else if (index<maxindex) {
	  memcpy(var+off+index*unit,sbuf,TMath::Min(buflen,unit));
	}
	else {
	  cout << "Error: index is out of range : "
	       << par << " " << index << " " << maxindex << endl;
	  continue;
	}
      }
      else { //2-dim of more - ignore
	cout << "Error: parameter with dimension >1 : "
	     << par << " " << dim << endl;
	continue;
      }
    }
      
    //lst->ls();
    //}
    // catch (const std::invalid_argument& ia) {
    //   std::cerr << "Invalid argument: " << ia.what() << '\n';
    // }

  }

  //cout << "sS: " << opt.sS[1] << " " << opt.Filename << endl;
  //exit(-1);
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


  //batch loop
  EvtFrm = 0;
  if (crs->batch) {
    if (strlen(datfname)==0) {
      cout << "No input file. Exiting..." << endl;
      exit(0);
    }

    if (!TestFile()) {
      exit(0);
    }

    hcl->Make_hist();

    crs->b_fana=true;
    crs->b_stop=false;

    crs->FAnalyze2(false);

    crs->b_fana=false;
    crs->b_stop=true;

    //allevents();

    cout << crs->rootname << endl;
    saveroot(crs->rootname.c_str());

    return 0;
  }


#ifdef CYUSB
  if (crs->Fmode!=2) {
    bool d = opt.decode;
    bool w = opt.raw_write;
    opt.decode=0;
    opt.raw_write=0;
    crs->Detect_device();
    opt.decode=d;
    opt.raw_write=w;
  }
#endif


  TApplication theApp("App",&argc,argv);
  //example();

  com = new Common();
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

  opt.raw_write=false;
  opt.dec_write=false;
  opt.root_write=false;

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

bool TestFile() {

  string dir, name, ext;

  if (crs->batch) {
    //strcpy(opt.Filename,crs->Fname);
    SplitFilename (string(crs->Fname),dir,name,ext);
    dir = TString(startdir);
    //cout << "Root_dir: " << dir << endl;

    crs->decname=dir;
    crs->rootname=dir;
    crs->decname.append("Dec/");
    crs->rootname.append("Root/");

    opt.dec_write=b_dec;

    if (b_dec) {
#ifdef LINUX
      mkdir(crs->decname.c_str(),0755);
#else
      _mkdir(crs->decname.c_str());
#endif
    }
    
    opt.root_write=b_root;
    if (b_root) {
#ifdef LINUX
      mkdir(crs->rootname.c_str(),0755);
#else
      _mkdir(crs->rootname.c_str());
#endif
    }

    crs->decname.append(name);
    crs->rootname.append(name);
  }
  else {
    SplitFilename(string(opt.Filename),dir,name,ext);
    dir.append(name);
    crs->rawname=dir;
    crs->decname=dir;
    crs->rootname=dir;
    crs->rawname.append(".raw");
  }

  crs->decname.append(".dec");
  crs->rootname.append(".root");
  cout << "Fname: " << crs->Fname << " " << crs->module << endl;
  cout << "rawname: " << crs->rawname << " " << opt.raw_write << endl;
  cout << "decname: " << crs->decname << " " << opt.dec_write << endl;
  cout << "rootname: " << crs->rootname << " " << opt.root_write << endl;
  // exit(0);
  if (!crs->juststarted) return true;

  //EMsgBoxIcon icontype = kMBIconStop;
  //EMsgBoxIcon icontype = kMBIconExclamation;
  //EMsgBoxIcon icontype = kMBIconQuestion;
  //EMsgBoxIcon icontype = kMBIconAsterisk;
  //Int_t buttons = kMBOk|kMBCancel;

  if ((opt.raw_write && !stat(crs->rawname.c_str(), &statb)) ||
      (opt.dec_write && !stat(crs->decname.c_str(), &statb)) ||
      (opt.root_write && !stat(crs->rootname.c_str(), &statb))) {

    if (crs->batch && !b_force) {
      cout << "file exists. Exiting..." << endl;
      exit(0);
    }
    else {
      return true;
    }

    Int_t retval;
    //new ColorMsgBox(gClient->GetRoot(), this,
    new TGMsgBox(gClient->GetRoot(), myM,
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

  //bRun = false;

  fMenuBar = new TGMenuBar(this, 35, 50, kHorizontalFrame);

  TGPopupMenu* fMenuFile = new TGPopupMenu(gClient->GetRoot());

  fMenuFile->AddEntry("Read Parameters", M_READINIT);
  fMenuFile->AddEntry("Save Parameters", M_SAVEINIT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Read ROOT file", M_READROOT);
  fMenuFile->AddEntry("Save ROOT file", M_SAVEROOT);
  fMenuFile->AddSeparator();
  //fMenuFile->AddEntry("Save ASCII file", M_SAVEASCII);
  //fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Export...", M_EXPORT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Browser\tCtrl+B", M_FILE_BROWSE);
  fMenuFile->AddEntry("Reset USB", M_RESET_USB);
  //fMenuFile->AddEntry("New Canvas\tCtrl+N", M_FILE_NEWCANVAS);

  //fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
  //fMenuFile->AddEntry("&Save", M_FILE_SAVE);

  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  //fMenuFile->AddPopup("&Cascaded menus", fCascadeMenu);
  fMenuFile->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  fMenuBar->AddPopup("&File", fMenuFile, 
		     new TGLayoutHints(kLHintsLeft|kLHintsTop,0,4,0,0));

  TGPopupMenu* fMenuProf = new TGPopupMenu(gClient->GetRoot());
  fMenuProf->AddEntry("Edit Ing-27 channel map", M_EDIT_CHMAP);

  fMenuProf->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  fMenuBar->AddPopup("&Profilometer", fMenuProf, 
  		     new TGLayoutHints(kLHintsLeft|kLHintsTop,0,4,0,0));

  TGPopupMenu* fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("Display Help file", M_HELP);
  //fMenuHelp->AddEntry("Test", M_TEST);
  fMenuHelp->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  /*
    fMenuBar->AddPopup("&Options", fMenuOptions, 
    new TGLayoutHints(kLHintsLeft|kLHintsTop));

    fMenuBar->AddPopup("Histograms", fMenuHist, 
    new TGLayoutHints(kLHintsLeft|kLHintsTop));

    fMenuBar->AddPopup("Analysis", fMenuAna, 
    new TGLayoutHints(kLHintsLeft|kLHintsTop));
  */
  
  fMenuBar->AddPopup("&Help", fMenuHelp,
		     new TGLayoutHints(kLHintsTop|kLHintsRight,0,4,0,0));

  AddFrame(fMenuBar, 
	   new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 5));

  //TGLabel *ver = new TGLabel(fMenuBar,VERSION);
  //fMenuBar->AddFrame(ver,new TGLayoutHints(kLHintsCenterY|kLHintsRight,0,4,0,0));

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


  const int butx=80,buty=40;

  TGGroupFrame* fGr1 = new TGGroupFrame(vframe1, "Acquisition", kVerticalFrame);
  fGr1->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  vframe1->AddFrame(fGr1, com->LayCT1);

  fStart = new TGTextButton(fGr1,"Start");
  //fStart->SetToggleButton(true);

  fStart->SetFont(tfont,false);
  fStart->Resize(butx,buty);
  fStart->ChangeOptions(fStart->GetOptions() | kFixedSize);
  //fStart->SetStyle("modern");

  fStart->ChangeBackground(com->fGreen);

  fStart->Connect("Clicked()","MainFrame",this,"DoStartStop()");
  fGr1->AddFrame(fStart, com->LayET1);

  fReset = new TGTextButton(fGr1,"Reset");
  fReset->SetFont(tfont,false);

  fReset->SetTextJustify(kTextCenterX);

  fReset->Resize(butx,buty);
  fReset->ChangeOptions(fStart->GetOptions() | kFixedSize);
  fReset->ChangeBackground(com->fCyan);

  fReset->Connect("Clicked()","MainFrame",this,"DoReset()");
  fGr1->AddFrame(fReset, com->LayET1);

  TGGroupFrame* fGr2 = new TGGroupFrame(vframe1, "Analysis", kVerticalFrame);
  fGr2->SetTitlePos(TGGroupFrame::kCenter);
  vframe1->AddFrame(fGr2, com->LayCT1);

  TGPopupMenu* fPopMenu = new TGPopupMenu(gClient->GetRoot());
  fPopMenu->AddEntry("Open+", 1);
  fPopMenu->AddEntry("Open-", 0);
  //fPopMenu->AddSeparator();
  //fPopMenu->Resize(butx,buty);
  //fPopMenu->ChangeOptions(fPopMenu->GetOptions() | kFixedSize);

  TGSplitButton *fOpen = new TGSplitButton(fGr2, new TGHotString("&Open+"),
					   fPopMenu,1);
  fOpen->SetSplit(true);
  fOpen->SetToolTipText("Open+: open data file with parameters\nOpen-: open data file without parameters");
  fOpen->SetFont(tfont,false);
  fOpen->Resize(butx,buty);
  fOpen->ChangeOptions(fOpen->GetOptions() | kFixedSize);
  fOpen->ChangeBackground(com->fOrng);
  fOpen->Connect("ItemClicked(Int_t)", "MainFrame", this, "DoOpen(Int_t)");
  fGr2->AddFrame(fOpen, com->LayET1);

  TGTextButton *fClose = new TGTextButton(fGr2,new TGHotString("&Close"));
  fClose->SetFont(tfont,false);
  fClose->Resize(butx,buty);
  fClose->ChangeOptions(fClose->GetOptions() | kFixedSize);
  fClose->ChangeBackground(com->fBlue);
  fClose->Connect("Clicked()","MainFrame",this,"DoClose()");
  fGr2->AddFrame(fClose, com->LayET1);

  TGTextButton *fReset2 = new TGTextButton(fGr2,new TGHotString("&Reset"));
  fReset2->SetFont(tfont,false);
  fReset2->Resize(butx,buty);
  fReset2->ChangeOptions(fReset2->GetOptions() | kFixedSize);
  fReset2->ChangeBackground(com->fCyan);
  fReset2->Connect("Clicked()","MainFrame",this,"DoReset()");
  //fReset2->Connect("Clicked()","CRS",crs,"Reset()");
  fGr2->AddFrame(fReset2, com->LayET1);

  fAna = new TGTextButton(fGr2,"&Analyze");
  fAna->SetFont(tfont,false);
  fAna->Resize(butx,buty);
  fAna->ChangeOptions(fAna->GetOptions() | kFixedSize);
  fAna->ChangeBackground(com->fGreen);
  fAna->Connect("Clicked()","MainFrame",this,"DoAna()");
  fGr2->AddFrame(fAna, com->LayET1);

  TGTextButton* f1b = new TGTextButton(fGr2,new TGHotString("&1 buf"));
  f1b->SetFont(tfont,false);
  f1b->Resize(butx,buty);
  f1b->ChangeOptions(f1b->GetOptions() | kFixedSize);
  f1b->ChangeBackground(com->fGreen);
  f1b->Connect("Clicked()","MainFrame",this,"Do1buf()");
  fGr2->AddFrame(f1b, com->LayET1);

  TGLabel *ver = new TGLabel(vframe1,VERSION);

  vframe1->AddFrame(ver,new TGLayoutHints(kLHintsBottom|kLHintsCenterX,0,0,0,4));

  fTab = new TGTab(hframe1, 300, 300);
  hframe1->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX |
					    kLHintsExpandY,3,3,2,2));

  fTab->Connect("Selected(Int_t)", "MainFrame", this, "DoTab(Int_t)");


  //fremake=false;

  //cout << "tab1: " << endl;
  tabfr[0] = fTab->AddTab("Parameters");
  tabfr[1] = fTab->AddTab("DAQ");
  tabfr[2] = fTab->AddTab("Analysis");
  tabfr[3] = fTab->AddTab("Peaks");
  //tabfr[2] = fTab->AddTab("Channels");
  tabfr[4] = fTab->AddTab("Events");
  tabfr[5] = fTab->AddTab("Histograms/Cuts");
  tabfr[6] = fTab->AddTab("Errors");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");
  //TGDockableFrame *tab4 = fTab->AddTab("Events");

  //parpar = new ParParDlg(tabfr[0], 600, 500);
  //parpar->Update();
  //tabfr[0]->AddFrame(parpar, LayEE1);

  //cout << "Maketabs1: " << endl;
  MakeTabs();
  //cout << "Maketabs2: " << endl;
  //fremake=true;

  //TGTabElement* tab6 = fTab->GetTabTab("Errors");
  //tab6->SetBackgroundColor(com->fRed);
  //cout << "tab6: " << tab6 << endl;
  //exit(1);
  //MakeTabs();

  if (crs->Fmode!=1) { //no CRS present
    crspar->AllEnabled(false);

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
  fGr2->AddFrame(hfr1, com->LayET1);

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
  hfr1->AddFrame(fNum1,com->LayLT3);

  fNb = new TGTextButton(hfr1,new TGHotString("&N buf"));
  //fNb->SetFont(tfont,false);
  fNb->Resize(35,22);
  fNb->ChangeOptions(fNb->GetOptions() | kFixedSize);
  fNb->ChangeBackground(com->fGreen);
  fNb->Connect("Clicked()","MainFrame",this,"DoNbuf()");
  hfr1->AddFrame(fNb, com->LayLT3);

  parpar->Update();

  //HiFrm->DoReset();
  //HiFrm->Update();

  //HiFrm->Update();
  fTab->SetTab(opt.seltab);

  tfont=gClient->GetResourcePool()->GetStatusFont()->GetFontStruct();

  TGHorizontalFrame *fStatFrame1 = new TGHorizontalFrame(this,10,10);
  TGHorizontalFrame *fStatFrame2 = new TGHorizontalFrame(this,10,10);
  AddFrame(fStatFrame1, com->LayET0);
  AddFrame(fStatFrame2, com->LayET0);

  const int fwid=120;

  const char* txtlab[n_stat] = {"Start","AcqTime","Events","Ev/sec","MTrig","MTrig/sec","Buffers","MB in","MB/sec","Raw MB out","Dec MB out"};

  const char* st_tip[n_stat] = {
    "Acquisition start",
    //"Total Acquisition Time",
    "Acquisition Time",
    "Total number of events received",
    "Event rate received (in Hz)",
    "Total number of main trigger events",
    "Main trigger event rate (in Hz)",
    "Number of buffers received",
    "Megabytes received",
    "Received megabytes per second",
    "Raw megabytes saved",
    "Decoded megabytes saved"
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
      fStatFrame1->AddFrame(fLab[i],com->LayL1);
      fStat[i]->SetWidth(fwid);
      fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kFixedWidth);
      fStatFrame2->AddFrame(fStat[i],com->LayL1);
    }
    else {
      fStatFrame1->AddFrame(fLab[i],com->LayE1);
      fStatFrame2->AddFrame(fStat[i],com->LayE1);
    }

  }
  
  //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
  //UpdateStatus();

  UpdateStatus(1);

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

  if (crs->b_acq && crs->Fmode==1) {
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
  anapar->Rebuild();
  pikpar->Rebuild();
  Resize(GetDefaultSize());
  //MapSubwindows();
  Layout();

  //cout << "main::Rebuild2: " << endl;
}

void MainFrame::MakeTabs() {

  int ntab=0;

  //cout << "tab0: " << endl;

  parpar = new ParParDlg(tabfr[0], 600, 500);
  parpar->Update();
  tabfr[0]->AddFrame(parpar, com->LayEE1);
  ntab++;

  //cout << "tab2: " << endl;
  crspar = new CrsParDlg(tabfr[1], 600, 500);
  crspar->Make_crspar(tabfr[1], 600, 210);
  tabfr[1]->AddFrame(crspar, com->LayEE2);
  ntab++;
  crspar->Update();
  //cout << "tab3: " << endl;

  anapar = new AnaParDlg(tabfr[2], 600, 500);
  anapar->Make_AnaPar(tabfr[2], 600, 210);
  tabfr[2]->AddFrame(anapar, com->LayEE2);
  ntab++;
  anapar->Update();

  pikpar = new DspParDlg(tabfr[3], 600, 500);
  pikpar->Make_DspPar(tabfr[3], 600, 210);
  tabfr[3]->AddFrame(pikpar, com->LayEE2);
  ntab++;
  pikpar->Update();

  EvtFrm = new EventFrame(tabfr[4], 620, 500,ntab);
  tabfr[4]->AddFrame(EvtFrm, com->LayEE1);
  ntab++;

  HiFrm = new HistFrame(tabfr[5], 800, 500,ntab);
  HiFrm->HiReset();
  tabfr[5]->AddFrame(HiFrm, com->LayEE1);
  ntab++;

  ErrFrm = new ErrFrame(tabfr[6], 800, 500);
  //HiFrm->HiReset();
  tabfr[6]->AddFrame(ErrFrm, com->LayEE1);
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
    fStart->ChangeBackground(com->fGreen);
    fStart->SetText("Start");
    //crs->b_stop=false;
    //crs->Show();
    crs->DoStartStop();

    if (opt.root_write) {
      saveroot(crs->rootname.c_str());
    }

  }
  else { // START is pressed here
    if (TestFile()) {
      //ParLock();
      fStart->ChangeBackground(com->fRed);
      fStart->SetText("Stop");
      crs->DoStartStop();
      //cout << "Start7: " << endl;


      fStart->ChangeBackground(com->fGreen);
      fStart->SetText("Start");

      crs->b_stop=true;
      crs->b_fana=false;
      crs->b_acq=false;
      crs->b_run=0;

      if (opt.root_write) {
	saveroot(crs->rootname.c_str());
      }

    }
    //crs->b_stop=true;
  }
#endif

}

void MainFrame::DoOpen(Int_t id) {

  if (!crs->b_stop) return;

  //id=12-id;
  //cout << "DoOpen: " << id << endl;

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
    crs->DoFopen(fi.fFilename,id);//1 - read toptions

    parpar->Update();
    crspar->Update();
    anapar->Update();
    pikpar->Update();

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
  // for (int i=0;i<crs->MAXTRANS;i++) {
  //   crs->Fbuf[i]=0;
  // }

  myM->SetTitle((char*)"");
  crspar->AllEnabled(true);

  parpar->Update();
  crspar->Update();
  anapar->Update();
  pikpar->Update();

#ifdef CYUSB
  crs->Detect_device();
  if (crs->Fmode==1) { //CRS is present
    crspar->AllEnabled(true);

    fStart->SetEnabled(true);
    fReset->SetEnabled(true);

    //opt.raw_write=false;
    //parpar->Update();
    TGCheckButton *te = (TGCheckButton*) parpar->FindWidget(&opt.raw_write);
    if (te) 
      te->SetEnabled(true);
  }
#endif

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
      saveroot(crs->rootname.c_str());
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
	saveroot(crs->rootname.c_str());
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
    fAna->ChangeBackground(com->fGreen);
    fAna->SetText("&Analyse");
    fNb->ChangeBackground(com->fGreen);
    gSystem->Sleep(100);
    crs->b_fana=false;
    crs->b_stop=true;
  }
  else { //start analysis of n buffers
    if (TestFile()) {
      fAna->ChangeBackground(com->fRed);
      fAna->SetText("P&ause");
      fNb->ChangeBackground(com->fRed);
      crs->b_fana=true;
      crs->b_stop=false;
      crs->DoNBuf2(opt.num_buf);
      fAna->ChangeBackground(com->fGreen);
      fAna->SetText("&Analyse");
      fNb->ChangeBackground(com->fGreen);
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

  new TGFileDialog(gClient->GetRoot(), this, nn, &fi);

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
      anapar->Update();
      pikpar->Update();

    }
    else { //Save pars
      Int_t retval=kMBOk;
      if (!stat(fi.fFilename, &statb)) {
	new TGMsgBox(gClient->GetRoot(), this,
		     "File exists",
		     msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
      }

      if (retval == kMBOk) {
	//saveinit(pname);
	gzFile ff = gzopen(pname,"wb");
	if (ff) {
	  crs->SaveParGz(ff,crs->module);
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

  char rname[255];

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

    strcpy(rname,fi.fFilename);

    readpar_root(rname);
    DoReset();
    //new_hist();

    readroot(rname);

    parpar->Update();
    crspar->Update();
    anapar->Update();
    pikpar->Update();
    HiFrm->Update();

    //nevent=opt.Nevt;
    //tof=opt.Tof;

    //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
    UpdateStatus(1);

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
    if (!stat(fi.fFilename, &statb)) {
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

  ErrFrm->Reset();
  //else {
  HiFrm->HiReset();
  parpar->Update();
  crspar->Update();
  anapar->Update();
  pikpar->Update();
  //}

  UpdateStatus(1);

}

void MainFrame::UpdateStatus(int rst) {

  int ii=0;

  static Long64_t bytes1=0;
  static Long64_t nevents_old=0;
  static Long64_t nevents2_old=0;
  static double t1=0;
  static double mb_rate,ev_rate,trig_rate;

  if (rst) {
    bytes1=0;
    nevents_old=0;
    nevents2_old=0;
    t1=0;
    mb_rate=0;
    ev_rate=0;
    trig_rate=0;
  }

  char txt[100];
  //time_t tt = opt.F_start.GetSec();

  //cout << "T_acq2: " << opt.T_acq << " " << crs->Tstart64 << endl;
  // if (opt.Tstop && opt.T_acq>opt.Tstop) {
  //   DoStartStop();
  // }

  time_t tt = (opt.F_start+788907600000)*0.001;
  struct tm *ptm = localtime(&tt);
  //struct tm *ptm = gmtime(&tt);
  strftime(txt,sizeof(txt),"%F %T",ptm);

  double dt = opt.T_acq - t1;

  if (dt>0.1) {
    mb_rate = (crs->inputbytes-bytes1)/MB/dt;
    ev_rate = (crs->nevents-nevents_old)/dt;
    trig_rate = (crs->nevents2-nevents2_old)/dt;
    //cout << "trig_rate: " << trig_rate << " " << dt << endl;

    bytes1=crs->inputbytes;
    nevents_old=crs->nevents;
    nevents2_old=crs->nevents2;
    t1=opt.T_acq;
  }

  fStat[ii++]->SetText(txt,kFALSE);

  //exit(1);
  //fStat[ii++]->SetText(TGString::Format("%0.1f",crs->F_acq),1);
  fStat[ii++]->SetText(TGString::Format("%0.1f",opt.T_acq),1);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.3f",ev_rate),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents2),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.3f",trig_rate),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nbuffers),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->inputbytes/MB),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",mb_rate),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->rawbytes/MB),kFALSE);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->decbytes/MB),kFALSE);

  //cout << txt << endl;
  //return;
  /*
    fBar1->SetText(txt,0);
    fBar1->SetText(TGString::Format("%0.2f",opt.T_acq),1);
    fBar1->SetText(TGString::Format("%lld",crs->nevents),2);
    fBar1->SetText(TGString::Format("%lld",crs->nevents2),3);
    fBar1->SetText(TGString::Format("%lld",crs->npulses),4);
    fBar1->SetText(TGString::Format("%lld",crs->nbuffers),5);
    fBar1->SetText(TGString::Format("%0.2f",crs->inputbytes/MB),6);
    fBar1->SetText(TGString::Format("%0.2f",crs->mb_rate),7);
    fBar1->SetText(TGString::Format("%0.2f",crs->rawbytes/MB),8);
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
    crs->SaveParGz(ff,crs->module);
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

void MainFrame::DoSaveRoot() {

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
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);

  if (fi.fFilename) {
    Int_t retval=kMBOk;
    if (!stat(fi.fFilename, &statb)) {
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

/*
void MainFrame::DoSaveAscii() {

  if (!crs->b_stop) return;




#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

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
    //cout << "DoTab1: " << name << endl;
    parpar->Update();
  }
  else if (name.EqualTo("DAQ",TString::kIgnoreCase)) {
    //cout << "DoTab2: " << name << endl;
    crspar->Update();
  }
  else if (name.EqualTo("Analysis",TString::kIgnoreCase)) {
    //cout << "DoTab3: " << name << endl;
    anapar->Update();
  }
  else if (name.EqualTo("Peaks",TString::kIgnoreCase)) {
    //cout << "DoTab3: " << name << endl;
    pikpar->Update();
  }
  else if (name.EqualTo("Events",TString::kIgnoreCase)) {
    //cout << "DoTab4: " << name << endl;
    if (crs->b_stop)
      EvtFrm->DrawEvent2();
  }
  else if (name.Contains("Histograms",TString::kIgnoreCase)) {
    //cout << "DoTab5: " << name << endl;
    if (!crs->b_acq)
      HiFrm->Update();
    //HiFrm->ReDraw();
  }
  else if (name.Contains("Errors",TString::kIgnoreCase)) {
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
    DoSaveRoot();
    break;
  // case M_SAVEASCII:
  //   DoSaveAscii();
  //   break;
  case M_READROOT:
    DoReadRoot();
    break;
  case M_FILE_BROWSE:
    new TBrowser();
    break;
  case M_RESET_USB:
    crs->DoResetUSB();
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

  case M_EDIT_CHMAP:
    {
      //cout << "ed: " << endl;
      Editor *ed = new Editor(this, 200, 400);
      ed->LoadPar();
      //ed->LoadBuffer(editortxt1);
      ed->Popup();
    }
    break;
  case M_HELP:

    strcpy(command,"xdg-open ");
    strcat(command,HELP);
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

//-------------------------
Common::Common() {

  gClient->GetColorByName("green", fGreen);
  gClient->GetColorByName("red", fRed);
  gClient->GetColorByName("cyan", fCyan);
  //gClient->GetColorByName("BlueViolet",fBluevio);
  fOrng=TColor::RGB2Pixel(255,114,86);
  fBlue = TColor::RGB2Pixel(135,92,231);
  fRed10=gROOT->GetColor(kRed-10)->GetPixel();

  LayCC0   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,0,0);
  LayCC0a  = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0,0,1,1);
  LayCC1   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 4, 4, 0, 0);
  LayCC2   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 0, 0, 5, 5);

  LayCB0   = new TGLayoutHints(kLHintsCenterX|kLHintsBottom,0,0,0,0);
  LayCB5 = new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5);
 
  LayCT1 = new TGLayoutHints(kLHintsCenterX|kLHintsTop,1,1,20,2);

  LayCLE2 = new TGLayoutHints(kLHintsCenterY|kLHintsLeft|kLHintsExpandX, 4, 2, 2, 2);

  LayE1 = new TGLayoutHints(kLHintsExpandX,1,1,0,0);
  LayET0  = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
  LayET1 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,5,5);
  //LayET2   = new TGLayoutHints(kLHintsExpandX|kLHintsTop, 0, 0, 2, 2);
  LayET3   = new TGLayoutHints(kLHintsExpandX|kLHintsTop, 2, 2, 0, 0);

  LayLT0   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 0,0,0,0);
  LayLT2  = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 0);
  LayLT3 = new TGLayoutHints(kLHintsLeft|kLHintsTop,1,1,1,1);
  LayLT4   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 11, 1, 1, 1);
  LayLT5   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 5, 1, 1, 1);
  //LayLT6  = new TGLayoutHints(kLHintsLeft|kLHintsTop, 150, 1, 1, 0);
 
  LayLE0   = new TGLayoutHints(kLHintsLeft|kLHintsExpandY);
  LayLE1  = new TGLayoutHints(kLHintsLeft|kLHintsExpandY,3,0,0,0);

  LayLC1  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,5,5,0,0);
  LayLC2  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,0,0,0);
  LayLC3  = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,0,5,0,0);

  LayL1 = new TGLayoutHints(kLHintsLeft,1,1,0,0);
  

  LayEC3 = new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 3, 3, 0, 0);

  LayEE0 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY);
  LayEE1 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1);
  LayEE2 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,3,3);

  //fL2a  = new TGLayoutHints(kLHintsLeft|kLHintsBottom,0,0,5,0);

}
Common::~Common() {
}

/*
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
*/

//-------------------------
ColorMsgBox::ColorMsgBox(const TGWindow *p, const TGWindow *main,
			 const char *title, const char *msg, EMsgBoxIcon icon,
			 Int_t buttons, Int_t *ret_code)
  : TGTransientFrame(p, main, 10, 10)
{
  UInt_t width, height;

  //Pixel_t fBluevio;
  //fBluevio=TColor::RGB2Pixel(255,114,86);

  cout << "ColorBox: " << endl;

  TGHorizontalFrame* fButtonFrame = new TGHorizontalFrame(this, 100, 20, kFixedWidth);
  AddFrame(fButtonFrame, com->LayCB5);
  TGVerticalFrame* fLabelFrame = new TGVerticalFrame(this, 60, 20);
  AddFrame(fLabelFrame, com->LayCB5);

  TGTextButton* fOK = new TGTextButton(fButtonFrame, new TGHotString("&OK"), 1);
  //fOK->Associate(this);
  fButtonFrame->AddFrame(fOK, com->LayEC3);
  //width = TMath::Max(width, fOK->GetDefaultWidth());
  TGTextButton* fCancel = new TGTextButton(fButtonFrame, new TGHotString("&Cancel"), 2);
  //fCancel->Associate(this);
  fButtonFrame->AddFrame(fCancel, com->LayEC3);
  //    width = TMath::Max(width, fCancel->GetDefaultWidth()); ++nb;

  //width = TMath::Max(width, fOK->GetDefaultWidth());


  TGLabel *label;
  label = new TGLabel(fLabelFrame, msg);
  label->SetTextJustify(kTextCenterX);

  this->SetBackgroundColor(com->fOrng);

  fLabelFrame->AddFrame(label, com->LayCLE2);

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


Editor::Editor(const TGWindow *main, UInt_t w, UInt_t h)
{
  // Create an editor in a dialog.

  str = new TString();

  fMain = new TGTransientFrame(gClient->GetRoot(), main, w, h);
  fMain->Connect("CloseWindow()", "Editor", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.

  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  fEdit = new TGTextEdit(fMain, w, h, kSunkenFrame | kDoubleBorder);
  fL1 = new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 3, 3, 3, 3);
  fMain->AddFrame(fEdit, fL1);
  fEdit->Connect("Opened()", "Editor", this, "DoOpen()");
  fEdit->Connect("Saved()",  "Editor", this, "DoSave()");
  fEdit->Connect("Closed()", "Editor", this, "DoClose()");

  // set selected text colors
  Pixel_t pxl;
  gClient->GetColorByName("#3399ff", pxl);
  fEdit->SetSelectBack(pxl);
  fEdit->SetSelectFore(TGFrame::GetWhitePixel());

  // TGTextButton* fRead = new TGTextButton(fMain, "  &Read  ");
  // fRead->Connect("Clicked()", "Editor", this, "DoOpen()");
  // fL2 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);
  // fMain->AddFrame(fRead, fL2);

  TGTextButton* fOK = new TGTextButton(fMain, "  &OK  ");
  fOK->Connect("Clicked()", "Editor", this, "DoOK()");
  fL2 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);
  fMain->AddFrame(fOK, fL2);

  SetTitle();

  fMain->MapSubwindows();

  fMain->Resize();

  // editor covers right half of parent window
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kRight);
}

Editor::~Editor()
{
  // Delete editor dialog.

  fMain->DeleteWindow();  // deletes fMain
}

void Editor::SetTitle()
{
  // Set title in editor window.

  TGText *txt = GetEditor()->GetText();
  Bool_t untitled = !strlen(txt->GetFileName()) ? kTRUE : kFALSE;

  char title[256];
  if (untitled)
    sprintf(title, "Channel map");
  else
    sprintf(title, "%s", txt->GetFileName());

  fMain->SetWindowName(title);
  fMain->SetIconName(title);
}

void Editor::Popup()
{
  // Show editor.

  fMain->MapWindow();
}

// void Editor::LoadBuffer(const char *buffer)
// {
//   // Load a text buffer in the editor.

//   fEdit->LoadBuffer(buffer);
// }

void Editor::LoadFile(const char *file)
{
  // Load a file in the editor.
  fEdit->LoadFile(file);
}

void Editor::LoadPar()
{
  char ss[100];
  //fEdit->LoadBuffer("# Lines starting with # are comments");
  fEdit->LoadBuffer("#Ing  N X-ch Y-ch");
  for (int i=0;i<16;i++) {
    sprintf(ss,"Ing  %2d %2d %2d",i,opt.Ing_x[i],opt.Ing_y[i]);
    fEdit->AddLine(ss);
  }
  fEdit->AddLine("#Prof N X-ch Y-ch");
  //fEdit->AddLine("#");
  for (int i=0;i<8;i++) {
    sprintf(ss,"Prof %2d %2d %2d",i,opt.Prof_x[i],opt.Prof_y[i]);
    fEdit->AddLine(ss);
  }
}

void Editor::CloseWindow()
{
  // Called when closed via window manager action.

  delete this;
}

void Editor::DoOK()
{
  // Handle ok button.

  TGText* txt = fEdit->GetText();
  //cout << txt->RowCount() << endl;
  for (int i=0;i<txt->RowCount();i++) {
    char* chr = txt->GetLine(TGLongPosition(0,i),100);
    if (chr) {
      std::stringstream ss(chr);
      TString ts;
      int j,xx,yy;
      ss >> ts >> j >> xx >> yy;
      //cout << i << " " << chr << " " << ts << " " << a << " " << b << " " << c << endl;
      delete chr;
      if (ts.EqualTo("Ing",TString::kIgnoreCase) && j>=0 && j<16) {
	opt.Ing_x[j]=xx;
	opt.Ing_y[j]=yy;
      }
      else if (ts.EqualTo("Prof",TString::kIgnoreCase) && j>=0 && j<8) {
	opt.Prof_x[j]=xx;
	opt.Prof_y[j]=yy;
      }
    }
  }

  crs->Make_prof_ch();
  CloseWindow();
}

void Editor::DoOpen()
{
  SetTitle();
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
}

void Editor::DoSave()
{
  SetTitle();
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
}

void Editor::DoClose()
{
  // Handle close button.

  CloseWindow();
}
