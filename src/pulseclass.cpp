#include "common.h"
#include "toptions.h"
#include "libcrs.h"
#include <iostream>

//extern Coptions cpar;
extern Toptions opt;
extern CRS* crs;

using namespace std;

PulseClass::PulseClass() {
  ptype=P_NOSTOP;
  //Nsamp=0;
  //Npeaks=0;
  //Analyzed=false;
  Tstamp64=0;
  State=0;
  //tdif=99;
  //sData=NULL;
  //Peaks=NULL;
}

void PulseClass::FindPeaks() {
  // always K-th derivative (D) is used
  // T1 - D>0, D(-1)<=0 (left zero-crossing of D)
  // T2 - D<=0, d(-1)>0 (right zero-crossing of D)
  // Pos - Maximum between T1 and T2
  /*
  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak
  */
  
  UInt_t kk=opt.kdrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  peak_type *pk=0;
  peak_type *p_prev=0;

  bool in_peak=false;
  //Float_t D,pD=0;//deriv, prev.dreiv
  Float_t* D = new Float_t[sData.size()]();
  //int peakpos[DSIZE];
  Float_t jmax=0;
  //int pp0=0; //temporary T1

  //D[0]=0;
  UInt_t j;
  for (j=kk;j<sData.size();j++) {
    D[j]=sData[j]-sData[j-kk];
    if (!in_peak) {
      if (D[j] >= opt.thresh[Chan]) {
	in_peak=true;
	Peaks.push_back(peak_type());
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
      }
    }
    else { //in_peak
      if (D[j]>jmax) {//maximum of D -> peak position
	jmax=D[j];
	pk->Pos=j;
	//printf("in_peak: %lld %d %d %d %0.1f %0.1f\n",crs->nevents,Chan,j,kk,D[j],jmax);
	// peakpos[Npeaks]=jmax=j-1;
	// Npeaks++;
	// if (deadtime) {
	//   j+=deadtime;
	//   in_peak=false;
	//   continue;
	// }
      }
      else if (D[j]<0) { //zero crossing -> end of the peak
	in_peak=false;
	pk->T2=j-1;
	//cout << "T2: " << sData[pk->T2]-sData[pk->T2-kk] << endl;
	//pk->Height=jmax;
	jmax=0;
	if (Peaks.size()>1) { //this is at least second peak
	  p_prev = pk-1;
	  if (pk->Pos - p_prev->Pos < opt.deadT[Chan])
	    Peaks.pop_back();
	  else if (pk->Pos - p_prev->Pos < opt.pile[Chan]) {
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
  //	 << Peaks.at(i).Pos << endl;
  //}

  // Peaks = new peak_type[Npeaks];
  // for (int i=0;i<Npeaks;i++) {
  //   Peaks[i].Pos = peakpos[i];
  // }

  //printf("FindPeaks: %d\n",Npeaks);

  delete[] D;

}

//-----------------------------

void PulseClass::PeakAna() {

  for (UInt_t i=0;i<Peaks.size();i++) {
    peak_type *pk = &Peaks[i];

    int sz=sData.size()-1;
    
    //peak time & position
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

    UInt_t kk=opt.kdrv[Chan];
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

    pk->Pos = pk->Time+0.5;


    pk->B1=pk->Pos+opt.bkg1[Chan];
    pk->B2=pk->Pos+opt.bkg2[Chan];
    pk->P1=pk->Pos+opt.peak1[Chan];
    pk->P2=pk->Pos+opt.peak2[Chan];

    if (pk->B1<0) pk->B1=0;
    if (pk->B2<0) pk->B2=0;
    if (pk->P1<0) {pk->P1=0; pk->Type|=P_B1;}
    if (pk->P2<0) {pk->P2=0; pk->Type|=P_B2;}

    if (pk->B1>sz) pk->B1=sz;
    if (pk->B2>sz) pk->B2=sz;
    if (pk->P1>sz) {pk->P1=sz; pk->Type|=P_B1;}
    if (pk->P2>sz) {pk->P2=sz; pk->Type|=P_B2;}

    Float_t bkg=0;
    int nbkg=0;
    //background
    for (int j=pk->B1;j<=pk->B2;j++) {
      bkg+=sData[j];
      nbkg++;
    }
    if (nbkg)
      bkg/=nbkg;
    else {
      cout << "zero background!!!: " << nbkg << " " << pk->B1 << " " << pk->B2 << endl;
    }

    int nn=0;
    //peak Area & Height
    pk->Area=0;
    pk->Height=0;
    for (int j=pk->P1;j<=pk->P2;j++) {
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

  }

}

EventClass::EventClass() {
  State=0;
  T=0;
  T0=99999;
  Analyzed=false;
}

void EventClass::Pulse_Ana_Add(PulseClass *pls) {

  for (UInt_t i=0;i<pulses.size();i++) {
    if (pls->Chan == pulses[i].Chan &&
	TMath::Abs(pls->Tstamp64-pulses[i].Tstamp64) < opt.tveto) {
      return;
    }
  }

  if (pls->State) {
    State=1;
    //cout << "state: " << Nevt << " " << (int) State << endl;
  }

  pulses.push_back(*pls);

  if (T==0) { //this is the first pulse in event
    T=pls->Tstamp64;
  }
  else if (pls->Tstamp64 < T) { //event exists & new pulse is earlier
    // -> correct T and T0
    if (T0<99998)
      T0+= T - pls->Tstamp64;
    T=pls->Tstamp64;
  }

  if (opt.Start[pls->Chan]) {
    if (pls->Peaks.size()) {
      Float_t dt = pls->Tstamp64 - T;

      peak_type *pk = &pls->Peaks.front();
      Float_t T2 = pk->Time - crs->Pre[pls->Chan] + dt;
      //Float_t T2 = pk->Time - cpar.preWr[pls->Chan] + dt;
      //Float_t T2 = pk->Time + dt;

      if (T2<T0) {
	T0=T2;
      }
      // cout << "Peak: " << (int) pls->Chan << " " << pk->Time << " " 
      // 	   << pk->Pos << " " << T0 << " " << T2 << " " << dt << " "
      // 	   << T << endl;
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


void PulseClass::PrintPulse(int pdata) {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64);
  if (pdata) {
    for (int i=0;i<(int)sData.size();i++) {
      printf("-- %d %f\n",i,sData[i]);
    }
  }
}
