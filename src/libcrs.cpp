#include "libcrs.h"

#include <zlib.h>

#include <sys/stat.h>

#ifdef CYUSB
#include "cyusb.h"
#endif

//#include <pthread.h>
#include "romana.h"

#include <malloc.h>
#include <TClass.h>
#include <TCanvas.h>

//#include <TSemaphore.h>
//TSemaphore sem;
#include "TThread.h"
#include "TMutex.h"

//TMutex Emut3;
TMutex stat_mut;
//TMutex ana_mut;

const int BFMAX=999999999;

TCondition* cond[CRS::MAXTRANS];

using namespace std;

extern EventFrame* EvtFrm;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern HClass* hcl;
extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern ChanParDlg *chanpar;
extern int debug; // for printing debug messages

const double MB = 1024*1024;

//Int_t ev_max;

//bool bstart=true;
//bool btest=false;

//pthread_t tid1;
TThread* trd_crs;
//TThread* trd_stat;
//TThread* trd_evt;
//TThread* trd_dum;
//TThread* trd_ana;

const Long64_t P64_0=-123456789123456789;

int event_thread_run;//=1;

//UInt_t list_min=100;

volatile char astat[CRS::MAXTRANS];

CRS* crs;
extern Coptions cpar;
extern Toptions opt;
extern int chanPresent;

//TCondition tcond1(0);

void printhlist(int n);
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

//static int timeout = 0;

#ifdef CYUSB
cyusb_handle *cy_handle;

void *handle_events_func(void *ctx)
{
  while (event_thread_run) {
    libusb_handle_events_completed(NULL,NULL);
  }
  return NULL;
}

/*
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
	crs->f_raw = gzopen(opt.fname_raw,crs->raw_opt);
	if (crs->f_raw) {
	  int res=gzwrite(crs->f_raw,transfer->buffer,transfer->actual_length);
	  gzclose(crs->f_raw);
	  crs->writtenbytes+=res;
	}
	else {
	  cout << "Can't open file: " << opt.fname_raw << endl;
	}
      }

      crs->nbuffers++;

    } //if (transfer->actual_length) {

    libusb_submit_transfer(transfer);

    stat_mut.Lock();
    crs->totalbytes+=transfer->actual_length;
    //opt.T_acq = t2.GetSec()-opt.F_start.GetSec()+
    //(t2.GetNanoSec()-opt.F_start.GetNanoSec())*1e-9;
    //opt.T_acq = (Long64_t(gSystem->Now()) - crs->T_start)*0.001;
    //cout << "T_acq: " << opt.T_acq << " " << crs->T_start << endl;

    stat_mut.UnLock();
  } //if (crs->b_acq) {

  //crs->nvp = (crs->nvp+1)%crs->ntrans;
  
}

#endif //CYUSB

void *handle_buf(void *ctx)
{

  int* nmax = (int*) ctx; 
  //cout << "handle_buf: " << *nmax << endl; 

  int i=1;
  int res;
  while ((res=crs->DoBuf()) && crs->b_fana && i<*nmax) {
    i++;
  }

  if (!res) { //end of file reached -> analyze all events
    crs->b_run=2;
    //gzclose(crs->f_read);
    //crs->f_read=0;
  }
  else { //otherwise just stop the analysis
    gSystem->Sleep(30);    
    crs->b_run=0;
  }

  //cout << "end_buf: " << crs->nbuffers << " " << i << " " << res << " " << crs->b_run << endl;
  crs->b_stop=true;
  return NULL;
}

void *handle_ana(void* ptr) {
  // when the event list (Levents) becomes larger than ev_min,
  // starts analysing the events (fillhist);
  // when it becomes larger than ev_max, starts erasing the events

  std::list<EventClass>::iterator it;

  //check if it's the beginning of the analysis -> then define crs->m_start
  //if (crs->m_start==crs->Levents.end()) {
  //cout << "ana_start: " << crs->Levents.empty() << " " << crs->b_stop << endl;
  if (crs->Levents.empty()) {
    //need this loop to have at least one event in Levents
    while (crs->Levents.empty() && !crs->b_stop) {
      //cout << "a9: " << crs->Levents.empty() << " " << crs->b_stop << endl;
      gSystem->Sleep(10);
    }
    crs->m_start = crs->Levents.begin();
  }

  //cout << "handle_ana: " << std::distance(crs->m_start,crs->Levents.begin()) << " " << EvtFrm << endl;

  int n2; //number of events to erase
  //m_event - first event which is not analyzed

  int ii=0; //temporary variable

  //MemInfo_t info;
  //bool unchecked=true;

  while (crs->b_run) {

    //gSystem->Sleep(100);

    // //aviod exhausting all memory
    // gSystem->GetMemInfo(&info);
    // if (unchecked && info.fMemFree<300) {
    //   opt.ev_max = crs->Levents.size()*0.005; //was sz*0.005;
    //   opt.ev_max*=100;
    //   if (opt.ev_max < 500) opt.ev_max=500;
    //   cout << "ev_max updated: " << opt.ev_max << endl;
    //   unchecked=false;
    // }

    if (opt.ev_min>=opt.ev_max) {
      opt.ev_min=opt.ev_max/2;
    }

    if (crs->b_run==2) { //analyze all events, then stop
      crs->m_event=crs->Levents.end();
      crs->b_run=0;
    }
    else { //analyze normally
      ii=0;
      for (it=--crs->Levents.end(); it!=crs->m_start && ii<opt.ev_min; --it) {
	++ii;
      }
      crs->m_event=it;
    }

    //cout << "st: " << ii << " " << std::distance(crs->m_start,crs->m_event) << " " << crs->m_event->Nevt << " " << crs->m_start->Nevt << endl;
    
    if (crs->m_event!=crs->m_start) { //there are some events to analyze
      
      //goto skip;

      if (EvtFrm) {
	// fill Tevents for EvtFrm::DrawEvent2
	EvtFrm->Tevents.clear();
	if (crs->m_event!=crs->Levents.end()) {
	  EvtFrm->Tevents.push_back(*crs->m_event);
	}
	EvtFrm->d_event=EvtFrm->Pevents->begin();
      }


      //skip:
      // cout << "ana: " << std::distance(crs->m_start,crs->m_event)
      // 	   << " " << std::distance(crs->m_event,crs->Levents.end()) << endl;

      // analyze events up to m_event
      for (it=crs->m_start; it!=crs->m_event;) {
	//if (it->Nevt>100000) {
	// cout << "ana7: " << it->Nevt << " " << std::distance(it,crs->m_event)
	//      << " " << crs->b_stop << " " << it->pulses.size()
	//      << endl;
	  //exit(0);
	  //}
	if (//!crs->b_stop &&
	    it->pulses.size()>=opt.mult1 && it->pulses.size()<=opt.mult2) {

	  it->FillHist(true);
	  it->FillHist(false);
	  //it->FillHist_old();
	  if (opt.dec_write) {
	    crs->Fill_Dec(&(*it));
	  }


	  //it->Analyzed=true;
	  ++crs->nevents2;
	  ++it;
	  //++(crs->n_ana);
	  //YK cout << "ana71: " << it->Nevt << " " << std::distance(it,crs->m_event) << " " << crs->nevents2 << endl;
	}
	else {
	  it=crs->Levents.erase(it);
	  // if (it!=crs->m_event)
	  //   cout << "ana72: " << it->Nevt
	  // 	 << " " << std::distance(it,crs->m_event)
	  // 	 << " " << std::distance(it,crs->Levents.end())
	  // 	 << " " << &*it
	  // 	 << " " << &*crs->m_event
	  // 	 << " " << &*crs->Levents.end()
	  // 	 << endl;
	}
      }

      // cout << "ana2: " << std::distance(crs->m_start,crs->m_event) << endl;

      // m_start now points to the first event which is not analyzed yet
      crs->m_start=crs->m_event;

      //cout << "ana2a: " << std::distance(crs->m_start,crs->m_event) << endl;

      // erase events if the list is too long
      n2 = crs->Levents.size()-opt.ev_max;
      if (n2>0) {
	ii=0;
	for (it=crs->Levents.begin(); it!=crs->m_start && ii<n2;) {
	  it=crs->Levents.erase(it);
	  //YK cout << "ana73: " << it->Nevt << " " << std::distance(it,crs->m_event) << endl;
	  ++ii;
	  //++it;
	  //--(crs->n_ana);
	}
      }

      //cout << "ana3: " << std::distance(crs->m_start,crs->m_event) << endl;

    } // if (n1>0)
    else {
      gSystem->Sleep(10);
    }
  } //while (!crs->b_run)

  if (opt.dec_write) {
    crs->Flush_Dec();
  }

  //cout << "end_ana: " << endl;

  return 0;
    
}

CRS::CRS() {

  /*
  //cout << TClass::GetClass("Toptions")->GetClassVersion() << endl;
  //exit(1);

    // begin test block
    std::list<int> mylist;
    std::list<int>::iterator it;
    std::list<int>::reverse_iterator rit;

    it = mylist.begin();
    cout << *it << endl;
    // set some initial values:
    for (int i=1; i<=5; ++i) mylist.push_back(i); // 1 2 3 4 5

    it = --mylist.end();
    //mylist.insert(mylist.end(),77);
    mylist.insert(it,77);
    //cout << *it << endl;

    for (it=mylist.begin(); it!=mylist.end(); ++it)
      std::cout << ' ' << *it;
    std::cout << '\n';

    //cout << *it << " " << *mylist.begin() << endl;
    exit(1);
    
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
    // end test block
*/

  //ev_max=2*opt.ev_min;

  //mean_event.Make_Mean_Event();

  for (int i=0;i<MAX_CH+ADDCH;i++) {
    type_ch[i]=255;
  }

  MAXTRANS2=MAXTRANS;
  memset(Pre,0,sizeof(Pre));

  for (int i=0;i<MAXTRANS;i++)
    cond[i]=new TCondition(0);

  Fmode=0;
  period=5;

  f_raw=0;
  f_read=0;
  f_dec=0;
  //f_tree=0;
  //Tree=0;

  batch=false;
  b_acq=false;
  b_fana=false;
  b_stop=true;
  b_run=0;
  //justopened=true;

  strcpy(raw_opt,"ab");
  //strcpy(dec_opt,"ab");

  Fbuf=NULL;
  Fbuf2=0;

  DecBuf=new UChar_t[DECSIZE]; //1 MB

  // if (Fbuf2)
  //   cout << "Fbuf2: " << Fbuf2 << endl;
  // else
  //   cout << "Fbuf3: " << Fbuf2 << endl;
  
  strcpy(Fname," ");
  DoReset();

  module=0;

#ifdef CYUSB
  cy_handle=NULL;
#endif

  event_thread_run=1;

  // b_acq=false;
  // b_fana=false;
  // bstart=true;

  chanPresent=MAX_CH;

  ntrans=MAXTRANS;
  //opt.usb_size=1024*1024;

  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] =NULL;
    buftr[i]=NULL;
  }

  //cout << "creating threads... " << endl;

  //trd_stat = new TThread("trd_stat", handle_stat, (void*) 0);
  //trd_stat->Run();
  //trd_evt = new TThread("trd_evt", handle_evt, (void*) 0);
  //trd_evt->Run();
  //trd_ana = new TThread("trd_ana", Ana_Events, (void*) 0);
  //trd_ana->Run();

  //mTh= new TThread("memberfunction",
  //(void(*)(void *))&Thread0,(void*) this);
 
  //cout << "threads created... " << endl;

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
  delete[] DecBuf;

}

/*
void CRS::Dummy_trd() {
  trd_dum = new TThread("trd_dum", handle_dum, (void*) 0);
  trd_dum->Run();
}
*/

#ifdef CYUSB

int CRS::Detect_device() {

  int r;

  module=0;
  chanPresent=32;
  ver_po=0;

  b_usbbuf=false;
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

#ifdef CYUSB
  trd_crs = new TThread("trd_crs", handle_events_func, (void*) 0);
  trd_crs->Run();
#endif

  // trd_stat = new TThread("trd_stat", handle_stat, (void*) 0);
  // trd_stat->Run();

  // trd_evt = new TThread("trd_evt", handle_evt, (void*) 0);
  // trd_evt->Run();

  cout << "CRS thread created... " << endl;

  //memset(buf_out,'\0',64);
  //memset(buf_in,'\0',64);

  //buf_out[0] = 1; //get card info
  Command2(4,0,0,0);

  int sz;
  sz = Command32(1,0,0,0);

  cout << "Info: " << sz << endl;
  cout << "Device code: " << int(buf_in[0]) << endl;
  cout << "Serial Nr: " << int(buf_in[1]) << endl;
  cout << "Number of working plates: " << int(buf_in[2]) << endl;
  cout << "Firmware version: " << int(buf_in[3]) << endl;

  //for (int i=0;i<sz;i++) {
  //cout << int(buf_in[i]) << " ";
  //}
  //cout << endl;
  ver_po=buf_in[4];

  if (buf_in[1]==3) { //serial Nr=3 -> crs2
    module=2;
    chanPresent=2;
    for (int j=0;j<chanPresent;j++) {
      type_ch[j]=0;
    }
  }
  else { //crs32
    module=32;
    int nplates= buf_in[3];
    chanPresent=nplates*4;
    if (ver_po==2) {//версия ПО=2
      //chanPresent=4;
      sz = Command32(10,0,0,0);
      sz--;
      for (int i=0;i<nplates;i++) {
	cout << "Channels(" << i << "):";
	for (int j=0;j<4;j++) {
	  type_ch[i*4+j]=buf_in[sz];
	  cout << " " << type_ch[i*4+j];
	}
	cout << endl;
	sz--;
	//cout << i << " " << sz << endl;
      }
      //cout << "nplates: " << nplates << " " << sz << endl;
      //exit(0);
      //for (int i=0;i<MAX_CH;i++) {
      //cout << " " << chtype[i];
      //}
      //for (int i=sz-1;i>=0;i--) {
      //cout << " " << int(buf_in[i]);
      //}
      //cout << endl;
    }
  }

  cout << "module: " << module << " chanPresent: " << chanPresent << endl;

  //cpar.InitPar(module);

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

int CRS::SetPar() {

  if (module==2) {
    AllParameters2();
  }
  else if (module==32) {
    // if (ver_po==1)
    //   AllParameters32_old();
    // else
      AllParameters32();
  }
  else {
    cout << "SetPar Error! No module found" << endl;
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
    //cout << "free: " << i << " " << (int) transfer[i]->flags << endl;
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

int CRS::Command32_old(byte cmd, byte ch, byte type, int par) {
  //для версии ПО 1
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
    return 0;
  }
  
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  if (cmd!=7) {
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      //cyusb_close();
    }
  }

  return len_in;
}

int CRS::Command32(byte cmd, byte ch, byte type, int par) {
  //для версии ПО 2
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
  case 8:
  case 9:
    len_out=1;
    len_in=2;
    break;
  case 10:
    len_out=1;
    len_in=9;
    break;
  default:
    cout << "Wrong CRS32 command: " << cmd << endl;
    return 0;
  }
  
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  if (cmd!=7) {
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      //cyusb_close();
    }
  }

  return len_in;
}

int CRS::Command2(byte cmd, byte ch, byte type, int par) {
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
    return 0;
  }

  len_in=2;

  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
  if (r) {
    printf("Error7! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  return len_in;
}

void CRS::Command_crs(byte cmd, byte chan) {

  if (module==32) {
    switch (cmd) {
    case 1: //enabl
      Command32(2,chan,11,(int)cpar.enabl[chan]);
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
	cpar.GetPar("thresh",module,chan,type_ch[chan],tmp,tmp,max);
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
  cout << "AllParameters32(): " << endl;

  for (byte chan = 0; chan < chanPresent; chan++) {
    Command32(2,chan,0,(int)cpar.acdc[chan]);
    Command32(2,chan,1,(int)cpar.inv[chan]);
    Command32(2,chan,2,(int)cpar.smooth[chan]);
    Command32(2,chan,3,(int)cpar.deadTime[chan]);
    Command32(2,chan,4,(int)cpar.preWr[chan]);
    Command32(2,chan,5,(int)cpar.durWr[chan]);
    Command32(2,chan,6,(int)cpar.kderiv[chan]);
    Command32(2,chan,7,(int)cpar.threshold[chan]);
    Command32(2,chan,8,(int)cpar.adcGain[chan]);
    // new commands
    Command32(2,chan,9,0); //delay
    //Command32(2,chan,10,0); //test signal
    Command32(2,chan,11,(int) cpar.enabl[chan]); //enabled
    //Command32(2,chan,11,1); //enabled
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
      cpar.GetPar("thresh",module,chan,type_ch[chan],tmp,tmp,max);
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
    if (b_usbbuf) {
      crs->Free_Transfer();
      gSystem->Sleep(50);
      crs->Init_Transfer();
    }
    b_usbbuf=false;

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
    b_fstart=false;
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
    //T_start = opt.F_start;
    if (opt.raw_write) {
      sprintf(raw_opt,"wb%d",opt.raw_compr);

      f_raw = gzopen(opt.fname_raw,raw_opt);
      if (f_raw) {
	cout << "Writing parameters... : " << opt.fname_raw << endl;
	SaveParGz(f_raw);
	gzclose(f_raw);
	}
      else {
	cout << "Can't open file: " << opt.fname_raw << endl;
      }

      sprintf(raw_opt,"ab%d",opt.raw_compr);
    }   

    if (opt.dec_write) {
      Reset_Dec();
      // sprintf(dec_opt,"wb%d",opt.dec_compr);

      // f_dec = gzopen(opt.fname_dec,dec_opt);
      // if (f_dec) {
      // 	cout << "Writing parameters... : " << opt.fname_dec << endl;
      // 	SaveParGz(f_dec);
      // 	gzclose(f_dec);
      // 	}
      // else {
      // 	cout << "Can't open file: " << opt.fname_dec << endl;
      // }

      // sprintf(dec_opt,"ab%d",opt.dec_compr);
    }   

    //nvp=0;
    //Levents.clear();

    cout << "Acquisition started" << endl;
    //gettimeofday(&t_start,NULL);

    TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
    cv->SetEditable(false);

    b_run=1;
    TThread* trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);;
    trd_ana->Run();

    Command2(3,0,0,0);

    while (!crs->b_stop) {
      Show();
      gSystem->Sleep(10);   
      gSystem->ProcessEvents();
      if (opt.Tstop && opt.T_acq>opt.Tstop) {
	//cout << "Stop1!!!" << endl;
	DoStartStop();
	//cout << "Stop2!!!" << endl;
      }
    }

    trd_ana->Join();
    trd_ana->Delete();

    //cout << "startstop1: " << endl;
    EvtFrm->Clear();
    EvtFrm->Pevents = &Levents;
    EvtFrm->d_event=--EvtFrm->Pevents->end();
    //cout << "startstop2: " << endl;

    //gSystem->Sleep(opt.tsleep);   
    gSystem->Sleep(10);   
    Show(true);
    cv->SetEditable(true);
  } //start
  else { //stop
    buf_out[0]=4;
    b_acq=false;
    cout << "Acquisition stopped" << endl;

    Command2(4,0,0,0);

    gSystem->Sleep(300);

    //cout << "Acquisition stopped2" << endl;
    Cancel_all(ntrans);
    b_stop=true;
    b_run=2;
    //cout << "Acquisition stopped3" << endl;
    //Select_Event();
    //EvtFrm->Levents = &Levents;
  }

  return 0;

}

#endif //CYUSB

void CRS::DoExit()
{
  cout << "CRS::DoExit" << endl;

  event_thread_run=0;
#ifdef CYUSB
  cyusb_close();
  if (trd_crs) {
    trd_crs->Delete();
  }
#endif
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

void CRS::DoReset() {

  //cout << "DoReset1: " << b_stop << endl;

  if (!b_stop) return;

  b_fstart=false;
  opt.T_acq=0;

  // if (Fmode==1) {
  //   Tstart64=-1;
  // }
  // else {
  //   Tstart64=0;
  // }
  Tstart64=0;

  //cout << "Doreset: " << Fmode << " " << Tstart64 << endl;

  Tstart0=0;
  //Offset64=0;
  //T_last_good=0;
  Pstamp64=P64_0;

  Levents.clear();
  //cout << "EvtFrm: " << EvtFrm << endl;
  if (EvtFrm) {
    EvtFrm->DoReset();
  }

  m_start=Levents.end();
  m_event=Levents.end();

  nevents=0;
  nevents2=0;

  Vpulses.clear();

  if (Fmode!=1) {
    memcpy(&Pre,&cpar.preWr,sizeof(Pre));
  }

  npulses=0;
  nbuffers=0;
  memset(npulses2,0,sizeof(npulses2));

  //npulses=0;
  npulses_buf=0;

  totalbytes=0;
  writtenbytes=0;

  //MAX_LAG=opt.event_buf/2;

  idx=0;
  idnext=0;
  lastfl=1;

  idec=0;

  printhlist(5);
  //cout << "f_read: " << f_read << endl;
  if (f_read)
    DoFopen(NULL,0);
  justopened=true;

  printhlist(6);
  // parpar->Update();
  if (crspar) {
    crspar->ResetStatus();
  }
  //if (HiFrm)
  //  cout << "DoReset2: " << HiFrm->h_time[1]->GetName() << endl;

  rPeaks.clear();
  //CloseTree();

}

void CRS::DoFopen(char* oname, int popt) {
  //popt: 1 - read opt from file; 0 - don't read opt from file
  int tp=0; //1 - adcm raw; 0 - crs2/32
  int bsize;
  int boffset;

  if (oname)
    strcpy(Fname,oname);

  // cout << "DoFopen: " << Fname << endl;
  // if (TString(Fname).EqualTo(" ",TString::kIgnoreCase)) {
  //   return;
  // }

  string dir, name, ext;
  SplitFilename(string(Fname),dir,name,ext);

  if (TString(ext).EqualTo(".dat",TString::kIgnoreCase)) {
    tp=1;
  }
  else if (TString(ext).EqualTo(".gz",TString::kIgnoreCase)) {
    tp=0;
  }
  else if (TString(ext).EqualTo(".raw",TString::kIgnoreCase)) {
    tp=0;
  }
  //else if (TString(ext).EqualTo(".dec",TString::kIgnoreCase)) {
  //tp=0;
  //}
  else {
    cout << "Unknown file type (extension): " << Fname << endl;
    if (f_read) gzclose(f_read);
    f_read=0;
    return;
  }

  if (f_read) gzclose(f_read);
  f_read = gzopen(Fname,"rb");
  cout << "f_read: " << f_read << endl;
  if (!f_read) {
    Fmode=0;
    cout << "Can't open file: " << Fname << endl;
    f_read=0;
    return;
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
  printhlist(7);
    if (ReadParGz(f_read,Fname,1,1,popt)) {
      gzclose(f_read);
      f_read=0;
      return;
    }
    bsize=opt.rbuf_size*1024;
    boffset=0;
    period=5;
  printhlist(8);
  }

  // if (Fmode==1) {
  //   Tstart64=-1;
  // }
  // else {
  //   Tstart64=0;
  // }
  Tstart64=0;

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

  if (myM) {
    myM->SetTitle(Fname);
    crspar->AllEnabled(false);
    //myM->fTab->SetEnabled(1,false);
    //cout << "fTab: " << myM->fTab << endl;
  }
}

int CRS::ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2) {
  //m1 - read Fmode (1/0); 1 - read from raw/dec file; 0 - read from par file
  //p1 - read cpar (1/0); p2 - read opt (1/0)
  UShort_t sz;

  UShort_t mod;
  gzread(ff,&mod,sizeof(mod));

  cout << "ReadParGz1 Fmode: " << Fmode << endl;

  gzread(ff,&sz,sizeof(sz));

  char buf[100000];
  gzread(ff,buf,sz);

  if (p1) 
    BufToClass("Coptions",(char*) &cpar, buf, sz);
  if (p2)
    BufToClass("Toptions",(char*) &opt, buf, sz);

  if (m1) {
    //T_start = opt.F_start;
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

  //cout << "ReadParGz: " << sz << " " << pname << endl;

  if (Fmode!=1) {
    memcpy(&Pre,&cpar.preWr,sizeof(Pre));
  }

  for (int i=0;i<opt.ncuts;i++) {
    char ss[64];
    sprintf(ss,"%d",i+1);
    hcl->cutG[i] = new TCutG(ss,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    hcl->cutG[i]->SetLineColor(i+2);
  }

  // printhlist(9);
  // //HiFrm->Clear_Ltree();
  // hcl->Make_cuts();
  // //HiFrm->Make_Ltree();
  // printhlist(10);

  cout << "ReadParGz2: " << sz << " " << pname << endl;
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

int CRS::DoBuf() {

  //cout << "gzread0: " << Fmode << " " << nbuffers << " " << BufLength << " " << opt.rbuf_size*1024 << endl;
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

    nbuffers++;

    if (batch) {
      cout << "Buffers: " << nbuffers << "     Decompressed MBytes: "
	   << totalbytes/MB << endl;
    }

    //if (!b_stop) {
    //opt.T_acq = (Levents.back().T - Tstart64)*1e-9*period;
    //}

    return nbuffers;
  }
  else {
    //b_fana=true;
    //myM->DoAna();
    return 0;
  }


}

void CRS::FAnalyze(bool nobatch) {

  if (gzeof(f_read)) {
    cout << "Enf of file: " << endl;
    return;
  }
  TCanvas *cv=0;
  //cout << "FAnalyze: " << gztell(f_read) << endl;
  if (justopened && opt.dec_write) {
    Reset_Dec();
  }
  justopened=false;

  //cout << "batch01: " << endl;

  static int nmax=BFMAX-1;

  if (nobatch) {
    EvtFrm->Clear();
    EvtFrm->Pevents = &EvtFrm->Tevents;

    cv=EvtFrm->fCanvas->GetCanvas();
    cv->SetEditable(false);
  }
  //T_start = gSystem->Now();

  //cout << "batch02: " << endl;
  TThread* trd_fana = new TThread("trd_fana", handle_buf, (void*) &nmax);;
  trd_fana->Run();
  b_run=1;
  //cout << "batch03: " << endl;
  TThread* trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);;
  trd_ana->Run();
  if (nobatch) {
    while (!crs->b_stop) {
      Show();
      gSystem->Sleep(10);   
      gSystem->ProcessEvents();
    }
  }
  //cout << "batch04: " << endl;
  trd_fana->Join();
  trd_fana->Delete();

  //cout << "batch05: " << endl;
  trd_ana->Join();
  trd_ana->Delete();

  //cout << "batch06: " << endl;
  //gSystem->Sleep(opt.tsleep);
  if (nobatch) {
    EvtFrm->Clear();
    EvtFrm->Pevents = &Levents;
    EvtFrm->d_event=--EvtFrm->Pevents->end();

    gSystem->Sleep(10);
    Show(true);
    //cout << "asdfasdf" << endl;
    cv->SetEditable(true);
  }
}

void CRS::DoNBuf(int nb) {

  if (gzeof(f_read)) {
    cout << "Enf of file: " << endl;
    return;
  }

  if (justopened && opt.dec_write) {
    Reset_Dec();
  }
  justopened=false;

  //cout << "FAnalyze: " << f_read << endl;
    
  //static int nmax=opt.num_buf;

  EvtFrm->Clear();
  EvtFrm->Pevents = &EvtFrm->Tevents;

  TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
  cv->SetEditable(false);

  TThread* trd_fana = new TThread("trd_fana", handle_buf, (void*)&nb);
  trd_fana->Run();
  b_run=1;
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
  //cout << "batch06a: " << endl;

  EvtFrm->Clear();
  EvtFrm->Pevents = &Levents;
  EvtFrm->d_event=--EvtFrm->Pevents->end();

  //gSystem->Sleep(opt.tsleep);   
  gSystem->Sleep(10);   
  Show(true);

  cv->SetEditable(true);
}
/*
void CRS::DoNBuf() {

  b_fana=true;
  b_stop=false;
  for (int i=0;i<opt.num_buf;i++) {
    if (!(DoBuf() && b_fana)) {
      break;
    }
  }

  b_stop=true;

}
*/
void CRS::Show(bool force) {

  static Long64_t tm1=0;//gSystem->Now();
  static Long64_t tm2;
  //= gSystem->Now();
  //MemInfo_t info;

  tm2=gSystem->Now();
  if (tm2-tm1>opt.tsleep || force) {
    tm1=tm2;

    //cout << "Show: " << endl;

    //cout << "show... " << info.fMemTotal << " " << info.fMemFree << " " << info.fMemUsed << " " << Levents.size() << " " << &*m_event << " " << m_event->Nevt << endl;

    if (myM) {
      if (myM->fTab->GetCurrent()==EvtFrm->ntab) {
	EvtFrm->fCanvas->GetCanvas()->SetEditable(true);
	EvtFrm->DrawEvent2();
	EvtFrm->fCanvas->GetCanvas()->SetEditable(false);
      }
      else if (myM->fTab->GetCurrent()==HiFrm->ntab) {
	//HiFrm->DrawHist();      
	HiFrm->ReDraw();
	//HiFrm->Update();
      }
      else {
	TString name = TString(myM->fTab->GetCurrentTab()->GetString());
	// if (name.EqualTo("Parameters",TString::kIgnoreCase)) {
	//   //cout << "DoTab1a: " << name << endl;
	//   parpar->Update();
	// }
	// else
	  if (name.EqualTo("DAQ",TString::kIgnoreCase)) {
	  crspar->UpdateStatus();
	}
      }
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


  // if (vv2!=Vpulses.rend()) { //this is not start of the acqisition
  //   cout << "aa: " << vv->size() << " " << vv2->size() << endl;
  //   for (UInt_t i=0;i<vv2->size();i++) {
  //     PulseClass *pls = &vv2->at(i);
  //     cout << "XXXX: " << i << " " << pls << " " << pls->Tstamp64 << " "
  // 	   << " " << pls->Counter << " " << (int) pls->ptype
  // 	   << " " << pls->sData.size() << " " << (int) pls->Chan
  // 	   << endl;
  //   }
  // }
  
  // cout << "Decode32 1: " << Vpulses.size() << " " << &*vv2
  //      << " " << &*Vpulses.rend() << endl;

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
    frmt = buffer[idx1+6];
    //YKYKYK!!!! do something with cnt - ???
    //int cnt = frmt & 0x0F;
    frmt = (frmt & 0xF0)>>4;
    data = buf8[idx8] & 0xFFFFFFFFFFFF;
    unsigned char ch = buffer[idx1+7];
    
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
      //if (idx8) {
      if (b_fstart) {
	ipp->ptype&=~P_NOSTOP; //pulse has stop
	vv->push_back(PulseClass());
	npulses++;
	ipp = &vv->back();
      }
      else { //idx8==0 -> pulse has start
	b_fstart=true;
	ipp->ptype&=~P_NOSTART;
      }
      ipp->Chan=ch;
      ipp->Tstamp64=data;// - cpar.preWr[ch];

    }
    else if (frmt==1) {
      ipp->State = buffer[idx1+5];
      ipp->Counter = data & 0xFFFFFFFFFF;
      // if (buffer[idx1+5]) {
      // 	cout << "state: " << (int) buffer[idx1+5] << " " << (int ) ipp->State << " " << data << " " << buf8[idx8] << endl;
      // }
    }
    else if (frmt==2) {

      if (ipp->sData.size()>=cpar.durWr[ipp->Chan]) {
	// cout << "32: ERROR Nsamp: "
	//      << " " << (ipp->Counter & 0x0F)
	//      << " " << ipp->sData.size() << " " << cpar.durWr[ipp->Chan]
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx8 //<< " " << transfer->actual_length
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<4;i++) {

	int zzz = data & 0xFFF;
	ipp->sData.push_back((zzz<<20)>>20);
	//ipp->sData[ipp->Nsamp++]=(zzz<<20)>>20;
	//printf("sData: %4d %12llx %5d\n",Nsamp-1,data,sData[Nsamp-1]);
	data>>=12;
      }
      //}
    }
    else if (frmt==3) {

      if (ipp->sData.size()>=cpar.durWr[ipp->Chan]) {
	// cout << "32: ERROR Nsamp: "
	//      << " " << (ipp->Counter & 0x0F)
	//      << " " << ipp->sData.size() << " " << cpar.durWr[ipp->Chan]
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx8 //<< " " << transfer->actual_length
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<3;i++) {

	int zzz = data & 0xFFFF;
	ipp->sData.push_back((zzz<<16)>>16);
	//ipp->sData[ipp->Nsamp++]=(zzz<<20)>>20;
	//printf("sData: %4d %12llx %5d\n",Nsamp-1,data,sData[Nsamp-1]);
	data>>=16;
      }
      //}
    }
    else {
      cout << "bad frmt: " << frmt << endl;
    }

    idx8++;
    idx1=idx8*8;
  }

  /*
  //if (crs->nbuffers>=52 && crs->nbuffers<=54) {
  cout << "buf: " << crs->nbuffers << " " << Vpulses.size() << " " << vv->size() << " " << endl;

  if (vv2!=Vpulses.rend()) { //this is not start of the acqisition
    //PulseClass *pls2 = &vv2->at(0);
    //cout << "bbb: " << &vv2->at(0) << " " << pls2 << " " << vv2->size() << endl;

    for (UInt_t i=0;i<vv2->size();i++) {
      PulseClass *pls = &vv2->at(i);
      cout << "prev: " << i << " " << pls << " " << pls->Tstamp64 << " "
      	   << " " << pls->Counter << " " << (int) pls->ptype
      	   << " " << pls->sData.size() << " " << (int) pls->Chan
      	   << endl;
    }
  }
  
  for (UInt_t i=0;i<vv->size();i++) {
      PulseClass *pls = &vv->at(i);
      cout << "VECT: " << i << " " << pls->Tstamp64 << " "
	   << " " << pls->Counter << " " << (int) pls->ptype
	   << " " << pls->sData.size() << " " << (int) pls->Chan
	   << endl;
    }
    //}
    */

  //cout << "Decode32 3: " << Vpulses.size() << endl;

  Make_Events();

  if (Vpulses.size()>2)
    Vpulses.pop_front();

} //decode32

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
      //if (idx2) {
      if (b_fstart) {
	ipp->ptype&=~P_NOSTOP; //pulse has stop
	//ipp->PrintPulse(1);
	vv->push_back(PulseClass());
	npulses++;
	ipp = &vv->back();
      }
      else { //idx2==0 -> pulse has start
	b_fstart=true;
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
      if (ipp->sData.size()>=cpar.durWr[ipp->Chan]) {
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

  Make_Events();

  if (Vpulses.size()>2)
    Vpulses.pop_front();

  //nvp++;
  //if (nvp>=ntrans) nvp=0;
  //nvp = (nvp+1)%ntrans;

} //decode2

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

      /*
      Long64_t t_orig = ipp->Tstamp64;
      static int ii;
      ii++;
      bool tst=false;
      if (ii>257030 && ii<257060) tst=true;
      // // test1 -> tstamp is bad for all events starting from 3
      // if (ii>=3) {
      // 	ipp->Tstamp64-=109304694026798308;
      // }
      // // test2 -> tstamp is bad only for event 3
      // if (ii==3) {
      // 	ipp->Tstamp64-=149304694026798308;
      // }
      */


      if (Pstamp64==P64_0) {
	Pstamp64=ipp->Tstamp64;
	Offset64=0;
	//cout << "Zero Offset64: " << Offset64 << endl;
      }

      if (Offset64)
	ipp->Tstamp64-=Offset64;

      Long64_t dt=ipp->Tstamp64-Pstamp64;
      //10 or 20 sec = 2e9
      if (abs(dt) > 2000000000) { //bad event - ignore it

	Offset64+=dt;
	if (abs(Offset64) < 20000000) //~100-200 msec
	  Offset64=0;

	//cout << "Offset64: " << Offset64 << endl;
	ipp->ptype|=P_BADTST;

	cout << "bad Tstamp: "<<dt<<" "<<ipp->Tstamp64<<" "<<Pstamp64<<" "<<Offset64<<endl;
      }
      else {
	Pstamp64=ipp->Tstamp64;
      }

      // if (tst)
      // if (Offset64) {
      // 	cout << "Tst: " << ii << " " << dt << " " << ipp->Tstamp64 << " " << Pstamp64 << " " << Offset64 << endl;
      // }

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

  //cout << "Offset64: " << nbuffers << " " << Offset64 << endl;
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
      cout << " " << (int)it->pulses.at(i).Chan<< "," << it->pulses.at(i).Tstamp64-Tstart64;
    }
    cout << endl;
  }
}

void CRS::Event_Insert_Pulse(PulseClass *pls) {

  event_iter it;
  event_iter it_last;
  Long64_t dt;

  //if (pls->ptype & 0xF) { //P_NOSTART | P_NOSTOP | P_BADCH | P_BADTST
  //if (pls->ptype & 0x7) {
  if (pls->ptype) { //any bad pulse
    cout << "bad pulse: " << (int) pls->Chan << " " << pls->Counter << " "
	 << pls->Tstamp64 << " " << (int) pls->ptype << endl;
    return;
  }
  
  //const Long64_t ev1=36090;
  //const Long64_t ev2=37010;

  //const Long64_t ev1=1;
  //const Long64_t ev2=0;

  //if (nbuffers < 1) {
  //pls->PrintPulse(0);
  //}

  if (pls->ptype & 0x7) {
    cout << "bad pulse2: " << (int) pls->Chan << " " << pls->Tstamp64 << " "
	 << (int) pls->ptype << endl;
    return;
  }
  
  npulses2[pls->Chan]++;

  if (opt.nsmoo[pls->Chan]) {
    pls->Smooth(opt.nsmoo[pls->Chan]);
  }
  pls->FindPeaks();
  pls->PeakAna();

  if (Levents.empty()) {
    //if (Tstart64<0) {
    Tstart64 = pls->Tstamp64;

    //cout << "TStart64: " << Tstart64 << endl;
    it=Levents.insert(Levents.end(),EventClass());
    it->Nevt=nevents;
    nevents++;
    it->Pulse_Ana_Add(pls);

    //Pstamp64=pls->Tstamp64;

    return;
  }

  //it_last=--Levents.end();
  //dt=pls->Tstamp64-it_last->T;







  // //10 or 20 sec
  // Long64_t dt1=-99;
  // Long64_t T1=-99;
  // if (it!=Levents.end()) {
  //   dt1=pls->Tstamp64-it->T;
  //   T1=it->T;
  // }
  // cout << "tt: " << dt << " " << dt1 << " " << pls->Tstamp64-Tstart64 << " "
  //      << T1-Tstart64 << endl;

  // if (dt>opt.tgate) { //add event at the end of the list
  //   it=Levents.insert(Levents.end(),EventClass());

  //   it->Nevt=nevents;
  //   nevents++;
  //   it->Pulse_Ana_Add(pls);

  //   Pstamp64=pls->Tstamp64;

  //   return;
  // }

  for (it=--Levents.end();it!=m_event;--it) {
    //for (rl=Levents.rbegin(); rl!=r_event; ++rl) {
    dt = (pls->Tstamp64 - it->T);
    //cout << "tt: " << it->Nevt << " " << dt << " " << pls->Tstamp64 << " " << it->T << endl;
    if (dt > opt.tgate) {
      //cout << "t1: " << endl;
      //add new event at the current position of the eventlist
      it=Levents.insert(++it,EventClass());
      it->Nevt=nevents;
      nevents++;
      it->Pulse_Ana_Add(pls);
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      //cout << "t2: " << endl;
      // coincidence event
      it->Pulse_Ana_Add(pls);
      return;
    }
  }

  if (debug)
    cout << "beginning: " << nevents << " " << pls->Tstamp64 << " " << dt
	 << " " << Levents.size() << endl;

  // if the current event is too early, insert it at the end of the event list
  it=Levents.insert(Levents.end(),EventClass());
  it->Nevt=nevents;
  it->Pulse_Ana_Add(pls);
  nevents++;

}

/*
void CRS::Event_Insert_Pulse(PulseClass *pls) {

  //const Long64_t ev1=36090;
  //const Long64_t ev2=37010;

  //const Long64_t ev1=1;
  //const Long64_t ev2=0;

  //if (nbuffers < 1) {
  //pls->PrintPulse(1);
  //}

  event_iter it;
  Long64_t dt;

  if (pls->ptype & 0xF) { //P_NOSTART | P_NOSTOP | P_BADCH | P_BADTST
    cout << "bad pulse: " << (int) pls->Chan << " " << pls->Tstamp64 << " "
	 << (int) pls->ptype << endl;
    return;
  }
  
  npulses2[pls->Chan]++;

  if (opt.nsmoo[pls->Chan]) {
    pls->Smooth(opt.nsmoo[pls->Chan]);
  }
  pls->FindPeaks();
  pls->PeakAna();

  if (Tstart64<0) {
    Tstart64 = pls->Tstamp64;
  }

  // if (nevents==10) {
  //   //pls->Tstamp64=111;
  //   pls->Tstamp64-=1e11;
  //   //cout << "event10: " << nevents << " " << pls->Tstamp64 << endl;
  // }

  //dt=pls->Tstamp64-Pstamp64;

  // cout << "dt1: " << (int) pls->Chan << " " << dt << " " << nevents << " "
  //      << pls->Tstamp64 << " " << T_last_good << " " << Pstamp64 << endl;

  //Pstamp64=pls->Tstamp64;

  if (!nevents) { //first event
    //add new event at the end of the list, set T_last_good and return
    it=Levents.insert(Levents.end(),EventClass());
    it->Nevt=nevents;
    nevents++;
    it->Pulse_Ana_Add(pls);

    //T_last_good=pls->Tstamp64;
    return;
  }


  // //10 or 20 sec
  // if (abs(dt) > 2000000000) { //bad event
  //   //now: ignore bad event
  //   // was://add new event at the end of the list and return

  //   //it=Levents.insert(Levents.end(),EventClass());
  //   //it->Nevt=nevents;
  //   nevents++;
  //   //it->Pulse_Ana_Add(pls);

  //   cout << "bad event: " << dt << " " << nevents << " " << pls->Tstamp64 << endl;
  //   //T_last_good=pls->Tstamp64;
  //   return;
  // }

  // pls->Tstamp64=T_last_good+dt;



  //if (nevents>32960)
  // cout << "dt2: " << (int) pls->Chan << " " << dt << " " << nevents << " "
  //      << pls->Tstamp64 << " " << T_last_good << " " << Pstamp64 << endl;


  // if (dt>opt.tgate) { //new event (the last one by time)
  //   //add new event at the end of the list, set T_last_good and return
  //   it=Levents.insert(Levents.end(),EventClass());
  //   it->Nevt=nevents;
  //   nevents++;
  //   it->Pulse_Ana_Add(pls);

  //   //T_last_good=pls->Tstamp64;

  //   return;
  // }


  // probably coincidence event (or event coming earlier than the last)
  for (it=--Levents.end();it!=m_event;--it) {
    dt = (pls->Tstamp64 - it->T);
    if (dt > opt.tgate) {
      //add new event at the current position of the eventlist
      it=Levents.insert(it,EventClass());
      it->Nevt=nevents;
      nevents++;
      it->Pulse_Ana_Add(pls);
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      // coincidence event
      it->Pulse_Ana_Add(pls);
      return;
    }
  }

  if (debug)
    cout << "beginning: " << nevents << " " << pls->Tstamp64 << " " << dt
	 << " " << Levents.size() << endl;

  // if the current event is too early, insert it at the end of the event list
  it=Levents.insert(Levents.end(),EventClass());
  it->Nevt=nevents;
  it->Pulse_Ana_Add(pls);
  nevents++;

}
*/

void CRS::Make_Events() {

  /*
  if (Levents.empty()) {
    opt.T_acq = 0;
  }
  //if (!Levents.empty()) {
  else {
    if (b_fana) //file analyzis
      opt.T_acq = (Levents.back().T - Tstart64)*1e-9*period;
    else //acquisition
      opt.T_acq = Levents.back().T*1e-9*period;
  }
  */

  //cout << "Make_Events: T_acq: " << opt.T_acq << " " << crs->Tstart64 << " " << Levents.back().T << endl;
  
  if (opt.Tstop && opt.T_acq>opt.Tstop) {
    if (b_acq) {
      //myM->DoStartStop();
      //myM->fStart->Emit("Clicked()");
      //myM->fStart->Clicked();
    }
    else if (b_fana) {
      crs->b_fana=false;
      crs->b_stop=true;
    }
    // crs->b_stop=true;
    // crs->b_fana=false;
    // crs->b_acq=false;
    // crs->b_run=0;



    //return;
  }

  if (!opt.analyze)
    return;

  std::vector<PulseClass>::iterator pls;

  //event_list elist;
  //Levents.push_back(elist);
  //Print_Pulses(nvp);

  /*
  EventClass* firstevent;
  if (Levents.empty()) {
    firstevent=NULL;
  }
  else {
    firstevent=&Levents.back();
  }
  */

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

  for (pls=vv->begin(); pls != --vv->end(); ++pls) {
    if (!(pls->ptype&P_NOSTOP)) {
      Event_Insert_Pulse(&(*pls));
      //Print_Events();
    }
  }

  //Print_Events();
  //Select_Event(firstevent);


}

void CRS::Select_Event(EventClass *evt) {

  if (Levents.empty())
    return;

  //if (b_acq) { //acquisition is running
  if (!b_stop) { //acquisition (or file) is running
    if (evt) {
      cout << "select1: " << evt << endl;
      EvtFrm->Tevents.clear();
      EvtFrm->Tevents.push_back(*evt);
      EvtFrm->Pevents=&EvtFrm->Tevents;
      EvtFrm->d_event=EvtFrm->Pevents->begin();
      cout << "select2: " << endl;
    }
  }
  else { //acq is not running -> file analysis or stop
    cout << "not running... " << endl;
    EvtFrm->Pevents=&Levents;    
    EvtFrm->d_event=--EvtFrm->Pevents->end();
  }

  //EvtFrm->d_event=m_event;
  //cout << "Select: " << EvtFrm->d_event->T << endl;

}

void CRS::Reset_Dec() {
  sprintf(dec_opt,"wb%d",opt.dec_compr);

  f_dec = gzopen(opt.fname_dec,dec_opt);
  if (f_dec) {
    cout << "Writing parameters... : " << opt.fname_dec << endl;
    SaveParGz(f_dec);
    gzclose(f_dec);
  }
  else {
    cout << "Can't open file: " << opt.fname_dec << endl;
  }

  sprintf(dec_opt,"ab%d",opt.dec_compr);
}

void CRS::Fill_Dec(EventClass* evt) {

  UShort_t sz = 3 + sizeof(Long64_t) + rPeaks.size()*sizeof(rpeak_type);
  if (idec+sz >= DECSIZE) {
    Flush_Dec();
  }
  DecBuf[idec++] = 'D';
  UShort_t* buf2 = (UShort_t*) (DecBuf+idec);
  *buf2 = sz;
  idec+=2;
  Long64_t* buf8 = (Long64_t*) (DecBuf+idec);
  Long64_t tt = evt->State;
  tt <<= 48;
  tt += evt->T;
  *buf8 = tt;
  idec+=sizeof(Long64_t);
  for (UInt_t i=0; i<rPeaks.size(); i++) {
    rpeak_type* buf = (rpeak_type*) (DecBuf+idec);
    memcpy(buf,&rPeaks[i],sizeof(rpeak_type));
    idec+=sizeof(rpeak_type);
  }
  rPeaks.clear();

}

void CRS::Flush_Dec() {

  //idec=0;
  //return;

  sprintf(dec_opt,"ab%d",opt.dec_compr);
  f_dec = gzopen(opt.fname_dec,dec_opt);
  if (!f_dec) {
    cout << "Can't open file: " << opt.fname_dec << endl;
    idec=0;
    return;
  }

  int res=gzwrite(f_dec,DecBuf,idec);
  if (res!=idec) {
    cout << "Error writing to file: " << opt.fname_dec << " " 
	 << res << " " << idec << endl;
  }
  idec=0;

  gzclose(f_dec);
  f_dec=0;

}
