#include <iostream>
#include "toptions.h"
#include <TClass.h>
#include <TSystem.h>
#include <cstdlib>

extern Coptions cpar;
using namespace std;

extern TList listmap;

Coptions::Coptions() {
  //ver = TClass::GetClass("Coptions")->GetClassVersion();
  // for (int i=0;i<MAX_CH;i++) {
  //   //chtype[i]=ch_other;
  //   enabl[i]=true;
  // }

  InitPar(1);

}

void Coptions::InitPar(int zero) {

  for (int i=0;i<MAX_CH+ADDCH;i++) {
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
    pls[i]=true;
    //Mask[i]=0xFF;
  }
  forcewr=false;
  DTW=1;

}

void Coptions::GetPar(const char* name, int module, int i, Int_t type_ch, int &par, int &min, int &max) {

  min=0;
  max=0;

  //CRS-2 ------------------------------------
  if (module==2) {
    if (!strcmp(name,"smooth")) {
      par = smooth[i];
      min = 0;
      max=10;
    }
    else if (!strcmp(name,"dt")) {
      par = deadTime[i];
      min = 1;
      max=16383;
    }
    else if (!strcmp(name,"pre")) {
      par = preWr[i];
      min = 0;
      max=8184;
    }
    else if (!strcmp(name,"len")) {
      par = durWr[i];
      min = 1;
      max=16379;
    }
    else if (!strcmp(name,"trig")) {
      par = trg[i];
      min = 1;
      max=1;
    }
    else if (!strcmp(name,"deriv")) {
      par = kderiv[i];
      min = 0;
      max=1023;
    }
    else if (!strcmp(name,"thresh")) {
      par = threshold[i];
      min = -2048;
      max = 2047;
    }
    else if (!strcmp(name,"gain")) {
      par = adcGain[i];
      min = 5;
      max=12;
    }
    else {
      cout << "GetPar: wrong name: " << name << " " << i << endl;
      exit(-1);
    }
  }//CRS-2
  //CRS-32, firmware<=2 --------------------------
  else if (module==32) {
    if (!strcmp(name,"smooth")) {
      par = smooth[i];
      min = 0;
      max=10;
    }
    else if (!strcmp(name,"dt")) {
      par = deadTime[i];
      min = 1;
      max=16383;
    }
    else if (!strcmp(name,"pre")) {
      par = preWr[i];
      min = 0;
      if (type_ch==0)
	max=4094;
      else if (type_ch==1)
	max=4093;
    }
    else if (!strcmp(name,"len")) {
      par = durWr[i];
      min = 1;
      max=32763;
    }
    else if (!strcmp(name,"trig")) {
      par = trg[i];
      min = 1;
      max=1;
    }
    else if (!strcmp(name,"deriv")) {
      par = kderiv[i];
      min = 0;
      max=1023;
    }
    else if (!strcmp(name,"thresh")) {
      par = threshold[i];
      if (type_ch==1) {
	min = -65536;
	max= 65535;
      }
      else {
	min = -2048;
	max = 2047;
      }
    }
    else if (!strcmp(name,"gain")) {
      par = adcGain[i];
      if (type_ch==1) {
	min = 0;
	max=3;
      }
      else {
	min = 5;
	max=12;
      }
    }
    else {
      cout << "GetPar: wrong name: " << name << " " << i << endl;
      exit(-1);
    }
  }
  //CRS-33 [CRS-32, firmware>=3] --------------------------
  else if (module>=33) {
    if (!strcmp(name,"smooth")) {
      par = smooth[i];
      min = 0;
      max=9;
    }
    else if (!strcmp(name,"dt")) {
      par = deadTime[i];
      min = 1;
      max=16383;
    }
    else if (!strcmp(name,"pre")) {
      par = preWr[i];
      min =-1024;
      if (type_ch==1) {
	max=511;
      }
      else {
	max=1023;
      }
    }
    else if (!strcmp(name,"len")) {
      par = durWr[i];
      min = 1;
      if (type_ch==1) {
	max=3048;
      }
      else {
	max=4068;
      }
    }
    else if (!strcmp(name,"trig")) {
      par = trg[i];
      min = 0;
      max=3;
    }
    else if (!strcmp(name,"deriv")) {
      par = kderiv[i];
      min = 1;
      max=1023;
    }
    else if (!strcmp(name,"thresh")) {
      par = threshold[i];
      if (type_ch==1) {
	min = -65536;
	max= 65535;
      }
      else {
	min = -2048;
	max = 2047;
      }
    }
    else if (!strcmp(name,"gain")) {
      par = adcGain[i];
      if (type_ch==1) {
	min = 0;
	max=3;
      }
      else {
	min = 5;
	max=12;
      }
    }
    else {
      cout << "GetPar: wrong name: " << name << " " << i << endl;
      exit(-1);
    }
  }


  if (type_ch==255) {
    min = -65536;
    max= 65535;
  }

}

Toptions::Toptions() {

  //cout << "toptions" << endl;
  for (int i=0;i<MAX_CH+ADDCH;i++) {
    chtype[i]=ch_NIM;
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
    timing[i]=3;
    T1[i]=-5;
    T2[i]=5;
    W1[i]=-5;
    W2[i]=5;

    E0[i]=0;
    E1[i]=1;
    E2[i]=0;
    Bc[i]=0;
  }

  // for (int i=MAX_CH;i<MAX_CH+ADDCH;i++) {
  //   chtype[i]=ch_other;
  // }

  raw_write=false;
  dec_write=false;
  root_write=false;

  raw_compr=1;
  dec_compr=1;
  root_compr=1;

  memset(Filename,0,sizeof(Filename));

  const char* types[]={"NaI","BGO","Si 1","Si 2","Stilb","Demon","HPGe",
			    "NIM","Other","Copy",""};
  for (int i=0;i<ADDCH;i++) {
    strcpy(tnames[i],types[i]);
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

  Tstart=0;
  Tstop=0;

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
  event_buf=1000;
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
