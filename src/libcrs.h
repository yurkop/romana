#ifndef crs_H
#define crs_H 1

//#include <pthread.h>
//#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <list>

#include <RQ_OBJECT.h>
//#include <TTree.h>
#include <TFile.h>
#include <TFormula.h>

#include "pulseclass.h"
#include "common.h"

typedef unsigned char byte;

typedef std::list<EventClass>::iterator event_iter;
typedef std::list<EventClass>::reverse_iterator event_reviter;

typedef std::pair<unsigned char*,int> Pair;

using namespace std;

/* struct pair { */
/*   char* buf; */
/*   size_t len; */
/* }; */

//---------------------------
class CRS {

RQ_OBJECT("CRS")

 public:
  CRS();
  virtual ~CRS();

  //--------constants---------
  static const int MAXTRANS=8;
  //static const int MAXTRANS=7;
  static const int MAXTRANS7=7;
  static const int RAWSIZE=10485760; //10 MB

  static const int DECSIZE=1048576; //1 MB
  static const int NDEC=100; // number of Dec buffers in ring

  //--------variables---------

  int MAXTRANS2; //real maxtrans, depends on usb_size
  
  gzFile f_raw;
  gzFile f_read;
  gzFile f_dec;
  bool juststarted;

  char raw_opt[5];
  char dec_opt[5];
  char Fname[255];

  string rawname;
  string decname;
  string rootname;
  string logname;

  UChar_t* DecBuf_ring;
  UChar_t* DecBuf; 
  ULong64_t* DecBuf8;
  Int_t idec; //index of DecBuf;
  std::list<Pair> decw_list;

  Int_t mdec1; //index of Dec buffer in ring for decoding
  Int_t mdec2; //index of Dec buffer in ring for writing
  bool b_decwrite[NDEC];
  Int_t dec_len[NDEC];

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
  Int_t N4; //количество раз, при которых L4 было >2.0
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
  //1-ADCM raw, 3 - ortec lis, 22 - crs2;
  //32 - old crs32, 33 - new crs32 with dsp, 34 - new crs32
  //41 - crs-8/16
  //51 - crs-128
  //72..79 - decoded file

  Int_t type_ch[MAX_CHTP];
  //0: 4-11bit;
  //1: crs-6/16 2-16bit;
  //2: crs-8/16 (16bit)

  //buffers for sending parameters...
  byte buf_out[64];
  byte buf_in[64];

  int ntrans; //number of "simultaneous" transfers

  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];

  Long64_t inputbytes;
  Long64_t rawbytes;
  Long64_t decbytes;
  Long64_t npulses; //total number of pulses (zero at Reset (Start button))
  //UInt_t npulses_buf; //pulses in the current buffer
  Long64_t nevents; //total number of events (zero at Reset (Start button))
  Long64_t nevents2; //number of analyzed/saved events
  Long64_t nbuffers; //total number of buffers (zero at Reset (Start button))
  //double mb_rate;
  //double ev_rate;
  Long64_t npulses2[MAX_CH]; //number of pulses per channel (softw)
  Long64_t npulses3[MAX_CH]; //number of pulses per channel (hardw)
  //Long64_t npulses3o[MAX_CH]; //number of pulses per channel (hardw)

  Int_t npulses_bad[MAX_CH]; //number of bad pulses per channel

  bool batch;
  bool silent;
  bool b_fstart;
  
  bool b_acq; // true - acquisition is running
  bool b_fana; // true - file analysis is running
  bool b_stop; // true if acquisition and analysis are stopped

  Int_t b_run; // used for trd_ana
  // b_run=0 - stop analysis immediately (pause)
  // b_run=1 - analyze events normally
  // b_run=2 - analyze all events, then stop 

  Long64_t Pstamp64; //previous tstamp (only for decode_adcm)
  Long64_t Offset64; //Tstamp offset in case of bad events

  Long64_t Tstart64; //Tstamp of the first event (or analysis/acquisition start)
  Long64_t Tstart0; //Tstamp of the ntof start pulses

  peak_type dummy_peak;
  PulseClass dummy_pulse;
  EventClass dummy_event;

  Int_t b_len[MAX_CH],
    p_len[MAX_CH],
    w_len[MAX_CH]; //length of window for bkg, peak and width integration in DSP

  Long64_t errors[MAX_ERR];
  Long64_t Counter[MAX_CH];

  string errlabel[MAX_ERR] = {
    "Bad buf start (obsolete):",
    "Bad channel:",
    "Channel mismatch:",
    "Bad frmt:",
    "Zero data:",
    "Wrong ADCM length:",
    "Bad ADCM Tstamp:",
    "Slow analysis:",
    "Slow decoding:",
    "Event lag exceeded:"
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
  const Int_t ING_Y=10000;
  const Int_t PROF_64=1000000;

  //--------functions---------

  //void Dummy_trd();

  void DoResetUSB();
#ifdef CYUSB
  int Detect_device();
  int SetPar();
  void Free_Transfer();
  void Submit_all(int ntr);
  void Cancel_all(int ntr);
  int Init_Transfer();
  int Command32_old(byte cmd, byte ch, byte type, int par);
  int Command32(byte cmd, byte ch, byte type, int par);
  void Check33(byte cmd, byte ch, int &a1, int &a2, int min, int max);
  int Command2(byte cmd, byte ch, byte type, int par);
  void AllParameters2(); // load all parameters
  void AllParameters41(); // load all parameters
  void AllParameters34(); // load all parameters
  void AllParameters33(); // load all parameters
  void AllParameters32(); // load all parameters
  int DoStartStop(); // start-stop acquisition
  void ProcessCrs(); // new process events in dostartstop
#endif


  void DoExit();
  //int Command_old(int len_out, int len_in); //send and receive command
  //void SendParametr(const char* name, int len_out); //send one parameter
  void DoReset(); //reset BPulses
  void DoFopen(char* oname, int popt);
  int ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2);
  void SaveParGz(gzFile &ff, Short_t mod);
  void DoProf(Int_t nn, Int_t *aa, Int_t off);
  void Make_prof_ch();

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

  void PulseAna(PulseClass &ipls);
  void Dec_Init(eventlist* &Blist, UChar_t frmt);
  void Dec_End(eventlist* &Blist, UInt_t iread);
  void Decode79(UInt_t iread, UInt_t ibuf);
  void Decode78(UInt_t iread, UInt_t ibuf);
  void Decode77(UInt_t iread, UInt_t ibuf);
  void Decode76(UInt_t iread, UInt_t ibuf);
  void Decode75(UInt_t iread, UInt_t ibuf);
  void Decode33(UInt_t iread, UInt_t ibuf);
  void Decode34(UInt_t iread, UInt_t ibuf);
  void Decode2(UInt_t iread, UInt_t ibuf);
  void Decode_adcm(UInt_t iread, UInt_t ibuf);

  int Searchsync(int &idx, UInt_t* buf4, int end);

  //int Set_Trigger();
  void Ana_start();
  void Ana2(int all);

  void Event_Insert_Pulse(eventlist *Elist, PulseClass* pls);
  void Make_Events(std::list<eventlist>::iterator BB);
  void Select_Event(EventClass *evt);

  void Reset_Raw();
  void Reset_Dec(Short_t mod);

  void Fill_Dec75(EventClass* evt);
  void Fill_Dec76(EventClass* evt);
  void Fill_Dec77(EventClass* evt);
  void Fill_Dec78(EventClass* evt);
  void Fill_Dec79(EventClass* evt);
  //void Flush_Dec_old();
  int Wr_Dec(UChar_t* buf, int len);
  void Flush_Dec();

  void Fill_Raw(EventClass* evt);
  void Flush_Raw();
  void Flush_Raw_MT(UChar_t* buf, int len);

  //void Print_Pulses();
  void Print_Events(const char* file=0);
  void Print_Peaks(const char* file=0);
  void Print_b1(int idx1, std::ostream *out);
  void Print_Buf_err(UInt_t ibuf, const char* file=0);
  void Print_Buf8(UChar_t* buf, Long64_t size, const char* file=0);
  ClassDef(CRS, 0)
};

#endif
