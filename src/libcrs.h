#ifndef crs_H
#define crs_H 1

//#include "cyusb.h"

//#include <pthread.h>
#include <libusb-1.0/libusb.h>
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

#include "pulseclass.h"
#include "common.h"

typedef unsigned char byte;
//typedef unsigned long long ULong64_t;
//typedef long long Long64_t;

typedef std::vector<PulseClass> pulse_vect;
//typedef std::list<EventClass> event_list;

//typedef std::list<event_list>::iterator event_list_iter;
//typedef std::list<event_list>::reverse_iterator event_list_reviter;

typedef std::list<pulse_vect>::iterator list_pulse_iter;
typedef std::list<pulse_vect>::reverse_iterator list_pulse_reviter;

typedef std::list<EventClass>::iterator event_iter;
typedef std::list<EventClass>::reverse_iterator event_reviter;

//using namespace std;

//---------------------------
class CRS {

RQ_OBJECT("CRS")

 public:
  CRS();
  virtual ~CRS();

  //--------constants---------
  static const int MAXTRANS=8;
  static const int DECSIZE=1048576; //1 MB

  //static const Int_t MAXEV=1000; //maximal number of events in Levents
  //--------variables---------

  int MAXTRANS2; //real maxtrans, depends on usb_size
  Int_t Pre[MAX_CH+ADDCH]; // pre-length for the analysys
  //Pre = opt.preWr for crs2/32; Pre=0 for adcm
  
  gzFile f_raw;
  gzFile f_read;
  gzFile f_dec;
  bool justopened;
  //TFile* f_dec;

  //TFile* f_tree;
  //TTree* Tree;

  char raw_opt[5];
  char dec_opt[5];
  Short_t Fmode; //1 - adcm raw; 2- crs2; 32 - crs32
  char Fname[255];
  UChar_t* Fbuf;
  UChar_t* Fbuf2;

  UChar_t* DecBuf;
  Int_t idec; //index of DecBuf;

  //Vpulses - list of vectors of pulses for Decode*
  // size of Vpulses can not be larger than 2
  // (contains current vector and previous vector)
  std::list<pulse_vect> Vpulses;

  //std::list<event_list> Levents; //list of events
  std::list<EventClass> Levents; //list of events

  // n_ana - number of events which are already analyzed, but not erased,
  // starting from "start"
  //int n_ana;

  std::list<EventClass>::iterator m_start;
  std::list<EventClass>::iterator m_event;
  //new events can be inserted only after m_event (up to Levents.end())
  //m_event points to the first event, which is not yet analyzed
  //it is safe to fill events starting from this element
  //m_start - temporary iterator for analysing events

  //Int_t m_flag; //flag used for cleaning the event list
  // 0 - after cleaning list or at the beginning
  // 1 - after setting min. marker -> list can be cleaned if > ev_max
  // 2 - list is analyzed, but not cleaned (at the end of file etc)

  Short_t module; //2 - crs2; 32 - crs32
  Int_t period;

  int ntrans; //number of "simultaneous" transfers

  //buffers for sending parameters...
  byte buf_out[64];
  byte buf_in[64];

  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];

  //timeval t_start, t_stop;
  Long64_t T_start; //start of the acuisition/analysis
  Long64_t totalbytes;
  Long64_t writtenbytes;
  Long64_t npulses; //total number of pulses (zero at Reset (Start button))
  UInt_t npulses_buf; //pulses in the current buffer
  Long64_t nevents; //total number of events (zero at Reset (Start button))
  Long64_t nevents2; //number of analyzed/saved events
  Long64_t nbuffers; //total number of buffers (zero at Reset (Start button))
  //double mb_rate;
  //double ev_rate;
  Int_t npulses2[MAX_CH]; //number of pulses per channel

  bool b_acq; // true - acquisition is running
  bool b_fana; // true - file analysis is running
  bool b_stop; // true if acquisition and analysis are stopped
  Int_t b_run; // used for trd_ana
  // b_run=0 - stop analysis immediately (pause)
  // b_run=1 - analyze events normally
  // b_run=2 - analyze all events, then stop 

  Long64_t T_last;

  //vars for decoding...

  //for adcm
  int idx; //index for Decode_adcm (in 32bit words, rbuf4)
  int rLen; // length of one m-link frame
  int BufLength; //length of the read buffer
  int idnext; //next expected idx pointing to new syncw
  int lastfl; //transient last fragment flag
  UInt_t* rbuf4; //only for decode_adcm
  UShort_t* rbuf2; //only for decode_adcm
  
  //for crs32...
  //ULong64_t *buf8; //buffer for 8-byte words
  //unsigned char *buf1; //buffer for 1-byte words
  //int r_buf8; //length of the buffer (in 8-byte words)
  //int idx8; // current index in the buffer (in 8-byte words) 
  //int idx1; // current index in the buffer (in 1-byte words)

  //for crs2...
  //unsigned short* buf2; //buffer for 2-byte words
  //int r_buf2; //length of the buffer (in 2-byte words)
  //int idx2; // current index in the buffer (in 2-byte words)
  
  //int frmt; //data format (byte 6)
  //int cnt[MAX_CH]; // last 4 bits of the counter
  //int cnt_prev[MAX_CH]; //previous cnt (may be used for counter consistency)
  //int nsmp; //temporary Nsamp

  //ULong64_t istamp64; //temporary tstamp64
  Long64_t Tstart64; //Tstamp of the first event (or analysis/acquisition start)
  Long64_t Tstart0; //Tstamp of the m_tof start pulses

  //Long64_t rTime; //Tstamp of decoded event
  //Char_t rState; //State of decoded event
  rpeak_type rP;
  std::vector<rpeak_type> rPeaks;

  //--------functions---------

  //void Dummy_trd();
  int Detect_device();
  void DoExit();
  int SetPar();
  int Init_Transfer();
  void Free_Transfer();
  void Submit_all(int ntr);
  void Cancel_all(int ntr);
  //int Command_old(int len_out, int len_in); //send and receive command
  void Command_crs(byte cmd, byte chan, int par);
  void Command32(byte cmd, byte ch, byte type, int par);
  void Command2(byte cmd, byte ch, byte type, int par);
  //void SendParametr(const char* name, int len_out); //send one parameter
  int DoStartStop(); // start-stop acquisition
  void DoReset(); //reset BPulses
  void DoFopen(char* oname, int popt);
  int ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2);
  void SaveParGz(gzFile &ff);

  //void DoFAna();
  void FAnalyze();
  int DoBuf();
  void DoNBuf(int nb);
  void Show(bool force=false);

  void AllParameters32(); // load all parameters
  //void AllParameters32_old(); // load all parameters
  void Decode32(UChar_t* buffer, int length);

  void AllParameters2(); // load all parameters
  void Decode2(UChar_t* buffer, int length);

  int Searchsync();
  void Decode_adcm();

  //void PrintPulse(int udata, bool pdata=false);

  //void PEvent() { if (b_pevent) Emit("PEvent()"); } //*SIGNAL*
  //void SigEvent() { Emit("SigEvent()"); } //*SIGNAL*
  void Event_Insert_Pulse(PulseClass* pulse);
  void Make_Events();
  void Select_Event(EventClass *evt);
  //void *Ana_Events(void* ptr);

  //void NewTree();
  //void CloseTree();
  void Reset_Dec();
  void Fill_Dec(EventClass* evt);
  void Flush_Dec();

  void Print_Pulses();
  void Print_Events();
  ClassDef(CRS, 0)
};

#endif
