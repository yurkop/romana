#include "romana.h"
#include <TSystem.h>


extern Toptions opt;
extern Coptions cpar;
extern CRS* crs;

Encoder::Encoder() {
}

Encoder::~Encoder() {
  if (DecBuf_ring2.b1)
    delete[] DecBuf_ring2.b1;
}

void Encoder::Encode_Start(int rst) {

  if (rst) {
    // if (opt.raw_write) {
    //   crs->Reset_Raw();
    // }   
    if (opt.dec_write) {
      Reset_Dec(opt.dec_format);
    }
    // if (opt.fTxt) {
    //   crs->Reset_Txt();
    // }
  }





  if (opt.nthreads>1) {
    //decode_thread_run=1;
    //mkev_thread_run=1;
    //ana_thread_run=1;
    wrt_thread_run=1;

    //ana_all=0;

    if (opt.dec_write) {
      ThreadArgs* args = new ThreadArgs(this, new int(0));
      trd_dec_write = new TThread("trd_dec_write", StaticWrapper, args);
      trd_dec_write->Run();
    }

    // if (opt.raw_write) {
    //   trd_raw_write = new TThread("trd_raw_write", handle_raw_write, (void*) 0);
    //   trd_raw_write->Run();
    // }

    //gSystem->Sleep(100);
  }


}

void Encoder::Encode_Stop(int end_ana) {

  if (end_ana) {
    if (opt.dec_write) {
      Flush_Dec3(end_ana);
    }
  }


  wrt_thread_run=0;
  if (trd_dec_write) {
    trd_dec_write->Join();
    trd_dec_write->Delete();
    trd_dec_write=0;
  }
}

void Encoder::Reset_Dec(Short_t mod) {
  sprintf(dec_opt,"wb%d",opt.dec_compr);

  // if (cpar.Trigger==1) //trigger on START channel
  //   mod=80;

  f_dec = gzopen(decname.c_str(),dec_opt);
  if (f_dec) {
    cout << "Writing parameters... : " << decname.c_str() << endl;
    crs->SaveParGz(f_dec,mod);
    gzclose(f_dec);
  }
  else {
    cout << "Can't open file: " << decname.c_str() << endl;
  }

  sprintf(dec_opt,"ab%d",opt.dec_compr);

  if (DecBuf_ring2.b1)
    delete[] DecBuf_ring2.b1;
  size_t sz = opt.decbuf_size*DEC_SIZE;
  DecBuf_ring2.b1 = new UChar_t[sz+OFF_SIZE];
  DecBuf_ring2.b3 = DecBuf_ring2.b1 + sz + OFF_SIZE;

  DecBuf_ring.b1 = DecBuf_ring2.b1;
  DecBuf_ring.b3 = DecBuf_ring2.b3 - OFF_SIZE;

  dec_list.clear();
  decbuf_it = dec_list.insert(dec_list.end(),BufClass());
  decbuf_it->b1 = DecBuf_ring.b1;
  decbuf_it->b = decbuf_it->b1;
  decbuf_it->b3 = decbuf_it->b1+DEC_SIZE;
  //decbuf_it->flag=??;  // считаем, что сделаны Findlast + анализ


  // mdec1=0;
  // mdec2=0;
  // memset(b_decwrite,0,sizeof(b_decwrite));
} //Reset_Dec

void Encoder::Fill_Dec(event_iter evt) {

  switch (opt.dec_format) {
  case 79:
  case 80:
    if (cpar.Trigger==1) { //trigger on START channel
      //Fill_Dec80(evt);
    }
    else {
      Fill_Dec79(evt);
    }
    break;
  case 81:
    //Fill_Dec81(evt);
    break;
  case 82:
    //Fill_Dec82(evt);
    break;
  default:
    ;
  } //switch

}

void Encoder::Fill_Dec79(event_iter evt) {
  //Fill_Dec79 - the same as 78, but different factor for Area

  // fill_dec is not thread safe!!!
  //format of decoded data:
  // 1) one 8byte header word:
  //    bit63=1 - start of event
  //    lowest 6 bytes - Tstamp
  //    byte 7 - Spin
  // 2a) if Spin7(bit7):
  //  N 8-byte words, each containing one pulse counter
  //    bytes 0-5 - Counter
  //    byte 7 - channel
  // 2b) else (!Spin7(bit7)):
  //  N 8-byte words, each containing one peak
  //    1st (lowest) 2 bytes - (unsigned) Area*5+1
  //    2 bytes - Time*100
  //    2 bytes - Width*1000
  //    1 byte - channel
  //    7 bits - amplitude

  *decbuf_it->ul = 0x8000 | evt->Spin;
  *decbuf_it->ul <<= 48;
  *decbuf_it->ul |= evt->Tstmp & sixbytes;

  ++decbuf_it->ul;

  if (evt->Spin & 128) { //Counters
    //prnt("ss ls;",BRED,"Counter:",evt->Tstmp,RST);
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      //prnt("ss d d ls;",BGRN,"Ch:",ipls->Chan,ipls->Pos,ipls->Counter,RST);
      *decbuf_it->ul=ipls->Counter;
      //Short_t* Decbuf2 = (Short_t*) DecBuf8;
      decbuf_it->s[3] = ipls->Chan;
      ++decbuf_it->ul;
      //++DecBuf8;
    }
  }
  else { //Peaks
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      if (ipls->Pos>-32222) {
	*decbuf_it->ul=0;
	//Short_t* Decbuf2 = (Short_t*) DecBuf8;
	//UShort_t* Decbuf2u = (UShort_t*) Decbuf2;
	//UChar_t* Decbuf1u = (UChar_t*) DecBuf8;
	if (ipls->Area<0) {
	  *decbuf_it->us = 0;
	}
	else if (ipls->Area>13106){
	  *decbuf_it->us = 65535;
	}
	else {
	  *decbuf_it->us = ipls->Area*5+1;
	}
	if (ipls->Time>327.6)
	  decbuf_it->s[1] = 32767;
	else if (ipls->Time<-327)
	  decbuf_it->s[1] = -32767;
	else
	  decbuf_it->s[1] = ipls->Time*100;

	if (ipls->Width>32.76)
	  decbuf_it->s[2] = 32767;
	else if (ipls->Width<-32.76)
	  decbuf_it->s[2] = -32767;
	else
	  decbuf_it->s[2] = ipls->Width*1000;

	decbuf_it->s[3] = ipls->Chan;
	if (ipls->Height<0)
	  decbuf_it->b[7] = 0;
	else
	  decbuf_it->b[7] = ((int)ipls->Height)>>8;
	++decbuf_it->ul;
	//++DecBuf8;
      }
    }
  }

  //idec = (UChar_t*)DecBuf8-DecBuf;
  //if (idec>DEC_SIZE) {
  if (decbuf_it->b >= decbuf_it->b3) {
    Flush_Dec3(0);
  }

} //Fill_Dec79

void Encoder::Flush_Dec3(int end_ana) {
  if (opt.nthreads==1) { //single thread
    Wr_Dec(decbuf_it->b1, decbuf_it->b-decbuf_it->b1);
    decbuf_it->b1 = DecBuf_ring.b1;
    decbuf_it->b = DecBuf_ring.b1;
    decbuf_it->b3 = DecBuf_ring.b1+DEC_SIZE;
  }
  else { //multithreading
    decw_mut.Lock();

    //печатает на экран информацию о каждом 10-м буфере
    static int rep=0;
    rep++;
    if (rep>9) {
    // if (rep>p) {
      double rr = DecBuf_ring.b3-DecBuf_ring.b1;
      rr/=(decbuf_it->b-decbuf_it->b1);
      prnt("s d x d l l l f;","Flush_dec3: ",dec_list.size(),decbuf_it->b1,
	   decbuf_it->b-decbuf_it->b1,decbuf_it->b1-DecBuf_ring.b1,
	   DecBuf_ring.b3-decbuf_it->b1,DecBuf_ring.b3-DecBuf_ring.b1,rr);
      rep=0;
    }

    auto prev = decbuf_it;

    if (!end_ana) {
      // если не конец -> задаем новый буфер
      decbuf_it = dec_list.insert(dec_list.end(),BufClass());
      if (prev->b < DecBuf_ring.b3) {
	decbuf_it->b1 = prev->b;
      }
      else {
	prnt("sss;",BYEL,"---end of DecBuf---: ",RST);
	decbuf_it->b1 = DecBuf_ring.b1;
      }
      decbuf_it->b = decbuf_it->b1;
      decbuf_it->b3 = decbuf_it->b1+DEC_SIZE;

      decbuf_it->bufnum = prev->bufnum+1;
      // bufnum всегда "линейный", независимо от многопоточности.
      // анализ событий идет всегда последовательно,
      // соответственно, и вызов Flush_Dec3 тоже последовательный
    }

    // устанавливаем флаг в старом буфере: можно писать
    prev->flag=9;

    decw_mut.UnLock();
  }

} //Flush_Dec3

int Encoder::Wr_Dec(UChar_t* buf, int len) {
  //return >0 if error
  //       0 - OK

  if (!len) //не пишем пустой буфер
    return 0;

  //sprintf(dec_opt,"ab%d",opt.dec_compr);
  f_dec = gzopen(decname.c_str(),dec_opt);
  if (!f_dec) {
    cout << "Can't open file: " << decname.c_str() << endl;
    f_dec=0;
    opt.dec_write=false;
    //idec=0;
    return 1;
  }

  int res=gzwrite(f_dec,buf,len);
  if (res!=len) {
    cout << "Error writing to file: " << decname.c_str() << " " 
	 << res << " " << len << endl;
    decbytes+=res;
    opt.dec_write=false;
    gzclose(f_dec);
    f_dec=0;
    return 2;
  }
  decbytes+=res;

  gzclose(f_dec);
  f_dec=0;
  return 0;

} //Wr_Dec

void Encoder::Handle_dec_write(void *ctx) {

  cmut.Lock();
  cout << "dec_write thread started: " << endl;
  cmut.UnLock();

  while (wrt_thread_run || !dec_list.empty()) {

    //prnt("ss ls;",BGRN,"decwr:",dec_list.size(),RST);

    if (!dec_list.empty()) { //пишем

      for (auto it=dec_list.begin();it!=dec_list.end();++it) {
	// проверка bufnum не нужна,
	// т.к. dec_list заполняется всегда последовательно
	//bool good_buf = it->flag==9 && it->bufnum==DecBuf_ring.bufnum;
	if (it->flag==9) {
	  //gSystem->Sleep(50); // для искусственного замедления (тест)
	  Wr_Dec(it->b1,it->b-it->b1);
	  decw_mut.Lock();
	  dec_list.erase(it);
	  decw_mut.UnLock();
	  break;
	}
      }

    } //if (!decw_list.empty())
    // else {
    //   gSystem->Sleep(5);
    // }
    //prnt("ss ls;",BRED,"decwr:",dec_list.size(),RST);

    gSystem->Sleep(55);

  } //while (wrt_thread_run)

  cmut.Lock();
  cout << "dec_write thread stopped: " << endl;
  cmut.UnLock();

  //return NULL;
} //handle_dec_write

// Статическая обертка
void* Encoder::StaticWrapper(void* arg) {
  ThreadArgs* args = static_cast<ThreadArgs*>(arg);
  args->instance->Handle_dec_write(args->user_arg);
  delete static_cast<int*>(args->user_arg); // чистим аргументы
  delete args; // чистим структуру
  return nullptr;
}
