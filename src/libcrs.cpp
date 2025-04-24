#include "libcrs.h"
#include "adcm_df.h"

//#include <iostream>
#include <zlib.h>
#include <sys/stat.h>
#include <iomanip>
#include<sys/mman.h>
#include <chrono>

//#ifdef CYUSB
//#include "cyusb.h"
//#endif

//#include <pthread.h>
#include "romana.h"
#include "decoder.h"

//#include <malloc.h>
#include <TClass.h>
#include <TCanvas.h>
#include <TDataMember.h>
#include <TSystem.h>

//#include <TSemaphore.h>
//TSemaphore sem;
#include "TMath.h"

#include "TThread.h"
#include "TMutex.h"
#include "TRandom.h"
#include "TApplication.h"

#include <bitset>

//int buf_inits=0;
//int buf_erase=0;


//TMutex Emut3;
TMutex stat_mut;
//TMutex ana_mut;

TMutex decode_mut;
TMutex raw_mut;
TMutex decw_mut;

//TMutex ringdec_mut;

TMutex cmut;

const int BFMAX=999999999;

#define ID_ADCM  0x2A50
#define ID_CMAP  0x504D
#define ID_EVNT  0x5645
#define ID_CNTR  0x5443

using namespace std;

extern MemInfo_t minfo;
extern ProcInfo_t pinfo;
// extern double rmem;

BufClass *InBuf;
DecoderClass* decoder;
Simul* sim2;

extern CRS* crs;
extern Coptions cpar;
extern Toptions opt;

extern EventFrame* EvtFrm;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern ErrFrame* ErrFrm;
extern HClass* hcl;
extern ParParDlg *parpar;
extern ChanParDlg *chanpar;
extern HistParDlg *histpar;

extern int debug; // for printing debug messages

extern char startdir[200];
const double MB = 1024*1024;
const Long64_t iMB = 1024*1024;

extern char mainname[200];

//MemInfo_t info;

TRandom rnd;

//Int_t ev_max;

//bool bstart=true;
//bool btest=false;

//const Long64_t P64_0=-123456789123456789;

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
const int MAXSHORT=32767;

//const Long64_t GLBSIZE=2147483648;//1024*1024*1024*2; //1024 MB
//const Long64_t GLBSIZE=1024*1024*1024; //1024 MB

int tr_size; //=opt.usb_size*1024; - размер трансфера в байтах

Long64_t gl_sz;
const Long64_t gl_off = iMB; //1MB, was 1024*128 - офсет GLBuf относительно GLBuf2 - нужен для копирования хвоста буфера в начало (см. AnaBuf) !! возможно, не нужен

UChar_t* GLBuf;
UChar_t* GLBuf2;

Long64_t b_start[CRS::MAXTHREADS]; //start of local buffer(part of GLBuf), included
Long64_t b_fill[CRS::MAXTHREADS]; //start of local buffer for reading/filling
Long64_t b_end[CRS::MAXTHREADS]; //end of local buffer(part of GLBuf), excluded

UInt_t gl_iread; //current number of readbuf or cback [0.. infinity)
UInt_t gl_ivect; //current number of pstruct in make_event [0.. infinity)
UInt_t gl_ibuf; //current buffer for decode* [0 .. gl_Nbuf]
UInt_t gl_Nbuf; //maximal number of buffers for decode*
//int gl_ntrd=6; //number of decode threads (and also sub-buffers)

int event_thread_run;//=1;
int decode_thread_run;
int mkev_thread_run;
int ana_thread_run;
int wrt_thread_run;
int dec_nr[CRS::MAXTHREADS];




TThread* trd_crs;
TThread* trd_dec[CRS::MAXTHREADS];
//TCondition dec_cond[CRS::MAXTRANS];
TThread* trd_mkev;
//TCondition mkev_cond[CRS::MAXTRANS];
//int mkev_check[CRS::MAXTRANS];
TThread* trd_ana;
//TCondition ana_cond;
//int ana_check;
TThread* trd_dec_write;
TThread* trd_raw_write;


UInt_t dec_iread[CRS::MAXTHREADS];
// dec_iread[i]=0 в начале
// handle_decode крутится для n потоков
// пока dec_iread==0 для данного потока, handle_decode ждет
// dec_iread[i]=0 если i-й буфер обработан. В буфер можно читать.
// Если dec_iread[i]=1, i-й буфер еще не обработан

int ana_all;

//TCondition evt_cond;
//int evt_check;

//TThread* trd_ev;
//TCondition ev_cond;
//int ev_check;

//int make_event_thread_run;
//int ana2_thread_run;

//UInt_t list_min=100;

//volatile char astat[CRS::MAXTRANS];

int chan_in_module;

Double_t tproc;

#ifdef TIMES
TTimeStamp tt1[10];
TTimeStamp tt2[10];
double ttm[10];
double dif;
#endif

EventClass levt;

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

//static int timeout = 0;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

#ifdef CYUSB
//cyusb_handle *cy_handle;

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
  // сохраняем текущий буфер
  //unsigned char* cbuf = trans->buffer;

  if (trans->actual_length) {

    // проверяем slow decoding
    while (dec_iread[gl_ibuf]) {
      ++crs->errors[ER_DEC]; //slow decoding
      gSystem->Sleep(1);
    }

    // запускаем анализ буфера (переделать!)
    double rr =
      double(trans->buffer-GLBuf+trans->actual_length)/gl_sz;
    UInt_t nn = (rr+1e-6)*gl_Nbuf;
    //if (opt.decode) {
    if (!opt.directraw) {
      if (nn!=gl_ibuf) {
	// cout << "cback2: " << itr << " " << nn << " " << gl_ibuf << endl;
	int length=trans->buffer-GLBuf-b_fill[gl_ibuf]+trans->actual_length;
	b_end[gl_ibuf]=b_fill[gl_ibuf]+length;
	// prnt("ss2d2d2d3ds;",KYEL,"cback:",gl_iread, itr, gl_ibuf,CheckMem(),RST);
	crs->AnaBuf(gl_ibuf);

      }
    } //if !opt.directraw

    // запускаем запись буфера, если opt.raw_write (переделать!)
    if (opt.raw_write && !opt.fProc) {

      if (opt.nthreads>1) {
	crs->Flush_Raw_MT(trans->buffer, trans->actual_length);
      }
      else {
	crs->f_raw = gzopen(crs->rawname.c_str(),crs->raw_opt);
	if (crs->f_raw) {
	  int res=gzwrite(crs->f_raw,trans->buffer,trans->actual_length);
	  gzclose(crs->f_raw);
	  crs->rawbytes+=res;
	}
	else {
	  cout << "Can't open file: " << crs->rawname.c_str() << endl;
	  opt.raw_write=false;
	}
      }
      
    }

    //trans->buffer=next_buf;
    // if (crs->b_acq) {
    //   libusb_submit_transfer(trans);
    // }


  } // if (trans->actual_length)

  //запускаем новый транс
  if (crs->b_acq) {
    int itr = *(int*) trans->user_data; //номер трансфера
    int i_prev = (itr+crs->ntrans-1)%crs->ntrans; //previous itr
    UChar_t* next_buf=crs->transfer[i_prev]->buffer + tr_size;
    if (next_buf+tr_size > GLBuf+gl_sz) {
      next_buf=GLBuf;
    }

    //prnt("ss d x x xs;",BGRN,"trf:",itr,gl_sz,trans->buffer-GLBuf,trans->actual_length,RST);
    trans->buffer=next_buf;
    libusb_submit_transfer(trans);

    stat_mut.Lock();
    crs->nbuffers++;
    crs->inputbytes+=trans->actual_length;
    stat_mut.UnLock();
  }

} //cback

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

// void *handle_master_thread(void *ctx) {
//   while (master_thread_run) {
//   }
// }

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

    // prnt("ss2d2d2d .1f d ds;",KGRN,"decWk1:",gl_iread, ibuf, gl_ibuf,
    // opt.T_acq,crs->Levents.size(),crs->Bufevents.size(),RST);
    crs->Decode_switch(ibuf);
    // prnt("ss2d2d2d .1f d ds;",KGRN,"decWk2:",gl_iread, ibuf, gl_ibuf,
    // opt.T_acq,crs->Levents.size(),crs->Bufevents.size(),RST);

  } //while

  // cmut.Lock();
  // cout << "Decode thread deleted: " << ibuf << endl;
  // cmut.UnLock();
  return NULL;
}

void *handle_mkev(void *ctx) {
  std::list<CRS::eventlist>::iterator BB;

  while (mkev_thread_run) {

    // gSystem->GetMemInfo(&info);
    // cout << "\033[33m";
    // cout << "Make_events1: " << gl_ivect << " " << crs->Bufevents.size()
    // 	 << " " << crs->Levents.size() << " " << info.fMemUsed << endl;
    // cout << "\033[0m";

    bool fdec=false; //proper dec.. finished
    while(!fdec && mkev_thread_run) {
      for (BB = crs->Bufevents.begin(); BB!=crs->Bufevents.end();++BB) {
	// cout << "mkev: " << crs->Bufevents.size() << " " << BB->front().Nevt
	//      << " " << gl_ivect << " " << (int) BB->front().Spin << endl;
	if (BB->back().Nevt==gl_ivect && BB->back().Spin>=254) {
	  fdec=true;
	  break;
	}
      }
      gSystem->Sleep(1);

    }

    if (!mkev_thread_run) {
      break;
    }

    //cout << "mkev: " << crs->Bufevents.size() << " " << gl_iread << " " << gl_ivect << " " << gl_ibuf << endl;
    // prnt("ss d ds;",KBLU,"mkev1:",
    // 	 crs->Levents.size(),crs->Bufevents.size(),RST);
    crs->Make_Events(BB);
    // prnt("ss d ds;",KBLU,"mkev2:",
    // 	 crs->Levents.size(),crs->Bufevents.size(),RST);
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

    crs->Ana2(ana_all);

    // вычисляем, нужно ли замедлять чтение файла, если анализ отстает
    if (crs->Fmode>1) { //только для чтения файла
      crs->L4=double(crs->Levents.size())/opt.ev_max;
      if (crs->L4>2) {
	++crs->N4;
      }
      else {
	crs->N4=0;
      }
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




/*
void D79(UChar_t* DBuf, int len, CRS::eventlist &Blist) {
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - Spin
  // 2) N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - (unsigned) Area*5+1
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel


  int idx1=0;
  UChar_t frmt = DBuf[idx1+7] & 0x80;
  EventClass* evt = &crs->dummy_event;
  int nevents=0;

  while (idx1<len) {
    frmt = DBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (DBuf+idx1);

      evt = &*Blist.insert(Blist.end(),EventClass());
      evt->Nevt=nevents;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->Spin = Bool_t((*buf8) & 0x1000000000000);
      // if (idx1==0) {
      // 	prnt("ss d d l xs;",BYEL,"D79:",nevents,idx1,evt->Tstmp,*buf8,RST);
      // }
      nevents++;
    }
    else {
    }

    idx1+=8;
  } //while (idx1<buf_len)

} //D79
*/





void *handle_dec_write(void *ctx) {
  
  cmut.Lock();
  cout << "dec_write thread started: " << endl;
  cmut.UnLock();

  while (wrt_thread_run || !crs->decw_list.empty()) {

    if (!crs->decw_list.empty()) { //write

      Pair p=crs->decw_list.front();
      UChar_t* buf = p.first;
      int len = p.second;

      decw_mut.Lock();
      // prnt("ss d x d ls;",KGRN,"decw_write: ",crs->decw_list.size(), buf, len, crs->DecBuf-crs->DecBuf_ring,RST);
      crs->decw_list.pop_front();
      decw_mut.UnLock();

      crs->Wr_Dec(buf,len);

    } //if (!crs->decw_list.empty())
    else {
      gSystem->Sleep(5);
    }

  } //while (wrt_thread_run)

  cmut.Lock();
  cout << "dec_write thread stopped: " << endl;
  cmut.UnLock();

  return NULL;
} //handle_dec_write

void *handle_raw_write(void *ctx) {

  cmut.Lock();
  cout << "raw_write thread started: " << endl;
  cmut.UnLock();

  //return 0;
  while (wrt_thread_run || !crs->rw_list.empty()) {

    if (!crs->rw_list.empty()) { //write

      Pair p=crs->rw_list.front();
      UChar_t* buf = p.first;
      int len = p.second;

      raw_mut.Lock();
      crs->rw_list.pop_front();
      //prnt("ss d ds;",KGRN,"raw_write: ",crs->rw_list.size(),buf-GLBuf,RST);
      raw_mut.UnLock();

      crs->f_raw = gzopen(crs->rawname.c_str(),crs->raw_opt);
      if (crs->f_raw) {
	int res=gzwrite(crs->f_raw,buf,len);
	if (res!=len) {
	  cout << "Error writing to file: " << crs->rawname.c_str() << " " 
	       << res << " " << len << endl;
	  crs->rawbytes+=res;
	  opt.raw_write=false;
	  gzclose(crs->f_raw);
	  crs->f_raw=0;
	  break;
	}
	gzclose(crs->f_raw);
	crs->f_raw=0;
	crs->rawbytes+=res;
	
      }
      else {
	cout << "Can't open file: " << crs->rawname.c_str() << endl;
	opt.raw_write=false;
      }
      
    }
    else {
      gSystem->Sleep(5);
    }

  } //while (wrt_thread_run)

  cmut.Lock();
  cout << "raw_write thread stopped: " << endl;
  cmut.UnLock();

  return NULL;
} //handle_raw_write

void CRS::Ana_start() {
  //set initial variables for analysis
  //should be called before first call of ana2
  // b_mem=false;

  MakeDecMask();

  tproc=0;

  if (!batch) {
    parpar->DaqDisable();
    histpar->DaqDisable();
  }

  if (opt.ev_min>=opt.ev_max) {
    opt.ev_min=opt.ev_max/2;
  }
  //Set_Trigger();
  for (int i=0;i<MAX_CH;i++) {
    b_len[i] = opt.Base2[i]-opt.Base1[i]+1;
    p_len[i] = opt.Peak2[i]-opt.Peak1[i]+1;
    w_len[i] = opt.W2[i]-opt.W1[i]+1;
    b_mean[i] = (opt.Base2[i]+opt.Base1[i])*0.5;
    p_mean[i] = (opt.Peak2[i]+opt.Peak1[i])*0.5;
    w_mean[i] = (opt.W2[i]+opt.W1[i])*0.5;
    use_2nd_deriv[i] = opt.sTg[i]==5 || (opt.sTg[i]==-1 && cpar.Trg[i]==5);
    //cout << "Use_2nd: " << i << " " << use_2nd_deriv[i] << endl;
  }

  sPeriod = opt.Period*1e-9;


  // Создаем список Mainlist (гистограмм в Main)
  hcl->Mainlist.clear();
  //for (auto it = hcl->Actlist.begin();it!=hcl->Actlist.end();++it) {
  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->hd->b) { //активные (созданные) гистограммы
      //Mdef md = *(*it);
      bool inmain = false;

      //prnt("ss s ss;",BGRN,"ML:",it->h_name.Data(),(*it)->h_name.Data(),RST);
      //prnt("ss l ls;",BBLU,"ML:",it->v_map.size(),(*it)->v_map.size(),RST);
      //cout << "md: " << it->v_map[0] << " " << (*it)->v_map[0] << " " << it->hd << endl;

      for (auto map = it->v_map.begin();map!=it->v_map.end();++map) {
	if (*(map) && *(it->hd->w+(*map)->nn)) { // если эта гистограмма в MAIN
	  inmain=true;
	  //prnt("ss ds;",BRED,"Main:",(*map)->nn,RST);
	}
	//else {
	  //*map=0;
	//}
      }
      if (inmain) {
	hcl->Mainlist.push_back(&*it);
      }
    }
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
    wrt_thread_run=1;

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

    if (opt.dec_write) {
      trd_dec_write = new TThread("trd_dec_write", handle_dec_write, (void*) 0);
      trd_dec_write->Run();
    }

    if (opt.raw_write) {
      trd_raw_write = new TThread("trd_raw_write", handle_raw_write, (void*) 0);
      trd_raw_write->Run();
    }
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

#ifdef TPROC
      TTimeStamp pt1,pt2;
      pt1.Set();
#endif

      Double_t hcut_flag[MAXCUTS] = {0}; //признак срабатывания окон
      hcl->FillHist(&*m_event,hcut_flag);
#ifdef TPROC
      pt2.Set();
      tproc+=pt2.AsDouble()-pt1.AsDouble();
#endif
      if (m_event->Spin & 64) { //Ms channel
	if (!opt.maintrig || hcut_flag[opt.maintrig]) {
	  ++crs->mtrig;
	  if (opt.dec_write) {
	    switch (opt.dec_format) {
	    case 79:
	    case 80:
	      if (cpar.Trigger==1) { //trigger on START channel
		crs->Fill_Dec80(&(*m_event));
	      }
	      else {
		//crs->Fill_Dec73(&(*m_event));
		//crs->Fill_Dec74(&(*m_event));
		//crs->Fill_Dec75(&(*m_event));
		//crs->Fill_Dec76(&(*m_event));
		//crs->Fill_Dec77(&(*m_event));
		//crs->Fill_Dec78(&(*m_event));
		crs->Fill_Dec79(&(*m_event));
	      }
	      break;
	    case 81:
	      crs->Fill_Dec81(&(*m_event));
	      break;
	    default:
	      ;
	    } //switch
	  } // if dec_write
	  if (opt.raw_write && opt.fProc) {
	    crs->Fill_Raw(&(*m_event));
	  }
	  if (opt.fTxt && opt.nthreads==1) {
	    crs->Print_OneEvent(&(*m_event));
	  }
	} //maintrig
      } // if spin
    } // mult
    // else {
    // 	m_event=crs->Levents.erase(m_event);
    // }
    ++m_event;
  }

  // erase events if the list is too long
  for (event_iter it=crs->Levents.begin(); it!=m_event && nmax>0;--nmax) {
    it=crs->Levents.erase(it);
  }

  //removed on 06.02.2020
  //cout << "Flush_ana YK: " << endl;
  if (all) {
    if (opt.dec_write) {
      crs->Flush_Dec();
    }
    if (opt.raw_write && opt.fProc) {
      crs->Flush_Raw();
    }
  }

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

} //Ana2

// void tt(int* i) {
//   cout << i << " " << *i << endl;
//   i = new int;
//   *i=20;
//   cout << i << " " << *i << endl;
// }

BufClass::BufClass(size_t sz) {
  Size=sz;
  Buf = new char[Size];
  //cout << "size: " << Size << endl;
}

BufClass::~BufClass() {
  delete[] Buf;
}

CRS::CRS() {

  /*
  int test;
  int imin,imax;
  cpar.GetParm("hS",0,cpar.hS,imin,imax);
  cpar.GetParm("fdiv",0,cpar.fdiv,imin,imax);
  exit(1);

  DecBuf8 = new ULong64_t[1024];
  ULong64_t *DD = DecBuf8;

  DecBuf8[0] = 9991;
  DecBuf8[1] = 9992;
  DecBuf8[2] = 9993;

  *(++DecBuf8)=0;
  *(++DecBuf8)=1;

  DecBuf8 = DD;
  cout << DecBuf8[0] << " " << DecBuf8[1] << " " << DecBuf8[2] << endl;
  exit(1);
  */

  //ev_max=2*opt.ev_min;

  //mean_event.Make_Mean_Event();
  memset(crs_ch,0,sizeof(crs_ch));

  InBuf = new BufClass(1024*iMB); //1GB
  //memset(UsbBuf->Buf,0,UsbBuf->Size);
  decoder = new DecoderClass();
  decoder->zfile = &f_read;

  GLBuf2 = NULL;//new UChar_t[GLBSIZE];
  GLBuf = NULL;//new UChar_t[GLBSIZE];
  //memset(GLBuf,0,GLBSIZE);

  dummy_pulse.ptype=P_BADPEAK;
  dummy_pulse.Chan=254;

  good_pulse.Pos=0;
  good_event.Spin=64; //Ms is set

  ndev=0;

  // dummy_peak.Area=0;
  // dummy_peak.Height=0;
  // dummy_peak.Width=0;
  // dummy_peak.Time=-100;

  //for (int i=0;i<MAX_CHTP;i++) {
  //type_ch[i]=255;
  //}

  //MAXTRANS2=MAXTRANS;
  //memset(Pre,0,sizeof(Pre));

  idev=0;
  devname=0;
  Fmode=0;
  opt.Period=5;

  f_raw=0;
  f_read=0;
  f_dec=0;
  //f_tree=0;
  //Tree=0;

  batch=false;
  scrn=0;
  b_noheader=false;

  b_acq=false;
  b_fana=false;
  b_stop=true;
  // b_mem=false;
  b_run=0;
  //justopened=true;

  txt_out=0;

  strcpy(raw_opt,"ab");
  //strcpy(dec_opt,"ab");

  DecBuf_ring=new UChar_t[DECSIZE*NDEC]; //1*100 MB
  DecBuf=DecBuf_ring;
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

  ntrans=MAXTRANS;
  //opt.usb_size=1024*1024;

  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] =NULL;
    buftr[i]=NULL;
    //Fbuf[i]=NULL;
  }

  trd_crs=0;
  trd_mkev=0;
  trd_ana=0;
  trd_dec_write=0;
  trd_raw_write=0;

  for (int i=0;i<MAXTHREADS;i++) {
    trd_dec[i]=0;
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
  delete InBuf;
  delete[] DecBuf_ring;
  delete[] RawBuf;
  delete[] GLBuf2;

}

/*
  void CRS::Dummy_trd() {
  trd_dum = new TThread("trd_dum", handle_dum, (void*) 0);
  trd_dum->Run();
  }
*/

void CRS::DoDetectDev() {
#ifdef CYUSB
  if (!b_stop)
    return;
  //if (Fmode==1 && module>=32) {
  if (Fmode==1) { //если модуль уже подключен, удаляем trd_crs
    event_thread_run=0;
    if (Fmode==1) {
      //cyusb_close();
      if (trd_crs) {
	trd_crs->Delete();
	trd_crs=0;
      }
      Fmode=0;
    }
  }
		
#ifdef P_LIBUSB
  prnt("sss;",BYEL,"Sleep 100",RST);
#endif
  gSystem->Sleep(100);
#ifdef P_LIBUSB
  prnt("sss;",BGRN,"Reset USB... ",RST);
#endif

  if (cy_handle) { //close USB
#ifdef P_LIBUSB
    prnt("sss;",BGRN,"cyusb_close",RST);
#endif
    if (!cy_list.empty()) {
      cyusb_close();
      cy_handle=0;
      cy_list.clear();
    }
  }

  Open_USB();

  switch (ndev) {
  case 1:
    crs->Init_device();
  case 0:
    if (myM)
      myM->Build();
    break;
  default:
    new PopFrame(myM,1,1,M_DEVICE);
  }
#endif //CYUSB
}

void CRS::DoResetUSB() {
#ifdef CYUSB
  if (!b_stop)
    return;
  //if (Fmode==1 && module>=32) {
  if (Fmode==1) { //если модуль уже подключен, удаляем trd_crs
    event_thread_run=0;
    //if (Fmode==1) {
    //cyusb_close();
    if (trd_crs) {
      trd_crs->Delete();
      trd_crs=0;
    }
    Fmode=0;
    //}
  }
		
#ifdef P_LIBUSB
  prnt("sss;",BYEL,"Sleep 100",RST);
#endif
  gSystem->Sleep(100);
#ifdef P_LIBUSB
  prnt("sss;",BGRN,"Reset USB... ",RST);
#endif

  if (cy_handle && module>=32) { //reset USB
#ifdef P_LIBUSB
    prnt("sss;",BGRN,"Command 7",RST);
#endif
    Command32(7,0,0,0); //reset usb command

#ifdef P_LIBUSB
    prnt("sss;",BGRN,"cyusb_close",RST);
#endif
    if (!cy_list.empty()) {
      cyusb_close();
      cy_handle=0;
      cy_list.clear();
    }

#ifdef P_LIBUSB
    prnt("sss;",BYEL,"Sleep 2000",RST);
#endif
    gSystem->Sleep(2000);
  }
#ifdef P_LIBUSB
  prnt("sss;",BGRN,"Done.",RST);
#endif

  Open_USB();
  //Set_USB(idev);
  if (myM) {
    Init_device();
    myM->EnableBut(myM->fGr1,Fmode==1);
  }
  //}
  //else {
  //prnt("sss;",BRED,"Module not found or reset not possible",RST);
  //}  
#endif //CYUSB
}

#ifdef CYUSB

int CRS::Open_USB() {
  // открывает USB
  // создает и заполняет cy_list

  //cout << "Open_USB" << endl;

  if (!cy_list.empty())
    cyusb_close();

  cy_list.clear();

  ndev = cyusb_open();

  if ( ndev < 0 ) {
    printf("Error opening library\n");
    return -1;
  }
  else if ( ndev == 0 ) {
    printf("No device found\n");
    return -2;
  }

  // if ( ndev > 1 ) {
  //   printf("More than 1 devices of interest found. Disconnect unwanted devices\n");
  //   return -3;
  // }

  int nn=ndev; //копия
  for (int i=0;i<nn;i++) {
    //if (idev<0 || i==idev)
    Set_USB(i);
    cy_list.push_back(cpar.GetDevice(0,0));

    TString sdev = cpar.GetDevice(0,0);
    if (devname && sdev.Contains(*devname,TString::kIgnoreCase)) {
      idev = i;
      ndev=1;
    }
  }

  return ndev;
}

void CRS::Set_USB(int i) {
  // задает cy_handle в сответствии с i
  // добавляет девайс с данным cy_handle в cy_list

  int r;

  cy_handle = cyusb_gethandle(i);
  //cout << "set: " << i << " " << cy_handle << endl;
  if (cyusb_getvendor(cy_handle) != 0x04b4 ) {
    printf("Cypress chipset not detected\n");
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -4;
  }

  // YK 29.09.20
  r=cyusb_reset_device(cy_handle);
  //prnt("ssds;",KRED,"cyusb_reset: ",r,RST);
  //gSystem->Sleep(100);

  if ( r != 0 ) {
    printf("Can't reset device. Exitting: %d\n",r);
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -5;
  }
  // YK 29.09.20

  r = cyusb_kernel_driver_active(cy_handle, 0);
  if ( r != 0 ) {
    printf("kernel driver active. Exitting\n");
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -6;
  }

  r = cyusb_claim_interface(cy_handle, 0);
  if ( r != 0 ) {
    printf("Error in claiming interface: %d %d\n",i,r);
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -7;
  }
  else
    prnt("ss ds;",BGRN,"Successfully claimed interface",i,RST);

  // r = cyusb_release_interface(cy_handle, 0);
  // if ( r != 0 ) {
  //   printf("Error in releasing interface: %d %d\n",i,r);
  //   cyusb_close();
  //   cy_handle=0;
  //   return -7;
  // }
  // else
  //   prnt("ss ds;",BBLU,"Successfully released interface",i,RST);

  Device_info();

  /*
    bool bcc=false;

    TString sdev = cpar.GetDevice(0,0);
    //cout << "sdev: " << sdev << endl;
    if (devname) {
    if (sdev.Contains(*devname,TString::kIgnoreCase))
    bcc=true;
    }
    else
    bcc=true;

    if (bcc) {
    cy_list.push_back(cpar.GetDevice(0,0));
    idev = cy_list.size()-1;
    }
  */

  //idev=i;
} //Set_USB

void CRS::Device_info() {

  //module=0;

// #ifdef P_LIBUSB
//       prnt("sss;",BYEL,"Sleep 100",RST);
// #endif
//   gSystem->Sleep(100);

  //Command32(7,0,0,0); //reset usb command
  //YK
  //Command2(4,0,0,0);

  //int sz;
  memset(buf_in,0,sizeof(buf_in));
  //sz =
  Command32(1,0,0,0);
  //sz =
  Command32(1,0,0,0); //не помню, зачем вызывать 2 раза (?)

  memcpy(cpar.device,buf_in+1,4);

  //prnt("ss s ds;",BGRN,cpar.GetDevice(module,0).c_str(),"Module:",module,RST);

} //Device_info

int CRS::Init_device() {

  Set_USB(idev);

  Fmode=0;
  module=0;
  //Short_t firmw=0;

  event_thread_run=1;
  trd_crs = new TThread("trd_crs", handle_events_func, (void*) 0);
  trd_crs->Run();

#ifdef P_LIBUSB
  prnt("sss;",BYEL,"Sleep 100",RST);
#endif
  gSystem->Sleep(100);
  //Command32(7,0,0,0); //reset usb command
  //YK
  //Command2(4,0,0,0);

  int sz;
  memset(buf_in,0,sizeof(buf_in));
  sz = Command32(1,0,0,0);
  sz = Command32(1,0,0,0); //не помню, зачем вызывать 2 раза (?)

  memcpy(cpar.device,buf_in+1,4);

  Short_t nplates=buf_in[3];
  Short_t ver_po=buf_in[4];

  //for (int i=0;i<sz;i++) {
  //cout << int(buf_in[i]) << " ";
  //}
  //cout << endl;
  int nch2=0;

  switch (cpar.device[0]) {
  case 1: //crs-32
  case 2: //crs-6/16
  case 3: //crs-16 or crs-2
    opt.Period=5;
    //crs2
    if (ver_po==0) { // -> crs2
      module=22;
      chan_in_module=2;
      nch2=2;
      opt.Nchan=2;
      for (int j=0;j<chan_in_module;j++) {
	crs_ch[j]=1; //было 0
	//type_ch[j]=0;
      }
      break;
    }

    //crs32/16/6
    module=32;
    chan_in_module=nplates*4;
    if (ver_po==1) {//версия ПО=1
      for (int i=0;i<nplates;i++) {
	//cout << "Channels(" << i << "):";
	for (int j=0;j<4;j++) {
	  crs_ch[i*4+j]=2; //было 0
	  //cout << " " << crs_ch[i*4+j];
	  nch2++;
	}
	//cout << endl;
      }
    }
    else {//версия ПО=2 или выше
      //chan_in_module=4;
      sz = Command32(10,0,0,0);
      sz--;
      for (int i=0;i<nplates;i++) {
	//cout << "Channels(" << i << "):";
	for (int j=0;j<4;j++) {
	  //crs_ch[i*4+j]=buf_in[sz];
	  if (buf_in[sz]==0)
	    crs_ch[i*4+j]=2;
	  else if (buf_in[sz]==1)
	    crs_ch[i*4+j]=3;
	  else
	    crs_ch[i*4+j]=0;

	  //cout << " " << crs_ch[i*4+j];
	  nch2++;
	}
	//cout << endl;
	sz--;
      }
      if (ver_po==3) {//версия ПО=3
	module=33;
      }
      else if (ver_po==4) {//версия ПО=4
	module=34;
      }
      else if (ver_po>=5 && ver_po<=6) {//версия ПО=5 или 6
	module=35;
      }
      else if (ver_po==7) {//версия ПО=7
	module=36;
      }
    }

    break;

  case 4: //crs-8/16
    //module=41;
    //module=42;
    //module=43;
    module=44;
    opt.Period=10;
    chan_in_module=nplates*8;
    for (int j=0;j<chan_in_module;j++) {
      crs_ch[j]=4; //было 2
      cout << " " << crs_ch[j];
      nch2++;
    }
    cout << endl;
    break;

  case 5: //crs-128
    //module=51/52;
    //module=53;
    module=54;
    opt.Period=10;
    chan_in_module=nplates*16;
    for (int j=0;j<chan_in_module;j++) {
      crs_ch[j]=5; //было 2
      cout << " " << crs_ch[j];
      nch2++;
    }
    cout << endl;
    break;

  case 6: //AK-32
    module=45;
    opt.Period=6.4;
    chan_in_module=nplates*4;

    sz = Command32(10,0,0,0);
    sz--;

    for (int i=0;i<nplates;i++) {
      //cout << "Channels(" << i << "):";
      for (int j=0;j<4;j++) {
	crs_ch[i*4+j]=20+buf_in[sz];
	if (buf_in[sz]==5)
	  crs_ch[i*4+j]=6;
	else if (buf_in[sz]==4)
	  crs_ch[i*4+j]=7;
	else
	  crs_ch[i*4+j]=0;

	//cout << " " << crs_ch[i*4+j];
	nch2++;
      }
      //cout << endl;
      sz--;
    }
    
    //exit(-1);
    break;

  default:
    cout << "unknown device: " << endl;
    exit(1);
  }

  if (opt.Nchan>nch2) {
    opt.Nchan=nch2;
  }

  for (int i=0;i<MAX_CH;i++) {
    cpar.Len[i]=cpar.ChkLen(i,module);
  }
  
  //prnt("ssd ss;",BGRN,"Module: ",module, cpar.GetDevice(module).c_str(), RST);
  prnt("ss s ds;",BGRN,cpar.GetDevice(module).c_str(),"Module:",module,RST);

  if (module>=22) {
    Fmode=1;
    strcpy(mainname,cpar.GetDevice(module,0).c_str());
  }
  else
    Fmode=0;

  //cout << "title: " << mainname << " " << module << endl;
  // if (myM) {
  //   myM->EnableBut(myM->fGr1,Fmode==1);
  //   myM->DoReset();
  //   myM->SetTitle(mainname);
  // }

  InitBuf();

  if (Init_Transfer()) {
    return -8;
  };

  //Submit_all(MAXTRANS);
  //YK
  //Command2(4,0,0,0);

  return 0;

} //Init_device

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
  case 35:
    AllParameters35();
    break;
  case 36:
    AllParameters36();
    break;
  // case 41:
  // case 51:
  //   AllParameters41();
  //   break;
  // case 42:
  // case 52:
  //   AllParameters42();
  //   break;
  // case 43:
  // case 53:
  //   AllParameters43();
  //   break;
  case 44:
  case 54:
    AllParameters44();
    break;
  case 45:
    AllParameters45();
    break;
  default:
    cout << "SetPar Error! No module found" << endl;
    return 3;
  }

  return 0;

  //Thr[11]=100;

}

void CRS::Free_Transfer() {

  Cancel_all(MAXTRANS);
#ifdef P_LIBUSB
      prnt("sss;",BYEL,"Sleep 50",RST);
#endif
  gSystem->Sleep(50);

  //cout << "---Free_Transfer---" << endl;

  //for (int i=0;i<ntrans;i++) {
  for (int i=0;i<MAXTRANS;i++) {
    //cout << "free: " << i << " " << (int) transfer[i]->flags << endl;
    //int res = libusb_cancel_transfer(transfer[i]);
    libusb_free_transfer(transfer[i]);
#ifdef P_LIBUSB
    prnt("ssds;",BBLU,"libusb_free_transfer: ", i, RST);
#endif

    //transfer[i]=NULL;

  }

  for (int i=0;i<MAXTRANS;i++) {
    if (buftr[i]) {
      buftr[i]=NULL;
    }
  }
#ifdef P_LIBUSB
      prnt("sss;",BYEL,"Sleep 50",RST);
#endif
  gSystem->Sleep(50);

}

void CRS::Submit_all(int ntr) {
  ntrans=0;
  int res=0;
  for (int i=0;i<ntr;i++) {
    res = libusb_submit_transfer(transfer[i]);
    //cout << i << " Submit: " << res << endl;
    if (res) {
      //cout << "Submit_Transfer error: " << res << " " << *(int*) transfer[i]->user_data << endl;
      //cout << libusb_error_name(res) << endl;
      break;
    }
    else {
      //cout << "Submit_Transfer: " << res << " " << *(int*) transfer[i]->user_data << endl;
      ntrans++;
    }

#ifdef P_LIBUSB
    prnt("ssd d ds;",BBLU,"libusb_submit_transfer: ", i, ntrans, *(int*) transfer[i]->user_data, RST);
#endif
    
  }

  if (res) {
    prnt("ssd ds;",BRED,"Submit_Transfer error: ", res, ntrans, RST);
    //cout << "Submit_Transfer error: " << res << " " << *(int*) transfer[i]->user_data << endl;
    cout << libusb_error_name(res) << endl;
  }

}

void CRS::Cancel_all(int ntr) {
  //gSystem->Sleep(2300);
  for (int i=0;i<ntr;i++) {
    if (transfer[i]) {
#ifdef P_LIBUSB
      int res = libusb_cancel_transfer(transfer[i]);
      prnt("ssd ds;",BBLU,"libusb_cancel_transfer: ", i, res, RST);
      if (res)
	cout << libusb_error_name(res) << ": " << i << endl;
#else
      libusb_cancel_transfer(transfer[i]);
#endif
      //cout << i << " Cancel: " << res << endl;
    }
  }
}

int CRS::Init_Transfer() {

  //cout << "---Init_Transfer---" << endl;

  //Cancel_all(MAXTRANS);

  ///* YK 29.09.20
  //cout << "---Init_Transfer2---" << endl;
  //int r= cyusb_reset_device(cy_handle);
  //prnt("ssds;",KRED,"cyusb_reset: ",r,RST);
  //gSystem->Sleep(100);
  //*/

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
  for (int i=0;i<MAXTRANS;i++) {
    transfer[i] = libusb_alloc_transfer(0);
    //transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;
		
    int* ntr = new int;
    (*ntr) = i;

    unsigned int timeout=0; //200
    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], tr_size, cback, ntr, timeout);
    //libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i], opt.usb_size*1024, cback, ntr, 0);
#ifdef P_LIBUSB
    prnt("ssds;",BBLU,"libusb_fill_bulk_transfer: ", i, RST);
#endif

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

  //cout << "submit7:" << endl;
  Submit_all(MAXTRANS);

  /*
    for (int i=0;i<ntrans;i++) {
    int res;
    res = libusb_cancel_transfer(transfer[i]);
    cout << i << " Cancel: " << res << endl;
    }
  */
  //cout << "Number of transfers: " << ntrans << endl;

  if (ntrans!=MAXTRANS) {
    for (int i=ntrans;i<MAXTRANS;i++) {
      //cout << "free: " << i << endl;
      //libusb_free_transfer(transfer[i]);
      cout << "delete: " << i << endl;
      if (buftr[i]) {
	buftr[i]=NULL;
      }
    }
    return 2;
  }
#ifdef P_LIBUSB
      prnt("sss;",BYEL,"Sleep 250",RST);
#endif
  gSystem->Sleep(250);
  Cancel_all(MAXTRANS);
#ifdef P_LIBUSB
      prnt("sss;",BYEL,"Sleep 250",RST);
#endif
  gSystem->Sleep(250);

  return 0;

}

int CRS::Command32_old(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
  //для версии ПО 1
  int transferred = 0;
  int r;

  int len_in,len_out;

  //UChar_t buf_out[6];//={0};
  //UChar_t buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (UChar_t) (par >> 16);
  buf_out[4] = (UChar_t) (par >> 8);
  buf_out[5] = (UChar_t) par;

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

int CRS::Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
  // if (cmd !=2 || cpar.on[ch]) {
#ifdef P_CMD
  prnt("ssd d d d xs",BGRN,"Cmd32: ",(int) cmd, (int) ch, (int) type, par, par, RST);
#endif
  // }

  //для версии ПО 2
  int transferred = 0;
  int r;

  int len_in,len_out;

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (UChar_t) (par >> 16);
  buf_out[4] = (UChar_t) (par >> 8);
  buf_out[5] = (UChar_t) par;

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

  std::ostringstream oss;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    oss << " Error_out";
    //printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  if (cmd==8) { // если сброс сч./буф. -> задержка
    gSystem->Sleep(20);    
  }

  if (cmd!=7 && cmd!=12) { //сброс USB и Тест
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      oss << " Error_in";
	//sprintf(ss,"Error_in");
      //printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      //cyusb_close();
    }
#ifdef P_CMD
    else
      oss << " " << len_in << " " << (int) buf_in[0];
	//sprintf(ss,"%d %d",len_in,buf_in[0]);
#endif
  }
#ifdef P_CMD
  else {
    oss << " none";
  }
  prnt("ssss;",BGRN," :",oss.str().data(),RST);
#endif

  return len_in;
}

int CRS::Command2(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
#ifdef P_CMD
  prnt("ssds;",BRED,"Cmd2: ",(int) cmd, RST);
#endif
  //cout << "Command2: " << (int) cmd << endl;
  int transferred = 0;
  int r;

  int len_in,len_out;

  //UChar_t buf_out[6];//={0};
  //UChar_t buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[4] = (UChar_t) (par >> 8);
  buf_out[3] = (UChar_t) par;

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

  //std::ostringstream oss;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    //oss << " Error_out";
    cyusb_error(r);
    //cyusb_close();
  }

#ifdef P_CMD
  prnt("ssds;",BRED,"out: ", r, RST);
#endif

  //cout << "Sleep(10000) start" << endl;
  //gSystem->Sleep(10000); //300
  //cout << "Sleep(10000) end" << endl;
  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
  if (r) {
    cout << "Error_in: " << r << endl;
    //oss << " Error_in";
    cyusb_error(r);
    //cyusb_close();
  }
// #ifdef P_CMD
//   else
//     oss << " " << len_in << " " << (int) buf_in[0];
//   prnt("s s ss;",KRED,"<<",oss.str().data(),RST);
// #endif
//   cout << "Command2: " << (int) cmd << " " << r << " after in" << endl;

#ifdef P_CMD
  prnt("ssds;",BRED,"in: ", r, RST);
#endif

  return len_in;
}

void CRS::Check33(UChar_t cmd, UChar_t ch, int &a1, int &a2, int min, int max) {
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
  // cout << "Check33: " << (int) ch << " " << (int) cmd << " " << a1
  // << " " << a2 << " " << len << endl;
}

void CRS::AllParameters45() {
  AllParameters44();
  //Индивидуальные параметры каналов:
  for (auto chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {
      Command32(2,chan,37,0); //входной импеданс - всегда 50 Ом
    }
  }
  //Общие параметры:
  //Command32(11,10,0,0); // режим работы прибора Multi/Single (игнорируется)
  Command32(11,11,0,8); // число обслуживаемых рабочих плат MAIN-ом USB3
}

void CRS::AllParameters44() {
  //cout << "Allparameters44: " << endl;
  UInt_t mask_discr, //маска для дискр,СС и пересчета 
    mask_start, //маска для СТАРТ
    wmask; //маска разрешений записи

  UInt_t mask_group;

  AllParameters36();

  int hard_logic=0;
  if (cpar.Trigger==2) { //Coinc
    hard_logic=1;
  }

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    
    if (cpar.on[chan]) {

      mask_discr=0b0000000000011; //bits 0,1 (было еще 11,12)
      if (opt.dsp[chan]) {
	mask_discr|=0b11101110000; // write DSP data
      }

      if (cpar.pls[chan]) { //add 1100
	mask_discr|=0b1100; // write pulse
      }

      mask_start=0b1100000000001; //bits 0,11,12 - bitmask for START: tst,count48,overflow

      int w = 1;
      mask_group=0;
      for (int j=0;j<2;j++) {
	if (cpar.group[chan][j]) {
	  if (cpar.coinc_w[j]>w)
	    w=cpar.coinc_w[j];
	  mask_group+=j+1;
	}
      }

      wmask=2; // 0010 - запись по сигналу СТАРТ - всегда

      if (cpar.Trigger==0) { //discr
	wmask|=1;
      }
      else if (cpar.Trigger==1) { //START
	mask_start|=mask_discr; //добавляем запись импульса по старту
      }
      else { //coinc
	wmask|=4; // запись по СС
	if (cpar.RD[chan]) {
	  wmask|=8; // запись по пересчету
	  //wmask|=4; // запись по СС
	}
      }

      Command32(2,chan,23,mask_discr & cpar.RMask); //bitmask для дискр overwr AllPrms36
      Command32(2,chan,24,mask_start & cpar.RMask); //bitmask для START
      Command32(2,chan,25,wmask); // битовая маска разрешений записи
      Command32(2,chan,26,mask_discr & cpar.RMask); //bitmask для СС и пересчета


      Command32(2,chan,27,(int) w); // длительность окна совпадений
      Command32(2,chan,28,(int) 0); // тип обработки повторных
      int red = cpar.RD[chan]-1;
      if (red<0) red=0;
      Command32(2,chan,29,(int) red); // величина пересчета P
      Command32(2,chan,30,200); // максимальное расстояние для дискриминатора типа 3: L (=200)
      Command32(2,chan,31,(int) mask_group); //битовая маска принадлежности к группам
      Command32(2,chan,34,cpar.F24); // разрядность форматирования отсчетов сигнала
    } // if (cpar.on[chan])
    else {
      Command32(2,chan,25,0); // битовая маска разрешений записи = 0
    }
  } //for

  //Общие параметры:

  Command32(11,1,0,cpar.Smpl);            // Sampling rate
  Command32(11,4,0,(int) hard_logic); // Использование схемы совпадений
  Command32(11,5,0,(int) cpar.mult_w1[0]); // минимальная множественность 0
  Command32(11,6,0,(int) cpar.mult_w2[0]); // максимальная множественность 0
  Command32(11,7,0,(int) cpar.mult_w1[1]); // минимальная множественность 1
  Command32(11,8,0,(int) cpar.mult_w2[1]); // максимальная множественность 1
  Command32(11,9,0,(int) 0); // регистрация перенапряжения

} //AllParameters44()

void CRS::AllParameters36() {
  //cout << "AllParameters36(): " << endl;

  UInt_t mask;

  //enable start channel by default
  Command32(2,255,11,1); //enabled

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.on[chan]=false;

    Command32(2,chan,11,(int) cpar.on[chan]); //enabled

    if (cpar.on[chan]) {
      Command32(2,chan,0,(int)cpar.AC[chan]);
      Command32(2,chan,1,(int)cpar.Inv[chan]);
      // hS - see below
      Command32(2,chan,3,(int)cpar.Dt[chan]);
      Command32(2,chan,4,(int)-cpar.Pre[chan]);
      Command32(2,chan,5,(int)cpar.Len[chan]);
      Command32(2,chan,6,(int)cpar.Drv[chan]);
      Command32(2,chan,7,(int)cpar.Thr[chan]);
      Command32(2,chan,8,(int)cpar.G[chan]);
      // new commands
      Command32(2,chan,9,(int)cpar.hD[chan]); //delay
      //Command32(2,chan,9,0); //delay
      Command32(2,chan,10,0); //test signal is off

      Int_t trg2=cpar.Trg[chan];
      Int_t cntr=0; //тип вычисления центроида (0,1)
      if (trg2==5) {
	trg2=2; //для триг5 устанавливаем триг2
	cntr=1; //и тип центроида 1
      }
      else if (trg2==6)
	trg2=4; //для триг6 устанавливаем триг4

      Command32(2,chan,12,(int) trg2); //trigger type

      Check33(13,chan,opt.Base1[chan],opt.Base2[chan],1,4095);
      Check33(15,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(17,chan,opt.Peak1[chan],opt.Peak2[chan],1,4095);
      Check33(19,chan,opt.T1[chan],opt.T2[chan],1,4095);
      Check33(21,chan,opt.W1[chan],opt.W2[chan],1,4095);


      mask=0b1100000000011; //bits 0,1 (было еще 11,12)
      if (opt.dsp[chan]) {
	mask|=0b11101110000; // write DSP data
      }

      if (cpar.pls[chan]) { //add 1100
	mask|=0b1100; // write pulse
      }

      Command32(2,chan,23,mask & cpar.RMask); //bitmask for discriminator
      Command32(2,chan,32,cntr); //type of centroid

      int pwin = opt.Peak2[chan]; //окно повторных срабатываний
      if (pwin<0) pwin=0;
      if (pwin>4095) pwin=4095;
      Command32(2,chan,33,pwin); //окно повторных срабатываний

      Command32(2,chan,35,(int)cpar.LT[chan]); //нижний порог дискр.(3,4) = 0

      //сглаживание
      Int_t sum=cpar.hS[chan];
      if (sum<=0) sum=1;

      Int_t kk=cpar.hS[chan]-1;
      UInt_t div=0;
      if (kk>0) {
	while (kk >>= 1) {
	  div++;
	}
	div++;
      }

      //prnt("ss d d ds;",BGRN,"sum2:",chan,sum,div,RST);
      Command32(2,chan,36,sum); //сглаживание: сумма
      Command32(2,chan,2,div); //сглаживание: деление

    } //if
  } //for

  // Start dead time DT
  if (cpar.DTW<=0) cpar.DTW=1;
  UChar_t type = cpar.DTW>>24;

  //Start source
  int st_src=cpar.St_Per ? 1 : 0;

  //Start imitator period
  int sprd=cpar.St_Per;
  if (sprd) sprd--;

  Command32(11,0,type,(UInt_t) cpar.DTW); // Start dead time DTW
  Command32(11,2,0,st_src);               // Start source
  Command32(11,3,0,sprd);                 // Start imitator period
  
}

void CRS::AllParameters35() {
  UInt_t mask;

  Int_t s_Trg[MAX_CHTP];
  Int_t drv[MAX_CHTP];

  for (int i=0;i<chan_in_module;i++) {
    s_Trg[i] = cpar.Trg[i];
    if (cpar.Trg[i]==5) {
      cpar.Trg[i]=2;
      drv[i]=1;
    }
    else {
      drv[i]=0;
    }
  }

  AllParameters33();

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {

      mask=0b1100000000011; //bits 0,1,11,12
      if (opt.dsp[chan]) {
	mask|=0b11101110000; // write DSP data
      }

      if (cpar.pls[chan]) { //add 1100
	mask|=0b1100; // write pulse
      }

      Command32(2,chan,23,mask & cpar.RMask); //bitmask for discriminator
      Command32(2,chan,32,drv[chan]); //type of centroid

      int pwin = opt.Peak2[chan]; //окно повторных срабатываний
      if (pwin<0) pwin=0;
      if (pwin>4095) pwin=4095;
      Command32(2,chan,33,pwin); //окно повторных срабатываний

    } //if
  } //for

  // Start dead time DT
  if (cpar.DTW<=0) cpar.DTW=1;
  UChar_t type = cpar.DTW>>24;

  //Start source
  int st_src=cpar.St_Per ? 1 : 0;

  //Start imitator period
  int sprd=cpar.St_Per;
  if (sprd) sprd--;

  Command32(11,0,type,(UInt_t) cpar.DTW); // Start dead time DTW
  Command32(11,2,0,st_src);               // Start source
  Command32(11,3,0,sprd);                 // Start imitator period

  for (int i=0;i<chan_in_module;i++) {
    cpar.Trg[i] = s_Trg[i];
  }

}

void CRS::AllParameters34()
{
  //cout << "AllParameters34(): " << endl;

  AllParameters33();
  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {
      //cpar.Mask[chan]=0xFF;
      UInt_t mask=0xC3; //11000011 - write tst,count40,count48,overflow
      if (opt.dsp[chan]) {//add 110000
	mask|=0x30; // write DSP data
      }
      if (cpar.pls[chan]) {//add 1100
	mask|=0xC; // write pulse
      }
      Command32(2,chan,23,mask & cpar.RMask); //bitmask
      //UInt_t mask = 0xFF;
      //Command32(2,chan,23,(UInt_t) mask); //bitmask
    }
  } //for

  // Start dead time DT
  if (cpar.DTW==0) cpar.DTW=1;
  UChar_t type = cpar.DTW>>24;
  Command32(11,0,type,(UInt_t) cpar.DTW);

}

void CRS::AllParameters33()
{
  //cout << "AllParameters33(): " << endl;

  //enable start channel by default
  Command32(2,255,11,1); //enabled

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.on[chan]=false;
    Command32(2,chan,11,(int) cpar.on[chan]); //enabled

    if (cpar.on[chan]) {
      Command32(2,chan,0,(int)cpar.AC[chan]);
      Command32(2,chan,1,(int)cpar.Inv[chan]);
      Command32(2,chan,2,(int)cpar.hS[chan]);
      Command32(2,chan,3,(int)cpar.Dt[chan]);
      Command32(2,chan,4,(int)-cpar.Pre[chan]);
      Command32(2,chan,5,(int)cpar.Len[chan]);
      Command32(2,chan,6,(int)cpar.Drv[chan]);
      Command32(2,chan,7,(int)cpar.Thr[chan]);
      Command32(2,chan,8,(int)cpar.G[chan]);
      // new commands
      Command32(2,chan,9,(int)cpar.hD[chan]); //delay
      //Command32(2,chan,9,0); //delay
      Command32(2,chan,10,0); //test signal is off

      Int_t trg2=cpar.Trg[chan];
      if (trg2>5) trg2=4; //для триг6 устанавливаем триг4
      Command32(2,chan,12,(int) trg2); //trigger type

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

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan>=opt.Nchan)
      cpar.on[chan]=false;
    Command32(2,chan,0,(int)cpar.AC[chan]);
    Command32(2,chan,1,(int)cpar.Inv[chan]);
    Command32(2,chan,2,(int)cpar.hS[chan]);
    Command32(2,chan,3,(int)cpar.Dt[chan]);
    Command32(2,chan,4,(int)cpar.Pre[chan]);
    Command32(2,chan,5,(int)cpar.Len[chan]);
    Command32(2,chan,6,(int)cpar.Drv[chan]);
    Command32(2,chan,7,(int)cpar.Thr[chan]);
    Command32(2,chan,8,(int)cpar.G[chan]);
    // new commands
    Command32(2,chan,9,0); //delay
    //Command32(2,chan,10,0); //test signal
    Command32(2,chan,11,(int) cpar.on[chan]); //enabled
    //Command32(2,chan,11,1); //enabled
  }

}

void CRS::AllParameters2()
{
  //cout << "AllParameters2(): " << endl;
  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    Command2(2,chan,0,(int)cpar.G[chan]);
    Command2(2,chan,1,(int)cpar.Inv[chan]);
    Command2(2,chan,2,(int)cpar.hS[chan]);
    if (cpar.on[chan]) {
      Command2(2,chan,3,(int)cpar.Drv[chan]);
      Command2(2,chan,4,(int)cpar.Thr[chan]);
    }
    else { // запрет срабатывания высоким порогом
      int tmp,max;
      cpar.GetParm("thresh",chan,cpar.Thr,tmp,max);
      //cpar.GetPar("thresh",module,chan,crs_ch[chan],tmp,tmp,max);
      //cout << "Off: " << (int) chan << " " << max << endl;
      Command2(2,chan,3,0);
      Command2(2,chan,4,max);
    }

    Command2(2,chan,5,(int)cpar.Pre[chan]);
    Command2(2,chan,6,(int)cpar.Len[chan]);
  }

  Command2(2,0,7,(int)cpar.forcewr);

}

int CRS::DoStartStop(int rst) {
  //rst=1 - reset timestamp; 0 - do not reset

  if (f_read) {
    gzclose(f_read);
    f_read=0;
  }
	
  if (!b_acq) { //start

    //cout << "start: " << rst << endl;
    crs->Free_Transfer();
#ifdef P_LIBUSB
      prnt("sss;",BYEL,"Sleep 50",RST);
#endif
    gSystem->Sleep(50);
    //cout << "Free_Transfer() finished" << endl;

    InitBuf();
    //cout << "InitBuf() finished" << endl;
    crs->Init_Transfer();
    //cout << "Init_Transfer() finished" << endl;

    DoReset(rst);
#ifdef P_LIBUSB
    cout << "DoReset() finished" << endl;
#endif
    if (rst) {
      HiFrm->DoRst();
    }
    //juststarted=true; already set in doreset

    TCanvas *cv=0;
    
    if (!batch) {
      parpar->Update();
      chanpar->Update();

      EvtFrm->Clear();
      EvtFrm->Pevents = &EvtFrm->Tevents;

      cv=EvtFrm->fCanvas->GetCanvas();
      cv->SetEditable(false);
      myM->UpdateTimer(rst);
      //myM->UpdateStatus(rst);
    }

    //if (module==32) {
    //Command32(7,0,0,0); //reset usb command
    //}

    /* YK 25.12.24
    // YK 29.09.20
#ifdef P_LIBUSB
    int r=cyusb_reset_device(cy_handle);
    prnt("ssds;",BBLU,"cyusb_reset: ",r,RST);
#else
    // YK 29.09.20
    cyusb_reset_device(cy_handle);
#endif
    */

    if (rst && crs->module>=32 && crs->module<=70) {
      Command32(8,0,0,0); //сброс сч./буф.
      //Command32(9,0,0,0); //сброс времени
    }

#ifdef P_LIBUSB
    auto t1 = std::chrono::system_clock::now();
    auto t2 = std::chrono::system_clock::now();
    std::time_t t_t1 = std::chrono::system_clock::to_time_t(t1);
    std::time_t t_t2 = std::chrono::system_clock::to_time_t(t2);
    Submit_all(ntrans);
    cout << "Submit_all() finished: " << std::ctime(&t_t1) << " " << std::ctime(&t_t2) << endl;
#else
    Submit_all(ntrans);
#endif

    //cout << "Period: " << opt.Period << endl;
    //opt.Period=5; //5ns for CRS module

    if (SetPar()) {
      return 3;
    }

    //buf_out[0]=3;
    b_acq=true;
    b_stop=false;
    //bstart=true;


    //Nsamp=0;
    //nsmp=0;

    cpar.F_start = gSystem->Now();
    Text_time("S:",cpar.F_start);

    if (rst) {
      if (opt.raw_write) {
	Reset_Raw();
      }   
      if (opt.dec_write) {
	Reset_Dec(79);
      }
      if (opt.fTxt) {
	Reset_Txt();
      }
    }

    //cout << "Acquisition started" << endl;
    //gettimeofday(&t_start,NULL);


    //InitBuf();

    ProcessCrs(rst);
    //ProcessCrs_old();


    //cout << "startstop1: " << endl;
    if (!batch) {
      EvtFrm->Clear();
      EvtFrm->Pevents = &Levents;
      EvtFrm->d_event=--EvtFrm->Pevents->end();
      //cout << "startstop2: " << endl;

      //gSystem->Sleep(opt.tsleep);   
      gSystem->Sleep(10);   
      Show(true);
      cv->SetEditable(true);
    }
  } //start
  else { //stop
    //buf_out[0]=4;
    cout << "Acquisition stopped" << endl;

    Command2(4,0,0,0);
#ifdef P_LIBUSB
    prnt("sss;",BYEL,"Sleep 300",RST);
#endif
    prnt("sss;",BYEL,"TODO: Sleep 300",RST);
    gSystem->Sleep(300); //1300 - проблема 3 в АК-32 устраняется
    b_acq=false;

    //cout << "Sleep(13000)" << endl;

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

void CRS::ProcessCrs(int rst) {
  b_run=1;
  Ana_start();

  //prnt("ssf d ls;",BBLU,"T_acq: ",opt.T_acq,crs->module,crs->Tstart64,RST);;
  //decode_thread_run=1;
  //tt1[3].Set();


  if (rst && crs->module>=32 && crs->module<=70) {
    //Command32(8,0,0,0); //сброс сч./буф.
    Command32(9,0,0,0); //сброс времени
  }
  Command2(3,0,0,0);

#ifdef P_TEST
  if (myM->test) {
    EndAna(1);
    return;
  }
#endif //P_TEST

  while (!crs->b_stop) {
    if (!batch) {
      Show();
    }
    else {
      ;
    }
    gSystem->Sleep(10);   
    gSystem->ProcessEvents();
    if (opt.Tstop && opt.T_acq>opt.Tstop) {
      //prnt("ssf f ls;",BRED,"Stop1!!!: ",opt.Tstop,opt.T_acq,crs->Tstart64,RST);;
      DoStartStop(0);
      //cout << "Stop2!!!" << endl;
    }
  }

  // cout << "stopped:" << endl;
  EndAna(1);

}

#endif //CYUSB

void CRS::Text_time(const char* hd, Long64_t f_time) {
  //convert btw gSystem->Now and time_t
  time_t tt = (cpar.F_start+788907600000)*0.001;
  struct tm *ptm = localtime(&tt);
  char ttt[100];
  strftime(ttt,sizeof(ttt),"%F %T",ptm);
  strcpy(txt_start,hd);
  strcat(txt_start,ttt);
}

void CRS::DoExit()
{
  cout << "CRS::DoExit" << endl;

  event_thread_run=0;
#ifdef CYUSB
  if (Fmode==1) {
    cyusb_close();
    cy_handle=0;
    if (trd_crs) {
      trd_crs->Delete();
      trd_crs=0;
    }
  }
#endif

}

void CRS::DoReset(int rst) {
  //rst=0 - "Continue"; rst=1 - "Start"

  if (!b_stop) return;

  if (EvtFrm) {
    EvtFrm->DoReset();
  }

  Levents.clear();

  N4=0;
  LMAX=0;
  SLP=0;

  // вставляем dum евент, чтобы Levents не был пустой  // не делаем, это плохо
  //m_event=Levents.insert(Levents.end(),EventClass());
  m_event=Levents.end();
  //m_end=Levents.end();
  //m_event=Levents.begin();

  Bufevents.clear();

  if (rst) {
    opt.T_acq=0;
    Tstart64=0;
    Offset64=0;
    //Tstart0=0;
    //Time0=0;

    //Pstamp64=P64_0;
    Pstamp64=0;

    nevents=0;
    mtrig=0;

    inputbytes=0;
    rawbytes=0;
    decbytes=0;

    npulses=0;
    nbuffers=0;
    memset(npulses2,0,sizeof(npulses2));
    memset(npulses2o,0,sizeof(npulses2));
    memset(npulses3o,0,sizeof(npulses3o));
    memset(Tst3o,0,sizeof(Tst3o));
    memset(rate_soft,0,sizeof(rate_soft));
    memset(rate_hard,0,sizeof(rate_hard));
    memset(npulses_bad,0,sizeof(npulses_bad));

    memset(fCounter,0,sizeof(fCounter));
    memset(fTime,0,sizeof(fTime));

    memset(errors,0,sizeof(errors));

    //if (f_read)
    if (Fmode==2)
      DoFopen(NULL,0,0); //reopen file; don't read cpar & opt
    juststarted=true;

    if (chanpar) {
      chanpar->UpdateStatus(rst);
    }

    for (auto i=0;i<opt.Nchan;i++) {
      if (opt.Mt[i]>=2) {
	hcl->b_base[i]=1;
      }
    }
  }

#ifdef TIMES
  memset(ttm,0,sizeof(ttm));
#endif
}

int CRS::DoFopen(char* oname, int copt, int popt) {
  //oname=null -> reopen the same file; !=null -> open new file 
  //copt: 1 - read cpar from file; 0 - don't read cpar from file
  //popt: 1 - read opt from file; 0 - don't read opt from file
  //return 0 - OK; 1 - error

  int tp=0; //1 - adcm raw; 0 - crs2/32/dec; 7? - Ortec Lis
  //module=0; //нужно для b_noheader

  if (oname)
    strcpy(Fname,oname);

  // if (TString(Fname).EqualTo(" ",TString::kIgnoreCase)) {
  //   return;
  // }

  string dir, name, ext2;
  SplitFilename(string(Fname),dir,name,ext2);
  TString ext(ext2);
  ext.ToLower();

  Tstart64=0;
  Offset64=0;

  //cout << "DoFopen: " << Fname << " " << name << endl;

  if (name.compare("17")==0) {
    module=17;

    //SimulateInit();
    sim2 = new Simul();

    Fmode=2;
    InitBuf();

    strcpy(mainname,Fname);
    if (myM) {
      myM->SetTitle(Fname);
      //daqpar->AllEnabled(false);
    }

    return 0;
    
  }

  if (ext.EqualTo(".dat")) {
    //determine file date/time and Tstart64
    int res = Detect_adcm();

    if (res==0) {
      prnt("ssss;",BRED,"Can't open file: ",Fname,RST);
      //f_read=0;
      return 1;
    }
    else if (res==2) {
      prnt("ssss;",BRED,"Unknown file format / can't find Tstamp in adcm/raw file: ",Fname,RST);
      //f_read=0;
      return 1;
    }
    else if (res==1) {
      prnt("ssss;",BYEL,"ADCM RAW File: ",Fname,RST);
    }
    else if (res==3) {
      prnt("ssss;",BYEL,"ADCM DEC File: ",Fname,RST);
    }

    module=res; //1 - adcm raw; 3 - adcm dec
    cpar.InitPar(0);
    opt.Period=opt.adcm_period;

    tp=1;
  }
  else if (ext.EqualTo(".lis")) {
    tp=2;
  }
  else if (ext.EqualTo(".gz")) {
    tp=0;
  }
  else if (ext.EqualTo(".raw")) {
    tp=0;
  }
  else if (ext.EqualTo(".dec")) {
    tp=0;
  }
  else {
    prnt("sss;",BRED,"Unknown file type (extension): ",ext.Data());
    prnt("ss;","Allowed extensions: .root, .dat, .raw, .dec, .gz",RST);
    if (f_read) gzclose(f_read);
    f_read=0;
    return 1;
  }

  if (f_read) gzclose(f_read);
  f_read = gzopen(Fname,"rb");
  if (!f_read) {
    cout << "Can't open file: " << Fname << endl;
    f_read=0;
    return 1;
  }

  if (b_noheader) { //не читаем заголовок
  }
  else { //иначе читаем
    if (tp==0) { //crs32 or crs2 or dec
      if (ReadParGz(f_read,Fname,1,copt,popt)) {
	gzclose(f_read);
	f_read=0;
	return 1;
      }
      cout << "opt.Period from file: " << opt.Period << endl;
      cout << "Git version from file: " << opt.gitver << endl;

      // update Dsp/dsp if file version is earlier than v0.870
      // только если файл открывается впервые (oname!=0)
      if (oname && string(opt.gitver).compare("v0.892")<0) {
	memcpy(opt.Dsp,opt.dsp,sizeof(opt.dsp));
      }
  
    }
    else if (tp==2) { //Ortec Lis
      //cout << "Ortec Lis File: " << Fname << endl;
      module=3;
      cpar.InitPar(0);
      opt.Period=200;

      char header[256];
      gzread(f_read,header,256);
    }
  } //читаем заголовок

  Fmode=2;
  InitBuf();

  // if (tp==1) { //adcm raw - determine durwr
  //   // cout << "durwr1: " << nvp << " " << vv2->size() << endl;
  //   // DoBuf();
  //   // cout << "durwr2: " << nvp << " " << vv2->size() << endl;
  // }

  strcpy(mainname,Fname);
  if (myM) {
    myM->EnableBut(myM->fGr1,0);
    if (myM->local_nch!=opt.Nchan || myM->local_nrows!=opt.Nrows) {
      myM->Rebuild();
      myM->local_nch=opt.Nchan;
      myM->local_nrows=opt.Nrows;
    }

    myM->SetTitle(Fname);
    //daqpar->AllEnabled(false);
  }

  prnt("ss s s s ds;",BGRN,"File:",Fname,
       cpar.GetDevice(module).c_str(),"Module:",module,RST);

  return 0;
} //DoFopen

int CRS::ReadParGz(gzFile &ff, char* pname, int m1, int cp, int op) {
  //m1 - read module (1/0) (читается только при открытии файла)
  //cp - read cpar (1/0) (читается всегда =1)
  //op - read opt (1/0) (не читается при открытии файла open- reopen 

  // 2 bytes: fmt - формат par файла
  //        if (fmt>=129) { //новый формат 129
  //           fmt - определяет номер формата записи par файла
  //           2 bytes - module (Short_t)
  //           4 bytes - sz (Int_t)
  //           sz bytes - buf
  //        }
  //        else { //старый формат
  //           fmt=module (Short_t)
  //           2 bytes - sz (UShort_t)
  //           sz bytes - buf
  //        }

  // return 0 - OK; 1 - error
  int res=0;

  UShort_t fmt, mod;
  Int_t sz=0;

  char buf[500000];

  //prtime("ReadParGz1");

  gzread(ff,&fmt,sizeof(Short_t));
  if (fmt==129) {
    gzread(ff,&mod,sizeof(Short_t));
    gzread(ff,&sz,sizeof(Int_t));
  }
  else {
    mod=fmt;
    fmt=129;
    gzread(ff,&sz,sizeof(UShort_t));
  }

  //prnt("sss d sd sd sds;",BGRN,"rpgz: ",pname,fmt,"mod=",mod,"module=",module,"sz=",sz,RST);

  //cout << "mod: " << mod << " " << fmt << " " << sz << endl;
  if (fmt!=129 || mod>100 || sz>5e5){//возможно, это текстовый файл
    //или старый файл без параметров
    prnt("sss d d ds;",BRED,"Header not found: ",pname,fmt,mod,sz,RST);
    return 1;
    
    /*
    prnt("sss d d ds;",BRED,"Header not found. Assuming module=2: ",pname,fmt,mod,sz,RST);
    if (f_read) gzclose(f_read);
    f_read = gzopen(Fname,"rb");
    if (!f_read) {
      cout << "Can't open file: " << Fname << endl;
      f_read=0;
    }
    module=22;
    return 0;
    */    
  }
  //cout << "ReadParGz: "<<pname<<" "<<sz<<" "<<m1<<" "<<cp<<" "<<op<<endl;

  //char* buf = new char[sz];
  gzread(ff,buf,sz);

  MakeVarList(cp,op);
  int ret=0;
  ret=FindVar(buf,sz,"gitver",opt.gitver);
  if (!ret) {
    memset(opt.gitver,0,sizeof(opt.gitver));
  }

  ret=BufToClass(buf,buf+sz,op);
  if (!ret) {
    prnt("ssss;",BRED,"Warning: error reading parameters: ",pname,RST);
    gSystem->Sleep(1000);
    res=1;
  }
  //exit(1);

  //correct "other" ch_type, if needed
  for (int i=0;i<MAX_CHTP;i++) {
    if (opt.chtype[i]>=MAX_TP+1) {
      opt.chtype[i]=MAX_TP+1;// other; was =1;
    }
  }

  //strcpy(opt.gitver,"v0.870");
  //int cm = string(opt.gitver).compare("v0.870");
  //cout << "GGG: " << opt.gitver << " " << cm << endl;

  Make_prof_ch();
  Text_time("S:",cpar.F_start);

  if (m1) {
    if (mod==2 || mod==22) {
      module=22;
      //cout << "CRS2 File: " << Fname << " " << module << endl;
    }
    else if (mod>=32) {
      module=mod;
      //cout << "CRS32 or decoded File: " << Fname << " " << module << endl;
    }
    else {
      Fmode=0;
      //cout << "Unknown file type: " << Fname << " " << mod << endl;
      res=1;
    }
  }

  for (int i=0;i<MAX_CH;i++) {
    cpar.Len[i]=cpar.ChkLen(i,module);
  }

  //Set_Trigger();

  if (op) {
    opt.raw_write=false;
    opt.dec_write=false;
    opt.root_write=false;
  }

  if (HiFrm && op) {
    //cout << "HiFrm0:" << endl;
    histpar->AddHist_2d();
    HiFrm->HiReset();
  }



  /*
  //F_start test
  Long64_t fstart=0;
  time_t tt=0;
  std::string str(buf,sz);
  size_t pos=str.find("F_start");
  if (pos!=std::string::npos) {
    char* buf2 = buf+pos;
    buf2 += strlen(buf2)+1+sizeof(short);
    fstart = *(Long64_t*) buf2;

    char txt[100];
    tt = (fstart+788907600000)*0.001;
    struct tm *ptm = localtime(&tt);
    strftime(txt,sizeof(txt),"%F %T",ptm);

    cout << "F_start: " << " " << txt << " " << fstart << " " << tt << endl;
  }
  else {
    cout << "F_start not found" << endl;
  }

  // cout << "F_start: " << opt.F_start << " " << fstart << endl;
  // opt.F_start = gSystem->Now();
  // cout << "F_start2: " << opt.F_start << endl;

  //end of F_start test
  */

  return res;
} //ReadParGz

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

  /*
  TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
  TIter nextd(lst);
  TDataMember *dm;
  char* popt = (char*)&opt;
  while ((dm = (TDataMember *) nextd())) {
    if (dm->GetDataType()==0 && TString(dm->GetName()).Contains("h_")) {
      cout << "member: " << dm->GetName() << " " << dm->GetDataType()
	   << " " << hex << (ULong64_t) popt+dm->GetOffset() << dec << endl;
      sz+=ClassToBuf("Hdef",dm->GetName(),popt+dm->GetOffset(),buf+sz);
    }
  }
*/

  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    //prnt("ss s ss;",BGRN,"MLst:",it->name.Data(),it->h_name.Data(),RST);
    //cout << it->hd << endl;    
    sz+=ClassToBuf("Hdef",it->h_name.Data(),(char*)it->hd,buf+sz);
  }

  //sz/=8;
  //sz=(sz+1)*8;
  sz = (sz/8+1)*8; //длина всегда кратна 8

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
      prnt("ssd d ds;",BRED,"Profilometer: chan is already used: ",
	   nn,aa[nn],off,RST);
    //cout << "Profilometer: chan " << aa[nn] << " is already used:" << endl; 
  }
}

void CRS::Make_prof_ch() {
  for (int i=0;i<MAX_CH;i++) {
    prof_ch[i]=-1;
    //cout << "prof_ch: " << i << " " << prof_ch[i] << endl;
  }

  if (opt.Prof_type==64) { //new 64x64
    for (int i=0;i<5;i++) {
      DoProf(i,opt.Prof64,PROF_64);
    }
  }
  else { //old 8x8
    for (int i=0;i<8;i++) {
      DoProf(i,opt.Prof_x,PROF_X);
      DoProf(i,opt.Prof_y,PROF_Y);
    }
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
    Long64_t end0 = b_end[gl_ibuf];
    FindLast(gl_ibuf, loc_ibuf, 8);
    Long64_t sz = end0 - b_end[gl_ibuf];
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

  //if (opt.decode) {
  if (!opt.directraw) {
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

  // cout << "Sleep... " << 500 << endl;
  // gSystem->Sleep(500);


  if (Fmode>1) { //только для чтения файла
    if (N4>3) {
      SLP+=5;
      ++errors[ER_ANA]; //slow analysis
    }
    else {
      if (SLP>0)
	SLP-=5;
    }
    if (SLP>0)
      gSystem->Sleep(SLP);
  }

  /*
  if (batch && scrn) {
    //++nn;
    if (nbuffers%scrn==0) {
      prnt("sls0.2fs0.1fs0.1fsd;",
      	   "Buf: ", nbuffers, "  Dec. MB: ",
      	   inputbytes/MB, "  T(s): ",int(opt.T_acq*10)*0.1,
      	   " %mem: ", CheckMem()/10.0, " slp: ", SLP);
    }
  }
  */
}

int CRS::DoBuf() {

  //cout << "gzread0: " << Fmode << " " << nbuffers << " " << opt.rbuf_size*1024 << " " << gl_ibuf << " " << dec_iread[gl_ibuf] << endl;
#ifdef TIMES
  tt1[0].Set();
#endif
  Long64_t length=0;

  if (scrn) {
    //++nn;
    if (nbuffers%scrn==0) {
      prnt("sls0.2fs0.1fs0.1fsd;",
      	   "Buf: ", nbuffers, "  Dec. MB: ",
      	   inputbytes/MB, "  T(s): ",int(opt.T_acq*10)*0.1,
      	   " %mem: ", CheckMem()/10.0, " slp: ", SLP);
    }
  }

  if (module==17) {

    sim2->SimulateEvents(opt.ev_max,nbuffers);
    //prnt("ss l ls;",BRED,"17:",nbuffers,nevents,RST);
    //gSystem->Sleep(100);
    
    crs->Ana2(0);

    nbuffers++;
    //return nbuffers;

  }
  else {
    while (dec_iread[gl_ibuf])
      gSystem->Sleep(1);

    length=gzread(f_read,GLBuf+b_fill[gl_ibuf],opt.rbuf_size*1024);
    b_end[gl_ibuf]=b_fill[gl_ibuf]+length;

    if (opt.raw_write && !opt.fProc) {
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

    if (length>0) {
      AnaBuf(1); // YK (1 - loc_ibuf, fake number)
      nbuffers++;
      inputbytes+=length;
      //return nbuffers;
    }
    else {
      return 0;
    }
  }

  return nbuffers;
}

// int CRS::CountChan() {
//   int res=0;
//   for (int i=0;i<opt.Nchan;i++) {
//     if (cpar.on[i]) res++;
//   }
//   return res;
// }

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
    if (opt.usb_size<=2048) {
      tr_size=opt.usb_size*1024;
    }
    else {
      tr_size=2048*1024;
    }
    //MAXTRANS2=MAXTRANS;

    gl_sz = opt.usb_size;
    gl_sz *= 1024*MAXTRANS*gl_Nbuf;
    //cout << "gl_sz: " << opt.usb_size << " " << gl_sz/MB << " MB" << endl;
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

  //cout << "GLBuf size: " << (gl_sz+gl_off)/1024 << " kB" << endl;
  //cout << "GLBuf size2: " << gl_sz << " " << gl_off << endl;

}

void CRS::StopThreads(int all) {

  //cout << "deleting threads... ";
  decode_thread_run=0;    
  for (UInt_t i=0;i<gl_Nbuf;i++) {
    dec_iread[i]=1; //=1;
    // dec_cond[i].Signal();
    if (trd_dec[i]) {
      trd_dec[i]->Join();
      trd_dec[i]->Delete();
      trd_dec[i]=0;
    }
    //dec_finished[i]=1;
  }

  gSystem->Sleep(50);
  mkev_thread_run=0;    

  if (trd_mkev) {
    trd_mkev->Join();
    trd_mkev->Delete();
    trd_mkev=0;
  }

  gSystem->Sleep(50);
  //cout << "StopThreads: " << endl;
  ana_thread_run=0;
  ana_all=all;
  //ana_check=1;
  //ana_cond.Signal();
  if (trd_ana) {
    trd_ana->Join();
    trd_ana->Delete();
    trd_ana=0;
  }

  //gSystem->Sleep(500);
  wrt_thread_run=0;
  if (trd_dec_write) {
    trd_dec_write->Join();
    trd_dec_write->Delete();
    trd_dec_write=0;
  }

  if (trd_raw_write) {
    trd_raw_write->Join();
    trd_raw_write->Delete();
    trd_raw_write=0;
  }

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

  if (HiFrm && opt.b_fpeaks && opt.Peak_print)
    HiFrm->pkprint=true;

  // cout << "EndAna: " << endl;
  // for (int i=0;i<opt.Nchan;i++) {
  //   cout << "Counts: " << npulses2[i] << " " << npulses3[i] << endl;
  // }

  //new (06.02.2020)
  //removed (04.08.2020)
  // if (opt.dec_write) {
  //   crs->Flush_Dec();
  // }
  // if (opt.raw_write && opt.fProc) {
  //   crs->Flush_Raw();
  // }

  if (!batch) {
    parpar->DaqEnable();
    histpar->DaqEnable();
  }

#ifdef TPROC
  prnt("ss fs;",BGRN,"tproc:",tproc,RST);
#endif
}

void CRS::FAnalyze2(bool nobatch) {

  if (gzeof(f_read)) {
    cout << "Enf of file: " << endl;
    //Ana2(1);
    if (nobatch)
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
    if (opt.fTxt) {
      Reset_Txt();
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

  Ana_start();
  //int res;
  while (crs->b_fana) {
    int res=crs->DoBuf();
    if (!res) break;
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
    if (opt.fTxt) {
      Reset_Txt();
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

  if (CheckMem()>=700) { //>70%
    cout << "Memory is too low. Exitting... " << pinfo.fMemResident*1e-3 << " " << minfo.fMemTotal << endl;

#ifdef CYUSB
    DoStartStop(0);
    // gSystem->Sleep(500);

    // cout << "Terminate..." << endl;
    // exit(-1);
    if (trd_crs) {
      trd_crs->Delete();
      trd_crs=0;
    }
#endif
	
    StopThreads(0);

    gApplication->Terminate(0);
    delete myM;
  }

  tm2=gSystem->Now();
  //cout << "Levents7: " << Levents.size() << " " << tm2 << " " << tm2-tm1 << endl;
  if (tm2-tm1>opt.tsleep || force) {

    // cout << "\033[35m";
    // cout << "Show: " << tm2-tm1 << " " << force << endl;
    // cout << "\033[0m";

    tm1=tm2;
    //cout << "show... " << opt.T_acq << " " << Levents.size() << " " << &*m_event << " " << m_event->Nevt << endl;

    if (myM) {
      if (myM->fTab->GetCurrent()==EvtFrm->ntab ||
	  EvtFrm->fDock->GetUndocked()) {

	EvtFrm->fCanvas->GetCanvas()->SetEditable(true);
	EvtFrm->DrawEvent2();
	EvtFrm->fCanvas->GetCanvas()->SetEditable(false);

      }
      if (myM->fTab->GetCurrent()==HiFrm->ntab || HiFrm->fDock->GetUndocked()) {
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
    chanpar->UpdateStatus();
    //myM->UpdateStatus();
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
  case 51:
    //Decode34(dec_iread[ibuf]-1,ibuf);
    //break;
  case 42:
  case 52:
    Decode34(dec_iread[ibuf]-1,ibuf);
    //Decode42(dec_iread[ibuf]-1,ibuf);
    break;
  case 35:
  case 43:
  case 53:
  case 36:
  case 44:
  case 54:
  case 45:
    Decode35(dec_iread[ibuf]-1,ibuf);
    //Decode42(dec_iread[ibuf]-1,ibuf);
    break;
  case 80:
    Decode80(dec_iread[ibuf]-1,ibuf);
    break;
  case 79:
    //decoder->Decode79(dec_iread[ibuf]-1,ibuf);
    //cout << "Decode79: " << f_read << " " << *decoder->zfile << endl;
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
  case 3:
    Decode_adcm_dec(dec_iread[ibuf]-1,ibuf);
    break;
  default:
    eventlist *Blist;
    Dec_Init(Blist,0);
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

  Long64_t sss=-1;
  //bool found=false;
  UChar_t lflag;

  switch (module) {
  case 1: //adcm
    for (Long64_t i=b_end[ibuf]-4;i>=b_start[ibuf];i-=4) {
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
	    //prnt("ssd d l ls;",BGRN,"FindLast: ",ibuf,ibuf2,b_start[ibuf2],
	    //b_end[ibuf],RST);
	    return;
	  }
	} //if sss>0
	sss=i;
      } //if frmt
    } //for i
    cout << "1: Error: no last event: maybe USB buffer is too small: " << ibuf << endl;
    break;
  case 3: //adcm_dec
    for (Long64_t i=b_end[ibuf]-2;i>=b_start[ibuf];i-=2) {
      frmt = *((UShort_t*) (GLBuf+i));
      if (frmt==ID_EVNT) {
	b_end[ibuf]=i;
	b_start[ibuf2]=i;
	return;
      }
    } //for i
    cout << "1: Error: no last event: maybe USB buffer is too small: " << ibuf << endl;
    break;
  case 22:
    for (Long64_t i=b_end[ibuf]-2;i>=b_start[ibuf];i-=2) {
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
  case 35:
  case 36:
  case 41:
  case 42:
  case 43:
  case 44:
  case 51:
  case 52:
  case 53:
  case 54:
  case 45:
    for (Long64_t i=b_end[ibuf]-8;i>=b_start[ibuf];i-=8) {
      //find frmt==0 -> this is the start of a pulse
      frmt = (GLBuf[i+6] & 0xF0);
      if (frmt==0) {
	//Long64_t len = b_end[ibuf]-i;
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
  case 80:
    for (Long64_t i=b_end[ibuf]-8;i>=b_start[ibuf];i-=8) {
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

void CRS::CheckDSP(PulseClass &ipls, PulseClass &ipls2) {
  //cout << "checkdsp: " << ipls.sData.size() << " " << ipls2.sData.size() << endl;
  bool bad=false;
  Float_t a[10],b[10];
  std::ostringstream oss_bad;
  std::ostringstream oss_good;

  //for (auto i=1;i<=8;i++) {
  for (auto i: {1,2,3,4}) {
    a[i] = *(Float_t*)((char*)&ipls + ipls.GetPtr(i));
    b[i] = *(Float_t*)((char*)&ipls2 + ipls2.GetPtr(i));
    if (a[i]!=b[i]) {
      bad=true;
      oss_bad << " " << i << ": " << a[i] << " " << b[i];
    }
    else {
      oss_good << " " << i << ": " << a[i] << " " << b[i];
    }
  }

  if (bad) {
    prnt("ss l ss;",BGRN,"bad:",ipls.Tstamp64,oss_bad.str().c_str(),RST);
    prnt("ss l ss;",BMAG,"good:",ipls.Tstamp64,oss_good.str().c_str(),RST);
    cout << "-------" << endl;
  }

  /*
  if (Peaks.size()!=2) {
    cout <<"CheckDSP: Peaks.size()!=2: " << Peaks.size()
	 << " " << Counter << endl;
    return;
  }
  const int nn=5;
  const Float_t eps=0.1;

  Float_t cc[nn];
  cc[0] = Peaks[0].Base - Peaks[1].Base;
  cc[1] = Peaks[0].Area0 - Peaks[1].Area0;
  cc[2] = Peaks[0].Height - Peaks[1].Height;
  cc[3] = Peaks[0].Time - Peaks[1].Time;
  cc[4] = Peaks[0].Width - Peaks[1].Width;

  Bool_t bad=false;
  for (int i=0;i<nn;i++) {
    if (abs(cc[i])>eps)
      bad=true;
  }

  if (bad) {
    printf(ANSI_COLOR_YELLOW"Error!\n");
    printf(ANSI_COLOR_RED
	   "Alp: %d E:%lld B:%8.1f A0:%8.1f H:%8.1f T:%8.1f W:%8.1f P:%4d\n" ANSI_COLOR_RESET,
	   Chan,Counter,Peaks[0].Base,Peaks[0].Area0,Peaks[0].Height,
	   Peaks[0].Time,Peaks[0].Width,Pos);
    printf(ANSI_COLOR_GREEN
	   "Kop: %d E:%lld B:%8.1f A0:%8.1f H:%8.1f T:%8.1f W:%8.1f P:%4d\n" ANSI_COLOR_RESET,
	   Chan,Counter,Peaks[1].Base,Peaks[1].Area0,Peaks[1].Height,
	   Peaks[1].Time,Peaks[1].Width,Pos);
  }
  else {
    //printf("%10lld OK\n",Counter);
  }

  Peaks.pop_back();
  */
}

bool CRS::MakeDecMask() {
  // проверяет opt.dec_mask и создает управляющие переменные для dec81/82:
  // sdec_e,sdec_p,sdec_d,sdec_c

  bool res=true;

  string mask = string(opt.dec_mask);
  string smask;
  size_t p;

  sdec_e.clear();
  sdec_p.clear();
  sdec_d=false;
  sdec_c=false;

  // записываем маску для события, заменяем найденные символы пробелом
  smask = string(mask_e);
  for (size_t i=0;i<mask.size();i++) {
    for (size_t j=0;j<smask.size();j++) {
      if (mask[i]==smask[j]) {
	sdec_e+=mask[i];
	smask[j]=' ';
	mask[i]=' ';
      }
    }
  }

  // записываем маску для импульсов, заменяем найденные символы пробелом
  smask = string(mask_p);
  for (size_t i=0;i<mask.size();i++) {
    for (size_t j=0;j<smask.size();j++) {
      if (mask[i]==smask[j]) {
	sdec_p+=mask[i];
	smask[j]=' ';
	mask[i]=' ';
      }
    }
  }

  p=mask.find_first_of("D");
  if (p!=std::string::npos) {
    sdec_d=true;
    mask[p]=' ';
  }

  p=mask.find_first_of("C");
  if (p!=std::string::npos) {
    sdec_c=true;
    mask[p]=' ';
  }

  //удаляем все пробелы
  p = mask.find_first_of(' ');
  while (p!=std::string::npos)
  {
    mask.erase(p,1);
    p=mask.find_first_of(' ');
  }

  //cout << "mask_e: " << sdec_e << endl;
  //cout << "mask_p: " << sdec_p << endl;
  //cout << "mask_dc: " << sdec_d << " " << sdec_c << " " << mask.size() << endl;

  // если после удаления пробелов что-то осталось -> что-то не то
  if (mask.size())
    res=false;
  return res;
}

void CRS::PulseAna(PulseClass &ipls) {
  if (!opt.Dsp[ipls.Chan]) { // не Dsp -> анализируем импульс
    if (opt.sS[ipls.Chan]>1) {
      ipls.Smooth(opt.sS[ipls.Chan]);
    }
    else if (opt.sS[ipls.Chan]<-1) {
      ipls.Smooth_hw(-opt.sS[ipls.Chan]);
    }
    ipls.PeakAna33();
  }
  else { //Dsp -> не анализируем
    PulseClass ipls2 = ipls;
    ipls.Pos=cpar.Pre[ipls.Chan];
    if (opt.checkdsp) {
      // if (ipls.Peaks.size()!=1) {
      // 	cout << "size!!!: " << ipls.Peaks.size() << endl;
      // }
      ipls2.PeakAna33();
      CheckDSP(ipls,ipls2);
    }
    // else {
    //   // для sTg=3,6,7 ищем пересечение нижнего порога/нуля
    //   switch (opt.sTg[ipls.Chan]) {
    //   case 3:
    //   case 6:
    // 	break;
    //   default:
    // 	break;
    //   }
    // }
  }

  //prnt("ss d l d f fs;",BRED, "WD:", ipls.Chan, ipls.Tstamp64, ipls.Pos, ipls.Time, ipls.Width, RST);
  
  ipls.Ecalibr(ipls.Area);
  if (hcl->b_base[ipls.Chan]) {
    ipls.Ecalibr(ipls.Base);
    ipls.Ecalibr(ipls.Sl1);
    ipls.Ecalibr(ipls.Sl2);
    ipls.Ecalibr(ipls.RMS1);
    ipls.Ecalibr(ipls.RMS2);
    //ipls.Bcalibr();
  }
}

void CRS::Dec_Init(eventlist* &Blist, UChar_t frmt) {
  decode_mut.Lock();
  //++buf_inits;
  Bufevents.push_back(eventlist());
  Blist = &Bufevents.back();
  decode_mut.UnLock();

  //prnt("ss ds;",KYEL,"Dec_Init:",Bufevents.size(),RST);

  // Blist->push_back(EventClass());
  // Blist->front().Nevt=iread;
  if (frmt) {
    //prnt("ss d ds;",KYEL,"bad buf start: ",nevents,frmt,RST);
    ++errors[ER_START]; //bad buf start
  }
}

void CRS::Dec_End(eventlist* &Blist, UInt_t iread, UChar_t sp) {
  Blist->push_back(EventClass());
  Blist->back().Nevt=iread;
  Blist->back().Spin=sp;
}

/*
void CRS::Decode79a(UInt_t iread, UInt_t ibuf) {
  //the same as 79, with additional calibration

  //ibuf - current sub-buffer
  int idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);
  PulseClass pls;//=dummy_pulse;
  PulseClass* ipls=&pls;
  static Long64_t Tst;
  static UChar_t Spn;

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (opt.fProc) { //fill pulses for reanalysis
      if (frmt) { //event start
	ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);
	Tst = (*buf8) & sixbytes;
	(*buf8)>>=48;
	Spn = UChar_t((*buf8) & 1);
	//Spn|=2;
      }
      else {
	//pls=PulseClass();

	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	ipls->Chan = buf2[3];
	ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
	ipls->Area=opt.E0[ipls->Chan] + opt.E1[ipls->Chan]*ipls->Area;

	//new2
	ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples
	ipls->Tstamp64=Tst;


	ipls->Spin=Spn;
	ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	Event_Insert_Pulse(Blist,ipls);
      }
    }
    else { //fill event
      if (frmt) { //event start	
	ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);
	evt = &*Blist->insert(Blist->end(),EventClass());
	evt->Nevt=nevents;
	nevents++;
	evt->Tstmp = (*buf8) & sixbytes;
	(*buf8)>>=48;
	evt->Spin = UChar_t((*buf8) & 1);
	//evt->Spin|=2;
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	pulse_vect::iterator itpls =
	  evt->pulses.insert(evt->pulses.end(),PulseClass());
	ipls = &(*itpls);
	ipls->Chan = buf2[3];
	ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
	ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01
	  + opt.sD[ipls->Chan]/opt.Period; //in samples
	ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;
	if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	  //prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	  evt->T0=ipls->Time;
	}
	ipls->Area=opt.E0[ipls->Chan] + opt.E1[ipls->Chan]*ipls->Area;
      }
    } //fill event

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread);

} //decode79a
*/

void CRS::Decode81(UInt_t iread, UInt_t ibuf) {
  //ibuf - current sub-buffer
  // предполагается, что:
  //b_start[ibuf] - начинается на начале события (не может быть в середине)
  //b_end[ibuf] - заканчивается на конце события (не может быть в середине)
  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);
  PulseClass pls=good_pulse;
  PulseClass* ipls=&pls;
  //static Long64_t Tst;
  //static UChar_t Spn;
  ULong64_t* Buf8;
  Short_t* DecBuf2;
  UShort_t* UDecBuf2;

  bool b_T = sdec_e.find('T')!=string::npos;
  bool b_N = sdec_e.find('N')!=string::npos;
  //if (!opt.fProc) { //fill event
  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit
    Buf8 = (ULong64_t*) (GLBuf+idx1);

    if (frmt) { //event start -> записываем старое событие, создаем новое
      evt = &*Blist->insert(Blist->end(),good_event);
    }

    if (b_T) {
      evt->Tstmp = (*Buf8) & sixbytes;
      (*Buf8)>>=48;
      //evt->Spin |= UChar_t((*buf8) & 1);
      evt->Spin |= UChar_t(*Buf8);
      ++Buf8;
    }
    if (b_N) {
      UDecBuf2 = (UShort_t*) Buf8;
      evt->Nevt=UDecBuf2[1];
      ++Buf8;
    }
    else {
      evt->Nevt=nevents;
      nevents++;
    }

    // в событии либо счетчики, либо пики
    if (/*(evt->Spin & 128) &&*/ sdec_c) { //Counters
    } //Counters
    else { //Peaks
      for (auto ipls=evt->pulses.begin(); ipls!=evt->pulses.end(); ++ipls) {
      }
    } //Peaks

    //here
    int YY  = 1;

    Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      UShort_t* buf2u = (UShort_t*) buf2;
      UChar_t* buf1u = (UChar_t*) buf2;
      pulse_vect::iterator itpls =
	evt->pulses.insert(evt->pulses.end(),pls);
      ipls = &(*itpls);
      ipls->Chan = buf2[3];

      if (evt->Spin & 128) {//Counters
	ipls->Counter = (*Buf8) & sixbytes;
	ipls->Pos = -32222;
      }
      else { //Peaks
	ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
	ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01
	  + opt.sD[ipls->Chan]/opt.Period; //in samples
	ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	ipls->Height = ((UInt_t) buf1u[7])<<8;

	if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	  //prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	  evt->T0=ipls->Time;
	}
	ipls->Ecalibr(ipls->Area);
      }


    //prnt("ss l d ls;",BCYN,"d79:",evt->Tstmp,evt->Spin,evt->pulses.size(),RST);

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread,254);

  //} //if (fill event)

  /*
  else { //fill pulses for reanalysis
    while (idx1<b_end[ibuf]) {
      frmt = GLBuf[idx1+7] & 0x80; //event start bit
      buf8 = (ULong64_t*) (GLBuf+idx1);

      if (frmt) { //event start
	Tst = (*buf8) & sixbytes;
	(*buf8)>>=48;
	//Spn = UChar_t((*buf8) & 1);
	Spn = UChar_t(*buf8);
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	UChar_t* buf1u = (UChar_t*) buf2;
	ipls->Chan = buf2[3];

	if (Spn & 128) { //Counters
	  ipls->Counter = (*buf8) & sixbytes;
	  ipls->Pos = -32222;
	}
	else { //Peaks
	  ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;

	  //new2
	  ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples
	  ipls->Tstamp64=Tst;// *opt.Period;

	  ipls->Spin=Spn;
	  ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	  ipls->Height = ((UInt_t) buf1u[7])<<8;

	  ipls->Ecalibr(ipls->Area);
	}
	Event_Insert_Pulse(Blist,ipls);
      }

      idx1+=8;
    } //while (idx1<buf_len)

    Dec_End(Blist,iread,255);

  } // else (if (opt.fProc))
  */

} //decode81

void CRS::Decode80(UInt_t iread, UInt_t ibuf) {
  //trigger on START channel

  //ibuf - current sub-buffer
  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);
  PulseClass pls=good_pulse;
  PulseClass* ipls=&pls;

  static std::bitset<MAX_CH> Channels; 
  //prnt("sl;","d80: ",nevents);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start	
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);
      evt = &*Blist->insert(Blist->end(),good_event);
      evt->Nevt=nevents;
      nevents++;
      evt->Tstmp = (*buf8) & sixbytes;
      (*buf8)>>=48;
      evt->Spin |= UChar_t((*buf8) & 1);
      //evt->Spin|=2;
      Channels.reset();
      //prnt("ss l l ls;",BBLU,"Dc:",evt->Nevt,evt->Tstmp,2,RST);
    }
    else {
      Int_t* buf4 = (Int_t*) (GLBuf+idx1);

      int ch = buf4[1];
      if (ch>=opt.Nchan || ch<0) { //bad channel
	++errors[ER_CH];
	cout << "Dec80 Bad channel: " << endl;
	idx1+=8;
	continue;
      }
      if (!Channels.test(ch)) {
	Channels.set(ch);
	pulse_vect::iterator itpls =
	  evt->pulses.insert(evt->pulses.end(),pls);
	itpls->Chan=ch;
	ipls = &(*itpls);
      }
      ipls->sData.push_back(buf4[0]);

      
      if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	//prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	evt->T0=ipls->Time;
      }
    }

    idx1+=8;
  } //while (idx1<buf_len)

  // if (evt->Spin)
  //   cout << "Dec80: " << evt->Tstmp << " " << evt->pulses[1].sData.size() << " " << idx1 << " " << b_end[ibuf] << endl;

  Dec_End(Blist,iread,254);


} //decode80

void CRS::Decode79(UInt_t iread, UInt_t ibuf) {
  //Decode79 - the same as 78, but different factor for Area

  //ibuf - current sub-buffer
  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);
  PulseClass pls=good_pulse;
  PulseClass* ipls=&pls;
  static Long64_t Tst;
  static UChar_t Spn;
  ULong64_t* buf8;

  //prnt("sl;","d79: ",nevents);

  if (opt.fProc) { //fill pulses for reanalysis
    while (idx1<b_end[ibuf]) {
      frmt = GLBuf[idx1+7] & 0x80; //event start bit
      buf8 = (ULong64_t*) (GLBuf+idx1);

      if (frmt) { //event start
	Tst = (*buf8) & sixbytes;
	(*buf8)>>=48;
	//Spn = UChar_t((*buf8) & 1);
	Spn = UChar_t(*buf8);
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	UChar_t* buf1u = (UChar_t*) buf2;
	ipls->Chan = buf2[3];

	if (Spn & 128) { //Counters
	  ipls->Counter = (*buf8) & sixbytes;
	  ipls->Pos = -32222;
	}
	else { //Peaks
	  ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;

	  //new2
	  ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples
	  // Замененить на:
	  // ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01
	  //  + opt.sD[ipls->Chan]/opt.Period; //in samples

	  ipls->Tstamp64=Tst;//*opt.Period;

	  ipls->Spin=Spn;
	  ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	  ipls->Height = ((UInt_t) buf1u[7])<<8;

	  ipls->Ecalibr(ipls->Area);
	}
	Event_Insert_Pulse(Blist,ipls);
      }

      idx1+=8;
    } //while (idx1<b_end)

    Dec_End(Blist,iread,255);

  } //if (opt.fProc)

  else { //!opt.fProc -> fill event
    while (idx1<b_end[ibuf]) {
      frmt = GLBuf[idx1+7] & 0x80; //event start bit
      buf8 = (ULong64_t*) (GLBuf+idx1);

      if (frmt) { //event start	
	evt = &*Blist->insert(Blist->end(),good_event);
	evt->Nevt=nevents;
	nevents++;
	evt->Tstmp = (*buf8) & sixbytes;
	(*buf8)>>=48;
	//evt->Spin |= UChar_t((*buf8) & 1);
	evt->Spin |= UChar_t(*buf8);
	//if (evt->Spin & 128) //Counters
	//prnt("ss l ds;",BGRN,"d79:",evt->Tstmp,evt->Spin,RST);
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	UChar_t* buf1u = (UChar_t*) buf2;
	pulse_vect::iterator itpls =
	  evt->pulses.insert(evt->pulses.end(),pls);
	ipls = &(*itpls);
	ipls->Chan = buf2[3];

	if (evt->Spin & 128) { //Counters
	  ipls->Counter = (*buf8) & sixbytes;
	  ipls->Pos = -32222;
	  //prnt("ss l ds;",BRED,"d79:",evt->Tstmp,evt->Spin,RST);
	}
	else { //Peaks
	  ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
	  ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01
	    + opt.sD[ipls->Chan]/opt.Period; //in samples
	  ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	  ipls->Height = ((UInt_t) buf1u[7])<<8;

	  if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	    //prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	    evt->T0=ipls->Time;
	  }
	  ipls->Ecalibr(ipls->Area);
	}
      }

      //prnt("ss l d ls;",BCYN,"d79:",evt->Tstmp,evt->Spin,evt->pulses.size(),RST);

      idx1+=8;
    } //while (idx1<buf_len)

    Dec_End(Blist,iread,254);

  }

} //decode79

void CRS::Decode78(UInt_t iread, UInt_t ibuf) {
  //Decode78 - the same as 77, but different factors for Time and Width
  //also, added determination of T0

  //ibuf - current sub-buffer

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;
  PulseClass pls=good_pulse;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->Spin = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),pls);
      //ipls->Peaks.push_back(PeakClass());
      //PeakClass *pk = &ipls->Peaks.back();
      ipls->Area = buf2[0];
      ipls->Time = buf2[1]*0.01; //in samples
      ipls->Width = buf2[2]*0.001;
      ipls->Chan = buf2[3];
      if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	evt->T0=ipls->Time;
      }
      ipls->Ecalibr(ipls->Area);
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread,254);

} //decode78

void CRS::Decode77(UInt_t iread, UInt_t ibuf) {
  //Decode77 is the same as 76, but since this version Time is relative to event Tstmp (was relative to pulse Tstamp64)

  //ibuf - current sub-buffer

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;
  PulseClass pls=good_pulse;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->Spin = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),pls);
      //ipls->Peaks.push_back(PeakClass());
      //PeakClass *pk = &ipls->Peaks.back();
      ipls->Area = buf2[0];
      ipls->Time = buf2[1]*0.1;
      ipls->Width = buf2[2];
      ipls->Chan = buf2[3];
      if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	evt->T0=ipls->Time;
      }
      ipls->Ecalibr(ipls->Area);
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread,254);

} //decode77

void CRS::Decode76(UInt_t iread, UInt_t ibuf) {

  //ibuf - current sub-buffer

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;
  PulseClass pls=good_pulse;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);

  while (idx1<b_end[ibuf]) {
    frmt = GLBuf[idx1+7] & 0x80; //event start bit

    if (frmt) { //event start
      ULong64_t* buf8 = (ULong64_t*) (GLBuf+idx1);

      evt = &*Blist->insert(Blist->end(),EventClass());
      evt->Nevt=nevents;
      nevents++;

      evt->Tstmp = (*buf8) & sixbytes;
      evt->Spin = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),pls);
      //ipls->Peaks.push_back(PeakClass());
      //PeakClass *pk = &ipls->Peaks.back();
      ipls->Area = buf2[0];
      ipls->Time = buf2[1]*0.1;
      ipls->Chan = buf2[3];
      ipls->Ecalibr(ipls->Area);
    }

    idx1+=8;
  } //while (idx1<buf_len)

  Dec_End(Blist,iread,254);

} //decode76

void CRS::Decode75(UInt_t iread, UInt_t ibuf) {

  //cout << "decode75_1: " << endl;
  //ibuf - current sub-buffer

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;
  PulseClass pls=good_pulse;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);

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
      evt->Spin = Bool_t((*buf8) & 0x1000000000000);
    }
    else {
      Short_t* buf2 = (Short_t*) (GLBuf+idx1);
      pulse_vect::iterator ipls =
	evt->pulses.insert(evt->pulses.end(),pls);
      //ipls->Peaks.push_back(PeakClass());
      //PeakClass *pk = &ipls->Peaks.back();
      ipls->Area = buf2[0];
      ipls->Time = buf2[1]*0.1;
      ipls->Chan = buf2[3];
      ipls->Ecalibr(ipls->Area);
    }

    idx1+=8;
  } //while (idx1<buf_len)

  //cout << "decode75_3: " << endl;
  Dec_End(Blist,iread,254);

  // cout << "decode75: " << idx1 << " " << Blist->size()
  //      << " " << Bufevents.size() << " " << Bufevents.begin()->size() << endl;

} //decode75

void CRS::Decode34(UInt_t iread, UInt_t ibuf) {
  //устаревший. Может не работать при Nchan меньшем, чем реально записанные каналы.

  //ibuf - current sub-buffer

  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  ULong64_t data;
  Short_t sss; // temporary var. to convert signed nbit var. to signed 16bit int
  Int_t iii; // temporary var. to convert signed nbit var. to signed 32bit int
  Long64_t lll; //Long temporary var.
  int n_frm=0; //counter for frmt4 and frmt5
  //PeakClass *ipk=&dummy_peak; //pointer to the current peak in the current pulse;
  //Double_t QX=0,QY=0,RX,RY;
  Double_t QX=0,RX,AY;
  Float_t Area0;

  eventlist *Blist;
  UChar_t frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;
  //PulseClass start_pls;
  //double tt;

  //Print_Buf_err(ibuf);
  //Print_Buf_err(ibuf,"buf.dat");
  //exit(1);
  //cout << "Decode34: " << ibuf << " " << buf_off[ibuf] << " " << vv->size() << " " << length << " " << hex << buf8[idx1/8] << dec << endl;

  //bool st=0;

  while (idx1<b_end[ibuf]) {
    frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
    data = buf8[idx1/8] & sixbytes;
    UChar_t ch = GLBuf[idx1+7];

    if (ch==255) {
      //double tt = start_pls.Tstamp64*opt.Period*1e-9;
      //cout << "c255: " << ibuf << " " << data << " " << (int) frmt << endl;
      //start signal
      idx1+=8;
      continue;      
    }
    else if (frmt<6) {
      if (ch>=opt.Nchan) { //bad channel

	++errors[ER_CH];
	ipls.ptype|=P_BADCH;
	idx1+=8;
	continue;
      }
      if (frmt && ch!=ipls.Chan) { //channel mismatch
	++errors[ER_MIS];
	ipls.ptype|=P_BADCH;
	idx1+=8;
	continue;
      }
    }

    switch (frmt) {
    case 0:
      if (buf8[idx1/8]==0) {
	++errors[ER_ZERO]; //zero data
	idx1+=8;
	continue;
      }
			
      // if (nevents>207000 && nevents<210000) {
      // 	ipls.PrintPulse(0);
      // }
      //analyze previous pulse
      if (ipls.ptype==0) {
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
      }
      // create new pulse
      ipls=PulseClass();
      npulses++;
      ipls.Chan=ch;
      ipls.Tstamp64=data;//+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];
      //cout << "Tstmp64: " << ipls.Tstamp64 << " " << Blist->back().Tstmp << " " << Blist->size() << endl;
      n_frm=0;
      break;
    case 1:
      ipls.Spin = GLBuf[idx1+5];
      ipls.Counter = data & 0xFFFFFFFFFF;
      break;
    case 2:
      if ((int)ipls.sData.size()>=cpar.Len[ipls.Chan]) {
	// cout << "32: ERROR Nsamp: "
	//      << " " << (ipls.Counter & 0x0F)
	//      << " " << ipls.sData.size() << " " << cpar.Len[ipls.Chan]
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
      if ((int)ipls.sData.size()>=cpar.Len[ipls.Chan]) {
	ipls.ptype|=P_BADSZ;
      }
      //else {
      for (int i=0;i<3;i++) {
	iii = data & 0xFFFF;
	ipls.sData.push_back((iii<<16)>>16);
	//ipls.sData.push_back((iii<<16)>>22);
	data>>=16;
	// if (npulses==6) {
	//   cout << ipls.sData.back() << " " << i1 << " " << i2 << endl;
	// }
      }
      // if (npulses==6 && ipls.sData.size()==702) {
      // 	cout << "iii: " << ipls.sData.size() << endl;
      // 	for (int j=0;j<ipls.sData.size();j++) {
      // 	  cout << j << " " << ipls.sData[j] << endl;
      // 	}
      // }


      //}
      break;
    case 4:
      if (opt.Dsp[ipls.Chan]) {
	ipls.Pos = cpar.Pre[ipls.Chan];
	// if (ipls.Peaks.size()==0) {
	//   ipls.Peaks.push_back(PeakClass());
	//   ipk=&ipls.Peaks[0];
	// }
	switch (n_frm) {
	case 0: //C – [24]; A – [24]
	  //area
	  iii = data & 0xFFFFFF;
	  Area0=((iii<<8)>>8);
	  Area0/=p_len[ipls.Chan];
	  //ipls.Area0=((iii<<8)>>8);
	  //ipls.Area0/=p_len[ipls.Chan];
	  data>>=24;
	  //bkg
	  iii = data & 0xFFFFFF;
	  ipls.Base=((iii<<8)>>8);
	  ipls.Base/=b_len[ipls.Chan];
	  ipls.Area=Area0 - ipls.Base;
	  //ipls.Area=ipls.Area0 - ipls.Base;
	  if (opt.Bc[ipls.Chan]) {
	    ipls.Area+=opt.Bc[ipls.Chan]*ipls.Base;
	  }
	  break;
	case 1: //H – [12]; QX – [36]
	  lll = data & 0xFFFFFFFFF;
	  QX=((lll<<28)>>28);
	  data>>=36;
	  //height
	  iii = data & 0xFFF;
	  ipls.Height=((iii<<20)>>20);
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
	    ipls.Time=QX/RX;
	  //else
	  //ipls.Time=-999;

	  //AY (Width)
	  if (ipls.Area) {
	    iii = data & 0xFFFFFF;
	    AY=((iii<<8)>>8);

	    ipls.Width=AY/w_len[ipls.Chan]-ipls.Base;
	    //ipls.Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipls.Width;
	    // if (opt.Bc[ipls.Chan]) {
	    //   ipls.Width+=opt.Bc[ipls.Chan]*ipls.Base;
	    // }
	    ipls.Width/=ipls.Area;
	  }
	  else {
	    ipls.Width=0;
	  }

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.Dsp[ipls.Chan])
      break;
    case 5:
      if (opt.Dsp[ipls.Chan]) {
	ipls.Pos = cpar.Pre[ipls.Chan];
	// if (ipls.Peaks.size()==0) {
	//   ipls.Peaks.push_back(PeakClass());
	//   ipk=&ipls.Peaks[0];
	// }
	switch (n_frm) {
	case 0: //C – [28]; H – [16]
	  //height
	  sss = data & 0xFFFF;
	  ipls.Height=sss;
	  data>>=16;
	  //bkg
	  iii = data & 0xFFFFFFF;
	  ipls.Base=((iii<<4)>>4);
	  ipls.Base/=b_len[ipls.Chan];
	  break;
	case 1: //A – [28]
	  //area
	  iii = data & 0xFFFFFFF;
	  Area0=((iii<<4)>>4);
	  Area0/=p_len[ipls.Chan];
	  ipls.Area=Area0 - ipls.Base;
	  // ipls.Area0=((iii<<4)>>4);
	  // ipls.Area0/=p_len[ipls.Chan];
	  // ipls.Area=ipls.Area0 - ipls.Base;
	  if (opt.Bc[ipls.Chan]) {
	    ipls.Area+=opt.Bc[ipls.Chan]*ipls.Base;
	  }
	  break;
	case 2: //QX – [40]
	  lll = data & 0xFFFFFFFFFF;
	  QX=((lll<<24)>>24);
	  break;
	case 3: //AY – [28]
	  if (ipls.Area) {

	    iii = data & 0xFFFFFFF;
	    AY=((iii<<4)>>4);

	    ipls.Width=AY/w_len[ipls.Chan]-ipls.Base;
	    //ipls.Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipls.Width;
	    // if (opt.Bc[ipls.Chan]) {
	    // 	ipls.Width+=opt.Bc[ipls.Chan]*ipls.Base;
	    // }
	    ipls.Width/=ipls.Area;
	  }
	  else {
	    ipls.Width=0;
	  }
	  // cout << "Width: " << ipls.Counter << " " << ipls.Width << " " << ipls.Area << " " << ipls.Area0 << endl;

	  break;
	case 4: //reserved – [24]; RX – [24]
	  //RX
	  iii = data & 0xFFFFFF;
	  RX=((iii<<8)>>8);
	  data>>=24;

	  if (RX!=0)
	    ipls.Time=QX/RX;
	  //else
	  //ipls.Time=-999;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.Dsp[ipls.Chan])
      break;
    case 6:
      {
	//tt = Blist->back().Tstmp*opt.Period*1e-9;
	
	//cout << "c6: " << ibuf << " " << (int) ch << " " << data << " " << npulses3[ch] << " " << ipls.Tstamp64 << " " << (int) ipls.Spin << endl;
	//if (!(ipls.Spin & 0x80)) {
	//ipls.Spin|=0x80;
	//}


	npulses3o[ch]=data;



	/*
	// if (opt.h_count.b) {
	//   Blist->rbegin()->Fill_Time_Extend(hcl->m_counter[ch]);
	//   //cout << "counters: " << (int) ch << " " << rate << " " << data << endl;
	//   EventClass::Fill1dwSt(true,hcl->m_counter,ch,opt.T_acq,rate);
	//   EventClass::Fill1dwSt(false,hcl->m_counter,ch,opt.T_acq,rate);
	// }
	*/
	break;
      }
    case 7:
      //cout << "c7: " << ibuf << " " << (int) ch << " " << data << endl;
      break;
    default:
      //Print_Buf_err(ibuf,"error.dat");
      //exit(1);

      ++errors[ER_FRMT];
      //cout << "bad frmt: " << frmt << endl;
    } //switch (frmt);

    //idx8++;
    idx1+=8;
  } //while (idx1<buf_len)

  //add last pulse to the list
  // if (nevents>207000 && nevents<210000) {
  //   cout << "last:" << endl;
  //   ipls.PrintPulse(0);
  // }
  if (ipls.ptype==0) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  Dec_End(Blist,iread,255);

} //decode34

void CRS::MakePk(PkClass &pk, PulseClass &ipls) {
  //ipls.Peaks.push_back(PeakClass());
  //PeakClass *ipk=&ipls.Peaks[0];
  Float_t Area0;

  ipls.Pos = cpar.Pre[ipls.Chan];

  Area0=pk.A/p_len[ipls.Chan];
  ipls.Base=pk.C/b_len[ipls.Chan];
  ipls.Area=Area0 - ipls.Base;
  // ipls.Area0=pk.A/p_len[ipls.Chan];
  // ipls.Base=pk.C/b_len[ipls.Chan];
  // ipls.Area=ipls.Area0 - ipls.Base;

  ipls.Height=pk.H;

  switch (cpar.Trg[ipls.Chan]) {
  case 3:
  case 6:
  case 7:
    {
      int kk = cpar.Drv[ipls.Chan];
      //cout << "pos: " << ipls.Pos << " " << kk << endl;
      //if (ipls.Pos>=kk && (int)ipls.sData.size()>ipls.Pos+1) {
      ipls.FindZero(kk,cpar.Thr[ipls.Chan],cpar.LT[ipls.Chan]);
      ipls.Time-=ipls.Pos;
    }
    break;
  default:
    if (pk.RX!=0)
      ipls.Time=Double_t(pk.QX)/pk.RX;
    else {
      ++errors[ER_TIME];
      //ipls.Time=-999;
      //YK;
    }
  } //switch

  //ipls.Time=pk.RX;

  Float_t wdth = pk.AY/w_len[ipls.Chan];

  if (opt.Mt[ipls.Chan]!=3) {
    if (ipls.Area) {
      ipls.Width=wdth-ipls.Base;
      ipls.Width/=ipls.Area;
    }
    else {
      ++errors[ER_WIDTH];
      ipls.Width=0;
    }
  }
  else {
    ipls.Sl2 = (ipls.Base - wdth)/(b_mean[ipls.Chan]-w_mean[ipls.Chan]);
    ipls.Area -= (p_mean[ipls.Chan]-b_mean[ipls.Chan])*ipls.Sl2;

    ipls.Width = ipls.Pos-cpar.Pre[ipls.Chan]-ipls.Time;
    //prnt("ss d l d f fs;",BGRN, "WD:", ipls.Chan, ipls.Tstamp64, ipls.Pos, ipls.Time, ipls.Width, RST);
    if (ipls.Width<-99) ipls.Width=-99;
    if (ipls.Width>99) ipls.Width=99;
  }

  //ipls.Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipls.Area;
  if (opt.Bc[ipls.Chan]) {
    ipls.Area+=opt.Bc[ipls.Chan]*ipls.Base;
  }

} //MakePk

void CRS::Decode35(UInt_t iread, UInt_t ibuf) {
  //romana test140_HPGe_2_Labr_4_Ing27_87prc_Generator_100hz_Na_top_Pb_1mm_SiO2.raw -p test139.par
  //error at Tstamp 38721814589

  //ibuf - current sub-buffer
  ULong64_t* buf8 = (ULong64_t*) GLBuf;//Fbuf[ibuf];

  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  ULong64_t data;
  Int_t d32;
  Short_t d16;
  PkClass pk;

  //UChar_t initevt; //инициирующее запись событие

  //PeakClass *ipk=&dummy_peak; //pointer to the current peak in the current pulse;

  eventlist *Blist;
  UChar_t frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;

  while (idx1<b_end[ibuf]) {
    frmt = (GLBuf[idx1+6] & 0xF0) >> 4;
    //initevt = (GLBuf[idx1+6] & 0x0F) >> 1;
    data = buf8[idx1/8] & sixbytes;
    UChar_t ch = GLBuf[idx1+7];

    // if (initevt!=3) {
    //   prnt("ss l d d d ds;",BGRN,"d35:",idx1,ch,frmt,initevt,ipls.ptype,RST);
    // }

    if (frmt && ch!=ipls.Chan) { //channel mismatch
      // if (initevt!=3) {
      // 	prnt("ss d d d ds;",BRED,"m35:",ch,ipls.Chan,frmt,initevt,RST);
      // }
      ++errors[ER_MIS];
      ipls.ptype|=P_BADCH;
      idx1+=8;
      continue;
    }

    //prnt("sss;",BRED,"Good",RST);
    //prnt(" d",frmt);

    switch (frmt) {
    case 0:
      //prnt("ss l d d ds;",BBLU,"CH:",idx1,ch,ipls.Chan,ipls.ptype,RST);
      if (buf8[idx1/8]==0) {
	++errors[ER_ZERO]; //zero data
	idx1+=8;
	continue;
      }

      if (ipls.ptype==0) { //analyze old pulse
	if (opt.Dsp[ipls.Chan]) {
	  MakePk(pk,ipls);
	}
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
      }

      // test ch, then create new pulse

      if (ch!=255 && ch>=opt.Nchan) { //bad channel
	++errors[ER_CH];
	ipls.ptype|=P_BADCH;
	//idx1+=8;
	//continue;
	break;
      }

      ipls=PulseClass();
      npulses++;
      ipls.Chan=ch;
      ipls.Tstamp64=data;//+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];

      if (ch==255) { //start channel
	ipls.Spin|=128; //bit 7 - hardware counters
	//prnt("ss ds;",BYEL,"STARTCH:",ch,RST);
      }

      break;

    case 1:
      ipls.Spin |= GLBuf[idx1+5] & 1;
      ipls.Counter = data & 0xFFFFFFFFFF;
      //prnt("ss d l ls;",BMAG,"CONT1:",ch,ipls.Tstamp64,ipls.Counter,RST);
      break;
    case 2:
      if ((int)ipls.sData.size()>=cpar.Len[ipls.Chan]) {
	ipls.ptype|=P_BADSZ;
	++errors[ER_LEN];
	idx1+=8;
	continue;
      }
      for (int i=0;i<4;i++) {
#ifdef BITS
	  (data>>=BITS)<<=BITS;
#endif
	d32 = data & 0x7FF; //11 bit
	ipls.sData.push_back((d32<<21)>>21);
	data>>=12;
      }
      break;
    case 3:
      if ((int)ipls.sData.size()>=cpar.Len[ipls.Chan]) {
	ipls.ptype|=P_BADSZ;
	++errors[ER_LEN];
	idx1+=8;
	continue;
      }
      if (cpar.F24) {
	for (int i=0;i<2;i++) {
#ifdef BITS
	  (data>>=BITS)<<=BITS;
#endif
	  d32 = data & 0xFFFFFF; //24 bit
	  Float_t f32 = (d32<<8)>>8;
	  //Float_t f33 = f32+1;
	  //Float_t f34 = f32-1;
	  //Double_t f32 = (d32<<8)>>8;

	  //printf("d32_0: %d %x %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f32/256.0f);
	  //printf("d32_1: %d %x %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f33/256.0f);
	  //printf("d32_2: %d %x %16.8f\n",((d32<<8)>>8),((d32<<8)>>8),f34/256.0f);

	  ipls.sData.push_back(f32/256.0f);
	  data>>=24;
	}
      }
      else { 
	for (int i=0;i<3;i++) {
#ifdef BITS
	  (data>>=BITS)<<=BITS;
#endif
	  d16 = data & 0xFFFF; //16 bit
	  ipls.sData.push_back(d16);
	  data>>=16;
	}
      }
      break;
    case 4: //C – [24]; A – [24]
      pk.A = data & 0xFFFFFF;
      pk.A = (pk.A<<8)>>8;
      data>>=24;
      pk.C = data & 0xFFFFFF;
      pk.C = (pk.C<<8)>>8;
      break;
    case 5: //RX – [12]; QX – [36]
      pk.QX = data & 0xFFFFFFFFF;
      pk.QX = (pk.QX<<28)>>28;
      data>>=36;
      pk.RX = data & 0xFFF;
      pk.RX = (pk.RX<<20)>>20;
      break;
    case 6: //AY – [28]; [E]; H – [16]
      pk.H = data & 0xFFFF;
      data>>=16;
      pk.E = data & 1;
      data>>=4;
      pk.AY = data & 0xFFFFFFF;
      pk.AY = (pk.AY<<4)>>4;
      break;
    case 8: //RX – [20]; C – [28]
      pk.C = data & 0xFFFFFFF;
      pk.C = (pk.C<<4)>>4;
      data>>=28;
      pk.RX = data & 0xFFFFF;
      pk.RX = (pk.RX<<12)>>12;
      break;
    case 9: //A – [28]
      pk.A = data & 0xFFFFFFF;
      pk.A = (pk.A<<4)>>4;
      break;
    case 10: //QX – [40]
      pk.QX = data & 0xFFFFFFFFFF;
      pk.QX = (pk.QX<<24)>>24;
      break;
    case 11: { //Counters
      ipls.Counter = data;
      ipls.Spin|=128; //bit 7 - hardware counters

      double dt = (ipls.Tstamp64 - Tst3o[ipls.Chan])*1e-9*opt.Period;
      if (dt) {
	rate_hard[ipls.Chan] = (ipls.Counter - npulses3o[ipls.Chan])/dt;
      }
      Tst3o[ipls.Chan] = ipls.Tstamp64;
      npulses3o[ipls.Chan] = ipls.Counter;

      //prnt("ss d l l f fs;",BBLU,"CONT:",ch,ipls.Tstamp64,ipls.Counter,dt,rate3[ipls.Chan],RST);
      break;
    }
    case 12:
      if (data & 1) {
	++errors[ER_OVF];
	ipls.Spin|=128; //bit 7 - hardware counters
	ipls.Spin|=4; //bit 2 - ER_OVF
      }
      //prnt("ss d l ls;",KGRN,"OVF:",ch,ipls.Tstamp64,data&1,RST);
      break;
    default:
      ++errors[ER_FRMT];
    } //switch (frmt);

    idx1+=8;
  } //while (idx1<buf_len)

  //add last pulse to the list
  // if (nevents>207000 && nevents<210000) {
  //   cout << "last:" << endl;
  //   ipls.PrintPulse(0);
  // }
  if (ipls.ptype==0) {
    if (opt.Dsp[ipls.Chan]) {
      MakePk(pk,ipls);
    }
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  Dec_End(Blist,iread,255);

} //decode35

void CRS::Decode2(UInt_t iread, UInt_t ibuf) {

  UShort_t* buf2 = (UShort_t*) GLBuf;

  Long64_t idx2=b_start[ibuf]/2;

  eventlist *Blist;
  UChar_t frmt = (buf2[idx2] & 0x7000)>>12;
  Dec_Init(Blist,frmt);
  PulseClass ipls=dummy_pulse;

  //cout << "decode2: " << idx2 << endl;
  while (idx2<b_end[ibuf]/2) {

    unsigned short uword = buf2[idx2];
    frmt = (uword & 0x7000)>>12;
    short data = uword & 0xFFF;
    UChar_t ch = (uword & 0x8000)>>15;

    if (frmt && ch!=ipls.Chan) { //channel mismatch
      ++errors[ER_MIS];
      ipls.ptype|=P_BADCH;
      idx2++;
      continue;
    }

    //prnt("ss l d d ds;",BGRN,"CH:",idx2,frmt,ch,ipls.ptype,RST);

    if (frmt==0) {
      //ipls.ptype&=~P_NOLAST; //pulse has stop

      //analyze previous pulse
      if (ipls.ptype==0) {
	PulseAna(ipls);
	Event_Insert_Pulse(Blist,&ipls);
	//cout << "Pana: " << Blist->size() << " " << ipls.Tstamp64 << endl;
      }

      // test ch, then create new pulse
      if (ch>=opt.Nchan) { //bad channel

	++errors[ER_CH];
	ipls.ptype|=P_BADCH;
	idx2++;
	continue;
      }

      ipls=PulseClass();
      npulses++;
      ipls.Chan = ch;
      ipls.Tstamp64=data;//+(Long64_t)opt.sD[ch];// - cpar.Pre[ch];
    }
    else if (frmt<4) {
      Long64_t t64 = data;
      ipls.Tstamp64+= (t64 << (frmt*12));
    }
    else if (frmt==4) {
      ipls.Counter=data;
    }
    else if (frmt==5) {
      if ((int)ipls.sData.size()>=cpar.Len[ipls.Chan]) {
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

  Dec_End(Blist,iread,255);

} //decode2


//-------------------------------

int CRS::Searchsync(Long64_t &idx, UInt_t* buf4, Long64_t end) {
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

int CRS::Detect_adcm() {
  //res=0 - can't open file
  //res=2 - can't detect file format
  //res=1 - adcm raw
  //res=3 - adcm dec

  int res=0;
  //Long64_t Tstop64;

  Long_t id;
  Long64_t size;
  Long_t flags;
  Long_t modtime;

  int ires=gSystem->GetPathInfo(Fname, &id, &size, &flags, &modtime);

  if (f_read) gzclose(f_read);
  f_read = gzopen(Fname,"rb");

  if (ires || !f_read) {
    //prnt("ssss;",BRED,"Can't open file: ",Fname,RST);
    f_read=0;
    return 0;
  }

  char buf1[iMB];
  int len = gzread(f_read,buf1,iMB);

  int r_id=0;
  bool strt=true;

  for (int i=0;i<len-4;++i) {
    UShort_t* buf2 = (UShort_t*) (buf1+i);
    switch (*buf2) {
    case ID_ADCM:
      r_id++;
      if (strt) {
	UShort_t rLen = *((UShort_t*) (buf1+i+4));
	int idnext=i+rLen*4-2;
	if (idnext<len) {
	  UInt_t* buf4 = (UInt_t*) (buf1+idnext);
	  //cout << "idnext: " << i << " " << idnext << " " << *buf4 << endl;
	  Tstart64 = buf4[-2];
	  Tstart64 <<= 32;
	  Tstart64 += buf4[-3];
	  Offset64=Tstart64;
	  strt=false;
	}
      }
      break;
    case ID_EVNT:
      r_id--;
      if (strt) {
	//struct stor_packet_hdr_t *hdr = (struct stor_packet_hdr_t *) (buf2);
	struct stor_ev_hdr_t *eh = (struct stor_ev_hdr_t *) (buf2+sizeof(struct stor_packet_hdr_t)/2);
	Tstart64=eh->ts;
	Offset64=Tstart64;
	strt=false;
      }
      break;
    }
  }

  prnt("ssls;",BGRN,"Tstart: ",Tstart64,RST);
  //cout << "r_id: " << r_id << " " << strt << " " << Tstart64 << endl;

  gzclose(f_read);
  f_read=0;


  cpar.F_stop = (modtime-788907600)*1000;
  Text_time("E:",cpar.F_stop);

  if (r_id==0) {
    res=2;
  }
  else if (r_id>0) {
    res=1;
  }
  else {
    res=3;
  }

  return res;





  /*
  Long64_t size4,lim=2000;

  int fd = open(Fname, O_RDONLY);
  void* buf = mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
  UInt_t* buf4 = (UInt_t*) buf;
  UShort_t* buf2 = (UShort_t*) buf;

  size4=size/4;
  len = std::min(size4,lim);

  for (int i=0;i<len;++i) {
    if (buf4[i] == 0x2a500100) {
      int rLen = buf2[i*2+3];
      int idnext=i+rLen;
      if (idnext<len) {
	Tstart64 = buf4[idnext-2];
	Tstart64 <<= 32;
	Tstart64 += buf4[idnext-3];
	res+=1;
	break;
      }
    }
  }
  cout << "Tstart64: " << hex << Tstart64 << dec << endl;

  // for (int i=size4-1;i>=size4-len;--i) {
  //   if (buf4[i] == 0x2a500100) {
  //     if (i>3) {
  // 	Tstop64 = buf4[i-2];
  // 	Tstop64 <<= 32;
  // 	Tstop64 += buf4[i-3];
  // 	res+=1;
  // 	break;
  //     }
  //   }
  // }

  // if (res==2) {
  //   double dt = (Tstop64 - Tstart64)*1e-9*opt.adcm_period;
  //   cpar.F_start = (modtime-dt-788907600)*1000;
  //   Text_time();

  //   cout << "txt_start: " << txt_start << " " << modtime << " " << dt << " " << Tstop64 << " " << Tstart64 << " " << size4 << endl;
  // }

  munmap (buf, size);
  close(fd);
  */







}

/*
int CRS::Find_adcmraw_start() {

  int res=0;
  char buf[iMB];
  UInt_t* buf4 = (UInt_t*) buf;
  UShort_t* buf2 = (UShort_t*) buf;

  int len = gzread(f_read,buf,iMB);

  int idx=0;
  len/=4;
  if (Searchsync(idx,buf4,len)) {
    int rLen = buf2[idx*2+3];
    int idnext=idx+rLen;
    if (idnext<len) {
      Tstart64 = buf4[idx+rLen-2];
      Tstart64 <<= 32;
      Tstart64 += buf4[idx+rLen-3];//+(Long64_t)opt.sD[ipls.Chan];
      res=1;
    }
  }

  gzrewind(f_read);
  return res;
}
*/
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

  Long64_t idx=b_start[ibuf]/4; // current index in the buffer (in 1-byte words)
  Long64_t idnext=0; //idx of the next event
  int rLen; // length of one m-link fram
  UInt_t header; //data header
  UShort_t id; // block id (=0,1,2)
  UChar_t lflag; //last fragment flag
  //UShort_t nfrag; //fragment number
  UShort_t nsamp; // number of samples in the fragment
  Long64_t length = b_end[ibuf]/4;
  int ch;

  //prnt("ssd l xs;",BBLU,"idx: ",ibuf,idx,buf4[idx],RST);

  while (idx<length) {
    //rLen (~rbuf[idx+1]) should be inside buf
    //at this point idx points to a valid syncw and idnext is also withing FBuf

    if (buf4[idx] != 0x2a500100) {
      prnt("ssl x d ls;",BRED,"Bad syncw: ",idx,buf4[idx],ibuf,b_start[ibuf]/4,RST);
      //cout << "bad syncw: " << idx << " " << buf4[idx] << endl;
      if (!Searchsync(idx,buf4,length)){
	cout << "sync word not found (YK: do something here...)" << endl;
	break;
      }
    }

    rLen = buf2[idx*2+3];
    idnext=idx+rLen;

    //Long64_t Tstop64 = buf4[idnext-2];
    //Tstop64 <<= 32;
    //Tstop64 += buf4[idnext-3];
    //cout << "while: " << idx << " " << idnext << " " << length << " " << Tstop64 << endl;

    if (idnext>length)
      break;

    header = buf4[idx+3];
    id=bits(header,26,31);
    //prnt("ssds;",BGRN,"id: ",id,RST);
    if (id==1) {
      prnt("ssls;",BBLU,"adcm:id==1 Surprise!!! Counters!: ",npulses,RST);
    }
    else { //id!=1
      //analyze previous pulse and insert it into the list

      nsamp=bits(header,7,17);
      if (nsamp+8!=rLen) {
	++errors[ER_ALEN]; //wrong adcm length
	goto next;
      }

      lflag=bits(header,6,6);
      //prnt("ssds;",BRED,"lflag: ",lflag,RST);
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

      if (!(ipls.ptype&P_NOLAST)) { //-> у ipls есть last fragment
	// -> insert new pulse

	ipls=PulseClass();
	npulses++;
	ipls.Chan=ch;

	ipls.Tstamp64 = buf4[idx+rLen-2];
	ipls.Tstamp64 <<= 32;
	ipls.Tstamp64 += buf4[idx+rLen-3] - Offset64;

	// симулируем "плохое" событие
	// if (npulses==10) {
	//   ipls.Tstamp64 += 200000000001;
	// }

	//prnt("ssl l ls;",BRED,"T64: ",ipls.Tstamp64,Pstamp64,npulses,RST);

	// if (Offset64)
	//   ipls.Tstamp64-=Offset64;

	Long64_t dt=ipls.Tstamp64-Pstamp64;




	//10 or 20 sec = 2e9
	if (abs(dt) > 2000000000) { //bad event: поправляем Offset64
	  Offset64+=dt;
	  //ipls.Tstamp64 -= dt;
	  ipls.Tstamp64 = Pstamp64;
	  prnt("ssl l l f f fs;",BYEL,"Bad Tstamp: ",dt,ipls.Tstamp64,Pstamp64,
	       dt*sPeriod,ipls.Tstamp64*sPeriod,Pstamp64*sPeriod,RST);
	  ipls.ptype|=P_BADTST;
	  ++errors[ER_TST]; //bad adcm Tstamp
	}




	//else {
	Pstamp64=ipls.Tstamp64;
	//}

      } //if (!(ipls.ptype&P_NOLAST))
      else { //у ipls нет "last fragment"
	// -> this is continuation of the prev.pulse
	//ничего не делаем, хотя можно было проверить соответствие ch и Tstamp
      }

      if (lflag) {
	ipls.ptype&=~P_NOLAST;
      }
      else {
	ipls.ptype|=P_NOLAST;
      }

      //prnt("ssd ds;",BRED,"last: ",lflag,ipls.ptype,RST);

      for (int i=0;i<nsamp*2;i+=2) {
	ipls.sData.push_back(buf2[idx*2+i+11]);
	ipls.sData.push_back(buf2[idx*2+i+10]);
      }

    } //id!=1

  next:
    idx=idnext;

  } //while

  //cout << "nevt1: " << nevents << " " << (int) (ipls.ptype&~P_NOLAST) << endl;

  if (!ipls.ptype) {
    PulseAna(ipls);
    Event_Insert_Pulse(Blist,&ipls);
  }

  //cout << "nevt2: " << nevents << endl;

  Dec_End(Blist,iread,255);

  //cout << "idx2: " << idx << endl;
} //Decode_adcm

//-------------------------------------
void CRS::Decode_adcm_dec(UInt_t iread, UInt_t ibuf) {

  int nev = 0,
    nmap = 0,
    ncnt = 0;

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  Dec_Init(Blist,0);
  PulseClass pls;//=dummy_pulse;
  PulseClass* ipls=&pls;

  Long64_t dp,
    p = b_start[ibuf],
    length = b_end[ibuf];




  // for (int i=0;i<1024;i+=2) {
  //   struct stor_packet_hdr_t *hdr = (struct stor_packet_hdr_t *) GLBuf+p+i;
  //   Short_t* buf2 = (Short_t*) GLBuf+p+i;
  //   cout << i << " " <<hex<< *buf2 << " " << hdr->id << dec<<endl;
  // }





  while (p < length) {
    dp = p + sizeof(struct stor_packet_hdr_t);
    struct stor_packet_hdr_t *hdr = (struct stor_packet_hdr_t *) (GLBuf+p);
    //cout << "hdr: "<<p<<" "<<hex<<hdr->id<<dec<<" "<<hdr->size<<endl;
    if (p + hdr->size > length) {
      prnt("ss d s ls;",KRED,"size > length:",hdr->size, "at pos:",p,RST);
      break;
    }
    switch (hdr->id) {
    case ID_CMAP:
      nmap++;
      // memcpy (&cmap, &buf[dp], sizeof (struct adcm_cmap_t));
      break;
    case ID_EVNT:{
      struct stor_ev_hdr_t *eh = (struct stor_ev_hdr_t *) (GLBuf+dp);
      //int n;
      nev++;
      // вставляем новое пустое событие
      evt = &*Blist->insert(Blist->end(),EventClass());
      //UInt_t* t32 = (UInt_t*) &evt->Tstmp;

      //evt->Tstmp = 11111111111;
      // cout << "ttxx: " << hex << evt->Tstmp << " " << t32[0] << " "
      // 	   << t32[1] << dec << endl;

      evt->Nevt=nevents;
      nevents++;

      
      evt->Tstmp = (Pstamp64 & 0xFFFFFFFF00000000) + eh->ts;
      //t32[0] = eh->ts;
      //t32[1] = T_prev1;
      Long64_t dt=evt->Tstmp-Pstamp64;

      if (abs(dt) > 0x7FFFFFFF) { //
	// разные по знаку dt обрабатываются по-разному в случае, если есть
	// события с "перепутанными" ts (маленькие чередуются с большими,
	// на грани переполнения)
	if (dt<0) {
	  // prnt("ss l l l l f ls;",BRED,"ts:",nevents,eh->ts,evt->Tstmp,
	  //      Pstamp64,evt->Tstmp*opt.Period*1e-9,0x7FFFFFFF,RST);
	  evt->Tstmp += 0x100000000;
	  // prnt("ss l l l l fs;",BBLU,"ts:",nevents,eh->ts,evt->Tstmp,
	  //      Pstamp64,evt->Tstmp*opt.Period*1e-9,RST);
	}
	else {
	  // prnt("ss l l l l f ls;",BMAG,"ts:",nevents,eh->ts,evt->Tstmp,
	  //      Pstamp64,evt->Tstmp*opt.Period*1e-9,0x7FFFFFFF,RST);
	  evt->Tstmp -= 0x100000000;
	  // prnt("ss l l l l fs;",BCYN,"ts:",nevents,eh->ts,evt->Tstmp,
	  //      Pstamp64,evt->Tstmp*opt.Period*1e-9,RST);
	}
      }
      // else {
      //   prnt("ss l d l ls;",BGRN,"ts:",nevents,eh->ts,evt->Tstmp,T_prev,RST);
      // }

      //prnt("ss l d d l ls;",BGRN,"ts:",nevents,eh->np,eh->ts,evt->Tstmp,Pstamp64,RST);

      Pstamp64 = evt->Tstmp;

      for (int n = 0; n < eh->np; n++) {
	struct stor_puls_t *hit = (struct stor_puls_t *) (GLBuf+dp + sizeof(struct stor_ev_hdr_t) + n * sizeof(struct stor_puls_t));
	pulse_vect::iterator itpls =
	  evt->pulses.insert(evt->pulses.end(),PulseClass());

	ipls = &(*itpls);
	ipls->Chan = hit->ch;
	ipls->Area = hit->a;
	ipls->Time = hit->t/10.0 //opt.Period //?? in ns -> in samples (10/16ns error in adcm format)
	  + opt.sD[ipls->Chan]; //in ns (not in samples)
	ipls->Width = hit->w;
	ipls->Pos = ipls->Time;
	if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	  //prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	  evt->T0=ipls->Time;
	}
	ipls->Ecalibr(ipls->Area);
      }
      break;
    }
    case ID_CNTR:
      ncnt++;
      // memcpy (&counters, &buf[dp], sizeof (struct adcm_counters_t));
      break;
    default:
      prnt("ss x s ls;",BRED,"Bad block ID:",hdr->id, "at pos:",p,RST);
      //fprintf (stderr, "Bad block ID %04X at pos %zu\n", hdr->id, p);
      return;
    }
    p += hdr->size;

    //PulseAna(ipls);
    //Event_Insert_Pulse(Blist,&ipls);

  } //while... (main event loop)

  //Dec_End(Blist,iread,255);

  //fprintf (stderr, "Complete %lld bytes, %u events, %u counters, %u maps\n", p, nev, ncnt, nmap);

  // if (!ipls.ptype) {
  //   PulseAna(ipls);
  //   Event_Insert_Pulse(Blist,&ipls);
  // }

  Dec_End(Blist,iread,254);

} //Decode_adcm_dec

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

void CRS::Print_OneEvent(EventClass* evt) {

  *txt_out << "--- Event: " << evt->Nevt << " M: " << evt->pulses.size() << " Tstamp: " << evt->Tstmp << endl;
  for (UInt_t i=0;i<evt->pulses.size();i++) {
    PulseClass pp = evt->pulses.at(i);
    *txt_out << "Ch: " << (int)pp.Chan << " Tstamp: " << pp.Tstamp64;
    // for (std::vector<PeakClass>::iterator pk=pp.Peaks.begin();
    // 	   pk!=pp.Peaks.end();++pk) {
    *txt_out << " Pk: " << pp.Time << " " << pp.Area << " " << pp.Width;
    //}
    *txt_out << endl;

    for (int j=0;j<(int)pp.sData.size();j++) {
      //*txt_out << j << " " << pp.sData[j] << endl;
      *txt_out << pp.sData[j] << " ";
    }
    *txt_out << endl;

  }

}

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
      // for (std::vector<PeakClass>::iterator pk=pp.Peaks.begin();
      // 	   pk!=pp.Peaks.end();++pk) {
      out << " Pk: " << pp.Time << " " << pp.Area << " " << pp.Width;
      //}
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

  out << "Nevt Chan Tstamp(ns) Area Time" << endl;
  for (std::list<EventClass>::iterator it=Levents.begin();
       it!=Levents.end();++it) {
    for (UInt_t i=0;i<it->pulses.size();i++) {
      PulseClass pp = it->pulses.at(i);
      // for (std::vector<PeakClass>::iterator pk=pp.Peaks.begin();
      // 	   pk!=pp.Peaks.end();++pk) {
      out << it->Nevt << " " << (int)pp.Chan << " " << it->Tstmp << " " << pp.Tstamp64*int(opt.Period) << " " << pp.Area << " " << pp.Time << endl;
      //}
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

void CRS::Print_Buf_err(UInt_t ibuf, const char* file) {
  //использовалось для поиска ошибки со сдвигом на 4 байта

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

  Long64_t idx1=b_start[ibuf];

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

void CRS::Print_Buf8(UChar_t* buf, Long64_t size, const char* file) {

  ULong64_t* buf8 = (ULong64_t*) buf;
  Long64_t idx8=0;

  ULong64_t data;
  UChar_t frmt,ch;

  while (idx8<size/8) {
    frmt = buf[idx8*8+6];
    frmt = (frmt & 0xF0)>>4;
    ch = buf[idx8*8+7];
    data = buf8[idx8] & sixbytes;
    
    printf("%6lld %4d %3d %16lld %16llx\n",idx8,ch,frmt,data,buf8[idx8]);
    //printf("%lld %lld %lld %lld\n",idx8,size,size/8,buf8[idx8]);
    ++idx8;
    // if (idx8>1000)
    //   exit(1);
  }
}

void CRS::Event_Insert_Pulse(eventlist *Elist, PulseClass* pls) {
  // вставляем импульс в список событий, анализируя окно совпадений
  // при необходимости, создается новое событие

  event_iter it;
  event_reviter rit;
  //event_iter it_last;
  Long64_t dt;

  if (pls->ptype) { //any bad pulse
    // cout << "bad pulse: " << (int) pls->ptype << " " << (int) pls->Chan
    // 	 << " " << pls->Counter << " " << pls->Tstamp64 << endl;
    if (pls->Chan<254)
      npulses_bad[pls->Chan]++;
    return;
  }

  if (!(pls->Spin&128)) {
    ++npulses2[pls->Chan];
  }

  // if (pls->Chan==4)
  //   prnt("ss d l ds;",BMAG,"ev_ins_pls:",pls->Chan,pls->Tstamp64,pls->Spin,RST);

  //YK1 prnt("ssl fs;",BRED,"Ts1: ",pls->Tstamp64,pls->Time,RST);

  /*
  // оставляем только дробную часть в pls->Time, остальное загоняем в Tstamp64
  int i_dt = pls->Time;
  pls->Time -= i_dt;
  pls->Simul2 -= i_dt;
  pls->Tstamp64+=i_dt;
  */

  // KK - удалить эту строчку:
  Long64_t T64 = pls->Tstamp64+Long64_t(opt.sD[pls->Chan]/opt.Period);

  // ищем совпадение от конца списка до начала, но не больше, чем opt.ev_min
  int nn=0;
  //for (it=--Elist.end();it!=m_event && nn>0 ;--it,--nn) {
  for (rit=Elist->rbegin();rit!=Elist->rend();++rit,++nn) {
    if (nn>=opt.ev_min) { //event lag exceeded
      //вставляем новое событие в конец

      //cout << "lag: " << nevents << " " << Elist->size() << endl;
      //it=Elist->insert(rit.base(),EventClass());
      it=Elist->insert(Elist->rbegin().base(),EventClass());
      it->Nevt=nevents;
      it->AddPulse(pls);
      nevents++;
      ++errors[ER_LAG];//event lag exceeded

      // prnt("s",BGRN);
      // it->PrintEvent(0);

      // int nn2=0;
      // for (event_reviter rit2=Elist->rbegin();rit2!=Elist->rend() && nn2<4;++rit2,++nn2) {
      // 	rit2->PrintEvent(0);
      // }
      // prnt("s",RST);

      return;
    }
    dt = (T64 - rit->Tstmp);
    //KK dt = (pls->Tstamp64 - rit->Tstmp);

    if (dt > opt.tgate) {
      //pls пришел позже, чем tgate -> добавляем новое событие в конец
      //add new event AFTER rit.base()
      //prnt("sss;",BYEL,"Ev_new0:",RST);
      it=Elist->insert(rit.base(),EventClass());
      it->Nevt=nevents;
      it->AddPulse(pls);
      nevents++;
      //prnt("ss d fs;",BGRN,"Ev_new:",it->pulses.size(),it->T0,RST);



      // it->PrintEvent(0);

      // int nn2=0;
      // for (event_reviter rit2=Elist->rbegin();rit2!=Elist->rend() && nn2<4;++rit2,++nn2) {
      // 	rit2->PrintEvent(0);
      // }



      return;
    }
    else if (TMath::Abs(dt) <= opt.tgate) { //add pls to existing event
      // coincidence event

      //if ((rit->Spin&128) != (pls->Spin&128)) {
      //prnt("ss l d l ds;",KGRN,"Spin128:",rit->Tstmp,rit->Spin,
      //pls->Tstamp64,pls->Spin,RST);
      //}

      //prnt("sss;",BYEL,"Ev_add0:",RST);
      rit->AddPulse(pls);
      //prnt("sss;",BYEL,"Ev_add1:",RST);
      return;
    }
  }

  // дошли до начала списка; вставляем новый event в начало 
  it=Elist->insert(rit.base(),EventClass());
  it->Nevt=nevents;
  it->AddPulse(pls);
  nevents++;
  



  //cout << "Lag1: " << rit->Tstmp << " " << rit.base()->Tstmp << " " << Elist->size() << " " << pls->Tstamp64 << endl;

  //++errors[ER_LAG];//event lag exceeded

  // rit=Elist->rbegin();
  // it=Elist->begin();
  // dt = rit->Tstmp - pls->Tstamp64;
  // cout << "!!! beginning !!! ---: "
  //      << nevents << " " << pls->Tstamp64 << " "
  //      << Elist->size() << " "
  //      << rit->Tstmp << " "
  //      << it->Tstmp << " "
  //      << dt
  //      << endl;

  // nn=opt.ev_min;
  // for (rit=Elist->rbegin();rit!=Elist->rend() && nn>0 ;++rit,--nn) {
  //   dt = (pls->Tstamp64 - rit->Tstmp);
  //   cout << nn << " " << dt << " " << pls->Tstamp64 << " " << rit->Tstmp << endl;
  // }

  // if the current event is too early, insert it at the end of the event list
  // it=Elist->insert(Elist->end(),EventClass());
  // it->Nevt=nevents;
  // it->AddPulse(pls);
  // nevents++;

} //Event_Insert_Pulse

//void CRS::Make_Events(plist_iter it) {
void CRS::Make_Events(std::list<eventlist>::iterator BB) {

  // cout << "Make_Events: T_acq: " << gl_ivect << " " << opt.T_acq
  //      << " " << crs->Tstart64 << " " << Levents.back().T0
  //      << " " << Levents.empty()
  //      << " " << BB->empty()
  //      << endl;
	
  // for (event_iter it=BB->begin(); it!=BB->end(); ++it) {
  //   prnt("ss l l f ds;","BB:",BGRN,it->Nevt,it->Tstmp,it->T0,it->Spin,RST);
  // }

  if (opt.Tstop && opt.T_acq>opt.Tstop) {
    //if (b_acq) {
      //myM->DoStartStop();
      //myM->fStart->Emit("Clicked()");
      //myM->fStart->Clicked();
    //}
    //else
    if (b_fana) {
      crs->b_fana=false;
      crs->b_stop=true;
    }
    //return;
  }

  int spn=0;
  if (!BB->empty()) {
    spn = BB->back().Spin;
    BB->pop_back();
  }

  if (!Levents.empty() && !BB->empty() && (spn==255)) {

    //merge beginning of BB and end of Levents
    evlist_iter it = BB->begin();
    Long64_t T_last = Levents.rbegin()->Tstmp + opt.tgate;

    //while (it!=BB->end() && it->Tstmp - rr->Tstmp<=opt.tgate*2) {
    //cout << "it: " << it->Tstmp << " " << T_last << endl;
    while (it!=BB->end() && it->Tstmp <= T_last) {
      for (UInt_t i=0;i<it->pulses.size();i++) {
	Event_Insert_Pulse(&Levents,&it->pulses[i]);
      }
      it=BB->erase(it);
    }
  }

  Levents.splice(Levents.end(),*BB);

  // for (event_iter it=Levents.begin(); it!=Levents.end(); ++it) {
  //   prnt("ss l l f d ls;","Le:",BRED,it->Nevt,it->Tstmp,it->T0,it->Spin,it->pulses.size(),RST);
  // }

  Bufevents.erase(BB);
  //++buf_erase;

  //m_end = crs->Levents.end();
  //std::advance(m_end,-opt.ev_min);  
}

/*
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
*/
void CRS::Reset_Raw() {
  sprintf(raw_opt,"wb%d",opt.raw_compr);

  f_raw = gzopen(crs->rawname.c_str(),raw_opt);
  if (f_raw) {
    cout << "Writing parameters... : " << crs->rawname.c_str() << " " << module << endl;
    if (opt.fProc) {
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
  rw_list.clear();
}

void CRS::Reset_Dec(Short_t mod) {
  sprintf(dec_opt,"wb%d",opt.dec_compr);

  if (cpar.Trigger==1) //trigger on START channel
    mod=80;

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

  DecBuf=DecBuf_ring;
  DecBuf8 = (ULong64_t*) DecBuf;
  idec=0;
  decw_list.clear();

  // mdec1=0;
  // mdec2=0;
  // memset(b_decwrite,0,sizeof(b_decwrite));
}

void CRS::Reset_Txt() {

  if (txt_out) {
    delete txt_out;
    txt_out=0;
  }

  if (txt_of.is_open()) {
    txt_of.close();
  }

  //SplitFilename(string(crs->Fname),dir,name,ext);
  //txtname=dir;
  //txtname.append(name);

  string txtname = opt.Filename;
  txtname.append(".txt");
  //cout << "txt_file: " << txtname << " " << txtname.size() << endl;

  std::streambuf * buf;
  //std::ofstream of;

  if (txtname.size()>4) { //4=".txt"
    txt_of.open(txtname);
    buf = txt_of.rdbuf();
    cout << "txt_file: " << txtname << " " << txtname.size() << endl;
  }
  else {
    buf = std::cout.rdbuf();
    cout << "txt_file: " << "cout" << " " << txtname.size() << endl;
  }
  txt_out = new std::ostream(buf);
  //std::ostream out(buf);
  //*txt_out << "--- Event: " << endl;
  //exit(-1);
}

void CRS::Fill_Dec79(EventClass* evt) {
  //Fill_Dec79 - the same as 78, but different factor for Area

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - Spin
  // 2a) if Spin7(bit7):
  //  N 8-byte words, each containing one pulse counter
  //    bytes 0-5 - Counter
  //    byte 7 - channel
  // 2b) else (!Spin7(bit7)):
  //  N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - (unsigned) Area*5+1
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel
  //    7 bits - amplitude

  /*
  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->Spin & 1) {
    *DecBuf8 |= 0x1000000000000;
  }
  */


  *DecBuf8 = 0x8000 | evt->Spin;
  *DecBuf8<<=48;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  //if (evt->Spin & 1) {
  //*DecBuf8 |= 0x1000000000000;
  //}



  //prnt("ss d l x ls;",BGRN,"Dec:",nevents,evt->Tstmp,*DecBuf8,(UChar_t*)DecBuf8-DecBuf,RST);
  // if ((UChar_t*)DecBuf8==DecBuf) {
  //   prnt("ss d l x ls;",BGRN,"Dec:",nevents,evt->Tstmp,*DecBuf8,(UChar_t*)DecBuf8-DecBuf,RST);
  // }
  ++DecBuf8;

  if (evt->Spin & 128) { //Counters
    //prnt("ss ls;",BRED,"Counter:",evt->Tstmp,RST);
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      //prnt("ss d d ls;",BGRN,"Ch:",ipls->Chan,ipls->Pos,ipls->Counter,RST);
      *DecBuf8=ipls->Counter;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[3] = ipls->Chan;
      ++DecBuf8;
    }
  }
  else { //Peaks
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      if (ipls->Pos>-32222) {
	*DecBuf8=0;
	Short_t* Decbuf2 = (Short_t*) DecBuf8;
	UShort_t* Decbuf2u = (UShort_t*) Decbuf2;
	UChar_t* Decbuf1u = (UChar_t*) DecBuf8;
	if (ipls->Area<0) {
	  *Decbuf2u = 0;
	}
	else if (ipls->Area>13106){
	  *Decbuf2u = 65535;
	}
	else {
	  *Decbuf2u = ipls->Area*5+1;
	}
	if (ipls->Time>327.6)
	  Decbuf2[1] = 32767;
	else if (ipls->Time<-327)
	  Decbuf2[1] = -32767;
	else
	  Decbuf2[1] = ipls->Time*100;

	if (ipls->Width>32.76)
	  Decbuf2[2] = 32767;
	else if (ipls->Width<-32.76)
	  Decbuf2[2] = -32767;
	else
	  Decbuf2[2] = ipls->Width*1000;

	Decbuf2[3] = ipls->Chan;
	if (ipls->Height<0)
	  Decbuf1u[7] = 0;
	else
	  Decbuf1u[7] = ((int)ipls->Height)>>8;
	//cout << evt->Nevt << " " << evt->Tstmp << " " << (int) evt->pulses[i].Chan << endl;
	++DecBuf8;
      }
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    //levt=*evt;
    //CRS::eventlist Blist;
    //D79(DecBuf,idec,Blist);
    //ULong64_t* buf8 = (ULong64_t*) (GLBuf);
    //prnt("s l l l l x;", "Flush:", levt.Tstmp, Blist.size(),
    // Blist.front().Tstmp, Blist.back().Tstmp, *buf8);
    //cout << "Flush: " << levt.Tstmp << endl;
    Flush_Dec();
  }

} //Fill_Dec79

void CRS::Fill_Dec80(EventClass* evt) {
  //Fill_Dec80 - for START trigger type

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - Spin
  // 2) N 8-byte words, each containing one sample
  //    (low) 4 bytes - (Int_t) data
  //    (high) 4 bytes - (Int_t) channel

  *DecBuf8 = 1;
  *DecBuf8<<=63;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  if (evt->Spin & 1) {
    *DecBuf8 |= 0x1000000000000;
  }

  ++DecBuf8;

  //prnt("ss l l ls;",BBLU,"Dc:",evt->Nevt,evt->Tstmp,evt->pulses.size(),RST);

  for (pulse_vect::iterator ipls=evt->pulses.begin();
       ipls!=evt->pulses.end();++ipls) {

    //prnt("ss d ls;",BMAG,"pls:",ipls->Chan,ipls->sData.size(),RST);

    for (UInt_t i=0;i<ipls->sData.size();++i) {

      Int_t* DecBuf4 = (Int_t*) DecBuf8;
      DecBuf4[0]=ipls->sData[i];
      DecBuf4[1]=ipls->Chan;
      ++DecBuf8;

    }
    
  }

  //prnt("ss l l ls;",BBLU,"Dc:",evt->Nevt,evt->Tstmp,evt->pulses[1].sData.size(),RST);

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    //levt=*evt;
    //CRS::eventlist Blist;
    //D79(DecBuf,idec,Blist);
    //ULong64_t* buf8 = (ULong64_t*) (GLBuf);
    //prnt("s l l l l x;", "Flush:", levt.Tstmp, Blist.size(),
    // Blist.front().Tstmp, Blist.back().Tstmp, *buf8);
    //cout << "Flush: " << levt.Tstmp << endl;
    Flush_Dec();
  }

} //Fill_Dec80

// int sgn(int a) {
// }

// int sgn(int a) {
// }





void CRS::Fill_Dec81(EventClass* evt) {

  cout << "Fill_dec81" << endl;
  ULong64_t* Dec0 = DecBuf8; //запоминаем начальный адрес буфера
  ULong64_t* DecN = 0; //адрес буфера для записи длины
  Short_t* DecBuf2;
  UShort_t* UDecBuf2;

  *DecBuf8 = 0x8000000000000000; //признак начала события

  // общие параметры события
  for (auto it=sdec_e.begin(); it!=sdec_e.end(); ++it) {
    switch (*it) {
    case 'T':
      *DecBuf8 |= ((ULong64_t) (evt->Spin & 7)) << 48; //нижние 3 бита
      *DecBuf8 |= evt->Tstmp & sixbytes;
      *(++DecBuf8)=0;
      break;
    case 'N':
      UDecBuf2 = (UShort_t*) DecBuf8;
      UDecBuf2[0] = evt->pulses.size();
      UDecBuf2[1] = evt->Nevt;
      DecN = DecBuf8; //запоминаем, куда писать длину события
      *(++DecBuf8)=0;
      break;
    }
  }

  // в событии либо счетчики, либо пики
  if ((evt->Spin & 128) && sdec_c) { //Counters
    for (auto ipls=evt->pulses.begin(); ipls!=evt->pulses.end(); ++ipls) {
      *DecBuf8=ipls->Counter & sixbytes;
      DecBuf2 = (Short_t*) DecBuf8;
      DecBuf2[3]=0x4000|ipls->Chan; //0x4000: признак счетчиков
      //DecBuf2[3]=0x4000; //признак счетчиков
      //DecBuf2[3]|=ipls->Chan;
      *(++DecBuf8)=0;
    }
  }
  else { //Peaks
    for (auto ipls=evt->pulses.begin(); ipls!=evt->pulses.end(); ++ipls) {
      int pos=0; //position in DecBuf8
      DecBuf2 = (Short_t*) DecBuf8;
      for (auto it=sdec_p.begin(); it!=sdec_p.end(); ++it) {
	switch (*it) {
	case 'A':
	  DecBuf2[pos] = ipls->Area;
	  break;
	case 't':
	  DecBuf2[pos] = ipls->Time;
	  break;
	case 'W':
	  DecBuf2[pos] = ipls->Width;
	  break;
	case 'H':
	  DecBuf2[pos] = ipls->Height;
	  break;
	case 'B':
	  DecBuf2[pos] = ipls->Base;
	  break;
	case 'S':
	  DecBuf2[pos] = ipls->Sl1;
	  break;
	case 's':
	  DecBuf2[pos] = ipls->Sl2;
	  break;
	case 'R':
	  DecBuf2[pos] = ipls->RMS1;
	  break;
	case 'r':
	  DecBuf2[pos] = ipls->RMS2;
	  break;
	case 'p':
	  DecBuf2[pos] = ipls->ptype;
	  break;
	} //switch
	pos++;
	if (pos>=3) {//слово заполнено, записываем Chan
	  DecBuf2[3]|=ipls->Chan;
	  *(++DecBuf8)=0;
	  DecBuf2 = (Short_t*) DecBuf8;
	  pos=0;
	}
      } // for sdec
      if (pos) {//значит, слово неполное, заканчиваем запись импульса
	DecBuf2[3]|=ipls->Chan;
	*(++DecBuf8)=0;
	DecBuf2 = (Short_t*) DecBuf8;
	pos=0;
      }
      if (sdec_d) { //записываем sData (в этой точке pos всегда 0)
	for (auto j=ipls->sData.begin(); j!=ipls->sData.end(); ++j) {
	  DecBuf2[pos] = *j;
	  pos++;
	  if (pos>=3) {
	    DecBuf2[3]|=ipls->Chan;
	    *(++DecBuf8)=0;
	    DecBuf2 = (Short_t*) DecBuf8;
	    pos=0;
	  }
	}
	if (pos) {//значит, слово неполное
	  DecBuf2[3]|=ipls->Chan;
	  *(++DecBuf8)=0;
	  //DecBuf2 = (Short_t*) DecBuf8;
	  //pos=0;
	}
      } //if (sdec_d)
    } //for ipls
  }

  // в конце записываем длину события, если надо
  if (DecN) {
    ULong64_t len = DecBuf8 - Dec0;
    *DecN |= ((len & 0xFFFFFF) << 32);
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    //levt=*evt;
    //CRS::eventlist Blist;
    //D79(DecBuf,idec,Blist);
    //ULong64_t* buf8 = (ULong64_t*) (GLBuf);
    //prnt("s l l l l x;", "Flush:", levt.Tstmp, Blist.size(),
    // Blist.front().Tstmp, Blist.back().Tstmp, *buf8);
    //cout << "Flush: " << levt.Tstmp << endl;
    Flush_Dec();
  }

} //Fill_Dec81


/*
void CRS::Fill_Dec81a(EventClass* evt) {
  //Fill_Dec81 - the same as 79 plus sData if 

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - Spin
  // 2a) if Spin7(bit7):
  //  N 8-byte words, each containing one pulse counter
  //    bytes 0-5 - Counter
  //    byte 7 - channel
  // 2b) else (!Spin7(bit7)):
  //  N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - (unsigned) Area*5+1
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel
  //    7 bits - amplitude
  //
  //      if opt.Pls -> Spin6(bit6) will be set:
  //      N 8-byte words, each containing one sample
  //      (low) 4 bytes - (Float_t) sData
  //      (high) 4 bytes - (Int_t) channel



  *DecBuf8 = 0x8000 | evt->Spin;
  *DecBuf8<<=48;
  *DecBuf8 |= evt->Tstmp & sixbytes;
  // if (evt->Spin & 1) {
  // *DecBuf8 |= 0x1000000000000;
  // }

  ++DecBuf8;

  if (evt->Spin & 128) { //Counters
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      *DecBuf8=ipls->Counter;
      Short_t* Decbuf2 = (Short_t*) DecBuf8;
      Decbuf2[3] = ipls->Chan;
      ++DecBuf8;
    }
  }
  else { //Peaks
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      if (ipls->Pos>-32222) {
	*DecBuf8=0;
	Short_t* Decbuf2 = (Short_t*) DecBuf8;
	UShort_t* Decbuf2u = (UShort_t*) Decbuf2;
	UChar_t* Decbuf1u = (UChar_t*) DecBuf8;
	if (ipls->Area<0) {
	  *Decbuf2u = 0;
	}
	else if (ipls->Area>13106){
	  *Decbuf2u = 65535;
	}
	else {
	  *Decbuf2u = ipls->Area*5+1;
	}
	if (ipls->Time>327.6)
	  Decbuf2[1] = 32767;
	else if (ipls->Time<-327)
	  Decbuf2[1] = -32767;
	else
	  Decbuf2[1] = ipls->Time*100;

	if (ipls->Width>32.76)
	  Decbuf2[2] = 32767;
	else if (ipls->Width<-32.76)
	  Decbuf2[2] = -32767;
	else
	  Decbuf2[2] = ipls->Width*1000;

	Decbuf2[3] = ipls->Chan;
	if (ipls->Height<0)
	  Decbuf1u[7] = 0;
	else
	  Decbuf1u[7] = ((int)ipls->Height)>>8;
	//cout << evt->Nevt << " " << evt->Tstmp << " " << (int) evt->pulses[i].Chan << endl;
	++DecBuf8;

	if (opt.Pls[ipls->Chan]) {
	  
	}
      }
    }
  }

  idec = (UChar_t*)DecBuf8-DecBuf;
  if (idec>DECSIZE) {
    //levt=*evt;
    //CRS::eventlist Blist;
    //D79(DecBuf,idec,Blist);
    //ULong64_t* buf8 = (ULong64_t*) (GLBuf);
    //prnt("s l l l l x;", "Flush:", levt.Tstmp, Blist.size(),
    // Blist.front().Tstmp, Blist.back().Tstmp, *buf8);
    //cout << "Flush: " << levt.Tstmp << endl;
    Flush_Dec();
  }

} //Fill_Dec81a
*/








void CRS::Fill_Dec82(EventClass* evt) {
  // эти параметры должны быть заданы в opt
  const bool bit_area=true;
  const bool bit_time=true;
  const bool bit_width=true;

  const Float_t DA=1;
  const Float_t DT=100;
  const Float_t DW=10000;

  //Fill_Dec81

  // fill_dec is not thread safe!!! (??? - not sure)


  //format of decoded data:
  // --- 16 bytes global header:
  // 1 byte: format of area/time/width
  //   bit0=1 - write area
  //   bit1=1 - write time
  //   bit2=1 - write width
  // 3 bytes - reserved
  // 4 bytes: float DA - constant for converting area (see peak area below)
  // 4 bytes: float DT - constant for converting time (see peak time below)
  // 4 bytes: float DW - constant for converting width (see peak width below)


  // Event header byte (2 lower bits):
  // = 01b: next 12 bytes - counters
  // = 10b: bit2=spin + next 6 bytes - tstamp     -> event start
  // = 00b: next 7 bytes - peak
  

  //format of decoded counters (header=01b):
  // header1 (1byte) = 01b
  //   1byte - channel1
  //   6bytes - tstamp (in nsec)
  //   6bytes - counter
  // header1 (1byte) = 01b
  //   1byte - channel2
  //   6bytes - tstamp (in nsec)
  //   6bytes - counter
  // header1 (1byte) = 01b
  //   1byte - channel3
  // ...


  //format of decoded event (header=10b):
  // -- event start --
  // header10 (1byte) = 10b + bit2=spin
  // tstamp (6bytes) (in nsec)
  // -- N peaks... 
  // header00 (1byte) = 00 (binary) -- peak1
  // ch (1byte)
  // area  (2bytes, optional)
  // time  (2bytes, optional)
  // width (2bytes, optional)
  // header00 (1byte) = 00 (binary) -- peak2
  // ch (1byte)
  // area  (2bytes, optional)
  // time  (2bytes, optional)
  // width (2bytes, optional)
  // ...
  // header00 (1byte) = 00 (binary) -- peak3
  // ...
  
  // peak area:  2bytes (Area*DA  (for DA - see global header))
  // peak time:  2bytes (Time*DT  (for DT - see global header))
  // peak width: 2bytes (Width*DW (for DW - see global header))

  *DecBuf1 = (evt->Spin&1) << 2; //spin
  *DecBuf1 |= 2; //header: binary 10 - event_start
  ++DecBuf1;

  DecBuf8 = (ULong64_t*) DecBuf1;
  *DecBuf8 = evt->Tstmp & sixbytes;

  DecBuf1+=6;

  for (pulse_vect::iterator ipls=evt->pulses.begin();
       ipls!=evt->pulses.end();++ipls) {
    if (ipls->Pos>-32222) {

      *DecBuf1 = 0;
      ++DecBuf1;
      *DecBuf1 = ipls->Chan;
      ++DecBuf1;

      Short_t* DecBuf2 = (Short_t*) DecBuf1;
      
      if (bit_area) {
	int X = ipls->Area*DA;
	(abs(X)<MAXSHORT) ? *DecBuf2 = X : *DecBuf2 = MAXSHORT*sgn(X);
	++DecBuf2;
      }
      if (bit_time) {
	int X = ipls->Time*DT;
	(abs(X)<MAXSHORT) ? *DecBuf2 = X : *DecBuf2 = MAXSHORT*sgn(X);
	++DecBuf2;
      }
      if (bit_width) {
	int X = ipls->Width*DW;
	(abs(X)<MAXSHORT) ? *DecBuf2 = X : *DecBuf2 = MAXSHORT*sgn(X);
	++DecBuf2;
      }

      DecBuf1 = (UChar_t*) DecBuf2;

    }
  }

  idec = DecBuf1-DecBuf;
  if (idec>DECSIZE) {
    Flush_Dec();
  }

} //Fill_Dec82

int CRS::Wr_Dec(UChar_t* buf, int len) {
  //return >0 if error
  //       0 - OK

  sprintf(dec_opt,"ab%d",opt.dec_compr);
  f_dec = gzopen(decname.c_str(),dec_opt);
  if (!f_dec) {
    cout << "Can't open file: " << decname.c_str() << endl;
    f_dec=0;
    opt.dec_write=false;
    //idec=0;
    return 1;
  }

  int res=gzwrite(f_dec,buf,len);
  if (res!=len) {
    cout << "Error writing to file: " << decname.c_str() << " " 
	 << res << " " << len << endl;
    decbytes+=res;
    opt.dec_write=false;
    gzclose(f_dec);
    f_dec=0;
    return 2;
  }
  decbytes+=res;

  gzclose(f_dec);
  f_dec=0;
  return 0;

}

/*
void CRS::Flush_Dec_old() {

  //idec=0;
  //return;

  cout << "Flush_dec_old: " << crs->decname.c_str() << " " << idec << endl;

  if (!idec) return;
  sprintf(dec_opt,"ab%d",opt.dec_compr);
  f_dec = gzopen(crs->decname.c_str(),dec_opt);
  if (!f_dec) {
    cout << "Can't open file: " << crs->decname.c_str() << endl;
    opt.dec_write=false;
    idec=0;
    return;
  }

  int res=gzwrite(f_dec,DecBuf,idec);
  if (res!=idec) {
    cout << "Error writing to file: " << crs->decname.c_str() << " " 
	 << res << " " << idec << endl;
    decbytes+=res;
    opt.dec_write=false;
    return;
  }
  idec=0;
  decbytes+=res;

  gzclose(f_dec);
  f_dec=0;
  DecBuf8 = (ULong64_t*) DecBuf;

}
*/
void CRS::Flush_Dec() {

  if (opt.nthreads==1) { //single thread
    Wr_Dec(DecBuf,idec);
    DecBuf8 = (ULong64_t*) DecBuf;
    DecBuf1 = DecBuf;
    idec=0;
  }
  else { //multithreading
    Pair p(DecBuf,idec);

    decw_mut.Lock();

    decw_list.push_back(p);
    static int rep=0;
    rep++;
    if (rep>9) {
      prnt("s d x d l l;","Flush_dec: ",decw_list.size(),DecBuf,idec,DecBuf-DecBuf_ring,DecBuf_ring+DECSIZE*NDEC-DecBuf);
      rep=0;
    }

    decw_mut.UnLock();

    DecBuf+=idec;
    if (DecBuf_ring+DECSIZE*NDEC-DecBuf<2*DECSIZE) {
      prnt("ssds;",BYEL,"---end of DecBuf---: ",NDEC,RST);
      DecBuf=DecBuf_ring;
    }
    DecBuf8 = (ULong64_t*) DecBuf;
    DecBuf1 = DecBuf;
    idec=0;
    
  }

  /*
    if (m1==0) {
    cout << "YK7: " << crs->decname.c_str() << " " << mdec1 << " " << mdec2 << " " << mdec1-mdec2 << " " << Levents.size() << " " << Bufevents.size() << endl;
    //" " << buf_inits << " " << buf_erase << endl;

    //buf_inits=0;
    //buf_erase=0;
  }

  //gSystem->Sleep(1000);
  dec_len[m1]=idec;
  CRS::eventlist Blist;
  D79(DecBuf,crs->dec_len[m1],Blist);

  prnt("ss d d d l l sx xs;",KBLU,"yyy:", mdec1, m1, Blist.size(),
       Blist.front().Tstmp, Blist.back().Tstmp, BYEL,
       DecBuf, *(ULong64_t*)DecBuf, RST);

  b_decwrite[m1]=true;

  // cout << "yyy: " << mdec1 << " " << m1 << " " << crs->b_decwrite[m1]
  //      << " " << crs->mtrig << endl;
  ++mdec1;

  DecBuf=DecBuf_ring+m1*2*DECSIZE;
  DecBuf8 = (ULong64_t*) DecBuf;
  idec=0;
  */
}

void P_buf8(int id,ULong64_t* buf8) {
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
    RawBuf[iraw+13]=ipls->Spin;
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
    opt.raw_write=false;
    iraw=0;
    return;
  }

  int res=gzwrite(f_raw,RawBuf,iraw);
  if (res!=iraw) {
    cout << "Error writing to file: " << crs->rawname.c_str() << " " 
	 << res << " " << iraw << endl;
    rawbytes+=res;
    opt.raw_write=false;
    return;
  }
  iraw=0;
  rawbytes+=res;

  gzclose(f_raw);
  f_raw=0;

}

void CRS::Flush_Raw_MT(UChar_t* buf, int len) {

  Pair p;
  p.first=buf;
  p.second=len;
  
  raw_mut.Lock();
  rw_list.push_back(p);
  prnt("s d d;","Flush_raw: ",rw_list.size(),buf-GLBuf);
  raw_mut.UnLock();

}

/*
double Pshape_Gaus(int j, double pos) {
  // return opt.SimSim[4]*(-TMath::Gaus(j,pos-10,opt.SimSim[5]/opt.Period,1) +
  //  			TMath::Gaus(j,pos,opt.SimSim[5]/opt.Period,1));
  return opt.SimSim[4]*TMath::Gaus(j,pos,opt.SimSim[5]/opt.Period,1);
}

double Pshape_RC(int j, double pos) {
  double lam=0;
  if (opt.SimSim[6]) lam=opt.Period/opt.SimSim[6];

  if (j<pos)
    return 0;
  else if (j<=pos+opt.SimSim[5]/opt.Period)
    return opt.SimSim[4]*(1-exp((pos-j)*lam));
  else
    return opt.SimSim[4]*(1-exp(-opt.SimSim[5]/opt.Period*lam))
      * exp((pos+opt.SimSim[5]/opt.Period-j)*lam);
}

void CRS::SimulateInit() {
  //if (opt.Nchan<4)
    opt.Nchan=4;

  opt.Period = opt.SimSim[0];

  for (int i=0;i<MAX_CHTP;i++) {
    cpar.Pre[i]=opt.SimSim[1];
    cpar.Len[i]=opt.SimSim[2];
    opt.St[i]=0;
  }

  opt.St[0]=1;

  //prnt("ss ds;",BGRN,"SimInit:",opt.h_time.b,RST);
  SimNameHist();

}

void CRS::SimNameHist() {

  //cout << "smodule=" << module << endl;

  for (auto it = hcl->MFill_list.begin();it!=hcl->MFill_list.end();++it) {
    if ((*it)->name.EqualTo("time",TString::kIgnoreCase)) {
      for (auto map = (*it)->v_map.begin();map!=(*it)->v_map.end();++map) {
	if (*map) {
	  if ((*map)->nn==2)
	    (*map)->hst->SetTitle("Exact time0 - pos0");
	  else if ((*map)->nn==3)
	    (*map)->hst->SetTitle("Exact time1 - time0");

	  // cout << "mflist: " << (*it)->hnum << " " << (*map)->nn
	  //      << " " << (*map)->hst->GetTitle() << endl;
	}
      }
    }
    
  }

  // if (hcl->m_time[2]) {
  //   hcl->m_time[2]->hst->SetTitle("Exact time0 - pos0");
  // }

  // if (hcl->m_time[3]) {
  //   hcl->m_time[3]->hst->SetTitle("Exact time1 - time0");
  // }

}


void CRS::SimulatePulse(int ch, Long64_t tst, double pos) {
  PulseClass ipls=PulseClass();
  ipls.Chan=ch;
  ipls.Tstamp64=tst;
  
  ipls.sData.resize(cpar.Len[ch]);
  if (opt.SimSim[3]==0) { //Gauss
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]=Pshape_Gaus(j,pos+cpar.Pre[ch]);
      //cout << i << " " << j << " " << evt->pulses[i].sData[j] << endl;
    }
  }
  else { //RC
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]=Pshape_RC(j,pos+cpar.Pre[ch]);
      //cout << i << " " << j << " " << evt->pulses[i].sData[j] << endl;
    }
  }

  PulseAna(ipls);
  // if (ipls.Chan==0)
  //   prnt("ss fs;",BRED,"Time1:",ipls.Time,RST);
  Event_Insert_Pulse(&Levents,&ipls);
  // if (ipls.Chan==0)
  //   prnt("ss fs;",BMAG,"Time2:",Levents.back().pulses[0].Time,RST);
}

void CRS::SimulateOneEvent(Long64_t Tst) {

  PulseClass pls;
  PulseClass *ipls;

  //time_00  - отклонение Time0 от pos0 (Time0-pos0)
  //simul_00 - отклонение Simul0 от pos0 (Simul0-pos0)
  //time_01  - разница между Time1 и Time0
  //simul_01 - разница между Simul1 и Simul0

  // реальная разница во времени между 2 импульсами (в ns)
  double delta = opt.SimSim[9]*gRandom->Rndm()-opt.SimSim[9]*0.5;
  delta/=opt.Period; //в сэмплах

  // положение p0 относительно дискриминатора (начала+Pre) (в нс)
  double pos0 = opt.SimSim[7]+opt.SimSim[8]*gRandom->Rndm();
  pos0/=opt.Period; //в сэмплах

  // положение p1 относительно p0 (в сэмплах)
  double pos1 = pos0 + delta;

  // (целая) разница между двумя импульсами в сэмплах
  Long64_t idelta = delta;
  pos1-=idelta;

  SimulatePulse(0, Tst, pos0);

  EventClass* evt = &Levents.back();
  ipls=&evt->pulses[0];
  //prnt("ss 9l 8.5f 8.5f 8.5f 8.5fs;",BWHT,"Pos0:",evt->Tstmp,evt->T0,pos0,ipls->Time-pos0,ipls->Simul2-pos0,RST);

  SimulatePulse(1, Tst+idelta, pos1);

  ipls=&pls;
  ipls->Chan=2;
  ipls->Tstamp64=Tst;
  ipls->Pos=evt->pulses[0].Pos;
  //ipls->Time=delta+evt->T0;
  ipls->Time=opt.SimSim[10]/opt.Period+pos0;
  //YK ipls->Simul2=pos0;//pos0+evt->T0;
  //prnt("ss f f fs;",BGRN,"Sim:",ipls->Time,pos0,pos1,RST);

  Event_Insert_Pulse(&Levents,ipls);

  ipls->Chan=3;
  ipls->Time=delta+evt->T0;

  Event_Insert_Pulse(&Levents,ipls);

}

void CRS::SimulateEvents(Long64_t n_evts, Long64_t Tst0) {
  for (int i=0;i<n_evts;i++) {
    SimulateOneEvent((Tst0*n_evts+i)*10000);
    //prnt("ss l l ls;",BGRN,"Sim:",i,Tst0,(Tst0*n_evts+i)*100000,RST);
  }
  //prnt("ss ls;",BGRN,"Sim:",(Tst0*n_evts),RST);
}
*/
