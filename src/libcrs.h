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
typedef unsigned long long ULong64_t;
typedef long long Long64_t;

//#define MAXTRANS 8
//const int MAXCH=32;
//const int ADDCH=8; //nai, bgo, si1, si2, stlb,demon,hpge,other

/*
class CRSPulse {
 public:
  CRSPulse();
  virtual ~CRSPulse() {};

  //--------variables---------

  //--------functions---------


};
*/
/*
//---------------------------
class MODPAR { //module parameters
 public:
  MODPAR();
  virtual ~MODPAR() {};

  // parameters of the crs32 or crs2 module

  int smooth[MAX_CH+ADDCH]; //Smooth - SS=0..10; S=2^SS
  int deadTime[MAX_CH+ADDCH]; // B = 1..16383
  int preWr[MAX_CH+ADDCH]; // pre-length M = 0..4094
  int durWr[MAX_CH+ADDCH]; // total length N = 1…32763 (internally - multiple of 4)
  unsigned int kderiv[MAX_CH+ADDCH]; // K = 0...1023; K=0 - trigger on the signal; k!=0 - on derivative
  int threshold[MAX_CH+ADDCH]; // T = –2048 .. +2047
  int adcGain[MAX_CH+ADDCH]; // G = 0..12
  bool acdc[MAX_CH+ADDCH]; // AC-1; DC-0
  bool inv[MAX_CH+ADDCH]; //0 - no inversion; 1 - inversion (individual)
  bool forcewr; //only for crs2
  // 0 - only triggered channel is written; 
  // 1 - both channels are written with any trigger

 public:
  void InitPar(int module);

  void GetPar(const char* name, int module, int i, int &par, int &min, int &max);

};
*/

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

  //PulseClass* BPulses[2]; //flip-flop pulses buffers
  //int NP; //number of BPulses buffer [0,1];

  gzFile f_raw;
  gzFile f_dec;

  Short_t Fmode;
  char Fname[255];
  UChar_t* Fbuf;

  Short_t nvp; //Vpulses "ring" counter
  std::vector<PulseClass2> Vpulses[MAXTRANS];
  std::list<EventClass1> Levents;
  //int i_ev; // - current Vevents buffer 
  Short_t MAX_LAG; // maximal e_lag, equals to half of the event_buf
  Short_t e_lag; //event analysis lag - see also opt.event_lag


  PulseClass* BPulses; //pulses buffer (ring buffer)
  int BP_len; //lengths of BPlses buffers
  int iBP; //index of BPulses;
  int BP_save; //frequency to save/analyze pulses buffer

  EventClass *BEvents; //events buffer (ring buffer)
  int EV_len;
  int iEV;
  int EV_draw; //frequency of drawing events
  int pul1,pul2,pul1a; //start-stop pulses for make_event; pul1a- tmp pulse

  PulseClass* ipp; //current Pulse

  int event_thread_run;//=1;
  //pthread_t tid1;

  Int_t module; //2 - crs2; 32 - crs32

  // parameters of the module
  //MODPAR parm;

  //static cyusb_handle *handle;
  //cyusb_handle *cy_handle;

  int ntrans; //number of "simultaneous" transfers
  //int buf_size; //size of one buffer in bytes
  //int chanPresent;

  //buffers for sending parameters...
  byte buf_out[64];
  byte buf_in[64];

  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];

  //timeval t_start, t_stop;
  ULong64_t totalbytes;
  Long64_t npulses; //total number of pulses (zero at Reset (Start button))
  UInt_t npulses_buf; //pulses in the current buffer
  Long64_t nevents; //total number of events (zero at Reset (Start button))
  Long64_t nbuffers; //total number of buffers (zero at Reset (Start button))
  

  int debug; // for printing debug messages
  bool bstart; //needed for FindStart
  bool b_acq; // true - acquisition running

  //Bool_t b_pevent;
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

  // pulse parameters
  //CRSPulse* pulse;

  //unsigned char Chan; //channel number
  //unsigned char Control; //Control word
  //ULong64_t Tstamp64; //actually Tstamp48
  //ULong64_t Counter; //pulse counter48
  //int Nsamp; //pulse length (must be equal to durWr[Chan])
  //float sData[100000];

  //--------functions---------

  int Detect_device();
  void DoExit();
  int SetPar();
  int Init_Transfer();
  void Free_Transfer();
  void Submit_all();
  void Cancel_all();
  //int Command_old(int len_out, int len_in); //send and receive command
  void Command32(byte cmd, byte ch, byte type, int par);
  void Command2(byte cmd, byte ch, byte type, int par);
  //void SendParametr(const char* name, int len_out); //send one parameter
  int DoStartStop(); // start-stop acquisition
  void Reset(); //reset BPulses
  void DoFopen(char* oname);
  void DoFAna();
  void FAnalyze();

  void AllParameters32(); // load all parameters
  //void AllParameters32_old(); // load all parameters
  void Decode32(UChar_t* buffer, int length);

  void AllParameters2(); // load all parameters
  void Decode2(UChar_t* buffer, int length);

  void PrintPulse(int udata, bool pdata=false);

  //void PEvent() { if (b_pevent) Emit("PEvent()"); } //*SIGNAL*
  //void SigEvent() { Emit("SigEvent()"); } //*SIGNAL*
  void ShowEvent();
  void Event_Insert_Pulse(PulseClass2* pulse);
  void Make_Events(int nbuf);

  //void Raw_Write(libusb_transfer *transfer);

  //void Created() { Emit("Created()"); } //*SIGNAL*
  //void Welcome() { printf("TestMainFrame has been created. Welcome!\n"); }

  ClassDef(CRS, 0)
};

#endif
