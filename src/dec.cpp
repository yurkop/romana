#include "dec.h"
#include "TRandom.h"

#include "romana.h"
// const ULong64_t sixbytes=0xFFFFFFFFFFFF;

#define ID_EVNT 0x5645

extern MyMainFrame *myM;

thread_local PulseClass good_pulse(0);
thread_local TRandom rnd;

bool Dec79(BufClass &Buf, EventClass &evt) {
  // признак начала события: бит63=1

  // sD = opt.sD[ipls->Chan]/opt.Period; //in samples

  // предполагаем, что при вызове Dec79 u82 всегда указывает на начало события
  evt.Tstmp = *Buf.u82.ul & sixbytes;
  evt.Spin |= Buf.u82.b[6]; // проверить!!! 6 или 7?
  ++Buf.u82.ul;

  while (true) {
    if (Buf.u82.b + 7 >= Buf.b3)
      return false; // Сначала границы
    if (Buf.u82.b[7] & 0x80)
      break; // Потом бит события

    pulse_vect::iterator ipls = evt.pulses.insert(evt.pulses.end(), good_pulse);

    ipls->Chan = Buf.u82.us[3];

    if (evt.Spin & 128) { // Counters
      ipls->Counter = *Buf.u82.ul & sixbytes;
      ipls->Pos = -32222;
      ipls->Time = 0; // нужно для nTof
      // prnt("ss l ds;",BRED,"d79:",evt.Tstmp,evt.Spin,RST);
    } else { // Peaks
      ipls->Area = (Buf.u82.us[0] + rnd.Rndm() - 1.5) * 0.2;
      ipls->Time = (Buf.u82.s[1] + rnd.Rndm() - 0.5) * 0.01;
      //+ opt.sD[ipls->Chan]/opt.Period; //in samples
      ipls->Width = (Buf.u82.s[2] + rnd.Rndm() - 0.5) * 0.001;

      ipls->Height = ((UInt_t)Buf.u82.b[7]) << 8;

      // if (opt.St[ipls->Chan] && ipls->Time < evt.T0) {
      // 	evt.T0=ipls->Time;
      // 	evt.ChT0=ipls->Chan;
      // }
      // ipls->Ecalibr(ipls->Area);
    }
    ++Buf.u82.ul;
  }
  return true;
}

/*
void Dum79(BufClass* Buf) {
  std::list<BufClass> buf_list;
  buf_iter buf_it = buf_list.insert(buf_list.end(),BufClass());


  for (int i=0;i<1000000;i++) {
    EventClass evt;
    Dec79(*buf_it,evt);
  }
}
*/

void BufClass::Ring_Write(BufClass &buf) {
  // нужно добавить проверку на доступность буфера (гонка с анализом)

  size_t size = buf.b3 - buf.b1;
  if (u82.b + size <= b3) {
    // Данные помещаются
    memcpy(u82.b, buf.b1, size);
    u82.b += size;
  } else {
    // Данные не помещаются - записываем в начало
    memcpy(b1, buf.b1, size);
    u82.b = b1 + size;
  }

  double iold = 1. * (u82.b - size - b1) / Megabyte;
  double inew = 1. * (u82.b - b1) / Megabyte;

  double prc = 1. * (u82.b - b1) / (b3 - b1) * 100;
  // myM->fHProgr1->SetPosition(prc);

  prnt("ss f f fs;", BMAG, "wrt:", iold, inew, prc, RST);
}

/*
void BufClass::Ring_Write(const UChar_t* data, size_t data_size) {
  // нужно добавить проверку на доступность буфера (гонка с анализом)

  if (u82.b + data_size <= b3) {
    // Данные помещаются
    memcpy(u82.b, data, data_size);
    u82.b += data_size;
  } else {
    // Данные не помещаются - записываем в начало
    memcpy(b1, data, data_size);
    u82.b = b1 + data_size;
  }

  double iold = 1.*(u82.b-data_size-b1)/Megabyte;
  double inew = 1.*(u82.b-b1)/Megabyte;

  double prc = 1.*(u82.b-b1)/(b3-b1)*100;
  myM->fHProgr1->SetPosition(prc);

  prnt("ss f f fs;",BMAG,"wrt:",iold,inew,prc,RST);
}
*/
