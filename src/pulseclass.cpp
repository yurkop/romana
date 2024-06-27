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
  //Time=0; //не знаю, почему был 0?
  Time=99999;
  //Width=88888;
}

//size_t PulseClass::GetPtr(Mdef* it) {
size_t PulseClass::GetPtr(Int_t hnum) {
  size_t ptr=0;
  switch (hnum) {
  case 1: //Area
    ptr = (char*)&(this->Area) - (char*)this;
    //cout << "area_ptrnum: " << ptr << endl;
    break;
  case 2: //Height
    ptr = (char*)&(this->Height) - (char*)this;
    break;
  case 3: //Width
    ptr = (char*)&(this->Width) - (char*)this;
    break;
  case 4: //Base
    ptr = (char*)&(this->Base) - (char*)this;
    break;
  case 5: //Sl1
    ptr = (char*)&(this->Sl1) - (char*)this;
    break;
  case 6: //Sl2
    ptr = (char*)&(this->Sl2) - (char*)this;
    break;
  case 7: //RMS1 - base
    ptr = (char*)&(this->RMS1) - (char*)this;
    break;
  case 8: //RMS2 - peak
    ptr = (char*)&(this->RMS2) - (char*)this;
    break;
  }
  if (ptr==0) {
    prnt("ss ds;",BRED, "Wrong hnum number in GetPtr:", hnum, RST);
    exit(-1);
  }
  return ptr;
}

Float_t PulseClass::CFD(int j, int kk, int delay, Float_t frac, Float_t &drv) {
  // возвращает CFD и одновременно drv в точке j

  // if (j+delay >= sData.size()) {
  //   prnt("ssd ds;",BRED,"CFD: ",j,delay,RST);  
  //   return 0;
  // }
  //else {

  Float_t d0 = sData[j+delay] - sData[j-kk+delay];
  drv = sData[j] - sData[j-kk];
  return drv*frac-d0;
  //}
}

void PulseClass::FindPeaks(Int_t sTrig, Int_t kk) {
  //Находим только первый пик

  //sTg: 0 - hreshold crossing of pulse;
  //      1 - threshold crossing of derivative;
  //      2 - максимум производной (первый локальный максимум)
  //      3 - rise of derivative;
  //      4 - fall of derivative;
  //      5 - threshold crossing of derivative, use 2nd deriv for timing.
  //      6 - fall of derivative, zero crossing
  //      7 - CFD, zero crossing

  //Pos=-32222;

  //Float_t* D = new Float_t[sData.size()]();
  Float_t D[20000]; //максимальная длина записи 16379
  Float_t Dpr;

  //cout << sizeof(Float_t)*sData.size() <<endl;
  //memset (D,0,sizeof(Float_t)*sData.size());
  D[0]=0;
  UInt_t j;
  UInt_t pp=0; // временный Pos

  switch (sTrig) {
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
  case 7: // для CFD триггер 1 используется для поиска Pos
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] >= opt.sThr[Chan]) {
	Pos=j;
	//Peaks.push_back(pk);
	break;
      }
    }
    break;
  case 4: // fall of derivative - сначала ищем превышение порога
  case 6: // fall of derivative, zero crossing - сначала ищем превышение порога
    {
      bool p2=false;
      for (j=kk;j<sData.size();j++) {
	D[j]=sData[j]-sData[j-kk];
	if (p2) {
	  if (D[j] < cpar.LT[Chan]) {
	    Pos=j-1;
	    break;
	  }
	}
	else if (D[j] >= opt.sThr[Chan]) {
	  p2=true;
	}
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
    //int pp=0;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] > cpar.LT[Chan] && Dpr<=cpar.LT[Chan]) {
	pp=j;
      }
      if (D[j] >= opt.sThr[Chan]) {
	Pos=pp;
	//Peaks.push_back(pk);
	break;
      }
      // else {
      //    break;
      // }
      Dpr=D[j];
    } //case 3
    break;
    /*
  case 7: {// CFD
    Pos=cpar.Pre[Chan];
    int delay = abs(opt.T1[Chan]);
    Dpr=-1e6;
    Float_t drv;

    //kk+delay всегда >=1
    //jj=kk+delay-1;
    //D[jj]=0;

    //for (j=kk+delay;j<sData.size();j++) {
    for (j=kk;j<sData.size()-delay;j++) {
      D[j] = CFD(j,kk,delay,opt.T2[Chan],drv);
      if (Dpr >= opt.sThr[Chan] && drv<Dpr) {
	delay=-1;
	break;
      }
      else if (D[j] <= 0) {
	//else -> значит это условие не может быть выполнено на том же шаге,
	//что и предыдущий if. Т.е. если pp задано, есть как минимум 2 точки
	pp=j;
      }
      Dpr=drv;
    }

    if (delay==-1) { //если выполнено, то pp и pp+1 существуют
      //Pos=pp;
      if (D[pp+1]!=D[pp])
	Time = pp - D[pp]/(D[pp+1] - D[pp]);
      else
	Time = pp;
    }
    break;
  } //CFD
    */
  default:
    break;
  }

  //delete[] D;

} //FindPeaks

void PulseClass::FindZero(Int_t kk) {
  // определяем точное пересечение нуля или LT (нижнего порога)
  // Pos - уже найден; kk - параметр производной
  // для Trig=4 Pos - точка ПЕРЕД пересечением нуля

  Float_t D[20000]; //максимальная длина записи 16379
  Float_t Dpr;
  UInt_t pp=0; // временный Pos

  switch (opt.sTg[Chan]) {
  case 3: //rise of derivalive
    if (Pos-kk>=1 && Pos<(int)sData.size()) {
      Float_t DD=sData[Pos]-sData[Pos-kk];
      Float_t Dpr=sData[Pos-1]-sData[Pos-kk-1];
      if (DD!=Dpr)
	Time = Pos-1 + (cpar.LT[Chan] - Dpr)/(DD-Dpr);
      else
	Time = Pos;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime2:",Chan,Tstamp64,RST);
    }
    break;
  case 6:
    if (Pos>=kk && Pos+1<(int)sData.size()) {
      Float_t Dpr=sData[Pos]-sData[Pos-kk];
      Float_t DD=sData[Pos+1]-sData[Pos-kk+1];
      if (DD!=Dpr)
	Time = Pos + (cpar.LT[Chan] - Dpr)/(DD-Dpr);
      else
	Time = Pos;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime3:",Chan,Tstamp64,RST);
    }
    break;
  case 7: {
    int delay = abs(opt.T1[Chan]);
    Dpr=-1e6;
    Float_t drv;

    //kk+delay всегда >=1
    //jj=kk+delay-1;
    //D[jj]=0;

    //for (j=kk+delay;j<sData.size();j++) {
    for (UInt_t j=kk;j<sData.size()-delay;j++) {
      D[j] = CFD(j,kk,delay,opt.T2[Chan],drv);
      if (Dpr >= opt.sThr[Chan] && drv<Dpr) {
	delay=-1;
	break;
      }
      else if (D[j] <= 0) {
	//else -> значит это условие не может быть выполнено на том же шаге,
	//что и предыдущий if. Т.е. если pp задано, есть как минимум 2 точки
	pp=j;
      }
      Dpr=drv;
    }

    if (delay==-1) { //если выполнено, то pp и pp+1 существуют
      //Pos=pp;
      if (D[pp+1]!=D[pp])
	Time = pp - D[pp]/(D[pp+1] - D[pp]);
      else
	Time = pp;
      //cout << "pp: " << Tstamp64 << " " << pp << " " << Time << " " << Pos << endl;
    }
  }
    break;
  default:
    break;
  }
    
  //prnt("ss d l d d fs;",BGRN,"Zero:",Chan,Tstamp64,cpar.Pre[Chan],Pos,Time,RST);

  /*
  Float_t Dpr=-1e6;
  for (UInt_t j=j0;j<sData.size();j++) {
    Float_t DD=sData[j]-sData[j-kk];
    if (DD <= 0 && Dpr > 0) {
      Pos=j-1;
      if (sTrig==6) { //zero crossing
	if (DD!=Dpr)
	  Time = Pos - Dpr/(DD-Dpr);
	else
	  Time = Pos;
      }
      break;
    }
    Dpr=DD;
  }
  */
  //prnt("ssfs;",BGRN,"Zero: ",Time,RST);  
}

//-----------------------------

void PulseClass::PeakAna33() {

  Float_t Area0=0; //integral of pulse without Base subtraction
  Float_t Area2=0; //integral of 1st deriv without Base subtraction

  Short_t B1; //left background window (included)
  Short_t B2; //right background window (included)
  Short_t P1; //left peak window (included)
  Short_t P2; //right peak window (included)
  //Short_t T1; //left zero crossing of deriv
  //Short_t T2; //right zero crossing of deriv
  Short_t T1; //left timing window (included)
  Short_t T2; //right timing window (included)
  Short_t W1; //left width window (included)
  Short_t W2; //right width window (included)

  Float_t xm,S_xx;
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
    if (sData.size()>1) { // нужно минимум 2 точки
      if (kk<1 || kk>=(int)sData.size()) kk=1;
      FindPeaks(opt.sTg[Chan],kk);
    } //if
  }
  else {//use hardware trigger
    Pos=cpar.Pre[Chan];
  }

  //prnt("ssl d ls;","P33: ",BYEL,Tstamp64,Pos,sData.size(),RST);

  if (Pos<=-32222) {// нет пиков
    return;
  }

  //pk=&Peaks.back();

  B1=Pos+opt.Base1[Chan];
  B2=Pos+opt.Base2[Chan];
  P1=Pos+opt.Peak1[Chan];
  P2=Pos+opt.Peak2[Chan];
  T1=Pos+opt.T1[Chan];
  T2=Pos+opt.T2[Chan];
  W1=Pos+opt.W1[Chan];
  W2=Pos+opt.W2[Chan];

  if (B1<0) B1=0;
  if (B2<=B1) B2=B1; //base can NOT be zero if B2==B1
  if (P1<0) P1=0;
  if (P2<=P1) P2=P1;
  if (T1<(int)kk) T1=kk;
  if (T2<=T1) T2=T1;
  if (W1<0) W1=0;
  if (W2<=W1) W2=W1;

  if (B1>=sz) B1=sz-1;
  if (B2>=sz) B2=sz-1;
  if (P1>=sz) P1=sz-1;
  if (P2>=sz) P2=sz-1;
  if (T1>=sz) T1=sz-1;
  if (T2>=sz) T2=sz-1;
  if (W1>=sz) W1=sz-1;
  if (W2>=sz) W2=sz-1;

  switch (opt.sTg[Chan]) {
  case 3:
  case 6:
  case 7:
    FindZero(kk);
    break;
    //case 7:
    //break;
  default: {
    Time=0;

    if (crs->use_2nd_deriv[Chan]) { //use 2nd deriv
      // 05.10.2020
      for (int j=T1;j<=T2;j++) {
	if (j<kk+1)
	  continue;
	Float_t dif2=sData[j]-sData[j-kk]-sData[j-1]+sData[j-kk-1];
	if (dif2>0) {
	  Time+=dif2*j;
	  Area2+=dif2;
	}
      }
    }
    else { //use 1st deriv
      for (int j=T1;j<=T2;j++) {
	if (j<kk)
	  continue;
	Float_t dif=sData[j]-sData[j-kk];
	if (dif>0) {
	  Time+=dif*j;
	  Area2+=dif;
	}
      }
    }

    if (abs(Area2)>1e-5) {
      Time/=Area2;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime1:",Chan,Tstamp64,RST);
      Time=(T1+T2)*0.5;
    }
  } //default
  } //switch

  /*
  if (opt.sTg[Chan]<6) { //not zero crossing
    Time=0;

    if (crs->use_2nd_deriv[Chan]) { //use 2nd deriv
      // 05.10.2020
      for (int j=T1;j<=T2;j++) {
	if (j<kk+1)
	  continue;
	Float_t dif2=sData[j]-sData[j-kk]-sData[j-1]+sData[j-kk-1];
	if (dif2>0) {
	  Time+=dif2*j;
	  Area2+=dif2;
	}
      }
    }
    else { //use 1st deriv
      for (int j=T1;j<=T2;j++) {
	if (j<kk)
	  continue;
	Float_t dif=sData[j]-sData[j-kk];
	if (dif>0) {
	  Time+=dif*j;
	  Area2+=dif;
	}
      }
    }

    if (abs(Area2)>1e-5) {
      Time/=Area2;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ssfs;",BGRN,"ErrTime1: ",sum,RST);
      Time=(T1+T2)*0.5;
    }
  } // if not zero crossing
  */

  //prnt("ss d l d d fs;",BYEL,"Peakana33:",Chan,Tstamp64,cpar.Pre[Chan],Pos,Time,RST);
  Time-=cpar.Pre[Chan];
  //Time-=Pos;

  //prnt("ss d l d d fs;",BGRN,"Peakana33:",Chan,Tstamp64,cpar.Pre[Chan],Pos,Time,RST);

  //pk->Time=sum;

  //cout << "Width2: " << (int) Chan << " " << Tstamp64 << " " << pk->Width2 << endl;
  //pk->Width+=0.1;

  //baseline
  Base=0;
  int nbkg=0;
  for (int j=B1;j<=B2;j++) {
    Base+=sData[j];
    nbkg++;
  }

  if (nbkg)
    Base/=nbkg;
  else {
    ++crs->errors[ER_BASE];
  }

  int nn=0;
  //peak Area & Height
  //Area0=0;
  Height=-99999;
  for (int j=P1;j<=P2;j++) {
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

  Width=0;
  nn=0;
  for (int j=W1;j<=W2;j++) {
    Width+=sData[j];
    nn++;
  }
  if (nn) {
    Width/=nn;
  }
  else {
    ++crs->errors[ER_WIDTH];
  }

  //if (hcl->b_base || opt.Mt[Chan]==2) {
  if (hcl->b_base[Chan]) {
    //slope1 (baseline)
    Sl1=0;
    S_xx = 0;
    xm = (B2+B1)*0.5; //mean x
    //nbkg=0;
    for (int j=B1;j<=B2;j++) {
      Float_t xx = j-xm;
      Sl1+=xx*(sData[j]-Base);
      S_xx+=xx*xx;
      //nbkg++;
    }
    if (S_xx) {
      Sl1/=S_xx;
    }

    //prnt("ss fs;",BRED,"Sl1:",Sl1,RST);

    if (opt.Mt[Chan]==3) { //альтернативный slope1 (W1-W2) вместо slope2
      Sl2 = 2*(Base - Width)/(B1+B2-W1-W2);
      RMS2 = 0;
    }
    else { //slope2 (peak)
      Sl2=0;
      S_xx = 0;
      xm = (P2+P1)*0.5; //mean x
      //nbkg=0;
      for (int j=P1;j<=P2;j++) {
	Float_t xx = j-xm;
	Sl2+=xx*(sData[j]-Area0);
	S_xx+=xx*xx;
	//nbkg++;
      }
      if (S_xx) {
	Sl2/=S_xx;
      }

      //RMS2 (peak)
      RMS2=0;
      nbkg=0;
      for (int j=P1;j<=P2;j++) {
	Float_t Yj = Area0+(j-(P1+P2)*0.5)*Sl2 - sData[j];
	RMS2+=Yj*Yj;
	nbkg++;
	//cout << "jjjj: " << j << " " << Base << " " << sData[j] << " " << Yj << endl;
      }
      if (nbkg) {
	RMS2 = sqrt(RMS2/nbkg);
      }

    } //else (slope2)
    //prnt("ss fs;",BGRN,"Sl2:",Sl2,RST);

    //RMS1 (baseline)
    RMS1=0;
    nbkg=0;
    for (int j=B1;j<=B2;j++) {
      Float_t Yj = Base+(j-(B1+B2)*0.5)*Sl1 - sData[j];
      RMS1+=Yj*Yj;
      nbkg++;
      //cout << "jjjj: " << j << " " << Base << " " << sData[j] << " " << Yj << endl;
    }
    if (nbkg) {
      RMS1 = sqrt(RMS1/nbkg);
    }

  } //if b_base

  if (opt.Mt[Chan]==2) {
    Area=Area0 - Base - ((P2+P1)-(B1+B2))*0.5*Sl1;
  }
  else if (opt.Mt[Chan]==3) {
    Area=Area0 - Base - ((P2+P1)-(B1+B2))*0.5*Sl2;
  }
  else {
    Area=Area0 - Base;
  }

  /* YK
  if (opt.Bc[Chan]) {
    Area+=opt.Bc[Chan]*Base;
  }
  YK */

  /*
  //cout << "W1: " << W1 << " " << W2 << " " << Pos << endl;
  if (W1<=Pos && W2<=Pos) {
    if (Base)
      Width/=Base;
    else
      Width=0;
  }
  else {
  */
  if (opt.Mt[Chan]!=3) {
    if (Area) {
      Width-=Base;
      Width/=Area;
    }
    else
      Width=0;
  }
  else { // для Mt=3 W=P-T
    Width = Pos-cpar.Pre[Chan]-Time;
    if (Width<-400) Width=-400;
    if (Width>400) Width=400;
  }



  if (opt.Mt[Chan]==1) {
    Area=Area2;
  }

  /*
  //Simul2
  if (Pos<1 || Pos+1>=sz) {
    Simul2=-99999;
  }
  else {
    Float_t y1=sData[Pos]-sData[Pos-kk];
    Float_t y2=sData[Pos+1]-sData[Pos+1-kk];
    if (y2<y1)
      Simul2= Pos - y1/(y2-y1);
    else
      Simul2=Pos+0.5;
  }
  Simul2-=cpar.Pre[Chan];//+opt.SimSim[9]/opt.Period;

  //Time=(Time+Simul2)/2;

  //Simul2-=Pos;

  //Time = Simul2;

  //prnt("ss l d f fs;",KGRN,"Simul2:",Tstamp64,Pos,Simul2,Time,RST);
  if (Tstamp64 < 10000)
    prnt("ss d l f f f f f f d ds;",BGRN,"pk:",Chan,Tstamp64,Area,Area0,Base,Sl1,Sl2,Width,opt.Mt[Chan],(int)hcl->b_base[Chan],RST);
  */

} //PeakAna33()

void PulseClass::Ecalibr(Float_t& XX) {
  switch (opt.calibr_t[Chan]) {
  case 2:
    XX=opt.E0[Chan] + opt.E1[Chan]*XX + opt.E2[Chan]*XX*XX;
    break;
  case 1:
    XX=opt.E0[Chan] + opt.E1[Chan]*XX;
    break;
  default:
    break;
  }
}

// void PulseClass::Bcalibr() {
//   switch (opt.calibr_t[Chan]) {
//   case 2:
//   case 1:
//     Base*=opt.E1[Chan];
//     Sl1*=opt.E1[Chan];
//     Sl2*=opt.E1[Chan];
//     RMS1*=opt.E1[Chan];
//     RMS2*=opt.E1[Chan];
//     break;
//   default:
//     break;
//   }
// }

EventClass::EventClass() {
  Spin=0;
  //Tstmp=-9999999999;
  T0=99999;
  //Analyzed=false;
}

void EventClass::AddPulse(PulseClass *pls) {
  // добавляет PulseClass *pls в текущее событие;
  // проверяет вето, если вето -> pls не добавляется
  // определяет Spin
  // определяет T0
  // поправляет pls->Time

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
    //pls->Simul2-=i_dt;
  }
  else {
    pls->Time+=pls->Tstamp64 - Tstmp;
    //pls->Simul2+=pls->Tstamp64 - Tstmp;
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

}

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
