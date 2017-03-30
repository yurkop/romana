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

void saveascii(TH1* hh) {

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

    fprintf(fout,"%f %f\n",xx,yy);
  }

  fclose(fout);

}

void readfile(char* name) {

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
      if (strncmp(cl->GetName(),"TH1",3)==0) {
	//if (cl->InheritsFrom("TH1")) {
	TH1 *hh = (TH1*)key->ReadObj();
	saveascii(hh);
	//cout << "name: " << h->GetName() << endl;
      }
      //cout << "clname: " << cl->GetName() << endl;
      //h->Draw();
   }
   //tf->GetList()->Print();

   tf->Close();

}

int main (int argc, char **argv)
{

  if (argc < 2) {
    printf("usage: %s rootfile\n",argv[0]);
    exit(-1);
  }

  //name=argv[1];

  //TH1D* dummy = new TH1D("dummy", "dummy", 10, 0, 1);
  //TH2D* dummy2 = new TH2D("dummy2", "dummy2", 10, 0, 1, 10, 0, 1);

  //TPaletteAxis* tpal = new TPaletteAxis();

  readfile(argv[1]);

}
