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

#include "pulseclass.h"
#include "common.h"

typedef unsigned char byte;
//typedef unsigned long long ULong64_t;
//typedef long long Long64_t;

//using namespace std;

//---------------------------
class CRS {

RQ_OBJECT("CRS")

 public:
  CRS();
  virtual ~CRS();

  //--------constants---------
  static const int MAXTRANS=8;

  static const Int_t MAXEV=1000; //maximal number of events in Levents
  //--------variables---------

  gzFile f_raw;
  gzFile f_dec;

  Short_t Fmode; //1 - adcm raw; 2- crs2; 32 - crs32
  char Fname[255];
  UChar_t* Fbuf;

  Short_t nvp; //Vpulses "ring" counter
  std::vector<PulseClass> Vpulses[MAXTRANS];
  std::list<EventClass> Levents;
  //Short_t MAX_LAG; // maximal e_lag, equals to half of the event_buf
  Short_t e_lag; //event analysis lag - see also opt.event_lag

  Int_t module; //2 - crs2; 32 - crs32

  int ntrans; //number of "simultaneous" transfers

  //buffers for sending parameters...
  byte buf_out[64];
  byte buf_in[64];

  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];

  //timeval t_start, t_stop;
  Long64_t totalbytes;
  Long64_t writtenbytes;
  Long64_t npulses; //total number of pulses (zero at Reset (Start button))
  UInt_t npulses_buf; //pulses in the current buffer
  Long64_t nevents; //total number of events (zero at Reset (Start button))
  Long64_t nevents2; //number of analyzed/saved events
  Long64_t nbuffers; //total number of buffers (zero at Reset (Start button))
  double mb_rate;
  double ev_rate;

  int debug; // for printing debug messages

  bool b_acq; // true - acquisition running
  bool b_fana; // true - file analysis running
  bool b_stop; // true if acquisition or analysis has stopped


  //vars for decoding...
  //for crs32...
  ULong64_t *buf8; //buffer for 8-byte words
  unsigned char *buf1; //buffer for 1-byte words
  int r_buf8; //length of the buffer (in 8-byte words)
  int idx8; // current index in the buffer (in 8-byte words) 
  int idx1; // current index in the buffer (in 1-byte words)

  //for crs2...
  unsigned short* buf2; //buffer for 2-byte words
  int r_buf2; //length of the buffer (in 2-byte words)
  int idx2; // current index in the buffer (in 2-byte words)
  
  int frmt; //data format (byte 6)
  int cnt[MAX_CH]; // last 4 bits of the counter
  int cnt_prev[MAX_CH]; //previous cnt (may be used for counter consistency)
  int nsmp; //temporary Nsamp

  ULong64_t istamp64; //temporary tstamp64

  //--------functions---------

  int Detect_device();
  void DoExit();
  int SetPar();
  int Init_Transfer();
  void Free_Transfer();
  void Submit_all();
  void Cancel_all();
  //int Command_old(int len_out, int len_in); //send and receive command
  void Command_crs(byte cmd, byte chan, int par);
  void Command32(byte cmd, byte ch, byte type, int par);
  void Command2(byte cmd, byte ch, byte type, int par);
  //void SendParametr(const char* name, int len_out); //send one parameter
  int DoStartStop(); // start-stop acquisition
  void Reset(); //reset BPulses
  void DoFopen(char* oname);
  //void DoFAna();
  void FAnalyze();
  int Do1Buf();
  void DoNBuf();

  void AllParameters32(); // load all parameters
  //void AllParameters32_old(); // load all parameters
  void Decode32(UChar_t* buffer, int length);

  void AllParameters2(); // load all parameters
  void Decode2(UChar_t* buffer, int length);

  //void PrintPulse(int udata, bool pdata=false);

  //void PEvent() { if (b_pevent) Emit("PEvent()"); } //*SIGNAL*
  //void SigEvent() { Emit("SigEvent()"); } //*SIGNAL*
  void Event_Insert_Pulse(PulseClass* pulse);
  void Make_Events(int nbuf);

  ClassDef(CRS, 0)
};

#endif
