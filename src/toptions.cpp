#include <iostream>
#include "toptions.h"
#include <TClass.h>
#include <TSystem.h>
#include <TDataMember.h>
#include <cstdlib>

extern Coptions cpar;
using namespace std;

//extern TList listmap;
//extern int debug;

Coptions::Coptions() {
  //ver = TClass::GetClass("Coptions")->GetClassVersion();
  // for (int i=0;i<MAX_CH;i++) {
  //   //chtype[i]=ch_other;
  //   enabl[i]=true;
  // }

  InitPar(1);

}

void Coptions::InitPar(int zero) {

  for (int i=0;i<MAX_CHTP;i++) {
    enabl[i]=true;
    acdc[i]=true;
    inv[i]=false;
    smooth[i]=0*zero;
    deadTime[i]=1*zero;
    preWr[i]=100*zero;
    if (zero)
      durWr[i]=200;
    else
      durWr[i]=1;
    trg[i]=1;
    kderiv[i]=1*zero;
    threshold[i]=50*zero;
    adcGain[i]=12*zero;
    fdiv[i]=0;
    pls[i]=true;
    //Mask[i]=0xFF;
  }
  forcewr=false;
  DTW=1;
  Smpl=0;
  St_Per=0;

}

void Coptions::GetPar(const char* name, int module, int i, Int_t type_ch, int &par, int &min, int &max) {

  min=0;
  max=-1;
  //cout << "GetPAr7: " << module << " " << name << endl;
    if (!strcmp(name,"smooth")) {
      par = smooth[i];
      min = 0;
      if (module==22 || module==32)
        max=10;
      else
        max=9;
    }
    else if (!strcmp(name,"dt")) {
      par = deadTime[i];
      min = 1;
      if (module==22)
        max=1;
      else
        max=16383;
    }
    else if (!strcmp(name,"pre")) {
      par = preWr[i];

      if (module==22) {
        min = 0;
        max=8184;        
      }
      else if (module==32) {
        min = 0;
        max=4093;        
      }
      else { //33,34,41
        //!! знак противоположный тому, что в Протоколе!!!
        // здесь отрицательный знак означает начало записи
        // "после" срабатывания дискриминатора
        max = 1024;
        if (type_ch==1)
          min=-511;
        else //0,2
          min=-1023;
      }
    }
    else if (!strcmp(name,"len")) {
      par = durWr[i];
      min = 1;
      if (module==22)
        max=16379;
      else if (module==32)
        max=32763;
      else { //33,34,41
        if (type_ch==0)
          max=4068;
        else if (type_ch==1)
          max=3048;
        else //2
          max=6114;
      }
    }
    else if (!strcmp(name,"deriv")) {
      par = kderiv[i];
      max=1023;
      if (module<=32) //22 or 32
        min = 0;
      else
        min = 1;
    }
    else if (!strcmp(name,"thresh")) {
      par = threshold[i];
      if (type_ch==0) {
        min=-2048;
        max=2047;
      }
      else { //1,2
        min= -65536;
        max= 65535;
      }
    }
    else if (!strcmp(name,"gain")) {
      par = adcGain[i];
      if (type_ch==0) {
        min=5;
        max=12;
      }
      else { //1,2
        min=0;
        max=3;
      }
    }
    else if (!strcmp(name,"delay")) {
      par = delay[i];
      min=0;
      if (module==22) {
        max=0;
      }
      else {
        if (type_ch==0)
          max=4075;
        else if (type_ch==1)
          max=4092;
        else //2
          max=1023;
      }
    }
    else if (!strcmp(name,"trig")) {
      par = trg[i];
      min=0;
      if (module==41)
        max=4;
      else if (module>=33) //33,34
        max=3;
      else
        max=0;
      // cout << "trig: " << module << " " << min << " " << max << endl;
    }
    else {
      cout << "GetPar: wrong name: " << name << " " << module << " " << i << endl;
      exit(-1);
    }

  if (type_ch==255) {
    min = -65536;
    max= 65535;
  }

}

Toptions::Toptions() {

  //cout << "toptions" << endl;

  memset(gitver,0,sizeof(gitver));
  maxch=0;
  maxtp=0;

  chkall=0;
  for (int i=0;i<MAX_CHTP;i++) {
    star[i]=true;
    chtype[i]=1;
    //chtype[i]=ch_NIM;
    dsp[i]=false;
    St[i]=true;
    //Mrk[i]=false;
    for (int j=0;j<NGRP;j++)
      Grp[i][j]=false;
    sS[i]=2;
    Thr[i]=cpar.threshold[i];

    Drv[i]=1;
    Delay[i]=0;
    Base1[i]=-10;
    Base2[i]=-5;
    Peak1[i]=5;
    Peak2[i]=30;
    dT[i]=100;
    Pile[i]=100;
    sTg[i]=-1;
    //timing[i]=3;
    T1[i]=-5;
    T2[i]=5;
    W1[i]=-5;
    W2[i]=5;

    E0[i]=0;
    E1[i]=1;
    E2[i]=0;
    Bc[i]=0;
  }

  // for (int i=MAX_CH;i<MAX_CHTP;i++) {
  //   chtype[i]=ch_other;
  // }

  raw_write=false;
  raw_flag=false;
  dec_write=false;
  root_write=false;

  raw_compr=1;
  dec_compr=1;
  root_compr=1;

  memset(Filename,0,sizeof(Filename));
  memset(ch_name,0,sizeof(ch_name));

  const char* types[MAX_TP]={"NaI","BGO","HPGe","Si","Stilb","Demon","Plast",
  "NIM"};
  for (int i=0;i<MAX_TP;i++) {
    strcpy(ch_name[i],types[i]);
  }
  
  // memset(fname_raw,0,sizeof(fname_raw));
  // memset(fname_dec,0,sizeof(fname_dec));
  // memset(fname_root,0,sizeof(fname_root));

  ev_min=10;
  ev_max=1000;

  tgate=500;
  tveto=10;
  mult1=1;
  mult2=32;

  xdiv=2;
  ydiv=2;
  b_stack=false;
  icheck=0;

  seltab=0;

  num_events=100000;
  num_buf=100;

  decode=true;
  //analyze_or_dsp=true;
  checkdsp=false;

  b_logy=false;
  b_stat=false;
  b_gcuts=false;

  start_ch=0;
  ntof_period=0;
  Flpath=10;
  TofZero=0;

  prof_nx=8;
  prof_ny=8;

  for (int i=0;i<5;i++) {
    Prof64[i]=-1;
  }
  Prof64_W[0]=0;
  Prof64_W[1]=0;
  Prof64_W[2]=0;
  for (int i=0;i<8;i++) {
    Prof_x[i]=-1;
    Prof_y[i]=-1;
  }
  for (int i=0;i<16;i++) {
    Ing_x[i]=-1;
    Ing_y[i]=-1;
  }

  Tstart=0;
  Tstop=0;
  Period=10;

  b_deriv[0]=true;
  b_deriv[1]=false;

  for (int i=0;i<10;i++) {
    b_peak[i]=true;
  }

  ncuts=0;
  memset(pcuts,0,sizeof(pcuts));

  usb_size=1024;
  rbuf_size=20000;
  nthreads=1;
  Nchan=32;
  tsleep=500;
  //event_buf=1000;
  event_lag=10;

  F_start = gSystem->Now();
  T_acq = 0;

  memset(formula,0,sizeof(formula));
  memset(cut_form,0,sizeof(cut_form));
  maintrig=0;
  //memset(maintrig,0,sizeof(maintrig));
  //strcpy(formula,"0");
}

Hdef::Hdef() {
  bins=1;
  bins2=1;
  min=0;
  max=1000;
  b=false;
  for (int i=0;i<MAX_CH+NGRP;i++) {
    c[i]=true;
    w[i]=false;
    cut[i]=0;
  }
}

/*
Hdef::Hdef(const Hdef& other) {
  cout << "hdcopy: " << endl;
  bins=other.bins;
  bins2=other.bins2;
  min=other.min;
  max=other.max;
  b=other.b;
  memcpy(c,other.c,sizeof(c));
  memcpy(w,other.w,sizeof(w));
  memcpy(cut,other.cut,sizeof(cut));
}

Hdef& Hdef::operator=(const Hdef& other) {
  cout << "hd==: " << sizeof(c) << " " << sizeof(other.c) << endl;
  //return *this = Hdef(other);


  bins=other.bins;
  bins2=other.bins2;
  min=other.min;
  max=other.max;
  b=other.b;
  for (int i=0;i<MAX_CH+NGRP;i++) {
    c[i]=other.c[i];
    cout << i << " " << c[i] << " " << other.c[i] << endl;
    //w[i]=other.w[i];
    //cut[i]=other.cut[i];
  }
  // memcpy(c,other.c,sizeof(c));
  // memcpy(w,other.w,sizeof(w));
  // memcpy(cut,other.cut,sizeof(cut));
}
*/
