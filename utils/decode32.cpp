#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <zlib.h>

typedef unsigned char UChar_t;
typedef unsigned short UShort_t;
typedef unsigned long long ULong64_t;
typedef long long Long64_t;
typedef float Float_t;

const int chanPresent = 32;         // maximal channel number
const unsigned int MAXSIZE = 32763; // maximal pulse length
int buf_size = 1024 * 4;            // read buffer size

class PulseClass2 {
public:
  Long64_t Tstamp64;          // time stamp
  Long64_t Counter;           // pulse counter
  std::vector<Float_t> sData; //(maybe smoothed) pulse data
  UChar_t Chan;               // channel number
  UChar_t Control;            // Control word
public:
  PulseClass2(){};
  virtual ~PulseClass2(){};
};

std::vector<PulseClass2> vv; // vector of pulses
int npulses = 0;
int nbuf = 0;

using namespace std;

void Decode2(UChar_t *buffer, int length) {}

void Decode32(UChar_t *buffer, int length) {

  PulseClass2 *ipp; // pointer to the current pulse

  ULong64_t *buf8 = (ULong64_t *)buffer;

  if (vv.empty()) { // this is start of the acqisition
    vv.push_back(PulseClass2());
    npulses++;
    ipp = &vv.back();
    ipp->Chan = buffer[7];
  } else {
    ipp = &vv.back();
  }

  unsigned short frmt;
  int idx8 = 0;
  int idx1 = 0;
  ULong64_t data;

  while (idx1 < length) {
    frmt = buffer[idx1 + 6];
    int cnt = frmt & 0x0F;
    frmt = (frmt & 0xF0) >> 4;
    data = buf8[idx8] & 0xFFFFFFFFFFFF;
    unsigned char ch = buffer[idx1 + 7];

    // printf("decode: %d %d %ld %d %d\n",idx8,idx1,ipp->sData.size(),frmt,ch);

    if ((ch >= chanPresent) || (frmt && ch != ipp->Chan)) {
      cout << "32: Bad channel: " << (int)ch << " " << (int)ipp->Chan << endl;
      // ipp->ptype|=P_BADCH;

      idx8++;
      idx1 = idx8 * 8;
      continue;
    }

    if (frmt == 0) {
      // make new pulse only if this is not the first record.
      // For the first record new pulse is already created at the beginning

      if (idx8) {
        vv.push_back(PulseClass2());
        npulses++;
        ipp = &vv.back();
      }
      // else { //idx8==0 -> pulse has start
      // }
      ipp->Chan = ch;
      ipp->Tstamp64 = data;

    } else if (frmt == 1) {
      ipp->Control = buffer[idx1 + 5] + 1;
      ipp->Counter = data & 0xFFFFFFFFFF;
    } else if (frmt == 2) {
      if ((int)ipp->sData.size() >= MAXSIZE) {
        cout << "32: ERROR Nsamp: " << nbuf << " " << ipp->sData.size() << endl;
      } else {
        for (int i = 0; i < 4; i++) {
          int zzz = data & 0xFFF;
          ipp->sData.push_back((zzz << 20) >> 20);
          data >>= 12;
        }
      }
    }

    idx8++;
    idx1 = idx8 * 8;
  }

  // nvp = (nvp+1)%ntrans;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    printf("usage: %s inp_file\n", argv[0]);
    exit(-1);
  }

  gzFile f_raw = gzopen(argv[1], "rb");

  UShort_t mod[2];
  gzread(f_raw, mod, sizeof(mod));

  UChar_t *Fbuf;
  UChar_t *header;
  int Fmode = mod[0];
  int h_size = mod[1];

  printf("mode: %d header_size: %d\n", Fmode, h_size);

  header = new UChar_t[h_size];
  Fbuf = new UChar_t[buf_size];

  // read and skip header
  gzread(f_raw, header, h_size);

  int res = 0;
  nbuf = 0;
  npulses = 0;
  ULong64_t totalbytes = 0;

  while ((res = gzread(f_raw, Fbuf, buf_size))) {
    totalbytes += res;

    if (Fmode == 2) {
      Decode2(Fbuf, res);
    } else if (Fmode == 32) {
      Decode32(Fbuf, res);
    }

    nbuf++;
    // printf("gzread: %d %d %lld\n", nbuf, res, totalbytes);
  }

  delete[] header;
  delete[] Fbuf;
  gzclose(f_raw);

  // print parameters of all pulses
  for (int i = 0; i < vv.size(); i++) {
    printf("pulse: %d %d %ld %lld %lld\n", i, vv[i].Chan, vv[i].sData.size(),
           vv[i].Counter, vv[i].Tstamp64);
  }

  // print waveform of the first pulse
  if (vv.size() > 0) {
    for (int i = 0; i < vv[0].sData.size(); i++) {
      printf("data: %d %f\n", i, vv[0].sData[i]);
    }
  }

  printf("total bytes: %lld total pulses: %ld\n", totalbytes, vv.size());
}
