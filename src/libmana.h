#ifndef libmana_H
#define libmana_H 1

#include "common.h"

using namespace std;

#include "mainframe.h"
//#include "chandlg.h"
//#include "numdlg.h"
//#include "pardlg.h"

//#ifdef ROMASH
#include "romana.h"
//#endif

#include <TH1.h>

void SplitFilename(string str, char *folder, char *name);
void SplitFilename(string str, char *folder, char *name, char* ext);
void new_hist();
void set_hist_attr();
void saveroot(char *name);
void readroot(char *name);
void clear_hist();
void greset();
//int newfile();
void readinit(const char* fname);
void saveinit(const char* fname);
void smooth(int n, int i);
void fill_nim(int ch);
void nim_peak(int ch, int j);
void findpeaks_nim(int ch, int n2, unsigned short* dat);
void peakana_ng(int ch, int n2, double* dat);
void findpeaks_ng(int ch, int n2, double* dat);
void peakana_gam(int ch, int n2, double* dat);
void findpeaks_gam(int ch, int n2, double* dat);
int deltat(int ng, unsigned int tt);
void startbeam(int ch);
int analyze();
//int searchsync();
//void fillevent();
//void fillnewevent();
//int nextevent();
void anabuf_adc2();
void anabuf_avm16();
void anabuf_adcm();
//int readbuf();
short int bits(int n, int i1, int i2);
//void readmonitor(char* fname);
void fitpeak(TH1* hh, double ww);
int getmax(TH1F* hist[]);
void swap_bytes(unsigned short* buf);

//void drawevent(int i, int opt_ch, int deriv);

void allevents();
void example();

void dumpbuf(int nn);
void dumpevent();

void mkstart();
void mktof();
void peaktime(int ch, double* dat, int method, int twin);

void FillHist(EventClass* evt);

typedef struct {

  Char_t nch; // number of channels in the event
  Char_t ch[MAX_CH]; // ch[nch] - current channel number
  Char_t flag[MAX_CH]; //flag[nch] - flag
  Float_t E[MAX_CH];  //E[nch] - energy
  Float_t T[MAX_CH];  //T[nch] - time
  Float_t W[MAX_CH];  //W[nch] - width
  //Int_t npk; // total number of peaks (can be many in one channel)
  //Int_t fpk[MAX_CH]; // pk[nch] - first peak ...

} event_t;

/*
class MFileDialog: public TGFileDialog {

 public:
  //MFileDialog(int &fType, const TGWindow* p = 0, const TGWindow* main = 0, EFileDialogMode dlg_type = kFDOpen, TGFileInfo* file_info = 0);
  MFileDialog(const TGWindow* p = 0, const TGWindow* main = 0, EFileDialogMode dlg_type = kFDOpen, TGFileInfo* file_info = 0);
  virtual ~MFileDialog() {};

};
*/

//class HClass {
//}

#endif
