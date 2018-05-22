#include <iostream>
#include "toptions.h"
#include <TClass.h>
#include <TSystem.h>
#include <cstdlib>

extern Coptions cpar;
using namespace std;

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
    kderiv[i]=1*zero;
    threshold[i]=1500*zero;
    adcGain[i]=12*zero;
  }
  forcewr=false;

}

void Coptions::GetPar(const char* name, int module, int i, Short_t type_ch, int &par, int &min, int &max) {

  min=0;
  max=0;

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
    if (module==32) {
      if (type_ch==0)
	max=4094;
      else if (type_ch==1)
	max=4093;
    }
    else if (module==2) {
      max=8184;
    }
    else {
      max=4093;
      //cout << "GetPar: wrong channel: " << name << " " << module << " " << i << " " << type_ch << endl;
      //exit(-1);
    }
  }
  else if (!strcmp(name,"len")) {
    par = durWr[i];
    min = 1;
    if (module==32) {
      max=32763;
    }
    else if (module==2) {
      max=16379;
    }
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

  if (type_ch==255) {
    min = -65536;
    max= 65535;
  }

}

Toptions::Toptions() {

  //cout << "toptions" << endl;
  for (int i=0;i<MAX_CH+ADDCH;i++) {
    chtype[i]=ch_other;
    Mrk[i]=true;
    nsmoo[i]=2;
    thresh[i]=cpar.threshold[i];

    bkg1[i]=-100;
    bkg2[i]=-10;
    peak1[i]=10;
    peak2[i]=100;
    deadT[i]=100;
    pile[i]=100;
    timing[i]=0;
    twin1[i]=99;
    twin2[i]=99;

    emult[i]=1;
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

  memset(fname_raw,0,sizeof(fname_raw));
  memset(fname_dec,0,sizeof(fname_dec));
  memset(fname_root,0,sizeof(fname_root));

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
  analyze=true;

  b_logy=false;
  b_stat=false;
  b_gcuts=false;

  start_ch=0;
  mtof_period=0;
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
  Nchan=32;
  tsleep=500;
  event_buf=1000;
  event_lag=10;

  F_start = gSystem->Now();
  T_acq = 0;

  memset(formula,0,sizeof(formula));
  memset(cut_form,0,sizeof(cut_form));
  //strcpy(formula,"0");
}

Hdef::Hdef() {
  bins=1;
  bins2=1;
  min=0;
  max=100;
  b=true;
  for (int i=0;i<MAX_CH;i++) {
    c[i]=true;
    w[i]=false;
    cut[i]=0;
  }
}
