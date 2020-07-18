#include "libcrs.h"

//#include <iostream>
#include <fstream>
#include <zlib.h>
#include <sys/stat.h>
#include <iomanip>

#ifdef CYUSB
#include "cyusb.h"
#endif

//#include <pthread.h>
#include "romana.h"

//#include <malloc.h>
#include <TClass.h>
#include <TCanvas.h>
#include <TDataMember.h>

//#include <TSemaphore.h>
//TSemaphore sem;
#include "TThread.h"
#include "TMutex.h"
#include "TRandom.h"
#include "TApplication.h"

//TMutex Emut3;
TMutex stat_mut;
//TMutex ana_mut;

TMutex dec_mut;

TMutex cmut;

const int BFMAX=999999999;

using namespace std;

extern MemInfo_t minfo;
extern ProcInfo_t pinfo;
// extern double rmem;

extern EventFrame* EvtFrm;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern ErrFrame* ErrFrm;
extern HClass* hcl;
extern ParParDlg *parpar;
extern DaqParDlg *daqpar;
extern AnaParDlg *anapar;
extern PikParDlg *pikpar;

extern int debug; // for printing debug messages

extern char startdir[200];
const double MB = 1024*1024;
const Long64_t iMB = 1024*1024;

//MemInfo_t info;

TRandom rnd;

//Int_t ev_max;

//bool bstart=true;
//bool btest=false;

int event_thread_run;//=1;
//pthread_t tid1;
TThread* trd_crs=0;
//TThread* trd_stat;
//TThread* trd_evt;
//TThread* trd_dum;
//TThread* trd_ana;

const Long64_t P64_0=-123456789123456789;

std::list<EventClass>::iterator m_event; //важный параметр
// m_event - указатель на первое непроанализированное событие в Levents
//std::list<EventClass>::iterator m_end; //важный параметр
// расстояние m_end до конца списка не должно быть больше,
// чем opt.ev_min
// в Ana данные анализируются от m_event до m_end
// или до Levents.end(), если конец анализа
// данные удаляются от Levents.begin() до m_event
// (но оставляется opt.ev_max событий)

//int MT=1;

const ULong64_t sixbytes=0xFFFFFFFFFFFF;

//const Long64_t GLBSIZE=2147483648;//1024*1024*1024*2; //1024 MB
//const Long64_t GLBSIZE=1024*1024*1024; //1024 MB

int tr_size; //=opt.usb_size*1024; - размер трансфера в байтах

Long64_t gl_sz;
const Long64_t gl_off = iMB; //1MB, was 1024*128 - офсет GLBuf относительно GLBuf2
UChar_t* GLBuf;
UChar_t* GLBuf2;

int b_start[CRS::MAXTRANS]; //start of local buffer(part of GLBuf), included
int b_fill[CRS::MAXTRANS]; //start of local buffer for reading/filling
int b_end[CRS::MAXTRANS]; //end of local buffer(part of GLBuf), excluded

UInt_t gl_iread; //current number of readbuf or cback [0.. infinity)
UInt_t gl_ivect; //current number of pstruct in make_event [0.. infinity)
UInt_t gl_ibuf; //current buffer for decode* [0 .. gl_Nbuf]
UInt_t gl_Nbuf; //maximal number of buffers for decode*
//int gl_ntrd=6; //number of decode threads (and also sub-buffers)

int decode_thread_run;
int mkev_thread_run;
int ana_thread_run;
int dec_nr[CRS::MAXTRANS];

TThread* trd_dec[CRS::MAXTRANS];
//TCondition dec_cond[CRS::MAXTRANS];

UInt_t dec_iread[CRS::MAXTRANS];
// dec_iread[i]=0 в начале
// handle_decode крутится для n потоков
// пока dec_iread==0 для данного потока, handle_decode ждет
// dec_iread[i]=0 если i-й буфер обработан. В буфер можно читать.
// Если dec_iread[i]=1, i-й буфер еще не обработан


TThread* trd_mkev;
//TCondition mkev_cond[CRS::MAXTRANS];
//int mkev_check[CRS::MAXTRANS];


int ana_all;
TThread* trd_ana;
//TCondition ana_cond;
//int ana_check;



//TCondition evt_cond;
//int evt_check;

//TThread* trd_ev;
//TCondition ev_cond;
//int ev_check;

//int make_event_thread_run;
//int ana2_thread_run;

//UInt_t list_min=100;

volatile char astat[CRS::MAXTRANS];

CRS* crs;
extern Coptions cpar;
extern Toptions opt;
int chan_in_module;


#ifdef TIMES
TTimeStamp tt1[10];
TTimeStamp tt2[10];
double ttm[10];
double dif;
#endif

//TCondition tcond1(0);

//void printhlist(int n);
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
  cout << "CRS thread created... " << endl;
  while (event_thread_run) {
    libusb_handle_events_completed(NULL,NULL);
  }
  cout << "CRS thread finished... " << endl;
  return NULL;
}

static void cback(libusb_transfer *trans) {

  if (trans->actual_length) {

    while (dec_iread[gl_ibuf]) {
      ++crs->errors[8];
      gSystem->Sleep(1);
    }

    int itr = *(int*) trans->user_data;
    int i_prev = (itr+crs->ntrans-1)%crs->ntrans; //previous itr
    //int i_next = (itr+1)%crs->ntrans; //next itr

    UChar_t* next_buf=crs->transfer[i_prev]->buffer + tr_size;
    if (next_buf+tr_size > GLBuf+gl_sz) {
      next_buf=GLBuf;
    }

    double rr =
      double(trans->buffer-GLBuf+trans->actual_length)/gl_sz;
    UInt_t nn = (rr+1e-6)*gl_Nbuf;

    if (opt.decode) {
      if (nn!=gl_ibuf) {
	// cout << "cback2: " << itr << " " << nn << " " << gl_ibuf << endl;
	int length=trans->buffer-GLBuf-b_fill[gl_ibuf]+trans->actual_length;
	b_end[gl_ibuf]=b_fill[gl_ibuf]+length;
	// prnt("ss2d2d2d3ds;",KYEL,"cback:",gl_iread, itr, gl_ibuf,CheckMem(),RST);
	crs->AnaBuf(gl_ibuf);
	//gl_ibuf=gl_iread%gl_Nbuf; //New YK

      }
    } //if decode

    if (opt.raw_write && !opt.raw_flag) {
      crs->f_raw = gzopen(crs->rawname.c_str(),crs->raw_opt);
      if (crs->f_raw) {
	int res=gzwrite(crs->f_raw,trans->buffer,trans->actual_length);
	gzclose(crs->f_raw);
	crs->rawbytes+=res;
      }
      else {
	cout << "Can't open file: " << crs->rawname.c_str() << endl;
      }
    }

    trans->buffer=next_buf;
    if (crs->b_acq) {
      libusb_submit_transfer(trans);
    }

    crs->nbuffers++;

    stat_mut.Lock();
    crs->inputbytes+=trans->actual_length;
    //opt.T_acq = t2.GetSec()-opt.F_start.GetSec()+
    //(t2.GetNanoSec()-opt.F_start.GetNanoSec())*1e-9;
    //opt.T_acq = (Long64_t(gSystem->Now()) - crs->T_start)*0.001;
    //cout << "T_acq: " << opt.T_acq << " " << crs->T_start << endl;

    stat_mut.UnLock();
  } // if (trans->actual_length)
}

#endif //CYUSB

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

void *handle_decode(void *ctx) {
  UInt_t ibuf = *(int*) ctx;

  // cmut.Lock();
  // cout << "Decode thread started: " << ibuf << endl;
  // cmut.UnLock();

  while (decode_thread_run) {

    while(!dec_iread[ibuf])
      //dec_cond[ibuf].Wait();
      gSystem->Sleep(1);

    if (!decode_thread_run) {
      break;
    }

    // prnt("ss2d2d2d .1f d ds;",KWHT,"decWk:",gl_iread, ibuf, gl_ibuf,opt.T_acq,
    // 	crs->Levents.size(),crs->Bufevents.size(),RST);

    crs->Decode_switch(ibuf);

  } //while

  // cmut.Lock();
  // cout << "Decode thread deleted: " << ibuf << endl;
  // cmut.UnLock();
  return NULL;
}

void *handle_mkev(void *ctx) {
  //return 0;

  // cmut.Lock();
  // cout << "Mkev thread started: " << endl;
  // cmut.UnLock();
  //UInt_t ibuf=gl_ibuf;
  //CRS::plist_iter it;

  std::list<CRS::eventlist>::iterator BB;
	
  while (mkev_thread_run) {

    // gSystem->GetMemInfo(&info);
    // cout << "\033[33m";
    // cout << "Make_events1: " << gl_ivect << " " << crs->Bufevents.size()
    // 	 << " " << crs->Levents.size() << " " << info.fMemUsed << endl;
    // cout << "\033[0m";

    bool fdec=false; //proper dec.. finished
    while(!fdec && mkev_thread_run) {
      //cout << "mkev: " << crs->Bufevents.size() << endl;
      for (BB = crs->Bufevents.begin(); BB!=crs->Bufevents.end();++BB) {
	// cout << "mkev: " << crs->Bufevents.size() << BB->front().Nevt
	//      << " " << gl_ivect << " " << (int) BB->front().State << endl;
	if (BB->back().Nevt==gl_ivect && BB->back().State==123) {
	  fdec=true;
	  break;
	}
      }
      gSystem->Sleep(1);
    }

    if (!mkev_thread_run) {
      break;
    }

    crs->Make_Events(BB);
    gl_ivect++;

  } //while (mkev_thread_run)

  // cmut.Lock();
  // cout << "Mkev thread deleted: " << endl;
  // cmut.UnLock();
  return NULL;

}

void *handle_ana(void *ctx) {

  //TTimeStamp tt1,tt2;
  //return 0;

  // cmut.Lock();
  // cout << "Ana thread started: " << endl;
  // cmut.UnLock();
  while (ana_thread_run) {

    while (ana_thread_run &&
	   //((int)crs->Levents.size()<=opt.ev_min ||
	   //m_event==m_end
	   (int) crs->Levents.size()<=opt.ev_max
	   ) {
      gSystem->Sleep(1);
    }

    // вызываем ana2 после каждого cback или DoBuf
    // Входные данные: Levents, МЕНЯЕТСЯ во время работы ana2 (если MT)
    // если ana_all==0 ->
    // анализируем данные от Levents.begin() до Levents.end()-opt.ev_min
    // если ana_all!=0 ->
    // анализируем данные от Levents.begin() до Levents.end()

    int nmax = crs->Levents.size()-opt.ev_max; //number of events to be deleted

    // cmut.Lock();
    // cout << "Ana2_MT: " << crs->Levents.size() << " " << ana_all << endl;
    // cmut.UnLock();

    if (m_event==crs->Levents.end()) {
      m_event=crs->Levents.begin();
    }

    std::list<EventClass>::iterator m_end = crs->Levents.end();
    //m_end = crs->Levents.end();
    if (!ana_all) { //analyze up to ev_min events
      int nmin=opt.ev_min;
      while (m_end!=m_event && nmin>0) {
	--m_end;
	--nmin;
      }
      //std::advance(m_end,-opt.ev_min);
    }

    //cout << KBLU << "Levents2: " << crs->Levents.size() << " " << crs->nevents << " " << nmax << RST << endl;
    //tt1.Set();
    //int n1=crs->nevents2;

    // analyze events from m_event to m_end
    while (m_event!=m_end) {
      if ((int)m_event->pulses.size()>=opt.mult1 &&
	  (int)m_event->pulses.size()<=opt.mult2) {

	m_event->FillHist(true);
	m_event->FillHist(false);
	//it->FillHist_old();
	if (!opt.maintrig || hcl->cut_flag[opt.maintrig]) {
	  ++crs->nevents2;
	  if (opt.dec_write) {
	    //crs->Fill_Dec73(&(*m_event));
	    //crs->Fill_Dec74(&(*m_event));
	    //crs->Fill_Dec75(&(*m_event));
	    //crs->Fill_Dec76(&(*m_event));
	    //crs->Fill_Dec77(&(*m_event));
	    //crs->Fill_Dec78(&(*m_event));
	    crs->Fill_Dec79(&(*m_event));
	  }
	  if (opt.raw_write && opt.raw_flag) {
	    crs->Fill_Raw(&(*m_event));
	  }
	}
	++m_event;
      }
      else {
	//cout << "Erase1: " << m_event->Nevt << " " << m_event->pulses.size() << endl;
	m_event=crs->Levents.erase(m_event);
      }
    }

    //tt2.Set();
    //crs->DT4=tt2.AsDouble()-tt1.AsDouble();
    //double tt=tt2.AsDouble()-tt1.AsDouble();
    //int n2=crs->nevents2-n1;
    //cout << KRED << "Levents3: " << crs->Levents.size() << " " << crs->nevents << " " << nmax << " " << tt << " " << tt/n2*1e7 << RST << endl;
    //cout << RST << endl;

    // erase events if the list is too long
    for (event_iter it=crs->Levents.begin(); it!=m_event && nmax>0;--nmax) {
      it=crs->Levents.erase(it);
    }


    if (ana_all) {
      if (opt.dec_write) {
	crs->Flush_Dec();
      }
      if (opt.raw_write && opt.raw_flag) {
	crs->Flush_Raw();
      }
    }

    //tt2.Set();
    //crs->DT4=tt2.AsDouble()-tt1.AsDouble();


    crs->L4=double(crs->Levents.size())/opt.ev_max;
    if (crs->L4>2) {
      ++crs->N4;
    }
    else {
      crs->N4=0;
    }
    //cout << KGRN << "Levents4: " << crs->Levents.size() << " " << crs->nevents << " " << nmax << " " << crs->L4 << " " << crs->N4 << RST << endl;

    // fill Tevents for EvtFrm::DrawEvent2
    if (EvtFrm) {
      EvtFrm->Tevents.clear();
      if (m_event!=crs->Levents.end()) {
	EvtFrm->Tevents.push_back(*m_event);
      }
      else if (!crs->Levents.empty()) {
	EvtFrm->Tevents.push_back(crs->Levents.back());     
      }
      EvtFrm->d_event=EvtFrm->Pevents->begin();
    }
	
    //cout << "Levents5: " << crs->Levents.size() << " " << crs->nevents << endl;
    // cmut.Lock();
    // cout << "Ana2_MT_end: " << crs->Levents.size()
    // 	 << " " << std::distance(crs->Levents.begin(),m_event)
    // 	 << " " << std::distance(m_event,crs->Levents.end())
    // 	 << " " << ana_all << endl;
    // cout << "---------------------------------" << endl;
    // cmut.UnLock();


    //ana_mut.UnLock();

  } //while (ana_thread_run)
  // cmut.Lock();
  // cout << "Ana thread deleted: " << endl;
  // cmut.UnLock();
  return NULL;
} //handle_ana

// void *handle_ev(void *ctx) {
//   int nvp=0;
//   cout << "Event thread started: " << endl;
//   while (decode_thread_run) {
//     while(!evt_check)
//       evt_cond.Wait();

//     cout << "mkevent: " << endl;
//     evt_check=0;
//   }
//   return NULL;
// }

/*
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
*/

/*
  int CRS::Set_Trigger() {
  int len = strlen(opt.maintrig);
  if (len==0) {
  b_maintrig=false;
  return 0;
  }
  else {
  maintrig = TFormula("Trig",opt.maintrig);
  int ires = maintrig.Compile();
  if (ires) { //bad formula
  b_maintrig=false;
  return 1;
  }
  else {
  crs->b_maintrig=true;
  return 2;
  }
  }
  }
*/

void CRS::Ana_start() {
  //set initial variables for analysis
  //should be called before first call of ana2
  // b_mem=false;

  if (opt.ev_min>=opt.ev_max) {
    opt.ev_min=opt.ev_max/2;
  }
  //Set_Trigger();
  for (int i=0;i<MAX_CH;i++) {
    b_len[i] = opt.Base2[i]-opt.Base1[i]+1;
    p_len[i] = opt.Peak2[i]-opt.Peak1[i]+1;
    w_len[i] = opt.W2[i]-opt.W1[i]+1;
  }
  //cout << "Command_start: " << endl;
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
  gzFile ff = gzopen("last.par","wb");
  SaveParGz(ff,module);
  gzclose(ff);

  memset(dec_iread,0,sizeof(dec_iread));
  if (opt.nthreads>1) {
    decode_thread_run=1;
    mkev_thread_run=1;
    ana_thread_run=1;

    //cout << "gl_Nbuf: " << gl_Nbuf << endl;
    //exit(1);
    for (UInt_t i=0;i<gl_Nbuf;i++) {
      //dec_iread[i]=0;
      //dec_check[i]=0;
      //dec_finished[i]=0;
      char ss[50];
      sprintf(ss,"trd_dec%d",i);
      dec_nr[i] = i;
      trd_dec[i] = new TThread(ss, handle_decode, (void*) &dec_nr[i]);
      trd_dec[i]->Run();

      //mkev_check[i]=0;
    }

    //cout << "dec ready: " << endl;
    //gSystem->Sleep(5000);
    trd_mkev = new TThread("trd_mkev", handle_mkev, (void*) 0);
    trd_mkev->Run();

    //cout << "mkev ready: " << endl;
    //gSystem->Sleep(5000);
    ana_all=0;
    //ana_check=0;

    trd_ana = new TThread("trd_ana", handle_ana, (void*) 0);
    trd_ana->Run();
    //gSystem->Sleep(5000);

    gSystem->Sleep(100);
  }

  //cout << "Ana_start finished..." << endl;
}

void CRS::Ana2(int all) {
  // вызываем ana2 после каждого cback или DoBuf
  // Входные данные: Levents, МЕНЯЕТСЯ во время работы ana2 (если MT)
  // если all==0 ->
  // анализируем данные от Levents.begin() до Levents.end()-opt.ev_min
  // если all!=0 ->
  // анализируем данные от Levents.begin() до Levents.end()

  //Print_Events();

  int nmax = crs->Levents.size()-opt.ev_max; //number of events to be deleted

  // cmut.Lock();
  // cout << "Ana2_MT: " << crs->Levents.size() << " " << all << endl;
  // cmut.UnLock();

  if (m_event==crs->Levents.end()) {
    m_event=crs->Levents.begin();
  }

  std::list<EventClass>::iterator m_end = crs->Levents.end();
  //m_end = crs->Levents.end();
  if (!all) { //analyze up to ev_min events
    int nmin=opt.ev_min;
    while (m_end!=m_event && nmin>0) {
      --m_end;
      --nmin;
    }
    //std::advance(m_end,-opt.ev_min);
  }

  // analyze events from m_event to m_end
  while (m_event!=m_end) {
    if ((int)m_event->pulses.size()>=opt.mult1 &&
	(int)m_event->pulses.size()<=opt.mult2) {

      m_event->FillHist(true);
      m_event->FillHist(false);
      //it->FillHist_old();
      if (!opt.maintrig || hcl->cut_flag[opt.maintrig]) {
	++crs->nevents2;
	if (opt.dec_write) {
	  //crs->Fill_Dec73(&(*m_event));
	  //crs->Fill_Dec74(&(*m_event));
	  //crs->Fill_Dec75(&(*m_event));
	  //crs->Fill_Dec76(&(*m_event));
	  //crs->Fill_Dec77(&(*m_event));
	  //crs->Fill_Dec78(&(*m_event));
	  crs->Fill_Dec79(&(*m_event));
	}
	if (opt.raw_write && opt.raw_flag) {
	  crs->Fill_Raw(&(*m_event));
	}
      }
      ++m_event;
    }
    else {
      //cout << "Erase1: " << m_event->Nevt << " " << m_event->pulses.size() << endl;
      m_event=Levents.erase(m_event);
    }
  }

  // erase events if the list is too long
  for (event_iter it=crs->Levents.begin(); it!=m_event && nmax>0;--nmax) {
    it=crs->Levents.erase(it);
  }

  //cout << "Levents3: " << Levents.size() << " " << nevents << endl;

  // removed on 06.02.2020
  // if (all) {
  //   if (opt.dec_write) {
  //     crs->Flush_Dec();
  //   }
  //   if (opt.raw_write && opt.raw_flag) {
  //     crs->Flush_Raw();
  //   }
  // }

  //cout << "Levents4: " << Levents.size() << " " << nevents << endl;

  // fill Tevents for EvtFrm::DrawEvent2
  if (EvtFrm) {
    EvtFrm->Tevents.clear();
    if (m_event!=Levents.end()) {
      EvtFrm->Tevents.push_back(*m_event);
    }
    else if (!Levents.empty()) {
      EvtFrm->Tevents.push_back(Levents.back());     
    }
    EvtFrm->d_event=EvtFrm->Pevents->begin();
  }

  //cout << "Levents5: " << Levents.size() << " " << nevents << endl;

} //ana2

CRS::CRS() {

  /*

    TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();

    TDataMember *dm;
    TIter nextd(lst);
    while ((dm = (TDataMember *) nextd())) {
    cout << dm->GetName() << " " << dm->GetDataType() << endl;
    }
    exit(1);
  
    TString s1 = "123124";
    cout << s1.First('3') << " " << s1.First('[') << " " << kNPOS << endl;
    exit(1);

    // begin test block
    std::list<int> mylist;
    std::list<int>::iterator it;
    std::list<int>::reverse_iterator rit;

    // set some initial values:
    for (int i=1; i<=11; ++i) mylist.push_back(i*10); // 1 2 3 4 5

    for (it=mylist.begin(); it!=mylist.end(); ++it)
    std::cout << ' ' << *it;
    std::cout << '\n';

    for (rit=mylist.rbegin(); rit!=mylist.rend(); ++rit) {
    if (*rit<75) {
    it = mylist.insert(rit.base(),75);
    cout << *it << endl;
    break;
    }
    }

    for (it=mylist.begin(); it!=mylist.end(); ++it)
    std::cout << ' ' << *it;
    std::cout << '\n';

    // for (rit=mylist.rbegin(); rit!=mylist.rend(); ++rit)
    //   std::cout << ' ' << *rit;
    // std::cout << '\n';

    // --rit;
    // std::cout << *rit << endl;

    it=mylist.begin();
    std::advance(it,3);

    cout << *it << endl;
    cout << std::distance(mylist.begin(),it) << " " << std::distance(it,mylist.end()) << endl;
	

    exit(1);
  */

  /*
    for (int x = 0, y = 0; (y <10 || x <10); x++, y++){
    cout << x << " " << y << endl;
    }
    exit(1);

    //cout << TClass::GetClass("Toptions")->GetClassVersion() << endl;
    //exit(1);

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

  GLBuf2 = NULL;//new UChar_t[GLBSIZE];
  GLBuf = NULL;//new UChar_t[GLBSIZE];
  //memset(GLBuf,0,GLBSIZE);

  dummy_pulse.ptype=P_BADPULSE;
  dummy_pulse.Chan=254;

  dummy_peak.Area=0;
  dummy_peak.Height=0;
  dummy_peak.Width=0;
  dummy_peak.Width2=0;
  //dummy_peak.Width3=0;
  dummy_peak.Time=-100;
  dummy_peak.Pos=0;

  for (int i=0;i<MAX_CHTP;i++) {
    type_ch[i]=255;
  }

  MAXTRANS2=MAXTRANS7;
  //memset(Pre,0,sizeof(Pre));

  Fmode=0;
  opt.Period=5;

  f_raw=0;
  f_read=0;
  f_dec=0;
  //f_tree=0;
  //Tree=0;

  batch=false;
  silent=false;
  b_acq=false;
  b_fana=false;
  b_stop=true;
  // b_mem=false;
  b_run=0;
  //justopened=true;

  strcpy(raw_opt,"ab");
  //strcpy(dec_opt,"ab");

  DecBuf=new UChar_t[2*DECSIZE]; //2*1 MB
  RawBuf=new UChar_t[RAWSIZE]; //10 MB

  //strcpy(Fname," ");
  memset(Fname,0,sizeof(Fname));
  DoReset();

  module=0;

#ifdef CYUSB
  cy_handle=NULL;
#endif

  event_thread_run=1;

  // b_acq=false;
  // b_fana=false;
  // bstart=true;

  chan_in_module=MAX_CH;

  ntrans=MAXTRANS7;
  //opt.usb_size=1024*1024;

  for (int i=0;i<MAXTRANS7;i++) {
    transfer[i] =NULL;
    buftr[i]=NULL;
    //Fbuf[i]=NULL;
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
  delete[] RawBuf;
  delete[] GLBuf2;

}

/*
  void CRS::Dummy_trd() {
  trd_dum = new TThread("trd_dum", handle_dum, (void*) 0);
  trd_dum->Run();
  }
*/

void CRS::DoResetUSB() {
#ifdef CYUSB
  if (!b_stop)
    return;
  if (Fmode==1 && module>=32) {
    cout << "Reset USB" << endl;

    event_thread_run=0;
    if (Fmode==1) {
      //cyusb_close();
      if (trd_crs) {
	trd_crs->Delete();
      }
    }
		
    Command32(7,0,0,0); //reset usb command
    Detect_device();
  }
  else {
    cout << "Module not found or reset not possible" << endl;
  }  
#endif
}

#ifdef CYUSB

int CRS::Detect_device() {

  int r;
  //Short_t firmw=0;

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

  event_thread_run=1;
  trd_crs = new TThread("trd_crs", handle_events_func, (void*) 0);
  trd_crs->Run();

  Command2(4,0,0,0);

  int sz;
  memset(buf_in,0,sizeof(buf_in));
  sz = Command32(1,0,0,0);

  Short_t device_code=buf_in[1];
  Short_t Serial_n=buf_in[2];
  Short_t nplates=buf_in[3];
  Short_t ver_po=buf_in[4];

  //cout << "Info: " << sz << endl;
  cout << "Device code: " << device_code << endl;
  cout << "Serial Nr: " << Serial_n << endl;
  cout << "Number of working plates: " << nplates << endl;
  cout << "Firmware version: " << ver_po << endl;
  //cout << "PO version: " << int(buf_in[4]) << endl;

  //for (int i=0;i<sz;i++) {
  //cout << int(buf_in[i]) << " ";
  //}
  //cout << endl;
  int nch2=0;
	
  switch (device_code) {
  case 1: //crs-32
  case 2: //crs-6/16
  case 3: //crs-16 or crs-2
    //crs2
    if (ver_po==0) { // -> crs2
      module=22;
      chan_in_module=2;
      nch2=2;
      opt.Nchan=2;
      for (int j=0;j<chan_in_module;j++) {
	type_ch[j]=0;
      }
      break;
    }

    //crs32/16/6
    module=32;
    chan_in_module=nplates*4;
    if (ver_po==1) {//версия ПО=1
      for (int i=0;i<nplates;i++) {
	cout << "Channels(" << i << "):";
	for (int j=0;j<4;j++) {
	  type_ch[i*4+j]=0;
	  cout << " " << type_ch[i*4+j];
	  nch2++;
	}
	cout << endl;
	//cout << i << " " << sz << endl;
      }
    }
    else {//версия ПО=2 или выше
      //chan_in_module=4;
      sz = Command32(10,0,0,0);
      sz--;
      for (int i=0;i<nplates;i++) {
	cout << "Channels(" << i << "):";
	for (int j=0;j<4;j++) {
	  type_ch[i*4+j]=buf_in[sz];
	  cout << " " << type_ch[i*4+j];
	  nch2++;
	}
	cout << endl;
	sz--;
	//cout << i << " " << sz << endl;
      }
      if (ver_po==3) {//версия ПО=3
	module=33;
      }
      else if (ver_po>=4) {//версия ПО=4 или выше
	module=34;
      }
    }

    break;

  case 4: //crs-8/16
    module=41;
    chan_in_module=nplates*8;
    for (int j=0;j<chan_in_module;j++) {
      type_ch[j]=2;
      cout << " " << type_ch[j];
      nch2++;
    }
    cout << endl;
    break;

  default:
    cout << "unknown device: " << endl;
    exit(1);
  }

  if (opt.Nchan>nch2) {
    opt.Nchan=nch2;
  }

  cout << "module: " << module << " chan_in_module: " << chan_in_module << endl;

  if (module>=22)
    Fmode=1;

  InitBuf();

  if (Init_Transfer()) {
    return 8;
  };

  //Submit_all(MAXTRANS);
  Command2(4,0,0,0);

  return 0;

}

int CRS::SetPar() {
  switch (module) {
  case 22:
    AllParameters2();
    break;
  case 32:
    AllParameters32();
    break;
  case 33:
    AllParameters33();
    break;
  case 34:
    AllParameters34();
    break;
  case 41:
    AllParameters41();
    break;
  default:
    cout << "SetPar Error! No module found" << endl;
    return 3;
  }

  return 0;

  //threshold[11]=100;

}

void CRS::Free_Transfer() {

  Cancel_all(MAXTRANS7);
  gSystem->Sleep(50);

  //cout << "---Free_Transfer---" << endl;

  //for (int i=0;i<ntrans;i++) {
  for (int i=0;i<MAXTRANS7;i++) {
    //cout << "free: " << i << " " << (int) transfer[i]->flags << endl;
    //int res = libusb_cancel_transfer(transfer[i]);
    libusb_free_transfer(transfer[i]);

    //transfer[i]=NULL;

  }

  for (int i=0;i<MAXTRANS7;i++) {
    if (buftr[i]) {
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

  // if (opt.rbuf_size<=opt.usb_size*2) {
  //   opt.rbuf_size=opt.usb_size*2;
  // }

  /*
    for (int i=0;i<MAXTRANS;i++) {
    //buftr[i] = new unsigned char[opt.usb_size*1024];
    buftr[i] = GLBuf+opt.usb_size*1024*i;
    //memset(buftr[i],0,sizeof(unsigned char)*opt.usb_size*1024);
    }
  */

  //cout << "---Init_Transfer 3---" << endl;
  ntrans=0;
  for (int i=0;i<MAXTRANS7;i++) {
    transfer[i] = libusb_alloc_transfer(0);
    //transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;
		
    int* ntr = new int;
    (*ntr) = i;

    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], tr_size, cback, ntr, 0);
    //libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], opt.usb_size*1024, cback, ntr, 0);

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

  /*
    if (opt.usb_size>1024) {
    MAXTRANS2=MAXTRANS-1;
    }
    else {
    MAXTRANS2=MAXTRANS;
    }
  */

  //cout << "reset_usb: " << endl;
  //Command32(7,0,0,0); //reset usb command
  //gSystem->Sleep(250);

  //cout << "submit7:" << endl;
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
      if (buftr[i]) {
	buftr[i]=NULL;
      }
    }
    return 2;
  }
  gSystem->Sleep(250);

  Cancel_all(MAXTRANS7);
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
  case 11:
    len_out=6;
    len_in=1;
    break;
  default:
    cout << "Wrong CRS32 command: " << (int) cmd << endl;
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

  // printf("buf_out:");
  // for (int i=0;i<6;i++) {
  // 	printf(" %d",buf_out[i]);
  // }
  // printf("\n");
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

void CRS::Check33(byte cmd, byte ch, int &a1, int &a2, int min, int max) {
  int len = a2-a1+1;
  if (len>max) {
    len=max;
    a2=a1+max-1;
  }
  if (len<min) {
    len=min;
    a2=a1+min-1;
  }
  Command32(2,ch,cmd,a1); //offset
  Command32(2,ch,cmd+1,len); //length
  //cout << "Check33: " << (int) ch << " " << (int) cmd << " " << a1
  //<< " " << a2 << " " << len << endl;
}

void CRS::AllParameters41()
{
  UInt_t mask;
  //cout << "AllParameters41(): " << endl;

  //enable start channel by default
  Command32(2,255,11,1); //start ch enabled

  for (byte chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.enabl[chan]=false;
    Command32(2,chan,11,(int) cpar.enabl[chan]); //enabled

    if (cpar.enabl[chan]) {
      Command32(2,chan,0,(int)cpar.acdc[chan]);
      Command32(2,chan,1,(int)cpar.inv[chan]);
      Command32(2,chan,2,(int)cpar.smooth[chan]);
      Command32(2,chan,3,(int)cpar.deadTime[chan]);
      Command32(2,chan,4,(int)-cpar.preWr[chan]);
      Command32(2,chan,5,(int)cpar.durWr[chan]);
      Command32(2,chan,6,(int)cpar.kderiv[chan]);
      Command32(2,chan,7,(int)cpar.threshold[chan]);
      Command32(2,chan,8,(int)cpar.adcGain[chan]);
      Command32(2,chan,9,(int)cpar.delay[chan]); //delay
      //Command32(2,chan,10,0); //test signal is off
      Command32(2,chan,12,(int) cpar.trg[chan]); //trigger type

      Check33(13,chan,opt.Base1[chan],opt.Base2[chan],1,4095);
      Check33(15,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(17,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(19,chan,opt.T1[chan],opt.T2[chan],1,4095);
      Check33(21,chan,opt.W1[chan],opt.W2[chan],1,4095);

      //cpar.Mask[chan]=0xFF;
      mask=0xC3; //11000011 - write tst,count40,count48,overflow
      if (opt.dsp[chan]) { //add 110000
	mask|=0x30; // write DSP data
      }

      if (cpar.pls[chan]) { //add 1100
	mask|=0xC; // write pulse
      }

      // mask=0xFF;
      // cout << "mask: " << (int) chan << " " << hex << mask << dec << endl;
      Command32(2,chan,23,mask); //bitmask for discriminator
      Command32(2,chan,24,mask); //bitmask for START
      //UInt_t mask = 0xFF;
      //Command32(2,chan,23,(UInt_t) mask); //bitmask
      mask=cpar.St_trig ? 2 : 1;
      // mask=0xFF;
      // cout << "mch: " << (int) chan << " " << mask << endl;
      Command32(2,chan,25,mask); //bitmask for START
      Command32(2,chan,30,200); // максимальное расстояние для дискриминатора типа 3: L

    }

  } //for

  // Start dead time DT
  if (cpar.DTW<=0) cpar.DTW=1;
  byte type = cpar.DTW>>24;
  Command32(11,0,type,(UInt_t) cpar.DTW);

  // Sampling rate
  Command32(11,1,0,cpar.Smpl);
  // FIR Filter
  //Command32(11,2,0,cpar.FIR);
	
  //Start source
  mask=cpar.St_Per ? 1 : 0;
  // mask=0xFF;
  // cout << "start source: " << mask << endl;
  Command32(11,2,0,mask);

  //Start imitator period
  mask=cpar.St_Per;
  if (mask) mask--;
  // cout << "start period: " << mask << endl;
  Command32(11,3,0,mask);

}

void CRS::AllParameters34()
{
  //cout << "AllParameters34(): " << endl;

  //enable start channel by default
  Command32(2,255,11,1); //enabled

  for (byte chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.enabl[chan]=false;
    Command32(2,chan,11,(int) cpar.enabl[chan]); //enabled

    if (cpar.enabl[chan]) {
      Command32(2,chan,0,(int)cpar.acdc[chan]);
      Command32(2,chan,1,(int)cpar.inv[chan]);
      Command32(2,chan,2,(int)cpar.smooth[chan]);
      Command32(2,chan,3,(int)cpar.deadTime[chan]);
      Command32(2,chan,4,(int)-cpar.preWr[chan]);
      Command32(2,chan,5,(int)cpar.durWr[chan]);
      Command32(2,chan,6,(int)cpar.kderiv[chan]);
      Command32(2,chan,7,(int)cpar.threshold[chan]);
      Command32(2,chan,8,(int)cpar.adcGain[chan]);
      // new commands
      //Command32(2,chan,9,(int)cpar.delay[chan]); //delay
      Command32(2,chan,9,0); //delay
      Command32(2,chan,10,0); //test signal is off
      Command32(2,chan,12,(int) cpar.trg[chan]); //trigger type

      Check33(13,chan,opt.Base1[chan],opt.Base2[chan],1,4095);
      Check33(15,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(17,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(19,chan,opt.T1[chan],opt.T2[chan],1,4095);
      Check33(21,chan,opt.W1[chan],opt.W2[chan],1,4095);

      //cpar.Mask[chan]=0xFF;
      UInt_t mask=0xC3; //11000011 - write tst,count40,count48,overflow
      if (opt.dsp[chan]) {//add 110000
	mask|=0x30; // write DSP data
      }
      if (cpar.pls[chan]) {//add 1100
	mask|=0xC; // write pulse
      }
      Command32(2,chan,23,mask); //bitmask
      //UInt_t mask = 0xFF;
      //Command32(2,chan,23,(UInt_t) mask); //bitmask
    }

  } //for

  // Start deat time DT
  if (cpar.DTW==0) cpar.DTW=1;
  byte type = cpar.DTW>>24;
  Command32(11,0,type,(UInt_t) cpar.DTW); //bitmask

}

void CRS::AllParameters33()
{
  cout << "AllParameters33(): " << endl;

  //enable start channel by default
  Command32(2,255,11,1); //enabled

  for (byte chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.enabl[chan]=false;
    Command32(2,chan,11,(int) cpar.enabl[chan]); //enabled

    if (cpar.enabl[chan]) {
      Command32(2,chan,0,(int)cpar.acdc[chan]);
      Command32(2,chan,1,(int)cpar.inv[chan]);
      Command32(2,chan,2,(int)cpar.smooth[chan]);
      Command32(2,chan,3,(int)cpar.deadTime[chan]);
      Command32(2,chan,4,(int)-cpar.preWr[chan]);
      Command32(2,chan,5,(int)cpar.durWr[chan]);
      Command32(2,chan,6,(int)cpar.kderiv[chan]);
      Command32(2,chan,7,(int)cpar.threshold[chan]);
      Command32(2,chan,8,(int)cpar.adcGain[chan]);
      // new commands
      Command32(2,chan,9,0); //delay
      //Command32(2,chan,10,0); //test signal
      Command32(2,chan,12,(int) cpar.trg[chan]); //trigger type

      Check33(13,chan,opt.Base1[chan],opt.Base2[chan],1,4095);
      Check33(15,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(17,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(19,chan,opt.T1[chan],opt.T2[chan],1,4095);
      Check33(21,chan,opt.W1[chan],opt.W2[chan],1,4095);

    }

  }

}

void CRS::AllParameters32()
{
  //cout << "AllParameters32(): " << endl;

  for (byte chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.enabl[chan]=false;
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
  //cout << "AllParameters2(): " << endl;
  for (byte chan = 0; chan < chan_in_module; chan++) {
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
  // int r;
  //int transferred = 0;
  //int len=2; //input/output length must be 2, not 1
  if (f_read) {
    gzclose(f_read);
    f_read=0;
  }
	
  if (!b_acq) { //start
		//if (b_usbbuf) {
    crs->Free_Transfer();
    gSystem->Sleep(50);

    InitBuf();
    crs->Init_Transfer();
    //}
    //b_usbbuf=false;

    DoReset();
    //juststarted=true; already set in doreset

    parpar->Update();
    daqpar->Update();
    anapar->Update();
    pikpar->Update();

    EvtFrm->Clear();
    EvtFrm->Pevents = &EvtFrm->Tevents;

    //if (module==32) {
    //Command32(7,0,0,0); //reset usb command
    //}
		
    // r=
    cyusb_reset_device(cy_handle);
    // cout << "cyusb_reset: " << r << endl;

    Submit_all(ntrans);
    opt.Period=5; //5ns for CRS module

    if (SetPar()) {
      return 3;
    }

    //buf_out[0]=3;
    b_acq=true;
    b_stop=false;
    b_fstart=false;
    //bstart=true;
    //inputbytes=0;
    //rawbytes=0;
    //b_pevent=true;
		
    //npulses=0;
    //nevents=0;
    //nbuffers=0;

    //Nsamp=0;
    //nsmp=0;

    opt.F_start = gSystem->Now();
    //T_start = opt.F_start;

    if (opt.raw_write) {
      Reset_Raw();
    }   
    if (opt.dec_write) {
      Reset_Dec(79);
    }   

    //cout << "Acquisition started" << endl;
    //gettimeofday(&t_start,NULL);

    TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
    cv->SetEditable(false);

    //InitBuf();


    ProcessCrs();
    //ProcessCrs_old();


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

    // cout << "Acquisition stopped2" << endl;
    Cancel_all(ntrans);
    b_stop=true;
    b_run=2;
    //cout << "Acquisition stopped3" << endl;
    //Select_Event();
    //EvtFrm->Levents = &Levents;
  }

  return 0;

}

void CRS::ProcessCrs() {
  b_run=1;
  Ana_start();

  //decode_thread_run=1;
  //tt1[3].Set();
  if (crs->module>=32 && crs->module<=49 /*34*/) {
    Command32(8,0,0,0); //сброс сч./буф.
    Command32(9,0,0,0); //сброс времени
  }
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

  // cout << "stopped:" << endl;
  EndAna(1);

}

/*
  void CRS::ProcessCrs_old() {
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
  }
*/

#endif //CYUSB

void CRS::DoExit()
{
  cout << "CRS::DoExit" << endl;

  event_thread_run=0;
#ifdef CYUSB
  if (Fmode==1) {
    cyusb_close();
    if (trd_crs) {
      trd_crs->Delete();
    }
  }
#endif

}

void CRS::DoReset() {

  // cout << "DoReset1: " << b_stop << endl;

  if (!b_stop) return;

  b_fstart=false;
  opt.T_acq=0;

  if (module==1) {
    Tstart64=-1;
  }
  else {
    Tstart64=0;
  }
  //Tstart64=0;

  // cout << "Dorese1a: " << Fmode << " " << Tstart64 << endl;

  Tstart0=0;
  //Offset64=0;
  //T_last_good=0;
  Pstamp64=P64_0;

  //cout << "EvtFrm: " << EvtFrm << endl;
  if (EvtFrm) {
    EvtFrm->DoReset();
  }
  // cout << "Doreset1b: " << Fmode << " " << Tstart64 << endl;

  Levents.clear();
  //L4=1;
  N4=0;
  SLP=0;
  // вставляем dum евент, чтобы Levents не был пустой  // не делаем, это плохо
  //m_event=Levents.insert(Levents.end(),EventClass());
  m_event=Levents.end();
  //m_end=Levents.end();
  //m_event=Levents.begin();

  nevents=0;
  nevents2=0;

  //Vpulses.clear();
  Bufevents.clear();
  //plist.clear();

  //for (int i=0;i<MAXTRANS;i++)
  //Vpulses[i].clear();

  //nvp=0;

  //vv = Vpulses+nvp; //- vector of pulses from current
  //vv2 = Vpulses+1-nvp; //- vector of pulses from previous buffer
  //create first pulse, which is always bad: Chan=254
  //this pulse will be ignored in Event_insert_pulse

  //pulse_vect::iterator ipls = Vpulses->insert(Vpulses->end(),PulseClass());
  //ipls->Chan=254;
  //ipls->ptype|=P_NOSTART;

  //ipk=&dummy_peak; //peak points to the dummy_peak
  //QX=0;QY=0;RX=1;RY=1;

  // if (module!=1) {
  //   //cout << "Pre!!!" << endl;
  //   memcpy(&Pre,&cpar.preWr,sizeof(Pre));
  // }

  npulses=0;
  nbuffers=0;
  memset(npulses2,0,sizeof(npulses2));
  memset(npulses3,0,sizeof(npulses3));
  memset(npulses_bad,0,sizeof(npulses_bad));
  memset(errors,0,sizeof(errors));

#ifdef TIMES
  memset(ttm,0,sizeof(ttm));
#endif

  //npulses=0;
  //npulses_buf=0;

  inputbytes=0;
  rawbytes=0;
  decbytes=0;

  //MAX_LAG=opt.event_buf/2;

  //idx=0;
  //idnext=0;
  //lastfl=1;

  //idec=0;

  //printhlist(5);
  //cout << "f_read: " << f_read << endl;
  if (f_read)
    DoFopen(NULL,0);
  juststarted=true;

  //printhlist(6);
  // parpar->Update();
  // cout << "DoReset2a: " << endl;
  if (daqpar) {
    daqpar->UpdateStatus(1);
    //daqpar->ResetStatus();
  }
  //if (HiFrm)
  // cout << "DoReset2: " << endl;

  //rPeaks73.clear();
  //rPeaks74.clear();
  //CloseTree();

  //Set_Trigger();  

}

void CRS::DoFopen(char* oname, int popt) {
  //popt: 1 - read opt from file; 0 - don't read opt from file
  int tp=0; //1 - adcm raw; 0 - crs2/32/dec; 2 - Ortec Lis
  //int bsize;
  //int boffset;

  if (oname)
    strcpy(Fname,oname);

  cout << "DoFopen: " << Fname << endl;
  // if (TString(Fname).EqualTo(" ",TString::kIgnoreCase)) {
  //   return;
  // }

  string dir, name, ext;
  SplitFilename(string(Fname),dir,name,ext);

  if (TString(ext).EqualTo(".dat",TString::kIgnoreCase)) {
    tp=1;
  }
  else if (TString(ext).EqualTo(".lis",TString::kIgnoreCase)) {
    tp=2;
  }
  else if (TString(ext).EqualTo(".gz",TString::kIgnoreCase)) {
    tp=0;
  }
  else if (TString(ext).EqualTo(".raw",TString::kIgnoreCase)) {
    tp=0;
  }
  else if (TString(ext).EqualTo(".dec",TString::kIgnoreCase)) {
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
  //cout << "f_read: " << f_read << endl;
  if (!f_read) {
    //Fmode=0;
    cout << "Can't open file: " << Fname << endl;
    f_read=0;
    return;
  }

  if (tp==0) { //crs32 or crs2 or dec
    //printhlist(7);
    if (ReadParGz(f_read,Fname,1,1,popt)) {
      gzclose(f_read);
      f_read=0;
      return;
    }
    opt.Period=5;
    // cout << "false_open: " << endl;
    // opt.raw_write=false;
    // opt.dec_write=false;
    // opt.root_write=false;
  }
  else if (tp==1) { //adcm raw
    cout << "ADCM RAW File: " << Fname << endl;
    module=1;
    cpar.InitPar(0);
    opt.Period=10;
  }
  else if (tp==2) { //Ortec Lis
    cout << "Ortec Lis File: " << Fname << endl;
    module=3;
    cpar.InitPar(0);
    opt.Period=200;

    char header[256];
    gzread(f_read,header,256);

    Int_t *fmt = (Int_t*) header;
    Int_t *style = (Int_t*) &header[4];
    Double_t *start_t = (Double_t*) &header[8];
    char txt[80];
    strncpy(txt,header+121,80);

    printf("hdr: %d %d %f %s\n",*fmt,*style,*start_t,txt);
  }

  //boffset=1024*1024;
  //bsize=opt.rbuf_size*1024+boffset;
  if (module==1) {
    Tstart64=-1;
  }
  else {
    Tstart64=0;
  }
  //Tstart64=0;

  //list_min=opt.ev_max-opt.ev_min;

  Fmode=2;

  //opt.raw_write=false;

  //cout << "smooth 0-39: " << cpar.smooth[39] << " " << opt.smooth[39] << endl;

  //ibuf=0;

  //cout << "Dofopen1: " << endl;
  InitBuf();
  //cout << "Dofopen2: " << endl;
  //nvp=0;

  if (tp==1) { //adcm raw - determine durwr
    // cout << "durwr1: " << nvp << " " << vv2->size() << endl;
    // DoBuf();
    // cout << "durwr2: " << nvp << " " << vv2->size() << endl;
  }

  if (myM) {
    myM->SetTitle(Fname);
    daqpar->AllEnabled(false);
    //myM->fTab->SetEnabled(1,false);
    //cout << "fTab: " << myM->fTab << endl;
  }
}

int CRS::ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2) {
  //m1 - read module (1/0); 1 - read from raw/dec file; 0 - read from par file
  //p1 - read cpar (1/0); p2 - read opt (1/0)

  // 2 bytes: fmt - формат par файла
  //        if (fmt>128) {
  //           fmt - определяет номер формата записи par файла
  //           2 bytes - module (Short_t)
  //           4 bytes - sz (Int_t)
  //           sz bytes - buf
  //        }
  //        else {
  //           fmt=module (Short_t)
  //           2 bytes - sz (UShort_t)
  //           sz bytes - buf
  //        }

  UShort_t fmt;
  UShort_t mod;
  Int_t sz;

  char vname[20];
  char buf[500000];

  //prtime("ReadParGz1");

  gzread(ff,&fmt,sizeof(Short_t));
  if (fmt>128) {
    gzread(ff,&mod,sizeof(Short_t));
    gzread(ff,&sz,sizeof(Int_t));
  }
  else {
    mod=fmt;
    gzread(ff,&sz,sizeof(UShort_t));
  }

  cout << "ReadParGz: " << pname << " " << sz << endl;

  //char* buf = new char[sz];
  gzread(ff,buf,sz);





  char* buf1=buf;
  // while (buf1) {
  //   buf1 = (char*) memmem(buf1+1,sz-1,"class",sizeof("class"));
  //   if (buf1) {
  //     int b1 = buf1-buf;
  //     cout << "b1: " << b1 << " " << buf1 << " " << buf1+8 << endl;
  //   }
  // }



  cout << "buf: " << (void*) buf << " " << buf1-buf << " " << sz << endl;


  /*
  UShort_t len=0;
  while (buf1<buf+sz) {
    memcpy(&len,buf1,sizeof(len));
    buf1+=sizeof(len);
    cout << "len: " << len << " " << buf1-buf << " " << sz << " " << buf1 << endl;
    buf1+=len;
  }
  // for (int i=0;i<sz;i++) {
  //   cout << buf[i];
  // }
  // cout << endl;

  exit(1);
  */

  int ret;

  if (p1) {
    ret=BufToClass("Coptions",vname,(char*) &cpar,buf1,buf+sz);
    if (!ret)
      cout << "ReadPar: class Coptions not found" << endl;
    //cout << "buf: " << (void*) buf << " " << buf1 << " " << buf1-buf << endl;
  } //if p1
  if (p2) {
    ret=BufToClass("Toptions",vname,(char*) &opt,buf1,buf+sz);
    if (!ret)
      cout << "ReadPar: class Toptions not found" << endl;
    //cout << "buf: " << (void*) buf << " " << buf1 << " " << buf1-buf << endl;

    TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
    do {
      Hdef hd;
      ret=BufToClass("Hdef",vname,(char*) &hd,buf1,buf+sz);
      //cout << "hh0: " << vname << " " << strlen(vname) << " " << ret << endl;
      TDataMember* dm = (TDataMember*) lst->FindObject(vname);
      if (dm) {
	Hdef* hh = (Hdef*) ((char*) &opt+dm->GetOffset());
	//cout << "off: " << &opt << " " << (char*)&opt.h_time-(char*)&opt << " " << dm->GetOffset() << endl;

	  
	//cout << "hh0a: " << vname << " " << ret << " " << hh << " " << &opt.h_time << endl;
      	//cout << "hh1: " << vname << endl;

	// for (int i=0;i<MAX_CH+NGRP;i++) {
	//   cout << i << " " << hh->c[i] << endl;
	// }

	if (ret)
	  (*hh)=Hdef(hd);
      	//cout << "hh2: " << vname << endl;
      }
    } while (ret==1);

    /*
    TIter nextd(lst);
    TDataMember *dm;
    char* popt = (char*)&opt;
    while ((dm = (TDataMember *) nextd())) {
      if (dm->GetDataType()==0 && TString(dm->GetName()).Contains("h_")) {
	//cout << "member: " << dm->GetName() << " " << dm->GetDataType() << " " << dm->GetOffset() << " " << (void*)popt+dm->GetOffset() << " " << &opt << " " << &(opt.h_time) << endl;
	;//YK BufToClass("Hdef",dm->GetName(),popt+dm->GetOffset(),buf1,buf+sz);
      }
    }
    */
  } //if p2


  for (int i=0;i<MAX_CHTP;i++) {
    if (opt.chtype[i]>=MAX_TP+1) {
      opt.chtype[i]=MAX_TP+1;// other; was =1;
    }
  }
  Make_prof_ch();

  if (m1) {
    //T_start = opt.F_start;
    if (mod==2 || mod==22) {
      module=22;
      cout << "CRS2 File: " << Fname << " " << module << endl;
    }
    else if (mod>=32) {
      module=mod;
      cout << "CRS32 File: " << Fname << " " << module << endl;
    }
    else {
      Fmode=0;
      cout << "Unknown file type: " << Fname << " " << mod << endl;
      //delete[] buf;
      return 1;
    }
  }

  //Set_Trigger();

  if (p2) {
    //cout << "false_gz: " << endl;
    opt.raw_write=false;
    opt.dec_write=false;
    opt.root_write=false;
  }

  if (HiFrm)
    HiFrm->HiReset();

  //delete[] buf;
  //prtime("ReadParGz2");
  return 0;
}

void CRS::SaveParGz(gzFile &ff, Short_t mod) {
  //see ReadParGz for format of the header

  //cout << "savepargz: " << endl;
  const int ZZ=500000;
  Short_t fmt = 129;
  char buf[ZZ];
  //char* buf = new char[ZZ];
  Int_t sz=0;

  strncpy(opt.gitver,GITVERSION,sizeof(opt.gitver)-1);
  opt.maxch=MAX_CH;
  opt.maxtp=MAX_TP;

  memset(buf,0,ZZ);
  sz+=ClassToBuf("Coptions","cpar",(char*) &cpar, buf+sz);
  sz+=ClassToBuf("Toptions","opt",(char*) &opt, buf+sz);

  TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
  TIter nextd(lst);
  TDataMember *dm;
  char* popt = (char*)&opt;
  while ((dm = (TDataMember *) nextd())) {
    if (dm->GetDataType()==0 && TString(dm->GetName()).Contains("h_")) {
      //cout << "member: " << dm->GetName() << " " << dm->GetDataType() << endl;
      sz+=ClassToBuf("Hdef",dm->GetName(),popt+dm->GetOffset(),buf+sz);
    }
  }

  //sz/=8;
  //sz=(sz+1)*8;
  sz = (sz/8+1)*8;

  gzwrite(ff,&fmt,sizeof(Short_t));
  gzwrite(ff,&mod,sizeof(Short_t));
  gzwrite(ff,&sz,sizeof(Int_t));
  gzwrite(ff,buf,sz);

  memset(opt.gitver,0,sizeof(opt.gitver));
  opt.maxch=0;
  opt.maxtp=0;

  //cout << "savepargz2: " << cpar.Smpl << endl;
  //delete[] buf;
  //cout << "SavePar_gz: " << sz << endl;

}

void CRS::DoProf(Int_t nn, Int_t *aa, Int_t off) {
  if (aa[nn]>=0 && aa[nn]<MAX_CH) {
    if (prof_ch[aa[nn]]<0)
      prof_ch[aa[nn]]=nn+off;
    else
      cout << "Profilometer: chan " << aa[nn] << " is already used" << endl; 
  }
}

void CRS::Make_prof_ch() {
  for (int i=0;i<MAX_CH;i++) {
    prof_ch[i]=-1;
    //cout << "prof_ch: " << i << " " << prof_ch[i] << endl;
  }

  for (int i=0;i<5;i++) {
    DoProf(i,opt.Prof64,PROF_64);
  }
  for (int i=0;i<8;i++) {
    DoProf(i,opt.Prof_x,PROF_X);
    DoProf(i,opt.Prof_y,PROF_Y);
  }
  for (int i=0;i<16;i++) {
    DoProf(i,opt.Ing_x,ING_X);
    DoProf(i,opt.Ing_y,ING_Y);
  }

  // for (int i=0;i<MAX_CH;i++) {
  //   cout << "Prof: " << i << " " << prof_ch[i] << endl;
  // }
}

void CRS::AnaBuf(int loc_ibuf) {

  UInt_t ibuf2 = gl_ibuf+1;
  //Если последний буфер в "кольце буферов" -> копируем "хвост" буфера в начало
  if (ibuf2==gl_Nbuf) { //last buffer in ring, jump to zero
    int end0 = b_end[gl_ibuf];
    FindLast(gl_ibuf, loc_ibuf, 8);
    int sz = end0 - b_end[gl_ibuf];
    memcpy(GLBuf-sz,GLBuf+b_end[gl_ibuf],sz);
    b_start[0]=-sz;
    b_fill[0]=0;
  }
  else { //normal buffer
    b_start[ibuf2]=b_end[gl_ibuf];
    b_fill[ibuf2]=b_end[gl_ibuf];
  }

#ifdef TIMES
  tt2[0].Set();
  dif = tt2[0].GetSec()-tt1[0].GetSec()+
    (tt2[0].GetNanoSec()-tt1[0].GetNanoSec())*1e-9;
  ttm[0]+=dif;
#endif


  if (opt.decode) {
    //buf_len[gl_ibuf]=BufLength;
    if (opt.nthreads>1) {
      Decode_any_MT(gl_iread, gl_ibuf, loc_ibuf);
    }
    else {
      Decode_any(gl_ibuf);
    }
    //cout << "Sleep..." << endl;
    //gSystem->Sleep(10);
  }
  gl_iread++;
  gl_ibuf=gl_iread%gl_Nbuf;

  if (N4>3) {
    SLP+=10;
    ++errors[7]; //slow analysis
  }
  else {
    if (SLP>0)
      SLP-=10;
  }

  if (SLP>0)
    gSystem->Sleep(SLP);

  //cout << "L4: " << L4 << " " << N4 << " " << SLP << endl;


  /*
  // if (!b_mem && CheckMem(70)) {
  if (false) {
  // if (!b_mem && true) {
  cout << "Memory is too low. Exitting... " << pinfo.fMemResident*1e-3 << " " << minfo.fMemTotal << endl;
  b_mem=true;

  // gSystem->Sleep(300);
  // EndAna(1);
  // opt.Tstop=0.001;
  // gSystem->Sleep(5000);
  // Command2(4,0,0,0);

  // myM->fStart->Emit("Clicked()");
  // myM->DoStartStop();

  cout << "Terminate..." << endl;
  gApplication->Terminate(0);
  delete myM;
  // myM->CloseWindow();
	
  // EndAna(1);
  // DoExit();
  //exit(-1);
  // myM->DoExit();
  }
  */

  if (batch && !silent) {
    cout << "Buf: " << nbuffers << "  Dec. MB: "
	 << inputbytes/MB << "  T(s): " << int(opt.T_acq*10)*0.1
	 << " %mem: " << CheckMem() << " slp: " << SLP << endl;
  }
  //if (!b_stop) {
  //opt.T_acq = (Levents.back().T - Tstart64)*1e-9*opt.Period;
  //}

}

int CRS::DoBuf() {

  // cout << "gzread0: " << Fmode << " " << nbuffers << " " << opt.rbuf_size*1024 << " " << gl_ibuf << " " << dec_iread[gl_ibuf] << endl;
#ifdef TIMES
  tt1[0].Set();
#endif

  while (dec_iread[gl_ibuf])
    gSystem->Sleep(1);

  // cout << "gzread1: " << Fmode << " " << nbuffers << " " << opt.rbuf_size*1024 << " " << gl_ibuf << " " << dec_iread[gl_ibuf] << endl;

  int length=gzread(f_read,GLBuf+b_fill[gl_ibuf],opt.rbuf_size*1024);
  b_end[gl_ibuf]=b_fill[gl_ibuf]+length;

  if (opt.raw_write && !opt.raw_flag) {
    crs->f_raw = gzopen(crs->rawname.c_str(),crs->raw_opt);
    if (crs->f_raw) {
      int res=gzwrite(crs->f_raw,GLBuf+b_fill[gl_ibuf],opt.rbuf_size*1024);
      gzclose(crs->f_raw);
      crs->rawbytes+=res;
    }
    else {
      cout << "Can't open file: " << crs->rawname.c_str() << endl;
    }
  }

  // cout << "gzread: " << Fmode << " " << module << " "
  //      << gl_iread << " " << gl_ibuf << " " << gl_Nbuf << " " << length
  //      << " " << b_fill[gl_ibuf] << endl;

  if (length>0) {
    AnaBuf(1); // YK (1 - loc_ibuf, fake number)
    nbuffers++;
    inputbytes+=length;
    return nbuffers;
  }
  else {
    return 0;
  }

}

/*
  void CRS::FAnalyze(bool nobatch) {

  if (gzeof(f_read)) {
  cout << "Enf of file: " << endl;
  return;
  }
  TCanvas *cv=0;
  //cout << "FAnalyze: " << gztell(f_read) << endl;
  if (juststarted && opt.dec_write) {
  Reset_Dec();
  }
  juststarted=false;

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
*/

void CRS::InitBuf() {
  gl_iread=0;
  gl_ivect=0;
  gl_ibuf=0;

  memset(b_fill,0,sizeof(b_fill));
  memset(b_start,0,sizeof(b_start));
  memset(b_end,0,sizeof(b_end));

  if (GLBuf2) {
    delete[] GLBuf2;
  }

  if (opt.nthreads>1)
    gl_Nbuf=opt.nthreads;
  else
    gl_Nbuf=8;

  if (Fmode==1) { //module is connected
    if (opt.usb_size<=1024) {
      tr_size=opt.usb_size*1024;
      MAXTRANS2=MAXTRANS7;
    }
    else if (opt.usb_size<=2048) {
      tr_size=opt.usb_size*1024;
      MAXTRANS2=MAXTRANS7;
    }
    else {
      tr_size=2048*1024;
      MAXTRANS2=MAXTRANS7;
    }

    gl_sz = opt.usb_size;
    gl_sz *= 1024*MAXTRANS2*gl_Nbuf;
    cout << "gl_sz: " << opt.usb_size << " " << gl_sz/MB << " MB" << endl;
    GLBuf2 = new UChar_t[gl_sz+gl_off];
    memset(GLBuf2,0,gl_sz+gl_off);
    GLBuf=GLBuf2+gl_off;

    for (int i=0;i<ntrans;i++) {
      buftr[i] = GLBuf+tr_size*i;
#ifdef CYUSB
      if (transfer[i])
	transfer[i]->buffer = buftr[i];
#endif
    }
  } // if Fmode==1
  else { //file analysis
    gl_sz = opt.rbuf_size;
    gl_sz *= 1024*gl_Nbuf;
    GLBuf2 = new UChar_t[gl_sz+gl_off];
    memset(GLBuf2,0,gl_sz+gl_off);
    GLBuf=GLBuf2+gl_off;

    for (UInt_t i=0;i<gl_Nbuf;i++) {
      buftr[i] = GLBuf+opt.rbuf_size*1024*i;
    }
  }

  cout << "GLBuf size: " << (gl_sz+gl_off)/1024 << " kB" << endl;
  //cout << "GLBuf size2: " << gl_sz << " " << gl_off << endl;

}

void CRS::StopThreads(int all) {

  //cout << "deleting threads... ";
  decode_thread_run=0;    
  for (UInt_t i=0;i<gl_Nbuf;i++) {
    dec_iread[i]=1; //=1;
    // dec_cond[i].Signal();
    trd_dec[i]->Join();
    trd_dec[i]->Delete();
    trd_dec[i]=0;
    //dec_finished[i]=1;
  }

  gSystem->Sleep(50);
  mkev_thread_run=0;    
		
  trd_mkev->Join();
  trd_mkev->Delete();
  trd_mkev=0;

  gSystem->Sleep(50);
  ana_thread_run=0;    
  ana_all=all;
  //ana_check=1;
  //ana_cond.Signal();
  trd_ana->Join();
  trd_ana->Delete();
  trd_ana=0;
  //cout << "done" << endl;
}

void CRS::EndAna(int all) {
  //all=1 -> finish analysis of all buffers
  //all=0 -> just stop, buffers remain not analyzed,
  //         can continue from this point on
  if (opt.nthreads>1) {
    gSystem->Sleep(50);
    while (!Bufevents.empty()) {
      gSystem->Sleep(10);
    }

    StopThreads(all);
  }
  else {
    if (all) {
      ana_all=all;
      Ana2(1);
    }
  }
  // cout << "EndAna: " << endl;
  // for (int i=0;i<opt.Nchan;i++) {
  //   cout << "Counts: " << npulses2[i] << " " << npulses3[i] << endl;
  // }

  //new (06.02.2020)
  if (opt.dec_write) {
    crs->Flush_Dec();
  }
  if (opt.raw_write && opt.raw_flag) {
    crs->Flush_Raw();
  }

}

void CRS::FAnalyze2(bool nobatch) {

  if (gzeof(f_read)) {
    cout << "Enf of file: " << endl;
    //Ana2(1);
    Show();
    return;
  }
  TCanvas *cv=0;

  if (juststarted) {
    if (opt.raw_write) {
      Reset_Raw();
    }   
    if (opt.dec_write) {
      Reset_Dec(79);
    }   
  }
  juststarted=false;

  if (nobatch) {
    EvtFrm->Clear();
    EvtFrm->Pevents = &EvtFrm->Tevents;

    cv=EvtFrm->fCanvas->GetCanvas();
    cv->SetEditable(false);
  }
  //T_start = gSystem->Now();

  cout << "FAnalyze: " << gztell(f_read) << endl;
  Ana_start();
  cout << "FAnalyze2: " << gztell(f_read) << endl;
  int res;
  while ((res=crs->DoBuf()) && crs->b_fana) {
    if (nobatch) {
      Show();
      gSystem->ProcessEvents();
    }
    // if (nbuffers%10==0) {
    //   //cout << "nbuf%10: " << nbuffers << endl;
    //   if (batch || opt.root_write) {
    // 	saveroot(crs->rootname.c_str());
    //   }
    // }
  }

  EndAna(1);

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

void CRS::DoNBuf2(int nb) {

  if (gzeof(f_read)) {
    cout << "Enf of file: " << endl;
    return;
  }

  if (juststarted) {
    if (opt.raw_write) {
      Reset_Raw();
    }   
    if (opt.dec_write) {
      Reset_Dec(79);
    }   
  }
  juststarted=false;

  EvtFrm->Clear();
  EvtFrm->Pevents = &EvtFrm->Tevents;

  TCanvas *cv=EvtFrm->fCanvas->GetCanvas();
  cv->SetEditable(false);


  Ana_start();

  int res;
  int i=1;
  while ((res=crs->DoBuf()) && crs->b_fana && i<nb) {
    //Ana2(0);
    Show();
    gSystem->ProcessEvents();
    ++i;
  }

  EndAna(0);

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

  if (CheckMem()>=700) {
    cout << "Memory is too low. Exitting... " << pinfo.fMemResident*1e-3 << " " << minfo.fMemTotal << endl;

    DoStartStop();
    // gSystem->Sleep(500);

    // cout << "Terminate..." << endl;
    // exit(-1);
#ifdef CYUSB
    if (trd_crs) {
      trd_crs->Delete();
    }
#endif
	
    StopThreads(0);

    gApplication->Terminate(0);
    delete myM;
  }

  tm2=gSystem->Now();
  // cout << "Levents7: " << Levents.size() << " " << tm2 << endl;
  if (tm2-tm1>opt.tsleep || force) {

    // cout << "\033[35m";
    // cout << "Show: " << tm2-tm1 << " " << force << endl;
    // cout << "\033[0m";

    tm1=tm2;
    //cout << "show... " << info.fMemTotal << " " << info.fMemFree << " " << info.fMemUsed << " " << Levents.size() << " " << &*m_event << " " << m_event->Nevt << endl;

    if (myM) {
      if (myM->fTab->GetCurrent()==EvtFrm->ntab || EvtFrm->fDock->GetUndocked()) {
	EvtFrm->fCanvas->GetCanvas()->SetEditable(true);
	EvtFrm->DrawEvent2();
	EvtFrm->fCanvas->GetCanvas()->SetEditable(false);
      }
      if (myM->fTab->GetCurrent()==HiFrm->ntab || HiFrm->fDock->GetUndocked()) {
	//HiFrm->DrawHist();      
	HiFrm->ReDraw();
	//HiFrm->Update();
      }
      // else {
      // 	TString name = TString(myM->fTab->GetCurrentTab()->GetString());
      // 	if (name.EqualTo("DAQ",TString::kIgnoreCase)) {
      // 	  daqpar->UpdateStatus();
      // 	}
      // }
    }
    daqpar->UpdateStatus();
    myM->UpdateStatus();
    ErrFrm->ErrUpdate();
#ifdef TIMES
    printf("T_acq: %0.2f Read: %0.2f Dec: %0.2f Make: %0.2f Ana: %0.2f Tot: %0.2f\n",opt.T_acq,ttm[0],ttm[1],ttm[2],ttm[3],ttm[4]);
#endif
    // cout << "Times: " << opt.T_acq;
    // for (int i=0;i<5;i++) {
    //   cout << " " << ttm[i];
    // }
    // cout << endl;

  }
  //}

  //cout << "Show end" << endl;
}

void CRS::Decode_switch(UInt_t ibuf) {
  if (ibuf!=gl_Nbuf-1) { // если текущий буфер (ibuf) не последний в кольце
    FindLast(ibuf,9,7);
  }

  switch (module) {
  case 32:
  case 33:
  case 34:
  case 41:
    Decode34(dec_iread[ibuf]-1,ibuf);
    break;
  case 79:
    Decode79(dec_iread[ibuf]-1,ibuf);
    break;
  case 78:
    Decode78(dec_iread[ibuf]-1,ibuf);
    break;
  case 77:
    Decode77(dec_iread[ibuf]-1,ibuf);
    break;
  case 76:
    Decode76(dec_iread[ibuf]-1,ibuf);
    break;
  case 75:
    Decode75(dec_iread[ibuf]-1,ibuf);
    break;
  case 22:
    Decode2(dec_iread[ibuf]-1,ibuf);
    break;
  case 1:
    Decode_adcm(dec_iread[ibuf]-1,ibuf);
    break;
  default:
    break;
  }

  // gSystem->Sleep(100);
  dec_iread[ibuf]=0;
  //--------end
}

void CRS::Decode_any_MT(UInt_t iread, UInt_t ibuf, int loc_ibuf) {
  //-----decode
  // prnt("ss2d2d2ds;",KBLU,"DecMT:",iread, loc_ibuf, gl_ibuf,RST);

  dec_iread[ibuf]=iread+1;
  // dec_cond[ibuf].Signal();

}

void CRS::Decode_any(UInt_t ibuf) {
  // #ifdef TIMES
  //   tt1[1].Set();
  // #endif

  Decode_switch(ibuf);

  // #ifdef TIMES
  //   tt2[1].Set();
  //   dif = tt2[1].GetSec()-tt1[1].GetSec()+
  //     (tt2[1].GetNanoSec()-tt1[1].GetNanoSec())*1e-9;
  //   ttm[1]+=dif;

  //   //-----Make_events
  //   tt1[2].Set();
  // #endif

  Make_Events(Bufevents.begin());

  // #ifdef TIMES
  //   tt2[2].Set();
  //   dif = tt2[2].GetSec()-tt1[2].GetSec()+
  //     (tt2[2].GetNanoSec()-tt1[2].GetNanoSec())*1e-9;
  //   ttm[2]+=dif;

  //   //-----Analyze
  //   tt1[3].Set();
  // #endif

  crs->Ana2(0);
	
  // #ifdef TIMES
  //   tt2[3].Set();
  //   dif = tt2[3].GetSec()-tt1[3].GetSec()+
  //     (tt2[3].GetNanoSec()-tt1[3].GetNanoSec())*1e-9;
  //   ttm[3]+=dif;

  //   dif = tt2[3].GetSec()-tt1[1].GetSec()+
  //     (tt2[3].GetNanoSec()-tt1[1].GetNanoSec())*1e-9;
  //   ttm[4]+=dif;
  // #endif

}

void CRS::FindLast(UInt_t ibuf, int loc_ibuf, int what) {

  //ibuf - current sub-buffer
  UInt_t ibuf2 = (ibuf+1)%gl_Nbuf; //next transfer/buffer
  UInt_t frmt;

  int sss=-1;
  //bool found=false;
  UShort_t lflag;

  switch (module) {
  case 1:
    for (int i=b_end[ibuf]-4;i>=b_start[ibuf];i-=4) {
      //find *frmt==0x2a500100 -> this is the start of a pulse
      frmt = *((UInt_t*) (GLBuf+i));
      //cout << "for: " << i << " " << hex << frmt << dec << endl;
      if (frmt==0x2a500100) {
	if (sss>0) {
	  UInt_t *header =  (UInt_t*) (GLBuf+i);
	  header+=3;
	  lflag=bits(*header,6,6);
	  if (lflag) {
	    b_end[ibuf]=sss;
	    b_start[ibuf2]=sss;
	    return;
	  }
	} //if sss>0
	sss=i;
      } //if frmt
    } //for i
    cout << "1: Error: no last event: maybe USB buffer is too small: " << ibuf << endl;
    break;
  case 22:
    for (int i=b_end[ibuf]-2;i>=b_start[ibuf];i-=2) {
      //find frmt==0 -> this is the start of a pulse
      frmt = (GLBuf[i+1] & 0x70);
      if (frmt==0) {
	b_end[ibuf]=i;
	b_start[ibuf2]=i;
	return;
      }
    }
    cout << "22: Error: no last event: maybe USB buffer is too small: " << ibuf << endl;
    break;
  case 32:
  case 33:
  case 34:
  case 41:
    for (int i=b_end[ibuf]-8;i>=b_start[ibuf];i-=8) {
      //find frmt==0 -> this is the start of a pulse
      frmt = (GLBuf[i+6] & 0xF0);
      if (frmt==0) {
	int len = b_end[ibuf]-i;
	b_end[ibuf]=i;
	b_start[ibuf2]=i;
        // prnt("ss2d2d2d 9d 9d 4d2ds;",KGRN,"FindL:",gl_iread, loc_ibuf, ibuf,
        //      b_start[ibuf], b_end[ibuf], len, what, RST);
        return;
      }
    }
    // prnt("ss2d2d2d 9d 9d 9d 9d2ds;",KMAG,"ErrFL:",gl_iread, loc_ibuf, ibuf,
    // 	b_start[ibuf], b_end[ibuf], b_start[ibuf2], b_end[ibuf]-b_start[ibuf], what, RST);
    break;
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
    for (int i=b_end[ibuf]-8;i>=b_start[ibuf];i-=8) {
      //find frmt==1 -> this is the start of a pulse
      frmt = GLBuf[i+7] & 0x80; //event start bit
      if (frmt) {
	b_end[ibuf]=i;
	b_start[ibuf2]=i;
	return;
      }
    }
    cout << "75: Error: no last event: maybe USB buffer is too small: " << ibuf << endl;
    break;
  default:
    cout << "Wrong module: " << module << endl;
  }

}

/*
  void CRS::Decode32(UChar_t *buffer, int length) {

  //PulseClass *ipls;
  //pulse_vect::iterator ipls;

  ULong64_t* buf8 = (ULong64_t*) buffer;

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

  if ((ch>=opt.Nchan) || (frmt && ch!=ipls->Chan)) {
  cout << "dec32: Bad channel: " << (int) ch
  << " " << (int) ipls->Chan
  << " " << idx8 //<< " " << nvp
  << endl;
  ipls->ptype|=P_BADCH;

  idx8++;
  idx1=idx8*8;
  continue;
  }

  if (frmt==0) {
  ipls->ptype&=~P_NOSTOP; //pulse has stop

  //analyze pulse
  ipls->FindPeaks();
  ipls->PeakAna();

  ipls = vv->insert(vv->end(),PulseClass());
  npulses++;
  ipls->Chan=ch;
  ipls->Tstamp64=data+opt.Delay[ch];// - cpar.preWr[ch];
  }
  else if (frmt==1) {
  ipls->State = buffer[idx1+5];
  ipls->Counter = data & 0xFFFFFFFFFF;
  }
  else if (frmt==2) {

  if (ipls->sData.size()>=cpar.durWr[ipls->Chan]) {
  // cout << "32: ERROR Nsamp: "
  //      << " " << (ipls->Counter & 0x0F)
  //      << " " << ipls->sData.size() << " " << cpar.durWr[ipls->Chan]
  //      << " " << (int) ch << " " << (int) ipls->Chan
  //      << " " << idx8 //<< " " << transfer->actual_length
  //      << endl;
  ipls->ptype|=P_BADSZ;
  }
  //else {
  for (int i=0;i<4;i++) {

  int zzz = data & 0xFFF;
  ipls->sData.push_back((zzz<<20)>>20);
  data>>=12;
  }
  //}
  }
  else if (frmt==3) {

  if (ipls->sData.size()>=cpar.durWr[ipls->Chan]) {
  // cout << "32: ERROR Nsamp: "
  //      << " " << (ipls->Counter & 0x0F)
  //      << " " << ipls->sData.size() << " " << cpar.durWr[ipls->Chan]
  //      << " " << (int) ch << " " << (int) ipls->Chan
  //      << " " << idx8 //<< " " << transfer->actual_length
  //      << endl;
  ipls->ptype|=P_BADSZ;
  }
  //else {
  for (int i=0;i<3;i++) {

  int zzz = data & 0xFFFF;
  ipls->sData.push_back((zzz<<16)>>16);
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

  } //decode32
*/

void CRS::PulseAna(PulseClass &ipls) {
  //cout << "PulseAna: " << endl;
  if (!opt.dsp[ipls.Chan]) {
    if (opt.sS[ipls.Chan]) {
      ipls.Smooth(opt.sS[ipls.Chan]);
    }
    ipls.PeakAna33();
  }
  else {
    if (opt.checkdsp) {
      // if (ipls.Peaks.size()!=1) {
      // 	cout << "size!!!: " << ipls.Peaks.size() << endl;
      // }
      ipls.PeakAna33();
      ipls.CheckDSP();
    }
  }
}

void CRS::Dec_Init(eventlist* &Blist, UChar_t frmt) {
  dec_mut.Lock();
  Bufevents.push_back(eventlist());
  Blist = &Bufevents.back();
  dec_mut.UnLock();

  // Blist->push_back(EventClass());
  // Blist->front().Nevt=iread;
  if (frmt) {
    // prnt("ss ds;",KYEL,"bad buf start: ",nevents,RST);
    ++errors[0]; //bad buf start
  }
}

void CRS::Dec_End(eventlist* &Blist, UInt_t iread) {
  Blist->push_back(EventClass());
  Blist->back().Nevt=iread;
  Blist->back().State=123;
}

void CRS::Decode79(UInt_t iread, UInt_t ibuf) {
  //Decode79 - the same as 78, but different factor for Area

  //ibuf - current sub-buffer

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->State = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      UShort_t* buf2u = (UShort_t*) buf2;
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),PulseClass());
      ipls->Peaks.push_back(peak_type());
      peak_type *pk = &ipls->Peaks.back();
      pk->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
      pk->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples
      pk->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;
      ipls->Chan = buf2[3];
      if (opt.St[ipls->Chan] && pk->Time < evt->T0) {
	evt->T0=pk->Time;
      }
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread);

} //decode79

void CRS::Decode78(UInt_t iread, UInt_t ibuf) {
  //Decode78 - the same as 77, but different factors for Time and Width
  //also, added determination of T0

  //ibuf - current sub-buffer

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->State = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),PulseClass());
      ipls->Peaks.push_back(peak_type());
      peak_type *pk = &ipls->Peaks.back();
      pk->Area = buf2[0];
      pk->Time = buf2[1]*0.01; //in samples
      pk->Width = buf2[2]*0.001;
      ipls->Chan = buf2[3];
      if (opt.St[ipls->Chan] && pk->Time < evt->T0) {
	evt->T0=pk->Time;
      }
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread);

} //decode78

void CRS::Decode77(UInt_t iread, UInt_t ibuf) {
  //Decode77 is the same as 76, but since this version Time is relative to event Tstmp (was relative to pulse Tstamp64)

  //ibuf - current sub-buffer

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->State = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),PulseClass());
      ipls->Peaks.push_back(peak_type());
      peak_type *pk = &ipls->Peaks.back();
      pk->Area = buf2[0];
      pk->Time = buf2[1]*0.1;
      pk->Width = buf2[2];
      ipls->Chan = buf2[3];
      if (opt.St[ipls->Chan] && pk->Time < evt->T0) {
	evt->T0=pk->Time;
      }
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread);

} //decode77

void CRS::Decode76(UInt_t iread, UInt_t ibuf) {

  //ibuf - current sub-buffer

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->State = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),PulseClass());
      ipls->Peaks.push_back(peak_type());
      peak_type *pk = &ipls->Peaks.back();
      pk->Area = buf2[0];
      pk->Time = buf2[1]*0.1;
      ipls->Chan = buf2[3];
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread);

} //decode76

void CRS::Decode75(UInt_t iread, UInt_t ibuf) {

  //cout << "decode75_1: " << endl;
  //ibuf - current sub-buffer

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,frmt);

  //cout << "decode75_2: " << endl;
  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    // else if ((ch>=opt.Nchan) ||
    // 	     (frmt && ch!=ipls.Chan)) {
    //   cout << "dec33: Bad channel: " << (int) ch
    // 	   << " " << (int) ipls.Chan
    // 	   << " " << idx1 << " " << ibuf
    // 	   << endl;
    //   ipls.ptype|=P_BADCH;

    //   //idx8++;
    //   idx1+=8;
    //   continue;
    // }


    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());

      evt->Tstmp = (*buf8) & sixbytes;
      evt->State = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),PulseClass());
      ipls->Peaks.push_back(peak_type());
      peak_type *pk = &ipls->Peaks.back();
      pk->Area = buf2[0];
      pk->Time = buf2[1]*0.1;
      ipls->Chan = buf2[3];
    }

    idx1+=8;
  } //while (idx1<buf_len)

  //cout << "decode75_3: " << endl;
  Dec_End(Blist,iread);

  // cout << "decode75: " << idx1 << " " << Blist->size()
  //      << " " << Bufevents.size() << " " << Bufevents.begin()->size() << endl;

} //decode75

void CRS::Decode33(UInt_t iread, UInt_t ibuf) {
  //ibuf - current sub-buffer

  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  ULong64_t data;
  Short_t sss; // temporary var. to convert signed nbit var. to signed 16bit int
  Int_t iii; // temporary var. to convert signed nbit var. to signed 32bit int
  Long64_t lll; //Long temporary var.
  int n_frm=0; //counter for frmt4 and frmt5
  peak_type *ipk=&dummy_peak; //pointer to the current peak in the current pulse;
  Double_t QX=0,QY=0,RX,RY;

  eventlist *Blist;
  UChar_t frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;

  //cout << "Decode33: " << ibuf << " " << buf_off[ibuf] << " " << vv->size() << " " << length << " " << hex << buf8[idx1/8] << dec << endl;

  while (idx1<b_end[ibuf]) {
    //debug_mess(idx1>16500,"DDDD0: ",idx1,buf1[idx1]);
    frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
    //YKYKYK!!!! do something with cnt - ???
    //int cnt = frmt & 0x0F;
    // frmt = (frmt & 0xF0)>>4;
    data = buf8[idx1/8] & sixbytes;
    unsigned char ch = GLBuf[idx1+7];

    if (ch==255) {
      //start signal
      idx1+=8;
      continue;      
    }
    else if ((ch>=opt.Nchan) ||
	     (frmt && ch!=ipls.Chan)) {
      cout << "dec33: Bad channel: " << (int) ch
	   << " " << (int) ipls.Chan
	   << " " << idx1 << " " << ibuf
	   << endl;
      ipls.ptype|=P_BADCH;

      //idx8++;
      idx1+=8;
      continue;
    }

    if (frmt==0) {
      //ipls->ptype&=~P_NOSTOP; //pulse has stop

      //analyze previous pulse
      if (ipls.ptype==0) {
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
	//cout << "Pana: " << Blist->size() << " " << ipls.Tstamp64 << endl;
      }

      // create new pulse
      ipls=PulseClass();
      npulses++;
      ipls.Chan=ch;
      ipls.Tstamp64=data+(Long64_t)opt.Delay[ch];// - cpar.preWr[ch];
      n_frm=0;
      //cout << "ipls: " << Blist->size() << " " << ipls.Tstamp64 << endl;
    }
    else if (frmt==1) {
      ipls.State = GLBuf[idx1+5];
      ipls.Counter = data & 0xFFFFFFFFFF;
      //cout << "Counter: " << ipls.Counter << " " << (int) ipls.Chan << endl;
      // if (buffer[idx1+5]) {
      // 	cout << "state: " << (int) buffer[idx1+5] << " " << (int ) ipls.State << " " << data << " " << buf8[idx8] << endl;
      // }
    }
    else if (frmt==2) {

      if ((int)ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
	// cout << "32: ERROR Nsamp: "
	//      << " " << (ipls.Counter & 0x0F)
	//      << " " << ipls.sData.size() << " " << cpar.durWr[ipls.Chan]
	//      << " " << (int) ch << " " << (int) ipls.Chan
	//      << " " << idx8 //<< " " << transfer->actual_length
	//      << endl;
	ipls.ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<4;i++) {

	iii = data & 0xFFF;
	ipls.sData.push_back((iii<<20)>>20);

	//ipls.sData[ipls.Nsamp++]=(iii<<20)>>20;
	//printf("sData: %4d %12llx %5d\n",Nsamp-1,data,sData[Nsamp-1]);
	data>>=12;
      }
      //}
    }
    else if (frmt==3) {

      if ((int)ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
	// cout << "33: ERROR Nsamp: "
	//      << " " << (ipls.Counter & 0x0F)
	//      << " " << ipls.sData.size() << " " << cpar.durWr[ipls.Chan]
	//      << " " << (int) ch << " " << (int) ipls.Chan
	//      << " " << idx8 //<< " " << transfer->actual_length
	//      << endl;
	ipls.ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<3;i++) {

	iii = data & 0xFFFF;
	ipls.sData.push_back((iii<<16)>>16);

	//YK
	//float aa = (iii<<16)>>16;
	//aa*=10*ipls.sData.size();
	//ipls.sData.push_back(aa);


	//ipls.sData[ipls.Nsamp++]=(iii<<20)>>20;
	//printf("sData: %4d %12llx %5d\n",Nsamp-1,data,sData[Nsamp-1]);
	data>>=16;
      }
      //}
    }
    else if (frmt==4) {
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [24]; A – [24]
		//area
	  iii = data & 0xFFFFFF;
	  ipk->Area0=((iii<<8)>>8);
	  ipk->Area0/=p_len[ipls.Chan];
	  data>>=24;
	  //bkg
	  iii = data & 0xFFFFFF;
	  ipk->Base=((iii<<8)>>8);
	  ipk->Base/=b_len[ipls.Chan];

	  ipk->Area=ipk->Area0 - ipk->Base;
	  //ipk->Area*=opt.emult[ipls.Chan];
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }

	  break;
	case 1: //H – [12]; QX – [36]
	  lll = data & 0xFFFFFFFFF;
	  QX=((lll<<28)>>28);
	  data>>=36;
	  //height
	  iii = data & 0xFFF;
	  ipk->Height=((iii<<20)>>20);
	  break;
	case 2: //QY – [36]
	  lll = data & 0xFFFFFFFFF;
	  QY=((lll<<28)>>28);
	  break;
	case 3: //RY – [20]; RX – [20]
		//RX
	  iii = data & 0xFFFFF;
	  RX=((iii<<12)>>12);
	  data>>=24;
	  //RY
	  iii = data & 0xFFFFF;
	  RY=((iii<<12)>>12);

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  if (RY!=0)
	    ipk->Width=QY/RY;
	  else
	    ipk->Width=-999;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
    }
    else if (frmt==5) {
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [28]; H – [16]
		//height
	  sss = data & 0xFFFF;
	  ipk->Height=sss;
	  data>>=16;
	  //bkg
	  iii = data & 0xFFFFFFF;
	  ipk->Base=((iii<<4)>>4);
	  ipk->Base/=b_len[ipls.Chan];
	  break;
	case 1: //A – [28]
		//area
	  iii = data & 0xFFFFFFF;
	  ipk->Area0=((iii<<4)>>4);
	  ipk->Area0/=p_len[ipls.Chan];

	  ipk->Area=ipk->Area0 - ipk->Base;
	  //ipk->Area*=opt.emult[ipls.Chan];
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }
	  break;
	case 2: //A – [28]
		//QX
	  lll = data & 0xFFFFFFFFFF;
	  QX=((lll<<24)>>24);
	  break;
	case 3: //QY – [40]
		//QY
	  lll = data & 0xFFFFFFFFFF;
	  QY=((lll<<24)>>24);
	  break;
	case 4: //RY – [24]; RX – [24]
		//RX
	  iii = data & 0xFFFFFF;
	  RX=((iii<<8)>>8);
	  data>>=24;
	  //RY
	  iii = data & 0xFFFFFF;
	  RY=((iii<<8)>>8);

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  if (RY!=0)
	    ipk->Width=QY/RY;
	  else
	    ipk->Width=-999;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
    }
    else {
      cout << "bad frmt: " << frmt << endl;
    }

    //idx8++;
    idx1+=8;
  } //while (idx1<buf_len)

  //add last pulse to the list
  if (ipls.ptype==0) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  Dec_End(Blist,iread);

} //decode33


void CRS::Decode34(UInt_t iread, UInt_t ibuf) {
  //ibuf - current sub-buffer

  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];

  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  ULong64_t data;
  Short_t sss; // temporary var. to convert signed nbit var. to signed 16bit int
  Int_t iii; // temporary var. to convert signed nbit var. to signed 32bit int
  Long64_t lll; //Long temporary var.
  int n_frm=0; //counter for frmt4 and frmt5
  peak_type *ipk=&dummy_peak; //pointer to the current peak in the current pulse;
  //Double_t QX=0,QY=0,RX,RY;
  Double_t QX=0,RX,AY;

  eventlist *Blist;
  UChar_t frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;

  //Print_Buf(ibuf,"buf.dat");
  //exit(1);
  //cout << "Decode34: " << ibuf << " " << buf_off[ibuf] << " " << vv->size() << " " << length << " " << hex << buf8[idx1/8] << dec << endl;

  //bool st=0;

  while (idx1<b_end[ibuf]) {
    frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
    // frmt = GLBuf[idx1+6];
    //YKYKYK!!!! do something with cnt - ???
    //int cnt = frmt & 0x0F;
    // frmt = (frmt & 0xF0)>>4;
    data = buf8[idx1/8] & sixbytes;
    unsigned char ch = GLBuf[idx1+7];

    // cout << "frmt: " << (int)frmt << " " << Blist->size() << " " << (int) ch << " " << (int) ipls.Chan << endl;

    // if (!frmt && Blist->empty()) {
    //   ++errors[0];
    //   //cout << "dec34: bad buf start: " << idx1 << " " << (int) ch << " " << frmt << endl;
    //   idx1+=8;
    //   continue;
    // }
    if (ch==255) {
      //start signal
      idx1+=8;
      continue;      
    }
    else if (frmt<6) {
      if (ch>=opt.Nchan) { //bad channel

	++errors[1];
	ipls.ptype|=P_BADCH;
	idx1+=8;
	continue;
      }
      if (frmt && ch!=ipls.Chan) { //channel mismatch
	++errors[2];
	ipls.ptype|=P_BADCH;
	idx1+=8;
	continue;
      }
    }

    switch (frmt) {
    case 0:
      if (buf8[idx1/8]==0) {
	++errors[4];
	idx1+=8;
	continue;
      }
			
      //analyze previous pulse
      if (ipls.ptype==0) {
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
      }
      // create new pulse
      ipls=PulseClass();
      npulses++;
      ipls.Chan=ch;
      ipls.Tstamp64=data+(Long64_t)opt.Delay[ch];// - cpar.preWr[ch];
      //cout << "Tstmp64: " << ipls.Tstamp64 << " " << Blist->back().Tstmp << " " << Blist->size() << endl;
      n_frm=0;
      break;
    case 1:
      ipls.State = GLBuf[idx1+5];
      ipls.Counter = data & 0xFFFFFFFFFF;
      break;
    case 2:
      if ((int)ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
	// cout << "32: ERROR Nsamp: "
	//      << " " << (ipls.Counter & 0x0F)
	//      << " " << ipls.sData.size() << " " << cpar.durWr[ipls.Chan]
	//      << " " << (int) ch << " " << (int) ipls.Chan
	//      << " " << idx8 //<< " " << transfer->actual_length
	//      << endl;
	ipls.ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<4;i++) {
	iii = data & 0x7FF;
	ipls.sData.push_back((iii<<21)>>21);
	data>>=12;
      }
      //}
      break;
    case 3:
      if ((int)ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
	ipls.ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<3;i++) {
	iii = data & 0xFFFF;
	ipls.sData.push_back((iii<<16)>>16);
	data>>=16;
      }
      //}
      break;
    case 4:
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [24]; A – [24]
	  //area
	  iii = data & 0xFFFFFF;
	  ipk->Area0=((iii<<8)>>8);
	  ipk->Area0/=p_len[ipls.Chan];
	  data>>=24;
	  //bkg
	  iii = data & 0xFFFFFF;
	  ipk->Base=((iii<<8)>>8);
	  ipk->Base/=b_len[ipls.Chan];
	  ipk->Area=ipk->Area0 - ipk->Base;
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }
	  break;
	case 1: //H – [12]; QX – [36]
	  lll = data & 0xFFFFFFFFF;
	  QX=((lll<<28)>>28);
	  data>>=36;
	  //height
	  iii = data & 0xFFF;
	  ipk->Height=((iii<<20)>>20);
	  break;
	  // case 2: //QY – [36]
	  //   lll = data & 0xFFFFFFFFF;
	  //   QY=((lll<<28)>>28);
	  //   break;
	case 2: //AY – [24]; RX – [20]
	  //RX
	  iii = data & 0xFFFFF;
	  RX=((iii<<12)>>12);
	  data>>=24;

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  //AY (Width)
	  if (ipk->Area) {
	    iii = data & 0xFFFFFF;
	    AY=((iii<<8)>>8);

	    ipk->Width=AY/w_len[ipls.Chan]-ipk->Base;
	    ipk->Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Width +
	      opt.E2[ipls.Chan]*ipk->Width*ipk->Width;
	    // if (opt.Bc[ipls.Chan]) {
	    //   ipk->Width+=opt.Bc[ipls.Chan]*ipk->Base;
	    // }
	    ipk->Width/=ipk->Area;
	  }
	  else {
	    ipk->Width=0;
	  }

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
      break;
    case 5:
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [28]; H – [16]
	  //height
	  sss = data & 0xFFFF;
	  ipk->Height=sss;
	  data>>=16;
	  //bkg
	  iii = data & 0xFFFFFFF;
	  ipk->Base=((iii<<4)>>4);
	  ipk->Base/=b_len[ipls.Chan];
	  break;
	case 1: //A – [28]
	  //area
	  iii = data & 0xFFFFFFF;
	  ipk->Area0=((iii<<4)>>4);
	  ipk->Area0/=p_len[ipls.Chan];

	  ipk->Area=ipk->Area0 - ipk->Base;
	  //ipk->Area*=opt.emult[ipls.Chan];
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }
	  break;
	case 2: //QX – [40]
	  lll = data & 0xFFFFFFFFFF;
	  QX=((lll<<24)>>24);
	  break;
	case 3: //AY – [28]
	  if (ipk->Area) {

	    iii = data & 0xFFFFFFF;
	    AY=((iii<<4)>>4);

	    ipk->Width=AY/w_len[ipls.Chan]-ipk->Base;
	    ipk->Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Width +
	      opt.E2[ipls.Chan]*ipk->Width*ipk->Width;
	    // if (opt.Bc[ipls.Chan]) {
	    // 	ipk->Width+=opt.Bc[ipls.Chan]*ipk->Base;
	    // }
	    ipk->Width/=ipk->Area;
	  }
	  else {
	    ipk->Width=0;
	  }
	  // cout << "Width: " << ipls.Counter << " " << ipk->Width << " " << ipk->Area << " " << ipk->Area0 << endl;

	  break;
	case 4: //reserved – [24]; RX – [24]
	  //RX
	  iii = data & 0xFFFFFF;
	  RX=((iii<<8)>>8);
	  data>>=24;

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
      break;
    case 6:
      // if (data) {
      // 	cout << "frmt6: " << (int) ch << " " << data << " " << npulses2[ch] << endl;
      // }
      npulses3[ch]=data;
      break;
    case 7:
      break;
    default:
      //Print_Buf(ibuf,"error.dat");
      //exit(1);

      ++errors[3];
      //cout << "bad frmt: " << frmt << endl;
    } //switch (frmt);

    //idx8++;
    idx1+=8;
  } //while (idx1<buf_len)

  //add last pulse to the list
  if (ipls.ptype==0) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  Dec_End(Blist,iread);

} //decode34


void CRS::Decode2(UInt_t iread, UInt_t ibuf) {

  UShort_t* buf2 = (UShort_t*) GLBuf;

  int idx2=b_start[ibuf]/2;

  eventlist *Blist;
  UChar_t frmt = (buf2[idx2] & 0x7000)>>12;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;

  //cout << "Decode2: " << iread << " " << ibuf << " " << idx2 << " " << b_end[ibuf] << endl;

  //cout << "decode2: " << idx2 << endl;
  while (idx2<b_end[ibuf]/2) {

    unsigned short uword = buf2[idx2];
    frmt = (uword & 0x7000)>>12;
    short data = uword & 0xFFF;
    unsigned char ch = (uword & 0x8000)>>15;

    // if (frmt && Blist->empty()) {
    //   cout << "dec2: bad buf start: " << idx2 << " " << (int) ch << " " << frmt << endl;
    //   idx2++;
    //   continue;
    // }
    // if ((ch>=opt.Nchan) || (frmt && ch!=ipls.Chan)) {
    //   cout << "decode2: Bad channel: " << (int) ch
    // 	   << " " << (int) ipls.Chan << " " << frmt << " " << ipls.sData.size() << endl;
    //   ipls.ptype|=P_BADCH;

    //   idx2++;
    //   continue;
    // }

    if (ch>=opt.Nchan) { //bad channel

      ++errors[1];
      ipls.ptype|=P_BADCH;
      idx2++;
      continue;
    }
    if (frmt && ch!=ipls.Chan) { //channel mismatch
      ++errors[2];
      ipls.ptype|=P_BADCH;
      idx2++;
      continue;
    }

    if (frmt==0) {
      //ipls.ptype&=~P_NOSTOP; //pulse has stop

      //analyze previous pulse
      if (ipls.ptype==0) {
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
	//cout << "Pana: " << Blist->size() << " " << ipls.Tstamp64 << endl;
      }

      // create new pulse
      ipls=PulseClass();
      npulses++;
      ipls.Chan = ch;
      ipls.Tstamp64=data+(Long64_t)opt.Delay[ch];// - cpar.preWr[ch];
    }
    else if (frmt<4) {
      Long64_t t64 = data;
      ipls.Tstamp64+= (t64 << (frmt*12));
    }
    else if (frmt==4) {
      ipls.Counter=data;
    }
    else if (frmt==5) {
      if ((int)ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
	// cout << "2: Nsamp error: " << ipls.sData.size()
	//      << " " << (int) ch << " " << (int) ipls.Chan
	//      << " " << idx2
	//      << endl;
	ipls.ptype|=P_BADSZ;
      }
      //else {
      ipls.sData.push_back((data<<21)>>21);
      //cout << "decode2b: " << idx2 << endl;
      //}
    }

    idx2++;
  } //while

  //add last pulse to the list
  if (ipls.ptype==0) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  Dec_End(Blist,iread);

} //decode2


//-------------------------------

int CRS::Searchsync(int &idx, UInt_t* buf4, int end) {
  //returns 1 if syncw is found;
  //returns 0 if syncw is not found;

  for (;idx<end;++idx) {
    if (buf4[idx] == 0x2a500100) {
      return 1;
    }
  }
  return 0;
}

//-------------------------------------

void CRS::Decode_adcm(UInt_t iread, UInt_t ibuf) {

  //cout << "decode_adcm: " << endl;
  //it is assumed that at the beginning of this procedure idx points
  //to the valid data (correct sync word and type)

  //idx+0 - syncw+frame type
  //idx+1 - rLen + frame counter
  //idx+2 - rubbish
  //idx+3 - header
  //idx+4 - event counter + Fragment number
  //idx+5 .. idx+5+nsamp-1 - data
  //idx+rLen-3 - tst
  //idx+rLen-2 - cnst
  //idx+rLen-1 - crc32

  // nsamp+8=rLen


  eventlist *Blist;
  Dec_Init(Blist,0);
  PulseClass ipls=dummy_pulse;


  UInt_t* buf4 = (UInt_t*) GLBuf;
  UShort_t* buf2 = (UShort_t*) GLBuf;

  int idx=b_start[ibuf]/4; // current index in the buffer (in 1-byte words)
  int idnext=0; //idx of the next event
  int rLen; // length of one m-link fram
  UInt_t header; //data header
  UShort_t id; // block id (=0,1,2)
  UShort_t lflag; //last fragment flag
  //UShort_t nfrag; //fragment number
  UShort_t nsamp; // number of samples in the fragment
  int length = b_end[ibuf]/4;
  int ch;

  //cout << "idx: " << idx << " " << nevents << endl;

  while (idx<length) {
    //rLen (~rbuf[idx+1]) should be inside buf
    //at this point idx points to a valid syncw and idnext is also withing FBuf

    if (buf4[idx] != 0x2a500100) {
      cout << "bad syncw: " << idx << " " << buf4[idx] << endl;
      if (!Searchsync(idx,buf4,length)){
	cout << "sync word not found (YK: do something here...)" << endl;
	break;
      }
    }

    // printf("idx: %3d",idx);
    // for (int i=0;i<12;i++) {
    //   printf("%6d",buf2[idx*2+i]);
    // }
    // printf("\n");

    rLen = buf2[idx*2+3];
    idnext=idx+rLen;

    //cout << "while: " << idx << " " << idnext << " " << length << endl;

    if (idnext>length)
      break;

    header = buf4[idx+3];
    id=bits(header,26,31);
    if (id==1) {
      cout << "adcm: id==1. What a luck, counters!: " << id << " " << npulses << endl;
    }
    else { //id!=1
      //analyze previous pulse and insert it into the list

      nsamp=bits(header,7,17);
      if (nsamp+8!=rLen) {
	++errors[5]; //wrong adcm length
	//cout << "wrong length: " << idx << " " << nsamp << " " << bits(header,6,17) << " " << rLen << endl;
	//idx=idnext;
	goto next;
      }

      //cout << "nsamp: " << nsamp << " " << bits(header,6,17) << " " << rLen << endl;

      lflag=bits(header,6,6);
      ch=bits(header,18,25);
      if ((ch>=opt.Nchan)) {
	cout << "adcm: Bad channel: " << (int) ch
	     << " " << idx //<< " " << nvp
	     << endl;
	//ipls.ptype|=P_BADCH;
	goto next;
      }

      if (ipls.ptype==0) { //if previous pulse is good -> analyze it
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
      }

      //cout << "newpulse: " << ch << " " << (int)ipls.ptype << " " << ipls.Tstamp64 << endl;
      //new pulse
      if (!(ipls.ptype&P_NOSTOP)) { //if previous pulse has STOP -> insert new pulse
	//cout << "--------" << endl;
	ipls=PulseClass();
	npulses++;
	ipls.Chan=ch;

	ipls.Tstamp64 = buf4[idx+rLen-2];
	ipls.Tstamp64 <<= 32;
	ipls.Tstamp64 += buf4[idx+rLen-3]+(Long64_t)opt.Delay[ipls.Chan];

	if (Pstamp64==P64_0) {
	  Pstamp64=ipls.Tstamp64;
	  Offset64=0;
	  //cout << "Zero Offset64: " << Offset64 << endl;
	}

	if (Offset64)
	  ipls.Tstamp64-=Offset64;

	Long64_t dt=ipls.Tstamp64-Pstamp64;
	//10 or 20 sec = 2e9
	if (abs(dt) > 2000000000) { //bad event - ignore it

	  Offset64+=dt;
	  if (abs(Offset64) < 20000000) //~100-200 msec
	    Offset64=0;

	  //cout << "Offset64: " << Offset64 << endl;
	  ipls.ptype|=P_BADTST;

	  ++errors[6]; //bad adcm Tstamp
	  //cout << "bad Tstamp: "<<dt<<" "<<ipls.Tstamp64<<" "<<Pstamp64<<" "<<Offset64<<endl;
	}
	else {
	  Pstamp64=ipls.Tstamp64;
	}

      } //if (!(ipls.ptype&P_NOSTOP))
      else { //previous pulse has no stop -> this is continuation of the prev.pulse
	//ничего не делаем, хотя можно было бы проверить соответствие ch и Tstamp
      }

      if (lflag) {
	ipls.ptype&=~P_NOSTOP;
      }
      else {
	ipls.ptype|=P_NOSTOP;
      }

      //if (lastfl) {

      //cout << "decode_adcm pulse: " << npulses << endl;


      //ipls.Counter = buf2[idx*2+8];

      //nbits=bits(header,0,3);
      //nw=bits(header,4,5);
      //ipls.Tstamp64 = buf4[idx+rLen-3];

      // static Float_t baseline=0;
      // if (ipls.sData.empty() && nsamp) {
      // 	baseline = buf2[idx*2+11];
      // }

      for (int i=0;i<nsamp*2;i+=2) {
	ipls.sData.push_back(buf2[idx*2+i+11]);
	ipls.sData.push_back(buf2[idx*2+i+10]);
	//ipls.sData.push_back(buf2[idx*2+i+11]-baseline);
	//ipls.sData.push_back(buf2[idx*2+i+10]-baseline);
      }

      // if (lflag) {
      // 	ipls.ptype&=~P_NOSTOP; //pulse has stop
      // 	//analyze pulse
      // 	ipls.FindPeaks();
      // 	ipls.PeakAna();
      // }

    } //id!=1

    //cout << "idnext: " << idx << " " << idnext << endl;
    //AnaMLinkFrame();

  next:
    idx=idnext;

  } //while

  //cout << "nevt1: " << nevents << " " << (int) (ipls.ptype&~P_NOSTOP) << endl;

  if (!ipls.ptype) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  //cout << "nevt2: " << nevents << endl;

  Dec_End(Blist,iread);

  //cout << "idx2: " << idx << endl;
} //Decode_adcm


//-------------------------------------

/*
  void CRS::PrintPulse(int udata, bool pdata) {

  printf("Pulse: %d %d %lld %d %lld %d %lld\n",
  udata,iBP,npulses,ipls->Chan,ipls->Counter,ipls->Nsamp,ipls->Tstamp64);
  //cout << endl;

  if (pdata) {
  for (int i=0;i<ipls->Nsamp;i++) {
  printf("%d %f\n",i,ipls->sData[i]);
  }
  }

  }
*/

/*
  void CRS::Print_Pulses() {
  //std::vector<PulseClass> *vv = Vpulses+nvp;
  //list_pulse_reviter vv = Vpulses.rbegin();

  cout << "Pulses: " << npulses;
  for (UInt_t i=0;i<vv->size();i++) {
  cout << " " << (int)vv->at(i).Chan << "," << vv->at(i).Tstamp64;
  }
  cout << endl;

  }
*/

void CRS::Print_Events(const char* file) {
  std::streambuf * buf;
  std::ofstream of;

  if (file) {
    of.open(file);
    buf = of.rdbuf();
  }
  else {
    buf = std::cout.rdbuf();
  }
  std::ostream out(buf);

  for (std::list<EventClass>::iterator it=Levents.begin();
       it!=Levents.end();++it) {
    out << "--- Event: " << it->Nevt << " M: " << it->pulses.size() << " Tstamp: " << it->Tstmp << endl;
    for (UInt_t i=0;i<it->pulses.size();i++) {
      PulseClass pp = it->pulses.at(i);
      out << "Ch: " << (int)pp.Chan << " Tstamp: " << pp.Tstamp64;
      for (std::vector<peak_type>::iterator pk=pp.Peaks.begin();
	   pk!=pp.Peaks.end();++pk) {
	out << " Pk: " << pk->Time << " " << pk->Area << " " << pk->Width;
      }
      out << endl;

      for (int j=0;j<(int)pp.sData.size();j++) {
	out << j << " " << pp.sData[j] << endl;
	//printf("-- %d %f\n",i,pp.sData[i]);
      }

    }
    //out << endl;
  }

  of.close();
}

void CRS::Print_Peaks(const char* file) {
  std::streambuf * buf;
  std::ofstream of;

  if (file) {
    of.open(file);
    buf = of.rdbuf();
  }
  else {
    buf = std::cout.rdbuf();
  }
  std::ostream out(buf);

  out << "Nevt Chan Tstamp(ns) Area" << endl;
  for (std::list<EventClass>::iterator it=Levents.begin();
       it!=Levents.end();++it) {
    for (UInt_t i=0;i<it->pulses.size();i++) {
      PulseClass pp = it->pulses.at(i);
      for (std::vector<peak_type>::iterator pk=pp.Peaks.begin();
	   pk!=pp.Peaks.end();++pk) {
	out << it->Nevt << " " << (int)pp.Chan << " " << pp.Tstamp64*int(opt.Period) << " " << pk->Area << endl;
      }
    }
    //out << endl;
  }

  of.close();
}

void CRS::Print_b1(int idx1, std::ostream *out) {
  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];
  unsigned short frmt;
  int cnt;
  int ch;
  ULong64_t data;

  frmt=GLBuf[idx1+6];
  cnt = frmt & 0x0F;
  frmt = (frmt & 0xF0)>>4;
  ch = GLBuf[idx1+7];
  data = buf8[idx1/8] & sixbytes;

  *out << setw(4) << frmt
       << setw(4) << ch
       << setw(4) << cnt
       << setw(15) << hex << data << dec;
}

void CRS::Print_Buf(UInt_t ibuf, const char* file) {
  std::streambuf * buf;
  std::ofstream of;

  if (file) {
    of.open(file);
    buf = of.rdbuf();
  }
  else {
    buf = std::cout.rdbuf();
  }
  std::ostream *out = new std::ostream(buf);

  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];

  int idx1=b_start[ibuf];

  int goodch=999;
  if (idx1-1>=b_start[ibuf])
    goodch = GLBuf[idx1+7];
  cout << "Most probably error in ch: " << goodch << endl;

  while (idx1<b_end[ibuf]) {

    Print_b1(idx1,out);
    if (idx1>=4) {
      idx1-=4;
      Print_b1(idx1,out);
      idx1+=4;
    }
    *out << hex << setw(17) << buf8[idx1/8] << dec << endl;
    idx1+=8;
  }

  of.close();
}

void CRS::Event_Insert_Pulse(eventlist *Elist, PulseClass* pls) {
  //void CRS::Event_Insert_Pulse(pulse_vect::iterator pls) {

  event_iter it;
  event_reviter rit;
  //event_iter it_last;
  Long64_t dt;

  //cout << "Event_Insert_Pulse: " << nevents << " " << (int) pls->Chan << " " << pls->Counter << " " << (int) pls->State << " " << npulses << " " << pls->Tstamp64 << " " << (int) pls->ptype << endl;

  if (pls->ptype) { //any bad pulse
    // cout << "bad pulse: " << (int) pls->ptype << " " << (int) pls->Chan
    // 	 << " " << pls->Counter << " " << pls->Tstamp64 << endl;
    if (pls->Chan<254)
      npulses_bad[pls->Chan]++;
    return;
  }

  ++npulses2[pls->Chan];

  if (Elist->empty()) {
    //if (Tstart64<0) {
    //Tstart64 = pls->Tstamp64;

    // cout << "TStart64: " << Tstart64 << endl;
    it=Elist->insert(Elist->end(),EventClass());
    it->Nevt=nevents;
    it->Pulse_Ana_Add(pls);
    nevents++;
    //m_event=Elist->begin();

    return;
  }

  // ищем совпадение от конца списка до начала, но не больше, чем opt.ev_min
  rit=Elist->rbegin();
  // cout << "Elist: " << nevents << " " << Elist->size() 
  // << " " << rit->Tstmp << " " << pls->Tstamp64 << endl;

  int nn=opt.ev_min;
  //for (it=--Elist.end();it!=m_event && nn>0 ;--it,--nn) {
  for (rit=Elist->rbegin();rit!=Elist->rend() && nn>0 ;++rit,--nn) {
    dt = (pls->Tstamp64 - rit->Tstmp);
    if (dt > opt.tgate) {
      //add new event AFTER rit.base()
      it=Elist->insert(rit.base(),EventClass());
      it->Nevt=nevents;
      nevents++;
      it->Pulse_Ana_Add(pls);
      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      //cout << "t2: " << endl;
      // coincidence event
      rit->Pulse_Ana_Add(pls);
      return;
    }
  }

  ++errors[9];//event lag exceeded
  if (debug) {
    cout << "!!! beginning!!! ---: "
	 << nevents << " " << pls->Tstamp64 << " " << dt
	 << " " << nn << " " << Elist->size() << endl;
  }

  // if the current event is too early, insert it at the end of the event list
  it=Elist->insert(Elist->end(),EventClass());
  it->Nevt=nevents;
  it->Pulse_Ana_Add(pls);
  nevents++;

} //Event_Insert_Pulse

//void CRS::Make_Events(plist_iter it) {
void CRS::Make_Events(std::list<eventlist>::iterator BB) {

  //cout << "Make_Events: T_acq: " << gl_ivect << " " << opt.T_acq << " " << crs->Tstart64 << " " << Levents.back().T << endl;
	
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
    //return;
  }

  if (BB->back().State==123) {
    BB->pop_back();
  }

  //cout << "LL: " << Levents.rbegin()->Nevt << " " << Levents.rbegin()->TT << endl;
  //UInt_t sz = Levents.size();

  //merge beginning of BB and end of Levents
  if (!Levents.empty() && !BB->empty()) {
    evlist_iter it = BB->begin();
    evlist_reviter rr = Levents.rbegin();
    while (it!=BB->end() && it->Tstmp - rr->Tstmp<=opt.tgate*2) {
      //cout << "merge: -------------" << endl;
      for (UInt_t i=0;i<it->pulses.size();i++) {
	Event_Insert_Pulse(&Levents,&it->pulses[i]);
      }
      it=BB->erase(it);
      rr=Levents.rbegin();
    }
  }

  Levents.splice(Levents.end(),*BB);
  //cout << "BB1: " << Levents.size() << " " << Bufevents.size() << endl;

  Bufevents.erase(BB);

  //m_end = crs->Levents.end();
  //std::advance(m_end,-opt.ev_min);  
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

void CRS::Reset_Raw() {
  sprintf(raw_opt,"wb%d",opt.raw_compr);

  f_raw = gzopen(crs->rawname.c_str(),raw_opt);
  if (f_raw) {
    cout << "Writing parameters... : " << crs->rawname.c_str() << " " << module << endl;
    if (opt.raw_flag) {
      SaveParGz(f_raw,34);
    }
    else {
      SaveParGz(f_raw,module);
    }
    gzclose(f_raw);
  }
  else {
    cout << "Can't open file: " << crs->rawname.c_str() << endl;
  }

  sprintf(raw_opt,"ab%d",opt.raw_compr);

  //RawBuf8 = (ULong64_t*) RawBuf;
  iraw=0;
}

void CRS::Reset_Dec(Short_t mod) {
  sprintf(dec_opt,"wb%d",opt.dec_compr);

  f_dec = gzopen(crs->decname.c_str(),dec_opt);
  if (f_dec) {
    cout << "Writing parameters... : " << crs->decname.c_str() << endl;
    SaveParGz(f_dec,mod);
    gzclose(f_dec);
  }
  else {
    cout << "Can't open file: " << crs->decname.c_str() << endl;
  }

  sprintf(dec_opt,"ab%d",opt.dec_compr);

  DecBuf8 = (ULong64_t*) DecBuf;
  idec=0;
}

/*
  void CRS::Fill_Dec73(EventClass* evt) {

  for (UInt_t i=0;i<evt->pulses.size();i++) {
  //int ch = evt->pulses[i].Chan;
  for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
  peak_type* pk = &evt->pulses[i].Peaks[j];
  rP73.Area   = pk->Area;
  //rP.Width  = pk->Width      ;
  rP73.Time   = pk->Time;
  rP73.Ch     = evt->pulses[i].Chan;
  //rP.Type   = pk->Type       ;
  rPeaks73.push_back(rP73);
  }
  }

  UShort_t sz = 3 + sizeof(Long64_t) + rPeaks73.size()*sizeof(rpeak_type73);
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
  tt += evt->Tstmp;
  *buf8 = tt;
  idec+=sizeof(Long64_t);
  for (UInt_t i=0; i<rPeaks73.size(); i++) {
  rpeak_type73* buf = (rpeak_type73*) (DecBuf+idec);
  memcpy(buf,&rPeaks73[i],sizeof(rpeak_type73));
  idec+=sizeof(rpeak_type73);
  }
  rPeaks73.clear();

  }

  void CRS::Fill_Dec74(EventClass* evt) {

  for (UInt_t i=0;i<evt->pulses.size();i++) {
  for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
  peak_type* pk = &evt->pulses[i].Peaks[j];
  rP74.Area   = pk->Area;
  //rP.Width  = pk->Width      ;
  rP74.Time   = pk->Time*10;
  rP74.Ch     = evt->pulses[i].Chan;
  //rP.Type   = pk->Type       ;
  rPeaks74.push_back(rP74);
  }
  }

  UShort_t sz = 3 + sizeof(Long64_t) + rPeaks74.size()*sizeof(rpeak_type74);
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
  tt += evt->Tstmp;
  *buf8 = tt;
  idec+=sizeof(Long64_t);
  for (UInt_t i=0; i<rPeaks74.size(); i++) {
  rpeak_type74* buf = (rpeak_type74*) (DecBuf+idec);
  memcpy(buf,&rPeaks74[i],sizeof(rpeak_type74));
  idec+=sizeof(rpeak_type74);
  }
  rPeaks74.clear();

  } //Fill_Dec74
*/

void CRS::Fill_Dec75(EventClass* evt) {
  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - State
  // 2) N 8 byte words, each containing one peak
  //    1st (lowest) 2 bytes - Area
  //    2 bytes - Time*10
  //    2 bytes - Width
  //    1 byte - channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->State) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];

      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[0] = pk->Area;
      Decbuf2[1] = pk->Time*10;
      Decbuf2[2] = 0;
      Decbuf2[3] = evt->pulses[i].Chan;
      ++DecBuf8;
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
    DecBuf8 = (ULong64_t*) DecBuf;
  }

} //Fill_Dec75

void CRS::Fill_Dec76(EventClass* evt) {
  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - State
  // 2) N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - Area
  //    2 bytes - Time*10
  //    2 bytes - Width
  //    1 byte - channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->State) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];
      *DecBuf8=0;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[0] = pk->Area;
      Decbuf2[1] = pk->Time*10;
      // Decbuf2[2] = 0;
      Decbuf2[3] = evt->pulses[i].Chan;
      ++DecBuf8;
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
    DecBuf8 = (ULong64_t*) DecBuf;
  }

} //Fill_Dec76

void CRS::Fill_Dec77(EventClass* evt) {
  //Decode77 is the same as 76, but since this version Time is relative to event Tstmp (was relative to pulse Tstamp64 in Decode76)
  //Width is recorded

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - State
  // 2) N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - Area
  //    2 bytes - Time*10
  //    2 bytes - Width
  //    1 byte - channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->State) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];
      *DecBuf8=0;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[0] = pk->Area;
      Decbuf2[1] = pk->Time*10;
      Decbuf2[2] = pk->Width;
      Decbuf2[3] = evt->pulses[i].Chan;
      ++DecBuf8;
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
    DecBuf8 = (ULong64_t*) DecBuf;
  }

} //Fill_Dec77

void CRS::Fill_Dec78(EventClass* evt) {
  //Fill_Dec78 - the same as 77, but different factors for Time and Width

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - State
  // 2) N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - Area
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->State) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];
      *DecBuf8=0;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[0] = pk->Area;
      Decbuf2[1] = pk->Time*100;
      Decbuf2[2] = pk->Width*1000;
      Decbuf2[3] = evt->pulses[i].Chan;
      ++DecBuf8;
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
    DecBuf8 = (ULong64_t*) DecBuf;
  }

} //Fill_Dec78

void CRS::Fill_Dec79(EventClass* evt) {
  //Fill_Dec79 - the same as 78, but different factor for Area

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - State
  // 2) N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - (unsigned) Area*5+1
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->State) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  for (UInt_t i=0;i<evt->pulses.size();i++) {
    for (UInt_t j=0;j<evt->pulses[i].Peaks.size();j++) {
      peak_type* pk = &evt->pulses[i].Peaks[j];
      *DecBuf8=0;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      UShort_t* Decbuf2u = (UShort_t*) Decbuf2;
      if (pk->Area<0) {
	*Decbuf2u = 0;
      }
      else if (pk->Area>13106){
	*Decbuf2u = 65535;
      }
      else {
	*Decbuf2u = pk->Area*5+1;
      }
      Decbuf2[1] = pk->Time*100;
      Decbuf2[2] = pk->Width*1000;
      Decbuf2[3] = evt->pulses[i].Chan;
      //cout << evt->Nevt << " " << evt->Tstmp << " " << (int) evt->pulses[i].Chan << endl;
      ++DecBuf8;
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
    DecBuf8 = (ULong64_t*) DecBuf;
  }

} //Fill_Dec79

void CRS::Flush_Dec() {

  //idec=0;
  //return;

  if (!idec) return;
  sprintf(dec_opt,"ab%d",opt.dec_compr);
  f_dec = gzopen(crs->decname.c_str(),dec_opt);
  if (!f_dec) {
    cout << "Can't open file: " << crs->decname.c_str() << endl;
    idec=0;
    return;
  }

  int res=gzwrite(f_dec,DecBuf,idec);
  if (res!=idec) {
    cout << "Error writing to file: " << crs->decname.c_str() << " " 
	 << res << " " << idec << endl;
  }
  idec=0;
  decbytes+=res;

  gzclose(f_dec);
  f_dec=0;

}

void p_buf8(int id,ULong64_t* buf8) {
  //ULong64_t* buf8 = (ULong64_t*) buf;

  Long64_t ibuf = buf8 - (ULong64_t*)crs->RawBuf;

  ULong64_t data;
  unsigned short frmt,ch;
  UChar_t* buf = (UChar_t*) buf8;

  frmt = buf[6];
  frmt = (frmt & 0xF0)>>4;
  ch = buf[7];
  data = *buf8 & sixbytes;
	
  printf("%1d: %6lld %4d %3d %16lld %20lld\n",id,ibuf,ch,frmt,data,*buf8);
  ibuf++;

}

void CRS::Fill_Raw(EventClass* evt) {
  const ULong64_t fmt1 = 1ull<<52;

  ULong64_t r8;
  ULong64_t rdata=0;
  UShort_t Mask;
  int shft;

  //cout << "Fill_Raw: " << iraw << endl;
  for (UInt_t i=0;i<evt->pulses.size();i++) {
    PulseClass *ipls = &evt->pulses[i];
    if (iraw + ipls->sData.size()/3 + 10 > RAWSIZE) {//+10 - с запасом
      Flush_Raw();
      //RawBuf8 = (ULong64_t*) RawBuf;
    }

    ULong64_t* RawBuf8 = (ULong64_t*) (RawBuf+iraw);
    //UShort_t* RawBuf2 = (UShort_t*) (RawBuf+6);

    //Tstamp - fmt0
    *RawBuf8=ipls->Tstamp64;
    RawBuf[iraw+6]=ipls->Counter & 0xF;
    RawBuf[iraw+7]=ipls->Chan;
    r8=(*RawBuf8)&0xFFFF000000000000;

    //p_buf8(0,RawBuf8);

    //Counter - fmt1
    ++RawBuf8;
    *RawBuf8=ipls->Counter & 0xFFFFFFFFFF;
    RawBuf[iraw+13]=ipls->State;
    r8+=fmt1;
    *RawBuf8|=r8;

    //p_buf8(1,RawBuf8);
		
    //Data - fmt2 or fmt3
    int f_dat=3;//data format (2 or 3)
    if (f_dat==2) { //11 bits per data
      r8+=fmt1; //bytes 6,7 -> fmt2
      shft=12;
      Mask=0x7FF;
    }
    else { //16 bits per data
      r8+=fmt1*2; //bytes 6,7 -> fmt3
      shft=16;
      Mask=0xFFFF;
    }
    ++RawBuf8;
    *RawBuf8=r8;
    int ish=0;
    for (int j=0;j<(int)ipls->sData.size();j++) {
      rdata=ipls->sData[j];
      rdata&=Mask;
      rdata<<=ish;
      *RawBuf8|=rdata;
      //cout << "rdata: " << *RawBuf8 << " " << rdata << endl;
      if (ish>30) { //32 or 36
	//p_buf8(f_dat,RawBuf8);
	RawBuf8++;
	*RawBuf8=r8;
	//cout << "r8: " << *RawBuf8 << " " << r8 << endl;
	ish=0;
      }
      else {
	ish+=shft;
      }
    }
    UChar_t* last = (UChar_t*) RawBuf8;
    iraw = last - RawBuf;
  }
} //Fill_Raw

void CRS::Flush_Raw() {

  /*
    cout << "Flush_Raw: " << iraw << endl;
    Long64_t i8=0;
    ULong64_t* buf8 = (ULong64_t*) RawBuf;
    while (i8<iraw/8) {
    printf("%6lld %20lld\n",i8,buf8[i8]);
    ++i8;
    }
  */

  if (!iraw) return;
  sprintf(raw_opt,"ab%d",opt.raw_compr);
  f_raw = gzopen(crs->rawname.c_str(),raw_opt);
  if (!f_raw) {
    cout << "Can't open file: " << crs->rawname.c_str() << endl;
    iraw=0;
    return;
  }

  int res=gzwrite(f_raw,RawBuf,iraw);
  if (res!=iraw) {
    cout << "Error writing to file: " << crs->rawname.c_str() << " " 
	 << res << " " << iraw << endl;
  }
  iraw=0;
  rawbytes+=res;

  gzclose(f_raw);
  f_raw=0;

}
