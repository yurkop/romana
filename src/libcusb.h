#ifndef cusb_H
#define cusb_H 1

#ifdef CYUSB

#include "cyusb.h"

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <vector>
#include <string>

#include <RQ_OBJECT.h>
#include <TFile.h>
#include <TFormula.h>
#include <TMath.h>
#include "TThread.h"

#include "common.h"
#include "dec.h"
#include "pulseclass.h"

#include <queue> // std::queue

using namespace std;

// forward
class CRS;

//---------------------------
class CUSB {

public:
  CUSB();
  virtual ~CUSB();

  //--------constants---------
  static const int MAXTRANS = 7;       // было 8

  //--------variables---------

  //---CRS
  short idev; // номер девайса в cyusb [0,1,2...].
              // Если =-1 -> номер девайса еще не выбран
  short ndev; // количество найденных модулей
  vector<string> cy_list; // список имен ВСЕХ подключенных девайсов
  TString devname; // имя, заданное в '-m'. Если =0 -> не задано

  cyusb_handle *cy_handle;

  // buffers for sending parameters...
  UChar_t buf_out[64];
  UChar_t buf_in[64];

  struct libusb_transfer *transfer[MAXTRANS];

  UInt_t mask_discr[MAX_CH + 1]; // маска записи по дискриминатору (ЦРС32) и по
                                 // СС и пересчету (ЦРС8+)

  int chan_in_module;

  int event_thread_run;
  TThread *trd_crs;

  //--------functions---------

  void DoDetectDev();
  void DoResetUSB();

  int Open_USB();
  void Set_USB(int i);
  void Device_info();
  int Init_device();
  int SetPar();
  void Free_Transfer();
  void Submit_all(int ntr);
  void Cancel_all(int ntr);
  int Init_Transfer3();
  int Init_Transfer();
  int Command32_old(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  void Check33(UChar_t cmd, UChar_t ch, int &a1, int &a2, int min, int max);
  int Command2(UChar_t cmd, UChar_t ch, UChar_t type, int par);
  void AllParameters2(); // load all parameters
  void AllParameters_AK32();
  void AllParameters44();     // load all parameters
  void AllParameters36();     // load all parameters
  void AllParameters35();     // load all parameters
  void AllParameters34();     // load all parameters
  void AllParameters33();     // load all parameters
  void AllParameters32();     // load all parameters
  int DoStartStop(int rst);   // start-stop acquisition
  void ProcessCrs(int rst);   // process events in dostartstop
  void DoExit();
  
  //ClassDef(CUSB, 0)
};

#endif // CYUSB

#endif
