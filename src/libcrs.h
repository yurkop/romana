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
//typedef unsigned long long ULong64_t;
//typedef long long Long64_t;

//typedef std::vector<PulseClass> pulse_vect;
//typedef std::list<EventClass> event_list;

//typedef std::list<event_list>::iterator event_list_iter;
//typedef std::list<event_list>::reverse_iterator event_list_reviter;

//typedef std::list<pulse_vect>::iterator list_pulse_iter;
//typedef std::list<pulse_vect>::reverse_iterator list_pulse_reviter;

typedef std::list<EventClass>::iterator event_iter;
typedef std::list<EventClass>::reverse_iterator event_reviter;

using namespace std;

/*
#pragma pack (push, 1)
struct rpeak_type73 {
  Float_t Area;
  //Float_t Height;
  Float_t Width;
  Float_t Time; //exact time relative to pulse start (from 1st deriv)
  UChar_t Ch; //Channel number
  //UChar_t Type; //peak type
};
#pragma pack (pop)

#pragma pack (push, 1)
struct rpeak_type74 {
  Short_t Area;
  //Float_t Height;
  //Short_t Width;
  Short_t Time; //exact time relative to pulse start (from 1st deriv)
  UChar_t Ch; //Channel number
  //UChar_t Type; //peak type
};
#pragma pack (pop)
*/

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
  static const int DECSIZE=1048576; //1 MB

  //static const Int_t MAXEV=1000; //maximal number of events in Levents
  //--------variables---------

  int MAXTRANS2; //real maxtrans, depends on usb_size
  //Int_t Pre[MAX_CH+MAX_TP]; // pre-length for the analysys
  //Pre = opt.preWr for crs2/32; Pre=0 for adcm
  
  gzFile f_raw;
  gzFile f_read;
  gzFile f_dec;
  bool juststarted;
  //TFile* f_dec;

  //TFile* f_tree;
  //TTree* Tree;

  char raw_opt[5];
  char dec_opt[5];
  Short_t Fmode; //0 - do nothing; 1 - CRS module online; 2 - file analysis
  char Fname[255];

  string rawname;
  string decname;
  string rootname;

  UChar_t* DecBuf;
  ULong64_t* DecBuf8;

  Int_t idec; //index of DecBuf;

  // struct Pstruct {
  //   UInt_t num;
  //   bool done;
  //   pulse_vect Vpulses;
  // };

  // std::list<Pstruct> plist;
  // typedef std::list<Pstruct>::iterator plist_iter;

  //Vpulses - list of vectors of pulses for Decode*
  // size of Vpulses can not be larger than 2
  // (contains current vector and previous vector)
  //std::list<pulse_vect> Vpulses;


  //YK pulse_vect Vpulses[MAXTRANS];


  //int nvp; //Vpulses index
  //pulse_vect *vv; //- vector of pulses from current buffer
  //pulse_vect *vv2; //- vector of pulses from previous buffer

  //pulse_vect::iterator ipls; //pointer to the current pulse in decode*
  //peak_type *ipk; //pointer to the current peak in the current pulse;
  //Double_t QX,QY,RX,RY;
  //Int_t n_frm; //counter for frmt4 and frmt5

  //std::list<event_list> Levents; //list of events
  typedef std::list<EventClass> eventlist;
  typedef std::list<EventClass>::iterator evlist_iter;
  typedef std::list<EventClass>::reverse_iterator evlist_reviter;

  eventlist Levents; //list of events
  std::list<eventlist> Bufevents;
  
  //EventClass mean_event;

  ///////##std::list<EventClass>::iterator m_start;


  //std::list<EventClass>::iterator m_event;


  // анализируем данные от m_start до m_event  
  //new events can be inserted only after m_event (up to Levents.end())
  //m_event points to the first event, which is not yet analyzed
  //it is safe to fill events starting from this element
  //m_start - temporary iterator for analysing events

  //Int_t m_flag; //flag used for cleaning the event list
  // 0 - after cleaning list or at the beginning
  // 1 - after setting min. marker -> list can be cleaned if > ev_max
  // 2 - list is analyzed, but not cleaned (at the end of file etc)

  Short_t module;
  //1-ADCM raw, 2 - crs2; 32 - old crs32, 33 - new crs32 with dsp

  Int_t type_ch[MAX_CH+MAX_TP]; //0-   4-11bit; 1-   2-16bit
  //Short_t ver_po;
  Int_t period;

  //buffers for sending parameters...
  byte buf_out[64];
  byte buf_in[64];

  int ntrans; //number of "simultaneous" transfers

  //int ibuf;//index of Fbuf[*]

  //int buf_off[MAXTRANS];
  //int buf_len[MAXTRANS];
  //unsigned char *buftr2[MAXTRANS];
  unsigned char *buftr[MAXTRANS];
  struct libusb_transfer *transfer[MAXTRANS];
  //UChar_t* Fbuf[MAXTRANS];
  //UChar_t* Fbuf2[MAXTRANS];

  //timeval t_start, t_stop;
  //Long64_t T_start; //start of the acuisition/analysis
  //Float_t F_acq; //file acquisition time
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

  Int_t npulses_bad[MAX_CH]; //number of bad pulses per channel

  //bool b_usbbuf;

  bool batch;
  bool b_fstart; // 
  
  bool b_acq; // true - acquisition is running
  bool b_fana; // true - file analysis is running
  bool b_stop; // true if acquisition and analysis are stopped
  Int_t b_run; // used for trd_ana
  // b_run=0 - stop analysis immediately (pause)
  // b_run=1 - analyze events normally
  // b_run=2 - analyze all events, then stop 

  //Long64_t T_last_good; //tstamp of the previous good event
  Long64_t Pstamp64; //previous tstamp (only for decode_adcm)
  Long64_t Offset64; //Tstamp offset in case of bad events

  //ULong64_t istamp64; //temporary tstamp64
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
  //0 - bad buf start
  //1 - ch>=Nchan
  //2 - channel mismatch
  //3 - bad frmt

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
  //void Command_crs(byte cmd, byte chan);
  void AllParameters34(); // load all parameters
  void AllParameters33(); // load all parameters
  void AllParameters32(); // load all parameters
  void AllParameters2(); // load all parameters
  int DoStartStop(); // start-stop acquisition
  void ProcessCrs(); // new process events in dostartstop
  //void ProcessCrs_old(); // old process events in dostartstop
#endif



  void DoExit();
  //int Command_old(int len_out, int len_in); //send and receive command
  //void SendParametr(const char* name, int len_out); //send one parameter
  void DoReset(); //reset BPulses
  void DoFopen(char* oname, int popt);
  int ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2);
  void SaveParGz(gzFile &ff, Short_t mod);

  //void DoFAna();
  //void FAnalyze(bool nobatch);
  void InitBuf();
  void EndAna(int all);
  void FAnalyze2(bool nobatch);
  void AnaBuf();
  int DoBuf();
  //void DoNBuf(int nb);
  void DoNBuf2(int nb);
  void Show(bool force=false);

  //void AllParameters32_old(); // load all parameters
  //void Decode_any(UChar_t** buffer, int length, int itr);

  void Decode_any_MT(UInt_t iread, UInt_t ibuf);

  void Decode_any(UInt_t iread, UInt_t ibuf);
  // FindLast* находит конец текущего буфера b_end[ibuf],
  // что является одновременно началом следующего b_start[ibuf2]
  void FindLast(UInt_t ibuf);

  // void FindLast75(UInt_t ibuf);
  // void FindLast76(UInt_t ibuf);
  // void FindLast33(UInt_t ibuf);
  // void FindLast2(UInt_t ibuf);
  // void FindLast_adcm(UInt_t ibuf);
  void PulseAna(PulseClass &ipls);
  void Decode78(UInt_t iread, UInt_t ibuf);
  void Decode77(UInt_t iread, UInt_t ibuf);
  void Decode76(UInt_t iread, UInt_t ibuf);
  void Decode75(UInt_t iread, UInt_t ibuf);
  void Decode33(UInt_t iread, UInt_t ibuf);
  void Decode34(UInt_t iread, UInt_t ibuf);
  void Decode2(UInt_t iread, UInt_t ibuf);
  void Decode_adcm(UInt_t iread, UInt_t ibuf);

  //void Decode32(UChar_t* buffer, int length);
  //void Decode33(UChar_t* buffer, int length, int ivp, int ivp2);
  //void Decode_adcm(UChar_t* buffer, int length);

  int Searchsync(int &idx, UInt_t* buf4, int end);

  //int Set_Trigger();
  void Ana_start();
  void Ana2(int all);

  //void PrintPulse(int udata, bool pdata=false);

  //void PEvent() { if (b_pevent) Emit("PEvent()"); } //*SIGNAL*
  //void SigEvent() { Emit("SigEvent()"); } //*SIGNAL*

  //void Event_Insert_Pulse(PulseClass* pulse);
  //void Event_Insert_Pulse(pulse_vect::iterator pls);
  void Event_Insert_Pulse(eventlist *Elist, PulseClass* pls);
  void Make_Events(std::list<eventlist>::iterator BB);
  void Select_Event(EventClass *evt);
  //void *Ana_Events(void* ptr);

  //void NewTree();
  //void CloseTree();
  void Reset_Dec(Short_t mod);
  // void Fill_Dec73(EventClass* evt);
  // void Fill_Dec74(EventClass* evt);
  void Fill_Dec75(EventClass* evt);
  void Fill_Dec76(EventClass* evt);
  void Fill_Dec77(EventClass* evt);
  void Fill_Dec78(EventClass* evt);
  void Flush_Dec();

  //void Print_Pulses();
  void Print_Events(const char* file=0);
  void Print_b1(int idx1, std::ostream *out);
  void Print_Buf(UInt_t ibuf, const char* file=0);
  ClassDef(CRS, 0)
};

#endif
