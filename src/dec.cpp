#include "dec.h"
#include "TRandom.h"

#include "romana.h"
// NOTE: sixbytes defined in romana.h
// const ULong64_t sixbytes = 0xFFFFFFFFFFFF; // ← do not uncomment

#define ID_EVNT 0x5645

extern MyMainFrame *myM;

thread_local PulseClass good_pulse(0,0);
thread_local TRandom rnd;

bool Dec79(BufClass &Buf, EventClass &evt) {
  // признак начала события: бит63=1

  // sD = opt.sD[ipls->Chan]/opt.Period; //in samples

  // предполагаем, что при вызове Dec79 u82 всегда указывает на начало события
  evt.Tstmp = *Buf.u82.ul & sixbytes;
  evt.Spin |= Buf.u82.b[6]; // Spin из байта 6
  ++Buf.u82.ul;

  while (true) {
    if (Buf.u82.b + 7 >= Buf.write_end)
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
  size_t size = buf.write_end - buf.write_start;
  if (u82.b + size < write_end) {
    // Данные помещаются
    memcpy(u82.b, buf.write_start, size);
    u82.b += size;
  } else {
    // Данные не помещаются - переносим хвост в начало
    size_t part1 = write_end - u82.b;
    size_t part2 = size - part1;
    memcpy(u82.b, buf.write_start, part1);
    memcpy(write_start, buf.write_start+part1, part2);
    u82.b = write_start + part2;
  }

  // double iold = 1. * (u82.b - size - write_start) / Megabyte;
  // double inew = 1. * (u82.b - write_start) / Megabyte;

  double prc = 1. * (u82.b - write_start) / (write_end - write_start) * 100;
  // myM->fHProgr1->SetPosition(prc);

  // prnt("ss fs;", BYEL, "wrt%:", prc, RST);
}

/*
void BufClass::Ring_Write(const UChar_t* data, size_t data_size) {
  // нужно добавить проверку на доступность буфера (гонка с анализом)

  if (u82.b + data_size <= write_end) {
    // Данные помещаются
    memcpy(u82.b, data, data_size);
    u82.b += data_size;
  } else {
    // Данные не помещаются - записываем в начало
    memcpy(write_start, data, data_size);
    u82.b = write_start + data_size;
  }

  double iold = 1.*(u82.b-data_size-write_start)/Megabyte;
  double inew = 1.*(u82.b-write_start)/Megabyte;

  double prc = 1.*(u82.b-write_start)/(write_end-write_start)*100;
  myM->fHProgr1->SetPosition(prc);

  prnt("ss f f fs;",BMAG,"wrt:",iold,inew,prc,RST);
}
*/
