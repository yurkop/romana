#include "decoder.h"
#include <iostream>

//extern Long64_t b_start[CRS::MAXTRANS];

DecoderClass::DecoderClass() {
  //zfile=;
}

DecoderClass::~DecoderClass() {
  gzclose(*zfile);
  *zfile=0;
}

/*
void DecoderClass::Reset_Dec(Short_t mod, Int_t compr) {
  // сбрасывает декодер перед началом записи в файл

  sprintf(dec_opt,"wb%d",compr);

  zfile = gzopen(zfname.c_str(),dec_opt);
  if (zfile) {
    cout << "Writing parameters... : " << crs->decname.c_str() << endl;
    SaveParGz(f_dec,mod);
    gzclose(f_dec);
  }
  else {
    cout << "Can't open file: " << crs->decname.c_str() << endl;
  }

  sprintf(dec_opt,"ab%d",opt.dec_compr);

  DecBuf=DecBuf_ring;
  DecBuf8 = (ULong64_t*) DecBuf;
  idec=0;
  decw_list.clear();

  // mdec1=0;
  // mdec2=0;
  // memset(b_decwrite,0,sizeof(b_decwrite));
} //Reset_Dec
*/

void DecoderClass::Decode79(UInt_t iread, UInt_t ibuf) {
  //Decode79 - the same as 78, but different factor for Area

  /*
  cout << "Decode79: " << zfile << endl;

  //ibuf - current sub-buffer
  Long64_t idx1=b_start[ibuf]; // current index in the buffer (in 1-byte words)

  EventClass* evt=&dummy_event;

  eventlist *Blist;
  UChar_t frmt = GLBuf[idx1+7] & 0x80;
  Dec_Init(Blist,!frmt);
  PulseClass pls=good_pulse;
  PulseClass* ipls=&pls;
  static Long64_t Tst;
  static UChar_t Spn;
  ULong64_t* buf8;

  //prnt("sl;","d79: ",nevents);

  if (opt.fProc) { //fill pulses for reanalysis
    while (idx1<b_end[ibuf]) {
      frmt = GLBuf[idx1+7] & 0x80; //event start bit
      buf8 = (ULong64_t*) (GLBuf+idx1);

      if (frmt) { //event start
	Tst = (*buf8) & sixbytes;
	(*buf8)>>=48;
	//Spn = UChar_t((*buf8) & 1);
	Spn = UChar_t(*buf8);
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	UChar_t* buf1u = (UChar_t*) buf2;
	ipls->Chan = buf2[3];

	if (Spn & 128) { //Counters
	  ipls->Counter = (*buf8) & sixbytes;
	}
	else { //Peaks
	  ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;

	  //new2
	  ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01; //in samples
	  ipls->Tstamp64=Tst;// *opt.Period;

	  ipls->Spin=Spn;
	  ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	  ipls->Height = ((UInt_t) buf1u[7])<<8;

	  ipls->Ecalibr(ipls->Area);
	}
	Event_Insert_Pulse(Blist,ipls);
      }

      idx1+=8;
    } //while (idx1<buf_len)

    Dec_End(Blist,iread,255);

  } //if (opt.fProc)

  else { //!opt.fProc -> fill event
    while (idx1<b_end[ibuf]) {
      frmt = GLBuf[idx1+7] & 0x80; //event start bit
      buf8 = (ULong64_t*) (GLBuf+idx1);

      if (frmt) { //event start	
	evt = &*Blist->insert(Blist->end(),good_event);
	evt->Nevt=nevents;
	nevents++;
	evt->Tstmp = (*buf8) & sixbytes;
	(*buf8)>>=48;
	//evt->Spin |= UChar_t((*buf8) & 1);
	evt->Spin |= UChar_t(*buf8);
	//prnt("ss l ds;",BGRN,"d79:",evt->Tstmp,evt->Spin,RST);
      }
      else {
	Short_t* buf2 = (Short_t*) (GLBuf+idx1);
	UShort_t* buf2u = (UShort_t*) buf2;
	UChar_t* buf1u = (UChar_t*) buf2;
	pulse_vect::iterator itpls =
	  evt->pulses.insert(evt->pulses.end(),pls);
	ipls = &(*itpls);
	ipls->Chan = buf2[3];

	if (evt->Spin & 128) { //Counters
	  ipls->Counter = (*buf8) & sixbytes;
	}
	else { //Peaks
	  ipls->Area = (*buf2u+rnd.Rndm()-1.5)*0.2;
	  ipls->Time = (buf2[1]+rnd.Rndm()-0.5)*0.01
	    + opt.sD[ipls->Chan]/opt.Period; //in samples
	  ipls->Width = (buf2[2]+rnd.Rndm()-0.5)*0.001;

	  ipls->Height = ((UInt_t) buf1u[7])<<8;

	  if (opt.St[ipls->Chan] && ipls->Time < evt->T0) {
	    //prnt("ssd f l f f fs;",KGRN,"pls: ",ipls->Chan,evt->T0,evt->Tstmp,ipls->Time,opt.sD[ipls->Chan],opt.Period,RST);
	    evt->T0=ipls->Time;
	  }
	  ipls->Ecalibr(ipls->Area);
	}
      }

      //prnt("ss l d ls;",BCYN,"d79:",evt->Tstmp,evt->Spin,evt->pulses.size(),RST);

      idx1+=8;
    } //while (idx1<buf_len)

    Dec_End(Blist,iread,254);

  }
  */
} //decode79
