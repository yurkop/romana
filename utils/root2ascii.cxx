//#include "root2ascii.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <TROOT.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TClass.h>
#include <TKey.h>

using namespace std;

//char* name;

void saveascii(int nopt, TH1* hh) {

  char fname[100];
  strcpy(fname,hh->GetName());
  strcat(fname,".dat");

  cout << "name: " << fname << endl;

  FILE* fout;

  fout=fopen(fname,"w");
  int nn=hh->GetNbinsX();
  for (int i=1;i<=nn;i++) {
    double xx = hh->GetBinCenter(i);
    double yy = hh->GetBinContent(i);

    if (nopt==0) {
      fprintf(fout,"%0.2f %0.1f\n",xx,yy);
    }
    else {
      fprintf(fout,"%d %0.1f\n",i,yy);
    }
  }

  fclose(fout);

}

void saveascii2(TH2* hh) {

  char fname[100];
  strcpy(fname,hh->GetName());
  strcat(fname,".dat");

  cout << "2d name: " << fname << endl;

  FILE* fout;

  fout=fopen(fname,"w");
  int nx=hh->GetNbinsX();
  int ny=hh->GetNbinsY();
  for (int j=1;j<=ny;j++) {
    for (int i=1;i<=nx;i++) {
      //double xx = hh->GetBinCenter(i);
      //double yy = hh->GetBinCenter(j);
      double zz = hh->GetBinContent(i,j);
      fprintf(fout," %0.0f",zz);
    }
    fprintf(fout,"\n");
  }

  fclose(fout);

}

void readfile(char* opt, char* name) {

  //TString s_opt = TString(opt);
  //cout << s_opt << endl;
  int nopt=0;

  if (TString(opt).EqualTo("1x",TString::kIgnoreCase)) {
    nopt=0;
  }
  else if (TString(opt).EqualTo("1i",TString::kIgnoreCase)) {
    nopt=1;
  }
  else if (TString(opt).EqualTo("2d",TString::kIgnoreCase)) {
    nopt=2;
  }
  else {
    cout << "Wrong option. Run program without arguments to see possible options" << endl;
    exit(-1);
  }

  TFile * tf = new TFile(name,"READ");
  if (!tf) {
    printf("File %s doesn'e exist\n",name);
    exit(-1);
  }

  cout << "-----------" << endl;

   TIter next(tf->GetListOfKeys());
   TKey *key;
   TCanvas c1;
   //c1.Print("hsimple.ps[");
   while ((key = (TKey*)next())) {
      TClass *cl = gROOT->GetClass(key->GetClassName());
      //cout << strcmp(cl->GetName(),"TH1") << endl;
      if (nopt<2 && strncmp(cl->GetName(),"TH1",3)==0) {
	TH1 *hh = (TH1*)key->ReadObj();
	saveascii(nopt,hh);
      }
      else if (nopt==2 && strncmp(cl->GetName(),"TH2",3)==0) {
	TH2 *hh = (TH2*)key->ReadObj();
	saveascii2(hh);
	//cout << "clname: " << cl->GetName() << " " << hh->GetName() << endl;
      }
      //h->Draw();
   }
   //tf->GetList()->Print();

   tf->Close();

}

int main (int argc, char **argv)
{

  if (argc < 3) {
    cout << "usage:" << endl;
    cout << argv[0] << " 1x rootfile" << endl;
    cout << "- convert all 1d histograms to ascii, write bin center as x coordinate" << endl;
    cout << argv[0] << " 1i rootfile" << endl;
    cout << "- convert all 1d histograms to ascii, write bin number as x coordinate" << endl;
    cout << argv[0] << " 2d rootfile" << endl;
    cout << "- convert all 2d histograms to ascii, write z-values as XY-table" << endl;
    exit(-1);
  }

  //name=argv[1];

  //TH1D* dummy = new TH1D("dummy", "dummy", 10, 0, 1);
  //TH2D* dummy2 = new TH2D("dummy2", "dummy2", 10, 0, 1, 10, 0, 1);

  //TPaletteAxis* tpal = new TPaletteAxis();

  readfile(argv[1],argv[2]);

}
