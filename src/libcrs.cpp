#include "libcrs.h"

#include <zlib.h>

#include <sys/stat.h>
#include "cyusb.h"
//#include <pthread.h>
#include "eventframe.h"
#include "romana.h"
#include <malloc.h>
#include <TClass.h>
#include <TCanvas.h>

//#include <TSemaphore.h>
//TSemaphore sem;
#include "TThread.h"

TMutex Emut3;
TMutex stat_mut;
//TMutex ana_mut;

const int BFMAX=999999999;

TCondition* cond[CRS::MAXTRANS];

using namespace std;

#ifndef CRSTXT
extern EventFrame* EvtFrm;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern ParParDlg *parpar;
extern ChanParDlg *crspar;
extern ChanParDlg *chanpar;
extern int debug; // for printing debug messages

#endif

const double MB = 1024*1024;

//Int_t ev_max;

//bool bstart=true;
//bool btest=false;

cyusb_handle *cy_handle;
//pthread_t tid1;
TThread* trd_crs;
//TThread* trd_stat;
//TThread* trd_evt;
//TThread* trd_dum;
//TThread* trd_ana;

int event_thread_run;//=1;

//UInt_t list_min=100;

volatile char astat[CRS::MAXTRANS];

CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern int chanPresent;

//TCondition tcond1(0);

//static const char * program_name;

/*
  static void print_usage(FILE *stream, int exit_code)
  {
  fprintf(stream, "Usage: %s\n", program_name);
  exit(exit_code);
  }
*/
/***********************************************************************/

//static gzFile fp;// = stdout;
//static int timeout_provided;
static int timeout = 0;
//static cyusb_handle *h1 = NULL;


void *handle_events_func(void *ctx)
{
  while (event_thread_run) {
    libusb_handle_events_completed(NULL,NULL);
  }
  return NULL;
}

void *handle_dum(void* ptr)
{

  //gSystem->Sleep(1900);
  HiFrm->Update();      
  ////cout << "Dum: " << endl;

  return NULL;

}

void *ballast(void* xxx) {

  libusb_transfer* transfer = (libusb_transfer*) xxx;

  cout << "ballast start: " << *(int*) transfer->user_data << endl;

  double a;
  for (int i=0;i<100000000;i++) {
    a=drand48()*drand48()/drand48();
    a++;
  }

  cout << "ballast stop: " << *(int*) transfer->user_data << endl;

  return NULL;

}

void *handle_buf(void *ctx)
{

  int* nmax = (int*) ctx; 
  //cout << "handle_buf: " << *nmax << endl; 

  int i=1;
  while (crs->Do1Buf() && crs->b_fana && i<*nmax) {
    i++;
  }

  //cout << "buf: " << crs->nbuffers << " " << i << endl;
  crs->b_stop=true;
  return NULL;
}

void *handle_ana(void* ptr) {
  // when the event list (Levents) becomes larger than ev_min,
  // starts analysing the events (fillhist);
  // when it becomes larger than ev_max, starts erasing the events

  //ana_mut.Lock();

  //std::list<event_list>::iterator Lit;
  std::list<EventClass>::iterator it;

  // start - first event which is NOT analyzed yet
  std::list<EventClass>::iterator start = crs->Levents.begin();
  // n_ana - number of events which are already analyzed, starting from "start"
  int n_ana=0;

  MemInfo_t info;
  gSystem->GetMemInfo(&info);
  Int_t memmax=info.fMemTotal*0.05*1024;

  //int *pp = (int*) ptr;

  //while (event_thread_run) {
  while (!crs->b_stop) {

    // cout << "ev_max: " << crs->Levents.size() << " " << opt.ev_max
    // 	 << " " << opt.ev_max*opt.rbuf_size << " Mb"
    // 	 << " " << memmax << " " << info.fMemTotal
    // 	 << " " << opt.ev_min << " " << opt.ev_max << endl;

    // if (opt.ev_max*TMath::Max(opt.rbuf_size,opt.usb_size) > memmax) {
    //   opt.ev_max=memmax/TMath::Max(opt.rbuf_size,opt.usb_size);
    //   opt.ev_min=opt.ev_max/2;
    // }
    if (opt.ev_min>=opt.ev_max) {
      opt.ev_min=opt.ev_max/2;
    }
    //opt.ev_max=opt.ev_min+5;

    int sz=crs->Levents.size();
    if (sz-n_ana>opt.ev_min) {
    //if (false) {
      //cout << "handle_ana: " << ptr << " " << crs->Levents.size() << endl;

      int nn = sz-n_ana-opt.ev_min;

      for (it=start; it!=crs->Levents.end() && nn>0; ++it) {
	--nn;
      }
      crs->m_event=it;

      for (it=start; it!=crs->m_event; ++it) {
	if (it->pulses.size()>=opt.mult1 && it->pulses.size()<=opt.mult2) {
	  HiFrm->FillHist(&(*it));
	  ++crs->nevents2;
	}
      }
      n_ana+=nn;
      start=crs->m_event;

      if (sz>opt.ev_max) {
	for (it=crs->Levents.begin(); it!=crs->m_event; ++it) {
	  crs->Levents.erase(it);
	  --n_ana;
	}
      }
    }
    else {
      gSystem->Sleep(500);
    }
  }

  //cout << "Ana2: " << nn << " " << crs->Levents.size() << endl;
  // << " " << (--rl)->Nevt << endl;

  //YKYK if (crs->m_flag!=2)
  //YKYK   crs->Levents.erase(crs->Levents.begin(),crs->m_event);

  //crs->m_flag=0;

  //cout << "ev_max3: " << crs->nevents2 << " " << crs->Levents.size() << " "
  // << crs->Levents.begin()->T << endl;
  //cout << "Ana2: " << crs->nevents2 << " " << crs->Levents.size() << endl;
  //gSystem->Sleep(200);


  //ana_mut.UnLock();
  return 0;
    
}

/*
  void *decode2(void* xxx) {
  libusb_transfer* transfer = (libusb_transfer*) xxx;
  crs->Decode2(transfer);
  return NULL;
  }
*/

static void cback(libusb_transfer *transfer) {

  //static TTimeStamp t1;

  //cout << "cback: " << endl;
  //return;

  //TTimeStamp t2;
  //t2.Set();

  crs->npulses_buf=0;

  if (crs->b_acq) {

  if (transfer->actual_length) {
    if (opt.decode) {
      if (crs->module==2) {
	crs->Decode2(transfer->buffer,transfer->actual_length);
      }
      else if (crs->module==32) {
	crs->Decode32(transfer->buffer,transfer->actual_length);
      }
    }
    if (opt.raw_write) {
      //cout << "raw_start: " << *(int*) transfer->user_data << endl;
      //crs->f_raw = gzopen(opt.fname_raw,"wb0");
      crs->f_raw = gzopen(opt.fname_raw,crs->raw_opt);
      //cout << "raw_opt: " << crs->raw_opt << endl;
      if (crs->f_raw) {
	//cout << "cback tell: " << gztell(crs->f_raw) << endl;
	int res=gzwrite(crs->f_raw,transfer->buffer,transfer->actual_length);
	//cout << "cback offset: " << gzoffset(crs->f_raw) << endl;
	gzclose(crs->f_raw);
	crs->writtenbytes+=res;
      }
      else {
	cout << "Can't open file: " << opt.fname_raw << endl;
      }
      //cout << "raw_stop: " << *(int*) transfer->user_data << endl;
    }

    crs->nbuffers++;

  }

    libusb_submit_transfer(transfer);

    stat_mut.Lock();
    crs->totalbytes+=transfer->actual_length;
    //opt.T_acq = t2.GetSec()-opt.F_start.GetSec()+
    //(t2.GetNanoSec()-opt.F_start.GetNanoSec())*1e-9;
    opt.T_acq = (Long64_t(gSystem->Now()) - opt.F_start)*0.001;

    stat_mut.UnLock();
  }

  //crs->nvp = (crs->nvp+1)%crs->ntrans;
  
}

CRS::CRS() {


  //cout << TClass::GetClass("Toptions")->GetClassVersion() << endl;
  //exit(1);
  /*
    std::list<int> mylist;
    std::list<int>::iterator it;
    std::list<int>::reverse_iterator rit;

    // set some initial values:
    for (int i=1; i<=5; ++i) mylist.push_back(i); // 1 2 3 4 5

    rit=mylist.rbegin();
    mylist.insert(rit.base(),10);

    for (rit=mylist.rbegin(); rit!=mylist.rend(); ++rit)
      std::cout << ' ' << *rit;
    std::cout << '\n';

    --rit;
    std::cout << *rit << endl;
    
    mylist.insert(mylist.begin(),0);
    //mylist.clear();
    //mylist.insert(mylist.begin(),21);
    std::cout << *rit << " " << mylist.back() << " " << *mylist.begin() << endl;

    std::cout << "mylist contains:";
    for (it=mylist.begin(); it!=mylist.end(); ++it)
      std::cout << ' ' << *it;
    std::cout << '\n';

    rit=++mylist.rbegin();
    mylist.insert(rit.base(),29);

    std::cout << "mylist contains:";
    for (it=mylist.begin(); it!=mylist.end(); ++it)
      std::cout << ' ' << *it;
    std::cout << '\n';

    it=--mylist.end();

    advance(it,-8);
    cout << *mylist.rbegin() << " " << *(--mylist.rend()) << " "
	 << *rit << " " << *rit.base() << " " << *it << endl;
    
    for (it=--mylist.end(); it!=mylist.begin(); --it) {
      std::cout << ' ' << *(it);
    }
    std::cout << '\n';

    std::cout << "end: " << *(it) << endl;



    std::list<int> Levents;
    std::list<int>::iterator rl;
    //std::list<int>::reverse_iterator rit;

    // set some initial values:
    for (int i=0; i<=10; i+=2) Levents.push_back(i); // 1 2 3 4 5

    std::cout << "Levents contains:" << endl;

    for (rl=Levents.begin(); rl!=Levents.end(); ++rl)
      std::cout << ' ' << *rl;
    std::cout << '\n';


    int kk=7;

    for (rl=--Levents.end();rl!=Levents.begin();--rl) {
      
      if (*rl < kk) {
	//add new event at the current position of the eventlist
	Levents.insert(++rl,kk);
	cout << "i: " << *rl << endl;
	break;
      }
      cout << " : " << *rl << endl;
    }
    

    for (rl=Levents.begin(); rl!=Levents.end(); ++rl)
      std::cout << ' ' << *rl;
    std::cout << '\n';
    

    exit(1);
  */

  //ev_max=2*opt.ev_min;

  MAXTRANS2=MAXTRANS;
  memset(Pre,0,sizeof(Pre));

  for (int i=0;i<MAXTRANS;i++)
    cond[i]=new TCondition(0);

  Fmode=0;
  period=5;

  f_raw=0;
  f_read=0;
  f_dec=0;

  b_acq=false;
  b_fana=false;
  b_stop=true;

  strcpy(raw_opt,"ab");
  strcpy(dec_opt,"ab");

  Fbuf=NULL;
  Fbuf2=0;

  // if (Fbuf2)
  //   cout << "Fbuf2: " << Fbuf2 << endl;
  // else
  //   cout << "Fbuf3: " << Fbuf2 << endl;
  
  strcpy(Fname," ");
  DoReset();

  module=0;

  cy_handle=NULL;

  event_thread_run=1;

  // b_acq=false;
  // b_fana=false;
  // bstart=true;

  chanPresent=32;

  ntrans=MAXTRANS;
  //opt.usb_size=1024*1024;

  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] =NULL;
    buftr[i]=NULL;
  }

  cout << "creating threads... " << endl;

  //trd_stat = new TThread("trd_stat", handle_stat, (void*) 0);
  //trd_stat->Run();
  //trd_evt = new TThread("trd_evt", handle_evt, (void*) 0);
  //trd_evt->Run();
  //trd_ana = new TThread("trd_ana", Ana_Events, (void*) 0);
  //trd_ana->Run();

  //mTh= new TThread("memberfunction",
  //(void(*)(void *))&Thread0,(void*) this);
 
  cout << "threads created... " << endl;

}

CRS::~CRS() {
  //b_acq=true;
  /*
    if (b_acq) {
    DoStartStop();
    }
  */

  //if (opt.raw_write) {
  //gzclose(f_raw);
  //}  

  DoExit();
  cout << "~CRS()" << endl;

}

/*
void CRS::Dummy_trd() {
  trd_dum = new TThread("trd_dum", handle_dum, (void*) 0);
  trd_dum->Run();
}
*/

int CRS::Detect_device() {

  int r;

  module=0;
  chanPresent=32;

  /*
    for (int i=0;i<ntrans;i++) {
    if (transfer[i]) {
    cout << "transfer : " << i << " " << transfer[i] << endl;
    libusb_free_transfer(transfer[i]);
    }
    }
  */

  //int ch=getche();
  //sleep(1000);

  r = cyusb_open();
  if ( r < 0 ) {
    printf("Error opening library\n");
    return 1;
  }
  else if ( r == 0 ) {
    printf("No device found\n");
    return 2;
  }
  if ( r > 1 ) {
    printf("More than 1 devices of interest found. Disconnect unwanted devices\n");
    return 3;
  }

  cy_handle = cyusb_gethandle(0);
  if ( cyusb_getvendor(cy_handle) != 0x04b4 ) {
    printf("Cypress chipset not detected\n");
    cyusb_close();
    return 4;
  }

  r=cyusb_reset_device(cy_handle);
  if ( r != 0 ) {
    printf("Can't reset device. Exitting\n");
    cyusb_close();
    return 5;
  }

  r = cyusb_kernel_driver_active(cy_handle, 0);
  if ( r != 0 ) {
    printf("kernel driver active. Exitting\n");
    cyusb_close();
    return 6;
  }
  r = cyusb_claim_interface(cy_handle, 0);
  if ( r != 0 ) {
    printf("Error in claiming interface\n");
    cyusb_close();
    return 7;
  }
  else printf("Successfully claimed interface\n");

  //int r1 = pthread_create(&tid1, NULL, handle_events_func, NULL);
  //if (r1) {
  //cout << "Error creating thread: " << r1 << endl;
  //}

  trd_crs = new TThread("trd_crs", handle_events_func, (void*) 0);
  trd_crs->Run();

  // trd_stat = new TThread("trd_stat", handle_stat, (void*) 0);
  // trd_stat->Run();

  // trd_evt = new TThread("trd_evt", handle_evt, (void*) 0);
  // trd_evt->Run();

  cout << "CRS thread created... " << endl;

  //memset(buf_out,'\0',64);
  //memset(buf_in,'\0',64);

  //buf_out[0] = 1; //get card info
  Command2(4,0,0,0);

  Command32(1,0,0,0);

  /*
    for (int i=0;i<6;i++) {
    cout << int(buf_in[i]) << " ";
    }
    cout << endl;
  */

  if (buf_in[1]==3) {
    module=2;
    chanPresent=2;
  }
  else {
    module=32;
    chanPresent=buf_in[3]*4;
  }

  cout << "module: " << module << " chanPresent: " << chanPresent << endl;

  cpar.InitPar(module);

  // for (int i=0;i<10000;i++) {
  //   Init_Transfer();
  //   Free_Transfer();
  // }


  //Init_Transfer();
  //Free_Transfer();
  //Init_Transfer();
  //Init_Transfer();
  //Init_Transfer();


  /*
  Init_Transfer();
  gSystem->Sleep(50);
  Cancel_all(MAXTRANS);
  gSystem->Sleep(50);
  r=cyusb_reset_device(cy_handle);
  cout << "cyusb_reset: " << r << endl;
  Submit_all(MAXTRANS);
  gSystem->Sleep(50);
  */

  if (Init_Transfer()) {
    return 8;
  };

  //Submit_all(MAXTRANS);

  Command2(4,0,0,0);

  return 0;

}

void CRS::DoExit()
{
  cout << "CRS::DoExit" << endl;

  event_thread_run=0;
  cyusb_close();
  if (trd_crs) {
    trd_crs->Delete();
  }
  //if (trd_stat) {
  //trd_stat->Delete();
  //}
  //if (trd_evt) {
  //trd_evt->Delete();
  //}
  //if (trd_ana) {
  //trd_ana->Delete();
  //}
  //pthread_join(tid1,NULL);
  //gzclose(fp);
  //exit(-1);
}

int CRS::SetPar() {

  if (module==2) {
    AllParameters2();
  }
  else if (module==32) {
    AllParameters32();
  }
  else {
    cout << "InitPar Error! No module found" << endl;
    return 3;
  }

  return 0;

  //threshold[11]=100;

}

void CRS::Free_Transfer() {

  Cancel_all(MAXTRANS);
  gSystem->Sleep(50);

  //cout << "---Free_Transfer---" << endl;

  //for (int i=0;i<ntrans;i++) {
  for (int i=0;i<MAXTRANS;i++) {
    cout << "free: " << i << " " << (int) transfer[i]->flags << endl;
    //int res = libusb_cancel_transfer(transfer[i]);
    libusb_free_transfer(transfer[i]);

    //transfer[i]=NULL;

    //buftr[i]=NULL;

  }

  for (int i=0;i<MAXTRANS;i++) {
    if (buftr[i]) {
      delete[] buftr[i];
      buftr[i]=NULL;
    }
  }
  gSystem->Sleep(50);

}

void CRS::Submit_all(int ntr) {
  ntrans=0;
  for (int i=0;i<ntr;i++) {
    int res;
    res = libusb_submit_transfer(transfer[i]);
    //cout << i << " Submit: " << res << endl;
    if (res) {
      cout << "Submit_Transfer error: " << res << " " << *(int*) transfer[i]->user_data << endl;
      cout << libusb_error_name(res) << endl;
      break;
    }
    else {
      //cout << "Submit_Transfer: " << res << " " << *(int*) transfer[i]->user_data << endl;
      ntrans++;
    }
  }
}

void CRS::Cancel_all(int ntr) {
  for (int i=0;i<ntr;i++) {
    //int res;
    if (transfer[i]) {
      //res = 
      libusb_cancel_transfer(transfer[i]);
      //cout << i << " Cancel: " << res << endl;
    }
  }
}

int CRS::Init_Transfer() {

  //cout << "---Init_Transfer---" << endl;

  //Cancel_all(MAXTRANS);

  //cout << "---Init_Transfer2---" << endl;
  //int r=
  cyusb_reset_device(cy_handle);
  //cout << "cyusb_reset: " << r << endl;

  for (int i=0;i<MAXTRANS;i++) {
    //if (buftr[i]) {
    //delete[] buftr[i];
    //}
    buftr[i] = new unsigned char[opt.usb_size*1024];
    memset(buftr[i],0,sizeof(unsigned char)*opt.usb_size*1024);
  }

  //cout << "---Init_Transfer 3---" << endl;
  ntrans=0;
  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] = libusb_alloc_transfer(0);
    //transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;
    
    int* ntr = new int;
    (*ntr) = i;

    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], opt.usb_size*1024, cback, ntr, 0);

    /*
    int res;
    res = libusb_submit_transfer(transfer[i]);

    if (res) {
      cout << "Init_Transfer error: " << res << " " << *(int*) transfer[i]->user_data << endl;
      cout << libusb_error_name(res) << endl;
      break;
    }
    else {
      ntrans++;
    }
    */

  }

  if (opt.usb_size>1024) {
    MAXTRANS2=MAXTRANS-1;
  }
  else {
    MAXTRANS2=MAXTRANS;
  }

  Submit_all(MAXTRANS2);

  /*
  for (int i=0;i<ntrans;i++) {
    int res;
    res = libusb_cancel_transfer(transfer[i]);
    cout << i << " Cancel: " << res << endl;
  }
  */
  cout << "Number of transfers: " << ntrans << endl;

  if (ntrans!=MAXTRANS2) {
    for (int i=ntrans;i<MAXTRANS2;i++) {
      //cout << "free: " << i << endl;
      //libusb_free_transfer(transfer[i]);
      cout << "delete: " << i << endl;
      if (buftr[i])
	delete[] buftr[i];
      buftr[i]=NULL;
    }
    return 2;
  }
  gSystem->Sleep(250);

  Cancel_all(MAXTRANS);
  gSystem->Sleep(250);

  return 0;

}

void CRS::Command32(byte cmd, byte ch, byte type, int par) {
  int transferred = 0;
  int r;

  int len_in,len_out;

  //byte buf_out[6];//={0};
  //byte buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (byte) (par >> 16);
  buf_out[4] = (byte) (par >> 8);
  buf_out[5] = (byte) par;

  switch (cmd) {
  case 1:
    len_out=1;
    len_in=5;
    break;
  case 2:
    len_out=6;
    len_in=1;
    break;
  case 3:
  case 4:
    len_out=1;
    len_in=1;
    break;
  case 5:
    len_out=2;
    len_in=7;
    break;
  case 6:
  case 7:
    len_out=1;
    len_in=2;
    break;
  default:
    cout << "Wrong CRS32 command: " << cmd << endl;
    return;
  }
  
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, timeout * 1000);
  if (r) {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  if (cmd!=7) {
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, timeout * 1000);
    if (r) {
      printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      //cyusb_close();
    }
  }

}

void CRS::Command2(byte cmd, byte ch, byte type, int par) {
  int transferred = 0;
  int r;

  int len_in,len_out;

  //byte buf_out[6];//={0};
  //byte buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[4] = (byte) (par >> 8);
  buf_out[3] = (byte) par;

  switch (cmd) {
  case 1:
  case 3:
  case 4:
    len_out=1;
    break;
  case 2:
    len_out=5;
    break;
  default:
    cout << "Wrong CRS2 command: " << cmd << endl;
    return;
  }

  len_in=2;

  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, timeout * 1000);
  if (r) {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, timeout * 1000);
  if (r) {
    printf("Error7! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

}

/*
int CRS::Command_old(int len_out, int len_in) {
  int transferred = 0;
  int r;

  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, timeout * 1000);
  if ( r == 0 ) {
  }
  else {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    cyusb_close();
    return 0;
  }

  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, timeout * 1000);
  if ( r == 0 ) {
  }
  else {
    printf("Error7! %d: \n",buf_out[1]);
    cyusb_error(r);
    cyusb_close();
    return 0;
  }

  return 1;
}
*/

/*
void CRS::SendParametr(const char* name, int len_out) {
  //int len_in=6; //input length must be at least 2, not 1
  //int len_out = 6;

  if (Command(len_out,6)) {
    if (debug) {
      printf("Chan: %2d Par: %3d %3d %3d %3d Answer: %d %s\n",
	     buf_out[1],buf_out[2],buf_out[3],buf_out[4],buf_out[5],
	     buf_in[0],name);
    }
  }

}
*/

void CRS::Command_crs(byte cmd, byte chan, int par) {

  if (module==32) {
    switch (cmd) {
    case 1: //enabl
      if (cpar.enabl[chan]) {
	Command32(2,chan,6,(int)cpar.kderiv[chan]);
	Command32(2,chan,7,(int)cpar.threshold[chan]);
      }
      else {
	int tmp,max;
	cpar.GetPar("thresh",module,chan,tmp,tmp,max);
	//cout << "Off: " << (int) chan << " " << max << endl;
	Command32(2,chan,6,0);
	Command32(2,chan,7,max);
      }
      break;
    case 2: //inv
      Command32(2,chan,1,(int)cpar.inv[chan]);
      break;
    case 3: //acdc
      Command32(2,chan,0,(int)cpar.acdc[chan]);
      break;
    case 4: //smooth
      Command32(2,chan,2,(int)cpar.smooth[chan]);
      break;
    case 5: //Dt
      Command32(2,chan,3,(int)cpar.deadTime[chan]);
      break;
    case 6: //Pre
      Command32(2,chan,4,(int)cpar.preWr[chan]);
      break;
    case 7: //Len
      Command32(2,chan,5,(int)cpar.durWr[chan]);
      break;
    case 8: //Gain
      Command32(2,chan,8,(int)cpar.adcGain[chan]);
      break;
    case 9: //Drv
      Command32(2,chan,6,(int)cpar.kderiv[chan]);
      break;
    case 10: //thresh
      Command32(2,chan,7,(int)cpar.threshold[chan]);
      break;
    }
  }
  else if (module==2) {
    switch (cmd) {
    case 1: //enabl
      if (cpar.enabl[chan]) {
	Command2(2,chan,3,(int)cpar.kderiv[chan]);
	Command2(2,chan,4,(int)cpar.threshold[chan]);
      }
      else {
	int tmp,max;
	cpar.GetPar("thresh",module,chan,tmp,tmp,max);
	//cout << "Off: " << (int) chan << " " << max << endl;
	Command2(2,chan,3,0);
	Command2(2,chan,4,max);
      }
      break;
    case 2: //inv
      Command2(2,chan,1,(int)cpar.inv[chan]);
      break;
    case 3: //acdc
      //no acdc
      break;
    case 4: //smooth
      Command2(2,chan,2,(int)cpar.smooth[chan]);
      break;
    case 5: //Dt
      //no Dt
      break;
    case 6: //Pre
      Command2(2,chan,5,(int)cpar.preWr[chan]);
      break;
    case 7: //Len
      Command2(2,chan,6,(int)cpar.durWr[chan]);
      break;
    case 8: //Gain
      Command2(2,chan,0,(int)cpar.adcGain[chan]);
      break;
    case 9: //Drv
      Command2(2,chan,3,(int)cpar.kderiv[chan]);
      break;
    case 10: //thresh
      Command2(2,chan,4,(int)cpar.threshold[chan]);
      break;
    }
  }

}

void CRS::AllParameters32()
{

  for (byte chan = 0; chan < chanPresent; chan++) {
    Command32(2,chan,0,(int)cpar.acdc[chan]);
    Command32(2,chan,1,(int)cpar.inv[chan]);
    Command32(2,chan,2,(int)cpar.smooth[chan]);
    Command32(2,chan,3,(int)cpar.deadTime[chan]);
    Command32(2,chan,4,(int)cpar.preWr[chan]);
    Command32(2,chan,5,(int)cpar.durWr[chan]);
    if (cpar.enabl[chan]) {
      Command32(2,chan,6,(int)cpar.kderiv[chan]);
      Command32(2,chan,7,(int)cpar.threshold[chan]);
    }
    else {
      int tmp,max;
      cpar.GetPar("thresh",module,chan,tmp,tmp,max);
      //cout << "Off: " << (int) chan << " " << max << endl;
      Command32(2,chan,6,0);
      Command32(2,chan,7,max);
    }
    Command32(2,chan,8,(int)cpar.adcGain[chan]);
  }

}

void CRS::AllParameters2()
{

  for (byte chan = 0; chan < chanPresent; chan++) {
    Command2(2,chan,0,(int)cpar.adcGain[chan]);
    Command2(2,chan,1,(int)cpar.inv[chan]);
    Command2(2,chan,2,(int)cpar.smooth[chan]);
   if (cpar.enabl[chan]) {
      Command2(2,chan,3,(int)cpar.kderiv[chan]);
      Command2(2,chan,4,(int)cpar.threshold[chan]);
    }
   else {
     int tmp,max;
     cpar.GetPar("thresh",module,chan,tmp,tmp,max);
     //cout << "Off: " << (int) chan << " " << max << endl;
     Command2(2,chan,3,0);
     Command2(2,chan,4,max);
   }

   Command2(2,chan,5,(int)cpar.preWr[chan]);
   Command2(2,chan,6,(int)cpar.durWr[chan]);
  }

  Command2(2,0,7,(int)cpar.forcewr);

}

int CRS::DoStartStop() {
  int r;
  //int transferred = 0;
  //int len=2; //input/output length must be 2, not 1

  if (f_read) {
    gzclose(f_read);
    f_read=0;
  }
  
  if (!b_acq) { //start
    DoReset();

    parpar->Update();
    crspar->Update();
    chanpar->Update();

    //EvtFrm->Levents = &Levents;
    EvtFrm->Clear();
    EvtFrm->Pevents = &EvtFrm->Tevents;

    //if (module==32) {
    //Command32(7,0,0,0); //reset usb command
    //}
    
    r=cyusb_reset_device(cy_handle);
    cout << "cyusb_reset: " << r << endl;

    Submit_all(ntrans);
    period=5; //5ns for CRS module

    if (SetPar()) {
      return 3;
    }

    //buf_out[0]=3;
    b_acq=true;
    b_stop=false;
    //bstart=true;
    //totalbytes=0;
    //writtenbytes=0;
    //b_pevent=true;
    
    //npulses=0;
    //nevents=0;
    //nbuffers=0;

    //Nsamp=0;
    //nsmp=0;

    opt.F_start = gSystem->Now();
    if (opt.raw_write) {
      sprintf(raw_opt,"wb%d",opt.raw_compr);
      struct stat buffer;
      cout << "stat: " << stat (opt.fname_raw, &buffer) << endl;
      //if (stat (opt.fname_raw, &buffer)) {
	//UShort_t mod=module;



        f_raw = gzopen(opt.fname_raw,raw_opt);
	if (f_raw) {
	  cout << "Writing options... : " << opt.fname_raw << endl;
	  SaveParGz(f_raw);
	  gzclose(f_raw);
	}
	else {
	  cout << "Can't open file: " << opt.fname_raw << endl;
	}



	//}
      sprintf(raw_opt,"ab%d",opt.raw_compr);
    }   

    //nvp=0;
    //Levents.clear();

    cout << "Acquisition started" << endl;
    //gettimeofday(&t_start,NULL);

    TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
    cv->SetEditable(false);
    
    TThread* trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);;
    trd_ana->Run();

    Command2(3,0,0,0);

    while (!crs->b_stop) {
      Show();
      gSystem->Sleep(10);   
      gSystem->ProcessEvents();
    }

    trd_ana->Join();
    trd_ana->Delete();

    gSystem->Sleep(opt.tsleep);   
    Show();
    cv->SetEditable(true);
  }
  else { //stop
    buf_out[0]=4;
    b_acq=false;
    cout << "Acquisition stopped" << endl;
    //gettimeofday(&t_stop,NULL);
    /*
    for (int i=0;i<ntrans;i++) {
      int res;
      res = libusb_cancel_transfer(transfer[i]);
      cout << i << " Cancel: " << res << endl;
    }
    */
    //if (opt.raw_write) {
    //gzclose(f_raw);
    //}
    //m_flag=2;
    Command2(4,0,0,0);

    gSystem->Sleep(300);

    //trd_ana->Run(&m_flag);

    Cancel_all(ntrans);
    b_stop=true;
    EvtFrm->Clear();
    EvtFrm->Pevents = &Levents;
    //Select_Event();
    //EvtFrm->Levents = &Levents;
  }

  /*
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len, &transferred, timeout * 1000);
  if ( r ) {
    printf("Error start_stop out:\n");
    cyusb_error(r);
    cyusb_close();
    return 1;
  }

  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len, &transferred, timeout * 1000);
  if ( r == 0 ) {
    if (debug>=3) {
      printf("Answer: %d\n",buf_in[0]);
      //printf("Answer: %d\n",buf_in[0]);
    }
    //printf("Answer: %d\n",buf[0]);
  }
  else {
    printf("Error start_stop in:\n");
    cyusb_error(r);
    cyusb_close();
    return 2;
  }

  // if (!b_acq) { //stop
  //   for (int i=0;i<ntrans;i++) {
  //     //int res= 
  //     libusb_cancel_transfer(transfer[i]);
  //     //cout << "Cancel: " << res << endl;
  //   }
  // }

  //cout << 999 << endl;
  */
  return 0;

}


void CRS::DoReset() {

  cout << "DoReset1: " << b_stop << endl;

  if (!b_stop) return;
    
  opt.T_acq=0;

  Tstart64=0;
  Tstart0=0;
  T_last=0;
  //cout << "crs::reset: " << endl;
  //cout << "crs::reset: " << (int) CRS::b_fana << endl;
  //exit(1);

  //b_acq=false;
  //b_fana=false;
  //b_stop=true;

  //bstart=true;

  //nvp=0;
  Levents.clear();
  //cout << "EvtFrm: " << EvtFrm << endl;
  if (EvtFrm) {
    EvtFrm->DoReset();
  }

  m_event=Levents.end();
  //m_event2=m_event;
  m_flag=0;
  nevents=0;
  nevents2=0;

  // if (opt.ev_max<=opt.ev_min) {
  //   opt.ev_max=opt.ev_min*2;
  // }
  // list_min=opt.ev_max-opt.ev_min;

  // for (int i=0;i<MAXTRANS;i++) {
  //   Vpulses[i].clear();
  // }
  Vpulses.clear();

  if (Fmode!=1) {
    memcpy(&Pre,&cpar.preWr,sizeof(Pre));
  }

  npulses=0;
  nbuffers=0;

  //npulses=0;
  npulses_buf=0;

  totalbytes=0;
  writtenbytes=0;

  //MAX_LAG=opt.event_buf/2;

  idx=0;
  idnext=0;
  lastfl=1;

  cout << "f_read: " << f_read << endl;
  if (f_read)
    DoFopen(NULL,0);

  // parpar->Update();
  // crspar->Update();
  // chanpar->Update();

  //gzrewind(f_raw);

  //if (HiFrm)
  //  cout << "DoReset2: " << HiFrm->h_time[1]->GetName() << endl;

}

void CRS::DoFopen(char* oname, int popt) {
  //popt: 1 - read opt from file; 0 - don't read opt from file
  int tp=0; //1 - adcm raw; 0 - crs2/32
  int bsize;
  int boffset;

  //cout << "Select77: " << crs->m_event->T << " "
  //     << crs->Levents.begin()->T << endl;
  //Print_Events();

  // if (opt.ev_max<=opt.ev_min) {
  //   opt.ev_max=opt.ev_min*2;
  // }

  if (oname)
    strcpy(Fname,oname);

  cout << "DoFopen: " << Fname << endl;

  if (TString(Fname).EqualTo(" ",TString::kIgnoreCase)) {
    return;
  }

  char dir[100], name[100], ext[100];
  SplitFilename(string(Fname),dir,name,ext);

  if (TString(ext).EqualTo("dat",TString::kIgnoreCase)) {
    tp=1;
  }
  else if (TString(ext).EqualTo("gz",TString::kIgnoreCase)) {
    tp=0;
  }
  else {
    cout << "Unknown file type (extension): " << Fname << endl;
    if (f_read) gzclose(f_read);
    f_read=0;
  }

  //cout << ext << endl;
  if (f_read) gzclose(f_read);
  f_read = gzopen(Fname,"rb");
  if (!f_read) {
    Fmode=0;
    cout << "Can't open file: " << Fname << endl;
    f_read=0;
  }

  if (tp) { //adcm raw
    cout << "ADCM RAW File: " << Fname << endl;
    Fmode=1;

    memset(Pre,0,sizeof(Pre));
    // for (int i=0;i<MAX_CH+ADDCH;i++) {
    //   cpar.preWr[i]=0;
    // }
    bsize=opt.rbuf_size*1024+4096;
    boffset=4096;
    period=10;
  }
  else { //crs32 or crs2
    if (ReadParGz(f_read,Fname,1,1,popt)) {
      gzclose(f_read);
      f_read=0;
      return;
    }
    bsize=opt.rbuf_size*1024;
    boffset=0;
    period=5;
  }

  //list_min=opt.ev_max-opt.ev_min;

  opt.raw_write=false;

  //cout << "smooth 0-39: " << cpar.smooth[39] << " " << opt.smooth[39] << endl;

  //cout << "Fopen2: " << (void*) Fbuf2 << " " << bsize << " " << boffset << endl;

  if (Fbuf2) {
    delete[] Fbuf2;
  }
  //cout << "Fopen3: " << (void*) Fbuf2 << " " << bsize << endl;
  Fbuf2 = new UChar_t[bsize];
  Fbuf = Fbuf2+boffset;
  memset(Fbuf2,0,boffset);

  rbuf4 = (UInt_t*) Fbuf;
  rbuf2 = (UShort_t*) Fbuf;


  // if (Fmode==1) {//adcm raw file
  //   if (Searchsync()) {
  //     cout<<Fname<<": Sync word in ADCM RAW file not found. File is closed."
  // 	  <<endl;
  //     gzclose(f_read);
  //     f_read=0;
  //     Fmode=0;
  //   }
  // }

  //EvtFrm->Levents = &Levents;

  // parpar->Update();
  // crspar->Update();
  // chanpar->Update();

  //parpar->Update();

  //cout << f_raw << endl;

  //cout << "smooth: " << cpar.smooth[0] << endl;
  //cout << "DoFopen2: " << f_read << " " << Fmode << endl;
  if (myM)
    myM->SetTitle(Fname);
}

int CRS::ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2) {
  //m1 - read Fmode (1/0); p1 - read cpar (1/0); p2 - read opt (1/0)
  UShort_t sz;

  UShort_t mod;
  gzread(ff,&mod,sizeof(mod));

  cout << "Initial Fmode: " << Fmode << endl;

  if (m1) {
    if (mod==2) {
      Fmode=2;
      cout << "CRS2 File: " << Fname << " " << mod << endl;
    }
    else if (mod==32) {
      Fmode=32;
      cout << "CRS32 File: " << Fname << " " << mod << endl;
    }
    else {
      Fmode=0;
      cout << "Unknown file type: " << Fname << " " << Fmode << endl;
      return 1;
    }
  }

  gzread(ff,&sz,sizeof(sz));

  //cout << "readpar_gz1: " << sz << endl;

  char buf[100000];
  gzread(ff,buf,sz);

  if (p1) 
    BufToClass("Coptions",(char*) &cpar, buf, sz);
  if (p2)
    BufToClass("Toptions",(char*) &opt, buf, sz);

  cout << "ReadParGz: " << sz << " " << pname << endl;
  //opt.raw_write=false;

  //if (Fbuf) delete[] Fbuf;
  //Fbuf = new UChar_t[opt.usb_size*1024];
  //EvtFrm->Levents = &Levents;

  if (Fmode!=1) {
    memcpy(&Pre,&cpar.preWr,sizeof(Pre));
  }

  //ev_max=2*opt.ev_min;

  //list_min=opt.ev_max-opt.ev_min;
  //cout << "readpar_gz3" << endl;
  return 0;
}

void CRS::SaveParGz(gzFile &ff) {

  char buf[100000];
  UShort_t sz=0;

  sz+=ClassToBuf("Coptions",(char*) &cpar, buf+sz);
  sz+=ClassToBuf("Toptions",(char*) &opt, buf+sz);

  gzwrite(ff,&module,sizeof(module));
  gzwrite(ff,&sz,sizeof(sz));
  gzwrite(ff,buf,sz);

  //cout << "SavePar_gz: " << sz << endl;

}

void CRS::FAnalyze() {

  //cout << "FAnalyze: " << f_read << endl;
    
  static int nmax=BFMAX-1;

  TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
  cv->SetEditable(false);
  TThread* trd_fana = new TThread("trd_fana", handle_buf, (void*) &nmax);;
  trd_fana->Run();
  TThread* trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);;
  trd_ana->Run();
  while (!crs->b_stop) {
    Show();
    gSystem->Sleep(10);   
    gSystem->ProcessEvents();
  }
  trd_fana->Join();
  trd_fana->Delete();
  trd_ana->Join();
  trd_ana->Delete();

  gSystem->Sleep(opt.tsleep);   
  Show();

  cv->SetEditable(true);
}

int CRS::Do1Buf() {

  BufLength=gzread(f_read,Fbuf,opt.rbuf_size*1024);
  //cout << "gzread: " << Fmode << " " << nbuffers << " " << BufLength << endl;
  if (BufLength>0) {
    crs->totalbytes+=BufLength;

    if (opt.decode) {
      if (Fmode==32) {
	Decode32(Fbuf,BufLength);
      }
      else if (Fmode==2) {
	Decode2(Fbuf,BufLength);
      }
      else if (Fmode==1) {
	BufLength/=sizeof(UInt_t);
	Decode_adcm();
      }
    }

    //gSystem->Sleep(500);

    //Select_Event();

    /*
    if (myM && myM->fTab->GetCurrent()==EvtFrm->ntab) {
      EvtFrm->DrawEvent2();      
    }

    if (myM && myM->fTab->GetCurrent()==HiFrm->ntab) {
      //HiFrm->DrawHist();      
      HiFrm->ReDraw();
    }
    */
    nbuffers++;
    //myM->UpdateStatus();
    //gSystem->ProcessEvents();

    //nvp = (nvp+1)%ntrans;

    return nbuffers;
  }
  else {
    //b_fana=true;
    //myM->DoAna();
    return 0;
  }


}

void CRS::DoNBuf() {

  //cout << "FAnalyze: " << f_read << endl;
    
  //static int nmax=opt.num_buf;

  TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
  cv->SetEditable(false);

  TThread* trd_fana = new TThread("trd_fana", handle_buf, (void*) &opt.num_buf);;
  trd_fana->Run();
  TThread* trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);;
  trd_ana->Run();
  while (!crs->b_stop) {
    Show();
    gSystem->Sleep(10);   
    gSystem->ProcessEvents();
  }
  //cout << "aaa1" << endl;
  trd_fana->Join();
  trd_fana->Delete();
  //cout << "aaa2" << endl;
  trd_ana->Join();
  trd_ana->Delete();
  //cout << "aaa3" << endl;

  gSystem->Sleep(opt.tsleep);   
  Show();

  cv->SetEditable(true);
}
/*
void CRS::DoNBuf() {

  b_fana=true;
  b_stop=false;
  for (int i=0;i<opt.num_buf;i++) {
    if (!(Do1Buf() && b_fana)) {
      break;
    }
  }

  b_stop=true;

}
*/
void CRS::Show() {

  //cout << "Show" << endl;
  static Long64_t bytes1=0;
  static Long64_t bytes2;
  static double t1;

  static Long64_t tm1=0;//gSystem->Now();
  static Long64_t tm2;
  //= gSystem->Now();
  MemInfo_t info;

  //while (!crs->b_stop) {
    tm2=gSystem->Now();
    if (tm2-tm1>opt.tsleep) {
      tm1=tm2;

      stat_mut.Lock();
      bytes2 = crs->totalbytes;
      stat_mut.UnLock();

      double dt = opt.T_acq - t1;
      if (dt>0.1)
	crs->mb_rate = (bytes2-bytes1)/MB/dt;
      else
	crs->mb_rate=0;

      t1=opt.T_acq;
      bytes1=bytes2;

      gSystem->GetMemInfo(&info);
      //cout << "show... " << info.fMemTotal << " " << info.fMemFree
      //   << " " << info.fMemUsed << endl;

      if (myM && myM->fTab->GetCurrent()==EvtFrm->ntab) {
	EvtFrm->fCanvas->GetCanvas()->SetEditable(true);
	EvtFrm->DrawEvent2();
	EvtFrm->fCanvas->GetCanvas()->SetEditable(false);
      }

      if (myM && myM->fTab->GetCurrent()==HiFrm->ntab) {
	//HiFrm->DrawHist();      
	HiFrm->ReDraw();
      }

      myM->UpdateStatus();

    }
    //}

    //cout << "Show end" << endl;
}

void CRS::Decode32(UChar_t *buffer, int length) {

  PulseClass *ipp;

  ULong64_t* buf8 = (ULong64_t*) buffer;

  pulse_vect pvect;
  Vpulses.push_back(pvect);
  list_pulse_reviter vv = Vpulses.rbegin(); //vv - current vector of pulses
  list_pulse_reviter vv2 = vv; //vv2 - vector of pulses from previous buffer
  ++vv2;

  //cout << "Decode32 1: " << Vpulses.size() << " " << &*vv2
  //     << " " << &*Vpulses.rend() << endl;

  if (vv2==Vpulses.rend()) { //this is start of the acqisition
    vv->push_back(PulseClass());
    npulses++;
    ipp = &vv->back();
    ipp->Chan = buffer[7];
    ipp->ptype|=P_NOSTART; // first pulse is by default incomplete
    //it will be reset if idx8==0 in the while loop
  }
  else {
    ipp=&vv2->back();
    // ipp points to the last pulse of the previous buffer
  }

  //cout << "Decode32 2: " << Vpulses.size() << endl;
  unsigned short frmt;
  int idx8=0;
  int idx1=0;
  ULong64_t data;

  while (idx1<length) {
    //frmt = (buffer[6] & 0xF0);
    frmt = buffer[idx1+6];
    //YKYKYK!!!! do something with cnt - ???
    //int cnt = frmt & 0x0F;
    frmt = (frmt & 0xF0)>>4;
    data = buf8[idx8] & 0xFFFFFFFFFFFF;
    unsigned char ch = buffer[idx1+7];
    
    //printf("d32: %d %d %d %d %d\n",idx1,frmt,cnt,ch,ipp->tdif);
    //cout << "vv: " << vv << endl;
    if ((ch>=chanPresent) || (frmt && ch!=ipp->Chan)) {
      cout << "32: Bad channel: " << (int) ch
	   << " " << (int) ipp->Chan
	   << " " << idx8 //<< " " << nvp
	   << endl;
      ipp->ptype|=P_BADCH;

      idx8++;
      idx1=idx8*8;
      continue;
    }

    if (frmt==0) {
      //make new pulse only if this is not the first record.
      //For the first record new pulse is already created at the beginning

      if (idx8) {

	//if (nbuffers < 3) {
	//cout << nbuf << " " << idx8 << " ";
	//ipp->PrintPulse();
	//}

	ipp->ptype&=~P_NOSTOP; //pulse has stop
	vv->push_back(PulseClass());
	npulses++;
	ipp = &vv->back();
      }
      else { //idx8==0 -> pulse has start
	ipp->ptype&=~P_NOSTART;
      }
      ipp->Chan=ch;
      ipp->Tstamp64=data;// - cpar.preWr[ch];

    }
    else if (frmt==1) {
      ipp->Control = buffer[idx1+5]+1;
      ipp->Counter = data & 0xFFFFFFFFFF;
    }
    else if (frmt==2) {

      if ((int)ipp->sData.size()>=cpar.durWr[ipp->Chan]) {
	// cout << "32: ERROR Nsamp: " << nbuf << " " << cnt
	//      << " " << (ipp->Counter & 0x0F)
	//      << " " << ipp->sData.size() << " " << cpar.durWr[ipp->Chan]
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx8 << " " << transfer->actual_length
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      /*
	else if (ipp->sData.size()%1000 == 0) {
	cout << "32: Nsamp: " << nbuf << " " << cnt
	<< " " << (ipp->Counter & 0x0F)
	<< " " << ipp->sData.size() << " " << cpar.durWr[ipp->Chan]
	<< " " << (int) ch << " " << (int) ipp->Chan
	<< " " << idx8 << " " << transfer->actual_length
	<< endl;
	ipp->ptype|=p_bad;
	}
      */
      //else {
      for (int i=0;i<4;i++) {
	//YKYKYK int zzz = data & 0xFFF;
	int zzz = data & 0xFFF;
	ipp->sData.push_back((zzz<<20)>>20);
	//ipp->sData[ipp->Nsamp++]=(zzz<<20)>>20;
	//printf("sData: %4d %12llx %5d\n",Nsamp-1,data,sData[Nsamp-1]);
	data>>=12;
      }
      //}
    }
    
    idx8++;
    idx1=idx8*8;
  }

  // for (UInt_t i=0;i<vv->size();i++) {
  //   cout << "Decode32: " << i << " " << vv->at(i).Tstamp64 << " "
  // 	 << (int) vv->at(i).ptype << endl;
  // }

  //cout << "Decode32 3: " << Vpulses.size() << endl;
  if (opt.analyze)
    Make_Events();

  if (Vpulses.size()>2)
    Vpulses.pop_front();

  //cout << "Decode32 4: " << Vpulses.size() << endl;
  /*
  static TTimeStamp t1;
  static TTimeStamp t2;
  t2.Set();
  double tt = t2.GetSec()-t1.GetSec()+
    (t2.GetNanoSec()-t1.GetNanoSec())*1e-9;

  if (myM && myM->fTab->GetCurrent()==HiFrm->ntab
      && tt*1000>opt.tsleep) {
    //HiFrm->DrawHist();      
    HiFrm->ReDraw();
    gSystem->ProcessEvents();
    t1=t2;
  }
  */


}

void CRS::Decode2(UChar_t* buffer, int length) {

  PulseClass *ipp;

  unsigned short* buf2 = (unsigned short*) buffer;
  //int nbuf = *(int*) transfer->user_data;

  pulse_vect pvect;
  Vpulses.push_back(pvect);
  list_pulse_reviter vv = Vpulses.rbegin(); //vv - current vector of pulses
  list_pulse_reviter vv2 = vv; //vv2 - vector of pulses from previous buffer
  ++vv2;

  if (vv2==Vpulses.rend()) { //this is start of the acqisition
    vv->push_back(PulseClass());
    npulses++;
    ipp = &vv->back();
    ipp->Chan = ((*buf2) & 0x8000)>>15;
    ipp->ptype|=P_NOSTART; // first pulse is by default incomplete
    //it will be reset if idx8==0 in the while loop
  }
  else {
    ipp=&vv2->back();
    // ipp points to the last pulse of the previous buffer
  }

  unsigned short frmt;
  int idx2=0;
  int len = length/2;

  //cout << "decode2: " << idx2 << endl;
  while (idx2<len) {

    unsigned short uword = buf2[idx2];
    frmt = (uword & 0x7000)>>12;
    short data = uword & 0xFFF;
    unsigned char ch = (uword & 0x8000)>>15;

    //cout << "decode2: " << idx2 << " " << frmt << " " << (int) ch << " " << data << " " << (int) ipp->Chan << endl;

    if ((ch>=chanPresent) || (frmt && ch!=ipp->Chan)) {
      cout << "2: Bad channel: " << (int) ch
	   << " " << (int) ipp->Chan << endl;
      ipp->ptype|=P_BADCH;

      idx2++;
      continue;
    }

    if (frmt==0) {
      //make new pulse only if this is not the first record.
      //For the first record new pulse is already created at the beginning
      if (idx2) {
	ipp->ptype&=~P_NOSTOP; //pulse has stop
	//ipp->PrintPulse(1);
	vv->push_back(PulseClass());
	npulses++;
	ipp = &vv->back();
      }
      else { //idx2==0 -> pulse has start
	ipp->ptype&=~P_NOSTART; // reset ptype, as this is a good pulse
      }
      ipp->Chan = ch;
      ipp->Tstamp64=data;// - cpar.preWr[ch];

    }
    else if (frmt<4) {
      Long64_t t64 = data;
      ipp->Tstamp64+= (t64 << (frmt*12));
    }
    else if (frmt==4) {
      ipp->Counter=data;
    }
    else if (frmt==5) {
      if ((int)ipp->sData.size()>=cpar.durWr[ipp->Chan]) {
	// cout << "2: Nsamp error: " << ipp->sData.size()
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx2
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      //else {
      //cout << "decode2a: " << idx2 << " " << ipp << " " << ipp->sData.size() << endl;
      ipp->sData.push_back((data<<21)>>21);
      //cout << "decode2b: " << idx2 << endl;
      //}
    }

    //cout << "decode2a: " << idx2 << endl;

    idx2++;
  }

  //return;
  //cond[nbuf]->Signal();

  //cout << "decode2a: " << idx2 << endl;

  //Fill_Tail(nvp);
  if (opt.analyze)
    Make_Events();

  if (Vpulses.size()>2)
    Vpulses.pop_front();

  //nvp++;
  //if (nvp>=ntrans) nvp=0;
  //nvp = (nvp+1)%ntrans;

}

//-------------------------------

int CRS::Searchsync() {
  //returns 0 if syncw is not found; otherwise len (length of the m-link frame)

  for (;idx<BufLength;idx++)
    if (rbuf4[idx] == 0x2a500100) {
      rLen = rbuf2[idx*2+3];
      idnext=idx+rLen;
      if (rLen>511)
	cout << "Bad BufLength: " << idx << " " << idnext << " "
	     << rLen << endl;
      else {
	return rLen;
      }
    }

  return 0;
}

//-------------------------------------

void CRS::Decode_adcm() {
  //it is assumed that at the beginning of this procedure idx points
  //to the valid data (correct sync word and type)

  //idx+0 - syncw+frame type
  //idx+1 - rLen + frame counter
  //idx+2 - rubbish
  //idx+3 - header
  //idx+4 - event counter
  //idx+5 .. idx+5+nsamp-1 - data
  //idx+rLen-3 - tst
  //idx+rLen-2 - cnst
  //idx+rLen-1 - crc32

  // nsamp+8=rLen

  UInt_t header; //data header
  //UShort_t nbits; //ADC bits
  //UShort_t nw; //samples per word = 2
  UShort_t lflag; //last fragment flag
  UShort_t nsamp; // number of samples in the fragment
  //UShort_t nch; // channel number
  UShort_t id; // block id (=0,1,2)

  PulseClass *ipp;

  //std::vector<PulseClass> *vv = Vpulses+nvp; //current vector of pulses

  pulse_vect pvect;
  Vpulses.push_back(pvect);
  list_pulse_reviter vv = Vpulses.rbegin(); //vv - current vector of pulses
  list_pulse_reviter vv2 = vv; //vv2 - vector of pulses from previous buffer
  ++vv2;

  //cout << "idx1: " << idx << " " << hex << rbuf4[idx] << dec << endl;
  
  if (vv2==Vpulses.rend()) { //this is start of the acqisition
    //-> search for the syncw
    idx=0;
    if (!Searchsync()){
      cout << "sync word not found1" << endl;
      return;
    }
  }
  /*
  else {
    cout << "YK (not done yet...)" << endl;
    // what's below should be rewritten...
    for (idx=0;idx<BufLength;idx++)
      if (rbuf4[idx] == 0x2a500100) {
	rLen = rbuf2[idx*2+3];
	idnext=idx+rLen;
	if (rLen>511)
	  cout << "Bad BufLength: " << idnext << " " << rLen << endl;
	else {
       	  break;
	}
      }
  }
  */
  //cout << "idx found: " << idx << " " << idnext << " " 
  //     << hex << rbuf4[idx] << " " << rbuf4[idnext] << dec << endl;

  // cout << "idx2: " << idx << " " << hex << rbuf4[idx-1] << " "
  //      << rbuf4[idx] << " " << rbuf4[idx+1]
  //      << dec << endl;

  //now idnext points to the next syncw (which may be larger than BufLength)
  while (idx+1 < BufLength) {
    //rLen (~rbuf[idx+1]) should be inside buf

    //at this point idx points to a valid syncw and idnext is also withing FBuf

    if (rbuf4[idx] != 0x2a500100) {
      cout << "bad syncw: " << idx << " " << rbuf4[idx] << endl;
      if (!Searchsync()){
	cout << "sync word not found (YK: do something here...)" << endl;
	break;
      }
    }
    else {
      rLen = rbuf2[idx*2+3];
      idnext=idx+rLen;
    }
    if (idnext>BufLength)
      break;

    header = rbuf4[idx+3];
    id=bits(header,26,31);
    if (id==1) {
      cout << "adcm: id==1. What a luck, counters!: " << id << " " << npulses << endl;
    }
    else {
      lflag=bits(header,6,6);
      nsamp=bits(header,7,17);
      if (nsamp+8!=rLen) {
	cout << "wrong BufLength: " << idx << " " << nsamp << " " << rLen << endl;
	//idx=idnext;
	goto next;
      }
      if (lastfl) {
	vv->push_back(PulseClass());
	npulses++;
      }
      ipp = &vv->back();
      ipp->Chan=bits(header,18,25);
      lastfl=lflag;

      //nbits=bits(header,0,3);
      //nw=bits(header,4,5);
      ipp->Tstamp64 = rbuf4[idx+rLen-2];
      ipp->Tstamp64 <<= 32;
      ipp->Tstamp64 += rbuf4[idx+rLen-3];
      //ipp->Tstamp64 = rbuf4[idx+rLen-3];

      if (lflag)
	ipp->ptype&=~P_NOSTOP; //pulse has stop

      //Long64_t crc32 = rbuf4[idx+rLen-1];

      /*
      cout << "Header: " << idx << " " << header << " "
	   << (int) ipp->Chan << " " << lflag << " "
	   << nsamp << " " << rLen << " " << hex << header
	   << dec << " " << ipp->Tstamp64
	   << dec << endl;

      cout << "Header2: " << idx << " " << hex << rbuf4[idx] << " "
	   << rbuf4[idx+3] << " " << rbuf4[idx+5] << " "
	   << "65:" << rbuf4[idx+65] << " " << rbuf4[idx+66] << " "
	   << rbuf4[idx+67] << " " << rbuf4[idx+68]
	   << dec << endl;
      */
      static Float_t baseline=0;
      if (ipp->sData.empty() && nsamp) {
	baseline = rbuf2[idx*2+11];
      }
      for (int i=0;i<nsamp*2;i+=2) {
	ipp->sData.push_back(rbuf2[idx*2+i+11]-baseline);
	ipp->sData.push_back(rbuf2[idx*2+i+10]-baseline);
	//cout << i << " " << rbuf2[idx*2+i+11] << endl;
	//cout << i << " " << rbuf2[idx*2+i+10] << endl;
      }

      // cout << "sData: " << ipp->Tstamp64 << endl;
      // for (UInt_t i=0;i<ipp->sData.size();i++) {
      // 	cout << i << " " << ipp->sData.at(i) << endl;
      // }
      
    }

    //cout << "idnext: " << idx << " " << idnext << endl;
    //AnaMLinkFrame();

  next:
    idx=idnext;


    /*
    if (rbuf4[idx] != 0x2a500100) {
      cout << "bad syncw: " << idx << " " << rbuf4[idx] << endl;
      if (!Searchsync()){
	cout << "sync word not found (YK: do something here...)" << endl;
	break;
      }
    }
    else {
      rLen = rbuf2[idx*2+3];
      idnext=idx+rLen;
    }
    */

  }

  if (opt.analyze)
    Make_Events();

  if (Vpulses.size()>2)
    Vpulses.pop_front();

  int sz=(BufLength-idx);
  if (sz>1024) {
    cout << "Bad adcm file. Frame size too large: " << sz << " " << idx << endl;
    exit(-1);
  }
  memcpy(rbuf4-sz,rbuf4+idx,sz*sizeof(UInt_t));
  //int idx2=idx;
  idx=-sz;

  // cout << "idnext: " << idx2 << " " << idx << " " << idnext << " "
  //      << BufLength << " " << hex << rbuf4[idx2] << " " << rbuf4[idx]
  //      << dec << endl;

} //Decode_adcm

//-------------------------------------

/*
void CRS::PrintPulse(int udata, bool pdata) {

  printf("Pulse: %d %d %lld %d %lld %d %lld\n",
	 udata,iBP,npulses,ipp->Chan,ipp->Counter,ipp->Nsamp,ipp->Tstamp64);
  //cout << endl;

  if (pdata) {
    for (int i=0;i<ipp->Nsamp;i++) {
      printf("%d %f\n",i,ipp->sData[i]);
    }
  }

}
*/

void CRS::Print_Pulses() {
  //std::vector<PulseClass> *vv = Vpulses+nvp;
  list_pulse_reviter vv = Vpulses.rbegin();

  cout << "Pulses: " << npulses;
  for (UInt_t i=0;i<vv->size();i++) {
    cout << " " << (int)vv->at(i).Chan << "," << vv->at(i).Tstamp64;
  }
  cout << endl;

}

void CRS::Print_Events() {
  int nn=0;
  cout << "Print_Events: " << Levents.begin()->T << endl;
  for (std::list<EventClass>::iterator it=Levents.begin();
       it!=Levents.end();++it) {
    cout << nn++ << " " << it->T << " :>>";
    for (UInt_t i=0;i<it->pulses.size();i++) {
      cout << " " << (int)it->pulses.at(i).Chan<< "," << it->pulses.at(i).Tstamp64;
    }
    cout << endl;
  }
}

void CRS::Event_Insert_Pulse(PulseClass *pls) {

  //const Long64_t ev1=36090;
  //const Long64_t ev2=37010;

  //const Long64_t ev1=1;
  //const Long64_t ev2=0;

  //if (nbuffers < 1) {
  //pls->PrintPulse();
  //}

  if (pls->ptype & 0x7) {
    cout << "bad pulse: " << (int) pls->Chan << " " << pls->Tstamp64 << " "
	 << (int) pls->ptype << endl;
    return;
  }
  
  if (opt.nsmoo[pls->Chan]) {
    pls->Smooth(opt.nsmoo[pls->Chan]);
  }
  pls->FindPeaks();
  pls->PeakAna();

  //static Long64_t last=0;
  
  //int nn=0;


  // cout << "m_event: " << Levents.size() << " " << &*m_event
  //      << " " << &*std::list<EventClass>::reverse_iterator(m_event)
  //      << " " << &*Levents.begin() << " " << &*Levents.end() 
  //      << " " << &*Levents.rbegin() << " " << &*Levents.rend() 
  //   //<< " " << &*(--Levents.end())
  //      << endl;

  // cout << "m_event3: " << Levents.size() << " " << m_event->T
  //      << " " << (&*std::list<EventClass>::reverse_iterator(m_event))->T
  //      << " " << Levents.begin()->T << " " << Levents.end()->T 
  //      << " " << Levents.rbegin()->T << " " << Levents.rend()->T
  //   //<< " " << &*(--Levents.end())
  //      << endl;

  // if (Levents.size()==6) {
  //   m_event=--Levents.end();
  //   Levents.erase(Levents.begin(),m_event);
  //   //Levents.insert(m_event,EventClass());
  // }
  

  // if (nevents>ev1 && nevents<ev2) {
  //   cout << "pls: " << nevents << " " << (int) pls->Chan << " " << pls->Counter
  // 	 << " " << pls->Tstamp64 << endl;
  // }

  event_iter it;
  event_reviter rl;
  //event_list_reviter Rit;

  Long64_t dt=pls->Tstamp64-T_last;

  //cout << "Insert: " << (int) pls->Chan << " " << pls->Tstamp64 << " "
  //<< T_last << " " << dt << " " << opt.tgate << endl;

  if (dt>opt.tgate) { //add event at the end of the list
    it=Levents.insert(Levents.end(),EventClass());
    it->Nevt=nevents;
    nevents++;
    it->Pulse_Ana_Add(pls);
    // if (nevents>ev1 && nevents<ev2) {
      // cout << "NewEv2: " << nevents << " " << (int) pls->Chan << " "
      // 	   << pls->Tstamp64 << " " << dt
      // 	   << " " << it->pulses.size() << " " << Levents.size() << endl;
    // }

    T_last=pls->Tstamp64;

    return;
  }

  //event_reviter r_event =
  //  event_reviter(m_event);

  int nn=0;
  //int n1=0;
  //int n2=0;

  for (rl=Levents.rbegin(); rl!=Levents.rend(); ++rl) {
    dt = (pls->Tstamp64 - rl->T);
    if (dt > opt.tgate) {
      //add new event at the current position of the eventlist
      it=Levents.insert(rl.base(),EventClass());
      it->Nevt=nevents;
      nevents++;
      it->Pulse_Ana_Add(pls);
      // if (nevents>ev1 && nevents<ev2) {
      //   cout << "NewEv: " << nevents << " " << (int) pls->Chan << " "
      //        << pls->Tstamp64 << " " << rl->T << " " << dt
      //        << " " << rl->pulses.size() << endl;
      // }
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      // coincidence event
      rl->Pulse_Ana_Add(pls);
      // if (nevents>ev1 && nevents<ev2) {
      //   cout << "OldEv: " << nevents << " " << (int) pls->Chan << " "
      //        << pls->Tstamp64 << " " << rl->T << " " << dt
      //        << " " << rl->pulses.size() << endl;
      // }
      return;
    }
    //nn++;
    //n2++;
    //if (nn>100) break;
  }

  if (debug)
    cout << "beginning: " << nevents << " " << pls->Tstamp64 << " " << dt
	 << " " << Levents.size() << " " << nn << endl;

  it=Levents.insert(Levents.end(),EventClass());
  it->Nevt=nevents;
  it->Pulse_Ana_Add(pls);
  nevents++;





  /*
  std::list<EventClass>::iterator rl;
  std::list<EventClass>::iterator r_event = m_event;

  cout << "m_event: " << Levents.size() << " " << &*m_event
       << " " << &*r_event << " " << &*(--r_event)
       << " " << &*Levents.begin() << " " << &*Levents.end() 
       << " " << &*(--Levents.end())
       << endl;

  //for (rl=Levents.rbegin(); rl!=Levents.rend(); ++rl) {
  for (rl=--Levents.end();rl!=--r_event;--rl) {

    dt = (pls->Tstamp64 - rl->T);
    if (dt > opt.tgate) {
      //add new event at the current position of the eventlist
      Levents.insert(++rl,EventClass());
      --rl;
      rl->Nevt=nevents;
      nevents++;
      rl->Pulse_Ana_Add(pls);
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      // coincidence event
      rl->Pulse_Ana_Add(pls);
      return;
    }
    else {
      cout << "loop: " << Levents.size() << " " << &*rl << endl;
    }
    // nn++;
    // if (nn>=opt.event_lag) {
    // }
  }
  */


  /*
  if (Levents.empty() || Levents.size()<100) {
    //at the very beginning set m_event to the beginning of Levents
    it=Levents.insert(Levents.begin(),EventClass());
    m_event=Levents.begin();
    //rl = event_reviter(m_event);
    //cout << "LLL: " << m_event->T << " " << rl->T << endl;
    return;
  }
  else {
    //this is rogue event; it's inserted before m_event and
    //(probably) not included in any coincidences
    it=Levents.insert(m_event,EventClass());
    //rl = event_reviter(m_event);
    //--rl;
    if (debug)
      cout << "beginning: " << dt << " " << Levents.size() << endl;
  }
  */

}

void CRS::Make_Events() {

  std::vector<PulseClass>::iterator pls;

  //event_list elist;
  //Levents.push_back(elist);
  //Print_Pulses(nvp);

  EventClass* firstevent;
  if (Levents.empty()) {
    firstevent=NULL;
  }
  else {
    firstevent=&Levents.back();
  }

  list_pulse_reviter vv = Vpulses.rbegin();
  list_pulse_reviter vv2 = vv;
  ++vv2;

  // if (!vv->empty())
  //   cout << "Make_events back: " << (int) vv->back().ptype << " " 
  // 	 << vv->back().Tstamp64 << endl;

  if (vv2!=Vpulses.rend()) {
    if (!(vv2->back().ptype&P_NOSTOP))
      Event_Insert_Pulse(&vv2->back());
    //Vpulses.pop_front();
    //Print_Events();
  }

  //cout << "Make_Events: " << &*Vpulses.rend() << " " << &*vv << " " << vv->size(); 

  //now insert all pulses from the current buffer, except last one
  //--vv;// = Vpulses+nvp;

  //cout << " " << &*vv << " " << vv->size() << endl;

  if (vv->size()<=1)
    return; //if vv contains 0 or 1 event, don't analyze it 

  //cout << "m_flag: " << m_flag << " " << (!m_flag && Levents.size()>list_min)
  //     << " " << Levents.size() << " " << list_min << endl;

  for (pls=vv->begin(); pls != --vv->end(); ++pls) {
    if (!(pls->ptype&P_NOSTOP)) {
      Event_Insert_Pulse(&(*pls));
      //Print_Events();
      /*YKYK
      if (!m_flag && Levents.size()>list_min) {
	m_event2=--Levents.end();
	m_flag=1;
	cout << "m_event2: " << m_event2->Nevt << endl;
      }
      */
    }
  }

  //YK don't understand why this is needed...
  //-----------
  // std::list<EventClass>::reverse_iterator evt;
  // UInt_t nn=0;
  // for (evt=crs->Levents.rbegin();evt!=crs->Levents.rend();evt++) {
  //   nn++;
  //   if (nn>2) break;
  // }

  //cout << "Make_Events: " << evt->T << " " << Levents.size() << endl;
  //PEvent();
  //-----------

  //Analyse events and clean (part of) the event list
  /*
  if (!m_flag && (int) Levents.size()>opt.ev_min) {
    m_event2=--Levents.end();
    m_flag=1;
    //cout << "ev_min: " << Levents.size() << " " << m_event2->T << endl;
  }
  */


  //cout << "Make_events: " << Levents.size() << endl;

  // if (Levents.size()>UInt_t(opt.ev_max)) {
  //   trd_ana->Run();
  //   //Ana_Events(0);
  // }

  // if (Levents.size()>10) {
  //   //Levents.pop_front();
  //   Levents.erase(Levents.begin());
  // }

  Select_Event(firstevent);


}

void CRS::Select_Event(EventClass *evt) {

  if (Levents.empty())
    return;

  if (b_acq) { //acquisition is running
    if (evt) {
      EvtFrm->Tevents.clear();
      EvtFrm->Tevents.push_back(*evt);
      EvtFrm->Pevents=&EvtFrm->Tevents;
    }
  }
  else { //acq is not running -> file analysis or stop
    EvtFrm->Pevents=&Levents;    
  }

  EvtFrm->d_event=EvtFrm->Pevents->begin();

}
