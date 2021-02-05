//----- PopFrame ----------------
#include "romana.h"

//#include <iostream>
#include <TCanvas.h>
#include <TPolyMarker.h>
#include <TF1.h>
#include <TSpectrum.h>
#include <TColor.h>
#include <TGlobal.h>
#include <sstream>

//extern Common* com;
extern GlbClass *glb;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern HClass* hcl;
extern EventFrame* EvtFrm;
extern Toptions opt;

const int ww[]={15,70,80,80,80};

//const double Co60[] = {1173,1332};

using namespace std;

PopFrame::PopFrame(const TGWindow *main, UInt_t w, UInt_t h, Int_t menu_id,
		   void* p) {
  //ee_calib=0;
  ptr=p;

  LayLC0   = new TGLayoutHints(kLHintsLeft|kLHintsTop);
  LayLC2   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 2,2,2,2);
  //LayCC2   = new TGLayoutHints(kLHintsCenterX|kLHintsTop, 2,2,2,2);
  LayEE2   = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY, 2,2,2,2);
  //LayLE2   = new TGLayoutHints(kLHintsLeft|kLHintsExpandY, 2,2,2,2);

  txt.SetTextSize(0.07);
  txt.SetTextAlign(22);
  fMain = new TGTransientFrame(gClient->GetRoot(), main, w, h);
  fMain->Connect("CloseWindow()", "PopFrame", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.

  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  if (menu_id==M_PROF_TIME) {
    AddProfTime(w,h);
  }
  // else if (menu_id==M_PRECALIBR) {
  //   AddEcalibr(w,h);
  // }
  else if (menu_id==M_ECALIBR) {
    AddEcalibr(w,h);
  }
  else if (menu_id==M_TCALIBR) {
    AddTcalibr();
  }

  fMain->MapSubwindows();
  fMain->Resize();
  // editor covers right half of parent window
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kCenter);
  fMain->MapWindow();
}
PopFrame::~PopFrame()
{
  fMain->DeleteWindow();  // deletes fMain
}
void PopFrame::CloseWindow()
{
  // Called when closed via window manager action.
  //cout << "fVar1: " << fVar << " " << this << endl;
  //fVar=NULL;
  HiFrm->b_adj=false;
  HiFrm->Update();

  delete this;
  //myM->p_pop=0;
  //*fVar=0;
  //cout << "fVar2: " << *fVar << " " << this << endl;
}

void PopFrame::AddProfTime(UInt_t w, UInt_t h) {
  fCanvas = new TRootEmbeddedCanvas("Events",fMain,w,h);
  fMain->AddFrame(fCanvas, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));

  TCanvas *cv=fCanvas->GetCanvas();
  cv->Clear();
  cv->Divide(1,2);

  TH1* hh;
  TGraph* gr;
  TF1* fit;
  int res=0;

  TString name = opt.Prof64_TSP;
  TString msg;
  HMap* map = (HMap*) hcl->allmap_list->FindObject(name);
  //TGListTreeItem *item=HiFrm->FindItem(ss);
  //cout << "map: " << map << endl;
  if (map) {
    res++;
    cv->cd(1);
    name+='r';
    hh = (TH1*) map->hst->Clone(name.Data());
    hh->Draw();
    hh->ShowPeaks(2,"",0.05);
    TList *functions = hh->GetListOfFunctions();
    TPolyMarker *pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
    if (pm) {
      res++;
      //functions->Remove(pm);
      int nn = pm->GetN();
      //cout << nn << " peaks found" << endl;
      if (nn>=31) {
	//nn=31;
	res++;
	gr = new TGraph(nn,pm->GetX(),pm->GetX());
	gr->Sort();
	nn = 31;
	gr->Set(nn);
	for (int i=0;i<nn;i++) {
	  gr->GetX()[i]=i;
	  //cout << "gr: " << i << " " << gr->GetY()[i] << endl;
	}
	gr->SetNameTitle("Time_Calibr","Time Calibration");
	gr->Fit("pol1","Q");
	cv->cd(2);
	gr->Draw("AL*");

	fit = gr->GetFunction("pol1");
	if (fit) {
	  res++;
	}
      } //
      else {
	msg = "Number of peaks if less than 31: ";
	msg+=nn;
      }
    } //pm
    else {
      msg = "Peaks not found";
    }
  }
  else {
    msg = TString("Histogram ") + opt.Prof64_TSP + " not found";
  }

  cv->cd(2);
  if (res<4) { //error
    txt.DrawTextNDC(0.5,0.5,msg);
    cout << "res: " << res << endl;
  }
  else { //OK
    opt.Prof64_W[0]=fit->GetParameters()[1]/opt.Period;
    msg = TString("Period = ") + opt.Prof64_W[0];
    txt.DrawTextNDC(0.5,0.5,msg);
    if (ptr) {
      PEditor* pe = (PEditor*) ptr;
      pe->LoadPar64();
    }
  }
  cv->Update();


  TGTextButton* fClose = new TGTextButton(fMain, "  &Close  ");
  fClose->Connect("Clicked()", "PopFrame", this, "CloseWindow()");
  fMain->AddFrame(fClose, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

  // TGTextButton* fOK = new TGTextButton(fMain, "  &OK  ");
  // fOK->Connect("Clicked()", "PopFrame", this, "DoProfTime()");
  // fMain->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));
}

void PopFrame::AddAdj(TGCompositeFrame* fcont1, HMap* map, int i) {
  hframe = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe,LayLC2);


  fLabel = new TGLabel(hframe," ");
  fLabel->ChangeOptions(fLabel->GetOptions()|kFixedWidth);
  fLabel->SetWidth(ww[0]);
  hframe->AddFrame(fLabel, LayLC0);
  fLabel->SetBackgroundColor(EvtFrm->gcol[i]);

  if (map) {
    fLabel = new TGLabel(hframe,map->GetName());
  }
  else {
    fLabel = new TGLabel(hframe,"All");
  }
  fLabel->ChangeOptions(fLabel->GetOptions()|kFixedWidth);
  fLabel->SetWidth(ww[1]);
  hframe->AddFrame(fLabel, LayLC0);
  //fLabel->SetBackgroundColor(EvtFrm->gcol[i]);

  for (int j=0;j<2;j++) {
    //cout << "fadj: " << i << " " << j << endl;


    fAdj[i][j]=new TGNumberEntryField(hframe,i*10+j,opt.adj[i][j],kr,ka,kn);
    fAdj[i][j]->Connect("TextChanged(char*)", "PopFrame", this, "DoAdj()");
    fAdj[i][j]->SetWidth(ww[j+2]);
    hframe->AddFrame(fAdj[i][j], LayLC0);


    // TGNumberEntryField *ff =new TGNumberEntryField(hframe,0,1);
    // //ff->Associate(fMain);
    // //fAdj[i][j]->Connect("TextChanged(char*)", "PopFrame", this, "DoAdj()");
    // //fAdj[i][j]->SetWidth(ww[j+2]);
    // hframe->AddFrame(ff, LayLC0);
  }
}

void PopFrame::AddEcalibr(UInt_t w, UInt_t h) {

  memcpy(&E0,&opt.E0,sizeof(E0));
  memcpy(&E1,&opt.E1,sizeof(E1));
  //memcpy(&E2,&opt.E2,sizeof(E2));
  memcpy(&adj,&opt.adj,sizeof(adj));

  HiFrm->b_adj=true;
  HiFrm->Update();

  memset(fAdj,0,sizeof(fAdj));

  fMain->SetWindowName("Pre-calibrate all marked 1D histograms");

  TGCanvas* fCanvas1 = new TGCanvas(fMain,360,h*2/3.0);
  fMain->AddFrame(fCanvas1, LayEE2);

  TGCompositeFrame* fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
  				10, 10, kVerticalFrame);
  fcont1->SetMapSubwindows(true);
  fCanvas1->SetContainer(fcont1);
  //fMain->SetCleanup(kDeepCleanup);
  fCanvas1->GetViewPort()->SetCleanup(kDeepCleanup);


  hframe = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe, LayLC2);

  const char* aa[] = {" ","hist","A0","A1"};
  for (int j=0;j<4;j++) {
    fLabel = new TGLabel(hframe,aa[j]);
    fLabel->ChangeOptions(fLabel->GetOptions()|kFixedWidth);
    fLabel->SetWidth(ww[j]);
    hframe->AddFrame(fLabel, LayLC0);
    //hframe->AddFrame(new TGLabel(hframe, aa[j]), LayLC2);
  }

  HiFrm->Make_Hmap_ChkList();

  TIter next(HiFrm->hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {

    HMap* map=(HMap*) obj;
    int i = map->nn;

    if (fAdj[i][0])
      continue;

    //TString chch = TString::Format("%15d",i);
    AddAdj(fcont1, map, i);
  }

  AddAdj(fcont1, 0, MAX_CH+NGRP);

  fEdit = new TGTextEdit(fMain, 360, h/3, kSunkenFrame|kDoubleBorder);
  fMain->AddFrame(fEdit,  LayEE2);

  if (!fEdit->LoadFile("ecalibr.dat")) {
    fEdit->LoadBuffer(
		      "# calibrate all marked energy histograms\n"
		      "# all ROI must be set befor calibration\n"
		      "# each ROI may contain several peaks,\n"
		      "# fitted simultaneously\n"
		      "#--------------------------------------\n"
		      "\n"
		      "1	# substrate poly\n"
		      "3	# Fit poly\n"
		      "\n"
		      "--- Peaks: #ROI peak1 peak2 ... peakN ---\n"
		      "1 846.7778  # 56F)\n"
		      "2 2223.245  # 1H(n,g)\n"
		      "3 6129.89   # 16O(n,n')\n"
		      "4 7645.58   # 56Fe(n,g)\n"
		      );
  }

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsBottom, 2,2,2,2));
  TGTextButton* fOK = new TGTextButton(hframe, "  Calibr  ");
  fOK->Connect("Clicked()", "PopFrame", this, "Do_Ecalibr()");
  hframe->AddFrame(fOK, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 5, 5, 5, 5));

  TGTextButton* fSave = new TGTextButton(hframe, "  Save  ");
  fSave->Connect("Clicked()", "PopFrame", this, "Do_Save_Ecalibr()");
  hframe->AddFrame(fSave, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 5, 5, 5, 5));

  TGTextButton* fClose = new TGTextButton(hframe, "  Close  ");
  fClose->Connect("Clicked()", "PopFrame", this, "CloseWindow()");
  hframe->AddFrame(fClose, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 5, 5, 5, 5));

  TGTextButton* fEApply = new TGTextButton(hframe, "  Apply  ");
  fEApply->Connect("Clicked()", "PopFrame", this, "Do_EApply()");
  hframe->AddFrame(fEApply, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 5, 5, 5, 5));

  TGTextButton* fRevert = new TGTextButton(hframe, "  Revert  ");
  fRevert->Connect("Clicked()", "PopFrame", this, "Do_Revert()");
  hframe->AddFrame(fRevert, new TGLayoutHints(kLHintsLeft|kLHintsBottom, 5, 5, 5, 5));

}

void PopFrame::AddTcalibr() {
  fwhm=1;
  npol=1;
  range=3;

  memset(sD,0,sizeof(sD));
  // for (int i=0;i<11;i++) {
  //   pframe[i]=0;
  // }

  fMain->SetWindowName("Time Calibration");
  npeaks=1;

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe, LayLC2);
  fNum = new TGNumberEntry(hframe, fwhm, 8, 11, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum, LayLC2);
  fLabel = new TGLabel(hframe, "Peak width (fwhm)");
  hframe->AddFrame(fLabel, LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe, LayLC2);
  fNum = new TGNumberEntry(hframe, range, 8, 12, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum, LayLC2);
  fLabel = new TGLabel(hframe, "+/- fit range");
  hframe->AddFrame(fLabel, LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe, LayLC2);
  fNum = new TGNumberEntry(hframe, npeaks, 8, 14, ki, ka, kl, 1, 10);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  hframe->AddFrame(fNum, LayLC2);
  fLabel = new TGLabel(hframe, "Number of peaks");
  hframe->AddFrame(fLabel, LayLC2);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));
  TGTextButton* fCalibr = new TGTextButton(hframe, "  &Calibr  ");
  fCalibr->Connect("Clicked()", "PopFrame", this, "Do_Tcalibr()");
  hframe->AddFrame(fCalibr, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));
  TGTextButton* fTApply = new TGTextButton(hframe, "  &Apply  ");
  fTApply->Connect("Clicked()", "PopFrame", this, "Do_TApply()");
  hframe->AddFrame(fTApply, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

}

void PopFrame::Do_Save_Ecalibr() {
  cout << "Calibration parameters are saved to 'ecalibr.dat'" << endl;
  fEdit->SaveFile("ecalibr.dat");
}

void PopFrame::DoAdj() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  int i = id/10;
  int j = id%10;

  opt.adj[i][j]=te->GetNumber();
  if (i==MAX_CH+NGRP) {
    for (int k=0;k<MAX_CH+NGRP;k++) {
    //for (int k=0;k<10;k++) {
      if (fAdj[k][j]) {
	opt.adj[k][j]=opt.adj[i][j];
	char text[256];
	snprintf(text, 255, "%g", opt.adj[i][j]);
	fAdj[k][j]->SetText(text,false);
	//fAdj[k][j]->SetNumber(opt.adj[k][j]);
      }
    }
  }
  HiFrm->Update();
}

void PopFrame::DoENum() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  if (id>=0 && id<10) {
    ee[id]=te->GetNumber();
    //cout << "ee: " << id << " " << ee[id] << endl;
  }
  else if (id==11) {
    fwhm=te->GetNumber();
  }
  else if (id==12) {
    range=te->GetNumber();
  }
  else if (id==13) {
    npol=te->GetNumber();
  }
  // else if (id==14) {
  //   npeaks=te->GetNumber();
  //   AddPeaks();
  // }
}

void PopFrame::Do_Ecalibr()
{

  //cout << "edt!!!: " << endl;
  TGText* tgt = fEdit->GetText();

  //double sig;
  //int range;
  //int iroi;

  //std::vector<double> v_roi;
  //std::vector<double> v_ee;


  bool b_peaks=false;
  //cout << "edt: " << tgt->RowCount() << endl;

  HiFrm->ee_calib.clear();
  HiFrm->bpol=1;
  HiFrm->fitpol=2;
  HiFrm->fitsig=0;
  HiFrm->nofit=0;

  for (int i=0;i<tgt->RowCount();i++) {
    //cout << "i: " << i << endl;
    char* chr = tgt->GetLine(TGLongPosition(0,i),100);
    if (chr) {
      //cout << "Line: " << i << " " << tgt->RowCount() << " " << chr << endl;
      TString str(chr);
      std::stringstream ss(chr);
      double ee;
      ss >> ee;

      if (str.Contains("substr",TString::kIgnoreCase)) {
	HiFrm->bpol=ee;
      }
      else if (str.Contains("nofit",TString::kIgnoreCase)) {
	HiFrm->nofit=ee;
      }
      else if (str.Contains("fit",TString::kIgnoreCase)) {
	HiFrm->fitpol=ee;
      }
      else if (str.Contains("fwhm",TString::kIgnoreCase)) {
	HiFrm->fitsig=ee/2.35;
      }
      else if (str.Contains("peaks",TString::kIgnoreCase)) {
	b_peaks=true;
      }
      if (ss.good() && b_peaks) {
	dvect dd;
	dd.push_back(ee-1);
	//iroi=ee;
	while (ss.good()) {
	  ss >> ee;
	  if (!ss.fail()) {
	    dd.push_back(ee);
	    //v_roi.push_back(iroi-1);
	    //v_ee.push_back(ee);
	    //ch_calib.push_back(ee);
	  }
	}
	if (dd.size()>1) {
	  HiFrm->ee_calib.push_back(dd);
	}
	//cout << endl;
      }
      //cout << chr << " " << e_calib.size() << endl;
      delete[] chr;
    }
  }




  // for (UInt_t i=0;i<HiFrm->ee_calib.size();++i) {
  //   cout << "ee: " << i;
  //   for (UInt_t j=0;j<HiFrm->ee_calib[i].size();++j) {
  //     cout << " " << HiFrm->ee_calib[i][j];
  //   }
  //   cout << endl;
  // }
  




  // string command = ".x ";
  // command+=MACRO;
  // command+="/test.C+";
  // gROOT->ProcessLine(command.c_str());


  HiFrm->Do_Ecalibr(this);

  E_Update();

}

void PopFrame::E_Update() {
  for (int i=0;i<MAX_CH+NGRP;i++) {
    for (int j=0;j<2;j++) {
      if (fAdj[i][j]) {
	char text[256];
         snprintf(text, 255, "%g", opt.adj[i][j]);
         fAdj[i][j]->SetText(text,false);
	   //fAdj[i][j]->SetNumber(opt.adj[i][j]);
      }
    }
  }
}

void PopFrame::Do_Tcalibr()
{

  HiFrm->Do_Tcalibr(this);

} //Do_Tcalibr

void PopFrame::Do_EApply() {
  for (int i=0;i<MAX_CH;i++) {
    if (fAdj[i][0]) {
      // opt.E0[i] = opt.adj[i][0]+opt.adj[i][1]*opt.E0[i]
      // 	+ opt.adj[i][2]*opt.E0[i]*opt.E0[i];
      // opt.E1[i] = opt.adj[i][1]*opt.E1[i]
      // 	+ 2*opt.adj[i][2]*opt.E0[i]*opt.E1[i];
      // opt.E2[i] = opt.adj[i][2]*opt.E1[i]*opt.E1[i];

      opt.E0[i] = opt.adj[i][0];
      opt.E1[i] = opt.adj[i][1];
      //opt.E2[i] = opt.adj[i][2];
    }
  }
  //E_Update();
}

void PopFrame::Do_TApply() {
  //cout << "Appl: " << endl;
  for (int i=0;i<MAX_CH;i++) {
    if (sD[i]) {
      opt.sD[i] = -sD[i];
    }
  }
  //E_Update();
}

void PopFrame::Do_Revert() {
  memcpy(&opt.E0,&E0,sizeof(E0));
  memcpy(&opt.E1,&E1,sizeof(E1));
  //memcpy(&opt.E2,&E2,sizeof(E2));
  memcpy(&opt.adj,&adj,sizeof(adj));

  E_Update();
}
