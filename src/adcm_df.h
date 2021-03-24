#ifndef ADCM_DF_H
#define ADCM_DF_H 1

// ISO C99
#include <inttypes.h>
#include <sys/types.h>
//#include <vector>

//#include "adcm_16.h"

#define ADC_NCH 64

//#define STOR_ID_CMAP  0x504D    /* 'MP' */
//#define STOR_ID_EVNT  0x5645    /* 'EV' */
//#define STOR_ID_CNTR  0x5443    /* 'CT' */

struct stor_packet_hdr_t {
  u_int16_t id;                 // data block ID
  u_int16_t size;               // block size, bytes
} __attribute__ ((packed));

struct stor_ev_hdr_t {
  u_int8_t np;                  // number of pulses following
  u_int8_t reserved1;
  u_int16_t reserved2;
  u_int32_t ts;                 // timestamp, 10 ns step
} __attribute__ ((packed));

struct stor_puls_t {
  u_int8_t ch;
  u_int8_t flags;
  float a;
  float t;
  float w;
} __attribute__ ((packed));

struct adcm_cmap_t {
  u_int32_t n;
  u_int8_t map[ADC_NCH];
};

struct adcm_counters_t {
  u_int32_t n;
  double time;
  u_int32_t rawhits[ADC_NCH];
};

//typedef std::vector<stor_puls_t> pulse_vect;

#endif /* ADCM_DF_H */
