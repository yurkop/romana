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
  case 9: //RiseTime
    ptr = (char*)&(this->Rtime) - (char*)this;
    break;
  default:
    prnt("ss ds;",BRED, "Wrong hnum number in GetPtr:", hnum, RST);
    EExit(-1);
  }
  // if (ptr==0) {
  //   prnt("ss ds;",BRED, "Wrong hnum number in GetPtr:", hnum, RST);
  //   exit(-1);
  // }
  //prnt("ss ds;",BGRN, "GetPtr:", hnum, RST);
  return ptr;
}

Float_t PulseClass::CFD(int j, int kk, int delay, Float_t frac, Float_t &drv) {
  // возвращает CFD и одновременно drv в точке j
  // CFD = drv[j] - dev[j+delay]*frac
  // CFD = производная минус производная, сдвинутая влево на delay
  //       и умноженная на frac

  // kk>0; delay>=0.
  // CFD существует в диапазоне:
  // - начальная точка (>=): kk
  // - конечная точка (<):   sData.size()-delay
  // Пример: for (UInt_t j=kk;j<sData.size()-delay;j++)

  // CFD сдвинута вправо относительно drv

  Float_t d0 = sData[j+delay] - sData[j-kk+delay]; //d0=drv[j+delay]
  drv = sData[j] - sData[j-kk];
  //old -> return drv*frac-d0;


  return drv-d0*frac*0.1; // -> original


  //return drv*frac-d0;
  //d0 = sData[j-delay] - sData[j-kk-delay];
  //return d0*frac - drv;



  //}
}

void PulseClass::FindPeaks(Int_t sTrig, Int_t kk, Float_t &cfd_frac) {
  //Находим только первый пик

  //sTg:  0 - hreshold crossing of pulse;
  //      1 - threshold crossing of derivative;
  //      2 - максимум производной (первый локальный максимум)
  //      3 - rise of derivative;
  //      4 - fall of derivative;
  //      5 - threshold crossing of derivative, use 2nd deriv for timing.
  //      6 - fall of derivative, zero crossing
  //      7 - CFD from deriv, zero crossing;

  //      8 - не работает: CFD from pulse, zero crossing; Pos==3 - rise of deriv
  //      9 - не работает: CFD, fraction; Pos==3 - rise of deriv

  //if (sTrig==7) sTrig = cpar.Trg[Chan]; //для sTrig7 используется cpar.Trig

  //Pos=-32222;

  //Float_t* D = new Float_t[sData.size()]();
  Float_t D[20000]; //максимальная длина записи 16379
  Float_t Dpr;

  //cout << sizeof(Float_t)*sData.size() <<endl;
  //memset (D,0,sizeof(Float_t)*sData.size());
  D[0]=0;
  UInt_t j;
  UInt_t pp=kk; // временный Pos

  switch (sTrig) {
  case 0: // threshold crossing of pulse
    for (j=0;j<sData.size();j++) {
      if (sData[j] > opt.sThr[Chan]) {
	Pos=j;
	//Peaks.push_back(pk);
	break;
      }
    }
    break;
  case 1: // threshold crossing of derivative;
    //case 7: // для sTrig7 используется cpar.Trig
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] > opt.sThr[Chan]) {
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
	else if (D[j] > opt.sThr[Chan]) {
	  p2=true;
	}
      }
    }
    break;
  case 2: // maximum of derivative; (первый локальный максимум после порога)
  case 5: // or fall of 2nd derivative (it's the same)
    Dpr=-1e6;
    //int jpr;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (Dpr > opt.sThr[Chan] && D[j]<Dpr) {
	Pos=j-1;
	//Peaks.push_back(pk);
	break;
      }
      Dpr=D[j];
    }
    break;
  case 3: // rise of derivative;
    //case 7: // CFD from deriv
    //case 8: // CFD from pulse
    Dpr=1;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (D[j] > cpar.LT[Chan] && Dpr<=cpar.LT[Chan]) {
	pp=j;
      }
      if (D[j] > opt.sThr[Chan]) {
	Pos=pp;
	//Peaks.push_back(pk);
	break;
      }
      Dpr=D[j];
    } //for
    break; //case 3 //,7,8

  case 7: {// CFD from deriv;
    Dpr=1;
    Float_t drv;
    for (j=kk;j<sData.size()-opt.DD[Chan];j++) {
      D[j]=CFD(j,kk,opt.DD[Chan],opt.FF[Chan],drv);
      //D[j]=sData[j]-sData[j-kk];
      if (D[j] > 0 && Dpr<=0) {
	pp=j;
      }
      if (D[j] > opt.sThr[Chan]) {
	Pos=pp;
	//Peaks.push_back(pk);
	break;
      }
      Dpr=D[j];
    } //for
    //cout << "Pos: " << Chan << " " << Pos << endl;
  } break; //case 7



    /*
  case 9: {// CFD fraction - POS: точка пересечения Max*frac находится
           // между Pos и Pos+1
    //Float_t max_frac= -99999;//opt.T2[Chan]*0.1;
    cfd_frac= -99999;
    //сначала находим первый локальный максимум после порога
    Dpr=-1e6;
    //int jpr;
    for (j=kk;j<sData.size();j++) {
      D[j]=sData[j]-sData[j-kk];
      if (Dpr > opt.sThr[Chan] && D[j]<Dpr) {
	Pos=j-1;
	cfd_frac=Dpr*opt.T2[Chan]*0.1; //Max*frac
	//Peaks.push_back(pk);
	break;
      }
      Dpr=D[j];
    }

    if (Pos!=-32222) { //пик найден
      //находим Pos
      do {
	Pos--;
	if (D[Pos]<=cfd_frac) {
	  break;
	}
      } while (Pos>kk);

      // for (int j=Pos-1;j>=kk;j--) {
      // 	if (D[j]<=max_frac) {
      // 	  Pos=j+1;
      // 	  break;
      // 	}
      // }
      
    }
    break;
  } //case 9
*/





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

void PulseClass::FindZero(Int_t kk, Int_t thresh, Float_t LT) {
  // определяем точное пересечение нуля или LT (нижнего порога)
  // Pos - уже найден; kk - параметр производной
  // для Trig=4 Pos - точка ПЕРЕД пересечением нуля
  // результат записывается в PulseClass::Time

  //cout << "stg: " << Tstamp64 << " " << opt.sTg[Chan] << endl;
  int stg = opt.sTg[Chan];
  if (stg<0) stg=cpar.Trg[Chan];
  switch (stg) {
  case 3: //rise of derivalive
    if (Pos > kk && Pos < (int)sData.size()) {
      Float_t DD=sData[Pos]-sData[Pos-kk];
      Float_t Dpr=sData[Pos-1]-sData[Pos-kk-1];
      if (DD!=Dpr)
	Time = Pos-1 + (LT - Dpr)/(DD-Dpr);
      else
	Time = Pos;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime2:",Chan,Tstamp64,RST);
    }
    break;
  case 6: //fall of derivalive
    // case 9: //CFD fraction
    if (Pos>=kk && Pos+1<(int)sData.size()) {
      Float_t Dpr=sData[Pos]-sData[Pos-kk];
      Float_t DD=sData[Pos+1]-sData[Pos-kk+1];
      if (DD!=Dpr)
	Time = Pos + (LT - Dpr)/(DD-Dpr);
      else
	Time = Pos;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime3:",Chan,Tstamp64,RST);
    } break;
  case 7: {
    if (Pos > kk && Pos < (int)sData.size() - opt.DD[Chan]) {
      Float_t drv;
      Float_t DD=CFD(Pos,kk,opt.DD[Chan],opt.FF[Chan],drv);
      Float_t Dpr=CFD(Pos-1,kk,opt.DD[Chan],opt.FF[Chan],drv);
      if (DD!=Dpr)
	Time = Pos-1 - Dpr/(DD-Dpr);
      else
	Time = Pos;
    }
    else
      ++crs->errors[ER_TIME];
  } break;
  case 77: { //old case 7
    // error (bin in Width spectrum):
    // 80 - не достигнут порог (thresh) - ошибка некритичная и допустимая
    // 85 - не найдено пересечение с 0 - возможно, допустимо
    // 89 - CFD всегда отрицательно - маловероятно и наверное недопустимо

    //int delay = abs(opt.T1[Chan]);
    if (kk >= (int)sData.size()-opt.DD[Chan]) {
      Pos=-32222;
      ++crs->errors[ER_TIME];
      break;
    }

    Float_t D[20000]; //максимальная длина записи 16379
    Int_t pp=kk; // временный Pos
    float max=-1e9;
    Float_t drv;
    Float_t Dpr = -1e6;
    bool porog=false;

    //sData.erase(sData.end()-10,sData.end());

    // CFD сдвинута вправо относительно drv
    // находим максимум - либо локальный после пересечения порога,
    // либо глобальный: pp - позиция максимума
    Int_t j;
    for (j=TMath::Max(kk,(int)Pos);j<(int)sData.size()-opt.DD[Chan];j++) {
      D[j]=CFD(j,kk,opt.DD[Chan],opt.FF[Chan],drv);
      //drv=sData[j]-sData[j-kk];
      if (D[j]>max) {
	max=D[j];
	pp=j;
      }
      if (drv>=thresh)
	porog=true;
      if (porog && D[j]>0 && D[j]<Dpr)
       	break;
      Dpr=D[j];
    }

    // если первая точка CFD<=0 -> error 89
    D[pp]=CFD(pp,kk,opt.DD[Chan],opt.FF[Chan],drv);
    if (D[pp]<=0) {
      //prnt("ss l d f fs;",BRED, "D<0:", Tstamp64, pp, D[pp], Pos-cpar.Pre[Chan]-Time, RST);
      Time=pp;
      Pos=Time+89;
      break;
    }

    // ищем пересечение с нулем
    pp--;
    while (pp>kk) {
      D[pp]=CFD(pp,kk,opt.DD[Chan],opt.FF[Chan],drv);
      if (D[pp]<=0) {
	break;
      }
      pp--;
    }

    // если пересечение не найдено -> error 85
    if (pp==kk) {
      Time=pp-1;
      Pos=Time+85;
      //prnt("ss l d d d fs;",BBLU, "Pos:", Tstamp64, pp, kk, sData.size(), Pos-cpar.Pre[Chan]-Time, RST);
      break;
    }
    //else
    //prnt("ss l d d ds;",BGRN, "Pos:", Tstamp64, pp, kk, sData.size(), RST);

    if (D[pp+1] == D[pp]) {
      prnt("ss d fs;",BRED,"DD:",Chan,D[pp],RST);
      Time = pp;
    }
    else
      Time = pp - D[pp]/(D[pp+1] - D[pp]);
    //cout << "pp: " << Tstamp64 << " " << pp << " " << Time << " " << Pos << endl;

    // если не достигнут порог -> error +80
    if (!porog) {
      Pos=Time+80;
    }

  } //case 7
    break;
  // case 8: {
  // }
  //   break;
  default:
    break;
  }
    
  //prnt("ssfs;",BYEL,"Zero: ",Time,RST);
}

//-----------------------------

void PulseClass::PeakAna33() {

  //Base,Height,Area,Width определяются относительно Pos

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
  Float_t wdth=0; //temporary Width
  Float_t cfd_frac=0;

  //prnt("sl d l d;","P33: ",Tstamp64,Pos,sData.size(),opt.sTg[Chan]);

  int sz=sData.size();
  Int_t kk=opt.sDrv[Chan];
  if (kk<1) kk=1;
  if (kk>=sz-1) kk=sz-1;

  if (sData.size()<=2) {
    return;
  }

  int stg = opt.sTg[Chan];

  if (stg>=0) { //use software trigger
    if (sData.size()>1) { // нужно минимум 2 точки
      if (kk<1 || kk>=(int)sData.size()) kk=1;
      FindPeaks(stg,kk,cfd_frac);
    } //if
  }
  else {//use hardware trigger
    stg=cpar.Trg[Chan];
    Pos=cpar.Pre[Chan];
  }

  //prnt("ssl d ls;","P33: ",BYEL,Tstamp64,Pos,sData.size(),RST);

  if (Pos<=-32222) {// нет пиков
    return;
  }

  //pk=&Peaks.back();

  B1=Pos+opt.B1[Chan];
  B2=Pos+opt.B2[Chan];
  P1=Pos+opt.P1[Chan];
  P2=Pos+opt.P2[Chan];
  T1=Pos+opt.T1[Chan];
  T2=Pos+opt.T2[Chan];
  W1=Pos+opt.W1[Chan];
  W2=Pos+opt.W2[Chan];

  if (B1<0) B1=0;
  if (P1<0) P1=0;
  if (T1<kk) T1=kk;
  if (W1<0) W1=0;

  if (B1>=sz) B1=sz-1;
  if (B2>=sz) B2=sz-1;
  if (P1>=sz) P1=sz-1;
  if (P2>=sz) P2=sz-1;
  if (T1>=sz) T1=sz-1;
  if (T2>=sz) T2=sz-1;
  if (W1>=sz) W1=sz-1;
  if (W2>=sz) W2=sz-1;

  if (B2<=B1) B2=B1; //base can NOT be zero if B2==B1
  if (P2<=P1) P2=P1;
  if (T2<=T1) T2=T1;
  if (W2<=W1) W2=W1;


  // 1. находим baseline
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

  // 2. поправляем весь импульс на PoleZero
  if (opt.Pz[Chan]>0) {
    PoleZero(opt.Pz[Chan]);
  }

  // 3. находим Area0 & Height
  int nn=0;
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

  // 4. находим wdth (интеграл от W1 до W2)
  nn=0;
  for (int j=W1;j<=W2;j++) {
    wdth+=sData[j];
    nn++;
  }
  if (nn) {
    wdth/=nn;
  }
  else {
    ++crs->errors[ER_WIDTH];
  }

  if (hcl->b_base[Chan]) {
    // 5. определяем slope1 (baseline)
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

    // if (opt.Mt[Chan]==3) { //альтернативный slope1 (W1-W2) вместо slope2
    //   Sl2 = 2*(Base - wdth)/(B1+B2-W1-W2);
    //   RMS2 = 0;
    // }
    if (opt.Mt[Chan]!=3) { //если Mt=3, см. ниже
      // 6. определяем slope2 (peak)
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

      // 7. определяем RMS2 (peak)
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

    } // если Mt не 3
    //prnt("ss fs;",BGRN,"Sl2:",Sl2,RST);

    // 8. определяем RMS1 (baseline)
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



  // 9. определяем Time и Area2
  Float_t LT=cpar.LT[Chan];
  switch (stg) {
    //case 9:
    //LT = cfd_frac;
  case 3:
  case 6:
  case 7:
    // FindZero определяет Time
    FindZero(kk,opt.sThr[Chan],LT);
    if (opt.Mt[Chan]==1) {
      for (int j=T1;j<=T2;j++) {
	if (j<kk)
	  continue;
	Area2+=sData[j]-sData[j-kk];
      }
    }
    break;
    //case 7:
    //break;
  default: { //0,1,2,4,5
    Time=0;

    if (crs->use_2nd_deriv[Chan]) { //use 2nd deriv
      // 05.10.2020
      for (int j=T1;j<=T2;j++) {
	if (j<kk+1)
	  continue;
	Float_t dif2=sData[j]-sData[j-kk]-sData[j-1]+sData[j-kk-1];
	//if (dif2>0) {
	Time+=dif2*j;
	Area2+=dif2;
	//}
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

    if (Area2) {
      Time/=Area2;
    }
    else {
      ++crs->errors[ER_TIME];
      //prnt("ss d ls;",BGRN,"ErrTime1:",Chan,Tstamp64,RST);
      Time=(T1+T2)*0.5;
    }
  } //default
  } //switch

  Time-=cpar.Pre[Chan];

  //prnt("ss d l d d fs;",BYEL,"Peakana33:",Chan,Tstamp64,cpar.Pre[Chan],Pos,Time,RST);


  //prnt("ss d l d d fs;",BGRN,"Peakana33:",Chan,Tstamp64,cpar.Pre[Chan],Pos,Time,RST);


  switch (opt.Mt[Chan]) {
  case 1: //Deriv
    Area=Area2;
    break;
  case 2: //Slope1 from linear regression
    Area = Area0 - Base - ((P1+P2)-(B1+B2))*0.5*Sl1;
    break;
  case 3: {//Slope2 from two areas
    int nnn = (B1+B2)-(W1+W2);
    if (nnn)
      Sl2 = 2*(Base - wdth)/nnn;
    else
      Sl2=0;
    RMS2 = 0;
    Area = Area0 - Base - ((P1+P2)-(B1+B2))*0.5*Sl2;
    break;
  }
  default: //case 0
    Area = Area0 - Base;
  }

  if (opt.Mt[Chan]!=3) {
    if (Area) {
      Width=wdth-Base;
      Width/=Area;
    }
    else
      Width=0;
  }
  else { // для Mt=3 W=P-T
    Width = Pos-cpar.Pre[Chan]-Time;
    if (Width<-99) Width=-99;
    if (Width>99) Width=99;
  }


  // 9. определяем RTime

  {
    //int j=Pos+Time-opt.DD[Chan];
    int j=Pos-opt.DD[Chan];
    if (j<kk) j=kk;
    Rtime=0;
    Float_t ss=0;
    Float_t dif;
    // от Pos-delay до порога
    do {
      dif=sData[j]-sData[j-kk];
      if (dif>0) {
	Rtime+=dif*j;
	ss+=dif;
      }
      j++;
    } while(dif<=opt.sThr[Chan] && j<(int)sData.size());

    // от порога до нижнего порога
    do {
      dif=sData[j]-sData[j-kk];
      Rtime+=dif*j;
      ss+=dif;
      j++;
    } while(dif>cpar.LT[Chan] && j<(int)sData.size());

    if (ss)
      Rtime/=ss;
    Rtime-=Pos;

    Float_t zz=0;
    if (opt.Pz[Chan]<0) {
      zz = 1 + 1.0/opt.Pz[Chan]*Rtime;
      Area*=zz;
    }
    //prnt("ss l d f f fs;",BRED,"RT:",Tstamp64,Pos,Time,Rtime,zz,RST);

  }
  
  /*
    for (int j=T1;j<=T2;j++) {
    if (j<kk)
    continue;
    Float_t dif=sData[j]-sData[j-kk];
    //if (dif>0) {
    Time+=dif*j;
    Area2+=dif;
    //}
    }
  */

  // if (Chan==16) {
  //   prnt("ss d l f f f f f f d ds;",BGRN,"pk:",Chan,Tstamp64,Area,Area0,Base,Sl1,Sl2,Width,opt.Mt[Chan],(int)hcl->b_base[Chan],RST);
  // }

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

//EventClass::EventClass() {
  //Spin=0;
  //Tstmp=-9999999999;
  //T0=99999;
  //Analyzed=false;
//}

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
    Spin|=64;
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

  if (opt.St[pls->Chan] && pls->Pos>-32222) {
    Float_t T7 = pls->Time+opt.sD[pls->Chan]/opt.Period;
    if (T7<T0) {
      T0=T7;
      ChT0=pls->Chan;
    }
  }

  pulses.push_back(*pls);

}

void PulseClass::Smooth(int nn) {

  //int n2=abs(nn)-1;
  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

  Float_t D[20000];
  Float_t sum=0;
  int len=0;

  for (int i=0;i<(int)sData.size();i++) {
    sum+=sData[i];
    len++;

    if (len>nn) {
      len--;
      sum-=sData[i-len];
    }

    // if (Tstamp64==0) {
    //   prnt("s d d d f f f;","Sm:",i,nn,len,sData[i],sum,sum/len);
    // }
    D[i]=sum/len;
  }
  //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.sS, ndiv, sData[i]);

  memcpy(&sData[0],D,sizeof(int)*sData.size());
}

void PulseClass::Smooth_hw(int nn) {

  //Int_t sum=nn;
  //if (sum<=0) sum=1;

  Int_t kk=nn-1;
  //UInt_t div=0;
  Float_t pp=2;
  while (kk >>= 1) {
    //div++;
    pp*=2;
  }
  //div++;
  //pp*=2;


  Float_t rr = nn/pp;
  //cout << "sum: " << nn << " " << pp << " " << rr << endl;


  Float_t D[20000];
  Float_t sum=0;
  int len=0;

  for (int i=0;i<(int)sData.size();i++) {
    sum+=sData[i];
    len++;

    if (len>nn) {
      len--;
      sum-=sData[i-len];
    }

    D[i]=int(sum/len*rr); 

    // if (Tstamp64==0) {
    //   prnt("s d d d f f f f;","Sm:",i,nn,len,sData[i],sum,sum/len*rr,D[i]);
    // }
 }
  //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.sS, ndiv, sData[i]);

  memcpy(&sData[0],D,sizeof(int)*sData.size());
}

/*
void PulseClass::Smooth_old(int nn) {

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
*/

void PulseClass::PoleZero(int Pz) {
  if (sData.size()<2) return;

  Float_t cor = 1. - 1./Pz;
  //cout << "t64: " << Tstamp64 << " " << Pz << " " << cor << endl;

  Float_t x0,x1 = sData[0]-Base;
  //x1=x[k-1] - предыдущая точка без поправки, но с вычтенным фоном
  for (UInt_t i=1;i<sData.size();i++) {
    x0 = sData[i];
    //if (i>2000) {
    Float_t dd = sData[i-1]-x1*cor; //dd=x'[k-1]-x[k-1]*cor
      // if (Tstamp64==2778120) {
      // 	cout << "pz: " << i << " " << sData[i] << " " << sData[i-1] << " " << x0
      // 	     << " " << x1 << " " << dd << endl;
      // }
    sData[i]+=dd-Base;
      //}
    x1=x0-Base;
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
