#ifndef pulseclass_H
#define pulseclass_H 1

#include <TROOT.h>


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
const UShort_t P_B1=1<<2; //bad left side (preceeds opt.peak1)
const UShort_t P_B2=1<<3; //bad right side (exceeds opt.peak2)
//const UShort_t P_B11=1<<4; //bad left side is ignored
const UShort_t P_B22=1<<5; //bad right side (no right zero-crossing T1)
const UShort_t P_B111=1<<6; //bad left size in timing



const UShort_t P_GAM=1<<9; //gamma
const UShort_t P_NEU=1<<10; //neutron
const UShort_t P_TAIL=1<<11; //tail



class peak_type {
 public:
  Float_t Area;
  Float_t Height;
  Float_t Width;
  Float_t Time; //exact time relative to pulse start (from 1st deriv)
  Float_t Time2; //exact time (from 2nd deriv)
  Short_t Pos; //position relative to pulse start (in samples)
  Short_t T1; //left zero crossing of deriv
  Short_t T2; //right zero crossing of deriv
  
  UShort_t Type; //peak type
  //Char_t ch;
 public:
  peak_type() {Type=0;};
  virtual ~peak_type() {};
  
};

//ptype==0 - > pulse had start and stop
const unsigned char P_NOSTART=1; //pulse has no start
const unsigned char P_NOSTOP=2; // pulse has no stop
const unsigned char P_BADCH=4;
const unsigned char P_BADSZ=8;
const unsigned char P_BADCNT=16;
//-----------------------------------------------

// Pulse is a signal registered in physical channel Chan
// It may (or may not) contain raw data (oscillogram)
// It may (or may not) contain peaks

class PulseClass2 {

 public:
  Long64_t Tstamp64; //64-bit timestamp (corrected for overflows)
  Long64_t Counter; //pulse counter
  //int Nsamp; //actual number of samples in the pulse
  std::vector <Float_t> sData; //(maybe smoothed) pulse data

  std::vector <peak_type> Peaks;
  //int Npeaks; // number of peaks found in the pulse
  //peak_type *Peaks;

  int tdif; //difference in tstamp from the event tstamp

  UChar_t Chan; //channel number
  UChar_t Control; //Control word
  UChar_t ptype; //pulse type: 0 - good pulse; (see P_* constants)
  //short *Data; //raw pulse data

  bool Analyzed; //true if pulse is already analyzed
 public:
  PulseClass2();// {};
  virtual ~PulseClass2() {};

  void Analyze();
  void FindPeaks();
  //void FindPeaks(Float_t thresh, int deadtime);
  void PeakAna();
  void Smooth(int n);
  void PrintPulse();

  //ClassDef(PulseClass, 0)
};

class EventClass1 { //event of pulses

 public:
  Long64_t T; //Timestamp
  Float_t T0; //time of the earliest start peak
  std::vector <PulseClass2> pulses;
  
 public:
  EventClass1();
  virtual ~EventClass1() {};
  void Pulse_Ana_Add(PulseClass2 *newpulse);
  //void PeakAna();
  //ClassDef(EventClass1, 0)
};

/*
class EventClass2 { //event of peaks

 public:
  Long64_t T; //Timestamp
  Float_t T0; //time of the earliest start peak
  std::vector <PulseClass2> pulses;
  
 public:
  EventClass2();
  virtual ~EventClass2() {};
  void Pulse_Ana_Add(PulseClass2 *newpulse);
  //void PeakAna();
  //ClassDef(EventClass1, 0)
};
*/

class PulseClass {
  // Pulse is a signal registered in physical channel Chan
  // It may (or may not) contain raw data (oscillogram)
  // It may (or may not) contain peaks

 public:
  Long64_t Tstamp64; //64-bit timestamp (corrected for overflows)
  Long64_t Counter; //pulse counter
  int Nsamp; //actual number of samples in the pulse
  Float_t *sData; //smoothed pulse data

  int Npeaks; // number of peaks found in the pulse
  peak_type *Peaks;

  int tdif; //difference in tstamp from the event tstamp

  UChar_t Chan; //channel number
  UChar_t Control; //Control word
  UChar_t ptype; //pulse type: 0 - good pulse; (see 
  //short *Data; //raw pulse data

  bool Analyzed; //true if pulse is already analyzed
 public:
  PulseClass(ULong64_t size);// {};
  PulseClass();// {};
  virtual ~PulseClass();

  void Analyze();
  void FindPeaks(Float_t thresh, int deadtime);
  void PeakAna();
  void Smooth(int n);

  //ClassDef(PulseClass, 0)
};

#endif
