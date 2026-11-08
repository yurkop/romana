#include "libcusb.h"

#ifdef CYUSB

#include "libcrs.h"
#include "adcm_df.h"

// #include <iostream>
#include <chrono>
#include <iomanip>
#include <sys/mman.h>
#include <sys/stat.h>
#include <zlib.h>
#include <csignal>

// #ifdef CYUSB
// #include "cyusb.h"
// #endif

// #include <pthread.h>
#include "romana.h"
#include "decoder.h"
#include "libsock.h"

// #include <malloc.h>
#include <TCanvas.h>
#include <TClass.h>
#include <TDataMember.h>
#include <TSystem.h>

// #include <TSemaphore.h>
// TSemaphore sem;
#include "TMath.h"

#include "TApplication.h"
#include "TMutex.h"
#include "TRandom.h"
#include "TThread.h"

#include <bitset>

// int buf_inits=0;
// int buf_erase=0;

// TMutex Emut3;
extern TMutex stat_mut;
// TMutex ana_mut;

extern TMutex decode_mut;

// TMutex ringdec_mut;

const int BFMAX = 999999999;

#define ID_ADCM 0x2A50
#define ID_CMAP 0x504D
#define ID_EVNT 0x5645
#define ID_CNTR 0x5443

using namespace std;

extern bool b_comment;
extern MemInfo_t minfo;
extern ProcInfo_t pinfo;
// extern double rmem;

extern Decoder *decoder;

extern CRS *crs;
extern CUSB *cusb;
extern ERaw *eraw;
extern EDec *edec;
extern Coptions cpar;
extern Toptions opt;

extern EventFrame *EvtFrm;
extern MyMainFrame *myM;
extern HistFrame *HiFrm;
extern ErrFrame *ErrFrm;
extern HClass *hcl;
extern ParParDlg *parpar;
extern ChanParDlg *chanpar;
extern HistParDlg *histpar;

extern int debug; // for printing debug messages

extern char startdir[200];
const double MB = 1024 * 1024;
const Long64_t iMB = 1024 * 1024;

extern char mainname[200];
extern char *Logname;
extern std::mutex mtx_log;

// MemInfo_t info;

extern thread_local TRandom rnd;

// Int_t ev_max;

// bool bstart=true;
// bool btest=false;

// const Long64_t P64_0=-123456789123456789;

extern std::list<EventClass>::iterator m_event; // важный параметр
// m_event - указатель на первое непроанализированное событие в Levents
// std::list<EventClass>::iterator m_end; //важный параметр
// расстояние m_end до конца списка не должно быть больше,
// чем opt.ev_min
// в Ana данные анализируются от m_event до m_end
// или до Levents.end(), если конец анализа
// данные удаляются от Levents.begin() до m_event
// (но оставляется opt.ev_max событий)

// int MT=1;

const int MAXSHORT = 32767;

// const Long64_t GLBSIZE=2147483648;//1024*1024*1024*2; //1024 MB
// const Long64_t GLBSIZE=1024*1024*1024; //1024 MB

// obsolete - проверить, где используется и удалить
extern int tr_size; //=opt.usb_size*1024; - размер трансфера в байтах

//---------- Input buffers--------------
extern BufClass InpBuf_ring2; // = OFF_SIZE + InpBuf_ring
extern BufClass
    InpBuf_ring; // буфер, куда записываются все вход. данные (usb или из файла)
// Реально создается InpBuf_ring2, начало которого на OFF_SIZE
// сдвинуто влево от InpBuf_ring

//---------- Input/output lists of buffers--------------
extern std::list<BufClass> inp_list; // входные данные (usb или из файла)
// std::deque<BufClass> raw_deq; // запись raw
// std::deque<BufClass> dec_deq; // запись dec
extern std::list<BufClass> dec_list;
// выходные данные (dec)
extern std::list<BufClass> raw_list;
// выходные данные (raw)

extern Long64_t gl_sz;
extern const Long64_t gl_off; // 1MB, was 1024*128 - офсет GLBuf относительно
                             // GLBuf2 - нужен для копирования хвоста буфера в
                             // начало (см. AnaBuf) !! возможно, не нужен

extern UChar_t *GLBuf;
extern UChar_t *GLBuf2;

extern Long64_t
    b_start[CRS::MAXTHREADS]; // start of local buffer(part of GLBuf), included
extern Long64_t b_fill[CRS::MAXTHREADS]; // start of local buffer for reading/filling
extern Long64_t b_end[CRS::MAXTHREADS]; // end of local buffer(part of GLBuf), excluded

extern UInt_t gl_iread; // current number of readbuf or cback [0.. infinity)
extern UInt_t gl_ivect; // current number of pstruct in make_event [0.. infinity)
extern UInt_t gl_ibuf;  // current buffer for decode* [0 .. gl_Nbuf]
extern UInt_t gl_Nbuf;  // maximal number of buffers for decode*
// int gl_ntrd=6; //number of decode threads (and also sub-buffers)

extern int decode_thread_run;
extern int mkev_thread_run;
extern int ana_thread_run;
extern int dec_nr[CRS::MAXTHREADS];

extern TThread *trd_dec[CRS::MAXTHREADS];
// TCondition dec_cond[CRS::MAXTRANS];
extern TThread *trd_mkev;
// TCondition mkev_cond[CRS::MAXTRANS];
// int mkev_check[CRS::MAXTRANS];
extern TThread *trd_ana;

extern UInt_t dec_iread[CRS::MAXTHREADS];
// dec_iread[i]=0 в начале
// handle_decode крутится для n потоков
// пока dec_iread==0 для данного потока, handle_decode ждет
// dec_iread[i]=0 если i-й буфер обработан. В буфер можно читать.
// Если dec_iread[i]=1, i-й буфер еще не обработан

extern int ana_all;

// TCondition evt_cond;
// int evt_check;

// TThread* trd_ev;
// TCondition ev_cond;
// int ev_check;

// int make_event_thread_run;
// int ana2_thread_run;

// UInt_t list_min=100;

// volatile char astat[CRS::MAXTRANS];

extern Double_t tproc;

#ifdef TIMES
TTimeStamp tt1[10];
TTimeStamp tt2[10];
double ttm[10];
double dif;
#endif

volatile sig_atomic_t g_usb_list_changed;

extern EventClass levt;

/***********************************************************************/

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

// cyusb_handle *cy_handle;

void error(const char *msg) {
  perror(msg);
  // exit(1);
}

void *handle_events_func(void *ctx) {
  cout << "CRS thread created... " << endl;
  while (cusb->event_thread_run) {
    libusb_handle_events_completed(NULL, NULL);
  }
  cout << "CRS thread finished... " << endl;
  return NULL;
}

static void usb_signal_handler(int) {
  g_usb_list_changed = 1;
  cout << "usb signal" << endl;
}

#ifdef ANA3

static void cback3(libusb_transfer *trans) {
  // сохраняем текущий буфер
  // unsigned char* cbuf = trans->buffer;

  double rr = trans->actual_length * 100;
  rr /= TR_SIZE;
  // prnt("ss 4d
  // l 4.0fss;",BGRN,"cbk:",inp_list.size(),trans->actual_length,rr,"%",RST);

  if (trans->actual_length) {

    // проверяем slow decoding ????
    while (dec_iread[gl_ibuf]) {
      ++crs->errors[ER_DEC]; // slow decoding
      gSystem->Sleep(1);
    }

    decoder->Add_to_copy_queue(trans->buffer, trans->actual_length);

    // // запускаем анализ и запись буфера
    // if (!opt.directraw) {
    //   // заполняем inp_list
    //   auto buf_it = inp_list.insert(inp_list.end(),BufClass());
    //   buf_it->write_start = trans->buffer;
    //   buf_it->write_end = trans->buffer+trans->actual_length;

    //   // КОПИРОВАНИЕ В ОТДЕЛЬНЫЙ БУФЕР
    //   if (analysis_ring_buffer.write_available(trans->actual_length)) {
    //     analysis_ring_buffer.write(trans->buffer, trans->actual_length);
    //   } else {
    //     ++crs->errors[ER_BUF_OVERFLOW]; // статистика переполнения
    //   }

    //   // разделение на однопоточный/многопоточный должно быть в AnaBuf3
    //   crs->AnaBuf3(buf_it);

    //   /*
    //   if (opt.nthreads==1) { // однопоточный (линейный) анализ
    // 	crs->AnaBuf3(buf_it);
    //   }
    //   else { // многопоточный анализ
    // 	// заполняем deq
    // 	// анализ
    // 	//crs->AnaBuf3_MT();
    //   }
    //   */
    // }

    // запись
    if (opt.raw_write && !opt.fProc) {
      // см. cback
      /*
      if (opt.nthreads==1) { // однопоточная (линейная) запись
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
      else // многопоточная запись
        crs->Flush_Raw_MT(trans->buffer, trans->actual_length);
      */
    }
  }

  // запускаем новый транс
  if (crs->b_acq) {
    // запущено 7 (ntrans=7) трансферов. Последний запущенный i_prev
    // адрес нового буфера для нового трансфера:
    // crs->transfer[i_prev]->buffer + TR_SIZE;
    // размер ВСЕГДА должен быть TR_SIZE

    int itr = *(int *)trans->user_data; // номер трансфера
    int i_prev = (itr + crs->ntrans - 1) % crs->ntrans; // previous itr
    UChar_t *next_buf = cusb->transfer[i_prev]->buffer + TR_SIZE;
    if (next_buf + TR_SIZE > InpBuf_ring.write_end) {
      next_buf = InpBuf_ring.write_start;
    }

    // prnt("ss d x x
    // xs;",BBLU,"trf:",itr,gl_sz,trans->buffer-GLBuf,trans->actual_length,RST);

    trans->buffer = next_buf;
    libusb_submit_transfer(trans);

    stat_mut.Lock();
    crs->nbuffers++;
    crs->inputbytes += trans->actual_length;
    stat_mut.UnLock();
  }

} // cback3

#else  // !ANA3

/*
void print255(UChar_t *trbuf, size_t length, int reset = 0) {
  // Выводим 64-битные слова
  typedef long long int int64;
  const int64 period = 200000; // 1 kHz для ЦРС-32
  // const int64 period = 8192; // коричневый кабель
  // const int64 period = 16777216; // синий кабель

  static int64 per[4]={};
  static int64 anum=0, num[4]={};
  double fr[4]={};

  if (reset) {
    anum = 0;
    for (int i = 0; i < 4; i++) {
      per[i] = period * (i + 1) + period / 2; // 1.5 2.5 3.5 4.5
      num[i] = 0;
    }
    return;
  }

  int64 *buffer = (int64 *) trbuf;
  static int64 tprev = 0;
  size_t word_count = length / 8;
  for (size_t i = 0; i < word_count; i++) {
    u_char *buf = (u_char *)&buffer[i];
    int ch = buf[7];
    int fmt = (buf[6] & 0xF0) >> 4;
    if (ch == 255 && fmt == 0) {
      int64 tstmp = buffer[i] & 0xFFFFFFFFFFFF;
      int64 dt = tstmp - tprev;
      anum++;
      if (dt < per[0])
        num[0]++;
      else if (dt < per[1])
        num[1]++;
      else if (dt < per[2])
        num[2]++;
      else
        num[3]++;

      // cout << anum << " " << ch << " " << dt << " " << tstmp << " " << hex
      //      << buffer[i] << dec;

      printf("%lld %lld",dt,tstmp);
      if (anum > 0) {
        for (int i = 0; i < 4; i++) {
          fr[i] = double(num[i]) / anum;
          printf(" %lld %0.5f", num[i],fr[i]);
        }
        printf("\n");
      }

      tprev = tstmp;
    }
  }
}
*/


/*
void print_time_shift_old(UChar_t *trbuf, size_t length, int reset = 0) {
  // Выводим 64-битные слова
  typedef long long int int64;
  const int64 limit = 1000; // порог определения начала события

  static int64 nn = 0;
  static int64 tprev = 0;
  static int64 T[256]={};
  static int64 TP[256]={};
  int64 dt[256]={};

  if (reset) {
    nn=0;

    // printf("%12lld %12lld %12lld %12lld\n", T[0],T[1],T[2],T[16]);
    return;
  }

  int64 *buffer = (int64 *) trbuf;
  size_t word_count = length / 8;
  for (size_t i = 0; i < word_count; i++) {
    u_char *buf = (u_char *)&buffer[i];
    int ch = buf[7];
    int fmt = (buf[6] & 0xF0) >> 4;
    if (fmt == 0) {
      int64 tstmp = buffer[i] & 0xFFFFFFFFFFFF;
      int64 diff = tstmp - tprev;
      if (diff>limit) {
        dt[0] = T[0] - T[16];
        dt[1] = T[1] - T[16];
        dt[2] = T[2] - T[16];
        printf("%8lld %14lld", nn, T[16]);
        for (int i=0;i<3;i++) {
          printf(" %3lld", dt[i]);
        }
        for (int i=0;i<3;i++) {
          printf(" %6lld", T[i]-TP[i]);
        }
        printf(" %6lld\n", T[16] - TP[16]);
        nn++;
      }

      // cout << ch << " " << diff << " " << tstmp << " " << tprev << " " << hex
      //       << buffer[i] << dec << endl;

      //printf("%lld %lld",dt,tstmp);

      TP[ch] = T[ch];
      T[ch] = tstmp;

      tprev = tstmp;
    }
  }
} // print_time_shift_old
*/

/*
void print_time_shift(unsigned char *buf, size_t length) {
  // ловит ошибку сдвижки времени
  typedef long long int int64;
  const int64 limit = 20001; // порог определения сдвижки для частоты 10 кГц

  static int64 nn = 0;
  static int64 T[256]={};
  static int64 TP[256]={};

  for (size_t i = 0; i < length; i+=8) {
    int64* buf64 = (int64*)&buf[i];
    int ch = buf[i+7];
    int fmt = (buf[i+6] & 0xF0) >> 4;
    if (fmt == 0) {
      int64 tstmp = *buf64 & 0xFFFFFFFFFFFF;

      TP[ch] = T[ch];
      T[ch] = tstmp;
      int64 dt = T[ch] - TP[ch];
      if (dt>limit) {
        cout << "shift: " << nn << " " << ch << " " << T[ch] << " " << TP[ch] << " " << dt << endl;
      }
      nn++;
    }
  }
} // print_time_shift
*/

static void cback(libusb_transfer *trans) {
  // сохраняем текущий буфер
  // unsigned char* cbuf = trans->buffer;

  if (trans->actual_length) {

    // проверяем slow decoding
    while (dec_iread[gl_ibuf]) {
      ++crs->errors[ER_DEC]; // slow decoding
      gSystem->Sleep(1);
    }

    // запускаем анализ буфера (переделать!)
    double rr = double(trans->buffer - GLBuf + trans->actual_length) / gl_sz;
    UInt_t nn = (rr + 1e-6) * gl_Nbuf;
    // if (opt.decode) {
    if (!opt.directraw) {
      if (nn != gl_ibuf) {
        // cout << "cback2: " << itr << " " << nn << " " << gl_ibuf << endl;
        int length =
            trans->buffer - GLBuf - b_fill[gl_ibuf] + trans->actual_length;
        b_end[gl_ibuf] = b_fill[gl_ibuf] + length;
        //prnt("ss2d2d3d ds;", KBLU, "cback:", gl_iread, gl_ibuf, CheckMem(), length, RST);
#ifdef TIMING
          {
            SimpleTimer timer(0,length); //acquire
          }
#endif
        crs->AnaBuf(gl_ibuf);
      }
    } // if !opt.directraw

    // запускаем запись буфера, если opt.raw_write (переделать!)
    if (opt.raw_write && !opt.fProc) {
      // для cback3, возможно, переделать, используя
      // буфер декодера. Типа *(eraw->buf_it) = *("decoder"->buf_it);

      eraw->buf_it->write_start = trans->buffer; // начало записи буфера
      eraw->buf_it->u82.b =
          trans->buffer + trans->actual_length; // конец записи буфера
      eraw->buf_it->write_end = eraw->buf_it->u82.b; // физический конец буфера

      // пока так. проверить все, что делает Flush3. возможно, для raw не
      // годится
      eraw->Flush3(0);
    }

    // print255(trans->buffer,trans->actual_length);
    // print_time_shift(trans->buffer,trans->actual_length);

    // trans->buffer=next_buf;
    //  if (crs->b_acq) {
    //    libusb_submit_transfer(trans);
    //  }

  } // if (trans->actual_length)

  // запускаем новый транс
  if (crs->b_acq) {
    int itr = *(int *)trans->user_data; // номер трансфера
    int i_prev = (itr + crs->ntrans - 1) % crs->ntrans; // previous itr
    UChar_t *next_buf = cusb->transfer[i_prev]->buffer + tr_size;
    if (next_buf + tr_size > GLBuf + gl_sz) {
      next_buf = GLBuf;
    }

    // prnt("ss d x x
    // xs;",BGRN,"trf:",itr,gl_sz,trans->buffer-GLBuf,trans->actual_length,RST);
    trans->buffer = next_buf;
    libusb_submit_transfer(trans);

    stat_mut.Lock();
    crs->nbuffers++;
    crs->inputbytes += trans->actual_length;
    stat_mut.UnLock();
  }

} // cback
#endif // !ANA3

//#endif // CYUSB

CUSB::CUSB() {

  /*
  std::vector<int> vec = {10, 20, 30, 40, 50, 60};

  auto r_begin = vec.rbegin(); // Указывает на 40 (последний элемент)

  // std::prev(rbegin()) -> Указывает на 30 (предпоследний)
  auto it_prev_rbegin = std::next(r_begin);
  std::cout << *std::next(r_begin) << std::endl; // Вывод: 30
  std::cout << *std::prev(r_begin) << std::endl; // Вывод: 30
  // std::cout << *it_prev_rbegin << std::endl; // Вывод: 30

  // std::next(rbegin()) -> Указывает на "конец" в обратном порядке (после 40)
  auto it_next_rbegin = std::next(r_begin);
  // Этот итератор будет равен vec.rend()
  std::cout << (it_next_rbegin == vec.rend() ? "Это rend()" : "Не rend()")
            << std::endl; // Вывод: Это rend()

  exit(0);

  GBuf2.write_start = new UChar_t[16];
  GBuf2.write_end = GBuf2.write_start + 16;

  GBuf2.write_start[0]=11;
  GBuf2.write_start[1]=12;

  union82 uu; // текущее положение в буфере
  uu.b = GBuf2.write_end;
  while (uu.b > GBuf2.write_start) {
    --uu.ui;
    cout << (void*)GBuf2.write_start << " " << uu.ui << " " << (int) *uu.ui << endl;
  }
  exit(1);


  char cmd[200];
  sprintf(cmd,"telegram-send \"romana alert: countrate in channel %d is low:
  %0.1f 1/s\"",4,24.324456);
  //string cmd = "telegram-send \"romana alert: countrate is too low\"";
  gSystem->Exec(cmd);
  exit(1);

  PkClass pk;
  pk.QX = -333;
  cout << hex << pk.QX << endl;
  pk.QX = (pk.QX<<28)>>28;
  cout << hex << pk.QX << endl;
  exit(1);


  exit(1);
  */

  ndev = 0;

  idev = 0;
  devname = "";
  // Fmode = 0;     // >>> Fmode — поле CRS
  // opt.Period = 5; // >>> opt — глобальный, не поле

  cy_handle = NULL;

  event_thread_run = 1;

  chan_in_module = MAX_CH;

  // ntrans = MAXTRANS;  // >>> ntrans — поле CRS

  for (int i = 0; i < MAXTRANS; i++) {
    transfer[i] = NULL;
    // buftr[i] = NULL;  // >>> buftr — поле CRS
    // Fbuf[i]=NULL;
  }

  trd_crs = 0;

  struct sigaction sa;
  sa.sa_handler = usb_signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &sa, nullptr);

  g_usb_list_changed = 0;

} // CUSB::CUSB()

CUSB::~CUSB() {
  // >>> В оригинале деструктор CRS::~CRS() вызывал DoExit().
  // >>> Теперь CRS::~CRS() вызывает cusb->DoExit() (см. libcrs.cpp).
  // >>> Здесь деструктор CUSB пуст, чтобы не дублировать DoExit().
}

void CUSB::DoExit() {
  // cout << "CRS::DoExit" << endl;

  event_thread_run = 0;
  if (crs->Fmode == 1) {
    cyusb_close();
    cy_handle = 0;
    if (trd_crs) {
      trd_crs->Delete();
      trd_crs = 0;
    }
  }
}

void CUSB::DoDetectDev() {
  //#ifdef CYUSB
  if (!crs->b_stop)
    return;
  // if (Fmode==1 && module>=32) {
  if (crs->Fmode == 1) { // если модуль уже подключен, удаляем trd_crs
    event_thread_run = 0;
    if (crs->Fmode == 1) {
      // cyusb_close();
      if (trd_crs) {
        trd_crs->Delete();
        trd_crs = 0;
      }
      crs->Fmode = 0;
    }
  }

#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 100", RST);
#endif
  gSystem->Sleep(100);
#ifdef P_LIBUSB
  prnt("sss;", BGRN, "Reset USB... ", RST);
#endif

  if (cy_handle) { // close USB
#ifdef P_LIBUSB
    prnt("sss;", BGRN, "cyusb_close", RST);
#endif
    if (!cy_list.empty()) {
      cyusb_close();
      cy_handle = 0;
      cy_list.clear();
    }
  }

  devname = "";
  Open_USB();

  switch (ndev) {
  case 1:
    cusb->Init_device(); // >>> Init_device — метод CUSB; вызываем через cusb
    // cusb->Init_device();
  case 0:
    if (myM)
      myM->Build();
    break;
  default:
    new PopFrame(myM, 1, 1, M_DEVICE);
  }
  //#endif // CYUSB
}

void CUSB::DoResetUSB() {
  //#ifdef CYUSB
  if (!crs->b_stop)
    return;
  // if (Fmode==1 && module>=32) {
  if (crs->Fmode == 1) { // если модуль уже подключен, удаляем trd_crs
    event_thread_run = 0;
    // if (Fmode==1) {
    // cyusb_close();
    if (trd_crs) {
      trd_crs->Delete();
      trd_crs = 0;
    }
    crs->Fmode = 0;
    //}
  }

#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 100", RST);
#endif
  gSystem->Sleep(100);
#ifdef P_LIBUSB
  prnt("sss;", BGRN, "Reset USB... ", RST);
#endif

  if (cy_handle && crs->module >= 32) { // reset USB
#ifdef P_LIBUSB
    prnt("sss;", BGRN, "Command 7", RST);
#endif
    Command32(7, 0, 0, 0); // reset usb command

#ifdef P_LIBUSB
    prnt("sss;", BGRN, "cyusb_close", RST);
#endif
    if (!cy_list.empty()) {
      cyusb_close();
      cy_handle = 0;
      cy_list.clear();
    }

#ifdef P_LIBUSB
    prnt("sss;", BYEL, "Sleep 2000", RST);
#endif
    gSystem->Sleep(2000);
  }
#ifdef P_LIBUSB
  prnt("sss;", BGRN, "Done.", RST);
#endif

  Open_USB();
  // Set_USB(idev);
  if (myM) {
    Init_device();
    myM->EnableBut(myM->fGr1, crs->Fmode == 1);
  }
  //}
  // else {
  // prnt("sss;",BRED,"Module not found or reset not possible",RST);
  //}
  //#endif // CYUSB
} //DoResetUSB

int CUSB::Open_USB() {
  // открывает USB
  // создает и заполняет cy_list

  // cout << "Open_USB" << endl;
  if (devname.EqualTo("0")) {
    ndev = 0;
    return ndev;
  }

  if (!cy_list.empty())
    cyusb_close();

  cy_list.clear();

  ndev = cyusb_open();

  if (ndev < 0) {
    printf("Error opening library\n");
    return -1;
  } else if (ndev == 0) {
    printf("No device found\n");
    return -2;
  }

  // if ( ndev > 1 ) {
  //   printf("More than 1 devices of interest found. Disconnect unwanted
  //   devices\n"); return -3;
  // }

  int nn = ndev; // копия
  for (int i = 0; i < nn; i++) {
    // if (idev<0 || i==idev)
    Set_USB(i);
    cy_list.push_back(cpar.GetDevice(0, 0));

    TString sdev = cpar.GetDevice(0, 0);
    if (devname.Length() && sdev.Contains(devname, TString::kIgnoreCase)) {
      idev = i;
      ndev = 1;
    }
  }

  return ndev;
}

void CUSB::Set_USB(int i) {
  // задает cy_handle в сответствии с i
  // добавляет девайс с данным cy_handle в cy_list

  int r;

  cy_handle = cyusb_gethandle(i);
  // cout << "set: " << i << " " << cy_handle << endl;
  if (cyusb_getvendor(cy_handle) != 0x04b4) {
    printf("Cypress chipset not detected\n");
    cyusb_close();
    cy_handle = 0;
    exit(-1);
    // return -4;
  }

  // YK 29.09.20
  r = cyusb_reset_device(cy_handle);
  // prnt("ssds;",KRED,"cyusb_reset: ",r,RST);
  // gSystem->Sleep(100);

  if (r != 0) {
    printf("Can't reset device. Exitting: %d\n", r);
    cyusb_close();
    cy_handle = 0;
    exit(-1);
    // return -5;
  }
  // YK 29.09.20

  r = cyusb_kernel_driver_active(cy_handle, 0);
  if (r != 0) {
    printf("kernel driver active. Exitting\n");
    cyusb_close();
    cy_handle = 0;
    exit(-1);
    // return -6;
  }

  r = cyusb_claim_interface(cy_handle, 0);
  if (r != 0) {
    printf("Error in claiming interface: %d %d\n", i, r);
    cyusb_close();
    cy_handle = 0;
    exit(-1);
    // return -7;
  } else
    prnt("ss ds;", BGRN, "Successfully claimed interface", i, RST);

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

  // idev=i;
} // Set_USB

void CUSB::Device_info() {
  // Command32(7,0,0,0); //reset usb command
  // YK
  // Command2(4,0,0,0);

  // int sz;
  memset(buf_in, 0, sizeof(buf_in));
  // sz =
  Command32(1, 0, 0, 0);
  // sz =
  Command32(1, 0, 0, 0); // не помню, зачем вызывать 2 раза (?)

  memcpy(cpar.device, buf_in + 1, 4);

  // prnt("ss s
  // ds;",BGRN,cpar.GetDevice(module,0).c_str(),"Module:",module,RST);

} // Device_info

int CUSB::Init_device() {

  Set_USB(idev);

  crs->Fmode = 0;
  crs->module = 0;
  // Short_t firmw=0;

  event_thread_run = 1;
  trd_crs = new TThread("trd_crs", handle_events_func, (void *)0);
  trd_crs->Run();

#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 100", RST);
#endif
  gSystem->Sleep(100);
  // Command32(7,0,0,0); //reset usb command
  // YK
  // Command2(4,0,0,0);

  int sz;
  memset(buf_in, 0, sizeof(buf_in));
  sz = Command32(1, 0, 0, 0);
  sz = Command32(1, 0, 0, 0); // не помню, зачем вызывать 2 раза (?)

  memcpy(cpar.device, buf_in + 1, 4);

  Short_t nplates = buf_in[3];
  opt.ver_po = buf_in[4];

  int nch2 = 0;

  switch (cpar.device[0]) {
  case 1: // crs-32
  case 2: // crs-6/16
  case 3: // crs-16 or crs-2
    opt.Period = 5;
    // crs2
    if (opt.ver_po == 0) { // -> crs2
      crs->module = 22;
      chan_in_module = 2;
      nch2 = 2;
      opt.Nchan = 2;
      for (int j = 0; j < chan_in_module; j++) {
        crs->crs_ch[j] = 1; // было 0
      }
      break;
    }

    // crs32/16/6
    crs->module = 32;
    chan_in_module = nplates * 4;
    if (opt.ver_po == 1) { // версия ПО=1
      for (int i = 0; i < nplates; i++) {
        // cout << "Channels(" << i << "):";
        for (int j = 0; j < 4; j++) {
          crs->crs_ch[i * 4 + j] = 2; // было 0
          // cout << " " << crs_ch[i*4+j];
          nch2++;
        }
        // cout << endl;
      }
    } else { // версия ПО=2 или выше
      // chan_in_module=4;
      sz = Command32(10, 0, 0, 0);
      sz--;
      for (int i = 0; i < nplates; i++) {
        // cout << "Channels(" << i << "):";
        for (int j = 0; j < 4; j++) {
          // crs_ch[i*4+j]=buf_in[sz];
          if (buf_in[sz] == 0)
            crs->crs_ch[i * 4 + j] = 2;
          else if (buf_in[sz] == 1)
            crs->crs_ch[i * 4 + j] = 3;
          else
            crs->crs_ch[i * 4 + j] = 0;

          // cout << " " << crs_ch[i*4+j];
          nch2++;
        }
        // cout << endl;
        sz--;
      }
      if (opt.ver_po == 3) { // версия ПО=3
        crs->module = 33;
      } else if (opt.ver_po == 4) { // версия ПО=4
        crs->module = 34;
      } else if (opt.ver_po >= 5 && opt.ver_po <= 6) { // версия ПО=5 или 6
        crs->module = 35;
      } else if (opt.ver_po >= 7) { // версия ПО=7,8
        crs->module = 36;
      }

      // для версии ПО=9 и выше задаем маску - оказалась не нужна!
      // if (opt.ver_po >= 9) {
      //   cpar.chmask = 0x1F;
      // }
    }

    break;

  case 4: // crs-8/16
    // module=41;
    // module=42;
    // module=43;
    crs->module = 44;
    opt.Period = 10;
    chan_in_module = nplates * 8;
    for (int j = 0; j < chan_in_module; j++) {
      crs->crs_ch[j] = 4; // было 2
      cout << " " << crs->crs_ch[j];
      nch2++;
    }
    cout << endl;
    break;

  case 5: // crs-128
    // module=51/52;
    // module=53;
    crs->module = 54;
    opt.Period = 10;
    chan_in_module = nplates * 16;
    for (int j = 0; j < chan_in_module; j++) {
      crs->crs_ch[j] = 5; // было 2
      cout << " " << crs->crs_ch[j];
      nch2++;
    }
    cout << endl;
    break;

  case 6: // AK-32
    crs->module = 45;
    opt.Period = 6.4;
    chan_in_module = nplates * 4;

    sz = Command32(10, 0, 0, 0);
    sz--;

    for (int i = 0; i < nplates; i++) {
      // cout << "Channels(" << i << "):";
      for (int j = 0; j < 4; j++) {
        crs->crs_ch[i * 4 + j] = 20 + buf_in[sz];
        if (buf_in[sz] == 5)
          crs->crs_ch[i * 4 + j] = 6;
        else if (buf_in[sz] == 4)
          crs->crs_ch[i * 4 + j] = 7;
        else
          crs->crs_ch[i * 4 + j] = 0;

        // cout << " " << crs_ch[i*4+j];
        nch2++;
      }
      // cout << endl;
      sz--;
    }

    // exit(-1);
    break;

  default:
    cout << "unknown device: " << endl;
    exit(1);
  }

  if (opt.Nchan > nch2) {
    opt.Nchan = nch2;
  }

  for (int i = 0; i < MAX_CH; i++) {
    cpar.Len[i] = cpar.ChkLen(i, crs->module);
  }

  // prnt("ssd ss;",BGRN,"Module: ",module, cpar.GetDevice(module).c_str(),
  // RST);
  prnt("ss s ds;", BGRN, cpar.GetDevice(crs->module).c_str(), "Module:", crs->module,
       RST);

  if (crs->module >= 22) {
    crs->Fmode = 1;
    strcpy(mainname, cpar.GetDevice(crs->module, 0).c_str());
  } else
    crs->Fmode = 0;

  // cout << "title: " << mainname << " " << module << endl;
  //  if (myM) {
  //    myM->EnableBut(myM->fGr1,Fmode==1);
  //    myM->DoReset();
  //    myM->SetTitle(mainname);
  //  }

  crs->InitBuf();

  #ifdef ANA3
  if (Init_Transfer3()) {
    return -8;
  };
#else  // !ANA3
  if (Init_Transfer()) {
    return -8;
  };
#endif // ANA3

  // Submit_all(MAXTRANS);
  // YK
  // Command2(4,0,0,0);

  return 0;

} // Init_device

int CUSB::SetPar() {
  switch (crs->module) {
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
  case 44: //CRS-8
  case 54: //CRS-128
    AllParameters44();
    break;
  case 45:
    AllParameters_AK32();
    break;
  default:
    cout << "SetPar Error! No module found" << endl;
    return 3;
  }

  return 0;

  // Thr[11]=100;
}

void CUSB::Free_Transfer() {

  Cancel_all(MAXTRANS);
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 50", RST);
#endif
  gSystem->Sleep(50);

  // cout << "---Free_Transfer---" << endl;

  // for (int i=0;i<ntrans;i++) {
  for (int i = 0; i < MAXTRANS; i++) {
    // cout << "free: " << i << " " << (int) transfer[i]->flags << endl;
    // int res = libusb_cancel_transfer(transfer[i]);
    libusb_free_transfer(transfer[i]);
#ifdef P_LIBUSB
    prnt("ssds;", BBLU, "libusb_free_transfer: ", i, RST);
#endif

    // transfer[i]=NULL;
  }

  for (int i = 0; i < MAXTRANS; i++) {
    if (crs->buftr[i]) {
      crs->buftr[i] = NULL;
    }
  }
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 50", RST);
#endif
  gSystem->Sleep(50);
}

void CUSB::Submit_all(int ntr) {
  crs->ntrans = 0;
  int res = 0;
  for (int i = 0; i < ntr; i++) {
    res = libusb_submit_transfer(transfer[i]);
    // cout << i << " Submit: " << res << endl;
    if (res) {
      // cout << "Submit_Transfer error: " << res << " " << *(int*)
      // transfer[i]->user_data << endl; cout << libusb_error_name(res) << endl;
      break;
    } else {
      // cout << "Submit_Transfer: " << res << " " << *(int*)
      // transfer[i]->user_data << endl;
      crs->ntrans++;
    }

#ifdef P_LIBUSB
    prnt("ssd d ds;", BBLU, "libusb_submit_transfer: ", i, crs->ntrans,
         *(int *)transfer[i]->user_data, RST);
#endif
  }

  if (res) {
    prnt("ssd ds;", BRED, "Submit_Transfer error: ", res, crs->ntrans, RST);
    // cout << "Submit_Transfer error: " << res << " " << *(int*)
    // transfer[i]->user_data << endl;
    cout << libusb_error_name(res) << endl;
  }
}

void CUSB::Cancel_all(int ntr) {
  // gSystem->Sleep(2300);
  for (int i = 0; i < ntr; i++) {
    if (transfer[i]) {
#ifdef P_LIBUSB
      int res = libusb_cancel_transfer(transfer[i]);
      prnt("ssd ds;", BBLU, "libusb_cancel_transfer: ", i, res, RST);
      if (res)
        cout << libusb_error_name(res) << ": " << i << endl;
#else
      libusb_cancel_transfer(transfer[i]);
#endif
      // cout << i << " Cancel: " << res << endl;
    }
  }
}

#ifdef ANA3
int CUSB::Init_Transfer3() {

  crs->ntrans = 0;
  for (int i = 0; i < MAXTRANS; i++) {
    transfer[i] = libusb_alloc_transfer(0);
    // transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;

    int *ntr = new int;
    (*ntr) = i;

    UChar_t *buf = InpBuf_ring.write_start + TR_SIZE * i;

    // unsigned int timeout=400*(i+1); //0 millisec
    unsigned int timeout = 1000 + 100 * i; // 0 millisec
    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buf, TR_SIZE,
                              cback3, ntr, timeout);

#ifdef P_LIBUSB
    prnt("ssds;", BBLU, "libusb_fill_bulk_transfer: ", i, RST);
#endif
  }

  // cout << "submit7:" << endl;
  Submit_all(MAXTRANS);

  if (crs->ntrans != MAXTRANS) {
    for (int i = crs->ntrans; i < MAXTRANS; i++) {
      // cout << "free: " << i << endl;
      // libusb_free_transfer(transfer[i]);
      cout << "delete: " << i << endl;
      if (crs->buftr[i]) {
        crs->buftr[i] = NULL;
      }
    }
    return 2;
  }
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 250", RST);
#endif
  gSystem->Sleep(250);
  Cancel_all(MAXTRANS);
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 250", RST);
#endif
  gSystem->Sleep(250);

  return 0;

} // Init_Transfer3
#else // !ANA3

int CUSB::Init_Transfer() {

  // cout << "---Init_Transfer---" << endl;

  // Cancel_all(MAXTRANS);

  ///* YK 29.09.20
  // cout << "---Init_Transfer2---" << endl;
  // int r= cyusb_reset_device(cy_handle);
  // prnt("ssds;",KRED,"cyusb_reset: ",r,RST);
  // gSystem->Sleep(100);
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

  // cout << "---Init_Transfer 3---" << endl;
  crs->ntrans = 0;
  for (int i = 0; i < MAXTRANS; i++) {
    transfer[i] = libusb_alloc_transfer(0);
    // transfer[i]->flags|=LIBUSB_TRANSFER_FREE_BUFFER;

    int *ntr = new int;
    (*ntr) = i;

    unsigned int timeout = 0; // 200

    // printf("Init_Transfer: i=%d buftr=%p tr_size=%d ntrans=%d cy_handle=%p\n",
    //    i, (void*)crs->buftr[i], tr_size, crs->ntrans, (void*)cy_handle);

    libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, crs->buftr[i], tr_size,
                              cback, ntr, timeout);
    // libusb_fill_bulk_transfer(transfer[i], cy_handle, 0x86, buftr[i],
    // opt.usb_size*1024, cback, ntr, 0);
#ifdef P_LIBUSB
    prnt("ssds;", BBLU, "libusb_fill_bulk_transfer: ", i, RST);
#endif

    /*
      int res;
      res = libusb_submit_transfer(transfer[i]);

      if (res) {
      cout << "Init_Transfer error: " << res << " " << *(int*)
      transfer[i]->user_data << endl; cout << libusb_error_name(res) << endl;
      break;
      }
      else {
      ntrans++;
      }
    */
  }

  // cout << "submit7:" << endl;
  Submit_all(MAXTRANS);

  /*
    for (int i=0;i<ntrans;i++) {
    int res;
    res = libusb_cancel_transfer(transfer[i]);
    cout << i << " Cancel: " << res << endl;
    }
  */
  // cout << "Number of transfers: " << ntrans << endl;

  if (crs->ntrans != MAXTRANS) {
    for (int i = crs->ntrans; i < MAXTRANS; i++) {
      // cout << "free: " << i << endl;
      // libusb_free_transfer(transfer[i]);
      cout << "delete: " << i << endl;
      if (crs->buftr[i]) {
        crs->buftr[i] = NULL;
      }
    }
    return 2;
  }
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 250", RST);
#endif
  gSystem->Sleep(250);
  Cancel_all(MAXTRANS);
#ifdef P_LIBUSB
  prnt("sss;", BYEL, "Sleep 250", RST);
#endif
  gSystem->Sleep(250);

  return 0;

} // Init_Transfer

#endif // ANA3

int CUSB::Command32_old(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
  // для версии ПО 1
  int transferred = 0;
  int r;

  int len_in, len_out;

  // UChar_t buf_out[6];//={0};
  // UChar_t buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (UChar_t)(par >> 16);
  buf_out[4] = (UChar_t)(par >> 8);
  buf_out[5] = (UChar_t)par;

  switch (cmd) {
  case 1:
    len_out = 1;
    len_in = 5;
    break;
  case 2:
    len_out = 6;
    len_in = 1;
    break;
  case 3:
  case 4:
    len_out = 1;
    len_in = 1;
    break;
  case 5:
    len_out = 2;
    len_in = 7;
    break;
  case 6:
  case 7:
    len_out = 1;
    len_in = 2;
    break;
  default:
    cout << "Wrong CRS32 command: " << cmd << endl;
    return 0;
  }

  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    printf("Error6! %d: \n", buf_out[1]);
    cyusb_error(r);
    // cyusb_close();
  }

  if (cmd != 7) {
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      printf("Error7! %d: \n", buf_out[1]);
      cyusb_error(r);
      // cyusb_close();
    }
  }

  return len_in;
}

int CUSB::Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
  // if (cmd !=2 || cpar.on[ch]) {
#ifdef P_CMD
  prnt("ssd d d d xs", BGRN, "Cmd32: ", (int)cmd, (int)ch, (int)type, par, par,
       RST);
#endif
  // }

  // для версии ПО 2
  int transferred = 0;
  int r;

  int len_in, len_out;

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (UChar_t)(par >> 16);
  buf_out[4] = (UChar_t)(par >> 8);
  buf_out[5] = (UChar_t)par;

  switch (cmd) {
  case 1:
    len_out = 1;
    len_in = 5;
    break;
  case 2:
    len_out = 6;
    len_in = 1;
    break;
  case 3:
  case 4:
    len_out = 1;
    len_in = 1;
    break;
  case 5:
    len_out = 2;
    len_in = 7;
    break;
  case 6:
  case 7:
  case 8:
  case 9:
    len_out = 1;
    len_in = 2;
    break;
  case 10:
    len_out = 1;
    len_in = 9;
    break;
  case 11:
    len_out = 6;
    len_in = 1;
    
    break;
  default:
    cout << "Wrong CRS32 command: " << (int)cmd << endl;
    return 0;
  }

  std::ostringstream oss;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    oss << " Error_out";
    // printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    // cyusb_close();
  }

  if (cmd == 8) { // если сброс сч./буф. -> задержка
    gSystem->Sleep(20);
  }

  if (cmd != 7 && cmd != 12) { // сброс USB и Тест
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      oss << " Error_in";
      // sprintf(ss,"Error_in");
      // printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      // cyusb_close();
    }
#ifdef P_CMD
    else
      oss << " " << len_in << " " << (int)buf_in[0];
      // sprintf(ss,"%d %d",len_in,buf_in[0]);
#endif
  }
#ifdef P_CMD
  else {
    oss << " none";
  }
  prnt("ssss;", BGRN, " :", oss.str().data(), RST);
#endif

  return len_in;
}

int CUSB::Command2(UChar_t cmd, UChar_t ch, UChar_t type, int par) {
#ifdef P_CMD
  prnt("ssds;", BRED, "Cmd2: ", (int)cmd, RST);
#endif
  // cout << "Command2: " << (int) cmd << endl;
  int transferred = 0;
  int r;

  int len_in, len_out;

  // UChar_t buf_out[6];//={0};
  // UChar_t buf_in[7];//={0};

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[4] = (UChar_t)(par >> 8);
  buf_out[3] = (UChar_t)par;

  switch (cmd) {
  case 1:
  case 3:
  case 4:
    len_out = 1;
    break;
  case 2:
    len_out = 5;
    break;
  default:
    cout << "Wrong CRS2 command: " << cmd << endl;
    return 0;
  }

  len_in = 2;

  // printf("buf_out:");
  // for (int i=0;i<6;i++) {
  // 	printf(" %d",buf_out[i]);
  // }
  // printf("\n");

  // std::ostringstream oss;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    // oss << " Error_out";
    cyusb_error(r);
    // cyusb_close();
  }

#ifdef P_CMD
  prnt("ssds;", BRED, "out: ", r, RST);
#endif

  // cout << "Sleep(10000) start" << endl;
  // gSystem->Sleep(10000); //300
  // cout << "Sleep(10000) end" << endl;
  r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
  if (r) {
    cout << "Error_in: " << r << endl;
    // oss << " Error_in";
    cyusb_error(r);
    // cyusb_close();
  }
  // #ifdef P_CMD
  //   else
  //     oss << " " << len_in << " " << (int) buf_in[0];
  //   prnt("s s ss;",KRED,"<<",oss.str().data(),RST);
  // #endif
  //   cout << "Command2: " << (int) cmd << " " << r << " after in" << endl;

#ifdef P_CMD
  prnt("ssds;", BRED, "in: ", r, RST);
#endif

  return len_in;
}

void CUSB::Check33(UChar_t cmd, UChar_t ch, int &a1, int &a2, int min, int max) {
  int len = a2 - a1 + 1;
  if (len > max) {
    len = max;
    a2 = a1 + max - 1;
  }
  if (len < min) {
    len = min;
    a2 = a1 + min - 1;
  }
  // Command32(2,ch,cmd,a1); //offset
  Command32(2, ch, cmd, len); // length
  // cout << "Check33: " << (int) ch << " " << (int) cmd << " " << a1
  // << " " << a2 << " " << len << endl;
}

void CUSB::AllParameters_AK32() { // AK-32
  AllParameters44();
  //AllParameters44_CFD(); // CFD, Rtime

  // Индивидуальные параметры каналов:
  for (auto chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {
      Command32(2, chan, 37, 0); // входной импеданс - всегда 50 Ом
    }
  }
  // Общие параметры:
  // Command32(11,10,0,0); // режим работы прибора Multi/Single (игнорируется)
  Command32(11, 11, 0, 8); // число обслуживаемых рабочих плат MAIN-ом USB3
}

/*
void CRS::AllParameters_CRS8() { // ЦРС-8
  AllParameters44();
  //AllParameters44_CFD(); // CFD, Rtime
}
*/

/*
void CRS::AllParameters_CFD() { // parameters from TZ CFD+ (2025)
  int F1 = 31,
      // F2=1,
      FRR = 5; // делитель=32

  int MR = 0; // способ вычисления QX/RX центроидаX
  int T3 = 1; // промежуточная длит. центроида
  for (auto chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {
      MR = opt.Mr[chan] + 1;
      if (MR > 1) {
        T3 = opt.Mr[chan];
        MR = 2;
      }

      int FD = abs(opt.DD[chan]);
      if (FD == 0)
        FD = 1;
      Command32(2, chan, 38, FD); // FD – задержка CFD
      Command32(2, chan, 39, F1); // F1 – множитель 1 CFD
      int ff = abs(opt.FF[chan]);
      if (ff == 0)
        ff = 1;
      Command32(2, chan, 40, ff);  // F2 – множитель 2 CFD
      Command32(2, chan, 41, FRR); // FRR – номер делителя CFD
      Command32(2, chan, 42, MR); // способ вычисления QX/RX центроидаX
      // 43: промежут. безусловная длительность центроидаX
      Check33(43, chan, opt.T1[chan], T3, 1, 4095);
    }
  }
}
*/

void CUSB::AllParameters44() {
  // cout << "Allparameters44: " << endl;
  UInt_t // mask_discr, // маска для дискр,СС и пересчета - задаются в
         // Allparameters36
      mask_start, // маска для СТАРТ
      wmask;      // маска разрешений записи

  UInt_t mask_group;

  AllParameters36();

  int hard_logic = 0;
  if (cpar.Trigger == 2) { // Coinc
    hard_logic = 1;
  }

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {

    if (cpar.on[chan]) {

      // mask_discr = 0b0000000000011; // bits 0,1 (было еще 11,12)
      // if (opt.dsp[chan]) {
      //   mask_discr |= 0b11101110000; // write DSP data
      //   if (cpar.Trg[chan] == 7) {
      //     mask_discr |= 1 << 13; // write CFD
      //   }
      // }

      // if (cpar.pls[chan]) {   // add 1100
      //   mask_discr |= 0b1100; // write pulse
      // }
      
      mask_discr[chan] &= 0xE7FF; //убираем биты 11,12 для ЦРС-8 и старше
      mask_start = 0b1110000000001; // bits 0,10,11,12 - bitmask for START:
                                    // tst,count48,overflow

      int w = 1;
      mask_group = 0;
      for (int j = 0; j < 2; j++) {
        if (cpar.group[chan][j]) {
          if (cpar.coinc_w[j] > w)
            w = cpar.coinc_w[j];
          mask_group += j + 1;
        }
      }

      wmask = 2; // 0010 - запись по сигналу СТАРТ - всегда

      if (cpar.Trigger == 0) { // discr
        wmask |= 1;
      } else if (cpar.Trigger == 1) { // START
        mask_start |= mask_discr[chan]; // добавляем запись импульса по старту
      } else {      // coinc
        wmask |= 4; // запись по СС
        if (cpar.RD[chan]) {
          wmask |= 8; // запись по пересчету
          // wmask|=4; // запись по СС
        }
      }

      Command32(2, chan, 23,
                mask_discr[chan] & cpar.RMask); // переписываем bitmask для дискр
      Command32(2, chan, 24,
                          mask_start & cpar.RMask); // bitmask для START
      Command32(2, chan, 25, wmask); // битовая маска разрешений записи
      Command32(2, chan, 26,
                mask_discr[chan] & cpar.RMask); // bitmask для СС и пересчета

      Command32(2, chan, 27, (int)w); // длительность окна совпадений
      Command32(2, chan, 28, (int)0); // тип обработки повторных
      int red = cpar.RD[chan] - 1;
      if (red < 0)
        red = 0;
      Command32(2, chan, 29, (int)red); // величина пересчета P
      Command32(
          2, chan, 30,
          200); // максимальное расстояние для дискриминатора типа 3: L (=200)
      Command32(2, chan, 31,
                (int)mask_group); // битовая маска принадлежности к группам
      Command32(2, chan, 34,
                cpar.F24); // разрядность форматирования отсчетов сигнала
    } // if (cpar.on[chan])
    else {
      Command32(2, chan, 25, 0); // битовая маска разрешений записи = 0
    }
  } // for

  // Общие параметры:

  Command32(11, 1, 0, cpar.Smpl); // Sampling rate
  Command32(11, 4, 0, (int)hard_logic); // Использование схемы совпадений
  Command32(11, 5, 0, (int)cpar.mult_w1[0]); // минимальная множественность 0
  Command32(11, 6, 0, (int)cpar.mult_w2[0]); // максимальная множественность 0
  Command32(11, 7, 0, (int)cpar.mult_w1[1]); // минимальная множественность 1
  Command32(11, 8, 0, (int)cpar.mult_w2[1]); // максимальная множественность 1
  Command32(11, 9, 0, (int)0); // регистрация перенапряжения

} // AllParameters44()

void CUSB::AllParameters36() {
  // cout << "AllParameters36(): " << endl;

  // enable start channel by default
  Command32(2, 255, 11, 1); // enabled

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan >= opt.Nchan)
      cpar.on[chan] = false;

    Command32(2, chan, 11, (int)cpar.on[chan]); // enabled

    if (cpar.on[chan]) {
      Command32(2, chan, 0, (int)cpar.AC[chan]);
      Command32(2, chan, 1, (int)cpar.Inv[chan]);
      // hS - see below
      Command32(2, chan, 3, (int)cpar.Dt[chan]);
      Command32(2, chan, 4, (int)-cpar.Pre[chan]);
      Command32(2, chan, 5, (int)cpar.Len[chan]);
      Command32(2, chan, 6, (int)cpar.Drv[chan]);
      Command32(2, chan, 7, (int)cpar.Thr[chan]);
      Command32(2, chan, 8, (int)cpar.G[chan]);
      // new commands
      Command32(2, chan, 9, (int)cpar.hD[chan]); // delay
      // Command32(2,chan,9,0); //delay
      Command32(2, chan, 10, 0); // test signal is off

      Int_t trg2 = cpar.Trg[chan];
      Int_t cntr = 0; // тип вычисления центроида (0,1)
      if (trg2 == 5) {
        trg2 = 2; // для триг5 устанавливаем триг2
        cntr = 1; // и тип центроида 1
      } else if (trg2 == 6)
        trg2 = 4; // для триг6 устанавливаем триг4

      Command32(2, chan, 12, (int)trg2); // trigger type

      // начало вычисляемого
      Command32(2, chan, 13, opt.B1[chan]);
      Command32(2, chan, 15, opt.P1[chan]);
      Command32(2, chan, 17, opt.P1[chan]);
      Command32(2, chan, 19, opt.T1[chan]);
      Command32(2, chan, 21, opt.W1[chan]);

      // длина вычисляемого
      Check33(14, chan, opt.B1[chan], opt.B2[chan], 1, 4095);
      Check33(16, chan, opt.P1[chan], opt.P2[chan], 1, 4095);
      Check33(18, chan, opt.P1[chan], opt.P2[chan], 1, 4095);
      Check33(20, chan, opt.T1[chan], opt.T2[chan], 1, 4095);
      Check33(22, chan, opt.W1[chan], opt.W2[chan], 1, 4095);

      //маска форматов по дискриминатору: для ЦРС-32 маска общая для дискр и START
      mask_discr[chan] = 0b1100000000011; // bits 0,1,11,12
      if (opt.dsp[chan]) {
        mask_discr[chan] |= 0b11101110000; // write DSP data
        if (cpar.Trg[chan] == 7) {
          mask_discr[chan] |= 1 << 13; // write CFD
        }
      }

      if (cpar.pls[chan]) { // add 1100
        mask_discr[chan] |= 0b1100;     // write pulse
      }

      Command32(2, chan, 23, mask_discr[chan] & cpar.RMask); // bitmask for discriminator
      Command32(2, chan, 32, cntr);              // type of centroid

      int pwin = opt.P2[chan]; // окно повторных срабатываний
      if (pwin < 0)
        pwin = 0;
      if (pwin > 4095)
        pwin = 4095;
      Command32(2, chan, 33, pwin); // окно повторных срабатываний

      Command32(2, chan, 35, (int)cpar.LT[chan]); // нижний порог дискр.(3,4) =
                                                  // 0

      // сглаживание
      Int_t sum = cpar.hS[chan];
      if (sum <= 0)
        sum = 1;

      Int_t kk = cpar.hS[chan] - 1;
      UInt_t div = 0;
      if (kk > 0) {
        while (kk >>= 1) {
          div++;
        }
        div++;
      }

      // prnt("ss d d ds;",BGRN,"sum2:",chan,sum,div,RST);
      Command32(2, chan, 36, sum); // сглаживание: сумма
      Command32(2, chan, 2, div);  // сглаживание: деление

      // CFD+
      int F1 = 31,
          // F2=1,
          FRR = 5; // делитель=32

      int T3 = 1; // промежуточная длит. центроида

      int MR = opt.Mr[chan] + 1; // способ вычисления QX/RX центроидаX
      if (MR > 1) {
        T3 = opt.Mr[chan];
        MR = 2;
      }

      int FD = abs(opt.DD[chan]);
      if (FD == 0)
        FD = 1;
      Command32(2, chan, 38, FD); // FD – задержка CFD
      Command32(2, chan, 39, F1); // F1 – множитель 1 CFD
      int ff = abs(opt.FF[chan]);
      if (ff == 0)
        ff = 1;
      Command32(2, chan, 40, ff);  // F2 – множитель 2 CFD
      Command32(2, chan, 41, FRR); // FRR – номер делителя CFD
      Command32(2, chan, 42, MR); // способ вычисления QX/RX центроидаX
      // 43: промежут. безусловная длит. центроидаX
      Check33(43, chan, opt.T1[chan], T3, 1, 4095);

    } // if cpar.on[chan]
  } // for

  // Start dead time DT
  Int_t DT = cpar.DTW[0];
  if (DT <= 0)
    DT = 1;

  // Start source
  int st_src = cpar.St_Per ? 1 : 0;

  // Start imitator period
  int sprd = cpar.St_Per;
  if (sprd)
    sprd--;

  UChar_t type = DT >> 24;
  Command32(11, 0, type, (UInt_t)DT); // Start dead time DTW
  type = cpar.DTW[1] >> 24;
  Command32(11, 12, type, (UInt_t)cpar.DTW[1]); // Start dead time DTW

  Command32(11, 2, 0, st_src);              // Start source
  Command32(11, 3, 0, sprd);                // Start imitator period
}

void CUSB::AllParameters35() {
  UInt_t mask;

  Int_t s_Trg[MAX_CHTP];
  Int_t drv[MAX_CHTP];

  for (int i = 0; i < chan_in_module; i++) {
    s_Trg[i] = cpar.Trg[i];
    if (cpar.Trg[i] == 5) {
      cpar.Trg[i] = 2;
      drv[i] = 1;
    } else {
      drv[i] = 0;
    }
  }

  AllParameters33();

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {

      mask = 0b1100000000011; // bits 0,1,11,12
      if (opt.dsp[chan]) {
        mask |= 0b11101110000; // write DSP data
      }

      if (cpar.pls[chan]) { // add 1100
        mask |= 0b1100;     // write pulse
      }

      Command32(2, chan, 23, mask & cpar.RMask); // bitmask for discriminator
      Command32(2, chan, 32, drv[chan]);         // type of centroid

      int pwin = opt.P2[chan]; // окно повторных срабатываний
      if (pwin < 0)
        pwin = 0;
      if (pwin > 4095)
        pwin = 4095;
      Command32(2, chan, 33, pwin); // окно повторных срабатываний

    } // if
  } // for

  // Start dead time DT
  Int_t DT = cpar.DTW[0];
  if (DT <= 0)
    DT = 1;
  UChar_t type = DT >> 24;

  // Start source
  int st_src = cpar.St_Per ? 1 : 0;

  // Start imitator period
  int sprd = cpar.St_Per;
  if (sprd)
    sprd--;

  Command32(11, 0, type, (UInt_t)DT); // Start dead time DTW
  Command32(11, 2, 0, st_src);              // Start source
  Command32(11, 3, 0, sprd);                // Start imitator period

  for (int i = 0; i < chan_in_module; i++) {
    cpar.Trg[i] = s_Trg[i];
  }
}

void CUSB::AllParameters34() {
  // cout << "AllParameters34(): " << endl;

  AllParameters33();
  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (cpar.on[chan]) {
      // cpar.Mask[chan]=0xFF;
      UInt_t mask = 0xC3;  // 11000011 - write tst,count40,count48,overflow
      if (opt.dsp[chan]) { // add 110000
        mask |= 0x30;      // write DSP data
      }
      if (cpar.pls[chan]) { // add 1100
        mask |= 0xC;        // write pulse
      }
      Command32(2, chan, 23, mask & cpar.RMask); // bitmask
      // UInt_t mask = 0xFF;
      // Command32(2,chan,23,(UInt_t) mask); //bitmask
    }
  } // for

  // Start dead time DT
  Int_t DT = cpar.DTW[0];
  if (DT == 0)
    DT = 1;
  UChar_t type = DT >> 24;
  Command32(11, 0, type, (UInt_t)DT);
}

void CUSB::AllParameters33() {
  // cout << "AllParameters33(): " << endl;

  // enable start channel by default
  Command32(2, 255, 11, 1); // enabled

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan >= opt.Nchan)
      cpar.on[chan] = false;
    Command32(2, chan, 11, (int)cpar.on[chan]); // enabled

    if (cpar.on[chan]) {
      Command32(2, chan, 0, (int)cpar.AC[chan]);
      Command32(2, chan, 1, (int)cpar.Inv[chan]);
      Command32(2, chan, 2, (int)cpar.hS[chan]);
      Command32(2, chan, 3, (int)cpar.Dt[chan]);
      Command32(2, chan, 4, (int)-cpar.Pre[chan]);
      Command32(2, chan, 5, (int)cpar.Len[chan]);
      Command32(2, chan, 6, (int)cpar.Drv[chan]);
      Command32(2, chan, 7, (int)cpar.Thr[chan]);
      Command32(2, chan, 8, (int)cpar.G[chan]);
      // new commands
      Command32(2, chan, 9, (int)cpar.hD[chan]); // delay
      // Command32(2,chan,9,0); //delay
      Command32(2, chan, 10, 0); // test signal is off

      Int_t trg2 = cpar.Trg[chan];
      if (trg2 == 6)
        trg2 = 4; // для триг6 устанавливаем триг4
      Command32(2, chan, 12, (int)trg2); // trigger type

      // начало вычисляемого
      Command32(2, chan, 13, opt.B1[chan]);
      Command32(2, chan, 15, opt.P1[chan]);
      Command32(2, chan, 17, opt.P1[chan]);
      Command32(2, chan, 19, opt.T1[chan]);
      Command32(2, chan, 21, opt.W1[chan]);

      // длина вычисляемого
      Check33(14, chan, opt.B1[chan], opt.B2[chan], 1, 4095);
      Check33(16, chan, opt.P1[chan], opt.P2[chan], 1, 4095);
      Check33(18, chan, opt.P1[chan], opt.P2[chan], 1, 4095);
      Check33(20, chan, opt.T1[chan], opt.T2[chan], 1, 4095);
      Check33(22, chan, opt.W1[chan], opt.W2[chan], 1, 4095);
    }
  }
}

void CUSB::AllParameters32() {
  // cout << "AllParameters32(): " << endl;

  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    if (chan >= opt.Nchan)
      cpar.on[chan] = false;
    Command32(2, chan, 0, (int)cpar.AC[chan]);
    Command32(2, chan, 1, (int)cpar.Inv[chan]);
    Command32(2, chan, 2, (int)cpar.hS[chan]);
    Command32(2, chan, 3, (int)cpar.Dt[chan]);
    Command32(2, chan, 4, (int)cpar.Pre[chan]);
    Command32(2, chan, 5, (int)cpar.Len[chan]);
    Command32(2, chan, 6, (int)cpar.Drv[chan]);
    Command32(2, chan, 7, (int)cpar.Thr[chan]);
    Command32(2, chan, 8, (int)cpar.G[chan]);
    // new commands
    Command32(2, chan, 9, 0); // delay
    // Command32(2,chan,10,0); //test signal
    Command32(2, chan, 11, (int)cpar.on[chan]); // enabled
    // Command32(2,chan,11,1); //enabled
  }
}

void CUSB::AllParameters2() {
  // cout << "AllParameters2(): " << endl;
  for (UChar_t chan = 0; chan < chan_in_module; chan++) {
    Command2(2, chan, 0, (int)cpar.G[chan]);
    Command2(2, chan, 1, (int)cpar.Inv[chan]);
    Command2(2, chan, 2, (int)cpar.hS[chan]);
    if (cpar.on[chan]) {
      Command2(2, chan, 3, (int)cpar.Drv[chan]);
      Command2(2, chan, 4, (int)cpar.Thr[chan]);
    } else { // запрет срабатывания высоким порогом
      int tmp, max;
      cpar.GetParm("thresh", chan, cpar.Thr, tmp, max);
      // cpar.GetPar("thresh",module,chan,crs_ch[chan],tmp,tmp,max);
      // cout << "Off: " << (int) chan << " " << max << endl;
      Command2(2, chan, 3, 0);
      Command2(2, chan, 4, max);
    }

    Command2(2, chan, 5, (int)cpar.Pre[chan]);
    Command2(2, chan, 6, (int)cpar.Len[chan]);
  }

  Command2(2, 0, 7, (int)cpar.forcewr);
}

int CUSB::DoStartStop(int rst) {
  // rst=1 - reset timestamp; 0 - do not reset

  if (crs->f_read) {
    gzclose(crs->f_read);
    crs->f_read = 0;
  }

  if (!crs->b_acq) { // start

    int res = crs->OpenLog(crs->flog, 1, 0, opt.Filename); // daq
    if (res)
      return 1;

    // b_wdog=0;
    crs->t_wdog = 0;
    Free_Transfer();
#ifdef P_LIBUSB
    prnt("sss;", BYEL, "Sleep 50", RST);
#endif
    gSystem->Sleep(50);
    // cout << "Free_Transfer() finished" << endl;

    if (rst && crs->module >= 32 && crs->module <= 70) {
      Command32(8, 0, 0, 0); // сброс сч./буф.
      // Command32(9,0,0,0); //сброс времени
    }

    crs->InitBuf();
    // cout << "InitBuf() finished" << endl;
#ifdef ANA3
    Init_Transfer3();
#else  // ANA3
    Init_Transfer();
#endif // ANA3
    // cout << "Init_Transfer() finished" << endl;

    crs->DoReset(rst);
#ifdef P_LIBUSB
    cout << "DoReset() finished" << endl;
#endif
    if (rst) {
      HiFrm->DoRst();
    }
    // juststarted=true; already set in doreset

    TCanvas *cv = 0;

    if (!crs->batch) {
      parpar->Update();
      chanpar->Update();

      EvtFrm->Clear();
      EvtFrm->Pevents = &EvtFrm->Tevents;

      cv = EvtFrm->fCanvas->GetCanvas();
      cv->SetEditable(false);
      myM->UpdateTimer(rst);
      // myM->UpdateStatus(rst);
    }

    // if (module==32) {
    // Command32(7,0,0,0); //reset usb command
    // }

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

#ifdef P_LIBUSB
    auto t1 = std::chrono::system_clock::now();
    auto t2 = std::chrono::system_clock::now();
    std::time_t t_t1 = std::chrono::system_clock::to_time_t(t1);
    std::time_t t_t2 = std::chrono::system_clock::to_time_t(t2);
    Submit_all(crs->ntrans);
    cout << "Submit_all() finished: " << std::ctime(&t_t1) << " "
         << std::ctime(&t_t2) << endl;
#else
    Submit_all(crs->ntrans);
#endif

    // cout << "Period: " << opt.Period << endl;
    // opt.Period=5; //5ns for CRS module

    if (SetPar()) {
      return 3;
    }

    // buf_out[0]=3;
    crs->b_acq = true;
    crs->b_stop = false;
    // bstart=true;

    // Nsamp=0;
    // nsmp=0;

    cpar.F_start = gSystem->Now();
    // Text_time("S:",cpar.F_start);
    crs->txt_start = crs->Text_time("", cpar.F_start);

    if (crs->LogOK < 3) { // 1 или 2
      mtx_log.lock();
      if (crs->LogOK == 1)
        fprintf(crs->flog, "Start: %s\n", crs->txt_start.Data());
      else //=2
        fprintf(crs->flog, "Continue: %s\n", crs->txt_start.Data());
      fclose(crs->flog);
      mtx_log.unlock();
    }

    if (rst) {
      // if (opt.raw_write) {
      // 	Reset_Raw();
      // }
      // if (opt.dec_write) {
      // 	encoder->Reset_Dec(opt.dec_format);
      // }
      if (opt.fTxt) {
        crs->Reset_Txt();
      }
      //Added by Nikita
      if (opt.fBin && opt.nthreads==1) 
      {
		crs->Reset_Bin();
      }
      //END
    }

    ProcessCrs(rst);
    // ProcessCrs_old();

    // cout << "startstop1: " << endl;
    if (!crs->batch) {
      EvtFrm->Clear();
      EvtFrm->Pevents = &crs->Levents;
      EvtFrm->d_event = --EvtFrm->Pevents->end();
      // cout << "startstop2: " << endl;

      // gSystem->Sleep(opt.tsleep);
      gSystem->Sleep(10);
      crs->Show(true);
      cv->SetEditable(true);
    }
  } // start
  else { // stop
    if (crs->LogOK < 3) {
      mtx_log.lock();
      crs->flog = fopen(opt.Daqlog, "a");
      cpar.F_stop = gSystem->Now();
      TString txt_stop = crs->Text_time("", cpar.F_stop);
      fprintf(crs->flog, "Stop:  %s\n", txt_stop.Data());
      // cout << "stop: " << txt_stop << " " << opt.T_acq << endl;
      mtx_log.unlock();
    }

    // buf_out[0]=4;
    // cout << "Acquisition stopped" << endl;

    Command2(4, 0, 0, 0);
#ifdef P_LIBUSB
    prnt("sss;", BYEL, "Sleep 300", RST);
#endif
    prnt("sss;", BYEL, "TODO: Sleep 300", RST);
    gSystem->Sleep(300); // 1300 - проблема 3 в АК-32 устраняется
    crs->b_acq = false;

    // cout << "Sleep(13000)" << endl;

    // cout << "Acquisition stopped2" << endl;
    Cancel_all(crs->ntrans);
    crs->b_stop = true;
    crs->b_run = 2;
    // cout << "Acquisition stopped3" << endl;
    // gSystem->Sleep(1300); //1300 - проблема 3 в АК-32 устраняется
    cout << "Acquisition stopped" << endl;

    if (crs->LogOK < 3) {
      mtx_log.lock();
      fprintf(crs->flog, "Run time from PC: %f sec\n",
              (cpar.F_stop - cpar.F_start) * 0.001);
      fprintf(crs->flog, "Run time from DAQ: %f sec\n", opt.T_acq);
      fclose(crs->flog);
      mtx_log.unlock();
    }
  }

  return 0;
}

void CUSB::ProcessCrs(int rst) {
  crs->b_run = 1;
  crs->Ana_start(rst);

  // prnt("ssf d ls;",BBLU,"T_acq: ",opt.T_acq,crs->module,crs->Tstart64,RST);;
  // decode_thread_run=1;
  // tt1[3].Set();

  if (rst && crs->module >= 32 && crs->module <= 70) {
    // Command32(8,0,0,0); //сброс сч./буф.
    Command32(9, 0, 0, 0); // сброс времени
  }
  Command2(3, 0, 0, 0);

  while (!crs->b_stop) {
    if (!crs->batch) {
      crs->Show();
    } else {
      ;
    }
    gSystem->Sleep(10);
    gSystem->ProcessEvents();
    if (opt.Tstop && opt.T_acq > opt.Tstop) {
      // prnt("ssf f ls;",BRED,"Stop1!!!:
      // ",opt.Tstop,opt.T_acq,crs->Tstart64,RST);;
      DoStartStop(0);
      // cout << "Stop2!!!" << endl;
    }

#ifdef P_TEST
    // if (myM->testdelay) {
    //   gSystem->Sleep(myM->testdelay);
    //   EndAna(1);
    //   return;
    // }
#endif // P_TEST
  }

  // cout << "stopped:" << endl;
  crs->EndAna(1);

} // ProcessCrs

#endif // CYUSB
