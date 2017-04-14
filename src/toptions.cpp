#include <iostream>
#include "toptions.h"

using namespace std;

Toptions::Toptions() {

  //cout << "toptions" << endl;
  for (int i=0;i<MAX_CH;i++) {
    //chtype[i]=(ChDef) 99;
    //cout << "chtype: " << chtype[i] << endl;
    channels[i]=ch_off2;
    //cout << i << " toptions " << channels[i] << endl;
    nsmoo[i]=2;
    enabl[i]=true;
  }

  ev_min=5000;
  ev_max=10000;

  tgate1=500;
  tgate2=100;
  mult1=1;
  mult2=32;
  
  //www[0]=1.0;
  //www[1]=5.0;
  /*
  gam_thresh = 20;
  gam_elim1=20;
  gam_elim2=9999;
  gam_bkg1=-20;
  gam_bkg2=-6;
  gam_border1=-5;
  gam_border2=100;
  gam_pile1=20;
  gam_pile2=120;
  gam_timing=1;
  gam_twin=0;

  ng_thresh = 15;
  ng_elim1=5;
  ng_elim2=9999;
  ng_bkg1=-15;
  ng_bkg2=-8;
  ng_border1=-7;
  ng_border2=8;
  ng_pile=20;
  ng_timing=1;
  ng_twin=0;

  nim_thresh1=500;
  nim_thresh2=50;
  nim_timing=1;
  nim_twin=0;
  */
  long_max=200;
  long_bins=1000;

  tof_max=3000;
  tof_bins=1;

  etof_max=20;
  etof_bins=20;

  tdc_max=25;
  tdc_bins=1000;

  mtof_max=50000;
  mtof_bins=5;

  sum_max=1000;
  sum_bins=1;

  rms_max=10;
  rms_bins=10;

  //tdc_bins1=1000;
  //tdc_bins2=1000;

  starts_thr1=10000;
  starts_thr2=6;

  beam1=0.0;
  beam2=0.0;

  /*
  wgam1=2;
  wgam2=3;
  wneu1=3;
  wneu2=4;
  wtail1=4;
  wtail2=5;
  */

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

  /*
  for (int i=0;i<30;i++) {
    opt_label[i]="";
    opt_tip[i]="";
  }
  */

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

  buf_size=1024;
  tsleep=500;
  event_buf=1000;
  event_lag=10;
  //printf("opt: %f\n", *(double*) opt_id[9]);

}

void Toptions::InitPar(int module) {

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

void Toptions::GetPar(const char* name, int module, int i, int &par, int &min, int &max) {

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

//Toptions::~Toptions() {
//}

//void Toptions::Read(char* filename) {
//}

