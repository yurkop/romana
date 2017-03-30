#include "common.h"
#include "toptions.h"
#include <iostream>

extern Toptions opt;

using namespace std;

PulseClass2::PulseClass2() {
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

void PulseClass2::FindPeaks() {
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
  
  UInt_t kk=opt.kderiv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  peak_type *pk=0;
  peak_type *p_prev=0;

  bool in_peak=false;
  Float_t D,pD=0;
  //int peakpos[DSIZE];
  Float_t jmax=0;
  int pp0=0;

  //D[0]=0;
  for (UInt_t j=kk;j<sData.size();j++) {
    D=sData[j]-sData[j-kk];
    if (!in_peak) {
      if (D>0 && pD<=0) pp0=j; 
      if (D >= opt.threshold[Chan]) {
	Peaks.push_back(peak_type());
	pk = &Peaks.back();
	pk->T1=pp0;
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
	  if (pk->Pos - p_prev->Pos < opt.deadTime[Chan])
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
    pk->Type|=P_B22;
  }
  
  // Peaks = new peak_type[Npeaks];
  // for (int i=0;i<Npeaks;i++) {
  //   Peaks[i].Pos = peakpos[i];
  // }

  //printf("FindPeaks: %d\n",Npeaks);

}

//-----------------------------

void PulseClass2::PeakAna() {

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
    int t3; //start window for timing
    int t4; //end window for timing

    if (opt.timing[Chan]) 
      tt=pk->Pos; //1->reference is Pos (zero crossing of 2nd drv)
    else
      tt=pk->T2; //0->reference is T2 (zero crossing of 1st drv)

    if (opt.twin1[Chan] == 99)
      t3=pk->T1;
    else
      t3=tt+opt.twin1[Chan];

    if (opt.twin2[Chan] == 99)
      t4=pk->T2;
    else
      t4=tt+opt.twin2[Chan];

    UInt_t kk=opt.kderiv[Chan];
    if (kk<1 || kk>=sData.size()) kk=1;

    if (t3<(int)kk) {t3=kk; pk->Type|=P_B111;}

    for (int j=t3;j<=t4;j++) {
      Float_t dif=sData[j]-sData[j-kk];
      pk->Time+=dif*j;
      sum+=dif;
      //nt++;
    }
    if (sum>1e-5)
      pk->Time/=sum;

    //cout << "TTT: " << t3 << " " << t4 << " " << pk->Time << " " << pk->T2
    // << " " << kk << endl;

  }

}

EventClass1::EventClass1() {
  T=0;
  T0=0;
}

void EventClass1::Pulse_Ana_Add(PulseClass2 *newpulse) {
  pulses.push_back(*newpulse);

  if (T==0 || newpulse->Tstamp64 < T) {
    T=newpulse->Tstamp64;
  }
  if (opt.Start[newpulse->Chan]) {
    for (UInt_t i=0;i<newpulse->Peaks.size();i++) {
      peak_type *pk = &newpulse->Peaks[i];
      //cout << "Peak: " << pk->Time << " " << pk->Pos << endl;
      if (T0<0.1 || pk->Time<T0) {
	T0=pk->Time;
      }
    }
  }
}

//---------------------------------------
PulseClass::PulseClass() {
  ptype=0;
  Nsamp=0;
  Npeaks=0;
  Analyzed=false;
  Tstamp64=0;
  //Data=NULL;
  //sData = new Float_t[size];
  Control=0;
  sData = NULL;
  tdif=99;
  //sData=NULL;
  Peaks=NULL;
}

PulseClass::PulseClass(ULong64_t size) {
  ptype=0;
  Nsamp=0;
  Npeaks=0;
  Analyzed=false;
  Tstamp64=0;
  //Data=NULL;
  sData = new Float_t[size];
  Control=0;
  //sData = NULL;
  tdif=99;
  //sData=NULL;
  Peaks=NULL;
}

PulseClass::~PulseClass() {
  /*
    if (Data) {
    //cout << "deleting Data" << endl;
    delete[] Data;
    }
  */
  /*
    if (sData)
    //cout << "delete pulse: " << sData << endl;
    delete[] sData;
    if (Peaks)
    delete[] Peaks;
  */
}

void PulseClass::Analyze() {

  //cout << "PulseClass::Analyze" << endl;

  if (opt.nsmoo[Chan]) {
    Smooth(opt.nsmoo[Chan]);
  }

  //FindPeaks(opt.ng_thresh,0);
  //PeakAna();

  switch (Chan) {
  case ch_gam:
    //findpeaks_gam(Chan, Nsamp, sData);
    //peaktime(Chan,sData,opt.gam_timing,opt.gam_twin);
    break;
  case ch_ng:
    break;
  case ch_nim:
    //findpeaks_ng(ch,NSamp, sEvent[i]);
    //peaktime(ch,sEvent[i],opt.ng_timing,opt.ng_twin);
    break;
  }

}

void PulseClass::FindPeaks(Float_t thresh, int deadtime) {
  // always first derivative is used
  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak

  Npeaks=0;
  bool in_peak=false;
  Float_t dif[DSIZE];
  int peakpos[DSIZE];
  int jmax=0;

  dif[0]=0;
  for (int j=1;j<Nsamp;j++) {
    dif[j]=sData[j]-sData[j-1];
    if (!in_peak) {
      if (dif[j] >= thresh) {
	in_peak=true;
	//printf("in_peak: %d %d %f %f\n",Chan,j,dif[j],thresh);
	continue;
      }
    }
    else { //in_peak
      if (!jmax && dif[j]<dif[j-1]) {//first maximum of dif -> peak position
	peakpos[Npeaks]=jmax=j-1;
	Npeaks++;
	if (deadtime) {
	  j+=deadtime;
	  in_peak=false;
	  continue;
	}
      }
      if (dif[j]<0) { //zero crossing -> end of the peak
	in_peak=false;
	jmax=0;
      }
    }
  }

  Peaks = new peak_type[Npeaks];
  for (int i=0;i<Npeaks;i++) {
    Peaks[i].Pos = peakpos[i];
  }

  //printf("FindPeaks: %d\n",Npeaks);

}

/*
  void PulseClass::PeakAna() {

  for (int i=0;i<Npeaks;i++) {
  //determine exact time
  //1st deriv
  Float_t mean1=0,sum1=0;
  for (int j=Peaks[i].Pos+1;j<Nsamp;j++) {
  Float_t dif=sData[j]-sData[j-1];
  if (dif<=0) {
  break;
  }
  mean1+=dif*j;
  sum1+=dif;
  }
  for (int j=Peaks[i].Pos;j>0;j--) {
  Float_t dif=sData[j]-sData[j-1];
  if (opt.ng_twin) {
  if (Peaks[i].Pos - j > opt.ng_twin) {
  break;
  }
  }
  else {
  if (dif<=0) {
  break;
  }
  }
  mean1+=dif*j;
  sum1+=dif;
  }
  Peaks[i].Time=mean1/sum1;

  //2nd deriv
  Float_t mean2=0,sum2=0;
  for (int j=Peaks[i].Pos;j>1;j--) {
  Float_t dif=sData[j]-2*sData[j-1]+sData[j-2];
  if (opt.ng_twin) {
  if (Peaks[i].Pos - j > opt.ng_twin) {
  break;
  }
  }
  else {
  if (dif<=0) {
  break;
  }
  }
  mean2+=dif*j;
  sum2+=dif;
  }
  Peaks[i].Time2=mean2/sum2;

  printf("Peak Time: %d %f %f\n",i,Peaks[i].Time,Peaks[i].Time2);

  }

  }
*/
void PulseClass::Smooth(int nn) {

  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

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

void PulseClass2::PrintPulse() {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64);
}
