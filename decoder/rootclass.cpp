#include "rootclass.h"
#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"

#include <iostream>

using namespace std;

rootclass::rootclass() { h_count = new TH1F("h_count", "h_count", 64, 0, 64); }

void rootclass::fillhist(pulse_vect *pulse) {

  for (pulse_vect::iterator it = pulse->begin(); it != pulse->end(); ++it) {
    // h_count->Fill(it->ch);
  }

  // for (UInt_t i=0;i<
  // h_count->Fill(
}

// void rootclass::bookhist() {
// }

void rootclass::saveroot(char *name) {

  TFile *tf = new TFile(name, "RECREATE");

  gROOT->cd();

  TIter next(gDirectory->GetList());

  tf->cd();

  TH1 *h;
  TObject *obj;
  while ((obj = (TObject *)next())) {
    if (obj->InheritsFrom(TH1::Class())) {
      h = (TH1 *)obj;
      // h->Print();
      if (h->GetEntries() > 0) {
        h->Write();
      }
    }
  }

  cout << "Histograms are saved in file: " << name << endl;

  tf->Close();
}
