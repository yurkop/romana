#ifndef pulseclass_H
#define pulseclass_H 1

#include <TROOT.h>
#include <list>

//#include <TH1.h>

//class HMap;
//class Mdef;

// class BufClass {
//  public:
//   char* buffer;
//   int bufsize;
// };

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

class PkClass {
public:
  Long64_t QX;
  Int_t RX,C,A,AY,CF1,CF2;
  Short_t H;
  UChar_t E;
};

class PulseClass {

public:

  Long64_t Tstamp64=0; //64-bit timestamp (corrected for overflows)
  Long64_t Counter=0; //pulse counter
  std::vector<Float_t> sData; //(maybe smoothed) pulse data

  UChar_t Chan=255; //channel number
  Short_t Pos=-32222; //pos of the trigger relative to pulse start (in samples)
               //Pos=-32222 -> default Pos - значит, пик не найден
  UChar_t Spin=0;
  //bit 0: channel state word (Control word - external input in crs32)
  //bit 2 (Spin|=4): было переполнение канального буфера (ER_OVF)
  //bit 6 (Spin|=64): event is writable in Dec (Ms - master channel)
  //bit 7 (=128): hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents
  UChar_t ptype=0; //pulse type: 0 - good pulse; (see P_* constants)

  Float_t Area=0; // pure area (Area0-Base)
  Float_t Base=0; // baseline
  //Float_t Area0; // area+baseline
  Float_t Sl1=0; // slope of background
  Float_t Sl2=0; // slope of peak
  Float_t RMS1=0; // noise of background
  Float_t RMS2=0; // noise of peak
  Float_t Height=0; // maximum of pulse in the same region as Area
  Float_t Width=0; // peak width
  Float_t Time=99999; // exact time relative to Pos (pulse start) in samples
  //Float_t Time2=-999999; // = Time - T0, в наносекундах
  Float_t Rtime=0; // RiseTime

  //Float_t Simul2; //another version of Time (for simulions)
  //Float_t Noise1;
  //Float_t Noise2;

  //bool Analyzed; //true if pulse is already analyzed
#ifdef APK
  PkClass ppk;
#endif

public:
  PulseClass() {Pos=-32222;};
  PulseClass(Short_t p) {Pos=p;};
  virtual ~PulseClass() {};

  size_t GetPtr(Int_t hnum);
  Float_t CFD(int i, int kk, int delay, Float_t frac, Float_t &drv);
  //void Analyze();
  Short_t FindPeaks(Int_t sTrig, Int_t kk, Float_t &cfd_frac);
  void FindZero(Int_t kk, Int_t stg, Int_t thresh, Float_t LT);
  //void FindPeaks(Float_t thresh, int deadtime);
  //void PeakAna();
  void PeakAna33(bool onlyT=false);
  //void CheckDSP();
  void Ecalibr(Float_t& XX);
  //void Bcalibr();
  void Smooth(int n);
  void Smooth_hw(int n);
  //void Smooth_old(int n);
  void PoleZero(int tau);
  void PrintPulse(int pdata=0);

  //ClassDef(PulseClass, 0)
};

typedef std::vector<PulseClass> pulse_vect;
//typedef pulse_vect::iterator pulse_iter;

class EventClass { //event of pulses

public:

  Long64_t Nevt=0;
  Long64_t Tstmp; //Event Timestamp (the earliest pulse threshold crossig)
  Float_t T0=99999; //time of the earliest *START* peak, relative to Tstmp, in samples
  UChar_t ChT0=255;// канал, в котором старт для Time
  UChar_t Spin=0;
  //bit 0: channel state word (Control word - external input in crs32)
  //bit 2 (=4): ER_OVF - присутствует только в каналах
  //bit 6 (Spin|=64): event is writable in Dec (Ms - master channel)
  //bit 7 (=128): hardware counters
  //Spin>=254: сигнализирует, что текущий кусок декодера завершился
  //Spin=255 - end of Blist, merge BB and Levents in Make_Events
  //Spin=254 - end of Blist, just splice BB and Levents

  //Long64_t Tstart0=0; //Timestamp of the start event
  std::vector <PulseClass> pulses;
  //std::vector <Long64_t> *Counters;
  //Bool_t Analyzed;

  // private:
  //  void Fill1dw(Bool_t first, HMap* map[], int ch, Float_t x, Double_t w=1);
  //  void Fill01dw(HMap* map[], int ch, Float_t x, Double_t w=1);

public:
  //EventClass();
  //virtual ~EventClass() {};

  void AddPulse(PulseClass *pls);
  //void FillHist(Double_t *hcut_flag);
  void PrintEvent(bool pls=1);
  void Fill_Dec(char* buf);
  //ClassDef(EventClass, 0)
};

typedef std::list<EventClass> eventlist;

#endif
