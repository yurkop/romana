//----- HClass ----------------
#include "romana.h"
#include <TStyle.h>

extern Toptions opt;

extern CRS* crs;
extern ParParDlg *parpar;
extern Coptions cpar;

// extern ULong_t fGreen;
// extern ULong_t fRed;
// extern ULong_t fCyan;

//extern TH1F *hrms;
//extern TH1F *htdc_a[MAX_CH]; //absolute tof (start=0)

extern HistFrame* EvtFrm;

//extern Double_t initcuts[MAXCUTS];

//TText txt;

//------------------------------

HMap::HMap() {
  hst = 0;
  chk = 0;
  wrk = 0;
  bitwk=0;
  cut_index = 0;
  memset(h_cuts,0,sizeof(h_cuts));
  h_MT=0;
}

HMap::HMap(const char* dname, TH1* hist, Bool_t* s, Bool_t* w,
	   Int_t *cuts) {
  hst = hist;
  chk = s;
  wrk = w;
  //memcpy(cut_index,cuts,sizeof(cut_index));
  cut_index = cuts;
  //cout << "index: " << (void*) cuts << " " << (void*) cut_index << endl;
  SetTitle(dname);
  if (hist)
    SetName(hist->GetName());
  else
    SetName(dname);
  memset(h_cuts,0,sizeof(h_cuts));
  h_MT=0;
  //list_cuts = new TList();
  //list_h_cuts = new TList();
}

HMap::~HMap() {
  //cout << "~hmap: " << GetName() << endl;
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
  if (h_MT) {
    delete h_MT->hst;
    h_MT->hst=0;
    delete h_MT;
    h_MT=0;
  }
  //delete list_cuts;
  //delete list_h_cuts;
  //list_h_cuts=0;
}

HMap::HMap(const HMap& other) {
  hst = other.hst;
  chk = other.chk;
  wrk = other.wrk;
  bitwk = other.bitwk;
  cut_index = other.cut_index;
  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  h_MT=other.h_MT;
  //list_cuts = (TList*) other.list_cuts->Clone();
  //list_h_cuts = (TList*) other.list_h_cuts->Clone();
}

//TH1F& TH1F::operator=(const TH1F &h1)
//RFive& operator=(const RFive& other)
HMap& HMap::operator=(const HMap& other) {
  hst = other.hst;
  chk = other.chk;
  wrk = other.wrk;
  bitwk = other.bitwk;
  cut_index = other.cut_index;
  for (int i=0;i<MAXCUTS;i++) {
    h_cuts[i]=other.h_cuts[i];
  }
  h_MT=other.h_MT;
  //list_cuts = (TList*) other.list_cuts->Clone();
  //list_h_cuts = (TList*) other.list_h_cuts->Clone();
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
    //initcuts[i]=1;
    //cutmap[i]=0;
  }

  wfalse=false;
  //Make_hist();
  map_list=NULL;
  hist_list=NULL;
  dir_list=NULL;
}

HClass::~HClass()
{
}

void NameTitle(char* name, char* title, int i, int cc, 
			  const char* nm, const char* axis) {
  // if (cc) {
  //   sprintf(name,"%s_%02d_cut%d",nm,i,cc);
  //   sprintf(title,"%s_%02d_cut%d%s_cut%d",nm,i,cc,axis,cc);
  // }
  // else {
    sprintf(name,"%s_%02d",nm,i);
    sprintf(title,"%s_%02d%s",nm,i,axis);
    //}
}

void HClass::Make_1d(const char* dname, const char* name, const char* title,
		     HMap* map[], Hdef* hd) {

  if (!hd->b) return;

  char name2[100];
  char title2[100];

  // HMap* dmap = new HMap(dname,NULL,NULL,NULL,NULL);
  // dir_list->Add(dmap);

  for (int i=0;i<opt.Nchan;i++) {
    NameTitle(name2,title2,i,0,name,title);
    int nn=hd->bins*(hd->max - hd->min);
    TH1F* hh=new TH1F(name2,title2,nn,hd->min,hd->max);
    map[i] = new HMap(dname,hh,hd->c+i,hd->w+i,hd->cut+i);
    map_list->Add(map[i]);
    hist_list->Add(map[i]->hst);
  }

  
  // sprintf(name2,"%s_all",name);
  // sprintf(title2,"%s_all%s",name,title);
  // int nn=hd->bins*(hd->max - hd->min);
  // TH1F* hh=new TH1F(name2,title2,nn,hd->min,hd->max);
  // map[MAX_CH] = new HMap(dname,hh,hd->c+MAX_CH,hd->w+MAX_CH,hd->cut+MAX_CH);
  // map_list->Add(map[MAX_CH]);
  // hist_list->Add(map[MAX_CH]->hst);


  for (int j=0;j<NGRP;j++) {
    sprintf(name2,"%s_g%d",name,j+1);
    sprintf(title2,"%s_g%d%s",name,j+1,title);
    int nn=hd->bins*(hd->max - hd->min);
    TH1F* hh=new TH1F(name2,title2,nn,hd->min,hd->max);
    map[MAX_CH+j] = new HMap(dname,hh,hd->c+MAX_CH+j,hd->w+MAX_CH+j,
			     hd->cut+MAX_CH+j);
    map_list->Add(map[MAX_CH+j]);
    hist_list->Add(map[MAX_CH+j]->hst);
  }

}

void HClass::Make_1d_pulse(const char* dname, const char* name,
			   const char* title, HMap* map[], Hdef* hd) {

  if (!hd->b) return;

  char name2[100];
  char title2[100];

  // HMap* dmap = new HMap(dname,NULL,NULL,NULL,NULL);
  // dir_list->Add(dmap);

  for (int i=0;i<opt.Nchan;i++) {
    NameTitle(name2,title2,i,0,name,title);

    Float_t min = -cpar.preWr[i];
    Float_t max = cpar.durWr[i]-cpar.preWr[i];
    Float_t bins = 1;

    int nn=bins*(max-min);
    //cout << "Hist: " << i << " " << nn << " " << min << " " << max << endl;
    TH1F* hh=new TH1F(name2,title2,nn,min,max);

    //cout << "cuts: " << (void*) cuts << " " << (void*) (cuts+i*MAXCUTS) << endl;
    map[i] = new HMap(dname,hh,hd->c+i,hd->w+i,hd->cut+i);
    map_list->Add(map[i]);
    hist_list->Add(map[i]->hst);

  }
}

void HClass::Make_2d(const char* dname, const char* name, const char* title,
		     HMap* map[], Hdef* hd, Hdef* hd1, Hdef* hd2) {

  if (!hd->b) return;

  char name2[100];
  char title2[100];

  for (int i=0;i<opt.Nchan;i++) {
    //sprintf(name2,"%s",name);
    //sprintf(title2,"%s%s",name,title);
    NameTitle(name2,title2,i,0,name,title);

    int n1=hd->bins*(hd1->max - hd1->min);
    int n2=hd->bins2*(hd2->max - hd2->min);
    TH2F* hh=new TH2F(name2,title2,n1,hd1->min,hd1->max,n2,hd2->min,hd2->max);

    map[i] = new HMap(dname,hh,hd->c+i,hd->w+i,hd->cut+i);
    map_list->Add(map[i]);
    hist_list->Add(map[i]->hst);
  }

}

void HClass::
Make_prof(int npix, const char* dname, const char* name,
	  const char* title, HMap* map[], Hdef* hd) {

  if (!hd->b) return;

  char name2[100];
  char title2[100];

  int nn = sqrt(npix);
  for (int j=nn-1;j>=0;j--) {
    for (int k=0;k<nn;k++) {
      sprintf(name2,"%s_%d_%d",name,k+1,j+1);
      sprintf(title2,"%s_%d_%d%s",name,k+1,j+1,title);
      //NameTitle(name2,title2,i,0,name,title);

      int i=k+(nn-j-1)*nn;
      TH2F* hh=new TH2F(name2,title2,nn,0,120,nn,0,120);

      map[i] = new HMap(dname,hh,hd->c+i,hd->w+i,hd->cut+i);
      map_list->Add(map[i]);
      hist_list->Add(map[i]->hst);
    }
  }

}

void HClass::Make_a0a1(const char* dname, const char* name, const char* title,
		     HMap* map[], Hdef* hd) {

  if (!hd->b) return;

  char name2[100];
  char title2[100];

  sprintf(name2,"%s",name);
  sprintf(title2,"%s%s",name,title);

  int nn=hd->bins*(hd->max - hd->min);
  TH2F* hh=new TH2F(name2,title2,nn,hd->min,hd->max,nn,hd->min,hd->max);

  map[0] = new HMap(dname,hh,hd->c+0,hd->w+0,hd->cut+0);
  map_list->Add(map[0]);
  hist_list->Add(map[0]->hst);

}

void HClass::Clone_Hist(HMap* map) {
  char cutname[99];
  char name[99],htitle[99];
  // clone map->hist as many times as there are cuts; put it in map->h_cuts[i]
  //do the same with MT

  //if (*map->wrk) {
    TH1* h0 = (TH1*) map->hst;
    for (int i=1;i<opt.ncuts;i++) {
      if (opt.pcuts[i]) {
	sprintf(cutname,"WORK_cut%d",i);
	sprintf(name,"%s_cut%d",h0->GetName(),i);
	sprintf(htitle,"%s_cut%d",h0->GetTitle(),i);
	TH1* hcut = (TH1*) h0->Clone();
	hcut->Reset();
	hcut->SetNameTitle(name,htitle);
	//cout << "clone: " << i << " " << hcut->GetName() << " " << gStyle << endl;
	hist_list->Add(hcut);
	HMap* mcut = new HMap(cutname,hcut,map->chk,&wfalse,map->cut_index);

	// add this map to the list h_cuts
	map->h_cuts[i]=mcut;
      }
    }

    // if (crs->b_maintrig) {
    //   sprintf(cutname,"WORK_MT");
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
      TH1* hcut = map->h_cuts[i]->hst;
      hist_list->Remove(hcut);
      delete mcut;
      map->h_cuts[i]=0;
    }
    else
      break;
  }

  HMap* mcut = map->h_MT;
  if (mcut) {
    TH1* hcut = map->h_MT->hst;
    hist_list->Remove(hcut);
    delete mcut;
    map->h_MT=0;
  }
  
}

void HClass::Make_cuts() {
  char cutname[99];
  //char name[99],htitle[99];

  // for (int i=0;i<MAXCUTS;i++) {
  //   cutcolor[i]=0;
  // }

  // loop through all maps
  // для каждой гистограммы (map) находим cuts, которые заданы в ней
  //cout << "make_cuts1: " << endl;
  TIter next(map_list);
  TObject* obj=0;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;

    //determine cut titles and colors
    int icut=1;
    for (int i=1;i<opt.ncuts;i++) {
      if (getbit(*(map->cut_index),i)) {
	cutcolor[i]=icut+1;
	icut++;
	//cutcolor[i]+=1;
	sprintf(cuttitle[i],"%s",map->GetName());
	//cout << "cutcolor: " << i << " " << cutcolor[icut-1] << " " << cuttitle[icut-1] << endl;
      }
    }

    //clone histograms if map flag is set
    if (*map->wrk) {
      Remove_Clones(map);
      Clone_Hist(map);
    }
  
  } //while obj

  //cout << "color[0]: " << cutcolor[0] << endl;
  //make cuts
  for (int i=1;i<opt.ncuts;i++) {
    //cout << "cuts1: " << i << endl;
    if (cutG[i]) {
      delete cutG[i];
      cutG[i]=0;
    }
    
    //cout << "cuts2: " << opt.ncuts << " " << i << endl;
    sprintf(cutname,"%d",i);

    if (opt.pcuts[i]==1) { //formula
      //cout << "cuts3: " << i << " " << cform[i] << " " << opt.cut_form[i] << endl;
      //b_formula=true;
      cform[i]->SetTitle(opt.cut_form[i]);
      cform[i]->Clear();
      //cout << "Compile1: " << endl;
      int ires = cform[i]->Compile();
      //cout << "Compile2: " << endl;
      if (ires) {//formula is not valid -> set it to "always false"
	sprintf(opt.cut_form[i],"0");
	cform[i]->SetTitle("0");
	cform[i]->Clear();
	cform[i]->Compile();
      }
      else { //formula is valid
	b_formula=true;
      }
      //cout << "cuts3a: " << i << endl;
    }
    else { //normal cut (1d or 2d)
      //cout << "cuts4: " << i << endl;
      //cout << "make_cuts: " << opt.ncuts << " " << i << " " << cutcolor[i] << " " << cutG[i]->GetLineColor() << endl;
    }
    //cout << "cutname: " << i << " " << cutname << " " << cuttitle[i] << endl;
    cutG[i] = new TCutG(cutname,opt.pcuts[i],opt.gcut[i][0],opt.gcut[i][1]);
    cutG[i]->SetTitle(cuttitle[i]);
    cutG[i]->SetLineColor(cutcolor[i]);
  }
  //cout << "make_cuts2: " << endl;
}

void HClass::Make_hist() {

  //opt.cut_per[9*MAXCUTS+4]=17;
  
  //char title[100];

  memset(T_prev,0,sizeof(T_prev));

  if (hist_list)
    delete hist_list;
  hist_list = new TList();
  hist_list->SetName("hist_list");
  hist_list->SetOwner(false);

  if (map_list)
    delete map_list;
  map_list = new TList();
  map_list->SetName("map_list");
  map_list->SetOwner(true);

  Make_1d("Time","time",";T(sec);Counts",m_time,&opt.h_time);
  Make_1d("Area","area",";Channel;Counts",m_area,&opt.h_area);
  Make_1d("Area0","area0",";Channel;Counts",m_area0,&opt.h_area0);
  Make_1d("Base","base",";Channel;Counts",m_base,&opt.h_base);
  Make_1d("Slope1","slope1",";Channel;Counts",m_slope1,&opt.h_slope1);
  Make_1d("Slope2","slope2",";Channel;Counts",m_slope2,&opt.h_slope2);
  Make_1d("Height","height",";Channel;Counts",m_height,&opt.h_hei);
  Make_1d("Tof","tof",";t(ns);Counts",m_tof,&opt.h_tof);
  Make_1d("Ntof","ntof",";t(mks);Counts",m_ntof,&opt.h_ntof);
  Make_1d("Etof","etof",";Energy(eV);Counts",m_etof,&opt.h_etof);
  Make_1d("Ltof","ltof",";Lambda(A);Counts",m_ltof,&opt.h_ltof);
  Make_1d("Period","period",";t(mks);Counts",m_per,&opt.h_per);
  Make_1d("Width","width",";width(a.u.);Counts",m_width,&opt.h_width);
  Make_1d("Width2","width2",";width(a.u.);Counts",m_width2,&opt.h_width2);
  //Make_1d("Width3","width3",";width(a.u.);Counts",m_width3,&opt.h_width3);
  Make_1d_pulse("Pulse","pulse",";samples;Amplitude",m_pulse,&opt.h_pulse);
  Make_1d_pulse("Deriv","deriv",";samples;Amplitude",m_deriv,&opt.h_deriv);

  Make_a0a1("A0A1","A0A1",";Channel;Channel",m_a0a1,&opt.h_a0a1);
  Make_2d("Area_Base","Area_Base",";Channel;Channel",m_area_base,&opt.h_area_base,&opt.h_area,&opt.h_base);
  Make_2d("Area_Sl1","Area_Sl1",";Channel;Channel",m_area_sl1,&opt.h_area_sl1,&opt.h_area,&opt.h_slope1);
  Make_2d("Area_Sl2","Area_Sl2",";Channel;Channel",m_area_sl2,&opt.h_area_sl2,&opt.h_area,&opt.h_slope2);
  Make_2d("Slope_12","Slope_12",";Channel;Channel",m_slope_12,&opt.h_slope_12,&opt.h_slope1,&opt.h_slope2);
  Make_2d("Area_Time","Area_Time",";Channel;Time (sec)",m_area_time,&opt.h_area_time,&opt.h_area,&opt.h_time);
  Make_2d("Area_Width","Area_Width",";Channel;Width (a.u.)",m_area_width,&opt.h_area_width,&opt.h_area,&opt.h_width);
  Make_2d("Area_Width2","Area_Width2",";Channel;Width (a.u.)",m_area_width2,&opt.h_area_width2,&opt.h_area,&opt.h_width2);
  //Make_2d("Area_Width3","Area_Width3",";Channel;Width (a.u.)",m_area_width3,&opt.h_area_width3,&opt.h_area,&opt.h_width3);
  //Make_2d("Width_12","Width_12",";Width(a.u.);Width2(a.u.)",m_width_12,&opt.h_width_12,&opt.h_width,&opt.h_width2);

  Make_prof(64,"Prof","Prof",";X (mm);Y (mm)",m_prof,&opt.h_prof);

  b_formula=false;
  if (opt.ncuts)
    Make_cuts();

  //cout << "make_cuts3: " << endl;
  //cout << "hist_list: " << m_ampl[0]->h_cuts[0]->hst << endl;
  //hist_list->ls();

  /*
  //Profilometer
  char name[100];
  char title[100];
  for (int i=0; i<64; i++){
    sprintf(name,"Profile_strip%02d",i);
    sprintf(title,"Profile_strip%02d;X_strip;Y_strip",i);
    h2_prof_strip[i] = new TH2F(name,title,8,0,8,8,0,8);
  }

  for (int i=0; i<64; i++){
    sprintf(name,"profile_real%02d",i);
    sprintf(title,"Profile_real%02d;X (mm);Y (mm)",i);
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
