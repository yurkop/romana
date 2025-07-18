#ifndef crs_H
#define crs_H 1

#ifdef CYUSB
#include "cyusb.h"
#endif

//#include <pthread.h>
//#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <list>

#include <RQ_OBJECT.h>
//#include <TTree.h>
#include <TFile.h>
#include <TFormula.h>
#include <TMath.h>

#include "pulseclass.h"
#include "common.h"

#include <queue>          // std::queue


//#include <TServerSocket.h>
//#include <TMonitor.h>

enum ERR_NUM {
  ER_START=0,
  ER_CH,
  ER_MIS,
  ER_FRMT,  
  ER_LEN,
  ER_ZERO,
  ER_ALEN,
  ER_TST,

  ER_AREA,
  ER_BASE,
  ER_WIDTH,
  ER_TIME,
  
  ER_ANA,
  ER_DEC,
  ER_LAG,
  ER_OVF,
};

#define mask_e "TN"
#define mask_p "AtWHBSsRrp"

//typedef unsigned char byte;

typedef std::list<EventClass>::iterator event_iter;
typedef std::list<EventClass>::reverse_iterator event_reviter;

typedef std::pair<unsigned char*,int> Pair;

using namespace std;

/* struct pair { */
/*   char* buf; */
/*   size_t len; */
/* }; */

class PkClass {
public:
  Long64_t QX;
  Int_t C,A,AY;
  Int_t RX;
  Short_t H;
  UChar_t E;
};

class BufClass {
public:
  char* Buf;
  size_t Size;
public:
  BufClass(size_t sz);
  ~BufClass();
};

#ifdef SOCK
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h> // For poll and pollfd

class SockClass {
  //RQ_OBJECT("SockClass")
public:
  int sockfd=0, //
    newsockfd=0;
  socklen_t clilen;
  char buffer[0x10000];
  struct sockaddr_in serv_addr, cli_addr;
  struct pollfd fds[1]; // Or more if you're monitoring other sockets

  std::queue<TString> l_com;
  std::queue<TString> l_par;

public:
  SockClass(int portno);
  ~SockClass();
  void Poll();
  //void Handle();
  void Eval_Buf();
  void Eval_Par();
  void Eval_Com();
  //void DoReset();
  //void Created() { Emit("Created()"); } //*SIGNAL*
};
#endif

//---------------------------
class CRS {

RQ_OBJECT("CRS")

 public:
  CRS();
  virtual ~CRS();

  //--------constants---------
  static const int MAXTHREADS=8;
  static const int MAXTRANS=7; //было 8
  //static const int MAXTRANS=7;
  //static const int MAXTRANS7=7;
  static const int RAWSIZE=10485760; //10 MB

  static const int DECSIZE=1048576; //1 MB
  static const int NDEC=300; // number of Dec buffers in ring

  //--------variables---------

  //int MAXTRANS2; //real maxtrans, depends on usb_size
  
  gzFile f_raw;
  gzFile f_read;
  gzFile f_dec;
  bool juststarted;

  std::ostream *txt_out;
  //std::streambuf *txt_buf;
  std::ofstream txt_of;

  char raw_opt[5];
  char dec_opt[5];
  char Fname[255]; //имя файла для чтения/обработки

  string rawname;
  string decname;
  string rootname;
  string logname;

  UChar_t* DecBuf_ring;
  UChar_t* DecBuf; 
  UChar_t* DecBuf1; //for Fill_Dec80+ 
  ULong64_t* DecBuf8;
  Int_t idec; //index of DecBuf;
  std::list<Pair> decw_list;

  Int_t mdec1; //index of Dec buffer in ring for decoding
  Int_t mdec2; //index of Dec buffer in ring for writing
  bool b_decwrite[NDEC];
  Int_t dec_len[NDEC];

  //std::string smask_e,smask_p; //полные маски для e(event) и p(pulse)
  std::string sdec_e,sdec_p; //реальные маски для e(event) и p(pulse)
  bool sdec_d,sdec_c; // маски для d (Data) и c (Counter)
  //std::vector<int> vdec;

  std::list<Pair> rw_list;
  UChar_t* RawBuf;
  //ULong64_t* RawBuf8;
  Int_t iraw; //index of RawBuf;

  typedef std::list<EventClass> eventlist;
  typedef std::list<EventClass>::iterator evlist_iter;
  typedef std::list<EventClass>::reverse_iterator evlist_reviter;

  eventlist Levents; //global list of events
  std::list<eventlist> Bufevents; //list of buffers for decoding


  //Double_t DT4; // Время, затраченное на 1 цикл handle_ana
  Double_t L4; //Levents.size at L4 (after erase in handle_ana)
  Int_t N4; //количество раз подряд, при которых L4 было >2.0
  size_t LMAX; //maximal length of Levents
  Int_t SLP; //sleep: increased if N4>3


  // анализируем данные от m_start до m_event  
  //new events can be inserted only after m_event (up to Levents.end())
  //m_event points to the first event, which is not yet analyzed
  //it is safe to fill events starting from this element
  //m_start - temporary iterator for analysing events

  //Int_t m_flag; //flag used for cleaning the event list
  // 0 - after cleaning list or at the beginning
  // 1 - after setting min. marker -> list can be cleaned if > ev_max
  // 2 - list is analyzed, but not cleaned (at the end of file etc)

  Short_t Fmode; //0 - do nothing; 1 - CRS module online; 2 - file analysis

  Short_t module;
  // 1 - ADCM raw
  // 3 - ADCM dec
  // 7? - ortec lis,
  // 22 - crs2;
  // 32 - old crs32, 33 - crs32 with dsp/ПО3, 34 - crs32/ПО4
  // 35 - crs32/ПО5-6
  // 36 - crs32/ПО7
  // 41,42,43,44 - crs-8/16
  // 51,52,53,54 - crs-128
  // 43,53,44,54 - new format (decode35)
  // 45 - AK-32
  // 72..80 - decoded file
  // 17 - simulated data

  //Int_t type_ch[MAX_CHTP];
  //0: 4-11bit;
  //1: crs-6/16 2-16bit;
  //2: crs-8/16 (16bit)

  Int_t crs_ch[MAX_CHTP]; //CRS channel type:
  //0 - undefined
  //1 - CRS-2
  //2 - CRS-32 11 bit
  //3 - CRS-16 16 bit
  //4 - CRS-8
  //5 - CRS-128
  //6 - AK32 14 bit
  //7 - AK32 12 bit

  //buffers for sending parameters...
  UChar_t buf_out[64];
  UChar_t buf_in[64];

  int ntrans; //actual number of "simultaneous" transfers

  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];

  Long64_t inputbytes;
  Long64_t rawbytes;
  Long64_t decbytes;
  Long64_t npulses; //total number of pulses (zero at Reset (Start button))
  //UInt_t npulses_buf; //pulses in the current buffer
  Long64_t nevents; //total number of events (zero at Reset (Start button))
  Long64_t mtrig; //number of analyzed/saved (master trigger) events
  Long64_t nbuffers; //total number of buffers (zero at Reset (Start button))
  //double mb_rate;
  //double ev_rate;
  Long64_t npulses2[MAX_CH+1]; //number of pulses per channel (softw)
  Long64_t npulses2o[MAX_CH+1]; //old number of pulses per channel (softw)
  Long64_t npulses3o[MAX_CH+1]; //old number of pulses per channel (hardw)
  Long64_t Tst3o[MAX_CH+1]; //old TimeStamp per channel (hardw)
  double rate_soft[MAX_CH+1]; //pulse rate per channel (softw)
  double rate_hard[MAX_CH+1]; //pulse rate per channel (hardw)
  //Long64_t npulses3o[MAX_CH]; //number of pulses per channel (hardw)
  double npulses_bad[MAX_CH+1]; //number of bad pulses per channel

  Long64_t fCounter[MAX_CH]; //old Counter
  Long64_t fTime[MAX_CH]; //old Tstamp for Counter

  //int nchan_on;

  bool batch; //batch mode
  bool abatch; //1 - acquisition in batch; 0 - file in batch
  int scrn; //screen output in batch mode

  bool b_noheader;

  bool b_acq; // true - acquisition is running
  bool b_fana; // true - file analysis is running
  bool b_stop; // true if acquisition and analysis are stopped
  //bool b_sockana=false;

  Int_t b_run; // used for trd_ana
  // b_run=0 - stop analysis immediately (pause)
  // b_run=1 - analyze events normally
  // b_run=2 - analyze all events, then stop 

  Long64_t Pstamp64; //previous tstamp (only for decode_adcm)
  Long64_t Offset64; //Tstamp offset (обычно равен Tstart64)

  Long64_t Tstart64; //Tstamp of the first event (or analysis/acquisition start)
  //Long64_t Tstart0; //Tstamp of the ntof start pulses
  //Float_t Time0; //Exact time of the ntof start pulses
  char txt_start[30]; //local text copy of F_start, start of the acquisition

  Double_t sPeriod; // Tstmp*sPeriod = sec
  // Double_t nsPeriod; // Tstmp*nsPeriod = nsec

  //PeakClass dummy_peak;
  PulseClass dummy_pulse;
  PulseClass good_pulse;
  EventClass dummy_event;
  EventClass good_event;

  Double_t b_len[MAX_CH],
    p_len[MAX_CH],
    w_len[MAX_CH]; //length of window for bkg, peak and width integration in DSP
  Double_t b_mean[MAX_CH],
    p_mean[MAX_CH],
    w_mean[MAX_CH]; //length of window for bkg, peak and width integration in DSP

  Bool_t use_2nd_deriv[MAX_CH]; //use 2nd deriv in pulseana33

  Long64_t Counter[MAX_CH];

  Long64_t errors[MAX_ERR];
  string errlabel[MAX_ERR] = {
    "Bad buf start (obsolete):", //ER_START,
    "Bad channel:",              //ER_CH,
    "Channel mismatch:",         //ER_MIS,
    "Bad frmt:",                 //ER_FRMT,
    "Bad length:",               //ER_LEN,
    "Zero data:",                //ER_ZERO,
    "Wrong ADCM length:",        //ER_ALEN,
    "Bad ADCM Tstamp:",          //ER_TST,

    "No area:",                  //ER_AREA
    "No baseline:",              //ER_BASE
    "No width:",                 //ER_WIDTH
    "No time:",                  //ER_TIME

    "Slow analysis:",            //ER_ANA,
    "Slow decoding:",            //ER_DEC,
    "Event lag exceeded:",       //ER_LAG,    
    "OVF:"                       //ER_OVF,    
  };
  Int_t prof_ch[MAX_CH];
  // -1: nothing;
  // 0..10: Prof8_x
  // 10..100: Prof8_y
  // 100..10000: Ing_x
  // 10000..1000000: Ing_y
  // >1000000: Prof64
  const Int_t PROF_X=0;
  const Int_t PROF_Y=10;
  const Int_t ING_X=100;
  const Int_t ING_Y=1000;
  //const Int_t ING_9=10000;  
  const Int_t PROF_64=1000000;

  //bool chan_changed = false;

  //TServerSocket *serv_sock;
  //TMonitor *mon;


  //SockClass *gsock;
  // int sockfd, newsockfd, portno;
  // socklen_t clilen;
  // char buffer[256];
  // struct sockaddr_in serv_addr, cli_addr;


  //--------functions---------

  //void Dummy_trd();

  void DoDetectDev();
  void DoResetUSB();

  //---CRS
  short idev; // номер девайса в cyusb [0,1,2...].
              //Если =-1 -> номер девайса еще не выбран
  short ndev;
  vector<string> cy_list; // список имен ВСЕХ подключенных девайсов
  TString *devname; // имя, заданное в '-m'. Если =0 -> не задано
#ifdef CYUSB
  cyusb_handle *cy_handle;

  int Open_USB();
  void Set_USB(int i);
  void Device_info();
  int Init_device();
  int SetPar();
  void Free_Transfer();
  void Submit_all(int ntr);
  void Cancel_all(int ntr);
  int Init_Transfer();
  int Command32_old(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  void Check33(UChar_t cmd, UChar_t ch, int &a1, int &a2, int min, int max);
  int Command2(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  void AllParameters2(); // load all parameters
  //void AllParameters41(); // load all parameters
  //void AllParameters42(); // load all parameters
  //void AllParameters43(); // load all parameters
  void AllParameters45(); // load all parameters
  void AllParameters44(); // load all parameters
  void AllParameters36(); // load all parameters
  void AllParameters35(); // load all parameters
  void AllParameters34(); // load all parameters
  void AllParameters33(); // load all parameters
  void AllParameters32(); // load all parameters
  int DoStartStop(int rst); // start-stop acquisition
  void ProcessCrs(int rst); // process events in dostartstop
#endif


  void Text_time(const char* hd, Long64_t f_time);
  void DoExit();
  //int Command_old(int len_out, int len_in); //send and receive command
  //void SendParametr(const char* name, int len_out); //send one parameter
  void DoReset(int rst=1); //reset BPulses
  int DoFopen(char* oname, int copt, int popt);
  void After_ReadPar(int op);
  int ReadParGz(gzFile &ff, char* pname, int m1, int cp, int op);
  void SaveParGz(gzFile &ff, Short_t mod);
  void DoProf(Int_t nn, Int_t *aa, Int_t off);
  void Make_prof_ch();

  //int CountChan();

  void InitBuf();
  void StopThreads(int all);
  void EndAna(int all);
  void FAnalyze2(bool nobatch);
  void AnaBuf(int loc_ibuf);
  int DoBuf();
  void DoNBuf2(int nb);
  void Show(bool force=false);

  void Decode_switch(UInt_t ibuf);
  void Decode_any_MT(UInt_t iread, UInt_t ibuf, int loc_ibuf);
  void Decode_any(UInt_t ibuf);

  // FindLast* находит конец текущего буфера b_end[ibuf],
  // что является одновременно началом следующего b_start[ibuf2]
  void FindLast(UInt_t ibuf, int loc_ibuf, int what);

  bool MakeDecMask();
  void CheckDSP(PulseClass &ipls, PulseClass &ipls2);
  void PulseAna(PulseClass &ipls);
  void Dec_Init(eventlist* &Blist, UChar_t frmt);
  void Dec_End(eventlist* &Blist, UInt_t iread, UChar_t sp);
  void Decode81(UInt_t iread, UInt_t ibuf);
  void Decode80(UInt_t iread, UInt_t ibuf);
  void Decode79(UInt_t iread, UInt_t ibuf);
  //void Decode79a(UInt_t iread, UInt_t ibuf);
  void Decode78(UInt_t iread, UInt_t ibuf);
  void Decode77(UInt_t iread, UInt_t ibuf);
  void Decode76(UInt_t iread, UInt_t ibuf);
  void Decode75(UInt_t iread, UInt_t ibuf);

  void MakePk(PkClass &pk, PulseClass &ipls);

  //void Decode33(UInt_t iread, UInt_t ibuf);
  //void Decode42(UInt_t iread, UInt_t ibuf);
  void Decode34(UInt_t iread, UInt_t ibuf);
  void Decode35(UInt_t iread, UInt_t ibuf);
  void Decode2(UInt_t iread, UInt_t ibuf);
  void Decode_adcm(UInt_t iread, UInt_t ibuf);
  void Decode_adcm_dec(UInt_t iread, UInt_t ibuf);

  int Searchsync(Long64_t &idx, UInt_t* buf4, Long64_t end);
  int Detect_adcm();
  //int Find_adcmraw_start();

  //int Set_Trigger();
  void Ana_start();
  void Ana2(int all);

  void Event_Insert_Pulse(eventlist *Elist, PulseClass* pls);
  void Make_Events(std::list<eventlist>::iterator BB);
  //void Select_Event(EventClass *evt);

  void Reset_Raw();
  void Reset_Dec(Short_t mod);
  void Reset_Txt();

  void Fill_Dec75(EventClass* evt);
  void Fill_Dec76(EventClass* evt);
  void Fill_Dec77(EventClass* evt);
  void Fill_Dec78(EventClass* evt);
  void Fill_Dec79(EventClass* evt);
  void Fill_Dec80(EventClass* evt);
  void Fill_Dec81(EventClass* evt);

  void Fill_Dec82(EventClass* evt);

  //void Fill_Txt(EventClass* evt);

  void Fill_Dec_Simul();
  //void Flush_Dec_old();
  int Wr_Dec(UChar_t* buf, int len);
  void Flush_Dec();

  void Fill_Raw(EventClass* evt);
  void Flush_Raw();
  void Flush_Raw_MT(UChar_t* buf, int len);

  //void Print_Pulses();
  void Print_OneEvent(EventClass* evt);
  void Print_Events(const char* file=0);
  void Print_Peaks(const char* file=0);
  void Print_b1(int idx1, std::ostream *out);
  void Print_Buf_err(UInt_t ibuf, const char* file=0);
  void Print_Buf8(UChar_t* buf, Long64_t size, const char* file=0);

  // void SimulateInit();
  // void SimNameHist();
  // void SimulatePulse(int ch, Long64_t tst, double pos);
  // void SimulateOneEvent(Long64_t Tst);
  // void SimulateEvents(Long64_t n_evts, Long64_t Tst0);

  ClassDef(CRS, 0)
};

#endif
