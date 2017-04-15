#include "libcrs.h"

#include <zlib.h>

#include <sys/stat.h>
#include "cyusb.h"
//#include <pthread.h>
#include "eventframe.h"
#include "romana.h"
#include <malloc.h>


//#include <TSemaphore.h>
//TSemaphore sem;
#include "TThread.h"

TMutex Mut;
TMutex stat_mut;
TCondition* cond[CRS::MAXTRANS];

using namespace std;

#ifndef CRSTXT
extern EventFrame* EvtFrm;
extern MyMainFrame *myM;
extern ParParDlg *parpar;
extern CrsParDlg *crspar;
extern CrsParDlg *chanpar;
#endif

const double MB = 1024*1024;

//bool bstart=true;
//bool btest=false;

cyusb_handle *cy_handle;
//pthread_t tid1;
TThread* trd_crs;
TThread* trd_stat;
TThread* trd_evt;

int event_thread_run;//=1;

volatile char astat[CRS::MAXTRANS];

CRS* crs;
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

void *handle_stat(void *ctx)
{

  static Long64_t bytes1=0;
  static Long64_t bytes2;

  static double t1;
  //static TTimeStamp t2;

  while (event_thread_run) {

    gSystem->Sleep(1234);

    stat_mut.Lock();
    bytes2 = crs->totalbytes;
    stat_mut.UnLock();
    //t2.Set();
    double dt = opt.T_acq - t1;
    if (dt>0.1)
      crs->mb_rate = (bytes2-bytes1)/MB/dt;
    else
      crs->mb_rate=0;

    t1=opt.T_acq;
    bytes1=bytes2;

    myM->UpdateStatus();
    //cout << "handle_stat: " << dt << " " << crs->mb_rate << endl;
  }
  return NULL;
}

void *handle_evt(void* ptr)
{
  //static int nn;

  //TEllipse * el1 = new TEllipse(0.25,0.25,.10,.20);
  //el1->SetFillColor(6);
  //el1->SetFillStyle(3008);

  while (event_thread_run) {
    //nn++;
    //cout << "trd2: " << nn << " " << myM->fTab->GetCurrent() << " " <<
    //EvtFrm->ntab << endl; 
    if (crs->b_acq && myM && myM->fTab->GetCurrent()==EvtFrm->ntab) {
      //cout << "trd2: " << nn << " " << myM->fTab->GetCurrent() << endl; 

      //std::list<EventClass>::reverse_iterator evt;
      UInt_t nn=0;
      for (EvtFrm->d_event=--crs->Levents.end();
	   EvtFrm->d_event!=--crs->Levents.begin();--EvtFrm->d_event) {
	nn++;
	if (nn>2) break;
      }
      //EvtFrm->d_event = &(*evt);

      EvtFrm->DrawEvent2();
      //Emut.UnLock();

      //Emut.UnLock();

    }
    else {
      //cout << "trd1: " << nn << " " << myM->fTab->GetCurrent() << endl;
    }

    //cout << "Block: " << EvtFrm->BlockAllSignals(false) << endl;

    gSystem->Sleep(opt.tsleep);

  }

  return 0;

}

// void *make_events_func(void *ctx)
// {
//   while (crs->event_thread_run) {
//     tcond1.Wait();
//     cout << "Thread MakeEvent" << endl;
//   }
//   return NULL;
// }

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

/*
  void *decode2(void* xxx) {
  libusb_transfer* transfer = (libusb_transfer*) xxx;
  crs->Decode2(transfer);
  return NULL;
  }
*/

static void cback(libusb_transfer *transfer) {

  static TTimeStamp t1;
  //ULong64_t rbytes=0;

  //Mut.Lock();
  TTimeStamp t2;
  //t2.Set();

  crs->npulses_buf=0;

  //Mut.UnLock();
  //goto skip;
  
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
      crs->f_raw = gzopen(opt.fname_raw,"ab");
      if (crs->f_raw) {
	int res=gzwrite(crs->f_raw,transfer->buffer,transfer->actual_length);
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

  if (crs->b_acq) {
    libusb_submit_transfer(transfer);

    stat_mut.Lock();
    crs->totalbytes+=transfer->actual_length;
    opt.T_acq = t2.GetSec()-opt.F_start.GetSec()+
      (t2.GetNanoSec()-opt.F_start.GetNanoSec())*1e-9;
    stat_mut.UnLock();
  }

  crs->nvp = (crs->nvp+1)%crs->ntrans;
  
}

CRS::CRS() {

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
    
    mylist.insert(mylist.begin(),21);
    //mylist.clear();
    //mylist.insert(mylist.begin(),21);
    std::cout << *rit << " " << mylist.back() << " " << *mylist.begin() << endl;
    std::cout << "mylist contains:";

    for (it=mylist.begin(); it!=mylist.end(); ++it)
      std::cout << ' ' << *it;
    std::cout << '\n';

    for (it=--mylist.end(); it!=--mylist.begin(); --it) {
      std::cout << ' ' << *(it);
    }
    std::cout << '\n';

    exit(1);
  */


  for (int i=0;i<MAXTRANS;i++)
    cond[i]=new TCondition(0);

  f_raw=0;
  f_dec=0;
  Fbuf=NULL;

  strcpy(Fname," ");
  Reset();

  module=0;

  cy_handle=NULL;

  event_thread_run=1;

  // b_acq=false;
  // b_fana=false;
  // bstart=true;
  debug=0;

  chanPresent=32;

  ntrans=MAXTRANS;
  //opt.buf_size=1024*1024;

  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] =NULL;
    buftr[i]=NULL;
  }

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

  trd_stat = new TThread("trd_stat", handle_stat, (void*) 0);
  trd_stat->Run();

  trd_evt = new TThread("trd_evt", handle_evt, (void*) 0);
  trd_evt->Run();

  cout << "threads created... " << endl;

  //memset(buf_out,'\0',64);
  //memset(buf_in,'\0',64);

  //buf_out[0] = 1; //get card info
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

  opt.InitPar(module);

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
  Cancel_all();
  gSystem->Sleep(50);
  r=cyusb_reset_device(cy_handle);
  cout << "cyusb_reset: " << r << endl;
  Submit_all();
  gSystem->Sleep(50);
  */

  if (Init_Transfer()) {
    return 8;
  };

  //Submit_all();

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
  if (trd_stat) {
    trd_stat->Delete();
  }
  if (trd_evt) {
    trd_evt->Delete();
  }
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

  Cancel_all();
  gSystem->Sleep(50);

  //cout << "---Free_Transfer---" << endl;

  for (int i=0;i<ntrans;i++) {
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

void CRS::Submit_all() {
  ntrans=0;
  for (int i=0;i<MAXTRANS;i++) {
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

void CRS::Cancel_all() {
  for (int i=0;i<MAXTRANS;i++) {
    int res;
    if (transfer[i]) {
      res = libusb_cancel_transfer(transfer[i]);
      //cout << i << " Cancel: " << res << endl;
    }
  }
}

int CRS::Init_Transfer() {

  //cout << "---Init_Transfer---" << endl;

  //Cancel_all();

  //cout << "---Init_Transfer2---" << endl;
  int r=cyusb_reset_device(cy_handle);
  //cout << "cyusb_reset: " << r << endl;

  for (int i=0;i<MAXTRANS;i++) {
    //if (buftr[i]) {
    //delete[] buftr[i];
    //}
    buftr[i] = new unsigned char[opt.buf_size*1024];
    memset(buftr[i],0,sizeof(unsigned char)*opt.buf_size*1024);
  }

  //cout << "---Init_Transfer 3---" << endl;
  ntrans=0;
  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] = libusb_alloc_transfer(0);
    //transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;
    
    int* ntr = new int;
    (*ntr) = i;

    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], opt.buf_size*1024, cback, ntr, 0);

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

  Submit_all();

  /*
  for (int i=0;i<ntrans;i++) {
    int res;
    res = libusb_cancel_transfer(transfer[i]);
    cout << i << " Cancel: " << res << endl;
  }
  */
  cout << "Number of transfers: " << ntrans << endl;

  if (ntrans!=MAXTRANS) {
    for (int i=ntrans;i<MAXTRANS;i++) {
      //cout << "free: " << i << endl;
      //libusb_free_transfer(transfer[i]);
      cout << "delete: " << i << endl;
      if (buftr[i])
	delete[] buftr[i];
      buftr[i]=NULL;
    }
    return 2;
  }
  gSystem->Sleep(50);

  Cancel_all();
  gSystem->Sleep(50);

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
      if (opt.enabl[chan]) {
	Command32(2,chan,6,(int)opt.kderiv[chan]);
	Command32(2,chan,7,(int)opt.threshold[chan]);
      }
      else {
	int tmp,max;
	opt.GetPar("thresh",module,chan,tmp,tmp,max);
	//cout << "Off: " << (int) chan << " " << max << endl;
	Command32(2,chan,6,0);
	Command32(2,chan,7,max);
      }
      break;
    case 2: //inv
      Command32(2,chan,1,(int)opt.inv[chan]);
      break;
    case 3: //acdc
      Command32(2,chan,0,(int)opt.acdc[chan]);
      break;
    case 4: //smooth
      Command32(2,chan,2,(int)opt.smooth[chan]);
      break;
    case 5: //Dt
      Command32(2,chan,3,(int)opt.deadTime[chan]);
      break;
    case 6: //Pre
      Command32(2,chan,4,(int)opt.preWr[chan]);
      break;
    case 7: //Len
      Command32(2,chan,5,(int)opt.durWr[chan]);
      break;
    case 8: //Gain
      Command32(2,chan,8,(int)opt.adcGain[chan]);
      break;
    case 9: //Drv
      Command32(2,chan,6,(int)opt.kderiv[chan]);
      break;
    case 10: //thresh
      Command32(2,chan,7,(int)opt.threshold[chan]);
      break;
    }
  }
  else if (module==2) {
    switch (cmd) {
    case 1: //enabl
      if (opt.enabl[chan]) {
	Command2(2,chan,3,(int)opt.kderiv[chan]);
	Command2(2,chan,4,(int)opt.threshold[chan]);
      }
      else {
	int tmp,max;
	opt.GetPar("thresh",module,chan,tmp,tmp,max);
	//cout << "Off: " << (int) chan << " " << max << endl;
	Command2(2,chan,3,0);
	Command2(2,chan,4,max);
      }
      break;
    case 2: //inv
      Command2(2,chan,1,(int)opt.inv[chan]);
      break;
    case 3: //acdc
      //no acdc
      break;
    case 4: //smooth
      Command2(2,chan,2,(int)opt.smooth[chan]);
      break;
    case 5: //Dt
      //no Dt
      break;
    case 6: //Pre
      Command2(2,chan,5,(int)opt.preWr[chan]);
      break;
    case 7: //Len
      Command2(2,chan,6,(int)opt.durWr[chan]);
      break;
    case 8: //Gain
      Command2(2,chan,0,(int)opt.adcGain[chan]);
      break;
    case 9: //Drv
      Command2(2,chan,3,(int)opt.kderiv[chan]);
      break;
    case 10: //thresh
      Command2(2,chan,4,(int)opt.threshold[chan]);
      break;
    }
  }

}

void CRS::AllParameters32()
{

  for (byte chan = 0; chan < chanPresent; chan++) {
    Command32(2,chan,0,(int)opt.acdc[chan]);
    Command32(2,chan,1,(int)opt.inv[chan]);
    Command32(2,chan,2,(int)opt.smooth[chan]);
    Command32(2,chan,3,(int)opt.deadTime[chan]);
    Command32(2,chan,4,(int)opt.preWr[chan]);
    Command32(2,chan,5,(int)opt.durWr[chan]);
    if (opt.enabl[chan]) {
      Command32(2,chan,6,(int)opt.kderiv[chan]);
      Command32(2,chan,7,(int)opt.threshold[chan]);
    }
    else {
      int tmp,max;
      opt.GetPar("thresh",module,chan,tmp,tmp,max);
      //cout << "Off: " << (int) chan << " " << max << endl;
      Command32(2,chan,6,0);
      Command32(2,chan,7,max);
    }
    Command32(2,chan,8,(int)opt.adcGain[chan]);
  }

}

void CRS::AllParameters2()
{

  for (byte chan = 0; chan < chanPresent; chan++) {
    Command2(2,chan,0,(int)opt.adcGain[chan]);
    Command2(2,chan,1,(int)opt.inv[chan]);
    Command2(2,chan,2,(int)opt.smooth[chan]);
   if (opt.enabl[chan]) {
      Command2(2,chan,3,(int)opt.kderiv[chan]);
      Command2(2,chan,4,(int)opt.threshold[chan]);
    }
   else {
     int tmp,max;
     opt.GetPar("thresh",module,chan,tmp,tmp,max);
     //cout << "Off: " << (int) chan << " " << max << endl;
     Command2(2,chan,3,0);
     Command2(2,chan,4,max);
   }

   Command2(2,chan,5,(int)opt.preWr[chan]);
   Command2(2,chan,6,(int)opt.durWr[chan]);
  }

  Command2(2,0,7,(int)opt.forcewr);

}


/*
void CRS::AllParameters32_old()
{

  memset(buf_out,'\0',64);

  buf_out[0] = 2;                 // команда "Управление"
  for (int chan = 0; chan < chanPresent; chan++)
    {
      buf_out[1] = (byte)chan;     // номер канала

      buf_out[2] = 0;         // тип параметра - связь по переменному/постоянному току
      if (opt.acdc[chan] == false) buf_out[5] = 0;
      else buf_out[5] = 1;
      SendParametr("coupling",6);

      buf_out[2] = 1;         // тип параметра - инверсия
      if (opt.inv[chan] == false) buf_out[5] = 0;
      else buf_out[5] = 1;
      SendParametr("inversion",6);

      buf_out[2] = 2;         // тип параметра - сглаживание
      buf_out[5] = (byte)opt.smooth[chan];
      SendParametr("smoothing",6);
                        
      buf_out[2] = 3;         // тип параметра - мертвое время дискриминатора
      buf_out[4] = (byte)(opt.deadTime[chan] >> 8);
      buf_out[5] = (byte)opt.deadTime[chan];
      SendParametr("dead time",6);

      buf_out[2] = 4;         // тип параметра - предзапись
      buf_out[4] = (byte)(opt.preWr[chan] >> 8);
      buf_out[5] = (byte)opt.preWr[chan];
      SendParametr("pre-record length",6);

      buf_out[2] = 5;         // тип параметра - длительность записи
      buf_out[4] = (byte)(opt.durWr[chan] >> 8);
      buf_out[5] = (byte)opt.durWr[chan];
      SendParametr("record length",6);

      if (opt.enabl[chan]) {
	buf_out[2] = 6;         // тип параметра - производная
	buf_out[4] = (byte)(opt.kderiv[chan] >> 8);
	buf_out[5] = (byte)opt.kderiv[chan];
	SendParametr("derivative",6);

	buf_out[2] = 7;         // тип параметра - порог
	buf_out[4] = (byte)(opt.threshold[chan] >> 8);
	buf_out[5] = (byte)opt.threshold[chan];
	SendParametr("threshold",6);
      }
      else {
	buf_out[2] = 6;         // тип параметра - производная
	buf_out[4] = 0;
	buf_out[5] = 0;
	SendParametr("derivative",6);

	int tmp,max;
	opt.GetPar("thresh",crs->module,chan,tmp,tmp,max);
	cout << "Off: " << chan << " " << max << endl;
	
	buf_out[2] = 7;         // тип параметра - порог
	buf_out[4] = (byte) (max >> 8);
	buf_out[5] = (byte) max;
	SendParametr("threshold",6);
      }

      buf_out[2] = 8;         // тип параметра - дополнительное усиление
      buf_out[5] = (byte)opt.adcGain[chan];
      SendParametr("gain",6);
    }

}
*/





int CRS::DoStartStop() {
  int r;
  //int transferred = 0;
  //int len=2; //input/output length must be 2, not 1

  if (!b_acq) { //start
    Reset();

    //if (module==32) {
    //Command32(7,0,0,0); //reset usb command
    //}
    
    r=cyusb_reset_device(cy_handle);
    cout << "cyusb_reset: " << r << endl;

    Submit_all();
    
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
    nsmp=0;

    opt.F_start.Set();
    if (opt.raw_write) {
      struct stat buffer;
      if (stat (opt.fname_raw, &buffer)) {
    	cout << "stat: " << stat (opt.fname_raw, &buffer) << endl;
	UShort_t mod[2];
	mod[0]=module;
	mod[1]=sizeof(opt);
    	f_raw = gzopen(opt.fname_raw,"wb");
	if (crs->f_raw) {
	  //Int_t size = sizeof(opt);
	  //opt.F_start.Print();
	  gzwrite(f_raw,mod,sizeof(mod));
	  gzwrite(f_raw,&opt,sizeof(opt));
	  gzclose(f_raw);
	}
	else {
	  cout << "Can't open file: " << opt.fname_raw << endl;
	}
      }
    }   

    //nvp=0;
    //Levents.clear();
    
    cout << "Acquisition started" << endl;
    //gettimeofday(&t_start,NULL);
    Command2(3,0,0,0);
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
    Command2(4,0,0,0);

    gSystem->Sleep(300);
    Cancel_all();
    b_stop=true;
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






void CRS::Reset() {

  opt.T_acq=0;

  //cout << "crs::reset: " << endl;
  //cout << "crs::reset: " << (int) CRS::b_fana << endl;
  //exit(1);

  b_acq=false;
  b_fana=false;
  b_stop=false;
  //bstart=true;

  nvp=0;
  Levents.clear();

  npulses=0;
  nevents=0;
  nbuffers=0;

  //npulses=0;
  npulses_buf=0;

  totalbytes=0;
  writtenbytes=0;

  //MAX_LAG=opt.event_buf/2;

  DoFopen(NULL);

  /*
  if (f_raw) {
    gzclose(f_raw);
    cout << "reset file: " << Fname << endl;
    f_raw = gzopen(Fname,"rb");
    if (!f_raw) {
      Fmode=0;
      cout << "Can't open file: " << Fname << endl;
      f_raw=0;
    }
  }
  */

}

void CRS::DoFopen(char* oname) {
  int tp=0; //1 - adcm raw; 0 - crs2/32

  if (oname)
    strcpy(Fname,oname);

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
    if (f_raw) gzclose(f_raw);
    f_raw=0;
  }

  //cout << ext << endl;
  if (f_raw) gzclose(f_raw);
  f_raw = gzopen(Fname,"rb");
  if (!f_raw) {
    Fmode=0;
    cout << "Can't open file: " << Fname << endl;
    f_raw=0;
  }

  if (tp) { //adcm raw
    cout << "ADCM RAW File: " << Fname << endl;
  }
  else {
    UShort_t mod[2];
    gzread(f_raw,mod,sizeof(Int_t));

    if (mod[0]==2) {
      Fmode=2;
      cout << "CRS2 File: " << Fname << " " << mod[1] << endl;
    }
    else if (mod[0]==32) {
      Fmode=32;
      cout << "CRS32 File: " << Fname << " " << mod[1] << endl;
    }
    else {
      Fmode=0;
      cout << "Unknown file type: " << Fname << " " << Fmode << endl;
      gzclose(f_raw);
      f_raw=0;
    }

    //YK - opt is not read from the file
    char* obuf = new char[mod[1]];
    gzread(f_raw,obuf,mod[1]);
    delete[] obuf;

    /*
    if (Fmode) {
      if (mod[1] == sizeof(opt)) {
	gzread(f_raw,&opt,sizeof(opt));
	cout << "Options are read from the file: " << Fname << endl;
      }
      else {
	char* obuf = new char[mod[1]];
	gzread(f_raw,obuf,mod[1]);
	cout << "Options are obslete: " << mod[1] << " " << sizeof(opt) 
	     << " " << Fname << endl;

	//Do something with old options

	delete[] obuf;
      }
    }
    */


  }

  if (Fmode) {
    opt.raw_write=false;
    parpar->Update();
    crspar->Update();
    chanpar->Update();

    if (Fbuf) delete[] Fbuf;
    Fbuf = new UChar_t[opt.buf_size*1024];

  }
  //cout << f_raw << endl;

}

// void CRS::DoFAna() {
//   if (!b_fana) { //start
//     b_fana=true;
//     FAnalyze();
//   }
//   else {
//     b_fana=false;
//   }
// }

void CRS::FAnalyze() {

  if (!f_raw) {
    cout << "File not open" << endl;
    return;
  }

  //Fbuf = new UChar_t[opt.buf_size*1024];

  b_stop=false;
  while (Do1Buf() && b_fana) {
    //Do1Buf();
    //if (!b_fana) break;
  }

  b_stop=true;

}

int CRS::Do1Buf() {

  int res=gzread(f_raw,Fbuf,opt.buf_size*1024);
  cout << "gzread: " << Fmode << " " << nbuffers << " " << res << endl;
  if (res>0) {
    crs->totalbytes+=res;

    if (Fmode==2) {
      Decode2(Fbuf,res);
    }
    else if (Fmode==32) {
      Decode32(Fbuf,res);
    }

    //gSystem->Sleep(500);

    if (myM && myM->fTab->GetCurrent()==EvtFrm->ntab) {
      UInt_t nn=0;
      for (EvtFrm->d_event=--crs->Levents.end();
	   EvtFrm->d_event!=--crs->Levents.begin();--EvtFrm->d_event) {
	nn++;
	if (nn>2) break;
      }

      EvtFrm->DrawEvent2();      
    }
    nbuffers++;
    myM->UpdateStatus();
    gSystem->ProcessEvents();
    return 1;
  }
  else {
    //b_fana=true;
    myM->DoAna();
    return 0;
  }


}

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

void CRS::Decode32(UChar_t *buffer, int length) {

  PulseClass *ipp;

  ULong64_t* buf8 = (ULong64_t*) buffer;
  //unsigned char* buf1 = (unsigned char*) buf8;
  //int nbuf = *(int*) transfer->user_data;

  std::vector<PulseClass> *vv = Vpulses+nvp;
  vv->clear();

  int nvp2=nvp-1;
  if (nvp2<0) nvp2=ntrans-1;

  if ((Vpulses+nvp2)->empty()) { //this is start of the acqisition
    vv->push_back(PulseClass());
    npulses++;
    ipp = &vv->back();
    ipp->Chan = buffer[7];
    ipp->ptype|=P_NOSTART; // first pulse is by default incomplete
    //it will be reset if idx8==0 in the while loop
  }
  else {
    ipp=&(Vpulses+nvp2)->back();
    // ipp points to the last pulse of the previous buffer
  }

  unsigned short frmt;
  int idx8=0;
  int idx1=0;
  ULong64_t data;

  while (idx1<length) {
    //frmt = (buffer[6] & 0xF0);
    frmt = buffer[idx1+6];
    int cnt = frmt & 0x0F;
    frmt = (frmt & 0xF0)>>4;
    data = buf8[idx8] & 0xFFFFFFFFFFFF;
    unsigned char ch = buffer[idx1+7];
    
    //printf("d32: %d %d %d %d %d\n",idx1,frmt,cnt,ch,ipp->tdif);
    //cout << "vv: " << vv << endl;
    if ((ch>=chanPresent) || (frmt && ch!=ipp->Chan)) {
      cout << "32: Bad channel: " << (int) ch
	   << " " << (int) ipp->Chan << endl;
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
      ipp->Tstamp64=data;

    }
    else if (frmt==1) {
      ipp->Control = buffer[idx1+5]+1;
      ipp->Counter = data & 0xFFFFFFFFFF;
    }
    else if (frmt==2) {

      if ((int)ipp->sData.size()>=opt.durWr[ipp->Chan]) {
	// cout << "32: ERROR Nsamp: " << nbuf << " " << cnt
	//      << " " << (ipp->Counter & 0x0F)
	//      << " " << ipp->sData.size() << " " << opt.durWr[ipp->Chan]
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx8 << " " << transfer->actual_length
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      /*
	else if (ipp->sData.size()%1000 == 0) {
	cout << "32: Nsamp: " << nbuf << " " << cnt
	<< " " << (ipp->Counter & 0x0F)
	<< " " << ipp->sData.size() << " " << opt.durWr[ipp->Chan]
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


  Make_Events(nvp);
  
}

//-------------------------------------

/*
void CRS::AllParameters2()
{
  memset(buf_out,'\0',64);

  buf_out[0] = 2;                 // команда "Управление"
  for (int chan = 0; chan < chanPresent; chan++)
    {
      buf_out[1] = (byte)chan;     // номер канала

      buf_out[2] = 0;         // Gain
      buf_out[3] = (byte)opt.adcGain[chan];
      SendParametr("gain",5);

      buf_out[2] = 1;         // тип параметра - инверсия
      if (opt.inv[chan] == false) buf_out[3] = 0;
      else buf_out[3] = 1;
      SendParametr("inversion",5);

      buf_out[2] = 2;         // тип параметра - сглаживание
      buf_out[3] = (byte)opt.smooth[chan];
      SendParametr("smoothing",5);
                        
      if (opt.enabl[chan]) {
	buf_out[2] = 3;         // тип параметра - производная
	buf_out[4] = (byte)(opt.kderiv[chan] >> 8);
	buf_out[3] = (byte)opt.kderiv[chan];
	SendParametr("derivative",5);

	buf_out[2] = 4;         // тип параметра - порог
	buf_out[4] = (byte)(opt.threshold[chan] >> 8);
	buf_out[3] = (byte)opt.threshold[chan];
	SendParametr("threshold",5);
      }
      else {
	buf_out[2] = 3;         // тип параметра - производная
	buf_out[4] = 0;
	buf_out[3] = 0;
	SendParametr("derivative",5);

	int tmp,max;
	opt.GetPar("thresh",crs->module,chan,tmp,tmp,max);
	cout << "Off: " << chan << " " << max << endl;

	buf_out[2] = 4;         // тип параметра - порог
	buf_out[4] = (byte)(max >> 8);
	buf_out[3] = (byte)max;
	SendParametr("threshold",5);
      }

      buf_out[2] = 5;         // тип параметра - предзапись
      buf_out[4] = (byte)(opt.preWr[chan] >> 8);
      buf_out[3] = (byte)opt.preWr[chan];
      SendParametr("pre-record length",5);

      buf_out[2] = 6;         // тип параметра - длительность записи
      buf_out[4] = (byte)(opt.durWr[chan] >> 8);
      buf_out[3] = (byte)opt.durWr[chan];
      SendParametr("record length",5);

      buf_out[2] = 7;         // тип параметра - принудительная запись
      buf_out[3] = (byte)opt.forcewr;
      SendParametr("gain",5);
    }
}
*/

 void CRS::Decode2(UChar_t* buffer, int length) {

  PulseClass *ipp;

  unsigned short* buf2 = (unsigned short*) buffer;
  //int nbuf = *(int*) transfer->user_data;

  std::vector<PulseClass> *vv = Vpulses+nvp;
  vv->clear();

  int nvp2=nvp-1;
  if (nvp2<0) nvp2=ntrans-1;

  if ((Vpulses+nvp2)->empty()) { //this is start of the acqisition
    vv->push_back(PulseClass());
    npulses++;
    ipp = &vv->back();
    ipp->Chan = ((*buf2) & 0x8000)>>15;
    ipp->ptype|=P_NOSTART; // first pulse is by default incomplete
    //it will be reset if idx8==0 in the while loop
  }
  else {
    ipp=&(Vpulses+nvp2)->back();
    // ipp points to the last pulse of the previous buffer
  }

  unsigned short frmt;
  int idx2=0;
  int len = length/2;

  while (idx2<len) {

    unsigned short uword = buf2[idx2];
    frmt = (uword & 0x7000)>>12;
    short data = uword & 0xFFF;
    unsigned char ch = (uword & 0x8000)>>15;

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
	vv->push_back(PulseClass());
	npulses++;
	ipp = &vv->back();
      }
      else { //idx2==0 -> pulse has start
	ipp->ptype&=~P_NOSTART; // reset ptype, as this is a good pulse
      }
      ipp->Chan = ch;
      ipp->Tstamp64=data;

    }
    else if (frmt<4) {
      Long64_t t64 = data;
      ipp->Tstamp64+= (t64 << (frmt*12));
    }
    else if (frmt==4) {
      ipp->Counter=data;
    }
    else if (frmt==5) {
      if ((int)ipp->sData.size()>=opt.durWr[ipp->Chan]) {
	// cout << "2: Nsamp error: " << ipp->sData.size()
	//      << " " << (int) ch << " " << (int) ipp->Chan
	//      << " " << idx2
	//      << endl;
	ipp->ptype|=P_BADSZ;
      }
      //else {
      ipp->sData.push_back((data<<21)>>21);
      //}
    }

    idx2++;
  }

  //return;
  //cond[nbuf]->Signal();

  //cout << "decode2a: " << idx2 << endl;

  //Fill_Tail(nvp);
  Make_Events(nvp);

  //nvp++;
  //if (nvp>=ntrans) nvp=0;
  //nvp = (nvp+1)%ntrans;

}

//-------------------------------

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

void CRS::Event_Insert_Pulse(PulseClass *newpulse) {

  //if (nbuffers < 1) {
  //newpulse->PrintPulse();
  //}

  newpulse->FindPeaks();
  newpulse->PeakAna();

  std::list<EventClass>::reverse_iterator rl;
  int nn=0;
  for (rl=Levents.rbegin(); rl!=Levents.rend(); ++rl) {
    Long64_t dt = newpulse->Tstamp64 - rl->T;
    if (dt > opt.tgate1) {
      //add new event at the current position of the eventlist
      Levents.insert(rl.base(),EventClass());
      nevents++;
      rl->Pulse_Ana_Add(newpulse);
      if (debug && nn>10)
	cout << "nn: " << nn << " " << Levents.size() << " " << opt.ev_max << endl;
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate1) { //add newpulse to existing event
      // coincidence event
      rl->Pulse_Ana_Add(newpulse);
      if (debug && nn>10)
	cout << "nn: " << nn << endl;
      return;
    }
    nn++;
    if (nn>=opt.event_lag) {
    }
  }

  
  if (debug && nn>10)
    cout << "nn: " << nn << endl;
  if (debug) 
    cout << "beginning" << endl;
  //add new event at the beginning of the eventlist
  Levents.insert(Levents.begin(),EventClass());
  nevents++;
  Levents.begin()->Pulse_Ana_Add(newpulse);

}

void CRS::Make_Events(int nvp) {

  std::vector<PulseClass>::iterator newpulse;

  int nvp2=nvp-1;
  if (nvp2<0) nvp2=ntrans-1;

  //first insert last pulse from the previous buffer
  std::vector<PulseClass> *vv = Vpulses+nvp2;
  if (!vv->empty() && vv->back().ptype&P_NOSTOP) {
    //cout << "Make_events: " << (int) newpulse->ptype << " " << newpulse->Tstamp64 << endl;
    //vv->back().FindPeaks();
    //vv->back().PeakAna();
    Event_Insert_Pulse(&vv->back());
  }

  //cout << "here1" << endl;
  //now insert all pulses from the current buffer, except last one
  vv = Vpulses+nvp;

  if (vv->size()<=1) return; //if vv contains 0 or 1 event, don't analyze it 

  for (newpulse=vv->begin(); newpulse != --vv->end(); ++newpulse) {
    if (!(newpulse->ptype&P_NOSTOP)) {
      //newpulse->FindPeaks();
      //newpulse->PeakAna();
      Event_Insert_Pulse(&(*newpulse));
    }
  }

  //YK don't understand why this is needed...
  //-----------
  std::list<EventClass>::reverse_iterator evt;
  UInt_t nn=0;
  for (evt=crs->Levents.rbegin();evt!=crs->Levents.rend();evt++) {
    nn++;
    if (nn>2) break;
  }
  //cout << "Make_Events: " << evt->T << endl;
  //PEvent();
  //-----------


  //Analyse events and clean (part of) the event list
  if ((int) Levents.size()>opt.ev_max) {
    //cout << "Size1: " << Levents.size() << " " << opt.ev_max-opt.ev_min << endl;
    Int_t nn=0;
    std::list<EventClass>::iterator rl;
    std::list<EventClass>::iterator next;

    for (rl=Levents.begin(); rl!=Levents.end(); rl=next) {
      next = rl;
      ++next;
      // if (rl->pulses.size()!=2) {
      // 	cout << "Event: " << nn << " " << rl->pulses.size() << " " << rl->T << endl;
      // }
      FillHist(&(*rl));
      Levents.erase(rl);
      nn++;
      if ((int)Levents.size()<=opt.ev_min) break;
    }
    //cout << "Make_Events Size2: " << Levents.size() << endl;
  }

}
