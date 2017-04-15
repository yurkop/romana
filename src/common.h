#ifndef common_H
#define common_H 1

#define ADC2
//#define ADCM

#ifdef ADC2
const int dstep = 5;
#endif
#ifdef ADCM
const int dstep = 10;
#endif

#include <zlib.h>
#include <TROOT.h>
#include <deque>

#include "pulseclass.h"

//#define AVM16_MASK1 0x7ff00000  //to read new type of data
#define AVM16_MASK1 0x00000000  //to read old type of data. Works also with new.
#define AVM16_MASK2 0x000fffff //complementary to MASK1

const int MAX_CH=32; //max number of channels
const int ADDCH=9; //nai, bgo, si1, si2, stlb, demon, hpge, NIM, other

#define MAX_R 4 //number of channel types (gam,ng,nim,off)
#define MAX_L 6 //line types (gam,neu,tail,unkn,pileup,frame)
#define MAX_P 5 //particle types (gam,neu,tail,unkn,pileup)

enum ChannelDef {
  ch_off2,
  ch_nim,
  ch_gam,
  ch_ng
};

enum ChDef {
  ch_nai,
  ch_bgo,
  ch_si1,
  ch_si2,
  ch_stilb,
  ch_demon,
  ch_hpge,
  ch_other,
  ch_empty
};

//#define BSIZE 131072 //1024*128
//#define BOFFSET 131072 //1024*128
#define DSIZE 131072
#define MAXRING 10

#define MXLN 40
#define MXNUM 60


enum MENU_COM {
  M_READINIT,
  M_SAVEINIT,
  M_READROOT,
  M_SAVEROOT,
  M_FILE_BROWSE,
  M_FILE_NEWCANVAS,
  //M_FILE_OPEN,
  //M_FILE_SAVE,
  M_FILE_EXIT,
  //M_THRESH,
  //M_PARAM,
  M_DEMON,
  M_SYNC,
  //M_MKMON,
  //M_MAKAN,
  M_HELP,
  M_TEST,
  //M_SI,
  //M_SIMAX,
  M_E0_7,
  M_E8_15,
  M_E16_23,
  M_E24_31,
  M_T0_7,
  M_T8_15,
  M_T16_23,
  M_T24_31,
  M_TOF0_7,
  M_TOF8_15,
  M_TOF16_23,
  M_TOF24_31,
  /*
  M_1_6,
  M_7_12,
  M_13_18,
  M_19_24,
  M_25_30,
  M_27_32,
  M_T1_6,
  M_T7_12,
  M_T13_18,
  M_T19_24,
  M_T25_30,
  M_T27_32,
  */
  //M_CHANNELS,

  //#ifdef ROMASH
  M_TOF0_5,
  M_TOF6_11,
  M_TOF12_17,
  M_TOF18_23
  //#endif
};

#endif
