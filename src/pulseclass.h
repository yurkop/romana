#ifndef pulseclass_H
#define pulseclass_H 1

#include <TROOT.h>
#include <TH1.h>

class HMap;

/*
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
*/

//peak Type:
const UChar_t P_PILE1=1<<0; //fisrt puleup pulse
const UChar_t P_PILE2=1<<1; //subsequent puleup pulse
const UChar_t P_B1=1<<2; //bad left side (opt.peak1 is out of range)
const UChar_t P_B2=1<<3; //bad right side (opt.peak2 is out of range)
const UChar_t P_B11=1<<4; //bad left or right side in timing (T3 or T4 is out of range)
const UChar_t P_B22=1<<5; //bad left or right side in width (T5 or T6 is out of range)
const UChar_t P_BAD=1<<7; //bad (dummy) peak


/*
class PeakClass {
public:
  Float_t Base; //baseline
  Float_t Area0; //area+background
  Float_t Area; //pure area (Area0-Base)
  Float_t Slope1;
  Float_t Slope2;
  //Float_t Noise1;
  //Float_t Noise2;
  Float_t Height; //maximum of pulse in the same region as Area
  Float_t Width; //peak width - Alpatov (in 1st deriv)
  //Float_t Width2; //peak width2 - romana3a
  //Float_t Width3; //peak width3 - Alpatov2 (in pulse)
  Float_t Time; //exact time relative to pulse start (from 1st deriv), also relative to event start, in samples
  //Float_t Time2; //exact time (from 2nd deriv)

  UChar_t Type; //peak type
  //UChar_t Chan; //channel number
  //Short_t Pos; //position relative to pulse start (in samples)
  //Short_t Pos2; //position of the 1st maximum in 1st derivative after threshold

  // Short_t B1; //left background window
  // Short_t B2; //right background window
  // Short_t P1; //left peak window
  // Short_t P2; //right peak window
  // Short_t T1; //left zero crossing of deriv
  // Short_t T2; //right zero crossing of deriv
  // Short_t T3; //left timing window
  // Short_t T4; //right timing window
  // Short_t T5; //left width window
  // Short_t T6; //right width window

  //Pos,T1,T2 - relative to pulse start, in samples
  //Time - relative to discriminator (+preWr), in samples

public:
  PeakClass();// {Type=0;};
//   virtual ~PeakClass() {};
  
};
*/

//ptype==0 - > pulse had start and stop
//const unsigned char P_NOSTART=1; //pulse has no start
const unsigned char P_NOLAST=2; // pulse has no last fragment
const unsigned char P_BADCH=4;
const unsigned char P_BADTST=8;
const unsigned char P_BADSZ=16;
const unsigned char P_BADCNT=32;
const unsigned char P_STARTCH=64;
const unsigned char P_BADPEAK=128;
//-----------------------------------------------

// Pulse is a signal registered in physical channel Chan
// It may (or may not) contain raw data (oscillogram)
// It may (or may not) contain peaks

class PulseClass {

 public:
  Long64_t Tstamp64; //64-bit timestamp (corrected for overflows)
  Long64_t Counter; //pulse counter
  std::vector <Float_t> sData; //(maybe smoothed) pulse data

  //PeakClass Peak;
  // std::vector <PeakClass> Peaks;

  UChar_t Chan; //channel number
  Short_t Pos; //position of the trigger relative to pulse start (in samples)
               //Pos=-32222 -> default Pos - значит, пик не найден
  UChar_t Spin;
  //bit 0: channel state word (Control word - external input in crs32)
  //bit 1 (Spin|=2): event is writable in Dec (Ms - master channel)
  //bit 2 (Spin|=4): было переполнение канального буфера (ER_OVF)
  //bit 7: hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents
  UChar_t ptype; //pulse type: 0 - good pulse; (see P_* constants)

  Float_t Base; //baseline
  Float_t Area0; //area+background
  Float_t Area; //pure area (Area0-Base)
  Float_t Slope1; //slope of background
  //Float_t Slope2; //slope of peak
  Float_t Simul; //slope of peak


  //Float_t Noise1;
  //Float_t Noise2;
  Float_t Height; //maximum of pulse in the same region as Area
  Float_t Width; //peak width
  Float_t Time; //exact time relative to pulse start (from 1st deriv)

  //bool Analyzed; //true if pulse is already analyzed
 public:
  PulseClass();// {};
  virtual ~PulseClass() {};

  void Analyze();
  void FindPeaks();
  //void FindPeaks(Float_t thresh, int deadtime);
  //void PeakAna();
  void PeakAna33();
  void CheckDSP();
  void Ecalibr();
  void Smooth(int n);
  void PrintPulse(int pdata=0);

  //ClassDef(PulseClass, 0)
};

typedef std::vector<PulseClass> pulse_vect;

class EventClass { //event of pulses

 public:
  Long64_t Nevt;
  UChar_t Spin;
  //bit 0: channel state word (Control word - external input in crs32)
  //bit 1 (Spin|=2): event is writable in Dec (Ms - master channel)
  //bit 7: hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents
  Long64_t Tstmp; //Timestamp of the earliest pulse (threshold crossig)
  Float_t T0; //time of the earliest *START* peak, relative to Tstmp, in samples
  std::vector <PulseClass> pulses;
  //std::vector <Long64_t> *Counters;
  //Bool_t Analyzed;
  //Bool_t ecut[MAXCUTS];

 private:
  void Fill1dw(Bool_t first, HMap* map[], int ch, Float_t x, Double_t w=1);

public:
  EventClass();
  virtual ~EventClass() {};
  //void Make_Mean_Event();
  //void Pulse_Mean_Add(PulseClass *newpulse);

  //void Pulse_Ana_Add(pulse_vect::iterator pls);
  void AddPulse(PulseClass *pls);
  void Fill_Time_Extend(HMap* map, void* hd);
  void Fill1d(Bool_t first, HMap* map[], int ch, Float_t x);

  static void Fill1dwSt(Bool_t first, HMap* map[], int ch, Float_t x, Double_t w);
  void Fill_Mean_Pulse(Bool_t first, HMap* map,
		       pulse_vect::iterator pls, int ideriv);
  void Fill_Mean1(TH1F* hh,  Float_t* Data, Int_t nbins, int ideriv);
  //void Fill_Mean1(TH1F* hh,  PulseClass* pls, UInt_t nbins, int ideriv);
  void Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y);
  void FillHist(Bool_t first);
  void FillHist_old();
  void PrintEvent(bool pls=1);
  //void PeakAna();
  //ClassDef(EventClass, 0)
};

#endif
