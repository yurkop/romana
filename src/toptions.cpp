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

  InitPar(1);

}

void Coptions::InitPar(int zero) {

  for (int i=0;i<MAX_CHTP;i++) {
    on[i]=true;
    AC[i]=true;
    Inv[i]=false;
    hS[i]=0*zero;
    Dt[i]=1*zero;
    Pre[i]=100*zero;
    if (zero)
      Len[i]=200;
    else
      Len[i]=1;
    Trg[i]=1;
    Drv[i]=1*zero;
    Thr[i]=50*zero;
    G[i]=12*zero;
    fdiv[i]=0;
    pls[i]=true;
    //Mask[i]=0xFF;

    group[i][0]=0;
    group[i][1]=0;
    ratediv[i]=0;
  }

  for (int i=0;i<2;i++) {
    coinc_w[i]=100;
    mult_w1[i]=1;
    mult_w2[i]=255;
  }
  
  forcewr=false;
  Trigger=0;
  DTW=1;
  Smpl=0;
  St_Per=0;

  F_start = gSystem->Now();
  F_stop = 0;
}

void Coptions::GetPar(const char* name, int module, int i, Int_t type_ch, int &par, int &min, int &max) {

  min=0;
  max=-1;
  //cout << "GetPAr7: " << module << " " << name << endl;
    if (!strcmp(name,"smooth")) {
      par = hS[i];
      min = 0;
      if (type_ch==3) //CRS-128
	max=7;
      else
        max=9;
    }
    else if (!strcmp(name,"dt")) {
      par = Dt[i];
      min = 1;
      if (module==22)
        max=1;
      else
        max=16383;
    }
    else if (!strcmp(name,"pre")) {
      par = Pre[i];

      if (module==22) {
        min = 0;
        max=8184;        
      }
      else if (module==32) {
        min = 0;
        max=4093;        
      }
      else { //33,34,35,41,51,52
        //!! знак противоположный тому, что в Протоколе!!!
        // здесь отрицательный знак означает начало записи
        // "после" срабатывания дискриминатора
        max = 1024;
        if (type_ch==1)
          min=-511;
	else if (type_ch==3) { //CRS-128
	  min=-511;
	}
        else //0,2
          min=-1023;
      }
    }
    else if (!strcmp(name,"len")) {
      par = Len[i];
      min = 1;
      if (module==22)
        max=16379;
      else if (module==32)
        max=32763;
      else { //33,34,35,41,51,52
        if (type_ch==0)
          max=4068;
        else if (type_ch==1)
          max=3048;
        else if (type_ch==2)
          max=6114;
        else //3
          max=1506;
      }
    }
    else if (!strcmp(name,"deriv")) {
      par = Drv[i];
      if (type_ch==3)
	max=255;
      else
	max=1023;
      if (module<=32) //22 or 32
        min = 0;
      else
        min = 1;
    }
    else if (!strcmp(name,"thresh")) {
      par = Thr[i];
      if (type_ch==0) {
        min=-2048;
        max=2047;
      }
      else { //1,2,3
        min= -65536;
        max= 65535;
      }
    }
    else if (!strcmp(name,"gain")) {
      par = G[i];
      if (type_ch==0) {
        min=5;
        max=12;
      }
      else { //1,2,3
        min=0;
        max=3;
      }
    }
    else if (!strcmp(name,"delay")) {
      par = hD[i];
      min=0;
      if (module==22) {
        max=0;
      }
      else {
        if (type_ch==0)
          max=4075;
        else if (type_ch==1)
          max=4092;
        else if (type_ch==2)
          max=1023;
        else //3
          max=255;
      }
    }
    else if (!strcmp(name,"trig")) {
      par = Trg[i];
      min=0;
      if (module==35)
        max=5;
      else if (module>=41 && module<=52)
        max=4;
      else if (module>=33) //33,34
        max=3;
      else
        max=0;
      // cout << "trig: " << module << " " << min << " " << max << endl;
    }
    else if (!strcmp(name,"ratediv")) {
      par = ratediv[i];
      min=0;
      max=1024;
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

  /*
  TCutG *cut = new TCutG();
  cuts.ll.push_back(std::make_pair(1,cut));
  cuts.ll.push_back(std::make_pair(2,new TCutG()));
  // cuts.gcuts.Add(cut);
  // //cuts.gcuts.push_back(TCutG());
  // //cuts.gcuts.push_back(TCutG());
  cout << "HCuts: " << cuts.ll.size() << endl;
  */

  memset(gitver,0,sizeof(gitver));
  maxch=0;
  maxtp=0;

  chkall=0;
  for (int i=0;i<MAX_CHTP;i++) {
    star[i]=true;
    chtype[i]=1;
    dsp[i]=false;
    St[i]=true;
    Ms[i]=true;
    //Mrk[i]=false;
    for (int j=0;j<NGRP;j++)
      Grp[i][j]=false;
    sS[i]=2;
    sThr[i]=cpar.Thr[i];

    sDrv[i]=1;
    sD[i]=0;
    Base1[i]=-10;
    Base2[i]=-5;
    Peak1[i]=5;
    Peak2[i]=30;
    dTm[i]=100;
    Pile[i]=100;
    sTg[i]=-1;
    //timing[i]=3;
    T1[i]=-5;
    T2[i]=5;
    W1[i]=-5;
    W2[i]=5;

    calibr_t[i]=1;
    E0[i]=0;
    E1[i]=1;
    E2[i]=0;
    Bc[i]=0;
  }

  raw_write=false;
  fProc=false;
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

  hard_logic=0;
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

  //decode=true;
  directraw=false;
  //analyze_or_dsp=true;
  checkdsp=false;

  b_logy=false;
  b_stat=false;
  b_gcuts=false;
  b_roi=false;

  start_ch=0;
  ntof_period=0;
  Flpath=10;
  TofZero=0;

  prof_nx=8;
  prof_ny=8;

  for (int i=0;i<4;i++) {
    Prof64[i]=i;
  }
  Prof64[4]=6;
  strcpy(Prof64_TSP,"time_06");
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

  //Tlim1=0;
  //Tlim2=0;
  Tstart=0;
  Tstop=0;
  Period=10;

  adcm_period=10;

  b_deriv[0]=true;
  b_deriv[1]=false;

  for (int i=0;i<10;i++) {
    b_peak[i]=true;
  }

  ncuts=0;
  memset(pcuts,0,sizeof(pcuts));

  usb_size=1024;
  rbuf_size=1024;
  nthreads=1;
  Nchan=32;
  tsleep=500;
  //event_buf=1000;
  event_lag=10;

  //F_start = gSystem->Now();
  T_acq = 0;

  memset(formula,0,sizeof(formula));
  memset(cut_form,0,sizeof(cut_form));
  maintrig=0;

  E_auto=2223;
  //int sz = sizeof(adj)/sizeof(Float_t)/2;
  for (int i=0;i<MAX_CH+NGRP+1;i++) {
    adj[i][0]=0;
    adj[i][1]=1;
    adj[i][2]=0;
  }
  //memset(maintrig,0,sizeof(maintrig));
  //strcpy(formula,"0");
}

Hdef::Hdef() {
  bins=1;
  bins2=1;
  min=0;
  max=1000;
  b=false;
  rb=1;
  for (int i=0;i<MAX_CH+NGRP;i++) {
    c[i]=true;
    w[i]=false;
    cut[i]=0;
  }
  memset(roi,0,sizeof(roi));
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
