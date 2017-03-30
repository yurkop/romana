#ifndef common_H
#define common_H 1

#define ADC2
//#define ADCM

#ifdef ADC2
const int dstep = 5;
#endif
#ifdef ADCM
const int dstep = 10;
#endif

#include <zlib.h>
#include <TROOT.h>
#include <deque>

#include "pulseclass.h"

//#define AVM16_MASK1 0x7ff00000  //to read new type of data
#define AVM16_MASK1 0x00000000  //to read old type of data. Works also with new.
#define AVM16_MASK2 0x000fffff //complementary to MASK1

const int MAX_CH=32; //max number of channels
const int ADDCH=9; //nai, bgo, si1, si2, stlb, demon, hpge, NIM, other

#define MAX_R 4 //number of channel types (gam,ng,nim,off)
#define MAX_L 6 //line types (gam,neu,tail,unkn,pileup,frame)
#define MAX_P 5 //particle types (gam,neu,tail,unkn,pileup)

enum ChannelDef {
  ch_off2,
  ch_nim,
  ch_gam,
  ch_ng
};

enum ChDef {
  ch_nai,
  ch_bgo,
  ch_si1,
  ch_si2,
  ch_stilb,
  ch_demon,
  ch_hpge,
  ch_other,
  ch_empty
};

//#define BSIZE 131072 //1024*128
//#define BOFFSET 131072 //1024*128
#define DSIZE 131072
#define MAXRING 10

#define MXLN 40
#define MXNUM 60


enum MENU_COM {
  M_READINIT,
  M_SAVEINIT,
  M_READROOT,
  M_SAVEROOT,
  M_FILE_BROWSE,
  M_FILE_NEWCANVAS,
  //M_FILE_OPEN,
  //M_FILE_SAVE,
  M_FILE_EXIT,
  //M_THRESH,
  //M_PARAM,
  M_DEMON,
  M_SYNC,
  //M_MKMON,
  //M_MAKAN,
  M_HELP,
  M_TEST,
  //M_SI,
  //M_SIMAX,
  M_E0_7,
  M_E8_15,
  M_E16_23,
  M_E24_31,
  M_T0_7,
  M_T8_15,
  M_T16_23,
  M_T24_31,
  M_TOF0_7,
  M_TOF8_15,
  M_TOF16_23,
  M_TOF24_31,
  /*
  M_1_6,
  M_7_12,
  M_13_18,
  M_19_24,
  M_25_30,
  M_27_32,
  M_T1_6,
  M_T7_12,
  M_T13_18,
  M_T19_24,
  M_T25_30,
  M_T27_32,
  */
  //M_CHANNELS,

  //#ifdef ROMASH
  M_TOF0_5,
  M_TOF6_11,
  M_TOF12_17,
  M_TOF18_23
  //#endif
};

//-----------------------------------------------
struct chan_type {

  ChannelDef Type; //channel type
  char name[16];
  Float_t thresh; //threshold
  Float_t elim1,elim2; // E limits
  bool common; //if true - use common parameters for the given channel type
  short col; //color
  short nsmoo; //smoothing parameter
  short pk1,pk2; //peak limits
  short bg1,bg2; //background limits
  short pipeup; //pileup window
  short dt; //dead time
  short tm,tw; //timing method; timing width

};

/*
//-----------------------------------------------
class PeakClass {

 public:
  PeakDef Type; //peak type
  int Pos; //peak position with respect to the parent pulse timestamp
  double Area;
  double Height;
  double Width;
  double Time; //exact time with respect to the parent pulse timestamp

 public:
  PeakClass() {Type=p_undet;};// {};
  virtual ~PeakClass();

  //ClassDef(PeakClass, 0)
};
*/

//-----------------------------------------------
// Event consists of up to MAX_CH pulses, coming in coincidence
class EventClass { 

 public:
  int nEpulses; //number of pulses in the Event
  Long64_t Tstamp64; //timestamp of the first pulse in this event
  //PulseClass *Epulses[MAX_CH];
  std::vector<PulseClass *> Epulses;
 public:
  EventClass();
  virtual ~EventClass();
  void Clear();
  //EventClass *Copy(EventClass);
  //void AddPulse(PulseClass *pulse);
  //PulseClass* AddPulse(int ch); //returns the address of the new pulse
  //ClassDef(EventClass, 0)
};

//-----------------------------------------------
// Buffer contains pulses (actually events)
// Each pulse is identified by its timestamp
// Pulses #can_be# (are?) grouped into Events
// The criterium: timestamps lie within some window
// Pulses of the same channel can be duplicated 
// (or have some overlapping regions)
// Overlapping pulses in the same channel are merged into one larger pulse
class BufClass {

 public:
  gzFile gzF;
  int r_buf; //actual binary buffer size
  unsigned short *uBuf;
  unsigned short *uBuf2;

  //unsigned short *buffer;
  bool EoF; //end of file;
  int nEbuf; //current event buffer number
  int nev; //number of events in the buffer
  //PulseClass *Bpulses;
  PulseClass *iPulse; //current pulse
  EventClass *Bevents;

  std::deque<PulseClass *> Bpulses;

  Long64_t istamp64; //current tstamp64
 public:
  BufClass();//(int size);
  virtual ~BufClass();
  //PulseClass* AddPulse(int ch); //returns the address of the new pulse
  //void ChangeSize(int size);
  void Do1buf();
  //void DoNbuf(int nn);
  void AddFirstPulse();
  void AddPulse();
  //void AddEvent();
  //void ReadHeader();
  //void LoadBuf_adc2();
  void LoadBuf_crs();
  //int ReadPulse();
  int NewFile();
  int ReadBuf();
  //ClassDef(EventClass, 0)
};

#endif
