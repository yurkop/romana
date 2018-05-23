#ifndef pulseclass_H
#define pulseclass_H 1

#include <TROOT.h>
#include <TH1.h>

class HMap;

enum PeakDef {
  p_undet,
  p_gam,
  p_neu,
  p_tail,
  p_unknown,
  p_nim,
  p_badl,
  p_badr,
  p_pileup,
  p_f_pileup
};

const UShort_t P_PILE1=1<<0; //fisrt puleup pulse
const UShort_t P_PILE2=1<<1; //subsequent puleup pulse
const UShort_t P_B1=1<<2; //bad left side (opt.peak1 is out of range)
const UShort_t P_B2=1<<3; //bad right side (opt.peak2 is out of range)
const UShort_t P_B11=1<<4; //bad left or right side in timing (T3 is out of range)
//const UShort_t P_B22=1<<5; //bad right side in timing (T4 is out of range)
//const UShort_t P_B111=1<<6; //bad left size in timing



const UShort_t P_GAM=1<<9; //gamma
const UShort_t P_NEU=1<<10; //neutron
const UShort_t P_TAIL=1<<11; //tail



class peak_type {
 public:
  Float_t Base;
  Float_t Area0;
  Float_t Area;
  Float_t Slope1;
  Float_t Slope2;
  Float_t Noise1;
  Float_t Noise2;
  Float_t Height;
  Float_t Width;
  Float_t Time; //exact time relative to pulse start (from 1st deriv)
  //Float_t Time2; //exact time (from 2nd deriv)
  Short_t Pos; //position relative to pulse start (in samples)
  Short_t Pos2; //position of the 1st maximum in 1st derivative after threshold
  Short_t B1; //left background window
  Short_t B2; //right background window
  Short_t P1; //left peak window
  Short_t P2; //right peak window
  Short_t T1; //left zero crossing of deriv
  Short_t T2; //right zero crossing of deriv
  Short_t T3; //left timing window
  Short_t T4; //right timing window

  //Pos,T1,T2 - relative to pulse start, in samples
  //Time - relative to discriminator (+preWr), in samples

  UShort_t Type; //peak type
  //Char_t ch;
 public:
  peak_type() {Type=0;};
  virtual ~peak_type() {};
  
};

#pragma pack (push, 1)
struct rpeak_type {
  Float_t Area;
  //Float_t Height;
  Float_t Width;
  Float_t Time; //exact time relative to pulse start (from 1st deriv)
  UChar_t Ch; //Channel number
  UChar_t Type; //peak type
};
#pragma pack (pop)

//ptype==0 - > pulse had start and stop
const unsigned char P_NOSTART=1; //pulse has no start
const unsigned char P_NOSTOP=2; // pulse has no stop
const unsigned char P_BADCH=4;
const unsigned char P_BADTST=8;
const unsigned char P_BADSZ=16;
const unsigned char P_BADCNT=32;
//-----------------------------------------------

// Pulse is a signal registered in physical channel Chan
// It may (or may not) contain raw data (oscillogram)
// It may (or may not) contain peaks

class PulseClass {

 public:
  Long64_t Tstamp64; //64-bit timestamp (corrected for overflows)
  Long64_t Counter; //pulse counter
  //int Nsamp; //actual number of samples in the pulse
  std::vector <Float_t> sData; //(maybe smoothed) pulse data

  std::vector <peak_type> Peaks;
  //int Npeaks; // number of peaks found in the pulse
  //peak_type *Peaks;

  //int tdif; //difference in tstamp from the event tstamp

  UChar_t Chan; //channel number
  UChar_t State; //channel state word (Control word - external input in crs32)
  UChar_t ptype; //pulse type: 0 - good pulse; (see P_* constants)
  //short *Data; //raw pulse data

  //bool Analyzed; //true if pulse is already analyzed
 public:
  PulseClass();// {};
  virtual ~PulseClass() {};

  void Analyze();
  void FindPeaks();
  //void FindPeaks(Float_t thresh, int deadtime);
  void PeakAna();
  void Smooth(int n);
  void PrintPulse(int pdata=0);

  //ClassDef(PulseClass, 0)
};

typedef std::vector<PulseClass> pulse_vect;

class EventClass { //event of pulses

 public:
  Long64_t Nevt;
  Char_t State; //state word from CRS module
  Long64_t T; //Timestamp of the earliest pulse (threshold crossig)
  Float_t T0; //time of the earliest *START* peak, relative to T
  std::vector <PulseClass> pulses;
  //Bool_t Analyzed;
  //Bool_t ecut[MAXCUTS];

 public:
  EventClass();
  virtual ~EventClass() {};
  //void Make_Mean_Event();
  //void Pulse_Mean_Add(PulseClass *newpulse);
  void Pulse_Ana_Add(pulse_vect::iterator pls);
  void Fill_Time_Extend(HMap* map);
  void Fill1d(Bool_t first, HMap* map[], int ch, Float_t x);
  void Fill_Mean_Pulse(Bool_t first, HMap* map,  PulseClass* pls);
  void Fill_Mean1(TH1F* hh,  PulseClass* pls, UInt_t nbins);
  void Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y);
  void FillHist(Bool_t first);
  void FillHist_old();
  //void PeakAna();
  //ClassDef(EventClass, 0)
};

#endif
