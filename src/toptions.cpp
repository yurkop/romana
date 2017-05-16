#include <iostream>
#include "toptions.h"
#include <TClass.h>

using namespace std;

Coptions::Coptions() {
  //ver = TClass::GetClass("Coptions")->GetClassVersion();
  for (int i=0;i<MAX_CH;i++) {
    chtype[i]=ch_other;
    enabl[i]=true;
  }
}

void Coptions::InitPar(int module) {

  if (module==2) {
    for (int i=0;i<MAX_CH+ADDCH;i++) {
      acdc[i]=false;
      inv[i]=false;
      smooth[i]=0;
      deadTime[i]=1;
      preWr[i]=100;
      durWr[i]=200;
      kderiv[i]=0;
      threshold[i]=1500;
      adcGain[i]=0;
    }
    forcewr=false;
  }
  else if (module==32) {
    for (int i=0;i<MAX_CH+ADDCH;i++) {
      acdc[i]=false;
      inv[i]=false;
      smooth[i]=0;
      deadTime[i]=1;
      preWr[i]=100;
      durWr[i]=200;
      kderiv[i]=0;
      threshold[i]=1500;
      adcGain[i]=0;
    }
    forcewr=false;
  }
  else {
    cout << "InitPar: wrong module: " << module << endl;
  }

}

void Coptions::GetPar(const char* name, int module, int i, int &par, int &min, int &max) {

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
      max=4094;
    }
    else if (module==2) {
      max=8184;
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
  else if (!strcmp(name,"gain")) {
    par = adcGain[i];
    min = 0;
    max=12;
  }
  else if (!strcmp(name,"thresh")) {
    par = threshold[i];
    min = 0;
    max=2047;
  }

}

Toptions::Toptions() {

  //cout << "toptions" << endl;
  for (int i=0;i<MAX_CH;i++) {
    //chtype[i]=(ChDef) 99;
    //cout << "chtype: " << chtype[i] << endl;
    channels[i]=ch_off2;
    //cout << i << " toptions " << channels[i] << endl;
    nsmoo[i]=2;
    //enabl[i]=true;
  }

  ev_min=5000;
  ev_max=10000;

  tgate=500;
  //tgate2=100;
  mult1=1;
  mult2=32;

  seltab=0;


  time_max=time_bins=1;
  tof_max=tof_bins=1;
  mtof_max=mtof_bins=1;
  amp_max=amp_bins=1;
  hei_max=hei_bins=1;

  time_min=0;
  tof_min=0;
  mtof_min=0;
  amp_min=0;
  hei_min=0;




  //tdc_bins1=1000;
  //tdc_bins2=1000;

  starts_thr1=10000;
  starts_thr2=6;

  beam1=0.0;
  beam2=0.0;

  gcut[0]=NULL;
  gcut[1]=NULL;
  gcut[2]=NULL;

  num_events=100000;
  num_buf=100;

  //b_gcut=false;

  b_osc=false;
  b_leg=false;
  //for (int jj=0;jj<6;jj++) {
  b_logy=false;
    //}
  b_time=false;

  //nsmoo=2;
  psd_ch=0;
  start_ch=0;
  //mon_ch=28;

  T0=0.0;
  LL=20.0;

  draw_opt=M_DEMON;

  for (int i=0;i<MAX_CH;i++) {
    sprintf(chname[i],"ch%02d",i);    
    color[i]=0;
    channels[i]=ch_off2;
    //chinv[i]=false;
    //sprintf(chname[i],"ch%0d",i);
  }

  sprintf(chname[0],"demon");
  sprintf(chname[2],"isomer");
  sprintf(chname[6],"pulser");
  sprintf(chname[8],"monitor1");
  sprintf(chname[12],"stilbene");
  sprintf(chname[14],"start");

  channels[0]=ch_ng;
  channels[2]=ch_nim;
  channels[6]=ch_nim;
  channels[8]=ch_nim;
  channels[12]=ch_ng;
  channels[14]=ch_nim;

  color[0]=1;
  color[2]=1;
  color[6]=1;
  color[8]=1;
  color[12]=1;
  color[14]=1;

  lcolor[0]=1;
  lcolor[1]=1;
  lcolor[2]=1;
  lcolor[3]=1;
  lcolor[4]=1;
  lcolor[5]=1;

  Tstart=0;
  Tstop=0;

  Ecalibr[0]=0.0;
  Ecalibr[1]=1.0;

  LongStamp=1;

  b_deriv[0]=true;
  b_deriv[1]=false;

  for (int i=0;i<10;i++) {
    b_peak[i]=true;
  }

  rBSIZE=131072; //1024*128
  EBufsize=100;
  period=5;

  usb_size=1024;
  rbuf_size=1024;
  tsleep=500;
  event_buf=1000;
  event_lag=10;
  //printf("opt: %f\n", *(double*) opt_id[9]);

}

//Toptions::~Toptions() {
//}

//void Toptions::Read(char* filename) {
//}

