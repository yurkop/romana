#include "common.h"
#include "toptions.h"
#include "libcrs.h"
#include "hclass.h"
#include "libmana.h"
#include <iostream>

#include "TSystem.h"

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

namespace PROF {

  const int prof_ch[32] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    7, 6, 5, 4, 3, 2, 1, 0,
    7, 5, 3, 1, 6, 4, 2, 0,
    0, 2, 4, 6, 1, 3, 5, 7
  };

}

using namespace std;

PulseClass::PulseClass() {
  //ptype=P_NOSTOP;
  ptype=0;
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
  // Pos - threshold crossing, later Pos is redefined as Time+0.5
  //(from exact time)
  // Pos2 - Maximum of D between T1 and T2


  /*
  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak
  */

  if (sData.size()<2)
    return;

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

  D[0]=0;
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

  int sz=sData.size()-1;

  for (UInt_t i=0;i<Peaks.size();i++) {
    peak_type *pk = &Peaks[i];
  
    //peak time & position
    //int nt=0;
    Float_t sum=0;
    pk->Time=0;
    int tt; //reference window (timing mode)
    //int t3; //start window for timing
    //int t4; //end window for timing

    if (opt.timing[Chan]==0) 
      tt=pk->Pos; //0->reference is Pos (threshold crossing)
    else if (opt.timing[Chan]==1)
      tt=pk->T1; //1->reference is T1 (Left zero crossing of 1st drv)
    else if (opt.timing[Chan]==2)
      tt=pk->T2; //2->reference is T2 (Right zero crossing of 1st drv)
    else
      tt=pk->Pos2; //3->reference is Pos2 (maximum in 1st deriv)

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
    if (pk->T4<(int)kk) {pk->T4=kk; pk->Type|=P_B11;}
    if (pk->T4>sz) {pk->T4=sz; pk->Type|=P_B11;}

    //if (t4>(int)kk) {t3=kk; pk->Type|=P_B111;}

    for (int j=pk->T3;j<pk->T4;j++) {
      Float_t dif=sData[j]-sData[j-kk];
      pk->Time+=dif*j;
      sum+=dif;
      //nt++;
    }
    if (sum>1e-5)
      pk->Time/=sum;
    else
      pk->Time=-999;

    pk->Time-=cpar.preWr[Chan];
    //cout << "TTT: " << Tstamp64 << " " << pk->Time << " " << pk->T2
    //<< " " << kk << endl;

    //pk->Pos = pk->Time+0.5;
    pk->Pos = pk->Pos2;

    //cout << "pk: " << (int) Chan << " " << pk->T3 << " " << pk->T4 << " " << pk->Time << " " << pk->Pos << endl;

    pk->B1=pk->Pos+opt.bkg1[Chan];
    pk->B2=pk->Pos+opt.bkg2[Chan];
    pk->P1=pk->Pos+opt.peak1[Chan];
    pk->P2=pk->Pos+opt.peak2[Chan];

    // if (Counter==150) {
    //   cout << "E150: " << Counter << " " << Tstamp64 << " " << pk->B1 << " " << pk->B2 << " " << pk->Pos << " " << pk->Pos2 << " " << pk->P1 << " " << pk->P2 << endl;
    // }

    if (pk->B1<0) pk->B1=0;
    if (pk->B2<0) pk->B2=0;
    if (pk->P1<0) {pk->P1=0; pk->Type|=P_B1;}
    if (pk->P2<0) {pk->P2=0; pk->Type|=P_B2;}

    if (pk->B1>sz) pk->B1=sz;
    if (pk->B2>sz) pk->B2=sz;
    if (pk->P1>sz) {pk->P1=sz; pk->Type|=P_B1;}
    if (pk->P2>sz) {pk->P2=sz; pk->Type|=P_B2;}

    //Float_t bkg=0;
    pk->Base=0;
    int nbkg=0;
    //background
    for (int j=pk->B1;j<pk->B2;j++) {
      pk->Base+=sData[j];
      nbkg++;
    }
    if (nbkg)
      pk->Base/=nbkg;
    else {
      pk->Base=-999999;
      //cout << "Error!!! Error!!! Error!!! Check it!!! zero background!!!: " << Counter << " " << Tstamp64 << " " << nbkg << " " << pk->B1 << " " << pk->B2 << endl;
    }

    int nn=0;
    //peak Area & Height
    pk->Area0=0;
    pk->Height=0;
    for (int j=pk->P1;j<pk->P2;j++) {
      pk->Area0+=sData[j];
      if (sData[j]>pk->Height) pk->Height = sData[j];
      nn++;
    }
    if (nn) {
      pk->Area0/=nn;
      pk->Area=pk->Area0-pk->Base;
    }
    else {
      pk->Area0=-999999;
      pk->Area=-9999999;
      //cout << "zero Area: " << Counter << " " << Tstamp64 << " " << pk->Pos << " " << pk->P1 << " " << pk->P2 << endl;
    }

    pk->Area*=opt.emult[Chan];
    pk->Area0*=opt.emult[Chan];
    pk->Base*=opt.emult[Chan];

  }

}

//-----------------------------

void PulseClass::PeakAna33() {

  if (sData.size()<2)
    return;

  //return;
  Short_t T5; //left width window
  Short_t T6; //right width window

  Peaks.push_back(peak_type());
  peak_type *pk = &Peaks.back();
  //pk->Pos=crs->Pre[Chan];
  pk->Pos=cpar.preWr[Chan];

  UInt_t kk=opt.kdrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;
  int sz=sData.size()-1;
    
  Float_t sum;

  pk->B1=pk->Pos+opt.bkg1[Chan];
  pk->B2=pk->Pos+opt.bkg2[Chan];
  pk->P1=pk->Pos+opt.peak1[Chan];
  pk->P2=pk->Pos+opt.peak2[Chan];
  pk->T3=pk->Pos+opt.twin1[Chan];
  pk->T4=pk->Pos+opt.twin2[Chan];
  T5=pk->Pos+opt.wwin1[Chan];
  T6=pk->Pos+opt.wwin2[Chan];

  if (pk->B1<0) pk->B1=0;
  if (pk->B2<0) pk->B2=0;
  if (pk->P1<0) {pk->P1=0; pk->Type|=P_B1;}
  if (pk->P2<0) {pk->P2=0; pk->Type|=P_B2;}

  if (pk->B1>sz) pk->B1=sz;
  if (pk->B2>sz) pk->B2=sz;
  if (pk->P1>sz) {pk->P1=sz; pk->Type|=P_B1;}
  if (pk->P2>sz) {pk->P2=sz; pk->Type|=P_B2;}


  if (pk->T3<(int)kk) {pk->T3=kk; pk->Type|=P_B11;}
  if (pk->T3>sz) {pk->T3=sz; pk->Type|=P_B11;}
  if (pk->T4<(int)kk) {pk->T4=kk; pk->Type|=P_B11;}
  if (pk->T4>sz) {pk->T4=sz; pk->Type|=P_B11;}

  pk->Time=0;
  sum=0;
  for (int j=pk->T3;j<pk->T4;j++) {
    Float_t dif=sData[j]-sData[j-kk];
    pk->Time+=dif*j;
    sum+=dif;
    //nt++;
  }
  if (sum>1e-5)
    pk->Time/=sum;
  else
    pk->Time=-999;

  pk->Time-=pk->Pos;
  //cout << "Time: " << pk->Time << " " << sum << " " << pk->T3 << " " << pk->T4 << endl;

  if (T5<(int)kk) {T5=kk;}
  if (T5>sz) {T5=sz;}
  if (T6<(int)kk) {T6=kk;}
  if (T6>sz) {T6=sz;}

  pk->Width=0;
  sum=0;
  for (int j=T5;j<T6;j++) {
    Float_t dif=sData[j]-sData[j-kk];
    pk->Width+=dif*j;
    sum+=dif;
    //nt++;
  }
  if (sum>1e-5)
    pk->Width/=sum;
  else
    pk->Width=-999;

  pk->Width-=pk->Pos;


  pk->Base=0;
  int nbkg=0;
  //background
  //return;
  for (int j=pk->B1;j<pk->B2;j++) {
    pk->Base+=sData[j];
    nbkg++;
  }

  //cout << "Bkg: " << nbkg << " " << opt.bkg2[Chan]-opt.bkg1[Chan] << endl;

  if (nbkg)
    pk->Base/=nbkg;
  else {
    cout << "Error!!! Error!!! Error!!! Check it!!! zero background!!!: " << this->Tstamp64 << " " << nbkg << " " << pk->B1 << " " << pk->B2 << endl;
  }

  int nn=0;
  //peak Area & Height
  pk->Area0=0;
  pk->Height=0;
  for (int j=pk->P1;j<pk->P2;j++) {
    pk->Area0+=sData[j];
    if (sData[j]>pk->Height) pk->Height = sData[j];
    nn++;
  }
  if (nn) {
    pk->Area0/=nn;
  }
  else {
    cout << "zero Area: " << this->Tstamp64 << " " << pk->Pos << " " << pk->P1 << " " << pk->P2 << endl;
  }
  pk->Area=pk->Area0 - pk->Base;
  pk->Area*=opt.emult[Chan];
  pk->Area0*=opt.emult[Chan];
  pk->Base*=opt.emult[Chan];

  // printf(ANSI_COLOR_RED
  // 	 "Alp: %10lld %8.1f %8.1f %8.1f %8.1f %8.1f\n" ANSI_COLOR_RESET,
  // 	 Counter,Bg,Ar,Ht,Tm,Wd);
  // printf(ANSI_COLOR_GREEN
  // 	 "Kop: %10lld %8.1f %8.1f %8.1f %8.1f %8.1f\n" ANSI_COLOR_RESET,
  // 	 Counter,pk->Base,pk->Area0,pk->Height,pk->Time,Wd);

} //PeakAna33()

void PulseClass::CheckDSP() {
  if (Peaks.size()!=2) {
    cout <<"CheckDSP: Peaks.size()!=2" << Peaks.size() << endl;
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
	   "Alp: %10lld %8.1f %8.1f %8.1f %8.1f %8.1f\n" ANSI_COLOR_RESET,
	   Counter,Peaks[0].Base,Peaks[0].Area0,Peaks[0].Height,
	   Peaks[0].Time,Peaks[0].Width);
    printf(ANSI_COLOR_GREEN
	   "Kop: %10lld %8.1f %8.1f %8.1f %8.1f %8.1f\n" ANSI_COLOR_RESET,
	   Counter,Peaks[1].Base,Peaks[1].Area0,Peaks[1].Height,
	   Peaks[1].Time,Peaks[1].Width);
  }
  else {
    //printf("%10lld OK\n",Counter);
  }

  Peaks.pop_back();
  
}

EventClass::EventClass() {
  State=0;
  T=0;
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
    for (UInt_t j=0;j<cpar.durWr[i];j++) {
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

  if (pp->sData.size() != cpar.durWr[pls->Chan]) {
    pp->sData.resize(cpar.durWr[pls->Chan]);
    cout << "Pulse_Mean_Add: resize: " << (int) pls->Chan
	 << " " << pp->sData.size() << endl;     
  }

  if (pls->sData.size()  != cpar.durWr[pls->Chan]) {
    cout << "Error: " << (int) pls->Chan << " " << pls->Counter
	 << " " << pls->sData.size() << endl;
    return;
  }
  for (UInt_t j=0;j<cpar.durWr[pls->Chan];j++) {
    pp->sData[j]+=pls->sData[j];
  }

  pp->Counter++;
  //cout << "Pulse_Mean_Add2: " << (int) pls->Chan << endl; 

}
*/
//void EventClass::Pulse_Ana_Add(PulseClass *pls) {
void EventClass::Pulse_Ana_Add(pulse_vect::iterator pls) {

  // if (opt.b_pulse) {
  //   crs->mean_event.Pulse_Mean_Add(pls);
  // }
  
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
    if (T0<99998) //if T0 already exists, just adjust it
      T0+= T - pls->Tstamp64;
    T=pls->Tstamp64;
  }

  if (opt.Start[pls->Chan]) {
    if (pls->Peaks.size()) {
      Float_t dt = pls->Tstamp64 - T;

      peak_type *pk = &pls->Peaks.front();
      //Float_t T2 = pk->Time - crs->Pre[pls->Chan] + dt;
      //Float_t T2 = pk->Time - cpar.preWr[pls->Chan] + dt;
      Float_t T2 = pk->Time + dt;

      if (T2<T0) {
	T0=T2;
      }
      // cout << "Peak: " << Nevt << " " << (int) pls->Chan
      // 	   << " " << pk->Time << " " 
      // 	   << pk->Pos << " " << T0 << " " << T2 << " " << dt << " "
      // 	   << T << endl;
    }
  }

  // if (T>=149270116965358390 && T<=149270116965364203) {
  //   cout << "Evt: " << Nevt << ":";
  //   for (UInt_t i=0;i<pulses.size();i++) {
  //     cout << " " << (int)pulses.at(i).Chan<< "," << pulses.at(i).Tstamp64-crs->Tstart64;
  //   }
  //   cout << endl;
  // }
  
}

void EventClass::Fill_Time_Extend(HMap* map) {
  TH1F* hh = (TH1F*) map->hst;
  Double_t max = hh->GetXaxis()->GetXmax();

  if (opt.T_acq > max) {
    max*=2;
    if (opt.T_acq>max) {
      cout << "Time leap is too large: " << this->Nevt << " " << opt.T_acq << " " << max << " " << crs->Tstart64 << endl;
    }
    else {
      int nbin = hh->GetNbinsX()*2;
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
  if (first) {
    //cout << "fill: " << map->hst->GetName() << endl;
    map[ch]->hst->Fill(x);
    // cout << "xx: " << x << endl;
    // if (x>map[ch]->hst->GetXaxis()->GetXmin() &&
    // 	x<map[ch]->hst->GetXaxis()->GetXmax()) {
    //   map[ch]->hst->AddBinContent(x);
    // }
    if (opt.ncuts) {
      for (int i=0;i<opt.ncuts;i++) {
	if (getbit(*(map[ch]->cut_index),i)) {
	  if (x>=hcl->cutG[i]->GetX()[0] && x<hcl->cutG[i]->GetX()[1]) {
	    hcl->cut_flag[i]=1;
	    //cout << "Cut: " << Nevt << " " << i << " " << ch << " " << x << endl;
	  }
	}
      }
    }
  }
  else if (*(map[ch]->wrk)) {
    for (int i=0;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {
	//cout << "Fill1d: " << Nevt << " " << x << endl;
	map[ch]->h_cuts[i]->hst->Fill(x);
	// if (x>=map[ch]->h_cuts[i]->hst->GetXaxis()->GetXmin() &&
	//     x<map[ch]->h_cuts[i]->hst->GetXaxis()->GetXmax()) {
	//   map[ch]->h_cuts[i]->hst->AddBinContent(x);
	// }
      }
    }
    if (crs->cut_main) {
      //cout << "Fill1d: " << Nevt << " " << x << endl;
      map[ch]->h_MT->hst->Fill(x);
      // if (x>=map[ch]->h_MT->hst->GetXaxis()->GetXmin() &&
      // 	  x<map[ch]->h_MT->hst->GetXaxis()->GetXmax()) {
      // 	map[ch]->h_MT->hst->AddBinContent(x);
      // }
    }
  }

  // if (opt.Mrk[ch] && ch<MAX_CH) {
  //   Fill1d(first,map,MAX_CH,x);
  // }
  for (int j=0;j<NGRP;j++) {
    if (opt.Grp[ch][j] && ch<MAX_CH) {
      Fill1d(first,map,MAX_CH+j,x);
    }
  }
}

void EventClass::Fill_Mean1(TH1F* hh, PulseClass* pls, UInt_t nbins) {
  Float_t* arr = hh->GetArray();
  int nent=hh->GetEntries();
  for (UInt_t j=0;j<nbins;j++) {
    Float_t val = arr[j+1]*nent+pls->sData[j];
    arr[j+1]=val/(nent+1);
  }
  hh->SetEntries(nent+1);
  // cout << "Pulse_Mean: " << (int) pls->Chan << " "
  // 	 << map->hst->GetEntries() << endl;  
}

void EventClass::Fill_Mean_Pulse(Bool_t first, HMap* map, PulseClass* pls) {

  //HMap* map = hcl->m_pulse[n];
  int ch = pls->Chan;
  
  // if (pls->sData.size() < cpar.durWr[pls->Chan]) {
  //   cout << "Error: " << (int) pls->Chan << " " << pls->Counter
  // 	 << " " << pls->sData.size() << endl;
  //   return;
  // }

  if (first) {
    //cout << "fill_mean: " << (int) ch << " " << map->hst->GetNbinsX() << " " << pls->sData.size() << endl;
    int newsz = pls->sData.size();
    if (map->hst->GetNbinsX() < newsz) {
      map->hst->
	SetBins(pls->sData.size(),-cpar.preWr[ch],newsz-cpar.preWr[ch]);
      // cout << "Pulse_Mean: resize: " << (int) pls->Chan
      // 	   << " " << map->hst->GetNbinsX() << endl;     
    }

    Fill_Mean1((TH1F*)map->hst, pls, newsz);
    
  } //if first

  else if (*(map->wrk)) {
    for (int i=0;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i]) {

	if (map->h_cuts[i]->hst->GetNbinsX() != (int) cpar.durWr[ch]) {
	  map->h_cuts[i]->hst->
	    SetBins(cpar.durWr[ch],-cpar.preWr[ch],cpar.durWr[ch]-cpar.preWr[ch]);
	}

	Fill_Mean1((TH1F*)map->h_cuts[i]->hst, pls, cpar.durWr[ch]);

      }
    }
  }

}

void EventClass::Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y) {
  if (first) {
    map->hst->Fill(x,y);
    if (opt.ncuts) {
      for (int i=0;i<MAXCUTS;i++) {
	if (getbit(*(map->cut_index),i)) {
	  //cout << "cut: " << i << " " << icut << " " << hcl->cutG[icut] << endl;
	  if (hcl->cutG[i]->IsInside(x,y)) {
	    hcl->cut_flag[i]=1;
	  }
	}
      }
    }
  }
  else if (*(map->wrk)) {
    for (int i=0;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i])
	map->h_cuts[i]->hst->Fill(x,y);      
    }
  }
}

void EventClass::FillHist(Bool_t first) {
  double DT = crs->period*1e-9;
  //int ch[MAX_CH];
  Double_t tt;
  Double_t ee;
  //Double_t tt2;
  //Double_t max,max2;
  //int nbin;

  //int icut=0;
  int mult=0;
  Long64_t tm;

  int nn=0;
  Float_t area[2] = {0,0};

  //cout << "FillHist: " << this->Nevt << endl;

  if (first) {
    opt.T_acq=(T-crs->Tstart64)*DT;
    //cout << "T_acq: " << opt.T_acq << " " << T << endl;

    if (opt.Tstop && (opt.T_acq > opt.Tstop || opt.T_acq < opt.Tstart)) {
      if (crs->b_fana) {
	crs->b_stop=true;
	crs->b_fana=false;
	crs->b_run=0;
      }
      return;
    }

    if (opt.ncuts)
      memset(hcl->cut_flag,0,sizeof(hcl->cut_flag));
  }

  // if (opt.Tstop && (T-crs->Tstart64)*DT > opt.Tstop) {
  //   return;
  // }

  for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;

    if (opt.h_pulse.b) {
      Fill_Mean_Pulse(first,hcl->m_pulse[ch],&pulses[i]);
    }

    for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
      peak_type* pk = &pulses[i].Peaks[j];

      // if (first) {
      // 	if (opt.dec_write) {
      // 	  crs->rP.Area   = pk->Area  ;
      // 	  //crs->rP.Height = pk->Height;
      // 	  crs->rP.Width  = pk->Width ;
      // 	  crs->rP.Time   = pk->Time  ;
      // 	  crs->rP.Ch     = ch        ;
      // 	  crs->rP.Type   = pk->Type  ;

      // 	  crs->rPeaks.push_back(crs->rP);
      // 	}
      // }

      if (opt.elim2[ch]>0 &&
	  (pk->Area<opt.elim1[ch] || pk->Area>opt.elim2[ch])) {
	continue;
      }

      if (opt.h_time.b) {
	if (first) {
	  Fill_Time_Extend(hcl->m_time[ch]);
	}
	Fill1d(first,hcl->m_time,ch,opt.T_acq);
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

      if (opt.h_area_base.b) {
	Fill2d(first,hcl->m_area_base[ch],pk->Area,pk->Base);
      }

      if (opt.h_hei.b) {
	Fill1d(first,hcl->m_height,ch,pk->Height);
      }

      if (opt.h_tof.b) {
	double dt = pulses[i].Tstamp64 - T;
	//tt = pk->Time - crs->Pre[ch] - T0 + dt;
	//tt = pk->Time - cpar.preWr[ch] - T0 + dt;
	tt = pk->Time - T0 + dt;
	Fill1d(first,hcl->m_tof,ch,tt*crs->period);
      }

      if (j==0) { //only for the first peak
	//mtof
	if (opt.h_mtof.b || opt.h_etof.b) {
	  if (ch==opt.start_ch) {
	    crs->Tstart0 = pulses[i].Tstamp64 + pk->Pos;
	  }
	  if (opt.Mrk[ch]) {
	    mult++;
	  }
	  if (mult && (i==pulses.size()-1)) { //last pulse
	    if (crs->Tstart0>0) {
	      if (mult>=opt.Nchan) mult=opt.Nchan-1;

	      tm = pulses[i].Tstamp64 + pk->Pos;
	      tt = (tm - crs->Tstart0)*0.001*crs->period;
	      if (opt.mtof_period>0.01 && tt>opt.mtof_period) {
		crs->Tstart0+=1000*opt.mtof_period;
		tt = (tm - crs->Tstart0)*0.001*crs->period;
	      }

	      if (opt.h_mtof.b) {
		Fill1d(first,hcl->m_mtof,mult,tt);
		Fill1d(first,hcl->m_mtof,0,tt);
	      }
	      if (opt.h_etof.b) {
		ee = 72.298*opt.Flpath/(tt-opt.TofZero);
		ee= ee*ee;
		Fill1d(first,hcl->m_etof,mult,ee);
		Fill1d(first,hcl->m_etof,0,ee);
	      }
	    }
	  } //if last pulse
	} //if (opt.h_mtof.b || opt.h_etof.b)

	if (opt.h_a0a1.b) {
	  if (ch==0) {
	    area[0]=pk->Area;
	    nn++;
	  }
	  if (ch==1) {
	    area[1]=pk->Area;
	    nn++;
	  }
	  if (nn==2) {
	    Fill2d(first,hcl->m_a0a1[0],area[0],area[1]);
	    nn++;
	  }
	}

	if (opt.h_per.b) {
	  tm = pulses[i].Tstamp64 + pk->Pos;
	  if (hcl->T_prev[ch]) {
	    tt = (tm - hcl->T_prev[ch])*0.001*crs->period; //convert to mks
	    Fill1d(first,hcl->m_per,ch,tt);
	  }
	  hcl->T_prev[ch]=tm;
	}
      } // if (j==0) only for 1st peak

    } //for peaks...

  } //for (UInt_t i=0;i<pulses.size()...

  /*
    if (opt.dec_write) {
    crs->rTime=T;
    crs->rState = State;
    crs->Tree->Fill();
    crs->rPeaks.clear();
    }
  */

  /*
    int ax=0,ay=0,px=0,py=0;
    if (pulses.size()==4) {
    for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;
    if (ch<8) { //prof_x
    px=PROF::prof_ch[ch];
    }
    else if (ch<16) { //prof y
    py=PROF::prof_ch[ch];
    }
    else if (ch<24) { //alpha y
    ay=PROF::prof_ch[ch];
    }
    else { //alpha x
    ax=PROF::prof_ch[ch];
    }
    }

    int ch_alpha = ax + ay*8;

    //cout << "prof: " << crs->nevents << " " << ch_alpha << endl;

    hcl->h2_prof_strip[ch_alpha]->Fill(px,py);
    hcl->h2_prof_real[ch_alpha]->Fill(px*15,py*15);    
    }
  */

  if (first) {
    if (hcl->b_formula) {
      for (int i=0;i<opt.ncuts;i++) {
	if (opt.pcuts[i]==1) {//formula
	  hcl->cut_flag[i]=hcl->cform[i]->EvalPar(0,hcl->cut_flag);
	}
	//cout << "cut_flag: " << Nevt << " " << i << " " << hcl->cut_flag[i] << endl;
      }
    }
    if (crs->b_maintrig) {
      crs->cut_main = crs->maintrig.EvalPar(0,hcl->cut_flag);
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
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64-crs->Tstart64);
  if (pdata) {
    for (int i=0;i<(int)sData.size();i++) {
      printf("-- %d %f\n",i,sData[i]);
    }
  }
}
