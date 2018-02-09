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

HMap::HMap(const char* dname, TH1* hist, Bool_t* s, Bool_t* w,
	   Char_t *cuts) {
  hst = hist;
  chk = s;
  wrk = w;
  //memcpy(cut_index,cuts,sizeof(cut_index));
  cut_index = cuts;
  //cout << "index: " << (void*) cuts << " " << (void*) cut_index << endl;
  SetName(hist->GetName());
  SetTitle(dname);
  memset(h_cuts,0,sizeof(h_cuts));
  //list_cuts = new TList();
  //list_h_cuts = new TList();
}

HMap::~HMap() {
  delete hst;
  hst=0;
  //memset(cut_index,0,MAXCUTS);
  for (int i=0;i<MAXCUTS;i++) {
    if (h_cuts[i]) {
      delete h_cuts[i]->hst;
      h_cuts[i]->hst=0;
      delete h_cuts[i];
      h_cuts[i]=0;
    }
  }
  //delete list_cuts;
  //delete list_h_cuts;
  //list_h_cuts=0;
}

HMap::HMap(const HMap& other) {
  hst = other.hst;
  chk = other.chk;
  wrk = other.wrk;
  cut_index = other.cut_index;
  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  //list_cuts = (TList*) other.list_cuts->Clone();
  //list_h_cuts = (TList*) other.list_h_cuts->Clone();
}

//TH1F& TH1F::operator=(const TH1F &h1)
//RFive& operator=(const RFive& other)
HMap& HMap::operator=(const HMap& other) {
  hst = other.hst;
  chk = other.chk;
  wrk = other.wrk;
  cut_index = other.cut_index;
  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  //list_cuts = (TList*) other.list_cuts->Clone();
  //list_h_cuts = (TList*) other.list_h_cuts->Clone();
  return *this;
}

//------------------------------

HClass::HClass()
{
  for (int i=0;i<MAXCUTS;i++) {
    cutG[i]=0;
    //cutmap[i]=0;
  }

  // for (int cc=0;cc<MAXCUTS;cc++) {
  //   for (int i=0;i<MAX_CH;i++) {
  //     h_ampl[i][cc]=0;
  //     h_height[i][cc]=0;
  //     h_time[i][cc]=0;
  //     h_tof[i][cc]=0;
  //     h_mtof[i][cc]=0;
  //     h_per[i][cc]=0;
  //   }
  //   h_2d[0][cc]=0;
  // }

  wfalse=false;
  //Make_hist();
  hilist=NULL;
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

void HClass::Make_1d(const char* dname, const char* name, const char* title,
		     HMap* map[],// TH1F* hh[MAX_CH][MAXCUTS],
		     Float_t bins, Float_t min, Float_t max,
		     Bool_t bb, Bool_t* chk, Bool_t* wrk,
		     Char_t *cuts) {

  if (!bb) return;

  char name2[100];
  char title2[100];

  for (int i=0;i<MAX_CH;i++) {
    //sprintf(name,"ampl_%02d",i);
    //sprintf(title,"ampl_%02d;Channel;Counts",i);
    NameTitle(name2,title2,i,0,name,title);
    int nn=bins*(max-min);
    TH1F* hh=new TH1F(name2,title2,nn,min,max);

    //cout << "cuts: " << (void*) cuts << " " << (void*) (cuts+i*MAXCUTS) << endl;
    map[i] = new HMap(dname,hh,chk+i,wrk+i,cuts+i*MAXCUTS);
    hilist->Add(map[i]);

  }
}

void HClass::Make_2d(const char* dname, const char* name, const char* title,
		     HMap* map[],// TH2F* hh[][MAXCUTS],
		     Float_t bins, Float_t min, Float_t max,
		     Bool_t bb, Bool_t* chk, Bool_t* wrk,
		     Char_t *cuts) {

  if (!bb) return;

  char name2[100];
  char title2[100];

  sprintf(name2,"%s",name);
  sprintf(title2,"%s%s",name,title);

  int nn=bins*(max-min);
  TH2F* hh=new TH2F(name2,title2,nn,min,max,nn,min,max);

  map[0] = new HMap(dname,hh,chk+0,wrk+0,cuts+0*MAXCUTS);
  hilist->Add(map[0]);

}

//void HClass::Clone_Hist(HMap* map) {
//}

void HClass::Make_cuts() {
  char cutname[99];
  char name[99],htitle[99];

  // loop through all maps
  TIter next(hilist);
  TObject* obj=0;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;

    //determine cut titles and colors
    for (int i=0;i<MAXCUTS;i++) {
      cutcolor[i]=map->cut_index[i];
      if (cutcolor[i]==0) {
    	break;
      }
      cutcolor[i]+=1;
      sprintf(cuttitle[i],"%s",map->GetName());
    }

    //clone histograms if map flag is set
    if (*map->wrk) {
      TH1* h0 = (TH1*) map->hst;
      for (int i=0;i<opt.ncuts;i++) {

	sprintf(cutname,"WORK_cut%d",i+1);
	sprintf(name,"%s_cut%d",h0->GetName(),i+1);
	sprintf(htitle,"%s_cut%d",h0->GetTitle(),i+1);
	TH1* hcut = (TH1*) h0->Clone();
	hcut->Reset();
	hcut->SetNameTitle(name,htitle);
	HMap* mcut = new HMap(cutname,hcut,map->chk,&wfalse,map->cut_index);

	// add this map to the list h_cuts
	map->h_cuts[i]=mcut;
      }
    }
  
  } //while obj

  //make cuts
  for (int i=0;i<opt.ncuts;i++) {
    if (cutG[i]) delete cutG[i];
    sprintf(cutname,"cut%d",i+1);
    cutG[i] = new TCutG(cutname,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    cutG[i]->SetTitle(cuttitle[i]);
    cutG[i]->SetLineColor(cutcolor[i]);
  }
  
}

void HClass::Make_hist() {

  //opt.cut_per[9*MAXCUTS+4]=17;
  
  //char title[100];
  //char name[100];
  //char title[100];

  memset(T_prev,0,sizeof(T_prev));

  if (hilist)
    delete hilist;
  hilist = new TList();
  hilist->SetName("hilist");
  hilist->SetOwner(true);

  //int cc=0;

  Make_1d("Amplitude","ampl",";Channel;Counts",m_ampl,opt.amp_bins,
  	  opt.amp_min,opt.amp_max,opt.b_amp,opt.s_amp,opt.w_amp,opt.cut_amp);
  Make_1d("Height","height",";Channel;Counts",m_height,opt.hei_bins,
  	  opt.hei_min,opt.hei_max,opt.b_hei,opt.s_hei,opt.w_hei,opt.cut_hei);
  Make_1d("Time","time",";T(sec);Counts",m_time,opt.time_bins,
  	  opt.time_min,opt.time_max,opt.b_time,opt.s_time,opt.w_time,opt.cut_time);
  Make_1d("TOF","tof",";t(ns);Counts",m_tof,opt.tof_bins,
  	  opt.tof_min,opt.tof_max,opt.b_tof,opt.s_tof,opt.w_tof,opt.cut_tof);
  Make_1d("MTOF","mtof",";t(mks);Counts",m_mtof,opt.mtof_bins,
  	  opt.mtof_min,opt.mtof_max,opt.b_mtof,opt.s_mtof,opt.w_mtof,opt.cut_mtof);
  Make_1d("Period","period",";t(mks);Counts",m_per,opt.per_bins,
  	  opt.per_min,opt.per_max,opt.b_per,opt.s_per,opt.w_per,opt.cut_per);

  Make_2d("H2d","h2d",";Channel;Channel",m_2d,opt.h2d_bins,
  	  opt.h2d_min,opt.h2d_max,opt.b_h2d,opt.s_h2d,opt.w_h2d,opt.cut_h2d);


  if (opt.ncuts)
    Make_cuts();

  /*
    //Profilometer
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

    /*
    //ibr2 - 2d
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
    //} //for cc
    */

}
