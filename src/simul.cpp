#include "romana.h"
#include <TRandom.h>

extern Coptions cpar;
extern Toptions opt;
extern HClass* hcl;
extern CRS* crs;


Simul::Simul() {
  //if (opt.Nchan<4)
    opt.Nchan=4;

  opt.Period = opt.SimSim[0];

  for (int i=0;i<MAX_CHTP;i++) {
    cpar.Pre[i]=opt.SimSim[1];
    cpar.Len[i]=opt.SimSim[2];
    opt.St[i]=0;
  }

  opt.St[0]=1;

  //prnt("ss ds;",BGRN,"SimInit:",opt.h_time.b,RST);
  SimNameHist();

}

double Simul::Pshape_Gaus(int j, double pos) {
  // return opt.SimSim[4]*(-TMath::Gaus(j,pos-10,opt.SimSim[5]/opt.Period,1) +
  //  			TMath::Gaus(j,pos,opt.SimSim[5]/opt.Period,1));
  return opt.SimSim[4]*TMath::Gaus(j,pos,opt.SimSim[5]/opt.Period,1);
}

double Simul::Pshape_RC(int j, double pos) {
  double lam=0;
  if (opt.SimSim[6]) lam=opt.Period/opt.SimSim[6];

  if (j<pos)
    return 0;
  else if (j<=pos+opt.SimSim[5]/opt.Period)
    return opt.SimSim[4]*(1-exp((pos-j)*lam));
  else
    return opt.SimSim[4]*(1-exp(-opt.SimSim[5]/opt.Period*lam))
      * exp((pos+opt.SimSim[5]/opt.Period-j)*lam);
}

double Simul::Pshape_Fourier(int j, double pos) {

  double ret=0;
  if (opt.SimSim[4]) {
    ret += 100.0*cos(j*TMath::Pi()/opt.SimSim[4]);
  }
  // if (opt.SimSim[5]) {
  //   ret += 100.0*sin(j*TMath::Pi()*0.5/opt.SimSim[5]);
  // }
  // if (opt.SimSim[6]) {
  //   ret += 100.0*sin(j*TMath::Pi()*0.5/opt.SimSim[6]);
  // }


  //cout << "ret: " << j << " " << j*TMath::Pi() << " " << ret << endl;
  return ret;
  
}

void Simul::SimNameHist() {

  //cout << "smodule=" << module << endl;

  for (auto it = hcl->MFill_list.begin();it!=hcl->MFill_list.end();++it) {
    if ((*it)->name.EqualTo("time",TString::kIgnoreCase)) {
      for (auto map = (*it)->v_map.begin();map!=(*it)->v_map.end();++map) {
	if (*map) {
	  if ((*map)->nn==2)
	    (*map)->hst->SetTitle("Exact time0 - pos0");
	  else if ((*map)->nn==3)
	    (*map)->hst->SetTitle("Exact time1 - time0");

	  // cout << "mflist: " << (*it)->hnum << " " << (*map)->nn
	  //      << " " << (*map)->hst->GetTitle() << endl;
	}
      }
    }
    
  }

  // if (hcl->m_time[2]) {
  //   hcl->m_time[2]->hst->SetTitle("Exact time0 - pos0");
  // }

  // if (hcl->m_time[3]) {
  //   hcl->m_time[3]->hst->SetTitle("Exact time1 - time0");
  // }

}

void Simul::SimulatePulse(int ch, Long64_t tst, double pos) {
  PulseClass ipls=PulseClass();
  ipls.Chan=ch;
  ipls.Tstamp64=tst;
  
  ipls.sData.resize(cpar.Len[ch]);
  int type = opt.SimSim[3];
  switch (type) {
  case 0: //Gauss
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]=Pshape_Gaus(j,pos+cpar.Pre[ch]);
      //prnt("ss ds;",BGRN,"SSim3:",j,RST);
    }
    break;
  case 1: //RC
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]=Pshape_RC(j,pos+cpar.Pre[ch]);
    }
    break;
  case 2: //Fourier
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]=Pshape_Fourier(j,pos+cpar.Pre[ch]);
    }
    break;
  default:
    ;
  }

  // add noise
  if (opt.SimSim[11]) {
    for (int j=0;j<cpar.Len[ch];j++) {
      ipls.sData[j]+=opt.SimSim[11]*gRandom->Gaus(0,1);
    }
  }

  crs->PulseAna(ipls);

  // if (ipls.Chan==0)
  //   prnt("ss fs;",BRED,"Time1:",ipls.Time,RST);
  crs->Event_Insert_Pulse(&crs->Levents,&ipls);
  // if (ipls.Chan==0)
  //   prnt("ss fs;",BMAG,"Time2:",Levents.back().pulses[0].Time,RST);
}

void Simul::SimulateOneEvent(Long64_t Tst) {

  PulseClass pls;
  PulseClass *ipls;

  //time_00  - отклонение Time0 от pos0 (Time0-pos0)
  //simul_00 - отклонение Simul0 от pos0 (Simul0-pos0)
  //time_01  - разница между Time1 и Time0
  //simul_01 - разница между Simul1 и Simul0

  // реальная разница во времени между 2 импульсами (в ns)
  double delta = opt.SimSim[9]*gRandom->Rndm()-opt.SimSim[9]*0.5;
  delta/=opt.Period; //в сэмплах

  // положение p0 относительно дискриминатора (начала+Pre) (в нс)
  double pos0 = opt.SimSim[7]+opt.SimSim[8]*gRandom->Rndm();
  pos0/=opt.Period; //в сэмплах

  // положение p1 относительно p0 (в сэмплах)
  double pos1 = pos0 + delta;

  // (целая) разница между двумя импульсами в сэмплах
  Long64_t idelta = delta;
  pos1-=idelta;

  SimulatePulse(0, Tst, pos0);

  EventClass* evt = &crs->Levents.back();
  ipls=&evt->pulses[0];
  //prnt("ss 9l 8.5f 8.5f 8.5f ls;",BWHT,"Pos0:",evt->Tstmp,evt->T0,pos0,pos1,idelta,RST);

  SimulatePulse(1, Tst+idelta, pos1);

  ipls=&pls;
  ipls->Chan=2;
  ipls->Tstamp64=Tst;
  ipls->Pos=evt->pulses[0].Pos;
  //ipls->Time=delta+evt->T0;
  ipls->Time=opt.SimSim[10]/opt.Period+pos0;
  //YK ipls->Simul2=pos0;//pos0+evt->T0;
  //prnt("ss f f fs;",BGRN,"Sim:",ipls->Time,pos0,pos1,RST);

  crs->Event_Insert_Pulse(&crs->Levents,ipls);

  ipls->Chan=3;
  ipls->Time=delta+evt->T0;

  crs->Event_Insert_Pulse(&crs->Levents,ipls);

}

void Simul::SimulateEvents(Long64_t n_evts, Long64_t Tst0) {
  for (int i=0;i<n_evts;i++) {
    SimulateOneEvent((Tst0*n_evts+i)*10000);
    //prnt("ss l l ls;",BGRN,"Sim:",i,Tst0,(Tst0*n_evts+i)*100000,RST);
  }
  //prnt("ss ls;",BGRN,"Sim:",(Tst0*n_evts),RST);
}
