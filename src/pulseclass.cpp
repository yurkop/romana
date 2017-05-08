#include "common.h"
#include "toptions.h"
#include <iostream>

extern Coptions cpar;
extern Toptions opt;

using namespace std;

PulseClass::PulseClass() {
  ptype=P_NOSTOP;
  //Nsamp=0;
  //Npeaks=0;
  Analyzed=false;
  Tstamp64=0;
  Control=0;
  tdif=99;
  //sData=NULL;
  //Peaks=NULL;
}

void PulseClass::FindPeaks() {
  // always K-th derivative (D) is used
  // T1 - D>0, D(-1)<=0 (left zero-crossing of D)
  // T2 - D<=0, d(-1)>0 (right zero-crossing of D)
  // Pos - Maximum between P0 and P1
  /*
  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak
  */
  
  UInt_t kk=cpar.kderiv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  peak_type *pk=0;
  peak_type *p_prev=0;

  bool in_peak=false;
  Float_t D,pD=0;//deriv, prev.dreiv
  //int peakpos[DSIZE];
  Float_t jmax=0;
  int pp0=0; //temporary T1

  //D[0]=0;
  for (UInt_t j=kk;j<sData.size();j++) {
    D=sData[j]-sData[j-kk];
    if (!in_peak) {
      if (D>0 && pD<=0) pp0=j;
      if (D >= cpar.threshold[Chan]) {
	Peaks.push_back(peak_type());
	pk = &Peaks.back();
	pk->T1=pp0;
	//pk->T2=0;
	in_peak=true;
	//printf("in_peak: %d %d %f %f\n",Chan,j,D[j],thresh);
	//continue;
      }
    }
    else { //in_peak
      if (D>jmax) {//maximum of D -> peak position
	jmax=D;
	pk->Pos=j;
	// peakpos[Npeaks]=jmax=j-1;
	// Npeaks++;
	// if (deadtime) {
	//   j+=deadtime;
	//   in_peak=false;
	//   continue;
	// }
      }
      else if (D<0) { //zero crossing -> end of the peak
	in_peak=false;
	pk->T2=j-1;
	//cout << "T2: " << sData[pk->T2]-sData[pk->T2-kk] << endl;
	//pk->Height=jmax;
	jmax=0;
	if (Peaks.size()>1) { //this is at least second peak
	  p_prev = pk-1;
	  if (pk->Pos - p_prev->Pos < cpar.deadTime[Chan])
	    Peaks.pop_back();
	  else if (pk->Pos - p_prev->Pos < opt.pile[Chan]) {
	    p_prev->Type|=P_PILE1;
	    pk->Type|=P_PILE2;
	  }
	}
      }
    }
    pD=D;
  }

  if (in_peak) { //end of the pulse -> peak has no end
    pk->T2=sData.size();
    //pk->Type|=P_B22;
  }
  
  // Peaks = new peak_type[Npeaks];
  // for (int i=0;i<Npeaks;i++) {
  //   Peaks[i].Pos = peakpos[i];
  // }

  //printf("FindPeaks: %d\n",Npeaks);

}

//-----------------------------

void PulseClass::PeakAna() {

  for (UInt_t i=0;i<Peaks.size();i++) {
    peak_type *pk = &Peaks[i];
    int b1=pk->Pos+opt.bkg1[Chan];
    int b2=pk->Pos+opt.bkg2[Chan];
    int p1=pk->Pos+opt.peak1[Chan];
    int p2=pk->Pos+opt.peak2[Chan];

    int sz=sData.size()-1;
    
    if (b1<0) b1=0;
    if (b2<0) b2=0;
    if (p1<0) {p1=0; pk->Type|=P_B1;}
    if (p2<0) {p2=0; pk->Type|=P_B2;}

    if (b1>sz) b1=sz;
    if (b2>sz) b2=sz;
    if (p1>sz) {p1=sz; pk->Type|=P_B1;}
    if (p2>sz) {p2=sz; pk->Type|=P_B2;}

    Float_t bkg=0;
    int nbkg=0;
    //background
    for (int j=b1;j<=b2;j++) {
      bkg+=sData[j];
      nbkg++;
    }
    if (nbkg)
      bkg/=nbkg;
    else {
      cout << "zero background!!!" << nbkg << " " << b1 << " " << b2 << endl;
    }

    int nn=0;
    //peak Area & Height
    pk->Area=0;
    pk->Height=0;
    for (int j=p1;j<=p2;j++) {
      pk->Area+=sData[j];
      if (sData[j]>pk->Height) pk->Height = sData[j];
      nn++;
    }
    if (nn) {
      pk->Area/=nn;
    }
    else {
      cout << "zero Area!!!" << endl;
    }
    pk->Area-=bkg;

    //peak time
    //int nt=0;
    Float_t sum=0;
    pk->Time=0;
    int tt; //reference window (timing mode)
    //int t3; //start window for timing
    //int t4; //end window for timing

    if (opt.timing[Chan]==0) 
      tt=pk->Pos; //0->reference is Pos (maximum in 1st deriv)
    else if (opt.timing[Chan]==1)
      tt=pk->T1; //1->reference is T1 (Left zero crossing of 1st drv)
    else //if (opt.timing[Chan]==2)
      tt=pk->T2; //2->reference is T2 (Right zero crossing of 1st drv)

    if (opt.twin1[Chan] == 99)
      pk->T3=pk->T1;
    else
      pk->T3=tt+opt.twin1[Chan];

    if (opt.twin2[Chan] == 99)
      pk->T4=pk->T2;
    else
      pk->T4=tt+opt.twin2[Chan];

    UInt_t kk=cpar.kderiv[Chan];
    if (kk<1 || kk>=sData.size()) kk=1;

    if (pk->T3<(int)kk) {pk->T3=kk; pk->Type|=P_B11;}
    if (pk->T3>sz) {pk->T3=sz; pk->Type|=P_B11;}
    if (pk->T4<(int)kk) {pk->T4=kk; pk->Type|=P_B22;}
    if (pk->T4>sz) {pk->T4=sz; pk->Type|=P_B22;}

    //if (t4>(int)kk) {t3=kk; pk->Type|=P_B111;}

    for (int j=pk->T3;j<pk->T4;j++) {
      Float_t dif=sData[j]-sData[j-kk];
      pk->Time+=dif*j;
      sum+=dif;
      //nt++;
    }
    if (sum>1e-5)
      pk->Time/=sum;

    //pk->Time+=cpar.preWr[Chan];
    //cout << "TTT: " << t3 << " " << t4 << " " << pk->Time << " " << pk->T2
    // << " " << kk << endl;

  }

}

EventClass::EventClass() {
  T=0;
  T0=99999;
}

void EventClass::Pulse_Ana_Add(PulseClass *newpulse) {
  pulses.push_back(*newpulse);

  if (T==0) { //this is the first pulse in event
    T=newpulse->Tstamp64;
  }
  else if (newpulse->Tstamp64 < T) { //event exists -> correct T and T0
    if (T0<99998)
      T0+= T - newpulse->Tstamp64;
    T=newpulse->Tstamp64;
  }

  if (opt.Start[newpulse->Chan]) {
    if (newpulse->Peaks.size()) {
      Float_t dt = newpulse->Tstamp64 - T;

      peak_type *pk = &newpulse->Peaks.front();
      Float_t T2 = pk->Time - cpar.preWr[newpulse->Chan] + dt;

      if (T2<T0) {
	T0=T2;
      }
      cout << "Peak: " << (int) newpulse->Chan << " " << pk->Time << " " 
	   << pk->Pos << " " << T0 << " " << T2 << " " << dt << " "
	   << T << endl;
    }
  }
}


void PulseClass::Smooth(int nn) {

  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

  int Nsamp = sData.size();

  for (int i=0;i<Nsamp;i++) {
    //int ll=1;
    for (int j=1;j<=nn;j++) {
      int k=i+j;
      if (k<Nsamp) {
	sData[i]+=sData[k];
	//ll++;
      }
    }

    int ndiv;
    if (Nsamp-i <= nn) {
      ndiv=Nsamp-i;
    }
    else {
      ndiv=nn+1;
    }

    sData[i]/=ndiv;
    //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.nsmoo, ndiv, sData[i]);

  }

}


void PulseClass::PrintPulse() {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64);
}
