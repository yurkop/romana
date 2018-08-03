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

const int MAX_CH=64; //max number of channels
const int ADDCH=9; //nai, bgo, si1, si2, stlb, demon, hpge, NIM, other

const int MAXCUTS=15; //20; //maximal number of cutG; should be less or equal to nr-1 of bits in hmap::bitwk; 
const int MAX_PCUTS=11; //maximal number of points in cutG
const int MAX_PADS=64; //maximal number of sub-pads in histframe

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
  ch_null,
  ch_nai,
  ch_bgo,
  ch_si1,
  ch_si2,
  ch_stilb,
  ch_demon,
  ch_hpge,
  ch_NIM,
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
  M_RESET_USB,
  M_EXPORT,
  M_FILE_EXIT,
  M_HELP,
};

#endif
