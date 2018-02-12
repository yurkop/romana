/****************************************************************************
 *                                                                          *
 * df_decode.c: Sample decode program                                       *
 *                                                                          *
 * Compile: gcc -Wall -O2 df_decode.c -o df_decode                          *
 *                                                                          *
 ****************************************************************************/


#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>

#include <iostream>

#include "decoder.h"
#include "rootclass.h"

using namespace std;

rootclass* rt;

/*****************************************************************************/
/*
size_t process_raw_buf (unsigned char *buf, size_t len)
{

  cout << "len: " << len << endl;

  size_t p = 0, dp = 0;
  struct adcm_cmap_t cmap;
  struct adcm_counters_t counters;
  int nev = 0, nmap = 0, ncnt = 0;

  pulse_vect pulse;

  memset (&counters, 0, sizeof (struct adcm_counters_t));
  memset (&cmap, 0, sizeof (struct adcm_cmap_t));

  while (((dp = p + sizeof (struct stor_packet_hdr_t)) < len)) {

    struct stor_packet_hdr_t *hdr = (struct stor_packet_hdr_t *) & buf[p];

    switch (hdr->id) {
    case STOR_ID_CMAP:
      nmap++;
      memcpy (&cmap, &buf[dp], sizeof (struct adcm_cmap_t));
      break;
    case STOR_ID_EVNT:{
      stor_ev_hdr_t *eh = (struct stor_ev_hdr_t *) & buf[dp];
      nev++;
      pulse.clear();
      for (int n = 0; n < eh->np; n++) {
        stor_puls_t *hit = (struct stor_puls_t *) & buf[dp + sizeof (struct stor_ev_hdr_t) + n * sizeof (struct stor_puls_t)];
	pulse.push_back(*hit);
        //printf ("%d\t%f\t%f\t%f\n", hit->ch, hit->a, hit->t, hit->w);
      }

      rt->fillhist(&pulse);

      break;
    }
    case STOR_ID_CNTR:
      ncnt++;
      memcpy (&counters, &buf[dp], sizeof (struct adcm_counters_t));
      break;
    default:
      fprintf (stderr, "Bad block ID %04X at pos %zu\n", hdr->id, p);
      exit (1);
    }
    p += hdr->size;
  }

  print_adcm_counters (&counters);
  return (0);
}
*/

size_t process_buf (unsigned char *buf, size_t len)
{

  UShort_t* buf2;
  Long64_t* buf8;

  cout << "len: " << len << endl;

  size_t idx = 0;
  UChar_t cntr;

  pulse_vect rPeaks;

  //while (((dp = p + sizeof (struct stor_packet_hdr_t)) < len)) {
  while (true) {

    cntr = buf[idx];
    if (cntr!='D') {
      cout << "wrong control word: " << (Int_t)cntr << " " << (Int_t) 'D' 
	   << " " << idx << endl;
    }

    buf2 = (UShort_t*) (buf+idx+1);
    UShort_t sz = *buf2;
    if (idx+sz >len)
      break;

    buf8 = (Long64_t*) (buf+idx+3);

    Long64_t tst = (*buf8)&0xffffff;
    Char_t state = ((*buf8)&0xff000000)>>48;

    int nn = sz-3-sizeof(Long64_t);
    cout << "decode: " << nn << " " << 1.0*nn/sizeof(rpeak_type) << " " << sizeof(rpeak_type) << endl;

    if (idx+sz>=len)
      break;

    idx+=sz;
  }

  return (0);
}
/*****************************************************************************/

int main (int argc, char *argv[])
{
  int fd;
  struct stat statbuf;
  size_t maplen;
  off_t nb;
  void *mapaddr;
  char dumpfilename[1024];
  char rootfile[1024];

  rt = new rootclass;
  //rt->bookhist();

  //cout << argc << endl;
  //exit(1);

  if (argc == 3) {
    strcpy (dumpfilename, argv[1]);
    strcpy (rootfile, argv[2]);
  } else {
    cout << "usage: " << argv[0] << " datafile rootfile" << endl;
    //fprintf (stderr, "USAGE: %s datafile\n", argv[0]);
    exit (1);
  }

  cout << sysconf(_SC_PAGE_SIZE) << endl;

  if ((stat (dumpfilename, &statbuf))) {
    fprintf (stderr, "stat('%s') failed: %s\n", dumpfilename, strerror (errno));
    exit (1);
  }
  fd = open (dumpfilename, O_RDONLY);
  if (fd < 0) {
    fprintf (stderr, "open('%s') failed: %s\n", dumpfilename, strerror (errno));
    exit (1);
  }
  nb = statbuf.st_size;
  maplen=nb;
  //maplen=4096;

  cout << "maplen: " << maplen << endl;

  mapaddr = mmap (0, maplen, PROT_READ, MAP_SHARED, fd, 0);
  if (mapaddr == MAP_FAILED) {
    fprintf (stderr, "mmap('%s') failed: %s\n", dumpfilename, strerror (errno));
    exit (1);
  }
  //printf ("Mapped %zu bytes from %ld to addr %p\n", maplen, mapoff, mapaddr);

  process_buf ((unsigned char *)mapaddr, maplen);

  munmap (mapaddr, maplen);

  printf ("%ld bytes done \n", maplen);

  rt->saveroot(rootfile);
  delete rt;

  close (fd);
  return 0;
}
