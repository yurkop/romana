#include "common.h"
#include "toptions.h"
#include "libcrs.h"
#include "hclass.h"
#include "libmana.h"
#include <iostream>

//extern Coptions cpar;
extern Toptions opt;
extern CRS* crs;
extern HClass* hcl;
extern MyMainFrame *myM;
//extern HistFrame* HiFrm;

namespace PROF {

  const int prof_ch[32] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    7, 6, 5, 4, 3, 2, 1, 0,
    7, 5, 3, 1, 6, 4, 2, 0,
    0, 2, 4, 6, 1, 3, 5, 7
  };

}

using namespace std;

PulseClass::PulseClass() {
  ptype=P_NOSTOP;
  //Nsamp=0;
  //Npeaks=0;
  //Analyzed=false;
  Tstamp64=0;
  State=0;
  //tdif=99;
  //sData=NULL;
  //Peaks=NULL;
}

void PulseClass::FindPeaks() {
  // always K-th derivative (D) is used
  // T1 - D>0, D(-1)<=0 (left zero-crossing of D)
  // T2 - D<=0, d(-1)>0 (right zero-crossing of D)
  // Pos - Maximum between T1 and T2
  /*
  // peak is the first maximum in deriv, above the threshold
  // if deadtime=0 - next peak is searched only after deriv crosses zero
  // if deadtime!=0 - next peak is searched after 
  // N=deadtime samples from the previous peak
  */
  
  UInt_t kk=opt.kdrv[Chan];
  if (kk<1 || kk>=sData.size()) kk=1;

  peak_type *pk=0;
  peak_type *p_prev=0;

  bool in_peak=false;
  //Float_t D,pD=0;//deriv, prev.dreiv
  Float_t* D = new Float_t[sData.size()]();
  //int peakpos[DSIZE];
  Float_t jmax=0;
  //int pp0=0; //temporary T1

  //D[0]=0;
  UInt_t j;
  for (j=kk;j<sData.size();j++) {
    D[j]=sData[j]-sData[j-kk];
    if (!in_peak) {
      if (D[j] >= opt.thresh[Chan]) {
	in_peak=true;
	Peaks.push_back(peak_type());
	pk = &Peaks.back();
	pk->T1=0;
	for (int n=j;n>0;n--) {
	  if (D[n]>0 && D[n-1]<=0) {
	    pk->T1=n;
	    break;
	  }
	}
	jmax=D[j];
	pk->Pos=j;
      }
    }
    else { //in_peak
      if (D[j]>jmax) {//maximum of D -> peak position
	jmax=D[j];
	pk->Pos=j;
	//printf("in_peak: %lld %d %d %d %0.1f %0.1f\n",crs->nevents,Chan,j,kk,D[j],jmax);
	// peakpos[Npeaks]=jmax=j-1;
	// Npeaks++;
	// if (deadtime) {
	//   j+=deadtime;
	//   in_peak=false;
	//   continue;
	// }
      }
      else if (D[j]<=0) { //zero crossing -> end of the peak
	in_peak=false;
	pk->T2=j-1;
	//cout << "T2: " << sData[pk->T2]-sData[pk->T2-kk] << endl;
	//pk->Height=jmax;
	jmax=0;
	if (Peaks.size()>1) { //this is at least second peak
	  p_prev = pk-1;
	  if (pk->Pos - p_prev->Pos < opt.deadT[Chan])
	    Peaks.pop_back();
	  else if (pk->Pos - p_prev->Pos < opt.pile[Chan]) {
	    p_prev->Type|=P_PILE1;
	    pk->Type|=P_PILE2;
	  }
	}
      }
    }
    //pD=D;
  }

  if (in_peak) { //end of the pulse -> peak has no end
    pk->T2=sData.size();
    //pk->Type|=P_B22;
  }
  
  //for (UInt_t i=0;i<Peaks.size();i++) {
  //cout << "FindPeaks: " << crs->nevents << " " << i << " " 
  //	 << Peaks.at(i).Pos << endl;
  //}

  // Peaks = new peak_type[Npeaks];
  // for (int i=0;i<Npeaks;i++) {
  //   Peaks[i].Pos = peakpos[i];
  // }

  //printf("FindPeaks: %d\n",Npeaks);

  delete[] D;

}

//-----------------------------

void PulseClass::PeakAna() {

  for (UInt_t i=0;i<Peaks.size();i++) {
    peak_type *pk = &Peaks[i];

    int sz=sData.size()-1;
    
    //peak time & position
    //int nt=0;
    Float_t sum=0;
    pk->Time=0;
    int tt; //reference window (timing mode)
    //int t3; //start window for timing
    //int t4; //end window for timing

    if (opt.timing[Chan]==0) 
      tt=pk->Pos; //0->reference is Pos (maximum in 1st deriv)
    else if (opt.timing[Chan]==1)
      tt=pk->T1; //1->reference is T1 (Left zero crossing of 1st drv)
    else //if (opt.timing[Chan]==2)
      tt=pk->T2; //2->reference is T2 (Right zero crossing of 1st drv)

    if (opt.twin1[Chan] == 99)
      pk->T3=pk->T1;
    else
      pk->T3=tt+opt.twin1[Chan];

    if (opt.twin2[Chan] == 99)
      pk->T4=pk->T2;
    else
      pk->T4=tt+opt.twin2[Chan];

    UInt_t kk=opt.kdrv[Chan];
    if (kk<1 || kk>=sData.size()) kk=1;

    if (pk->T3<(int)kk) {pk->T3=kk; pk->Type|=P_B11;}
    if (pk->T3>sz) {pk->T3=sz; pk->Type|=P_B11;}
    if (pk->T4<(int)kk) {pk->T4=kk; pk->Type|=P_B22;}
    if (pk->T4>sz) {pk->T4=sz; pk->Type|=P_B22;}

    //if (t4>(int)kk) {t3=kk; pk->Type|=P_B111;}

    for (int j=pk->T3;j<pk->T4;j++) {
      Float_t dif=sData[j]-sData[j-kk];
      pk->Time+=dif*j;
      sum+=dif;
      //nt++;
    }
    if (sum>1e-5)
      pk->Time/=sum;

    //pk->Time+=cpar.preWr[Chan];
    //cout << "TTT: " << t3 << " " << t4 << " " << pk->Time << " " << pk->T2
    // << " " << kk << endl;

    pk->Pos = pk->Time+0.5;

    //cout << "pk: " << (int) Chan << " " << pk->T3 << " " << pk->T4 << " " << pk->Time << " " << pk->Pos << endl;

    pk->B1=pk->Pos+opt.bkg1[Chan];
    pk->B2=pk->Pos+opt.bkg2[Chan];
    pk->P1=pk->Pos+opt.peak1[Chan];
    pk->P2=pk->Pos+opt.peak2[Chan];

    if (pk->B1<0) pk->B1=0;
    if (pk->B2<0) pk->B2=0;
    if (pk->P1<0) {pk->P1=0; pk->Type|=P_B1;}
    if (pk->P2<0) {pk->P2=0; pk->Type|=P_B2;}

    if (pk->B1>sz) pk->B1=sz;
    if (pk->B2>sz) pk->B2=sz;
    if (pk->P1>sz) {pk->P1=sz; pk->Type|=P_B1;}
    if (pk->P2>sz) {pk->P2=sz; pk->Type|=P_B2;}

    Float_t bkg=0;
    int nbkg=0;
    //background
    for (int j=pk->B1;j<=pk->B2;j++) {
      bkg+=sData[j];
      nbkg++;
    }
    if (nbkg)
      bkg/=nbkg;
    else {
      cout << "zero background!!!: " << this->Tstamp64 << " " << nbkg << " " << pk->B1 << " " << pk->B2 << endl;
    }

    int nn=0;
    //peak Area & Height
    pk->Area=0;
    pk->Height=0;
    for (int j=pk->P1;j<=pk->P2;j++) {
      pk->Area+=sData[j];
      if (sData[j]>pk->Height) pk->Height = sData[j];
      nn++;
    }
    if (nn) {
      pk->Area/=nn;
    }
    else {
      cout << "zero Area: " << this->Tstamp64 << " " << pk->Pos << " " << pk->P1 << " " << pk->P2 << endl;
    }
    pk->Area-=bkg;

  }

}

EventClass::EventClass() {
  State=0;
  T=0;
  T0=99999;
  //Analyzed=false;
}

void EventClass::Pulse_Ana_Add(PulseClass *pls) {

  for (UInt_t i=0;i<pulses.size();i++) {
    if (pls->Chan == pulses[i].Chan &&
	TMath::Abs(pls->Tstamp64-pulses[i].Tstamp64) < opt.tveto) {
      return;
    }
  }

  if (pls->State) {
    State=1;
    //cout << "state: " << Nevt << " " << (int) State << endl;
  }

  pulses.push_back(*pls);

  if (T==0) { //this is the first pulse in event
    T=pls->Tstamp64;
  }
  else if (pls->Tstamp64 < T) { //event exists & new pulse is earlier
    // -> correct T and T0
    if (T0<99998)
      T0+= T - pls->Tstamp64;
    T=pls->Tstamp64;
  }

  if (opt.Start[pls->Chan]) {
    if (pls->Peaks.size()) {
      Float_t dt = pls->Tstamp64 - T;

      peak_type *pk = &pls->Peaks.front();
      Float_t T2 = pk->Time - crs->Pre[pls->Chan] + dt;
      //Float_t T2 = pk->Time - cpar.preWr[pls->Chan] + dt;
      //Float_t T2 = pk->Time + dt;

      if (T2<T0) {
	T0=T2;
      }
      // cout << "Peak: " << (int) pls->Chan << " " << pk->Time << " " 
      // 	   << pk->Pos << " " << T0 << " " << T2 << " " << dt << " "
      // 	   << T << endl;
    }
  }

  // cout << "Evt: " << Nevt << ":";
  // for (UInt_t i=0;i<pulses.size();i++) {
  //   cout << " " << (int)pulses.at(i).Chan<< "," << pulses.at(i).Tstamp64-crs->Tstart64;
  // }
  // cout << endl;
  
}

void EventClass::Fill_Time_Extend(HMap* map) {
  TH1F* hh = (TH1F*) map->hst;
  Double_t max = hh->GetXaxis()->GetXmax();

  if (opt.T_acq > max) {
    max*=2;
    if (opt.T_acq>max) {
      cout << "Time leap is too large: " << this->Nevt << " " << opt.T_acq << " " << crs->Tstart64 << endl;
    }
    else {
      int nbin = hh->GetNbinsX()*2;
      Float_t* arr = new Float_t[nbin+2];
      memset(arr,0,sizeof(Float_t)*(nbin+2));
      Float_t* arr2 = hh->GetArray();
      memcpy(arr,arr2,hh->GetSize()*sizeof(Float_t));
      hh->SetBins(nbin,0,max);
      hh->Adopt(nbin+2,arr);
    }
  }
}

void EventClass::Fill1d(Bool_t first, HMap* map, Float_t x) {
  if (first) {
    //cout << "fill: " << map->hst->GetName() << endl;
    map->hst->Fill(x);
    if (opt.ncuts) {
      for (int i=0;i<MAXCUTS;i++) {
	int icut=map->cut_index[i]-1;
	if (icut<0)
	  break;
	//cout << "cut: " << i << " " << icut << " " << hcl->cutG[icut] << endl;
	if (x>=hcl->cutG[icut]->GetX()[0] && x<hcl->cutG[icut]->GetX()[1]) {
	  hcl->cut_flag[icut+1]=true;
	}
      }
    }
  }
  else if (*(map->wrk)) {
    for (int i=0;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i+1])
	map->h_cuts[i]->hst->Fill(x);      
    }
  }
}

void EventClass::Fill2d(Bool_t first, HMap* map, Float_t x, Float_t y) {
  if (first) {
    map->hst->Fill(x,y);
    if (opt.ncuts) {
      for (int i=0;i<MAXCUTS;i++) {
	int icut=map->cut_index[i]-1;
	if (icut<0)
	  break;
	//cout << "cut: " << i << " " << icut << " " << hcl->cutG[icut] << endl;
	if (hcl->cutG[icut]->IsInside(x,y)) {
	  hcl->cut_flag[icut+1]=true;
	}
      }
    }
  }
  else if (*(map->wrk)) {
    for (int i=0;i<opt.ncuts;i++) {
      if (hcl->cut_flag[i+1])
	map->h_cuts[i]->hst->Fill(x,y);      
    }
  }
}

void EventClass::FillHist(Bool_t first) {
  double DT = crs->period*1e-9;
  //int ch[MAX_CH];
  Double_t tt;
  //Double_t max,max2;
  //int nbin;

  //int icut=0;
  int mult=0;
  Long64_t tm;

  int nn=0;
  Float_t amp[2] = {0,0};

  //cout << "FillHist: " << this->Nevt << endl;

  if (first) {
    opt.T_acq=(T-crs->Tstart64)*DT;

    if (opt.Tstop && opt.T_acq>opt.Tstop) {
      crs->b_stop=true;
      crs->b_fana=false;
      crs->b_acq=false;
      //return;
    }
    if (opt.ncuts)
      memset(hcl->cut_flag,0,sizeof(hcl->cut_flag));
  }

  for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;

    for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
      peak_type* pk = &pulses[i].Peaks[j];

      if (first) {
	if (opt.dec_write) {
	  crs->rP.Area   = pk->Area;
	  crs->rP.Height = pk->Height;
	  crs->rP.Width  = pk->Width ;
	  crs->rP.Time   = pk->Time  ;
	  crs->rP.Ch     = ch    ;
	  crs->rP.Type   = pk->Type  ;

	  crs->rPeaks.push_back(crs->rP);
	}
      }

      if (opt.b_time) {
	if (first)
	  Fill_Time_Extend(hcl->m_time[ch]);
	Fill1d(first,hcl->m_time[ch],opt.T_acq);
      }

      if (opt.b_amp) {
	Fill1d(first,hcl->m_ampl[ch],pk->Area*opt.emult[ch]);
      }

      if (opt.b_hei) {
	Fill1d(first,hcl->m_height[ch],pk->Height);
      }

      if (opt.b_tof) {
	double dt = pulses[i].Tstamp64 - T;
	tt = pk->Time - crs->Pre[ch] - T0 + dt;
	Fill1d(first,hcl->m_tof[ch],tt*crs->period);
      }

      if (j==0) { //only for the first peak
	if (opt.b_mtof) {
	  if (ch==opt.start_ch) {
	    crs->Tstart0 = pulses[i].Tstamp64 + pk->Pos;
	  }
	  if (opt.Mt[ch]) {
	    mult++;
	  }
	  if (mult && (i==pulses.size()-1)) { //last pulse
	    if (crs->Tstart0>0) {
	      if (mult>=MAX_CH) mult=MAX_CH-1;

	      tm = pulses[i].Tstamp64 + pk->Pos;
	      tt = (tm - crs->Tstart0)*0.001*crs->period;
	      if (opt.mtof_period>0.01 && tt>opt.mtof_period) {
		crs->Tstart0+=opt.mtof_period;
		tt = (tm - crs->Tstart0)*0.001*crs->period;
	      }

	      Fill1d(first,hcl->m_mtof[mult],tt);
	      Fill1d(first,hcl->m_mtof[0],tt);
	    }
	  } //if last pulse
	} //if b_mtof

	if (opt.b_h2d) {
	  if (ch==0) {
	    amp[0]=pk->Area*opt.emult[ch];
	    nn++;
	  }
	  if (ch==1) {
	    amp[1]=pk->Area*opt.emult[ch];
	    nn++;
	  }
	  if (nn==2) {
	    Fill2d(first,hcl->m_2d[0],amp[0],amp[1]);
	    nn++;
	  }
	}

	if (opt.b_per) {
	  tm = pulses[i].Tstamp64 + pk->Pos;
	  if (hcl->T_prev[ch]) {
	    tt = (tm - hcl->T_prev[ch])*0.001*crs->period; //convert to mks
	    Fill1d(first,hcl->m_per[ch],tt);
	  }
	  hcl->T_prev[ch]=tm;
	}
      } // only for 1st peak

    } //for peaks...

  } //for (UInt_t i=0;i<pulses.size()...

  /*
    if (opt.dec_write) {
    crs->rTime=T;
    crs->rState = State;
    crs->Tree->Fill();
    crs->rPeaks.clear();
    }
  */

  /*
    int ax=0,ay=0,px=0,py=0;
    if (pulses.size()==4) {
    for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;
    if (ch<8) { //prof_x
    px=PROF::prof_ch[ch];
    }
    else if (ch<16) { //prof y
    py=PROF::prof_ch[ch];
    }
    else if (ch<24) { //alpha y
    ay=PROF::prof_ch[ch];
    }
    else { //alpha x
    ax=PROF::prof_ch[ch];
    }
    }

    int ch_alpha = ax + ay*8;

    //cout << "prof: " << crs->nevents << " " << ch_alpha << endl;

    hcl->h2_prof_strip[ch_alpha]->Fill(px,py);
    hcl->h2_prof_real[ch_alpha]->Fill(px*15,py*15);    
    }
  */

  if (first && hcl->b_formula) {
    for (int i=0;i<opt.ncuts;i++) {
      if (opt.pcuts[i]==1) {//formula
	hcl->cut_flag[i+1]=hcl->cform[i+1]->EvalPar(0,hcl->cut_flag);
      }
      //cout << "cut_flag: " << Nevt << " " << i << " " << hcl->cut_flag[i+1] << endl;
    }
  }

}

/*
void EventClass::FillHist_old() {
  double DT = crs->period*1e-9;
  //int ch[MAX_CH];
  Double_t tt;
  Double_t max,max2;
  int nbin;

  int icut=0;
  int mult=0;
  Long64_t tm;

  //static Long64_t T_prev[MAX_CH];

  //cout << "FillHist: " << endl;

  if (opt.b_h2d) {
    if (pulses.size()>=2) {
      Float_t amp[2];
      int nn=0;
      for (UInt_t i=0;i<pulses.size();i++) {
	int ch = pulses[i].Chan;
	for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
	  peak_type* pk = &pulses[i].Peaks[j];

	  if (ch==0 && j==0) {
	    amp[0]=pk->Area*opt.emult[ch];
	    nn++;
	  }
	  if (ch==1 && j==0) {
	    amp[1]=pk->Area*opt.emult[ch];
	    nn++;
	  }

	}
      }
      if (nn==2) {
	if (opt.ncuts) {
	  // for (int j=0;j<opt.ncuts;j++) {
	  //   if (hcl->cutG[j]->IsInside(amp[0],amp[1])) {
	  //     icut=j+1;
	  //     break;
	  //   }
	  // } 
	}

	hcl->h_2d[0][0]->Fill(amp[0],amp[1]);
	if (icut) {
	  hcl->h_2d[0][icut]->Fill(amp[0],amp[1]);
	}
      }
    }
  }

  for (UInt_t i=0;i<pulses.size();i++) {
    int ch = pulses[i].Chan;

    for (UInt_t j=0;j<pulses[i].Peaks.size();j++) {
      peak_type* pk = &pulses[i].Peaks[j];

      if (opt.b_amp) {
	hcl->h_ampl[ch][0]->Fill(pk->Area*opt.emult[ch]);
	if (icut) {
	  hcl->h_ampl[ch][icut]->Fill(pk->Area*opt.emult[ch]);
	}
      }

      if (opt.b_hei) {
	hcl->h_height[ch][0]->Fill(pk->Height);
	if (icut) {
	  hcl->h_height[ch][icut]->Fill(pk->Height);
	}
      }

      tt = pulses[i].Tstamp64 - crs->Tstart64;
      tt += pk->Pos;
      opt.T_acq=tt*DT;

      if (opt.Tstop && opt.T_acq>opt.Tstop) {
	//cout << "Tstop: " << opt.T_acq << " " << opt.Tstop << " " << myM->fStart << endl;
	crs->b_stop=true;
	crs->b_fana=false;
	crs->b_acq=false;
	//myM->fAna->Emit("Clicked()");
      }

      if (opt.b_time) {
	max = hcl->h_time[ch][0]->GetXaxis()->GetXmax();

	if (opt.T_acq > max) {
	  max2=max*2;
	  if (opt.T_acq>max2) {
	    cout << "Time leap is too large: " << this->Nevt << " " << ch << " " << opt.T_acq << " " << pulses[i].Tstamp64 << " " << crs->Tstart64 << endl;
	  }
	  else {
	    nbin = hcl->h_time[ch][0]->GetNbinsX()*max2/max;
	    Float_t* arr = new Float_t[nbin+2];
	    memset(arr,0,sizeof(Float_t)*(nbin+2));
	    Float_t* arr2 = hcl->h_time[ch][0]->GetArray();
	    memcpy(arr,arr2,hcl->h_time[ch][0]->GetSize()*sizeof(Float_t));
	    hcl->h_time[ch][0]->SetBins(nbin,0,max2);
	    hcl->h_time[ch][0]->Adopt(nbin+2,arr);
	  }

	}

	hcl->h_time[ch][0]->Fill(opt.T_acq);
	if (icut) {
	  hcl->h_time[ch][icut]->Fill(opt.T_acq);
	}
      }

      if (opt.b_tof) {
	double dt = pulses[i].Tstamp64 - T;
	tt = pk->Time - crs->Pre[ch] - T0 + dt;
	hcl->h_tof[ch][0]->Fill(tt*crs->period);
	if (icut) {
	  hcl->h_tof[ch][icut]->Fill(tt*crs->period);
	}
      }

      if (opt.dec_write) {
	crs->rP.Area   = pk->Area;
	crs->rP.Height = pk->Height;
	crs->rP.Width  = pk->Width ;
	crs->rP.Time   = pk->Time  ;
	crs->rP.Ch     = ch    ;
	crs->rP.Type   = pk->Type  ;

	crs->rPeaks.push_back(crs->rP);
      }

      if (j==0) { //only for the first peak
	if (opt.b_mtof) {
	  if (ch==opt.start_ch) {
	    crs->Tstart0 = pulses[i].Tstamp64 + pk->Pos;
	  }
	  if (opt.Mt[ch]) {
	    mult++;
	  }
	  if (mult && (i==pulses.size()-1)) { //last pulse
	    if (crs->Tstart0>0) {
	      if (mult>=MAX_CH) mult=MAX_CH-1;

	      tm = pulses[i].Tstamp64 + pk->Pos;
	      tt = (tm - crs->Tstart0)*0.001*crs->period;

	      hcl->h_mtof[mult][0]->Fill(tt);
	      hcl->h_mtof[0][0]->Fill(tt);
	      if (icut) {
		hcl->h_mtof[mult][icut]->Fill(tt);
		hcl->h_mtof[0][0]->Fill(tt);
	      }
	    }
	  } //if last pulse
	} //if b_mtof

	if (opt.b_per) {
	  tm = pulses[i].Tstamp64 + pk->Pos;
	  if (hcl->T_prev[ch]) {
	    tt = (tm - hcl->T_prev[ch])*0.001*crs->period; //convert to mks
	    hcl->h_per[ch][0]->Fill(tt);
	    if (icut) {
	      hcl->h_per[ch][icut]->Fill(tt);
	    }
	  }
	  hcl->T_prev[ch]=tm;
	  // if (ch==30) {
	  //   cout << "Prev: " << ch << " " << tt << endl;
	  // }
	}
      } // only for 1st peak

    } //for peaks...

  } //for (UInt_t i=0;i<pulses.size()...

  // int ax=0,ay=0,px=0,py=0;
  // if (pulses.size()==4) {
  //   for (UInt_t i=0;i<pulses.size();i++) {
  //     int ch = pulses[i].Chan;
  //     if (ch<8) { //prof_x
  // 	px=PROF::prof_ch[ch];
  //     }
  //     else if (ch<16) { //prof y
  // 	py=PROF::prof_ch[ch];
  //     }
  //     else if (ch<24) { //alpha y
  // 	ay=PROF::prof_ch[ch];
  //     }
  //     else { //alpha x
  // 	ax=PROF::prof_ch[ch];
  //     }
  //   }

  //   int ch_alpha = ax + ay*8;

  //   //cout << "prof: " << crs->nevents << " " << ch_alpha << endl;

  //   hcl->h2_prof_strip[ch_alpha]->Fill(px,py);
  //   hcl->h2_prof_real[ch_alpha]->Fill(px*15,py*15);    
  // }

}
*/
void PulseClass::Smooth(int nn) {

  //sData = new double[nsamp];
  //memset(sData,0,nsamp*sizeof(double));

  int Nsamp = sData.size();

  for (int i=0;i<Nsamp;i++) {
    //int ll=1;
    for (int j=1;j<=nn;j++) {
      int k=i+j;
      if (k<Nsamp) {
	sData[i]+=sData[k];
	//ll++;
      }
    }

    int ndiv;
    if (Nsamp-i <= nn) {
      ndiv=Nsamp-i;
    }
    else {
      ndiv=nn+1;
    }

    sData[i]/=ndiv;
    //printf("Smooth: %d %d %d %d %f\n",i,nsamp-i, opt.nsmoo, ndiv, sData[i]);

  }

}


void PulseClass::PrintPulse(int pdata) {
  printf("Pulse: %2d %2d %6ld %10lld %10lld\n",Chan,ptype,sData.size(),Counter,Tstamp64-crs->Tstart64);
  if (pdata) {
    for (int i=0;i<(int)sData.size();i++) {
      printf("-- %d %f\n",i,sData[i]);
    }
  }
}
