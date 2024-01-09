#ifndef pulseclass_H
#define pulseclass_H 1

#include <TROOT.h>
#include <TH1.h>

class HMap;
class Mdef;

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
  //bit 7 (=128): hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents
  UChar_t ptype; //pulse type: 0 - good pulse; (see P_* constants)

  Float_t Area; // pure area (Area0-Base)
  Float_t Base; // baseline
  //Float_t Area0; // area+baseline
  Float_t Sl1; // slope of background
  Float_t Sl2; // slope of peak
  Float_t Height; // maximum of pulse in the same region as Area
  Float_t Width; // peak width
  Float_t Time; // exact time relative to Pos (pulse start)

  //Float_t Simul2; //another version of Time (for simulions)
  //Float_t Noise1;
  //Float_t Noise2;

  //bool Analyzed; //true if pulse is already analyzed
 public:
  PulseClass();// {};
  virtual ~PulseClass() {};

  size_t GetPtr(Mdef* it);
  Float_t CFD(int i, int kk, int delay, Float_t frac, Float_t &drv);
  //void Analyze();
  void FindPeaks(Int_t sTrig, Int_t kk);
  void FindZero(Int_t sTrig, Int_t kk, Int_t j0);
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
//typedef pulse_vect::iterator pulse_iter;

class EventClass { //event of pulses

 public:
  Long64_t Nevt;
  UChar_t Spin;
  //bit 0: channel state word (Control word - external input in crs32)
  //bit 1 (Spin|=2): event is writable in Dec (Ms - master channel)
  //bit 7 (=128): hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents
  Long64_t Tstmp; //Event Timestamp (the earliest pulse threshold crossig)
  //Long64_t Tstart0=0; //Timestamp of the start event
  Float_t T0; //time of the earliest *START* peak, relative to Tstmp, in samples
  std::vector <PulseClass> pulses;
  //std::vector <Long64_t> *Counters;
  //Bool_t Analyzed;
  //Bool_t ecut[MAXCUTS];

 private:
  void Fill1dw(Bool_t first, HMap* map[], int ch, Float_t x, Double_t w=1);
  //void Fill01dw(HMap* map[], int ch, Double_t hcut_flag[], Float_t x, Double_t w=1);
  void Fill01dw(HMap* map[], int ch, Float_t x, Double_t w=1);

public:
  EventClass();
  virtual ~EventClass() {};
  //void Make_Mean_Event();
  //void Pulse_Mean_Add(PulseClass *newpulse);

  //void Pulse_Ana_Add(pulse_vect::iterator pls);
  void AddPulse(PulseClass *pls);




  /*
  void Fill_Time_Extend(HMap* map, void* hd);
  void Fill1d(Bool_t first, HMap* map[], int ch, Float_t x);
  //void Fill01d(HMap* map[], int ch, Double_t hcut_flag[], Float_t x);
  void Fill01d(HMap* map[], int ch, Float_t x);
  //void Fill10(HMap* map[], int ch, Float_t x, Double_t *hcut_flag);
  //void Fill11(Mdef* md, EventClass* evt, pulse_vect::iterator ipls, Double_t *hcut_flag);
  static void Fill1dwSt(Bool_t first, HMap* map[], int ch, Float_t x, Double_t w);
  void Fill_Mean_Pulse(Bool_t first, HMap* map,
		       pulse_vect::iterator pls, int ideriv);
  void Fill_Mean1(TH1F* hh,  Float_t* Data, Int_t nbins, int ideriv);
  //void Fill_Mean1(TH1F* hh,  PulseClass* pls, UInt_t nbins, int ideriv);
  void Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y);
  // void FillFirst();
  // void FillSecond();
  */

  void FillHist();
  //YK_OLD void FillHist_old(Bool_t first);
  void PrintEvent(bool pls=1);
  //void PeakAna();
  //ClassDef(EventClass, 0)
};

#endif
