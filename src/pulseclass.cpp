#include "common.h"
#include "toptions.h"
#include "libcrs.h"
#include "hclass.h"
#include "libmana.h"
#include <iostream>
#include <math.h>

#include "TSystem.h"
#include "TRandom.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


//extern Coptions cpar;
extern Toptions opt;
extern CRS* crs;
extern HClass* hcl;
extern MyMainFrame *myM;
//extern HistFrame* HiFrm;
extern Coptions cpar;

const double mks=0.001;

//Double_t initcuts[MAXCUTS];

using namespace std;

PeakClass::PeakClass() {
  Type=0;
  //Area=0;
  //Width=0;
  //B1=0;
}

PulseClass::PulseClass() {
  //ptype=P_NOSTOP;
  ptype=0;
  //Nsamp=0;
  //Npeaks=0;
  //Analyzed=false;
  Tstamp64=0;
  Spin=0;
  //tdif=99;
  //sData=NULL;
  //Peaks=NULL;
}

void PulseClass::FindPeaks() {
  //Находим только первый пик

  //sTg: 0 - hreshold crossing of pulse;
  //      1 - threshold crossing of derivative;
  //      2 - maximum of derivative;
  //      3 - rise of derivative;
  //      4 - fall of derivative;
  //      5 - threshold crossing of derivative, use 2nd deriv for timing.

  if (sData.size()<2)
    return;

  UInt_t kk=opt.sDrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  PeakClass pk=PeakClass();
  //PeakClass *p_prev=0;

  //bool in_peak=false;
  Float_t* D = new Float_t[sData.size()]();
  Float_t Dpr;
  //Float_t jmax=0;

  D[0]=0;
  UInt_t j;
  UInt_t jj=0;

  switch (opt.sTg[Chan]) {
  case 0: // hreshold crossing of pulse
    for (j=0;j<sData.size();j++) {
      if (sData[j] >= opt.sThr[Chan]) {
	Pos=j;
	Peaks.push_back(pk);
	break;
      }
    }
    break;
  case 1: // threshold crossing of derivative;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] >= opt.sThr[Chan]) {
	Pos=j;
	Peaks.push_back(pk);
	break;
      }
    }
    break;
  case 2: // maximum of derivative;
  case 5: // or fall of 2nd derivative (it's the same)
    Dpr=-1e6;
    //int jpr;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (Dpr >= opt.sThr[Chan] && D[j]<Dpr) {
	Pos=j-1;
	Peaks.push_back(pk);
	break;
      }
      Dpr=D[j];
    }
    break;
  case 3: // rise of derivative;
    Dpr=1;
    //int jj=0;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] > 0 && Dpr<=0) {
	jj=j;
      }
      if (D[j] >= opt.sThr[Chan]) {
	Pos=jj;
	Peaks.push_back(pk);
	break;
      }
      // else {
      //    break;
      // }
      Dpr=D[j];
    }
    break;
  case 4: // fall of derivative;
    Dpr=1;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] <= 0 && jj) {
	Pos=j-1;
	Peaks.push_back(pk);
	break;
      }
      if (D[j] >= opt.sThr[Chan]) {
	jj=1;
      }
      // else {
      //    break;
      // }
      //Dpr=D[j];
    }
    break;
  default:
    break;
  }

  //printf("FindPeaks: %d\n",Npeaks);

  delete[] D;

} //FindPeaks



/*
  void PulseClass::FindPeaks() {
  // always K-th derivative (D) is used
  // T1 - D>0, D(-1)<=0 (left zero-crossing of D)
  // T2 - D<=0, d(-1)>0 (right zero-crossing of D)
  // Pos - threshold crossing, later Pos is redefined as Time+0.5
  //(from exact time)
  // Pos2 - Maximum of D between T1 and T2



  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak


  if (sData.size()<2)
  return;

  UInt_t kk=opt.sDrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  PeakClass *pk=0;
  PeakClass *p_prev=0;

  bool in_peak=false;
  //Float_t D,pD=0;//deriv, prev.dreiv
  Float_t* D = new Float_t[sData.size()]();
  //int peakpos[DSIZE];
  Float_t jmax=0;
  //int pp0=0; //temporary T1

  D[0]=0;
  UInt_t j;
  for (j=kk;j<sData.size();j++) {
  D[j]=sData[j]-sData[j-kk];
  if (!in_peak) {
  if (D[j] >= opt.sThr[Chan]) {
  in_peak=true;
  Peaks.push_back(PeakClass());
  pk = &Peaks.back();
  pk->T1=0;
  for (int n=j;n>0;n--) {
  if (D[n]>0 && D[n-1]<=0) {
  pk->T1=n;
  break;
  }
  }
  jmax=D[j];
  pk->Pos=j;
  pk->Pos2=j;
  }
  }
  else { //in_peak
  if (D[j]>jmax) {//maximum of D -> peak position
  jmax=D[j];
  pk->Pos2=j;
  //printf("in_peak: %lld %d %d %d %0.1f %0.1f\n",crs->nevents,Chan,j,kk,D[j],jmax);
  // peakpos[Npeaks]=jmax=j-1;
  // Npeaks++;
  // if (deadtime) {
  //   j+=deadtime;
  //   in_peak=false;
  //   continue;
  // }
  }
  else if (D[j]<=0) { //zero crossing -> end of the peak
  in_peak=false;
  pk->T2=j-1;
  //cout << "T2: " << sData[pk->T2]-sData[pk->T2-kk] << endl;
  //pk->Height=jmax;
  jmax=0;
  if (Peaks.size()>1) { //this is at least second peak
  p_prev = pk-1;
  if (pk->Pos - p_prev->Pos < opt.dT[Chan])
  Peaks.pop_back();
  else if (pk->Pos - p_prev->Pos < opt.Pile[Chan]) {
  p_prev->Type|=P_PILE1;
  pk->Type|=P_PILE2;
  }
  }
  }
  }
  //pD=D;
  }

  if (in_peak) { //end of the pulse -> peak has no end
  pk->T2=sData.size();
  //pk->Type|=P_B22;
  }
  
  //for (UInt_t i=0;i<Peaks.size();i++) {
  //cout << "FindPeaks: " << crs->nevents << " " << i << " " 
  //   << Peaks.at(i).Pos << endl;
  //}

  // Peaks = new PeakClass[Npeaks];
  // for (int i=0;i<Npeaks;i++) {
  //   Peaks[i].Pos = peakpos[i];
  // }

  //printf("FindPeaks: %d\n",Npeaks);

  delete[] D;

  }
*/
//-----------------------------

//-----------------------------

void PulseClass::PeakAna33() {

  Short_t B1; //left background window
  Short_t B2; //right background window
  Short_t P1; //left peak window
  Short_t P2; //right peak window
  //Short_t T1; //left zero crossing of deriv
  //Short_t T2; //right zero crossing of deriv
  Short_t T1; //left timing window
  Short_t T2; //right timing window
  Short_t W1; //left width window
  Short_t W2; //right width window

  // if (Chan==7) {
  //   cout << "Peakana33: " << (int) Chan << " " << Peaks.size() << " " << sData.size() << endl;
  // }

  PeakClass *pk;

  int sz=sData.size();
  Int_t kk=opt.sDrv[Chan];
  if (kk<1 || kk>=sz-1) kk=1;

  if (sData.size()<=2) {
    //   Peaks.push_back(PeakClass());
    //   Peaks.back().Pos=cpar.Pre[Chan];
    //   Peaks.back().Time=0;
    return;
  }

  //return;
  //Short_t T5; //left width window
  //Short_t T6; //right width window

  if (opt.sTg[Chan]<0) { //use hardware trigger
    Peaks.push_back(PeakClass());
    //pk = &Peaks.back();
    //pk->Pos=crs->Pre[Chan];
    Pos=cpar.Pre[Chan];
  }
  else { //use software trigger
    FindPeaks();
  }

  if (Peaks.empty()) {
    return;
  }

  pk=&Peaks.back();
  Float_t sum;

  B1=Pos+opt.Base1[Chan];
  B2=Pos+opt.Base2[Chan]+1;
  P1=Pos+opt.Peak1[Chan];
  P2=Pos+opt.Peak2[Chan]+1;
  T1=Pos+opt.T1[Chan];
  T2=Pos+opt.T2[Chan]+1;
  W1=Pos+opt.W1[Chan];
  W2=Pos+opt.W2[Chan]+1;

  //cout << "B2: " << pk->B1 << " " << pk->B2 << endl;

  if (B1<0) B1=0;
  //if (B2<=B1) B2=B1+1;
  if (B2<=B1) B2=B1; //base can be zero if B2==B1
  if (P1<0) {P1=0; pk->Type|=P_B1;}
  if (P2<=P1) {P2=P1+1; pk->Type|=P_B2;}

  if (B1>=sz) B1=sz-1;
  if (B2>sz) B2=sz;
  if (P1>=sz) {P1=sz-1; pk->Type|=P_B1;}
  if (P2>sz) {P2=sz; pk->Type|=P_B2;}

  if (T1<(int)kk) {T1=kk; pk->Type|=P_B11;}
  if (T2<=T1) {T2=T1+1; pk->Type|=P_B11;}
  if (W1<(int)kk) {W1=kk; pk->Type|=P_B22;}
  if (W2<=W1) {W2=W1+1; pk->Type|=P_B22;}

  if (T1>sz) {T1=sz-1; pk->Type|=P_B11;}
  if (T2>sz) {T2=sz; pk->Type|=P_B11;}
  if (W1>sz) {W1=sz-1; pk->Type|=P_B22;}
  if (W2>sz) {W2=sz; pk->Type|=P_B22;}

  pk->Time=0;
  sum=0;

  if (crs->use_2nd_deriv[Chan]) { //use 2nd deriv
    // 05.10.2020
    for (int j=T1;j<T2;j++) {
      if (j<kk+1)
	continue;
      Float_t dif2=sData[j]-sData[j-kk]-sData[j-1]+sData[j-kk-1];
      if (dif2>0) {
	pk->Time+=dif2*j;
	sum+=dif2;
      }
      //cout << "dif2: " << j << " " << dif2 << " " << sData[j] << endl;
    }
    // cout << "T7: " << Tstamp64 << " " << Pos << " " << T1 << " " << T2
    // 	 << " " << pk->Time << " " << sum << endl; 
  }
  else { //use 1st deriv
    for (int j=T1;j<T2;j++) {
      if (j<kk)
	continue;
      Float_t dif=sData[j]-sData[j-kk];
      if (dif>0) {
	pk->Time+=dif*j;
	sum+=dif;
      }
    }
  }
  if (abs(sum)>1e-5) {
    pk->Time/=sum;
  }
  else
    pk->Time=(T1+T2)*0.5;

  pk->Time-=cpar.Pre[Chan];

  //pk->Time=sum;

  /*
    pk->Width=0;
    sum=0;
    for (int j=W1;j<W2;j++) {
    //if (j>=0 && j<sz && j-kk>=0 && j-kk<sz) {
    Float_t dif=sData[j]-sData[j-kk]+gRandom->Rndm();
    //Float_t dif=sData[j];//-sData[j-kk];
    pk->Width+=dif*j;
    sum+=dif;
    //}
    //nt++;
    }
    if (abs(sum)>1e-5) {
    //sum+=gRandom->Rndm();
    //pk->Width+=gRandom->Rndm();
    pk->Width/=sum;
    }
    else
    //pk->Width=-999+Pos;
    pk->Width=(W1+W2)*0.5;

    //pk->Width-=Pos;
    pk->Width-=cpar.Pre[Chan];
  */


  /*
  double bkg2=(sData[P2]+sData[P1])/2;
  double asum2=0;
  double mean2=0;
  double rms2=0;
  for (int j=P1;j<P2;j++) {
    double w=sData[j]-bkg2;
    double aw=fabs(w);
    //sum2+=w;
    asum2+=aw;
    mean2+=aw*j;
    rms2+=aw*j*j;
    //nrms++;
  }
  mean2=mean2/asum2;
  pk->Width2=sqrt(rms2/asum2-mean2*mean2);
  */



  //cout << "Width2: " << (int) Chan << " " << Tstamp64 << " " << pk->Width2 << endl;
  //pk->Width+=0.1;

  //baseline
  pk->Base=0;
  int nbkg=0;
  for (int j=B1;j<B2;j++) {
    pk->Base+=sData[j];
    nbkg++;
  }

  //cout << "Bkg: " << nbkg << " " << opt.bkg2[Chan]-opt.bkg1[Chan] << endl;

  if (nbkg)
    pk->Base/=nbkg;
  else {
    //cout << "Error!!! Error!!! Error!!! Check it!!! zero background!!!: " << this->Tstamp64 << " " << nbkg << " " << B1 << " " << B2 << endl;
  }

  int nn=0;
  //peak Area & Height
  pk->Area0=0;
  pk->Height=-1e30;
  for (int j=P1;j<P2;j++) {
    pk->Area0+=sData[j];
    if (sData[j]>pk->Height) pk->Height = sData[j];
    nn++;
  }
  if (nn) {
    pk->Area0/=nn;
  }
  else {
    cout << "zero Area: " << this->Tstamp64 << " " << Pos << " " << P1 << " " << P2 << endl;
  }

  //calibration
  pk->Area=pk->Area0 - pk->Base;
  //pk->Area*=opt.emult[Chan];
  pk->Area=opt.E0[Chan] + opt.E1[Chan]*pk->Area +
    opt.E2[Chan]*pk->Area*pk->Area;

  if (opt.Bc[Chan]) {
    pk->Area+=opt.Bc[Chan]*pk->Base;
  }

  //width === area2
  pk->Width=0;
  if (pk->Area) {
    nn=0;
    for (int j=W1;j<W2;j++) {
      pk->Width+=sData[j];
      nn++;
    }
    if (nn) {
      //cout << "w3: " << (int) Chan << " " << pk->Width << " " << sum << " " << pk->Width/sum << endl;
      pk->Width/=nn;
    }
    else {
      cout << "zero Width: " << this->Tstamp64 << " " << Pos << " " << P1 << " " << P2 << endl;
    }
  	
    pk->Width-=pk->Base;
    //YK
    pk->Width=opt.E0[Chan] + opt.E1[Chan]*pk->Width +
    opt.E2[Chan]*pk->Width*pk->Width;
    pk->Width/=pk->Area;
  }

  //slope1 (baseline)
  pk->Slope1=0;
  nbkg=0;
  for (int j=B1+1;j<B2;j++) {
    pk->Slope1+=sData[j]-sData[j-1];
    nbkg++;
  }

  if (nbkg)
    pk->Slope1/=nbkg;

  //slope2 (peak)
  pk->Slope2=0;
  nbkg=0;
  for (int j=P1+1;j<P2;j++) {
    pk->Slope2+=sData[j]-sData[j-1];
    nbkg++;
  }

  if (nbkg)
    pk->Slope2/=nbkg;

} //PeakAna33()

void PulseClass::CheckDSP() {
  if (Peaks.size()!=2) {
    cout <<"CheckDSP: Peaks.size()!=2: " << Peaks.size()
	 << " " << Counter << endl;
    return;
  }
  const int nn=5;
  const Float_t eps=0.1;

  Float_t cc[nn];
  cc[0] = Peaks[0].Base - Peaks[1].Base;
  cc[1] = Peaks[0].Area0 - Peaks[1].Area0;
  cc[2] = Peaks[0].Height - Peaks[1].Height;
  cc[3] = Peaks[0].Time - Peaks[1].Time;
  cc[4] = Peaks[0].Width - Peaks[1].Width;

  Bool_t bad=false;
  for (int i=0;i<nn;i++) {
    if (abs(cc[i])>eps)
      bad=true;
  }

  if (bad) {
    printf(ANSI_COLOR_YELLOW"Error!\n");
    printf(ANSI_COLOR_RED
	   "Alp: %d E:%lld B:%8.1f A0:%8.1f H:%8.1f T:%8.1f W:%8.1f P:%4d\n" ANSI_COLOR_RESET,
	   Chan,Counter,Peaks[0].Base,Peaks[0].Area0,Peaks[0].Height,
	   Peaks[0].Time,Peaks[0].Width,Pos);
    printf(ANSI_COLOR_GREEN
	   "Kop: %d E:%lld B:%8.1f A0:%8.1f H:%8.1f T:%8.1f W:%8.1f P:%4d\n" ANSI_COLOR_RESET,
	   Chan,Counter,Peaks[1].Base,Peaks[1].Area0,Peaks[1].Height,
	   Peaks[1].Time,Peaks[1].Width,Pos);
  }
  else {
    //printf("%10lld OK\n",Counter);
  }

  Peaks.pop_back();
	
}

EventClass::EventClass() {
  Spin=0;
  //Tstmp=-9999999999;
  T0=99999;
  //Analyzed=false;
}

/*
  void EventClass::Make_Mean_Event() {
  for (int i=0;i<MAX_CH;i++) {
  PulseClass pp = PulseClass();
  pp.ptype=0;
  pp.Chan=i;
  pp.Counter=0;
  for (UInt_t j=0;j<cpar.Len[i];j++) {
  pp.sData.push_back(0);
  }
  pulses.push_back(pp);
  }

  //cout << "Make_mean: " << pulses.size() << endl; 

  }

  void EventClass::Pulse_Mean_Add(PulseClass *pls) {

  //cout << "Pulse_Mean_Add1: " << pulses.size() << " " << (int) pls->Chan << endl; 
  if (pls->Chan >=opt.Nchan) {
  cout << "Pulse_Mean_Add: wrong channel: " << (int) pls->Chan << endl; 
  }
  PulseClass *pp = &pulses.at(pls->Chan);

  if (pp->sData.size() != cpar.Len[pls->Chan]) {
  pp->sData.resize(cpar.Len[pls->Chan]);
  cout << "Pulse_Mean_Add: resize: " << (int) pls->Chan
  << " " << pp->sData.size() << endl;     
  }

  if (pls->sData.size()  != cpar.Len[pls->Chan]) {
  cout << "Error: " << (int) pls->Chan << " " << pls->Counter
  << " " << pls->sData.size() << endl;
  return;
  }
  for (UInt_t j=0;j<cpar.Len[pls->Chan];j++) {
  pp->sData[j]+=pls->sData[j];
  }

  pp->Counter++;
  //cout << "Pulse_Mean_Add2: " << (int) pls->Chan << endl; 

  }
*/
void EventClass::Pulse_Ana_Add(PulseClass *pls) {
  //void EventClass::Pulse_Ana_Add(pulse_vect::iterator pls) {

  // if (opt.b_pulse) {
  //   crs->mean_event.Pulse_Mean_Add(pls);
  // }
  //Float_t dt;
  //Long64_t dt;

  for (UInt_t i=0;i<pulses.size();i++) {
    if (pls->Chan == pulses[i].Chan &&
	TMath::Abs(pls->Tstamp64-pulses[i].Tstamp64) < opt.tveto) {
      return;
    }
  }

  // cout << "state1: " << Nevt << " " << (int) Spin << " " << (int) pls->Chan
  //      << " " << opt.Master[pls->Chan] << endl;
  if (opt.Ms[pls->Chan]) {
    Spin|=2;
  }

  if (pls->Spin & 1) {
    Spin|=1;
  }

  //cout << "state2: " << Nevt << " " << (int) Spin << endl;
  if (pulses.empty()) { //this is the first pulse in the event
    //dt=0;
    Tstmp=pls->Tstamp64;
  }
  else {
    //dt = pls->Tstamp64 - Tstmp;
    for (std::vector<PeakClass>::reverse_iterator pk=pls->Peaks.rbegin();
	 pk!=pls->Peaks.rend();++pk) {
      //pk->Time+=dt;
      pk->Time+=pls->Tstamp64 - Tstmp;
    }
  }

  pulses.push_back(*pls);

  // if (Tstmp==-9999999999) { //this is the first pulse in the event
  //   Tstmp=pls->Tstamp64;
  // }
  // else if (pls->Tstamp64 < Tstmp) { //event exists & new pulse is earlier
  //   // -> correct T and T0
  //   if (T0<99998) //if T0 already exists, just adjust it
  //     T0+= Tstmp - pls->Tstamp64;
  //   Tstmp=pls->Tstamp64;
  // }

  if (opt.St[pls->Chan]) {
    if (pls->Peaks.size()) {
      //Float_t dt = pls->Tstamp64 - Tstmp;

      PeakClass *pk = &pls->Peaks.front();
      //Float_t T2 = pk->Time - crs->Pre[pls->Chan] + dt;
      //Float_t T2 = pk->Time - cpar.Pre[pls->Chan] + dt;
      //Float_t T2 = pk->Time + dt;

      //if (T2<T0) {
      if (pk->Time<T0) {
	T0=pk->Time;
      }
      // cout << "Peak: " << Nevt << " " << (int) pls->Chan
      //       << " " << pk->Time << " " 
      //       << pk->Pos << " " << T0 << endl;
    }
  }

}

void EventClass::Fill_Time_Extend(HMap* map) {
  TH1F* hh = (TH1F*) map->hst;
  Double_t max = hh->GetXaxis()->GetXmax();

  if (opt.T_acq > max) {
    if (opt.T_acq - max > 1e4) { //larger than several hours
      cout << "Time leap is too large: " << this->Nevt << " " << opt.T_acq << " " << max << " " << crs->Tstart64 << endl;
    }
    else {
      Double_t rt = opt.T_acq/max;

      int nn=2;
      while (rt>2) {
	nn*=2;
	rt*=0.5;
      }

      max*=nn;

      int nbin = hh->GetNbinsX()*nn;
      Float_t* arr = new Float_t[nbin+2];
      memset(arr,0,sizeof(Float_t)*(nbin+2));
      Float_t* arr2 = hh->GetArray();
      memcpy(arr,arr2,hh->GetSize()*sizeof(Float_t));
      hh->SetBins(nbin,0,max);
      hh->Adopt(nbin+2,arr);
    }
  }
}

void EventClass::Fill1d(Bool_t first, HMap* map[], int ch, Float_t x) {
  // if first -> check cuts and set cut_flag
  if (first) {
    //int bin = map[ch]->hst->FindFixBin(x);
    //map[ch]->hst->AddBinContent(bin);
    map[ch]->hst->Fill(x);
    if (opt.ncuts) {
      for (int i=1;i<opt.ncuts;i++) {
	if (getbit(*(map[ch]->hd->cut+map[ch]->nn),i)) {
	  if (x>=hcl->cutG[i]->GetX()[0] && x<hcl->cutG[i]->GetX()[1]) {
	    //if (hcl->cut_flag[i]==0)
	    hcl->cut_flag[i]=1;
	  }
	  //else
	  //hcl->cut_flag[i]=-1;
	}
      }
    }
  }
  else if (*(map[ch]->hd->w+map[ch]->nn)) {
    for (int i=1;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {
	//int bin = map[ch]->h_cuts[i]->hst->FindFixBin(x);
	//map[ch]->h_cuts[i]->hst->AddBinContent(bin);
	map[ch]->h_cuts[i]->hst->Fill(x);
      }
    }
    // if (crs->cut_main) {
    //   map[ch]->h_MT->hst->Fill(x);
    // }
  }

  if (ch<MAX_CH)
    for (int j=0;j<NGRP;j++)
      if (opt.Grp[ch][j])
        Fill1d(first,map,MAX_CH+j,x);
}

void EventClass::Fill1dw(Bool_t first, HMap* map[], int ch, Float_t x,
			 Double_t w) {
  // if first -> check cuts and set cut_flag
  if (first) {
    map[ch]->hst->Fill(x,w);
    if (opt.ncuts) {
      for (int i=1;i<opt.ncuts;i++) {
	if (getbit(*(map[ch]->hd->cut+map[ch]->nn),i)) {
	  //if (getbit(*(map[ch]->cut_index),i)) {
	  if (x>=hcl->cutG[i]->GetX()[0] && x<hcl->cutG[i]->GetX()[1]) {
	    hcl->cut_flag[i]=1;
	  }
	}
      }
    }
  }
  else if (*(map[ch]->hd->w+map[ch]->nn)) {
    //else if (*(map[ch]->wrk)) {
    for (int i=1;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {
	map[ch]->h_cuts[i]->hst->Fill(x,w);
      }
    }
  }

  if (ch<MAX_CH)
    for (int j=0;j<NGRP;j++)
      if (opt.Grp[ch][j])
        Fill1dw(first,map,MAX_CH+j,x,w);
}

void EventClass::Fill_Mean1(TH1F* hh, Float_t* Data, Int_t nbins, int ideriv) {
  Float_t zz=0;
  Float_t* arr = hh->GetArray();
  int nent=hh->GetEntries();
  int min = TMath::Min(hh->fN-2,nbins);
  //cout << "min: " << hh->fN << " " << nbins << " " << min << endl;
  for (Int_t j=0;j<min;j++) {
    if (ideriv==0) {
      zz = Data[j];
    }
    else if (ideriv==1) {
      if (j) zz = Data[j]-Data[j-1];
      else zz=0;
    }

    Float_t val = arr[j+1]*nent+zz;
    arr[j+1]=val/(nent+1);
    //hh->SetBinError(j+1,10);
  }
  hh->SetEntries(nent+1);
  // cout << "Pulse_Mean: " << (int) pls->Chan << " "
  //   << map->hst->GetEntries() << endl;  
}
void EventClass::Fill_Mean_Pulse(Bool_t first, HMap* map, PulseClass* pls,
				 int ideriv) {

  //HMap* map = hcl->m_pulse[n];
  int ch = pls->Chan;
	
  // if (pls->sData.size() < cpar.Len[pls->Chan]) {
  //   cout << "Error: " << (int) pls->Chan << " " << pls->Counter
  //   << " " << pls->sData.size() << endl;
  //   return;
  // }
  if (first) {
    int newsz = pls->sData.size();
    if (map->hst->GetNbinsX() < newsz) {
      map->hst->
	SetBins(pls->sData.size(),-cpar.Pre[ch],newsz-cpar.Pre[ch]);
    }
    Fill_Mean1((TH1F*)map->hst, &pls->sData[0], newsz, ideriv);
  } //if first
  else if (*(map->hd->w+map->nn)) {
    //else if (*(map->wrk)) {
    for (int i=1;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {

	if (map->h_cuts[i]->hst->GetNbinsX() != (int) cpar.Len[ch]) {
	  map->h_cuts[i]->hst->
	    SetBins(cpar.Len[ch],-cpar.Pre[ch],cpar.Len[ch]-cpar.Pre[ch]);
	}

	Fill_Mean1((TH1F*)map->h_cuts[i]->hst, &pls->sData[0], cpar.Len[ch], ideriv);

      }
    }
  }

}

void EventClass::Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y) {
  if (first) {
    //int bin = map->hst->FindFixBin(x,y);
    //map->hst->AddBinContent(bin);
    map->hst->Fill(x,y);
    if (opt.ncuts) {
      for (int i=1;i<opt.ncuts;i++) {
	if (getbit(*(map->hd->cut+map->nn),i)) {
	  //if (getbit(*(map->cut_index),i)) {
	  if (hcl->cutG[i]->IsInside(x,y)) {
	    //if (hcl->cut_flag[i]==0)
	    hcl->cut_flag[i]=1;
	  }
	  //else
	  //hcl->cut_flag[i]=-1;
	}
      }
    }
  }
  else if (*(map->hd->w+map->nn)) {
    //else if (*(map->wrk)) {
    for (int i=1;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {
	//int bin = map->h_cuts[i]->hst->FindFixBin(x,y);
	//map->h_cuts[i]->hst->AddBinContent(bin);
	map->h_cuts[i]->hst->Fill(x,y);
      }
    }
  }
}

void EventClass::FillHist(Bool_t first) {
  double DT = opt.Period*1e-9;
  Double_t tt;
  Double_t ee,sqee;
  //int mult=0;
  Long64_t tm;

  Float_t AA[MAX_CH+1] = {}; //initialize to zero

  //cout << "FillHist: " << this->Nevt << endl;

  // инициализация
  if (first) {
    if (crs->Tstart64<0) {
      crs->Tstart64 = Tstmp;
      //cout << "Tstart64: " << crs->Tstart64 << endl;
    }
  	
    opt.T_acq=(Tstmp - crs->Tstart64)*DT;

    if (opt.Tlim2) {
      if (opt.T_acq > opt.Tlim2) {
	if (crs->b_fana) {
	  crs->b_stop=true;
	  crs->b_fana=false;
	  crs->b_run=0;
	}
	return;
      }
      else if (opt.T_acq < opt.Tlim1) {
	return;
      }
    }

    if (opt.ncuts) {
      //memcpy(hcl->cut_flag,initcuts,sizeof(hcl->cut_flag));
      memset(hcl->cut_flag,0,sizeof(hcl->cut_flag));
    }
  }

  for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;

    // if (ch==7) {
    //   cout << "ch7: " << (int) ch << " " << pulses[i].Peaks.size() << endl;
    // }

    if (opt.h_pulse.b) {
      Fill_Mean_Pulse(first,hcl->m_pulse[ch],&pulses[i],0);
    }

    if (opt.h_deriv.b) {
      Fill_Mean_Pulse(first,hcl->m_deriv[ch],&pulses[i],1);
    }

    for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
      PeakClass* pk = &pulses[i].Peaks[j];

      // if (opt.elim2[ch]>0 &&
      //      (pk->Area<opt.elim1[ch] || pk->Area>opt.elim2[ch])) {
      //    continue;
      // }

      if (opt.h_rate.b) {
	if (first) {
	  Fill_Time_Extend(hcl->m_rate[ch]);
	}
	Fill1d(first,hcl->m_rate,ch,opt.T_acq);
      }

      if (opt.h_area.b) {
	Fill1d(first,hcl->m_area,ch,pk->Area);
      }

      if (opt.h_area0.b) {
	Fill1d(first,hcl->m_area0,ch,pk->Area0);
      }

      if (opt.h_base.b) {
	Fill1d(first,hcl->m_base,ch,pk->Base);
      }

      if (opt.h_slope1.b) {
	Fill1d(first,hcl->m_slope1,ch,pk->Slope1);
      }

      if (opt.h_slope2.b) {
	Fill1d(first,hcl->m_slope2,ch,pk->Slope2);
      }

      if (opt.h_width.b) {
	Fill1d(first,hcl->m_width,ch,pk->Width);
      }

      // if (opt.h_width2.b) {
      // 	Fill1d(first,hcl->m_width2,ch,pk->Width2);
      // }

      // if (opt.h_width3.b) {
      //    Fill1d(first,hcl->m_width3,ch,pk->Width3/pk->Area);
      // }

      if (opt.h_area_base.b) {
	Fill2d(first,hcl->m_area_base[ch],pk->Area,pk->Base);
      }

      if (opt.h_area_sl1.b) {
	Fill2d(first,hcl->m_area_sl1[ch],pk->Area,pk->Slope1);
      }

      if (opt.h_area_sl2.b) {
	Fill2d(first,hcl->m_area_sl2[ch],pk->Area,pk->Slope2);
      }

      if (opt.h_slope_12.b) {
	Fill2d(first,hcl->m_slope_12[ch],pk->Slope1,pk->Slope2);
      }

      if (opt.h_area_time.b) {
	Fill2d(first,hcl->m_area_time[ch],pk->Area,opt.T_acq);
      }

      if (opt.h_area_width.b) {
	Fill2d(first,hcl->m_area_width[ch],pk->Area,pk->Width);
      }

      // if (opt.h_area_width2.b) {
      // 	Fill2d(first,hcl->m_area_width2[ch],pk->Area,pk->Width2);
      // }

      // if (opt.h_area_width3.b) {
      //    Fill2d(first,hcl->m_area_width3[ch],pk->Area,pk->Width3/pk->Area);
      // }

      // if (opt.h_width_12.b) {
      //    Fill2d(first,hcl->m_width_12[ch],pk->Width,pk->Width2);
      // }

      if (opt.h_hei.b) {
	Fill1d(first,hcl->m_height,ch,pk->Height);
      }

      if (opt.h_time.b && T0!=99999) {
	//double dt = pulses[i].Tstamp64 - Tstmp;
	//tt = pk->Time - T0 + dt;
	tt = pk->Time - T0;
	Fill1d(first,hcl->m_time,ch,tt*opt.Period+opt.sD[ch]);




	/*
	static float P0;
	Long64_t dt = pulses[i].Tstamp64-Tstmp;
	float pp = pulses[i].Pos-cpar.Pre[ch]-pk->Time+dt;
	const char* col;
	const char* nm = "Time: ";

	if (tt>1.5)
	  col=BRED;
	else
	  col=BBLU;

	if (!tt) {
	  nm="T0: ";
	  P0 = pp;
	}

	prnt("ssd f f f ls;",col,nm,Nevt,tt,pp,pp-P0,dt,RST);

	//cout << "Time: " << Nevt << " " << tt*opt.Period << " " << pp*opt.Period << " " << endl;
	//cout << "Time: " << Nevt << " " << pk->Time << " " << T0 << endl;
	if (tt) {
	  Fill1d(first,hcl->m_time,ch+2,pp);
	}
	else {
	  Fill1d(first,hcl->m_time,ch+2,pp);
	}
	Fill1d(first,hcl->m_time,ch,tt+opt.sD[ch]);
	*/




      }

      if (j==0) { //only for the first peak
	//ntof
	if (opt.h_ntof.b || opt.h_etof.b || opt.h_ltof.b) {
	  // определяем старт
	  if (ch==opt.start_ch) {
	    crs->Tstart0 = Tstmp + Long64_t(pk->Time);
	  }
	  //if ((i==pulses.size()-1)) { //last pulse
	  if (crs->Tstart0>0) {
	    //tm = pulses[i].Tstamp64 + Long64_t(pk->Time);
	    tm = Tstmp + Long64_t(pk->Time);
	    tt = (tm - crs->Tstart0)*mks*opt.Period;

	    if (tt>0) {
	      //check for missed starts
	      if (opt.ntof_period>0.01 && tt>opt.ntof_period) {
		crs->Tstart0+=Long64_t(1000*opt.ntof_period/opt.Period);
		tt = (tm - crs->Tstart0)*mks*opt.Period;
	      }
	      if (opt.h_ntof.b) {
		Fill1d(first,hcl->m_ntof,ch,tt);
	      }
	      if (opt.h_area_ntof.b) {
		Fill2d(first,hcl->m_area_ntof[ch],pk->Area,tt);
	      }
	      if (opt.h_etof.b) {
		ee = 72.298*opt.Flpath/(tt-opt.TofZero);
		ee= ee*ee;
		Fill1d(first,hcl->m_etof,ch,ee);
	      }
	      if (opt.h_ltof.b) {
		sqee = 72.298*opt.Flpath/(tt-opt.TofZero);
		double lambda = 0.286*sqee;
		Fill1d(first,hcl->m_ltof,ch,lambda);
	      }
	    } //if tt>0
	  }
	  //} //if last pulse
	} //if (opt.h_ntof.b || opt.h_etof.b)

	if (opt.h_axay.b && (ch<=opt.h_axay.bins2)) {
	  AA[ch]=pk->Area;
	}

	if (opt.h_per.b) {
	  //tm = pulses[i].Tstamp64 + Long64_t(pk->Time);
	  tm = Tstmp + Long64_t(pk->Time);
	  if (hcl->T_prev[ch] && tm!=hcl->T_prev[ch]) {
	    tt = (tm - hcl->T_prev[ch])*mks*opt.Period; //convert to mks

	    //int mm=tt/32;
	    //mm*=32;
	    //tt-=mm;

	    //cout << "tt: " << tt << " " << tm << " " << hcl->T_prev[ch] << endl;
	    Fill1d(first,hcl->m_per,ch,tt);
	  }
	  hcl->T_prev[ch]=tm;
	}
      } // if (j==0) only for 1st peak
    } //for peaks...
  } //for (UInt_t i=0;i<pulses.size()...

  if (opt.h_axay.b) {
    int ii=0;
    int nmax=opt.h_axay.bins2;
    for (int i=0;i<=nmax;i++) {
      for (int j=i+1;j<=nmax;j++) {
       if (AA[i] && AA[j]) {
         Fill2d(first,hcl->m_axay[ii],AA[i],AA[j]);
       }
       ii++;
     }
   }
 }

  /*
    if (opt.dec_write) {
    crs->rTime=T;
    crs->rSpin = Spin;
    crs->Tree->Fill();
    crs->rPeaks.clear();
    }
  */


  if (opt.h_prof.b) {
    if (first) {
      memset(hcl->h_sum,0,sizeof(hcl->h_sum));
      for (UInt_t i=0;i<pulses.size();i++) {
	Int_t pp = crs->prof_ch[pulses[i].Chan];
	hcl->h_xy=-1;
	hcl->h_off=1;
	if (pp>=crs->PROF_64) {
	  pp-=crs->PROF_64;
	  switch (pp) {
	  case 0: //P+(33-64) (X)
	    //hcl->h_p=hcl->m_prof_x[0];
	    //hcl->h_a=hcl->m_prof_x[2];
	    hcl->h_xy=0;
	    hcl->h_off=33;
	    break;
	  case 1: //P+(1-32) (X)
	    //hcl->h_p=hcl->m_prof_x[0];
	    //hcl->h_a=hcl->m_prof_x[2];
	    hcl->h_xy=0;
	    break;
	  case 2: //N+(33-64) (Y)
	    //hcl->h_p=hcl->m_prof_x[1];
	    //hcl->h_a=hcl->m_prof_x[3];
	    hcl->h_xy=1;
	    hcl->h_off=33;
	    break;
	  case 3: //N+(1-32) (Y)
	    //hcl->h_p=hcl->m_prof_x[1];
	    //hcl->h_a=hcl->m_prof_x[3];
	    hcl->h_xy=1;
	    break;
	  case 4:
	    // do nothing
	    break;
	  default:
	    cout << "wrong prof channel: " << pp << " " << pulses[i].Chan << endl;
	  } //switch

	  //cout << "hcl: " << i << " " << pp << " " << hcl->h_xy << " " << hcl->h_off << endl;
	  if (hcl->h_xy>=0) {//one of Prof64 position channels
	    PulseClass *pulse = &pulses[i];  
	    int dt=(pulse->Tstamp64-Tstmp)-cpar.Pre[pulse->Chan];
	    int size = pulse->sData.size();

	    for (int kk=-1;kk<31;kk++) {
	      int x1 = 0+opt.Prof64_W[0]+opt.Prof64_W[1]*kk;
	      //int x2 = x1 + opt.Prof64_W[2];
	      int xmin = TMath::Max(-dt+x1,0);
	      int xmax = TMath::Min(size,-dt+x1+opt.Prof64_W[2]);
	      //double sum=0;
	      for (int j=xmin;j<xmax;j++) {
		// if (j<0 || j>=size) {
		//   cout << "j!!! sise!: " << j << " " << size << endl;
		// }
		hcl->h_sum[hcl->h_xy][hcl->h_off+kk]+=pulse->sData[j];
	      }
	      if (xmax-xmin>0) {
		hcl->h_sum[hcl->h_xy][hcl->h_off+kk]/=(xmax-xmin);
	      }

	      // if (kk==1 && pp>=0 && pp<=3)
	      //   cout << "sum2/N2: " << Nevt << " " << i << " " << (int)pulse->Chan << " " << kk << " " << sum << " " << xmax-xmin << " " << pp << " " << first << endl;
	    } //for kk
	  } //if xy>0  
	} //if pp

	// else if (pp>=crs->ING_Y)
	//    ay=pp-crs->ING_Y;
	// else if (pp>=crs->ING_X)
	//    ax=pp-crs->ING_X;
	// else if (pp>=crs->PROF_Y)
	//    py=pp-crs->PROF_Y;
	// else if (pp>=crs->PROF_X)
	//    px=pp-crs->PROF_X;
	//cout << "prof: " << (int)pulses[i].Chan << " " << pp << " " << p64 << endl;
      } //for i pulses.size()

      if (!opt.h_prof_x.b) { //old profilometer
	int ax=999,ay=999,px=999,py=999;//,p64=0;

	if (pulses.size()==4) {
	  for (UInt_t i=0;i<pulses.size();i++) {
	    Int_t pp = crs->prof_ch[pulses[i].Chan];

	    if (pp>=crs->PROF_64)
	      ;//p64=pp-crs->PROF_64;
	    else if (pp>=crs->ING_Y)
	      ay=pp-crs->ING_Y;
	    else if (pp>=crs->ING_X)
	      ax=pp-crs->ING_X;
	    else if (pp>=crs->PROF_Y)
	      py=pp-crs->PROF_Y;
	    else if (pp>=crs->PROF_X)
	      px=pp-crs->PROF_X;
	    //cout << "prof: " << (int)pulses[i].Chan << " " << pp << " " << p64 << endl;
	  } //for

	  int ch_alpha = ax + (opt.prof_ny-ay-1)*opt.prof_ny;

	  //cout << "prof: " << crs->nevents << " " << ch_alpha << endl;
	  if (ch_alpha>=0 && ch_alpha<opt.prof_ny*opt.prof_nx)
	    Fill2d(first,hcl->m_prof[ch_alpha],px*15+1,py*15+1);
	  //else {
	  //}

	} //if size==4
      }//if old
      //cout << "sum: " << hcl->h_sum[0][15] << " " << hcl->h_sum[1][15] << endl;
      Fill_Mean1((TH1F*)hcl->m_prof_x[2]->hst, hcl->h_sum[0], 64, 0); //X
      Fill_Mean1((TH1F*)hcl->m_prof_x[3]->hst, hcl->h_sum[1], 64, 0); //Y
    } //if first
    else {
    }

  } //if h_prof.b


  if (first) {
    // for (int i=0;i<opt.ncuts;i++) {
    //   hcl->cut_flag[i]=(hcl->cut_flag[i]>0);
    // }
    if (hcl->b_formula) {
      for (int i=1;i<opt.ncuts;i++) {
	if (opt.pcuts[i]==1) {//formula
	  hcl->cut_flag[i]=hcl->cform[i]->EvalPar(0,hcl->cut_flag);
	}
	//cout << "cut_flag: " << Nevt << " " << i << " " << hcl->cut_flag[i] << endl;
      }
    }
    // if (crs->b_maintrig) {
    //   crs->cut_main = crs->maintrig.EvalPar(0,hcl->cut_flag);
    // }
  }

}

void PulseClass::Smooth(int nn) {

  int n2=abs(nn);
  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

  int Nsamp = sData.size();

  for (int i=0;i<Nsamp;i++) {
    //int ll=1;
    for (int j=1;j<=n2;j++) {
      int k=i+j;
      if (k<Nsamp) {
	sData[i]+=sData[k];
	//ll++;
      }
    }

    int ndiv;
    if (Nsamp-i <= n2) {
      ndiv=Nsamp-i;
    }
    else {
      ndiv=n2+1;
    }

    sData[i]/=ndiv;
    if (nn<0)
      sData[i] = roundf(sData[i]);
    //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.sS, ndiv, sData[i]);

  }

}


void PulseClass::PrintPulse(int pdata) {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64-crs->Tstart64);
  if (pdata) {
    for (int i=0;i<(int)sData.size();i++) {
      printf("-- %d %f\n",i,sData[i]);
    }
  }
}

void EventClass::PrintEvent() {
  printf("Event: %8lld %10lld %10.2f %2ld\n",Nevt,Tstmp,T0,pulses.size());
  for (UInt_t i=0;i<pulses.size();i++) {
    for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
      printf("  %3d %lld %10.2f\n",pulses[i].Chan,pulses[i].Tstamp64,pulses[i].Peaks[j].Time);
    }
  }
}
