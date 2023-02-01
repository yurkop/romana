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

PulseClass::PulseClass() {
  Pos=-32222; // -> no peak found:
              // Area, Time, Width не имеют смысла
  //ptype=P_NOSTOP;
  ptype=0;
  Spin=0;

  //Tstamp64=0;
  //Area=0;
  Time=0;
  //Time=99999;
  //Width=88888;
}

void PulseClass::FindPeaks() {
  //Находим только первый пик

  //sTg: 0 - hreshold crossing of pulse;
  //      1 - threshold crossing of derivative;
  //      2 - maximum of derivative;
  //      3 - rise of derivative;
  //      4 - fall of derivative;
  //      5 - threshold crossing of derivative, use 2nd deriv for timing.

  //Pos=-32222;

  if (sData.size()<2) {
    return;
  }

  UInt_t kk=opt.sDrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

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
	//Peaks.push_back(pk);
	break;
      }
    }
    break;
  case 1: // threshold crossing of derivative;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] >= opt.sThr[Chan]) {
	Pos=j;
	//Peaks.push_back(pk);
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
	//Peaks.push_back(pk);
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
	//Peaks.push_back(pk);
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
	//Peaks.push_back(pk);
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
  // }

  //prnt("sl d l d;","P33: ",Tstamp64,Pos,sData.size(),opt.sTg[Chan]);

  int sz=sData.size();
  Int_t kk=opt.sDrv[Chan];
  if (kk<1 || kk>=sz-1) kk=1;

  if (sData.size()<=2) {
    return;
  }

  if (opt.sTg[Chan]>=0) { //use software trigger
    FindPeaks();
  }
  else {//use hardware trigger
    Pos=cpar.Pre[Chan];
  }

  //prnt("ssl d ls;","P33: ",BYEL,Tstamp64,Pos,sData.size(),RST);

  if (Pos<=-32222) {// нет пиков
    return;
  }


  //pk=&Peaks.back();
  Float_t sum;

  B1=Pos+opt.Base1[Chan];
  B2=Pos+opt.Base2[Chan]+1;
  P1=Pos+opt.Peak1[Chan];
  P2=Pos+opt.Peak2[Chan]+1;
  T1=Pos+opt.T1[Chan];
  T2=Pos+opt.T2[Chan]+1;
  W1=Pos+opt.W1[Chan];
  W2=Pos+opt.W2[Chan]+1;

  if (B1<0) B1=0;
  //if (B2<=B1) B2=B1+1;
  if (B2<=B1) B2=B1; //base can be zero if B2==B1
  if (P1<0) P1=0;
  if (P2<=P1) P2=P1+1;

  if (B1>=sz) B1=sz-1;
  if (B2>sz) B2=sz;
  if (P1>=sz) P1=sz-1;
  if (P2>sz) P2=sz;

  if (T1<(int)kk) T1=kk;
  if (T2<=T1) T2=T1+1;
  if (W1<(int)kk) W1=kk;
  if (W2<=W1) W2=W1+1;

  if (T1>sz) T1=sz-1;
  if (T2>sz) T2=sz;
  if (W1>sz) W1=sz-1;
  if (W2>sz) W2=sz;

  Time=0;
  sum=0;

  if (crs->use_2nd_deriv[Chan]) { //use 2nd deriv
    // 05.10.2020
    for (int j=T1;j<T2;j++) {
      if (j<kk+1)
	continue;
      Float_t dif2=sData[j]-sData[j-kk]-sData[j-1]+sData[j-kk-1];
      if (dif2>0) {
	Time+=dif2*j;
	sum+=dif2;
      }
    }
  }
  else { //use 1st deriv
    for (int j=T1;j<T2;j++) {
      if (j<kk)
	continue;
      Float_t dif=sData[j]-sData[j-kk];
      if (dif>0) {
	Time+=dif*j;
	sum+=dif;
      }
    }
  }
  if (abs(sum)>1e-5) {
    Time/=sum;
  }
  else {
    ++crs->errors[ER_TIME];
    //prnt("ssfs;",BGRN,"ErrTime1: ",sum,RST);
    Time=(T1+T2)*0.5;
  }

  Time-=cpar.Pre[Chan];

  //cout << "Peakana33: " << (int) Chan << " " << Tstamp64 << " " << Time << endl;

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
  Base=0;
  int nbkg=0;
  for (int j=B1;j<B2;j++) {
    Base+=sData[j];
    nbkg++;
  }

  if (nbkg)
    Base/=nbkg;
  else {
    ++crs->errors[ER_BASE];
  }

  //prnt("ssl d d d fs;",BBLU,"Base: ",Tstamp64,B1,B2,nbkg,Base,RST);

  int nn=0;
  //peak Area & Height
  Area0=0;
  Height=-99999;
  for (int j=P1;j<P2;j++) {
    Area0+=sData[j];
    if (sData[j]>Height) Height = sData[j];
    nn++;
  }
  if (nn) {
    Area0/=nn;
  }
  else {
    ++crs->errors[ER_AREA];
    //cout << "zero Area: " << this->Tstamp64 << " " << Pos << " " << P1 << " " << P2 << endl;
  }

  //calibration
  Area=Area0 - Base;
  //pk->Area*=opt.emult[Chan];
  //Area=opt.E0[Chan] + opt.E1[Chan]*Area;

  if (opt.Bc[Chan]) {
    Area+=opt.Bc[Chan]*Base;
  }

  //width === area2
  Width=0;
  if (Area) {
    nn=0;
    for (int j=W1;j<W2;j++) {
      Width+=sData[j];
      nn++;
    }
    if (nn) {
      Width/=nn;
    }
    else {
      ++crs->errors[ER_WIDTH];
      //cout << "zero Width: " << this->Tstamp64 << " " << Pos << " " << P1 << " " << P2 << endl;
    }
  	
    Width-=Base;
    //YK
    //Width=opt.E0[Chan] + opt.E1[Chan]*Width;
    Width/=Area;
  }

  //slope1 (baseline)
  Slope1=0;
  nbkg=0;
  for (int j=B1+1;j<B2;j++) {
    Slope1+=sData[j]-sData[j-1];
    nbkg++;
  }
  if (nbkg)
    Slope1/=nbkg;

  //slope2 (peak)
  /*
  Slope2=0;
  nbkg=0;
  for (int j=P1+1;j<P2;j++) {
    Slope2+=sData[j]-sData[j-1];
    nbkg++;
  }
  if (nbkg)
    Slope2/=nbkg;
  */

  //Simul
  if (Pos<1 || Pos+1>=sz) {
    Simul=-99999;
  }
  else {
    Float_t y1=sData[Pos]-sData[Pos-kk];
    Float_t y2=sData[Pos+1]-sData[Pos+1-kk];
    if (y2<y1)
      Simul= Pos - y1/(y2-y1);
    else
      Simul=Pos+0.5;
  }
  Simul-=cpar.Pre[Chan]+1;

  //prnt("ss l d f fs;",BGRN,"Simul:",Tstamp64,Pos,Simul,Time,RST);

} //PeakAna33()

void PulseClass::CheckDSP() {
  /*
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
  */
}

void PulseClass::Ecalibr() {
  switch (opt.calibr_t[Chan]) {
  case 2:
    Area=opt.E0[Chan] + opt.E1[Chan]*Area + opt.E2[Chan]*Area*Area;
    break;
  case 1:
    Area=opt.E0[Chan] + opt.E1[Chan]*Area;
    break;
  default:
    break;
  }
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
void EventClass::AddPulse(PulseClass *pls) {
  // добавляет PulseClass *pls в текущее событие;
  // проверяет вето, если вето -> pls не добавляется
  // определяет Spin
  // определяет T0
  // поправляет pls->Time

  
  //void EventClass::AddPulse(pulse_vect::iterator pls) {

  // if (opt.b_pulse) {
  //   crs->mean_event.Pulse_Mean_Add(pls);
  // }
  //Float_t dt;
  //Long64_t dt;

  //prnt("ss d f l l fs;",BGRN,"PAA0:",pls->Chan,pls->Time,Nevt,Tstmp,T0,RST);

  for (UInt_t i=0;i<pulses.size();i++) { //reject identical pulses
    if (pls->Chan == pulses[i].Chan &&
	TMath::Abs(pls->Tstamp64-pulses[i].Tstamp64) < opt.tveto) {
      return;
    }
  }

  if (opt.Ms[pls->Chan]) {
    Spin|=2;
  }

  //if (pls->Spin & 1) {
  //Spin|=1;
  //}
  Spin|=(pls->Spin & 129); //копируем bit7 и bit0 to event spin

  if (pulses.empty()) { //this is the first pulse in the event
    int i_dt = opt.sD[pls->Chan]/opt.Period;
    Tstmp=pls->Tstamp64+i_dt;
    pls->Time-=i_dt;
    //Tstmp=pls->Tstamp64;
  }
  else {
    pls->Time+=pls->Tstamp64 - Tstmp;
  }

  pulses.push_back(*pls);

  if (opt.St[pls->Chan] && pls->Pos>-32222) {
    Float_t T1 = pls->Time+opt.sD[pls->Chan]/opt.Period;
    if (T1<T0) {
      T0=T1;
    }
    // if (pls->Time<T0) {
    //   T0=pls->Time;
    // }
  }

  //prnt("ss d f l l fs;",BRED,"PAA1:",pls->Chan,pls->Time,Nevt,Tstmp,T0,RST);
}

void EventClass::Fill_Time_Extend(HMap* map, void* hd) {
  if (!map) return;

  TH1F* hh = (TH1F*) map->hst;
  Double_t max = hh->GetXaxis()->GetXmax();

  //prnt("ssl l f fs;",BGRN,"T_acq: ",Nevt,Tstmp,opt.T_acq,max,RST);

  if (opt.T_acq > max) {
    //prnt("ssl l fs;",BYEL,"T_acq: ",Nevt,Tstmp,opt.T_acq-max,RST);
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

      //hh->SetBins(nbin,0,max);


      //Float_t* arr = new Float_t[nbin+2];
      //memset(arr,0,sizeof(Float_t)*(nbin+2));

      TIter next(hcl->allmap_list);
      TObject* obj;
      while ( (obj=(TObject*)next()) ) {
	HMap* map2 = (HMap*) obj;
	//if (map2->hd==&opt.h_rate) {
	if (map2->hd==hd) {
	  TH1F* hh2 = (TH1F*) map2->hst;
	  hh2->SetBins(nbin,0,max);
	}
      }

      /*
      TIter next(hcl->allmap_list);
      TObject* obj;
      while ( (obj=(TObject*)next()) ) {
	HMap* map2 = (HMap*) obj;
	if (map2->hd==&opt.h_rate) {
	  //cout << "map2: " << map2->GetName() << " " << map2->GetTitle()
	  //     << " " << (map2->hd==&opt.h_rate) << endl;
	  TH1F* hh2 = (TH1F*) map2->hst;
	  Float_t* arr2 = hh2->GetArray();
	  memcpy(arr,arr2,hh2->GetSize()*sizeof(Float_t));
	  hh2->SetBins(nbin,0,max);
	  hh2->Adopt(nbin+2,arr);
	}
      }
      */


      /*
      for (int i=0;i<MAX_CH+NGRP;i++) {
	TH1F* hh2 = (TH1F*) hcl->m_rate[i]->hst;
	Float_t* arr2 = hh2->GetArray();
	memcpy(arr,arr2,hh2->GetSize()*sizeof(Float_t));
	hh2->SetBins(nbin,0,max);
	hh2->Adopt(nbin+2,arr);
      }
      */


    }
  }
}

void EventClass::Fill1dw(Bool_t first, HMap* map[], int ch, Float_t x,
			 Double_t w) {
  // базовый метод для заполнения 1-гистограмм:
  // без проверки на существование и без заполнения групп

  // if first -> заполняем гистограмму, проверяем cuts и устанавливаем cut_flag
  if (first) {
    map[ch]->hst->Fill(x,w);
    if (opt.ncuts) {
      for (int i=1;i<opt.ncuts;i++) {
	if (getbit(*(map[ch]->hd->cut+map[ch]->nn),i)) {
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

}

void EventClass::Fill1d(Bool_t first, HMap* map[], int ch, Float_t x) {
  if (!map[ch]) return;

  Fill1dw(first,map,ch,x);

  if (ch<MAX_CH)
    for (int j=0;j<NGRP;j++)
      if (opt.Grp[ch][j])
        Fill1dw(first,map,MAX_CH+j,x);
}

void EventClass::Fill_Mean1(TH1F* hh, Float_t* Data, Int_t nbins, int ideriv) {
  Float_t zz=0;
  Float_t* arr = hh->GetArray();
  Double_t* w2 = hh->GetSumw2()->GetArray();
  int nent=hh->GetEntries();
  int min = TMath::Min(hh->fN-2,nbins);
  // cout << "Fill_mean: " << hh->fN << " " << nbins << " " << min
  //      << " " << hh->GetSumw2() << " " << hh->ClassName() << endl;
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
void EventClass::Fill_Mean_Pulse(Bool_t first, HMap* map,
				 pulse_vect::iterator pls, int ideriv) {
  if (!map) return;

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
  if (!map) return;
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
  int mult[NGRP+1];
  Long64_t tm;

  //prnt("ss l l fs;",BRED,"evnt:",Nevt,Tstmp,T0,RST);


  Float_t AA[MAX_CH+1] = {}; //initialize to zero
  //int ch_alpha=-1;

  // инициализация
  if (first) {  	
    opt.T_acq=(Tstmp/*- crs->Tstart64*/)*DT;

    if (opt.Tstop) {
      if (opt.T_acq > opt.Tstop) {
	if (crs->b_fana) {
	  crs->b_stop=true;
	  crs->b_fana=false;
	  crs->b_run=0;
	}
	return;
      }
      else if (opt.T_acq < opt.Tstart) {
	return;
      }
    }

    if (opt.ncuts) {
      memset(hcl->cut_flag,0,sizeof(hcl->cut_flag));
    }
  }

  // обработка сигналов Старт/счетчиков
  if (Spin & 128) { //Start channel
    if (first) {
      if (opt.start_ch==255) {
	//crs->Tstart0 = Tstmp + Long64_t(ipls->Time);
	crs->Tstart0 = Tstmp;
	crs->Time0 = 0;
      }

      if (opt.h_hwrate.b) {
	for (pulse_vect::iterator ipls=pulses.begin();ipls!=pulses.end();
	     ++ipls) {
	  int ch = ipls->Chan;

	  if (first) {
	    Fill_Time_Extend(hcl->m_hwrate[ch],&opt.h_hwrate);
	  }
	  if (ch<opt.Nchan) {
	    //cout << ch << " " << ipls->Counter << endl;
	    Fill1dw(first,hcl->m_hwrate,ch,opt.T_acq,ipls->Counter);
	    // if (ch==0) {
	    //   prnt("ss d ls;",BGRN,"cntr: ",ch,ipls->Counter,RST);
	    // }
	    //Fill1d(first,hcl->m_hwrate,ch,opt.T_acq);
	  }
	}
      }

    }
    return;
  }

  memset(mult,0,sizeof(mult));

  for (pulse_vect::iterator ipls=pulses.begin();ipls!=pulses.end();++ipls) {
    //cout << "pulse: " << Nevt << " " << Tstmp << " " << (int) ipls->Chan << " " << ipls->Pos << endl;
    if (ipls->Pos<=-32222) // пропускаем импульсы, где не найден пик
      continue;

    //ipls->Ecalibr();
    //for (UInt_t i=0;i<pulses.size();i++) {
    int ch = ipls->Chan;


    mult[NGRP]++;
    for (int j=0;j<NGRP;j++)
      if (opt.Grp[ch][j])
	mult[j]++;


    if (opt.h_pulse.b) {
      Fill_Mean_Pulse(first,hcl->m_pulse[ch],ipls,0);
    }

    if (opt.h_deriv.b) {
      Fill_Mean_Pulse(first,hcl->m_deriv[ch],ipls,1);
    }

    // if (opt.elim2[ch]>0 &&
    //      (pk->Area<opt.elim1[ch] || pk->Area>opt.elim2[ch])) {
    //    continue;
    // }

    if (opt.h_rate.b) {
      if (first) {
	Fill_Time_Extend(hcl->m_rate[ch],&opt.h_rate);
      }
      Fill1d(first,hcl->m_rate,ch,opt.T_acq);
    }

    if (opt.h_area.b) {
      Fill1d(first,hcl->m_area,ch,ipls->Area);
    }

    if (opt.h_area0.b) {
      Fill1d(first,hcl->m_area0,ch,ipls->Area0);
    }

    if (opt.h_base.b) {
      Fill1d(first,hcl->m_base,ch,ipls->Base);
    }

    if (opt.h_slope1.b) {
      Fill1d(first,hcl->m_slope1,ch,ipls->Slope1);
    }

    // if (opt.h_slope2.b) {
    //   Fill1d(first,hcl->m_slope2,ch,ipls->Slope2);
    // }
    if (opt.h_simul.b) {
      Double_t tt2 = ipls->Simul - T0;
      Fill1d(first,hcl->m_simul,ch,tt2*opt.Period+opt.sD[ch]);
    }

    if (opt.h_width.b) {
      Fill1d(first,hcl->m_width,ch,ipls->Width);
    }

    if (opt.h_area_base.b) {
      Fill2d(first,hcl->m_area_base[ch],ipls->Area,ipls->Base);
    }

    if (opt.h_area_sl1.b) {
      Fill2d(first,hcl->m_area_sl1[ch],ipls->Area,ipls->Slope1);
    }

    // if (opt.h_area_sl2.b) {
    //   Fill2d(first,hcl->m_area_sl2[ch],ipls->Area,ipls->Slope2);
    // }

    // if (opt.h_slope_12.b) {
    //   Fill2d(first,hcl->m_slope_12[ch],ipls->Slope1,ipls->Slope2);
    // }

    if (opt.h_time_simul.b) {
      tt = ipls->Time - T0;
      Double_t tt2 = ipls->Simul - T0;
      Fill2d(first,hcl->m_time_simul[ch],tt*opt.Period+opt.sD[ch],
	     tt2*opt.Period+opt.sD[ch]);
    }

    if (opt.h_area_time.b) {
      Fill2d(first,hcl->m_area_time[ch],ipls->Area,opt.T_acq);
    }

    if (opt.h_area_width.b) {
      Fill2d(first,hcl->m_area_width[ch],ipls->Area,ipls->Width);
    }

    if (opt.h_hei.b) {
      Fill1d(first,hcl->m_height,ch,ipls->Height);
    }

    if (opt.h_time.b && T0!=99999) {
      //double dt = pulses[i].Tstamp64 - Tstmp;
      //tt = Time - T0 + dt;
      tt = ipls->Time - T0;
      Fill1d(first,hcl->m_time,ch,tt*opt.Period+opt.sD[ch]);
    }

    //ntof
    if (opt.h_ntof.b || opt.h_etof.b || opt.h_ltof.b) {
      // определяем старт
      if (ch==opt.start_ch) {
	//crs->Tstart0 = Tstmp + Long64_t(ipls->Time);
	crs->Tstart0 = Tstmp;
	crs->Time0 = ipls->Time;
      }
      if (crs->Tstart0>0) {
	//tm = Tstmp + Long64_t(ipls->Time);
	//tt = (tm - crs->Tstart0)*mks*opt.Period;

	tt = Tstmp - crs->Tstart0;
	tt+= ipls->Time - crs->Time0;
	tt*= mks*opt.Period;

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
	    Fill2d(first,hcl->m_area_ntof[ch],ipls->Area,tt);
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
      AA[ch]=ipls->Area;
    }

    if (opt.h_per.b) {
      //tm = pulses[i].Tstamp64 + Long64_t(Time);
      tm = Tstmp + Long64_t(ipls->Time);
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
  } //for (pulse_vect::iterator ipls=pulses.begin()...

  if (opt.h_mult.b) {
    Fill1dw(first,hcl->m_mult,0,mult[NGRP]);
      for (int j=0;j<NGRP;j++)
	if (hcl->m_mult[MAX_CH+j])
	  Fill1dw(first,hcl->m_mult,MAX_CH+j,mult[j]);
  }

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

  //Fill variables for Profilometer (new or old)
  if (first && opt.h_prof.b) { //Profilometer

    if (opt.h_prof64.b) { //new profilometer
      memset(hcl->h_sum,0,sizeof(hcl->h_sum));
      int ax=999,ay=999;
      //int px=999,py=999;

      for (UInt_t i=0;i<pulses.size();i++) {
	Int_t pp = crs->prof_ch[pulses[i].Chan];
	int h_xy=-1;
	int h_off=1;
	int sgn=1;
	if (pp>=crs->PROF_64) {
	  pp-=crs->PROF_64;
	  switch (pp) {
	  case 0: //P+(33-64) (X)
	    h_xy=0;
	    //h_off=33;
	    h_off=62;
	    sgn=-1;
	    break;
	  case 1: //P+(1-32) (X)
	    h_xy=0;
	    break;
	  case 2: //N+(33-64) (Y)
	    h_xy=1;
	    break;
	  case 3: //N+(1-32) (Y)
	    h_xy=1;
	    h_off=62;
	    sgn=-1;
	    break;
	  case 4:
	    // do nothing
	    break;
	  default:
	    cout << "wrong prof channel: " << pp << " " << pulses[i].Chan << endl;
	  } //switch

	  if (h_xy>=0) {//one of Prof64 position channels
	    PulseClass *pulse = &pulses[i];  
	    int dt=(pulse->Tstamp64-Tstmp) - cpar.Pre[pulse->Chan] - T0;
	    int size = pulse->sData.size();

	    for (int kk=-1;kk<31;kk++) {
	      int jj=h_off+kk*sgn;
	      int x1 = opt.Prof64_W[1] + opt.Prof64_W[0]*kk - dt;
	      int x2 = x1 + opt.Prof64_W[2];
	      int xmin = TMath::Max(x1,0);
	      int xmax = TMath::Min(x2,size);
	      if (xmax-xmin>0) {
		for (int j=xmin;j<xmax;j++) {
		  hcl->h_sum[h_xy][jj]+=pulse->sData[j];
		}
		hcl->h_sum[h_xy][jj]*=-1.0/(xmax-xmin);
		// if (kk==1) {
		//   prnt("sd d fs;",BRED,Nevt,pulse->Chan,hcl->h_sum[h_xy][h_off+kk],RST);
		// }
	      }
	    } //for kk
	  } //if xy>0  
	} //if pp>=crs->PROF_64

	else if (pp>=crs->ING_Y) {
	  tt = (pulses[i].Time - T0)*opt.Period+opt.sD[pulses[i].Chan];
	  if (abs(tt)<=opt.Prof64_GAT) {
	    ay=pp-crs->ING_Y;
	    //Fill1d(first,hcl->m_time,pulses[i].Chan-16,tt);
	  }
	}
	else if (pp>=crs->ING_X) {
	  tt = (pulses[i].Time - T0)*opt.Period+opt.sD[pulses[i].Chan];
	  if (abs(tt)<=opt.Prof64_GAT) {
	    ax=pp-crs->ING_X;
	    //Fill1d(first,hcl->m_time,pulses[i].Chan-16,tt);
	  }
	}
	// else if (pp>=crs->PROF_Y)
	// 	py=pp-crs->PROF_Y;
	// else if (pp>=crs->PROF_X)
	// 	px=pp-crs->PROF_X;

      } //for i pulses.size()

      // for (int i=0;i<64;i++) {
      // 	cout << "sum: " << Nevt << " " << i << " " << hcl->h_sum[0][i]
      // 	     << " " << hcl->h_sum[1][i] << endl;
      // }

      hcl->ch_alpha = ax + (opt.prof_ny-ay-1)*opt.prof_ny;

      // if (hcl->ch_alpha>=0 && hcl->ch_alpha<opt.prof_ny*opt.prof_nx) {
      //   if (ax==3) {
      // 	prnt("ss l d d ds;",BRED,"Prof:",Nevt,ax,ay,hcl->ch_alpha,RST);
      //   }
      // }

    } //if (opt.h_prof64.b) //new profilometer
    else { //old profilometer
    }
  } //if (first && opt.h_prof.b) //Profilometer


    // profilometer 2d histograms (new)
    if (opt.h_prof.b) {

      if (hcl->ch_alpha>=0 && hcl->ch_alpha<opt.prof_ny*opt.prof_nx) {
	for (int i=0;i<64;i++) {
	  if (hcl->h_sum[0][i]>opt.Prof64_THR) {
	    for (int j=0;j<64;j++) {
	      if (hcl->h_sum[1][j]>opt.Prof64_THR) {
		Fill2d(first,hcl->m_prof[hcl->ch_alpha],(i+0.5)*1.875,(j+0.5)*1.875);
		//Fill2d(first,hcl->m_prof[hcl->ch_alpha],(i+0.5),(j+0.5));
	      } //if Y
	    }
	  } //if X
	}
      }

    }

    if (opt.h_prof64.b) { //histograms for new 64x64 profilometer

      Fill_Mean1((TH1F*)hcl->m_prof64[2]->hst, hcl->h_sum[0], 64, 0); //X
      Fill_Mean1((TH1F*)hcl->m_prof64[3]->hst, hcl->h_sum[1], 64, 0); //Y

      for (int i=0;i<64;i++) {
	if (hcl->h_sum[0][i]>opt.Prof64_THR) {
	  //prnt("ss3d f ds;",BRED,"X:",i,hcl->h_sum[0][i],opt.Prof64_THR,RST);
	  Fill1d(first,hcl->m_prof64,0,i+0.5);
	  for (int j=0;j<64;j++) {
	    if (hcl->h_sum[1][j]>opt.Prof64_THR) {
	      //cout << "xy: " << i << " " << j << endl;
	      Fill2d(first,hcl->m_prof64[4],i+0.5,j+0.5);
	      Fill2d(first,hcl->m_prof64[5],(i+0.5)*1.875,(j+0.5)*1.875);
	    } //if Y
	  }
	} //if X
      }

      for (int i=0;i<64;i++) {
	if (hcl->h_sum[1][i]>opt.Prof64_THR) {
	  //prnt("ss3d f ds;",BGRN,"Y:",i,hcl->h_sum[1][i],opt.Prof64_THR,RST);
	  Fill1d(first,hcl->m_prof64,1,i+0.5);
	} //if Y
      }

    } //if h_prof.b

  // "formula cuts"
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

/*
void PulseClass::Smooth_old(int nn) {

  int n2=abs(nn-1);
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
*/

void PulseClass::Smooth(int nn) {

  int n2=abs(nn)-1;
  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

  int Nsamp = sData.size();

  for (int i=Nsamp-1;i>0;i--) {
    //int ll=1;
    for (int j=1;j<=n2;j++) {
      int k=i-j;
      if (k>0) {
	sData[i]+=sData[k];
	//ll++;
      }
    }

    int ndiv;
    if (i <= n2) {
      ndiv=i;
    }
    else {
      ndiv=n2+1;
    }

    //if (i<10) {
    //cout << "ll: " << i << " " << n2 << " " << ll << " " << ndiv << endl;
    //}

    sData[i]/=ndiv;
    if (nn<0)
      sData[i] = roundf(sData[i]);
    //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.sS, ndiv, sData[i]);

  }

}


void PulseClass::PrintPulse(int pdata) {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64/*-crs->Tstart64*/);
  if (pdata) {
    for (int i=0;i<(int)sData.size();i++) {
      printf("-- %d %f\n",i,sData[i]);
    }
  }
}

void EventClass::PrintEvent(bool pls) {
  printf("Event: %8lld %10lld %10.2f %2ld\n",Nevt,Tstmp,T0,pulses.size());
  if (pls)
    for (UInt_t i=0;i<pulses.size();i++) {
      printf("  %3d %lld %10.2f\n",pulses[i].Chan,pulses[i].Tstamp64,pulses[i].Time);
    }
}

