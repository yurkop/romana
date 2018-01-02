//----- HClass ----------------
#include "romana.h"


extern Toptions opt;

extern CRS* crs;
extern ParParDlg *parpar;

extern ULong_t fGreen;
extern ULong_t fRed;
extern ULong_t fCyan;

//extern TH1F *hrms;
//extern TH1F *htdc_a[MAX_CH]; //absolute tof (start=0)

extern HistFrame* EvtFrm;

//TText txt;

//------------------------------

HClass::HClass()
{
  Make_hist();
}

HClass::~HClass()
{

}

void NameTitle(char* name, char* title, int i, int cc, 
			  const char* nm, const char* axis) {
  if (cc) {
    sprintf(name,"%s_%02d_cut%d",nm,i,cc);
    sprintf(title,"%s_%02d_cut%d%s_cut%d",nm,i,cc,axis,cc);
  }
  else {
    sprintf(name,"%s_%02d",nm,i);
    sprintf(title,"%s_%02d%s",nm,i,axis);
  }
}

void HClass::Make_hist() {

  //char title[100];
  char name[100];
  char title[100];

  for (int cc=0;cc<MAXCUTS;cc++) {
    for (int i=0;i<MAX_CH;i++) {
      //sprintf(name,"ampl_%02d",i);
      //sprintf(title,"ampl_%02d;Channel;Counts",i);
      NameTitle(name,title,i,cc,"ampl",";Channel;Counts");
      int nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
      h_ampl[i][cc]=new TH1F(name,title,nn,opt.amp_min,opt.amp_max);
    }

    for (int i=0;i<MAX_CH;i++) {
      //sprintf(name,"height_%02d",i);
      //sprintf(title,"height_%02d;Channel;Counts",i);
      NameTitle(name,title,i,cc,"height",";Channel;Counts");
      int nn=opt.hei_bins*(opt.hei_max-opt.hei_min);
      h_height[i][cc]=new TH1F(name,title,nn,opt.hei_min,opt.hei_max);
    }

    for (int i=0;i<MAX_CH;i++) {
      //sprintf(name,"time_%02d",i);
      //sprintf(title,"time_%02d;T(sec);Counts",i);
      NameTitle(name,title,i,cc,"time",";T(sec);Counts");
      int nn=opt.time_bins*(opt.time_max-opt.time_min);
      h_time[i][cc]=new TH1F(name,title,nn,opt.time_min,opt.time_max);
    }
  
    for (int i=0;i<MAX_CH;i++) {
      //sprintf(name,"tof_%02d",i);
      //sprintf(title,"tof_%02d;t(ns);Counts",i);
      NameTitle(name,title,i,cc,"tof",";t(ns);Counts");
      int nn=opt.tof_bins*(opt.tof_max-opt.tof_min);
      h_tof[i][cc]=new TH1F(name,title,nn,opt.tof_min,opt.tof_max);
    }

    for (int i=0;i<MAX_CH;i++) {
      //sprintf(name,"mtof_%02d",i);
      //sprintf(title,"mtof_%02d;t(mks);Counts",i);
      NameTitle(name,title,i,cc,"mtof",";t(mks);Counts");
      int nn=opt.mtof_bins*(opt.mtof_max-opt.mtof_min);
      h_mtof[i][cc]=new TH1F(name,title,nn,opt.mtof_min,opt.mtof_max);
    }

    /*
    for (int i=0; i<64; i++){
      //sprintf(name,"Profile_strip%02d",i);
      //sprintf(title,"Profile_strip%02d;X_strip;Y_strip",i);
      h2_prof_strip[i] = new TH2F(name,title,8,0,8,8,0,8);
    }

    for (int i=0; i<64; i++){
      //sprintf(name,"profile_real%02d",i);
      //sprintf(title,"Profile_real%02d;X (mm);Y (mm)",i);
      h2_prof_real[i] = new TH2F(name,title,8,0,120,8,0,120); 
    }
    */

    if (cc) {
      sprintf(name,"h2d_cut%d",cc);
      sprintf(title,"h2d_cut%d;Channel;Counts",cc);
    }
    else {
      sprintf(name,"h2d");
      sprintf(title,"h2d;Channel;Counts");
    }
    int nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
    h_2d[cc]=new TH2F(name,title,nn,opt.amp_min,opt.amp_max,
		  nn,opt.amp_min,opt.amp_max);
  } //for cc

}

void HClass::NewBins() {

  int nn;

  for (int cc=0;cc<MAXCUTS;cc++) {
    for (int i=0;i<MAX_CH;i++) {
      if (h_ampl[i][cc]) {
	nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
	h_ampl[i][cc]->SetBins(nn,opt.amp_min,opt.amp_max);
	h_ampl[i][cc]->Reset();
      }
    
      if (h_height[i][cc]) {
	nn=opt.hei_bins*(opt.hei_max-opt.hei_min);
	h_height[i][cc]->SetBins(nn,opt.hei_min,opt.hei_max);
	h_height[i][cc]->Reset();
      }

      if (h_time[i][cc]) {
	nn=opt.time_bins*(opt.time_max-opt.time_min);
	h_time[i][cc]->SetBins(nn,opt.time_min,opt.time_max);
	h_time[i][cc]->Reset();
      }
    
      if (h_tof[i][cc]) {
	nn=opt.tof_bins*(opt.tof_max-opt.tof_min);
	h_tof[i][cc]->SetBins(nn,opt.tof_min,opt.tof_max);
	h_tof[i][cc]->Reset();
      }

      if (h_mtof[i][cc]) {
	nn=opt.mtof_bins*(opt.mtof_max-opt.mtof_min);
	h_mtof[i][cc]->SetBins(nn,opt.mtof_min,opt.mtof_max);
	h_mtof[i][cc]->Reset();
      }
    }
    //for (int i=0; i<1; i++) {
    if (h_2d[cc]) {
      nn=opt.amp_bins*(opt.amp_max-opt.amp_min);
      h_2d[cc]->SetBins(nn,opt.amp_min,opt.amp_max,
			nn,opt.amp_min,opt.amp_max);
      h_2d[cc]->Reset();
    }
    //}
  }

  // for (int i=0; i<64; i++) {
  //   h2_prof_strip[i]->Reset();
  //   h2_prof_real[i]->Reset();
  // }

}

