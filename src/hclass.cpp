//----- HClass ----------------
#include <climits>
#include "romana.h"
#include <TStyle.h>
#include "TRandom.h"
#include <sstream>

extern Toptions opt;

extern CRS* crs;
extern ParParDlg *parpar;
extern Coptions cpar;
extern HClass* hcl;
extern TRandom rnd;

//------------------------------

Float_t Mdef::VarTime(EventClass* e, PulseClass* p){
  //PulseClass* pls = (PulseClass*) p;
  //cout << "vartime: " << pls->Chan << " " << (pls->Time - e->T0)*opt.Period+opt.sD[pls->Chan] << endl;
  return (p->Time - e->T0)*opt.Period+opt.sD[p->Chan];
}

Float_t Mdef::VarRate(EventClass* e, PulseClass* p){
  Float_t x = e->Tstmp*crs->sPeriod;
  return x;
}

Float_t Mdef::VarNtof(EventClass* e, PulseClass* p){
  static Long64_t Tstart0=LLONG_MAX; //Tstamp of the ntof start pulses
  static Float_t Time0=0; //Exact time of the ntof start pulses

  Float_t tt=0;
  if (p->Chan==opt.start_ch) {
    Tstart0 = e->Tstmp;
    Time0 = p->Time;
    return 0;
  }

  tt = e->Tstmp - Tstart0;
  tt+= p->Time - Time0;
  tt*= mks*opt.Period;

  //check for missed starts - obsolete...
  // if (opt.ntof_period>0.01 && tt>opt.ntof_period) {
  //   crs->Tstart0+=Long64_t(1000*opt.ntof_period/opt.Period);
  //   tt = (tm - crs->Tstart0)*mks*opt.Period;
  // }

  return tt;
}

Float_t Mdef::VarEtof(EventClass* e, PulseClass* p){
  static Long64_t Tstart0=LLONG_MAX; //Tstamp of the ntof start pulses
  static Float_t Time0=0; //Exact time of the ntof start pulses

  Float_t tt=0;
  if (p->Chan==opt.start_ch) {
    Tstart0 = e->Tstmp;
    Time0 = p->Time;
    return 0;
  }

  tt = e->Tstmp - Tstart0;
  tt+= p->Time - Time0;
  tt*= mks*opt.Period;

  Float_t ee = 72.298*opt.Flpath/(tt-opt.TofZero);

  //check for missed starts - obsolete...
  // if (opt.ntof_period>0.01 && tt>opt.ntof_period) {
  //   crs->Tstart0+=Long64_t(1000*opt.ntof_period/opt.Period);
  //   tt = (tm - crs->Tstart0)*mks*opt.Period;
  // }

  return ee*ee;
}

Float_t Mdef::VarLtof(EventClass* e, PulseClass* p){
  static Long64_t Tstart0=LLONG_MAX; //Tstamp of the ntof start pulses
  static Float_t Time0=0; //Exact time of the ntof start pulses

  Float_t tt=0;
  if (p->Chan==opt.start_ch) {
    Tstart0 = e->Tstmp;
    Time0 = p->Time;
    return 0;
  }

  tt = e->Tstmp - Tstart0;
  tt+= p->Time - Time0;
  tt*= mks*opt.Period;

  Float_t sqee = 72.298*opt.Flpath/(tt-opt.TofZero);
  Float_t lambda = 0.286*sqee;

  //check for missed starts - obsolete...
  // if (opt.ntof_period>0.01 && tt>opt.ntof_period) {
  //   crs->Tstart0+=Long64_t(1000*opt.ntof_period/opt.Period);
  //   tt = (tm - crs->Tstart0)*mks*opt.Period;
  // }

  return lambda;
}

void Mdef::Time_Extend(UChar_t ch, Double_t T) {
  if (!v_map[ch]) return;

  TH1F* hh = (TH1F*) v_map[ch]->hst;
  Double_t max = hh->GetXaxis()->GetXmax();

  if (T > max) {
    if (T - max > 1e4) { //larger than several hours
      cout << "Time leap is too large: " << crs->nevents << " " << T << " " << max << " " << crs->Tstart64 << endl;
    }
    else {
      Double_t rt = T/max;
      int nn=2;
      while (rt>2) {
	nn*=2;
	rt*=0.5;
      }
      max*=nn;
      int nbin = hh->GetNbinsX()*nn;
      for (auto it = v_map.begin();it!=v_map.end();++it) {
	if (*it) {
	  (*it)->hst->SetBins(nbin,0,max);
	  if (opt.ncuts) { //расширяем также все h_cuts
	    for (int i=1;i<opt.ncuts;i++) {
	      if ((*it)->h_cuts[i]) {
		(*it)->h_cuts[i]->hst->SetBins(nbin,0,max);
	      }
	    }
	  }
	}
      }

    }
  }
}

void Mdef::Time_Extend_2d(UChar_t ch, Double_t xx, Double_t yy) {
  //только по оси Y
  if (!v_map[ch]) return;

  TH1* hh = v_map[ch]->hst;
  if (hh->GetDimension()!=2) return;
  //bool change=false;

  int nx = hh->GetNbinsX();
  Double_t xmin = hh->GetXaxis()->GetXmin();
  Double_t xmax = hh->GetXaxis()->GetXmax();
  int ny = hh->GetNbinsY();
  Double_t ymin = hh->GetYaxis()->GetXmin();
  Double_t ymax = hh->GetYaxis()->GetXmax();

  if (my->hnum==21) {
    if (yy > ymax) {
      Double_t rt = yy/ymax;
      int nn=2;
      while (rt>2) {
	nn*=2;
	rt*=0.5;
      }
      ymax*=nn;
      ny*=nn;
      //change=true;
      for (auto it = v_map.begin();it!=v_map.end();++it) {
	if (*it) {
	  (*it)->hst->SetBins(nx,xmin,xmax,ny,ymin,ymax);
	  if (opt.ncuts) { //расширяем также все h_cuts
	    for (int i=1;i<opt.ncuts;i++) {
	      if ((*it)->h_cuts[i]) {
		(*it)->h_cuts[i]->hst->SetBins(nx,xmin,xmax,ny,ymin,ymax);
	      }
	    }
	  }
	}
      }
    }
  }

  // if (change) {
  //   for (auto it = v_map.begin();it!=v_map.end();++it) {
  //     if (*it)
  // 	(*it)->hst->SetBins(nx,xmin,xmax,ny,ymin,ymax);
  //   }
  // }

}

void Mdef::Fill_01(HMap* map, Float_t x, Double_t *hcut_flag, int ncut) {
  // заполняем 1d гистограмму и 1-мерный cut
  if (!map) return;
  if (ncut) {
    map=map->h_cuts[ncut];
    if (!map) return;
  }

  //double ww = map->hst->GetXaxis()->GetBinWidth(1);
  //double rr = ww*(rnd.Rndm(x)-0.5);
  //cout << "addrnd: " << x << " " << ww << " " << rr << endl;
  if (opt.addrandom) {
    x += map->hst->GetXaxis()->GetBinWidth(1)*(rnd.Rndm(x)-0.5);
  }
  map->hst->Fill(x);

  if (opt.ncuts && !ncut) {
    for (int i=1;i<opt.ncuts;i++) {
      //если в этой гистограмме задан cut i
      if (getbit(*(map->hd->cut+map->nn),i)) {
	if (x>=hcl->cutG[i]->GetX()[0] && x<hcl->cutG[i]->GetX()[1]) {
	  hcut_flag[i]=1;
	}
      }
    }
  }
}

void Mdef::Fill_02(HMap* map, Float_t x, Float_t y, Double_t *hcut_flag,
		   int ncut) {
  // заполняем 2d гистограмму и 2-мерный cut
  if (!map) return;
  if (ncut) {
    map=map->h_cuts[ncut];
    if (!map) return;
  }

  if (opt.addrandom) {
    x += map->hst->GetXaxis()->GetBinWidth(1)*(rnd.Rndm(x)-0.5);
    y += map->hst->GetYaxis()->GetBinWidth(1)*(rnd.Rndm(y)-0.5);
  }
  map->hst->Fill(x,y);

  if (opt.ncuts && !ncut) {
    for (int i=1;i<opt.ncuts;i++) {
      if (getbit(*(map->hd->cut+map->nn),i)) {
	if (hcl->cutG[i]->IsInside(x,y)) {
	  hcut_flag[i]=1;
	}
      }
    }
  }
}

void Mdef::Fill_1d(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<opt.Nchan && ipls->Pos>-32222) {
      // вызываем Fill_01 для данной гистограммы и группы, которой она принадлежит
      Float_t x = (this->*GetX)(evt,&*ipls);
      Fill_01(v_map[ipls->Chan],x,hcut_flag,ncut);

      for (int j=0;j<NGRP;j++)
	if (opt.Grp[ipls->Chan][j])
	  Fill_01(v_map[MAX_CH+j],x,hcut_flag,ncut);

    }
  }
}

void Mdef::Fill_1d_Extend(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<opt.Nchan && ipls->Pos>-32222) {
      // вызываем Fill_01 для данной гистограммы и группы, которой она принадлежит
      Float_t x = (this->*GetX)(evt,&*ipls);
      Time_Extend(ipls->Chan, x);
      Fill_01(v_map[ipls->Chan],x,hcut_flag,ncut);

      for (int j=0;j<NGRP;j++)
	if (opt.Grp[ipls->Chan][j])
	  Fill_01(v_map[MAX_CH+j],x,hcut_flag,ncut);

    }
  }
}

void Mdef::Fill_2d(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<opt.Nchan && ipls->Pos>-32222) {
      // вызываем Fill_02 для данной гистограммы

      Float_t x = (mx->*mx->GetX)(evt,&*ipls);
      Float_t y = (my->*my->GetX)(evt,&*ipls);

      Fill_02(v_map[ipls->Chan],x,y,hcut_flag,ncut);

      //YK!!!
      for (int j=0;j<NGRP;j++)
       	if (opt.Grp[ipls->Chan][j])
       	  Fill_02(v_map[MAX_CH+j],x,y,hcut_flag,ncut);

    }
  }
}

void Mdef::Fill_2d_Extend(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<opt.Nchan && ipls->Pos>-32222) {
      // вызываем Fill_02 для данной гистограммы

      Float_t x = (mx->*mx->GetX)(evt,&*ipls);
      Float_t y = (my->*my->GetX)(evt,&*ipls);

      Time_Extend_2d(ipls->Chan,x,y);
      Fill_02(v_map[ipls->Chan],x,y,hcut_flag,ncut);

      // for (int j=0;j<NGRP;j++)
      // 	if (opt.Grp[ipls->Chan][j])
      // 	  Fill_02(v_map[MAX_CH+j],x,y,hcut_flag);

    }
  }
}

void Mdef::Fill_axay(EventClass* evt, Double_t *hcut_flag, int ncut) {
  Float_t AA[MAX_CH+1] = {}; //initialize to zero
  int nmax = hd->bins2+1;

  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<nmax && ipls->Pos>-32222) {
      // вызываем Fill_02 для данной гистограммы

      AA[ipls->Chan] = 1e5+(mx->*mx->GetX)(evt,&*ipls);

      //YK!!! для axay-гистограмм Grp не имеет смысла... или имеет(?)
      // for (int j=0;j<NGRP;j++)
      //  	if (opt.Grp[ipls->Chan][j])
      //  	  Fill_02(v_map[MAX_CH+j],x,y,hcut_flag,ncut);

    }
  }

  for (int y=1;y<nmax;y++) {
    int k0 = y*(y-1)/2;
    for (int x=0;x<y;x++) {
      int ii = k0+x;
      if (AA[x] && AA[y])
	Fill_02(v_map[ii],AA[x]-1e5,AA[y]-1e5,hcut_flag,ncut);
    }
  }

}

void Mdef::Fill_HWRate(EventClass* evt, Double_t *hcut_flag, int ncut) {
  if (evt->Spin & 128) {
    if (evt->pulses.empty()) return;

    for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
      if (ipls->Chan<opt.Nchan) { //отсекаем канал 255
	double t1 = crs->fTime[ipls->Chan]*crs->sPeriod;
	double t2 = evt->Tstmp*crs->sPeriod;
	crs->fTime[ipls->Chan] = evt->Tstmp;

	// вызываем Time_Extend
	Time_Extend(ipls->Chan,t2);

	TH1* hh = v_map[ipls->Chan]->hst;
	double ww = hh->GetBinWidth(1);
	int bin1 = hh->FindFixBin(t1);

	// определяем, в какие бины гистограмм записывать счетчики
	std::vector<double> cnt;
	double sum=0;
	double low = hh->GetBinLowEdge(bin1);

	double ff=0;
	do {
	  ff = low+ww-t1;
	  cnt.push_back(ff);
	  //bin1++;
	  low+=ww;
	  t1=low;
	  sum+=ff;
	} while (low+ww<t2);
	ff = t2-low;
	if (ff>0) {
	  cnt.push_back(ff);
	  sum+=ff;
	}

	// записываем счетчики
	if (!cnt.empty()) {

	  Long64_t count2 = ipls->Counter - crs->fCounter[ipls->Chan];
	  crs->fCounter[ipls->Chan] = ipls->Counter;

	  int bin=bin1;
	  for (auto it=cnt.begin();it!=cnt.end();++it) {
	    double cc = *it/sum*count2;
	    v_map[ipls->Chan]->hst->AddBinContent(bin,cc);
	    bin++;
	  }
	}

      } //if (ipls->Chan<opt.Nchan)
    }
  }
}

void Mdef::FillMult(EventClass* evt, Double_t *hcut_flag, int ncut) {
  Float_t mult[NGRP+1] = {0};
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    // пропускаем импульсы с "плохим" Chan и где не найден пик
    if (ipls->Chan<opt.Nchan && ipls->Pos>-32222) {
      mult[NGRP]++;
      for (int j=0;j<NGRP;j++)
	if (opt.Grp[ipls->Chan][j])
	  mult[j]++;
    }
  }

  Fill_01(v_map[0],mult[NGRP],hcut_flag,ncut);

  for (int j=0;j<NGRP;j++) {
    if (v_map[MAX_CH+j])
      Fill_01(v_map[MAX_CH+j],mult[j],hcut_flag,ncut);
  }

}

void Mdef::Fill_Mean1(HMap* map,Float_t* Data,int nbins,int ideriv,int ncut) {
  if (!map) return;
  if (ncut) {
    map=map->h_cuts[ncut];
    if (!map) return;
  }
  if (nbins<=0) return;

  // if (map->hst->GetNbinsX() < nbins) {
  //   map->hst->SetBins(nbins,-cpar.Pre[ch],nbins-cpar.Pre[ch]);
  // }

  double nent=map->hst->GetEntries()/nbins;
  double val;

  if (ideriv==0) { //pulse
    for (auto j=0;j<nbins;j++) {
      val = map->hst->GetBinContent(j+1)*nent + Data[j];
      map->hst->SetBinContent(j+1,val/(nent+1));
    }
    //map->Nent++;
  }
  else { //deriv
    for (auto j=0;j<nbins;j++) {
      auto jk = j-ideriv;
      if (jk>=0 && jk<nbins)
	val=Data[j] - Data[jk];
      else
	val=0;
      val += map->hst->GetBinContent(j+1)*nent;
      map->hst->SetBinContent(j+1,val/(nent+1));
    }
    //map->Nent++;
  }
}

void Mdef::FillMeanPulse(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    int ch = ipls->Chan;
    if (ch<opt.Nchan && v_map[ch]) {
      int newsz = ipls->sData.size();

      TH1* hh = v_map[ch]->hst;
      if (hh->GetNbinsX() < newsz) {
      	hh->SetBins(newsz,-cpar.Pre[ch],newsz-cpar.Pre[ch]);
      }

      if (hnum==51) { //pulse
	Fill_Mean1(v_map[ch], ipls->sData.data(), newsz, 0, ncut);
      }
      else { //deriv
	Fill_Mean1(v_map[ch], ipls->sData.data(), newsz, opt.sDrv[ch], ncut);
      }

    }
  }
}

void Mdef::Fill_Ampl(EventClass* evt, Double_t *hcut_flag, int ncut) {
  for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
    int ch = ipls->Chan;
    if (ch<opt.Nchan) { // && v_map[ch] - не нужен, т.к. проверяется в Fill_01
      for (auto j=ipls->sData.begin();j!=ipls->sData.end();++j) {
	Fill_01(v_map[ch],*j,hcut_flag,ncut);
      }
    }
  }
}

void Mdef::FillProf(EventClass* evt, Double_t *hcut_flag, int ncut) {
  int ch_alpha=-1;
  Float_t h_sum[2][64] = {}; //[xy][pos], initialized to zero
  double tt;
  if (opt.Prof_type==64) { //new profilometer
    int a_x=-9999,a_y=-9999;
    for (auto ipls=evt->pulses.begin();ipls!=evt->pulses.end();++ipls) {
      Int_t pp = crs->prof_ch[ipls->Chan];
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
	  cout << "wrong prof channel: " << pp << " " << ipls->Chan << endl;
	} //switch

	if (h_xy>=0) {//one of Prof64 position channels
	  //PulseClass *pulse = &pulses[i];  
	  int dt=(ipls->Tstamp64-evt->Tstmp) - cpar.Pre[ipls->Chan] - evt->T0;
	  int size = ipls->sData.size();

	  for (int kk=-1;kk<31;kk++) {
	    int jj=h_off+kk*sgn;
	    int x1 = opt.Prof64_W[1] + opt.Prof64_W[0]*kk - dt;
	    int x2 = x1 + opt.Prof64_W[2];
	    int xmin = TMath::Max(x1,0);
	    int xmax = TMath::Min(x2,size);
	    if (xmax-xmin>0) {
	      for (int j=xmin;j<xmax;j++) {
		h_sum[h_xy][jj]+=ipls->sData[j];
	      }
	      h_sum[h_xy][jj]*=-1.0/(xmax-xmin);
	    }
	  } //for kk
	} //if xy>0  
      } //if pp>=crs->PROF_64

      else if (pp>=crs->ING_Y) {
	tt = (ipls->Time - evt->T0)*opt.Period+opt.sD[ipls->Chan];
	if (abs(tt)<=opt.Prof64_GAT) {
	  a_y=pp-crs->ING_Y;
	  //Fill1d(first,hcl->m_time,ipls->Chan-16,tt);
	}
      }
      else if (pp>=crs->ING_X) {
	tt = (ipls->Time - evt->T0)*opt.Period+opt.sD[ipls->Chan];
	if (abs(tt)<=opt.Prof64_GAT) {
	  a_x=pp-crs->ING_X;
	  //Fill1d(first,hcl->m_time,ipls->Chan-16,tt);
	}
      }
      // else if (pp>=crs->PROF_Y)
      // 	py=pp-crs->PROF_Y;
      // else if (pp>=crs->PROF_X)
      // 	px=pp-crs->PROF_X;

      //prnt("ss l l d ds;",BGRN,"Prof:",Nevt,Tstmp,ipls->Chan,pp,RST);

    } //for (auto ipls)

    if (opt.Ing_type==256) {
      ch_alpha = a_x + (opt.prof_ny-a_y-1)*opt.prof_ny;
    }
    else {
      ch_alpha = a_x;
    }

  } //if (opt.Prof_type==64) //new profilometer
  //else { //old profilometer
  //}

  if (hcl->mdef_prof->hd->b) {
    if (ch_alpha>=0 && ch_alpha<(int)hcl->mdef_prof->v_map.size()) {
      for (int i=0;i<64;i++) {
	if (h_sum[0][i]>opt.Prof64_THR) { //X
	  for (int j=0;j<64;j++) {
	    if (h_sum[1][j]>opt.Prof64_THR) { //Y
	      Fill_02(hcl->mdef_prof->v_map[ch_alpha],
		       (i+0.5)*1.875,(j+0.5)*1.875,hcut_flag,ncut);
	    } //if Y
	  }
	} //if X
      }
    }
  }

  if (hcl->mdef_prof_int->hd->b) {
    Fill_Mean1(hcl->mdef_prof_int->v_map[2], h_sum[0], 64, 0, ncut); //X
    Fill_Mean1(hcl->mdef_prof_int->v_map[3], h_sum[1], 64, 0, ncut ); //X

    for (int i=0;i<64;i++) {
      if (h_sum[0][i]>opt.Prof64_THR) {
	Fill_01(hcl->mdef_prof_int->v_map[0],i+0.5,hcut_flag,ncut);
	for (int j=0;j<64;j++) {
	  if (h_sum[1][j]>opt.Prof64_THR) {
	    Fill_02(hcl->mdef_prof_int->v_map[4],i+0.5,j+0.5,hcut_flag,ncut);
	    Fill_02(hcl->mdef_prof_int->v_map[5],(i+0.5)*1.875,(j+0.5)*1.875,hcut_flag,ncut);
	  } //if Y
	}
      } //if X
    }

    for (int i=0;i<64;i++) {
      if (h_sum[1][i]>opt.Prof64_THR) {
	Fill_01(hcl->mdef_prof_int->v_map[1],i+0.5,hcut_flag,ncut);
      } //if Y
    }
  }

} //FillProf

//------------------------------

HMap::HMap() {
  hst = 0;
  gr = 0;
  hd = 0;
  nn = 0;
  //Nent=0;
  //flg=0;
  memset(h_cuts,0,sizeof(h_cuts));
  //h_MT=0;
}

// HMap::HMap(const char* dname) : HMap() {
//   SetTitle(dname);
//   SetName(dname);
//   //cout << "HMAP: " << GetName() << " " << hst << endl;
// }

HMap::HMap(const char* dname, Hdef* hd1) : HMap() {
  hd = hd1;

  SetTitle(dname);
  SetName(dname);
  //memset(h_cuts,0,sizeof(h_cuts));
}

HMap::HMap(const char* dname, TH1* hist, Hdef* hd1, int i) : HMap() {
  hst = hist;
  hd = hd1;
  nn = i;

  SetTitle(dname);
  if (hist)
    SetName(hist->GetName());
  else
    SetName(dname);
  //memset(h_cuts,0,sizeof(h_cuts));
}

HMap::HMap(const char* dname, TGraphErrors* gr1, Hdef* hd1, int i) : HMap() {
  gr = gr1;
  hd = hd1;
  nn = i;

  SetTitle(dname);
  if (gr)
    SetName(gr->GetName());
  else
    SetName(dname);
  //memset(h_cuts,0,sizeof(h_cuts));
}

HMap::~HMap() {
  //cout << "~hmap: " << GetName() << " " << hst << endl;
  if (hst) {
    delete hst;
    hst=0;
  }
  if (gr) {
    delete gr;
    gr=0;
  }
  //cout << "~hmap1: " << GetName() << " " << hst << endl;
  //memset(cut_index,0,MAXCUTS);
  for (int i=0;i<MAXCUTS;i++) {
    if (h_cuts[i]) {
      delete h_cuts[i]->hst;
      h_cuts[i]->hst=0;
      delete h_cuts[i];
      h_cuts[i]=0;
    }
  }
  // if (h_MT) {
  //   delete h_MT->hst;
  //   h_MT->hst=0;
  //   delete h_MT;
  //   h_MT=0;
  // }
  //cout << "~hmap2: " << GetName() << endl;
}

HMap::HMap(const HMap& other) : TNamed(other) {
  hst = other.hst;
  gr = other.gr;
  hd = other.hd;
  nn = other.nn;
  //Nent = other.Nent;
  //flg = other.flg;

  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  //h_MT=other.h_MT;
}

//TH1F& TH1F::operator=(const TH1F &h1)
//RFive& operator=(const RFive& other)
HMap& HMap::operator=(const HMap& other) {
  hst = other.hst;
  gr = other.gr;
  hd = other.hd;
  nn = other.nn;
  //Nent = other.Nent;
  //flg = other.flg;

  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  //h_MT=other.h_MT;
  return *this;
}

//------------------------------


HClass::HClass()
{
  char ss[99];
  for (int i=0;i<MAXCUTS;i++) {
    cutG[i]=0;
    sprintf(ss,"form%d",i+1);
    cform[i]=new TFormula(ss,"0");
    strcpy(cuttitle[i],"");
  }

  //wfalse=false;
  map_list=NULL;
  allmap_list=NULL;
  dir_list=NULL;

  Make_Mlist();

}

HClass::~HClass()
{
}

//------------------------------

void HClass::Make_Mlist() {
  Mlist.clear();

  TList* l1 = TClass::GetClass("Toptions")->GetListOfDataMembers();
  TDataMember *d1 = (TDataMember*) l1->First();
  while (d1) {
    //Hdef с комментарием
    if (!strcmp(d1->GetTypeName(),"Hdef") && strlen(d1->GetTitle())) {
      Mdef md;

      md.hd = (Hdef*) ((char*)&opt + d1->GetOffset());

      md.h_name = d1->GetName();
      istringstream iss(d1->GetTitle());
      iss>>md.hnum;
      iss>>md.name;
      iss>>md.x_title;
      iss>>md.y_title;
      int pos = iss.tellg();
      if (pos>0)
	md.tip = iss.str().substr(pos+1);

      //YK!!!
      int nn = MAX_CH+NGRP;
      if (md.hnum==53) //prof
	nn = 256;
      else if (md.hnum==54) //prof_int
	nn=6;

      md.v_map.resize(nn);

      Mlist.push_back(md);
    }
    d1 = (TDataMember*)l1->After(d1);
  }

}

Mdef* HClass::Add_h2(int id1, int id2) {
  Mdef md;

  md.hnum = 100*id1+id2;

  mdef_iter m1 = Find_Mdef(id1);
  mdef_iter m2 = Find_Mdef(id2);

  if (m1==Mlist.end() || m2==Mlist.end()) return 0;

  md.name = m1->name + '_' +  m2->name;
  md.h_name = m1->name+'.'+m2->name;

  md.hd = new Hdef();

  //YK!!
  int nn = MAX_CH+NGRP; //different 1d histograms
  if (id1==id2) { //axay: same 1d histograms 
    nn = MAX_AXAY*MAX_AXAY+NGRP;
  }

  md.v_map.resize(nn);

  Mlist.push_back(md);

  return &Mlist.back();
}

bool check_Base(int num) {
  // проверяем, есть ли хоть одна 1d или 2d гистограмма с base,slope,RMS
  bool res=false;
  int nn[3];
  nn[0]=num;
  nn[1]=num/100;
  nn[2]=num%100;
  for (int i=0;i<3;i++) {
    if (nn[i]>=4 && nn[i]<=8) //4-base; 5,6-slope; 6,7-RMS
      res=true;
  }
  return res;
}

void HClass::Make_hist() {

  //crs->nchan_on = crs->CountChan();

  if (allmap_list)
    delete allmap_list;
  allmap_list = new TList();
  allmap_list->SetName("allmap_list");
  allmap_list->SetOwner(false);

  if (map_list)
    delete map_list;
  map_list = new TList();
  map_list->SetName("map_list");
  map_list->SetOwner(true);

  MFill_list.clear();
  Mdef* mprof=0;

  bool b_bbb=false; 
  memset(b_base,0,sizeof(b_base));

  Hdef* hd2=0;
  PulseClass pls;
  for (auto it = Mlist.begin();it!=Mlist.end();++it) {
    //prnt("ss d ss;",BGRN,"mkhst:",it->hnum,it->h_name.Data(),RST);

    //YK!! //Здесь можно обнулять v_map...
    memset(it->v_map.data(),0,it->v_map.size()*sizeof(HMap*));
    if (it->hnum>0 && it->hnum<10) {// standard pulse variable
      Make_1d(it,opt.Nchan);
      it->ptr = pls.GetPtr(it->hnum);
      it->GetX = &Mdef::VarPulse;
      it->MFill = &Mdef::Fill_1d;
    }
    else if (it->hnum==11) { //Time
      Make_1d(it,opt.Nchan);
      it->GetX = &Mdef::VarTime;
      it->MFill = &Mdef::Fill_1d;
      //YK: it->MFill_cut = &Mdef::Fill_1d_cut???; здесь и ниже недоделано
    }
    else if (it->hnum==12) { //nTOF
      //if (opt.b_ntof) {
      Make_1d(it,opt.Nchan);
      it->GetX = &Mdef::VarNtof;
      it->MFill = &Mdef::Fill_1d;
    }
    else if (it->hnum==13) { //etof
      //if (opt.b_ntof) {
      Make_1d(it,opt.Nchan);
      it->GetX = &Mdef::VarEtof;
      it->MFill = &Mdef::Fill_1d;
    }
    else if (it->hnum==14) { //ltof
      //if (opt.b_ntof) {
      Make_1d(it,opt.Nchan);
      it->GetX = &Mdef::VarLtof;
      it->MFill = &Mdef::Fill_1d;
    }
    else if (it->hnum==15) { // Period
      Make_1d(it,opt.Nchan);
    }
    else if (it->hnum==21) {//Rate
      Make_1d(it,opt.Nchan);
      it->GetX = &Mdef::VarRate;
      it->MFill = &Mdef::Fill_1d_Extend;
    }
    else if (it->hnum==22) {//HWRate
      Make_1d(it,opt.Nchan);
      it->MFill = &Mdef::Fill_HWRate;
    }
    else if (it->hnum==48) {//Ampl
      Make_1d(it,opt.Nchan);
      it->MFill = &Mdef::Fill_Ampl;
    }
    else if (it->hnum==49) {//Mult
      Make_1d(it,0);
      it->MFill = &Mdef::FillMult;
    }
    else if (it->hnum==51 || it->hnum==52) {//pulse
      Make_1d_pulse(it);
      it->MFill = &Mdef::FillMeanPulse;
    }
    else if (it->hnum==53) { //prof
      mdef_prof = &*it;
      hd2 = it->hd;
      Make_prof(it);
      it->MFill = &Mdef::FillProf;
    }
    else if (it->hnum==54) { //prof_int
      mdef_prof_int = &*it;
      Make_prof_int(it,hd2);
      it->MFill = &Mdef::FillProf;
    }
    else if (it->hnum>100) { //2d
      int res = Make_2d(it);
      if (res==1) { //normal 2d
	if (it->mx->hnum==21 || it->my->hnum==21)
	  it->MFill = &Mdef::Fill_2d_Extend;
	else
	  it->MFill = &Mdef::Fill_2d;
      }
      else if (res==2) { //axay
	it->MFill = &Mdef::Fill_axay;
      }
    }
    else {
      prnt("ss s ds;",BRED, "Wrong histogram type:", it->h_name.Data(),it->hnum, RST);
      exit(-1);
    }

    if (it->hd->b) {
      //добавляем все активные. Для профилометров - добавляем только одного
      if (it->hnum==53 || it->hnum==54) {
	if (mprof==0) {
	  mprof = &*it;
	  MFill_list.push_back(&*it);
	}
      }
      else
	MFill_list.push_back(&*it);

      if (check_Base(it->hnum)) b_bbb=true; 

      //cout << "Make_hist: " << it->hnum << " " << b_base << endl;
    }

  } //for (auto it = Mlist.begin()...

  if (b_bbb) memset(b_base,1,sizeof(b_base));

  for (auto i=0;i<opt.Nchan;i++) {
    if (opt.Mt[i]>=2) {
      b_base[i]=1;
    }
  }

  // оставляем только один член с профилометром в MFill_list
  // cout << "----------" << endl;
  // for (auto it = MFill_list.begin();it!=MFill_list.end();++it) {
  //   prnt("sss d ds;",BYEL,"MF_list: ",(*it)->h_name.Data(),(*it)->hd->b,(*it)->hnum,RST);
  // }

  b_formula=false;
  if (opt.ncuts)
    Make_cuts();

} //Make_hist

mdef_iter HClass::Find_Mdef(int id) {
  for (auto it = Mlist.begin();it!=Mlist.end();++it) {
    if (it->hnum==id) return it;
  }
  return Mlist.end();
}

void NameTitle(char* name, char* title, int i, int maxi, 
			  const char* nm, const char* axis) {
  if (maxi==0) {
    sprintf(name,"%s",nm);
    sprintf(title,"%s%s",nm,axis);
  }
  else {
    sprintf(name,"%s_%02d",nm,i);
    sprintf(title,"%s_%02d%s",nm,i,axis);
  }
}

void HClass::Make_1d(mdef_iter md, int maxi) {
  if (!md->hd->b) return;

  char name2[100];
  char title2[100];

  TString title = ';'+md->x_title+';'+md->y_title;
  TString name = md->name;
  name.ToLower();

  int NN=maxi;
  if (NN<1) NN=1;

  for (int i=0;i<NN;i++) {
    if (cpar.on[i]) {
      NameTitle(name2,title2,i,maxi,name.Data(),title.Data());
      int nn=md->hd->bins*(md->hd->max - md->hd->min);
      if (nn<1) nn=1;
      TH1F* hh=new TH1F(name2,title2,nn,md->hd->min,md->hd->max);
      md->v_map[i] = new HMap(md->name.Data(),hh,md->hd,i);
      map_list->Add(md->v_map[i]);
      allmap_list->Add(md->v_map[i]);
    }
  }

  for (int j=0;j<NGRP;j++) {
    for (int ch=0;ch<opt.Nchan;ch++) {
      if (opt.Grp[ch][j]) {
        sprintf(name2,"%s_g%d",name.Data(),j+1);
        sprintf(title2,"%s_g%d%s",name.Data(),j+1,title.Data());
        int nn=md->hd->bins*(md->hd->max - md->hd->min);
        if (nn<1) nn=1;
        TH1F* hh=new TH1F(name2,title2,nn,md->hd->min,md->hd->max);
	md->v_map[MAX_CH+j] = new HMap(md->name.Data(),hh,md->hd,MAX_CH+j);
        map_list->Add(md->v_map[MAX_CH+j]);
        allmap_list->Add(md->v_map[MAX_CH+j]);
        break;
      }
    }
  }

}

void HClass::Make_1d_pulse(mdef_iter md) {
  if (!md->hd->b) return;

  char name2[100];
  char title2[100];

  TString title = ';'+md->x_title+';'+md->y_title;
  TString name = md->name;
  name.ToLower();

  for (int i=0;i<opt.Nchan;i++) {
    if (cpar.on[i]) {
      NameTitle(name2,title2,i,opt.Nchan,name.Data(),title.Data());

      Float_t min = -cpar.Pre[i];
      Float_t max = cpar.Len[i]-cpar.Pre[i];
      Float_t bins = 1;

      int nn=bins*(max-min);
      TH1F* hh=new TH1F(name2,title2,nn,min,max);
      hh->Sumw2();

      md->v_map[i] = new HMap(md->name.Data(),hh,md->hd,i);
      map_list->Add(md->v_map[i]);
      allmap_list->Add(md->v_map[i]);
    }
  }
}

void HClass::Make_prof(mdef_iter md) {
  if (!md->hd->b) return;

  char name2[100];
  char title2[100];

  TString title = ';'+md->x_title+';'+md->y_title;
  TString name = "prof";

  //2d
  for (int j=opt.prof_ny-1;j>=0;j--) {
  //for (int j=0;j<opt.prof_ny;j++) {
    for (int k=0;k<opt.prof_nx;k++) {
      sprintf(name2,"%s_%d_%d",name.Data(),k+1,j+1);
      sprintf(title2,"%s_%d_%d%s",name.Data(),k+1,j+1,title.Data());

      int i=k+(opt.prof_ny-j-1)*opt.prof_ny;
      TH2F* hh=new TH2F(name2,title2,md->hd->bins,0,120,md->hd->bins2,0,120);

      md->v_map[i] = new HMap(md->name.Data(),hh,md->hd,i);
      map_list->Add(md->v_map[i]);
      allmap_list->Add(md->v_map[i]);
    }
  }

}

void HClass::Make_prof_int(mdef_iter md, Hdef* hd2) {
  if (!md->hd->b) return;
  if (!hd2) return;

  char name2[100];
  char title2[100];

  const char* name3[] = {"prof_x","prof_y","prof_ax","prof_ay","prof_nm","prof_xy"};

  for (int i=0;i<4;i++) {
    sprintf(name2,"%s",name3[i]);
    sprintf(title2,"%s;N strip",name3[i]);

    TH1F* hh=new TH1F(name2,title2,hd2->bins,0,hd2->bins);
    hh->Sumw2();
    hh->SetOption("E");

    md->v_map[i] = new HMap(md->name.Data(),hh,md->hd,i);
    map_list->Add(md->v_map[i]);
    allmap_list->Add(md->v_map[i]);
  }

  sprintf(name2,"%s",name3[4]);
  sprintf(title2,"%s%s",name3[4],";X(strip);Y(strip)");
  TH2F* hh=new TH2F(name2,title2,hd2->bins,0,64,hd2->bins2,0,64);
  md->v_map[4] = new HMap(md->name.Data(),hh,md->hd,4);
  map_list->Add(md->v_map[4]);
  allmap_list->Add(md->v_map[4]);

  sprintf(name2,"%s",name3[5]);
  sprintf(title2,"%s%s",name3[5],";X(mm);Y(mm)");
  hh=new TH2F(name2,title2,hd2->bins,0,120,hd2->bins2,0,120);
  md->v_map[5] = new HMap(md->name.Data(),hh,md->hd,5);
  map_list->Add(md->v_map[5]);
  allmap_list->Add(md->v_map[5]);

}

int HClass::Make_2d(mdef_iter md) {
  if (!(md->hd->b)) return 0;

  int id1 = md->hnum/100;
  int id2 = md->hnum%100;

  mdef_iter m1 = Find_Mdef(id1);
  mdef_iter m2 = Find_Mdef(id2);

  if (m1==Mlist.end() || m2==Mlist.end()) {
    prnt("ss s d ds;",BRED,"Can't make 2d histograms:",
	 md->name.Data() ,id1, id2, RST);
    return 0;
  }

  md->name = m1->name+'_'+m2->name;
  md->x_title = m1->x_title;
  md->y_title = m2->x_title;
  //md->tip = "2-dimensional histogram: "+md->name+"\nMin Max are taken from the corresponding 1d histograms";

  char name2[100];
  char title2[200];

  TString title = ';'+md->x_title+';'+md->y_title;
  TString name = md->name;
  name.ToLower();

  int n1=md->hd->bins*(m1->hd->max - m1->hd->min);
  int n2=md->hd->bins2*(m2->hd->max - m2->hd->min);
  if (n1<1) n1=1;
  if (n2<1) n2=1;

  md->mx = &*m1;
  md->my = &*m2;

  if (id1!=id2) { //normal 2d
  //-------------------
    for (int i=0;i<opt.Nchan;i++) {
      if (cpar.on[i]) {
	NameTitle(name2,title2,i,opt.Nchan,name.Data(),title.Data());
	  
	TH2F* hh=new TH2F(name2,title2,n1,m1->hd->min,m1->hd->max,
			  n2,m2->hd->min,m2->hd->max);

	md->v_map[i] = new HMap(md->name.Data(),hh,md->hd,i);
	map_list->Add(md->v_map[i]);
	allmap_list->Add(md->v_map[i]);
      }
    }
    for (int j=0;j<NGRP;j++) {
      for (int ch=0;ch<opt.Nchan;ch++) {
	if (opt.Grp[ch][j]) {
	  sprintf(name2,"%s_g%d",name.Data(),j+1);
	  sprintf(title2,"%s_g%d%s",name.Data(),j+1,title.Data());

	  TH2F* hh=new TH2F(name2,title2,n1,m1->hd->min,m1->hd->max,
			    n2,m2->hd->min,m2->hd->max);

	  md->v_map[MAX_CH+j] = new HMap(md->name.Data(),hh,md->hd,MAX_CH+j);
	  map_list->Add(md->v_map[MAX_CH+j]);
	  allmap_list->Add(md->v_map[MAX_CH+j]);

	  break;
	}
      }
    }
    return 1;
  } //normal 2d
  //-------------------
  else { //axay
    int nmax=md->hd->bins2+1;
    for (int y=1;y<nmax;y++) {
      int k0 = y*(y-1)/2;
      for (int x=0;x<y;x++) {
	if (cpar.on[x] && cpar.on[y]) {
	  int ii = k0+x;
	  name = m1->name+"2d";
	  name.ToLower();
	  sprintf(name2,"%s_%02d_%02d",name.Data(),x,y);
	  sprintf(title2,"%s%s",name2,title.Data());

	  int n1=md->hd->bins*(m1->hd->max - m1->hd->min);
	  if (n1<1) n1=1;

	  TH2F* hh=new TH2F(name2,title2,n1,m1->hd->min,m1->hd->max,
			    n1,m1->hd->min,m1->hd->max);
	  md->v_map[ii] = new HMap(md->name.Data(),hh,md->hd,ii);
	  map_list->Add(md->v_map[ii]);
	  allmap_list->Add(md->v_map[ii]);
	} //if cpar.on
      } //for x
    } //for y
    return 2;
  } //else

}

void HClass::FillHist(EventClass* evt, Double_t *hcut_flag) {
  // общие переменные для события
  //Double_t hcut_flag[MAXCUTS] = {0}; //признак срабатывания окон
  //int mult[NGRP+1] = {0};
  opt.T_acq=(evt->Tstmp/*- crs->Tstart64*/)*crs->sPeriod;
  
  // проверка opt.Tstop -> Нужно добавить это в fTimer (?)
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

  //заполняем все гистограммы в Actlist
  for (auto it = MFill_list.begin();it!=MFill_list.end();++it) {
    ((*it)->*(*it)->MFill)(evt,hcut_flag,0);
  }

  // "formula cuts"
  if (b_formula) {
    for (int i=1;i<opt.ncuts;i++) {
      if (opt.pcuts[i]==1) {//formula
	hcut_flag[i]=cform[i]->EvalPar(0,hcut_flag);
      }
      //cout << "cut_flag: " << Nevt << " " << i << " " << cut_flag[i] << endl;
    }
  }




  for (int i=1;i<opt.ncuts;i++) {
    if (hcut_flag[i]) {
      //заполняем все cut-гистограммы в Mainlist (YK???)
      for (auto it = Mainlist.begin();it!=Mainlist.end();++it) {
	//cout << "ML2: " << it->h_name << " " << endl;
	(*(&*it)->*(*(&*it))->MFill)(evt,hcut_flag,i);
      }
    }
  }

} //FillHist

void HClass::Clone_Hist(HMap* map) {
  char cutname[99];
  char name[99],htitle[99];
  // clone map->hist as many times as there are cuts; put it in map->h_cuts[i]
  //do the same with MT

  //if (*map->wrk) {
  TH1F* h0 = (TH1F*) map->hst;
  for (int i=1;i<opt.ncuts;i++) {
    if (opt.pcuts[i]) {
      sprintf(cutname,"MAIN_cut%d",i);
      sprintf(name,"%s_cut%d",h0->GetName(),i);
      sprintf(htitle,"%s_cut%d",h0->GetTitle(),i);
      TH1F* hcut = (TH1F*) h0->Clone();
      hcut->Reset();
      hcut->SetNameTitle(name,htitle);

      //hist_list->Add(hcut);
      HMap* mcut = new HMap(cutname,hcut,map->hd,map->nn);
      //mcut->flg=1; //cut flag

      // add this map to the list h_cuts
      map->h_cuts[i]=mcut;
      allmap_list->Add(mcut);
    }
    else {
      map->h_cuts[i]=0;
    } 
  }

  // if (crs->b_maintrig) {
  //   sprintf(cutname,"MAIN_MT");
  //   sprintf(name,"%s_MT",h0->GetName());
  //   sprintf(htitle,"%s_MT",h0->GetTitle());
  //   TH1* hcut = (TH1*) h0->Clone();
  //   hcut->Reset();
  //   hcut->SetNameTitle(name,htitle);
  //   //cout << "clone: " << i << " " << hcut->GetName() << " " << gStyle << endl;
  //   hist_list->Add(hcut);
  //   HMap* mcut = new HMap(cutname,hcut,map->chk,&wfalse,map->cut_index);

  //   // add this map to the list h_cuts
  //   map->h_MT=mcut;
  // }
  //}
}

void HClass::Remove_Clones(HMap* map) {
  // remove clones of map->hist from map->h_cuts[i] and from hist_list
  //cout << "Remove: " << map << " " << map->GetName() << endl;
  for (int i=0;i<MAXCUTS;i++) {
  //for (int i=0;i<opt.ncuts;i++) {
    //cout << i << " " << map->h_cuts[i] << endl;
    HMap* mcut = map->h_cuts[i];
    if (mcut) {
      //TH1* hcut = map->h_cuts[i]->hst;
      //hist_list->Remove(hcut);
      allmap_list->Remove(mcut);
      delete mcut;
      map->h_cuts[i]=0;
    }
    else
      break;
  }

  // HMap* mcut = map->h_MT;
  // if (mcut) {
  //   TH1* hcut = map->h_MT->hst;
  //   hist_list->Remove(hcut);
  //   delete mcut;
  //   map->h_MT=0;
  // }
  
}

void HClass::Make_cuts() {
  char cutname[99];

  opt.ncuts=0;
  for (int i=1;i<MAXCUTS;i++) {
    if (opt.pcuts[i])
      opt.ncuts=i+1;
  }

  //char name[99],htitle[99];

  // for (int i=0;i<MAXCUTS;i++) {
  //   cutcolor[i]=0;
  // }

  // loop through all maps
  // для каждой гистограммы (map) находим cuts, которые заданы в ней
  TIter next(map_list);
  TObject* obj=0;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;

    //determine cut titles and colors
    int icut=1;
    for (int i=1;i<opt.ncuts;i++) {
      if (getbit(*(map->hd->cut+map->nn),i)) {
	//if (getbit(*(map->cut_index),i)) {
	cutcolor[i]=icut+1;
	icut++;
	//cutcolor[i]+=1;
	sprintf(cuttitle[i],"%s",map->GetName());
      }
    }

    //clone histograms if MAIN flag is set
    if (*(map->hd->w+map->nn)) {
      //if (*map->wrk) {
      Remove_Clones(map);
      Clone_Hist(map);
    }
  
  } //while obj

  //make cuts
  for (int i=1;i<opt.ncuts;i++) {
    //cout << "cuts1: " << i << endl;
    if (cutG[i]) {
      delete cutG[i];
      cutG[i]=0;
    }
    
    sprintf(cutname,"%d",i);

    if (opt.pcuts[i]==1) { //formula
      //b_formula=true;
      cform[i]->SetTitle(opt.cut_form[i]);
      cform[i]->Clear();
      int ires = cform[i]->Compile();
      if (ires) {//formula is not valid -> set it to "always false"
	sprintf(opt.cut_form[i],"0");
	cform[i]->SetTitle("0");
	cform[i]->Clear();
	cform[i]->Compile();
      }
      else { //formula is valid
	b_formula=true;
      }
    }
    else { //normal cut (1d or 2d)
    }
    cutG[i] = new TCutG(cutname,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    cutG[i]->SetTitle(cuttitle[i]);
    cutG[i]->SetLineColor(cutcolor[i]);
  }
}
