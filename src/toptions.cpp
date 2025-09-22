#include <iostream>
#include "toptions.h"
#include <TClass.h>
#include <TSystem.h>
#include <TDataMember.h>
#include <cstdlib>
#include "romana.h"
#include <map>

extern CRS* crs;
extern Coptions cpar;
GG *gg;

using namespace std;

//extern TList listmap;
//extern int debug;

Coptions::Coptions() {

  gg = new(GG);
  InitPar(1);
  InitMinMax();

}

void Coptions::InitPar(int zero) {

  for (int i=0;i<MAX_CHTP;i++) {
    //crs_ch[i]=0; //255; //undefined
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
    LT[i]=0;
    G[i]=12*zero;
    fdiv[i]=0;
    pls[i]=true;
    //Mask[i]=0xFF;

    group[i][0]=0;
    group[i][1]=0;
    RD[i]=0;
  }

  for (int i=0;i<2;i++) {
    coinc_w[i]=100;
    mult_w1[i]=1;
    mult_w2[i]=255;
  }
  
  RMask=0xFFFF;
  forcewr=false;
  Trigger=0;
  Smpl=0;
  St_Per=0;
  F24=0;
  DTW=1;

  F_start = gSystem->Now();
  F_stop = 0;
  //Thr2=0*zero;

}

std::string Coptions::GetDevice(int module, int opt) {
  string res;
  if (device[0]) {
    switch (device[0]) {
    case 1: //crs-32
      res="CRS-32";
      break;
    case 2: //crs-6/16
      res="CRS-6/16";
      break;
    case 3: //crs-16 or crs-2
      if (device[3]==0) // -> crs2
	res="CRS-2";
      else
	res="CRS-16";
      break;
    case 4: //crs-8/16
      res="CRS-8";
      break;
    case 5: //crs-128
      res="CRS-128";
      break;
    case 6: //AK-32
      res="AK-32";
      break;
    default:
      res="unknown";
      break;
    }

    if (device[1]>0)
      res+="_N"+std::to_string(device[1]);

    if (opt) {
      res+=" Npl:"+std::to_string(device[2]);
      res+=" Ver:"+std::to_string(device[2]);
    }
  } //device[0]
  else {
    switch (module) {
    case 1:
      res="ADCM raw";
      break;
    case 3:
      res="ADCM dec";
      break;
    case 2:
    case 22:
      res="CRS-2";
      break;
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
      res="CRS-32";
      break;
    case 41:
    case 42:
    case 43:
    case 44:
      res="CRS-8";
      break;
    case 51:
    case 52:
    case 53:
    case 54:
      res="CRS-128";
      break;
    case 45:
      res="AK-32";
      break;
    default:
      res="unknown";
      break;
    }
  }

  if (opt)
    res ="Device: "+res;

  return res;
}
		      
void Coptions::InitMinMax() {
  //см. parameters.xlsx
  // 8+8: первая строчка - мин; вторая - макс.
  // 8 чисел: crs_ch (см. libcrs.h):
  //0 - undefined (=no device)
  //1 - CRS-2
  //2 - CRS-32 11 bit
  //3 - CRS-16 16 bit
  //4 - CRS-8
  //5 - CRS-128
  //6 - AK32 14 bit
  //7 - AK32 12 bit
  

  arr mhS,mDt,mPre,mLen,mDrv,mThr,mLT,mG,mhD,mTrg,mRD;

  // см. sum: сглаживание (суммирование)
  mhS = {0,0,0,0,0,0,0,0,
	 512,0,512,512,512,128,128,128};
  // мертвое время дискриминатора
  mDt = {0,0,1,1,1,1,1,1,
	 16383,0};
  fill_n(&mDt[MM+2],6,16383);
  // предзапись M
  mPre = {-1023,0,-1023,-511,-1023,-511,-511,-511,
          1024,8184};
  fill_n(&mPre[MM+2],6,1024);
  // общая длина записи
  mLen = {1,0,1,1,1,1,1,1,
	  12000,16379,4068,3048,6114,1506,12000,12000};
  // параметр производной
  mDrv = {1,0,1,1,1,1,1,1,
	  1023,1023,1023,1023,1023,255,255,255};
  // порог срабатывания
  mThr = {-65536,0,-2048};
  fill_n(&mThr[3],5,-65536);
  mThr[MM]=65535; mThr[MM+1]=2047; mThr[MM+2]=2047;
  fill_n(&mThr[MM+3],5,65535);
  // нижний порог дискриминатора типов 3, 4
  mLT = mThr;
  mLT[3]=0; mLT[MM+3]=0;
  // дополнительное усиление
  mG = {0,5,5,0,0,0,0,0,
	12,12,12,3,3,3,4,4};
  // задержка
  mhD = {0,0,0,0,0,0,0,0,
	 250,0,4075,4092,1023,255,250,250};
  // тип срабатывания дискриминатора
  mTrg = {0,0,0,0,0,0,0,0,
	  7,1,6,6,6,6,7,7};
  // величина пересчета P ("незаписанных" срабатываний дискриминатора)
  mRD = {0,0,0,0,0,0,0,0,
	 1023,0,0,0,1023,1023,1023,1023};

  //arr mhS,mDt,mPre,mLen,mDrv,mThr,mLT,mG,mhD,mTrg,mRD;

  gg->mcpar[hS] = mhS;
  gg->mcpar[Dt] = mDt;
  gg->mcpar[Pre] = mPre;
  gg->mcpar[Len] = mLen;
  gg->mcpar[Drv] = mDrv;
  gg->mcpar[Thr] = mThr;
  gg->mcpar[LT] = mLT;
  gg->mcpar[G] = mG;
  gg->mcpar[hD] = mhD;
  gg->mcpar[Trg] = mTrg;
  gg->mcpar[RD] = mRD;

  // cout << "mhS: " << mhS[7] << " " << mhS[MM+7] << " "
  //      << mcpar[hS][7] << " " << mcpar[hS][MM+7] << endl;
}

void Coptions::GetParm(const char* name, int i, void *par, int &min, int &max) {
  // std::vector<int> myvector(10);
  // try {
  //   myvector.at(20)=100;      // vector::at throws an out-of-range
  // }
  // catch (const std::out_of_range& oor) {
  //   std::cerr << "Out of Range error: " << oor.what() << '\n';
  // }

  min=-9999999;//0;
  max=9999999;//-1;
  //if (crs->crs_ch[i]==0) return;

  arr xx;

  try {
    xx = gg->mcpar.at(par);
    min = xx.at(crs->crs_ch[i]);
    max = xx.at(MM+crs->crs_ch[i]);
  }
  catch (const std::out_of_range& oor) {
    prnt("ss s d d ss;",BRED,"ErrGetParm:",name,i,crs->crs_ch[i],oor.what(),RST);
    return;
  }

  //prnt("ss s d d d ds;",BGRN,"GetParm:",name,i,crs->crs_ch[i],min,max,RST);

} //GetParm

Int_t Coptions::ChkLen(Int_t i, Int_t module) {
  //выравнивает длину записи кратно 3 или 4 
  if (module==2 || module==22) return Len[i];

  int dd=1;
  switch (crs->crs_ch[i]) {
  case 2:
    dd=4;
    break;
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    dd=3;
    break;
  default:
    return Len[i];
  }

  int res = ((Len[i]+dd-1)/dd)*dd;
  if (res<dd) res=dd;
  //prnt("ss d d d d ds;",BRED,"ChkLen: ",i,Len[i],crs->crs_ch[i],module,res,RST);
  return res;
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
    Dsp[i]=0;
    Pls[i]=false;
    St[i]=true;
    Ms[i]=true;
    //Mrk[i]=false;
    for (int j=0;j<NGRP;j++)
      Grp[i][j]=false;
    sS[i]=0;
    sThr[i]=cpar.Thr[i];
    sLT[i]=cpar.LT[i];

    sDrv[i]=1;
    sD[i]=0;
    B1[i]=-10;
    B2[i]=-5;
    P1[i]=5;
    P2[i]=30;
    dTm[i]=100;
    Pile[i]=100;
    sTg[i]=-1;
    //timing[i]=3;
    DD[i]=1;
    FF[i]=2;
    T1[i]=-5;
    T2[i]=5;
    W1[i]=-5;
    W2[i]=5;

    Mt[i]=0;
    Mr[i]=0;
    calibr_t[i]=1;
    E0[i]=0;
    E1[i]=1;
    E2[i]=0;
    //Bc[i]=0;
    Pz[i]=0;

    Fpath[i]=10;
  }

  raw_write=false;
  fProc=false;
  fTxt=false;
  dec_write=false;
  root_write=false;

  raw_compr=1;
  dec_compr=1;
  root_compr=1;

  addrandom=false;

  dec_format=79;
  strcpy(dec_mask,"T AtW C");

  memset(Filename,0,sizeof(Filename));
  memset(Daqlog,0,sizeof(Daqlog));
  memset(Log,0,sizeof(Log));

  memset(ch_name,0,sizeof(ch_name));
  strcpy(Filename,"test01");

  const char* types[MAX_TP]={"NaI","BGO","HPGe","Si","Stilb","Demon","Plast",
  "NIM"};
  for (int i=0;i<MAX_TP;i++) {
    strcpy(ch_name[i],types[i]);
  }
  
  // memset(fname_raw,0,sizeof(fname_raw));
  // memset(fname_dec,0,sizeof(fname_dec));
  // memset(fname_root,0,sizeof(fname_root));

  ev_min=100; //event lag
  ev_max=1000; //Event_list size

  //hard_logic=0;
  tgate=500;
  tveto=10;
  mult1=1;
  mult2=9999;

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
  b_fpeaks=false;

  start_ch=0;
  ntof_period=0;
  //Flpath=10;
  TofZero=0;

  Prof_type=64;
  Ing_type=256;

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
  Prof64_THR=50;
  Prof64_GAT=100;
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
  //event_lag=100;

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
  SimSim[0]=10; //opt.Period
  SimSim[1]=10; //cpar.Pre
  SimSim[2]=24; //cpar.Len
  SimSim[3]=0; //Pulse type
  SimSim[4]=1000; //Amp
  SimSim[5]=1; //Sig/RC_Width
  SimSim[6]=1; //RC

  SimSim[7]=-10; //Pos min
  SimSim[8]=20; //Pos spread
  SimSim[9]=15; //Window
  SimSim[10]=0; //Time delta (time-pos)
  SimSim[11]=0; //Noise

  //SimSim[9]=0; //Simul2-pos

  // SimSim[10]=1; //CFD delay
  // SimSim[11]=1; //CFD fraction

  Peak_thr=0.2;
  Peak_wid=0;
  Peak_bwidth=20;
  Peak_maxpeaks=2;
  Peak_smooth=0;
  Peak_show_sm=false;
  Peak_use_mean=false;
  Peak_print=false;

  for (auto i=0;i<MOP;i++) {
    OptPar[i][0]=0;
    OptPar[i][1]=0;
    OptPar[i][2]=2;
    OptPar[i][3]=1;
  }

  memset(wrk_check,0,sizeof(wrk_check));

  //sThr2=cpar.Thr2;

  Nrows = 8;
  ScrollPos=0;

  hx_slider[0]=0;
  hx_slider[1]=1;
  hy_slider[0]=1;
  hy_slider[1]=0;

#ifdef YUMO
  yumo_x1=0;
  yumo_x2=1;
  yumo_y1=2;
  yumo_y2=3;
#endif

}

Hdef::Hdef() {
  bins=1;
  bins2=1;
  min=0;
  max=1000;
  b=false;
  htp=false;
  rb=1;
  rb2=1;
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
