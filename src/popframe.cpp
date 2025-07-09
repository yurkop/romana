//----- PopFrame ----------------
#include "romana.h"

//#include <iostream>
#include <TCanvas.h>
#include <TPolyMarker.h>
#include <TF1.h>
#include <TSpectrum.h>
#include <TColor.h>
#include <TGlobal.h>
#include <TSystem.h>
#include <sstream>

//extern Common* com;
extern GlbClass *glb;
extern MyMainFrame *myM;
extern HistFrame* HiFrm;
extern HClass* hcl;
extern EventFrame* EvtFrm;
extern ChanParDlg* chanpar;
extern Toptions opt;
extern CRS* crs;

const int ww[]={15,90,60,60,60};

//const double Co60[] = {1173,1332};

using namespace std;

PopFrame::PopFrame(const TGWindow *main, UInt_t w, UInt_t h, MENU_COM menu_id,
		   void* p) {
  m_id=menu_id;
  //myM->pops.at(m_id)=1;

  chklist.Clear();
  //ee_calib=0;
  ptr=p;
  fListBox=0;

  LayLC0   = new TGLayoutHints(kLHintsLeft|kLHintsCenterY,2,2,2,2);
  LayLC1   = new TGLayoutHints(kLHintsLeft|kLHintsTop,20,2,2,2);
  LayLC2   = new TGLayoutHints(kLHintsLeft|kLHintsTop, 2,2,2,2);
  LayCC3   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 150,150,50,5);
  LayCC4   = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 150,150,20,150);
  LayEE2   = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY, 0,0,2,2);
  LayCB1   = new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5);
  LayCT1   = new TGLayoutHints(kLHintsCenterX|kLHintsTop, 0, 0, 2, 2);
  LayBut1 = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 5, 5, 5, 5);
  LayBut1a = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 25, 25, 5, 5);
  LayBut2 = new TGLayoutHints(kLHintsLeft|kLHintsCenterY, 15, 5, 1, 1);
  LayBut3 = new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 55, 55, 5, 5);

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
    fMain->ChangeOptions(fMain->GetOptions()|kFixedWidth);
    AddTcalibr();
  }
  else if (menu_id==M_PEAKS) {
    fMain->ChangeOptions(fMain->GetOptions()|kFixedWidth);
    AddPeaks();
  }
  else if (menu_id==M_OPTPAR) {
    fMain->ChangeOptions(fMain->GetOptions()|kFixedWidth);
    AddOptPar();
  }
#ifdef CYUSB
  else if (menu_id==M_DEVICE) {
    AddDevice();
  }
#endif
// #ifdef YUMO
//   else if (menu_id==M_YUMO) {
//     AddYumo();
//   }
// #endif //YUMO
#ifdef P_TEST
  else if (menu_id==M_TEST) {
    AddTest();
  }
#endif //P_TEST

  fMain->MapSubwindows();
  fMain->Resize();
  // editor covers right half of parent window
  //fMain->CenterOnParent(kTRUE, TGTransientFrame::kCenter);
  fMain->MapWindow();
}
PopFrame::~PopFrame()
{
  if (myM)
    myM->pops.at(m_id)=0;
  fMain->DeleteWindow();  // deletes fMain
}
void PopFrame::CloseWindow()
{
  // Called when closed via window manager action.
  if (fListBox) {
    //cout << "CloseW: " << fListBox->GetSelected() << endl;
#ifdef CYUSB
    crs->idev = fListBox->GetSelected();
    //cout << "idev: " << crs->idev << endl;
    crs->Init_device();
#endif
    myM->Build();
    //myM->EnableBut(myM->fGr1,Fmode==1);
    //myM->DoReset();
    //myM->SetTitle(mainname);
  }

  HiFrm->b_adj=false;
  HiFrm->HiUpdate();

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
    hh->ShowPeaks(2,"",0.002);
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
	msg = "Number of peaks is less than 31: ";
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
    //cout << "res: " << res << endl;
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
  fMain->AddFrame(fClose, LayCB1);

  // TGTextButton* fOK = new TGTextButton(fMain, "  &OK  ");
  // fOK->Connect("Clicked()", "PopFrame", this, "DoProfTime()");
  // fMain->AddFrame(fOK, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));
}

void PopFrame::AddAdj(TGCompositeFrame* fcont1, HMap* map, int i) {
  ULong_t colr = EvtFrm->gcol[i];

  if (i==MAX_CH+NGRP) {
    colr = TColor::RGB2Pixel(245,230,210);
  }


  hframe = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe,LayLC2);

  fLabel = new TGLabel(hframe," ");
  fLabel->ChangeOptions(fLabel->GetOptions()|kFixedWidth);
  fLabel->SetWidth(ww[0]);
  hframe->AddFrame(fLabel, LayLC0);
  fLabel->SetBackgroundColor(colr);

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

  for (int j=0;j<3;j++) {
    //cout << "fadj: " << i << " " << j << " " << opt.adj[i][j] << endl;
    fAdj[i][j]=new TGNumberEntryField(hframe,i*10+j,opt.adj[i][j],kr,ka,kn);
    fAdj[i][j]->Connect("TextChanged(char*)", "PopFrame", this, "DoAdj()");
    fAdj[i][j]->SetWidth(ww[j+2]);
    hframe->AddFrame(fAdj[i][j], LayLC0);
    if (i==MAX_CH+NGRP)
      fAdj[i][j]->ChangeBackground(colr);
  }
}

void PopFrame::AddEcalibr(UInt_t w, UInt_t h) {

  const ULong_t colr = TColor::RGB2Pixel(200,240,230);
  const char* txt =
    "Calibrate all visible 1D histograms that have \"area\" in their name.\n"
    "'Visible' means plotted either in Stack or in X/Y mode.\n"
    "If there are several visible histograms with the same number,\n"
    "like area_07 and area_07_cut1, only the first one will be calibrated.\n\n"
    "For 1-point pre-calibration:\n"
    "- edit reference value for the pre-calibration;\n"
    "- use scrollbar in the Plots tab to select window in such a way that the peak\n"
    " which corresponds to the reference value is maximal in the window;\n"
    "- press Auto. The position of the peak will be auto-calibrated;\n"
    "- press Save to copy the calibration coefficients to channels' parameters;\n"
    "- close calibration window;\n"
    "- reanalyze the data with new calibration.";
  fLabel = new TGLabel(fMain,txt);
  fMain->AddFrame(fLabel, LayCT1);
  fLabel->SetBackgroundColor(colr);


  for (int i=0;i<MAX_CH+NGRP+1;i++) {
    if (opt.adj[i][1]<=0) opt.adj[i][1]=1e-4;
  }

  memcpy(&E0,&opt.E0,sizeof(E0));
  memcpy(&E1,&opt.E1,sizeof(E1));
  memcpy(&E2,&opt.E2,sizeof(E2));
  memcpy(&adj,&opt.adj,sizeof(adj));
  memcpy(&calibr_t,&opt.calibr_t,sizeof(calibr_t));

  HiFrm->b_adj=true;
  HiFrm->HiUpdate();

  memset(fAdj,0,sizeof(fAdj));

  fMain->SetWindowName("Calibrate all marked 1D Area histograms");

  hframe = new TGHorizontalFrame(fMain,1,1);
  fMain->AddFrame(hframe, LayLC2);

  const char* aa[] = {" ","hist","A0","A1","A2"};
  for (int j=0;j<5;j++) {
    TGTextEntry* fLab = new TGTextEntry(hframe, aa[j]);
    fLab->SetState(false);
    fLab->ChangeOptions(fLab->GetOptions()|kFixedSize|kRaisedFrame);
    fLab->SetToolTipText("Pre-calibration coefficient");
    fLab->SetWidth(ww[j]);
    fLab->SetAlignment(kTextCenterX);
    hframe->AddFrame(fLab, LayLC0);
    //hframe->AddFrame(new TGLabel(hframe, aa[j]), LayLC2);
  }

  TGTextButton* fAuto = new TGTextButton(hframe, " Auto ");
  fAuto->Connect("Clicked()", "PopFrame", this, "Do_Auto_Ecalibr()");
  fAuto->SetToolTipText("Auto pre-calibrate using the highest peak in the range");
  hframe->AddFrame(fAuto, LayBut2);


  TGNumberEntryField* fN =
    new TGNumberEntryField(hframe, 21, opt.E_auto, TGNumberFormat::kNESReal);

  fN->SetWidth(50);
  fN->SetToolTipText("Reference value for auto pre-calibration");
  fN->Connect("TextChanged(char*)", "PopFrame", this, "DoENum()");
  hframe->AddFrame(fN, LayBut2);


  TGCanvas* fCanvas1 = new TGCanvas(fMain,360,h*1/2.0);
  fMain->AddFrame(fCanvas1, LayEE2);

  TGCompositeFrame* fcont1 = new TGCompositeFrame(fCanvas1->GetViewPort(), 
  				10, 10, kVerticalFrame);
  fcont1->SetMapSubwindows(true);
  fCanvas1->SetContainer(fcont1);
  //fMain->SetCleanup(kDeepCleanup);
  fCanvas1->GetViewPort()->SetCleanup(kDeepCleanup);

  HiFrm->Make_Hmap_ChkList();

  AddAdj(fcont1, 0, MAX_CH+NGRP);

  //Pixel_t fCyan9;
  //fCyan9 = TColor::RGB2Pixel(1535,255,2);
  //fAdj[MAX_CH+NGRP][0]->ChangeBackground(fCyan9);

  TIter next(HiFrm->hmap_chklist);
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {

    HMap* map=(HMap*) obj;
    int i = map->nn;

    TString str(map->GetName());
    if (!str.Contains("area",TString::kIgnoreCase) || fAdj[i][0])
      continue;

    //TString chch = TString::Format("%15d",i);
    chklist.Add(map);
    AddAdj(fcont1, map, i);
  }

  fEdit = new TGTextEdit(fMain, 360, h/4, kSunkenFrame|kDoubleBorder);
  fMain->AddFrame(fEdit,  LayEE2);

  if (!fEdit->LoadFile("ecalibr.dat")) {
    Do_Default();
  }

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsBottom, 2,2,2,2));

  TGTextButton* fCalibr = new TGTextButton(hframe, "  Calibr  ");
  fCalibr->SetToolTipText("Perform calibration using PeakFit");
  fCalibr->Connect("Clicked()", "PopFrame", this, "Do_Ecalibr()");
  hframe->AddFrame(fCalibr, LayBut1);

  TGTextButton* fSave = new TGTextButton(hframe, "  Save  ");
  fSave->SetToolTipText("Save calibration settings in \"ecalibr.dat\" file");
  fSave->Connect("Clicked()", "PopFrame", this, "Do_Save_Ecalibr()");
  hframe->AddFrame(fSave, LayBut1);

  TGTextButton* fClose = new TGTextButton(hframe, "  Close  ");
  fClose->SetToolTipText("Close calibration window");
  fClose->Connect("Clicked()", "PopFrame", this, "CloseWindow()");
  hframe->AddFrame(fClose, LayBut1);

  TGTextButton* fEApply = new TGTextButton(hframe, "  Apply  ");
  fEApply->SetToolTipText("Apply calibration parameters for next analysis (copy A0-A2 coefficients to E0-E2)");
  fEApply->Connect("Clicked()", "PopFrame", this, "Do_EApply()");
  hframe->AddFrame(fEApply, LayBut1);

  TGTextButton* fDefault = new TGTextButton(hframe, "  Default  ");
  fDefault->SetToolTipText("Load default calibration settings");
  fDefault->Connect("Clicked()", "PopFrame", this, "Do_Default()");
  hframe->AddFrame(fDefault, LayBut1);

  TGTextButton* fRevert = new TGTextButton(hframe, "  Revert  ");
  fRevert->SetToolTipText("Revert to the initial pre-calibration coefficients A0-A2");
  fRevert->Connect("Clicked()", "PopFrame", this, "Do_Revert()");
  hframe->AddFrame(fRevert, LayBut1);

}

void PopFrame::AddNum(double val, int id, const char* label, const char* tip) {
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain);
  fMain->AddFrame(hframe, LayLC1);
  TGNumberEntry *fNum = new TGNumberEntry(hframe, val, 8, id, kr, ka, kn);
  fNum->GetNumberEntry()->Connect("TextChanged(char*)", "PopFrame", this,
				  "DoENum()");
  fNum->GetNumberEntry()->SetToolTipText(tip);
  hframe->AddFrame(fNum, LayLC0);
  fLabel = new TGLabel(hframe, label);
  hframe->AddFrame(fLabel, LayLC0);
}

void PopFrame::AddChk(bool val, int id, const char* label, const char* tip) {
  TGHorizontalFrame *hframe = new TGHorizontalFrame(fMain);
  fMain->AddFrame(hframe, LayLC2);


  TGCheckButton *fchk = new TGCheckButton(hframe, label, id);
  fchk->SetState((EButtonState)val);
  fchk->Connect("Toggled(Bool_t)", "PopFrame", this, "DoENum()");
  fchk->SetToolTipText(tip);

  hframe->AddFrame(fchk, LayLC2);

  //fLabel = new TGLabel(hframe, label);
  //hframe->AddFrame(fLabel, LayLC2);
}

void PopFrame::AddTcalibr() {

  if (sizeof(local_sD)!=sizeof(opt.sD)) {
    prnt("sss;",BRED,"local_sD!",RST);
    return;
  }

  const ULong_t colr = TColor::RGB2Pixel(220,230,250);
  const char* txt =
    "To auto-calibrate Time spectra:\n\n"
    " - plot all Time histograms in multiple sub-windows (no Stack)\n"
    "      in such a way that the reference peak\n"
    "      is the highest in the window;\n"
    " - select \"Peaks\"; optionally optimize Peak search parameters;\n"
    " - enter a reference value to which all peak positions will be adjusted;\n"
    " - press \"Apply\" to adjust sD parameters to new peak positions;\n"
    " - (re)start analysis/acquisition.\n\n"
    "\"Revert\" will restore the sD values that were set\n"
    "   when the Time calibration menu was opened.";
  
  fLabel = new TGLabel(fMain,txt);
  fMain->AddFrame(fLabel, LayCT1);
  fLabel->SetBackgroundColor(colr);

  memcpy(local_sD,opt.sD,sizeof(local_sD));
  // for (int i=0;i<11;i++) {
  //   pframe[i]=0;
  // }

  fMain->SetWindowName("Time Calibration");
  npeaks=1;

  AddNum(time_ref,11,"New peak position");

  hframe = new TGHorizontalFrame(fMain,10,10);
  //fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));
  fMain->AddFrame(hframe,LayBut1);

  TGTextButton* fTApply = new TGTextButton(hframe, "  &Apply  ");
  fTApply->Connect("Clicked()", "PopFrame", this, "Do_TApply()");
  hframe->AddFrame(fTApply, LayBut1a);

  TGTextButton* fCalibr = new TGTextButton(hframe, "  &Revert  ");
  fCalibr->Connect("Clicked()", "PopFrame", this, "Do_TRevert()");
  hframe->AddFrame(fCalibr, LayBut1a);
}

void PopFrame::AddPeaks() {
  fMain->SetWindowName("Peak Search");

  AddNum(opt.Peak_thr,1031,"Threshold","Peak threshold relative to the highest peak");
  AddNum(opt.Peak_smooth,1032,"Smooth","Smooth before peak search");
  AddNum(opt.Peak_wid,1033,"Width","Minimal width for peak search\npeaks with width below this number are rejected");
  AddNum(opt.Peak_bwidth,1034,"Background","width of the background");
  AddNum(opt.Peak_maxpeaks,1035,"MaxPeaks","Maximal number of peaks");
  AddChk(opt.Peak_show_sm,1036,"Show smoothed","Show smoothed histogram");
  AddChk(opt.Peak_use_mean,1037,"Use Mean/RMS","Use Mean/RMS insted of fit");
  AddChk(opt.Peak_print,1038,"Print","Print peak parameters");

  /*
  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));
  TGTextButton* fCalibr = new TGTextButton(hframe, "  &Calibr  ");
  fCalibr->Connect("Clicked()", "PopFrame", this, "Do_Tcalibr()");
  hframe->AddFrame(fCalibr, LayBut1);
  TGTextButton* fTApply = new TGTextButton(hframe, "  &Apply  ");
  fTApply->Connect("Clicked()", "PopFrame", this, "Do_TApply()");
  hframe->AddFrame(fTApply, LayBut1);
  */
}

void PopFrame::AddOnePar(TGCompositeFrame* fcont1, int id, void* var,
			 const char* nm, int mmin, int mmax) {
  hframe = new TGHorizontalFrame(fcont1,10,10);
  fcont1->AddFrame(hframe,LayLC2);

  //int nn=ps_all.pv.size();
  PClass pp;
  pp.name=nm;
  pp.var = var;
  pp.bb = opt.OptPar[id][0];
  pp.min = opt.OptPar[id][1];
  pp.max = opt.OptPar[id][2];
  pp.step = opt.OptPar[id][3];
  ps_all.pv.at(id) = pp;

  TGCheckButton *fchk = new TGCheckButton(hframe, "", id*10);
  fchk->SetState((EButtonState)opt.OptPar[id][0]);
  fchk->Connect("Toggled(Bool_t)", "PopFrame", this, "DoPar()");
  hframe->AddFrame(fchk, LayLC2);

  TGNumberEntry *fNum[3];
  fNum[0] = new TGNumberEntry(hframe, opt.OptPar[id][1]-1, 8, id*10+1,
			      kr,ka,kl,mmin,mmax);
  fNum[1] = new TGNumberEntry(hframe, opt.OptPar[id][2]-1, 8, id*10+2,
			      kr,ka,kl,mmin,mmax);
  fNum[2] = new TGNumberEntry(hframe, opt.OptPar[id][3]-1, 5, id*10+3,
			      kr,ka,kl,-10,10);

  for (int j=0;j<3;j++) {
    fNum[j]->GetNumberEntry()->Connect("TextChanged(char*)","PopFrame",
				       this,"DoPar()");
    //следующая команда вынуждает проверку min/max
    fNum[j]->IncreaseNumber(TGNumberFormat::kNSSSmall, 1, kFALSE);

    fNum[j]->SetWidth(ww[j+2]);
    hframe->AddFrame(fNum[j], LayLC0);
    // if (i==MAX_CH+NGRP)
    //   fAdj[i][j]->ChangeBackground(colr);
  }

  TGLabel* fLabel = new TGLabel(hframe, nm);
  hframe->AddFrame(fLabel, LayLC0);
  

}

void PopFrame::AddOptPar() {

  //memset(local_sD,0,sizeof(local_sD));
  // for (int i=0;i<11;i++) {
  //   pframe[i]=0;
  // }

  fMain->SetWindowName("Optimize Parameters");

  hframe = new TGHorizontalFrame(fMain,1,1);
  fMain->AddFrame(hframe, LayLC2);

  const char* aa[] = {" ","hist","A0","A1","A2"};
  for (int j=0;j<5;j++) {
    TGTextEntry* fLab = new TGTextEntry(hframe, aa[j]);
    fLab->SetState(false);
    fLab->ChangeOptions(fLab->GetOptions()|kFixedSize|kRaisedFrame);
    fLab->SetToolTipText("Pre-calibration coefficient");
    fLab->SetWidth(ww[j]);
    fLab->SetAlignment(kTextCenterX);
    hframe->AddFrame(fLab, LayLC0);
    //hframe->AddFrame(new TGLabel(hframe, aa[j]), LayLC2);
  }

  ps_all.pv.clear();
  ps_all.pv.resize(MOP);

  //AddNum(range,12,"+/- fit range");
  AddOnePar(fMain,0,(void*)opt.sS,"sS",-100,100);
  AddOnePar(fMain,1,(void*)opt.sTg,"sTg",1,7);
  AddOnePar(fMain,2,(void*)opt.sDrv,"sDrv",1,100);
  AddOnePar(fMain,3,(void*)opt.Mt,"Mt",0,3);
  AddOnePar(fMain,4,(void*)opt.DD,"DD",1,100);
  AddOnePar(fMain,5,(void*)opt.FF,"FF",1,10);
  AddOnePar(fMain,6,(void*)opt.B1,"B1",-1000,1000);
  AddOnePar(fMain,7,(void*)opt.B2,"B2",-1000,1000);
  AddOnePar(fMain,8,(void*)opt.P1,"P1",-1000,10000);
  AddOnePar(fMain,9,(void*)opt.P2,"P2",-1000,10000);
  AddOnePar(fMain,10,(void*)opt.T1,"T1",-100,100);
  AddOnePar(fMain,11,(void*)opt.T2,"T2",-100,100);
  AddOnePar(fMain,12,(void*)opt.W1,"W1",0,100);
  AddOnePar(fMain,13,(void*)opt.W2,"W2",0,100);

  hframe = new TGHorizontalFrame(fMain,10,10);
  fMain->AddFrame(hframe,new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 2,2,2,2));

  TGTextButton* fCalibr = new TGTextButton(hframe, "  &Start  ");
  fCalibr->Connect("Clicked()", "PopFrame", this, "Do_OptPar()");
  hframe->AddFrame(fCalibr, LayBut1);

  npeaks=1;
}

#ifdef CYUSB
void PopFrame::AddDevice() {
  fMain->SetWindowName("Select Device");

  fLabel = new TGLabel(fMain,"More than one device found. Select Device:");
  fMain->AddFrame(fLabel, LayCC3);

  fListBox = new TGListBox(fMain, 2);


  //char tmp[20];
  for (UInt_t i = 0; i < crs->cy_list.size(); ++i) {
    //sprintf(tmp, "Entry %i", i);
    fListBox->AddEntry(crs->cy_list[i].c_str(), i);
  }

  fListBox->Connect("DoubleClicked(Int_t)", "PopFrame", this, "CloseWindow()");

  //crs->idev=0;
  fListBox->Select(crs->idev);

  fListBox->Resize(150, 60);
  fMain->AddFrame(fListBox, LayCC4);

  TGTextButton* fClose = new TGTextButton(fMain, "  &Close  ");
  fClose->Connect("Clicked()", "PopFrame", this, "CloseWindow()");
  fMain->AddFrame(fClose, LayCB1);

  fMain->SetWMPosition(0,0);
}

#endif

#ifdef P_TEST
void PopFrame::AddTest() {
  fMain->SetWindowName("Test");

  delay = 0;
  n_iter = 1;
  AddNum(delay,901,"Delay (ms)");
  AddNum(n_iter,902,"N_iter");

  TGTextButton* fTest = new TGTextButton(fMain, "      &Test1      ",1);
  fTest->Connect("Clicked()", "PopFrame", this, "Do_Test()");
  fTest->SetToolTipText("Stop-Pusk (cmd4-cmd3) N times");
  fMain->AddFrame(fTest, LayBut3);

  fTest = new TGTextButton(fMain, "      &Test2      ",2);
  fTest->Connect("Clicked()", "PopFrame", this, "Do_Test()");
  fTest->SetToolTipText("Information N times");
  fMain->AddFrame(fTest, LayBut3);

  fTest = new TGTextButton(fMain, "      &Test3      ",3);
  fTest->Connect("Clicked()", "PopFrame", this, "Do_Test()");
  fTest->SetToolTipText("Command32(8) N times");
  fMain->AddFrame(fTest, LayBut3);

  fTest = new TGTextButton(fMain, "      &Test4      ",4);
  fTest->Connect("Clicked()", "PopFrame", this, "Do_Test()");
  fTest->SetToolTipText("myM->DoStartStop(1) N[x2] times");
  fMain->AddFrame(fTest, LayBut3);

}

void PopFrame::Do_Test() {
  TGButton *btn = (TGButton *) gTQSender;
  int id = btn->WidgetId();

  //int sz=0;
  if (crs->Fmode == 1) {
    for (int i=0;i<n_iter;i++) {
      cout << "test: " << i << " " << crs->Fmode << " " << id << endl;
      switch (id) {
      case 1:
	crs->Command2(4,0,0,0); //stop
	gSystem->Sleep(delay); //300
	crs->Command2(3,0,0,0); //pusk
	gSystem->Sleep(delay); //300
	break;
      case 2:
	//sz =
	crs->Command32(1,0,0,0); //info
	gSystem->Sleep(delay); //300
	break;
      case 3:
	//sz =
	crs->Command32(8,0,0,0); //сброс сч./буф.
	gSystem->Sleep(delay); //300
	break;
      case 4:
	//prnt("ssd d ds;",BMAG,"Press1: ",i,crs->b_acq,delay,RST);
	myM->test=true;
	myM->fStart->Clicked();
	gSystem->Sleep(delay); //300
	myM->fStart->Clicked();
	gSystem->Sleep(delay); //300
	myM->test=false;
	break;
      default:
	break;
      }
    }
  }
  cout << "test finished: " << endl;
}
#endif //P_TEST

void PopFrame::Do_Save_Ecalibr() {
  cout << "Calibration parameters are saved to 'ecalibr.dat'" << endl;
  fEdit->SaveFile("ecalibr.dat");
}

void PopFrame::DoPar() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  TGCheckButton *chk = (TGCheckButton*) gTQSender;

  Int_t id = te->WidgetId();
  int i = id/10;
  int j = id%10;

  switch (j) {
  case 0:
    ps_all.pv.at(i).bb=chk->GetState();
    opt.OptPar[i][j]=chk->GetState();
    //cout << i << " " << pv.at(i).bb << endl;
    break;
  case 1:
    ps_all.pv.at(i).min=te->GetNumber();
    opt.OptPar[i][j]=te->GetNumber();
    break;
  case 2:
    ps_all.pv.at(i).max=te->GetNumber();
    opt.OptPar[i][j]=te->GetNumber();
    break;
  case 3:
    ps_all.pv.at(i).step=te->GetNumber();
    opt.OptPar[i][j]=te->GetNumber();
    break;
  default:
    ;
  }

  //cout << "ij: " << i << " " << j << " " << opt.OptPar[2][2] << endl;

}

void PopFrame::DoAdj() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  Int_t id = te->WidgetId();
  int i = id/10;
  int j = id%10;

  opt.adj[i][j]=te->GetNumber();
  if (i==MAX_CH+NGRP) {
    for (int k=0;k<MAX_CH+NGRP;k++) {
      if (fAdj[k][j]) {
	opt.adj[k][j]=opt.adj[i][j];
	char text[256];
	snprintf(text, 255, "%g", opt.adj[i][j]);
	fAdj[k][j]->SetText(text,false);
      }
    }
  }
  HiFrm->HiUpdate();
}

void PopFrame::DoENum() {
  TGNumberEntryField *te = (TGNumberEntryField*) gTQSender;
  TGCheckButton *chk = (TGCheckButton*) gTQSender;

  Int_t id = te->WidgetId();

  //cout << "id: " << id << endl;

  switch (id) {
  case 11:
    time_ref=te->GetNumber();
    break;
  case 21:
    opt.E_auto=te->GetNumber();
    break;
  case 1031:
    opt.Peak_thr=te->GetNumber();
    break;
  case 1032:
    opt.Peak_smooth=te->GetNumber();
    break;
  case 1033:
    opt.Peak_wid=te->GetNumber();
    break;
  case 1034:
    opt.Peak_bwidth=te->GetNumber();
    break;
  case 1035:
    opt.Peak_maxpeaks=te->GetNumber();
    break;
  case 1036:
    opt.Peak_show_sm = chk->GetState();
    break;
  case 1037:
    opt.Peak_use_mean = chk->GetState();
    break;
  case 1038:
    opt.Peak_print = chk->GetState();
    HiFrm->pkprint = chk->GetState();
    break;
  case 901:
    delay = te->GetNumber();
    break;
  case 902:
    n_iter = te->GetNumber();
    break;
  default:
    ;
  }

  if (id/1000==1) {
    HiFrm->HiUpdate();
  }
  // else if (id==14) {
  //   npeaks=te->GetNumber();
  //   AddPeaks();
  // }
}

void PopFrame::Do_Auto_Ecalibr()
{

  for (int i=0;i<MAX_CH+NGRP+1;i++) {
    adj[i][0]=0;
    adj[i][1]=1;
    adj[i][2]=0;
  }

  
  double a1,a2;
  if (opt.b_stack)
    HiFrm->X_Slider(HiFrm->st_plot,a1,a2);

  for (size_t i = 0; i < HiFrm->pad_hist.size(); ++i) {
    TH1* hh = (TH1*) HiFrm->pad_hist[i];
    if (hh && hh->Integral()>0) {
      TString str(hh->GetName());
      if (str.Contains("area",TString::kIgnoreCase)) {
	int nn = HiFrm->pad_map[i]->nn;
	if (opt.b_stack)
	  hh->GetXaxis()->SetRangeUser(a1,a2);
	double xm = hh->GetXaxis()->GetBinCenter(hh->GetMaximumBin());
	opt.adj[nn][0] = 0;
	opt.adj[nn][1] *= opt.E_auto/xm;
	opt.adj[nn][2] = 0;
	//cout << "dr2: " << hh->GetName() << " " << xm << " " << nn << endl;
      }
    }
  }

  E_Update();
  HiFrm->HiUpdate();

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

      if (str.Contains("#1"))
	HiFrm->nofit=ee;
      else if (str.Contains("#2"))
	HiFrm->bpol=ee;
      else if (str.Contains("#3"))
	HiFrm->fitpol=ee;
      else if (str.Contains("#4"))
	HiFrm->fitsig=ee/2.35;
      else if (str.Contains("Peaks:"))
	b_peaks=true;

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

  HiFrm->Do_Ecalibr(this);

  E_Update();

}

void PopFrame::E_Update() {
  char text[256];
  for (int i=0;i<MAX_CH+NGRP;i++) {
    for (int j=0;j<3;j++) {
      if (fAdj[i][j]) {
         snprintf(text, 255, "%g", opt.adj[i][j]);
         fAdj[i][j]->SetText(text,false);
      }
    }
  }
}

void PopFrame::Do_EApply() {
  for (int i=0;i<MAX_CH;i++) {
    if (fAdj[i][0]) {
      switch (opt.calibr_t[i]) {
      case 2:
	prnt("ss d ss;",BRED,"quadratic calibration for ch:",i,"not possible",RST);
	break;
      case 1:
	//prnt("ss d f f fs;",BGRN,"E1:",i,opt.E0[i],opt.E1[i],opt.E2[i],RST);
	opt.E2[i] = opt.adj[i][2]*opt.E1[i]*opt.E1[i];
	opt.E1[i] = opt.E1[i]*(opt.adj[i][1] +
			       2*opt.adj[i][2]*opt.E0[i]);
	opt.E0[i] = opt.adj[i][0] + opt.E0[i]*(opt.adj[i][1]+
					       opt.adj[i][2]*opt.E0[i]);
	//prnt("ss d f f fs;",BGRN,"EE:",i,opt.E0[i],opt.E1[i],opt.E2[i],RST);
	break;
      case 0:
	opt.E0[i] = opt.adj[i][0];
	opt.E1[i] = opt.adj[i][1];
	opt.E2[i] = opt.adj[i][2];
	break;
      default:
	;
      }

      if (opt.adj[i][2])
	opt.calibr_t[i]=2;
      else
	opt.calibr_t[i]=1;
    }
  }
  chanpar->Update();
  //E_Update();
}

void PopFrame::Do_TApply() {

  for (UInt_t i=0;i<HiFrm->fitvect.size();++i) {
    //cout << "pad_map: " << i << " " << HiFrm->pad_map[i] << endl;
    if (HiFrm->pad_hist[i] && HiFrm->pad_hist[i]->InheritsFrom(TH1::Class()) &&
	TString(HiFrm->pad_hist[i]->GetName()).
	Contains("time",TString::kIgnoreCase)) {

      FClass *fits = &HiFrm->fitvect.at(i);
      Float_t dt = fits->vv.at(0).p;
      int nn = HiFrm->pad_map.at(i)->nn;
      opt.sD[nn]=local_sD[nn]-dt+time_ref;

      prnt("ss d s fs;",BGRN,"T:",nn,HiFrm->pad_map[i]->GetName(),dt,RST);
    }
    else
      prnt("ss ds;",BRED,"Wrong Pad:",i+1,RST);
  }

  chanpar->Update();

}

void PopFrame::Do_TRevert() {
  memcpy(opt.sD,local_sD,sizeof(local_sD));  
  chanpar->Update();
}

/*
void PopFrame::DoOpt(UInt_t it) {

  for (UInt_t i=0;i<pvect.size();++i) {
    for (int j=pvect.at(i).min;j<=pvect.at(i).max;j+=pvect.at(i).step) {
      pvect.at(i).ivar=j;
      prnt("ss ",BGRN,"pp:");
      for (UInt_t i=0;i<pvect.size();++i) {
	cout << " " << pvect.at(i).ivar;
      }
      prnt("s;",RST);
    }
  }

}
*/

void PopFrame::DoOpt(UInt_t i) {
  //cout << "it: " << i << " " << pvect.size() << endl;
  if (i>=ps.pv.size()) {

    pss.push_back(ps);

    // prnt("ss",BYEL,"pp:");
    // for (UInt_t k=0;k<pv.size();++k) {
    //   cout << " " << pv.at(k).ivar;
    // }
    // prnt("s;",RST);
    return;
  }
  for (int j=ps.pv.at(i).min;j<=ps.pv.at(i).max;j+=ps.pv.at(i).step) {
    //prnt("ss d ds;",BGRN,"p:",i,j,RST);
    ps.pv.at(i).ivar=j;
    DoOpt(i+1);
  }
}

void PopFrame::Do_OptPar()
{
  //cout << "sz: " << sizeof(opt) << endl;
  gzFile ff = gzopen("last.par","wb");
  crs->SaveParGz(ff,crs->module);
  gzclose(ff);
  

  char* opt2 = new char[sizeof(opt)];
  //Toptions opt2;
  memcpy(opt2, &opt, sizeof(opt));

  pss.clear();
  ps.pv.clear();

  for (auto it=ps_all.pv.begin();it!=ps_all.pv.end();++it) {
    if (it->bb) {
      ps.pv.push_back(*it);
    }
  }

  DoOpt(0);

  for (auto it=pss.begin();it!=pss.end();++it) {
    prnt("ss",BBLU,"pp:");
    for (UInt_t k=0;k<it->pv.size();++k) {
      cout << " " << it->pv.at(k).ivar;
    }
    prnt("s;",RST);
  }

  TList* fhist_list = new TList();
  //int nn=0;
  for (auto it=pss.begin();it!=pss.end();++it) {
     // if (nn>5555555)
     //   break;
     // nn++;

    //копируем параметры
    for (UInt_t k=0;k<it->pv.size();++k) {
      for (int m=0;m<MAX_CHTP;++m) {
	Int_t* zz = (Int_t*) it->pv.at(k).var;
	*(zz+m) = it->pv.at(k).ivar;
      }
      //cout << k << " " << it->pv.at(k).var << " " << &opt.sS << " " << opt.sS[7] << " " << opt.sTg[4] << endl;
    }

    HiFrm->DoRst();

    std::vector<std::vector<Float_t>> esave;
    for (auto evt = crs->Levents.begin();evt!=crs->Levents.end();++evt) {
      evt->T0=99999;
      evt->ChT0=255;
      esave.clear();
      for (auto pls = evt->pulses.begin();pls!=evt->pulses.end();++pls) {
	esave.push_back(pls->sData);
	crs->PulseAna(*pls);
	pls->Time+=pls->Tstamp64 - evt->Tstmp;
	if (opt.St[pls->Chan] && pls->Pos>-32222) {
	  Float_t T7 = pls->Time+opt.sD[pls->Chan]/opt.Period;
	  if (T7<evt->T0) {
	    evt->T0=T7;
	    evt->ChT0=pls->Chan;
	  }
	}
      }
      //cout << evt->Nevt << " " << evt->Tstmp << endl;
      Double_t hcut_flag[MAXCUTS] = {0}; //признак срабатывания окон
      hcl->FillHist(&*evt,hcut_flag);
      for (UInt_t i = 0; i < evt->pulses.size(); ++i) {
	evt->pulses[i].sData = esave[i];
      }
    }
    HiFrm->ReDraw();

    //FClass *fits = HiFrm->GetFits(hh);
    //if (fits) {

    cout << "pv: " << it->pv.size() << endl;
    TString ss;
    prnt("ss",BRED,"pp:");
    for (UInt_t k=0;k<it->pv.size();++k) {
      cout << " xxx " << endl;
      cout << " z " << it->pv.at(k).name << ": w " << it->pv.at(k).ivar;
      ss+=" ";
      ss+=it->pv.at(k).name;
      ss+=it->pv.at(k).ivar;
    }

    for (UInt_t i=0;i<HiFrm->fitvect.size();++i) {
      if (HiFrm->pad_hist[i] && HiFrm->pad_hist[i]->InheritsFrom(TH1::Class())){
	FClass *fits = &HiFrm->fitvect.at(i);
	Float_t w = fits->vv.at(0).w;
	cout << " " << w;
	TString s2 = fits->hst->GetTitle()+ss+" "+w;
	cout << s2 << endl;
	fits->hst->SetTitle(s2);
	fhist_list->Add(&*fits->hst);
      }
    }
    prnt("s;",RST);

  } //pss

  // hcl->fhist_list.Clear();
  // for (auto it=pss.begin();it!=pss.end();++it) {
  //   hcl->fhist_list.Add();
  // }

  hcl->Root_to_newtree("opt",fhist_list);


  delete fhist_list;
  char* opt3 = (char*) &opt;
  memcpy(opt3, opt2, sizeof(opt));

} //Do_OptPar

void PopFrame::Do_Default() {
  fEdit->LoadBuffer(
"# calibrate all marked energy histograms\n"
"# all ROI must be set before calibration\n"
"# each ROI may contain several peaks,\n"
"# fitted simultaneously\n"
"#--------------------------------------\n"
"\n"
"0       #1 nofit - set to 1 to draw fit functions w/o fit\n"
"2       #2 substrate poly - \n"
"2       #3 Fit poly\n"
"120     #4 fwhm\n"
"\n"
"-- Peaks: NROI peak1 pos1 peak2 pos2 ... --\n"
"-- (pos are relative to ROI borders [0..1]) --\n"
"-- (pos=0 - automatic peak search) --\n"
"-- peaks can be excluded by adding # in front --\n"
"\n"
"1 846.7778  0.5 # 56Fe(n,n')\n"
"2 1238.3    0.5 # 56Fe(n,n')\n"
"3 2223.245  0.5 # 1H(n,g)\n"
"4 4439.8    0.5 # 12C(n,n')\n"
"#5 5618.89   0.3 6129.89  0.7 # 16O(n,n')\n"
"5 7134.6    0.25 7645.58  0.6 # 56Fe(n,g)\n"
"#7 10100      0.6 # ???\n"
		    );
}

void PopFrame::Do_Revert() {
  memcpy(&opt.E0,&E0,sizeof(E0));
  memcpy(&opt.E1,&E1,sizeof(E1));
  memcpy(&opt.E2,&E2,sizeof(E2));
  memcpy(&opt.adj,&adj,sizeof(adj));
  memcpy(&opt.calibr_t,&calibr_t,sizeof(calibr_t));

  E_Update();
}
