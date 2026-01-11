#ifndef common_H
#define common_H 1

#define ADC2
// #define ADCM

#ifdef ADC2
const int dstep = 5;
#endif
#ifdef ADCM
const int dstep = 10;
#endif

#include <TGFrame.h>
#include <TROOT.h>
#include <deque>
#include <zlib.h>

// #include "pulseclass.h"

// #define AVM16_MASK1 0x7ff00000  //to read new type of data
#define AVM16_MASK1 0x00000000 // to read old type of data. Works also with new.
#define AVM16_MASK2 0x000fffff // complementary to MASK1

const ULong64_t sixbytes = 0xFFFFFFFFFFFF;

const int Megabyte = 1024 * 1024; // 1 MB

const Long64_t TR_SIZE = 2 * Megabyte; // 2 MB - размер трансфера в байтах
const Long64_t OFF_SIZE = 2 * Megabyte; // 2 MB, на всякий случай
// OFF_SIZE должно быть >= макс. размера одного события (импульса)
// Для AK-32 это ~32kB
// 2 MB - больше 32kB

// const Long64_t DEC_SIZE = Megabyte; //1 MB

const int MAXLOG = 8192; // максимальная длина записи в логе
const int MAX_CH = 256; // 256; //max number of channels
const int MAX_TP = 8;   // 8 types, MAX_TP=other, MAX_TP+1=copy, MAX_TP+2=swap
const int MAX_CHTP = MAX_CH + MAX_TP + 1;
const int NGRP = 4; // number of groups of channels
// const int MAX_AXAY = sqrt(MAX_CH); //maximal number of channels used in
// AXAY-histograms

const int MAXROI = 20; // maximal number of ROI

const int MAXCUTS = 31; // maximal number of cuts; Если 31 - выдает ошибку (?).
                        // не может быть больше 31=число бит в Int_t.
const int MAX_PCUTS = 11; // maximal number of points in cutG
const int MAX_PADS = 256; // maximal number of sub-pads in histframe

// const int MAX_ERR=18; //maximal number of error types;

// const int A0A1_MAX=2; // maximal number of input channels used in A0A1 2d
// hists

const int NDIV = 5; // количество графов в eventframe
                    // ==максимальное количество делений окна Events

#define MAX_R 4 // number of channel types (gam,ng,nim,off)
#define MAX_L 6 // line types (gam,neu,tail,unkn,pileup,frame)
#define MAX_P 5 // particle types (gam,neu,tail,unkn,pileup)

// #define BSIZE 131072 //1024*128
// #define BOFFSET 131072 //1024*128
#define DSIZE 131072
#define MAXRING 10

#define MXLN 40
#define MXNUM 60

const int MAIN_WIDTH = 958;  // width of the main window
const int MAIN_HEIGHT = 694; // height of the main window
// const int MAIN_WIDTH = 630; //width of the main window
// const int MAIN_HEIGHT = 550; //height of the main window

const int W2_HEIGHT = 210; // height of the split frame containing ch_types

const int HFRAME_WIDTH = 400; // width of histogram frames

const int MXPK = 14; // eventframe

const double mks = 0.001;

enum MENU_COM {
  M_READINIT,
  M_SAVEINIT,
  M_READROOT,
  M_SAVEROOT,
  M_SAVEASCII,
  M_FILE_BROWSE,
  M_RESET_USB,
  M_DETECT_DEV,
  M_EXPORT,
  M_EXIT,
  M_EDIT_PROF8,
  M_EDIT_PROF64,
  M_EDIT_CUTG,
  M_PROF_TIME,
  // M_PRECALIBR,
  M_ECALIBR,
  M_TCALIBR,
  M_PEAKS,
  M_OPTPAR,
  M_DEVICE,
  // #ifdef YUMO
  //   M_YUMO,
  // #endif
  M_TEST,
  // M_HELP,
  M_COUNT // нужно для вектора pops
};

enum ERR_NUM {
  // ER_START = 0,
  ER_PBUF = 0,
  ER_CH,
  ER_MIS,
  ER_FRMT,
  ER_LEN,
  ER_ZERO,
  ER_ALEN,
  ER_TST,

  ER_AREA,
  ER_BASE,
  ER_WIDTH,
  ER_TIME,
  ER_RTIME,

  ER_ANA,
  ER_DEC,
  ER_LAG,
  ER_OVF,
  ER_CFD,
  ER_HEAT,
};
const int MAX_ERR = ER_HEAT + 1;

//---------------------
enum P_Def {
  p_null, // 0
  p_fnum,
  p_fnum2,
  p_inum,
  p_chk,
  p_cmb,
  p_txt,
  p_but,
  p_chn, // 7
  p_stat // -> double
};

struct Pmap {
  TGFrame *field; // address of the input widget
  void *data;     // base address of the parameter
  void *data2;    // base address of the second (parallel) parameter
  UInt_t cmd;     // опции (биты)
  UShort_t off;   // offset to the base address in units of step
  P_Def type;     // p_fnum p_inum p_chk p_cmb p_txt p_but p_chn p_stat
  UChar_t all;    // 1 - all/ALL/* parameters; >1 - channel type
  UChar_t step;   // step in units of _type_size

  // cmd bits:
  // 0x1: (bit0) 1: start/stop DAQ
  // 0xE: (bit1-3) ( << 1) change color (see fCol[7])
  // 0xF0: (bit4-7) Action (1..15)
  // 0x100 (bit8) disble during acq
  // 0x200 (bit9) disble fields not existing in certain devices
  // 0x400 (bit10) enable/disble fields for ntof analysis
  // 0x800 (bit11) disble fields for file analysis (mostly cpar)

  // Action (bit4-7):
  //  in DoDaqNum:
  //  1 - DoReset
  //  2 - Hireset
  //  3 - Hi->Update()
  //  // 4 - match Trg & Drv for CRS2 (not realized)
  //  5 - group4
  //  6 - проверка Len кратно 3 или 4
  //  7 - установка tsleep в Timer
  //  8 - chanpar->Update() (для отображения второго параметра apar2)

  // in DoDaqChk:
  // 5 - group4 for module51

  /*
  // in UpdateField (chk):
  // 2 - 2d hist (2 fields)
  // 3 - 1d hist (4 fields)
  // 4 - profilometer hist
  */

  // UChar_t chan; //for Command_crs :seems to be not needed (21.01.2020)
};

#endif
