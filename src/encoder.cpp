#include "romana.h"
#include <TSystem.h>
#include <filesystem>
#include <condition_variable>

extern Toptions opt;
extern Coptions cpar;
extern CRS* crs;

Encoder::Encoder() {
}

Encoder::~Encoder() {
}

void Encoder::Encode_Start(int rst, int mm, bool &bb, int cc, UChar_t* BBuf,
			   Long64_t r_size, Long64_t o_size) {
  // rst=rst; mm - module; bb - write flag; cc - compression
  // BBuf: если не 0, задает Buf_ring; если 0, Buf_ring создается
  // r_size - total size of buffer (на самом деле + o_size)
  // o_size - offset size

  w_module=mm;
  b_wrt=&bb;
  w_compr=cc;

  if (rst) {
    if (*b_wrt) {
      Reset_Wrt();

      // Новый код с безопасным управлением памятью:
      if (BBuf) {
	Buf_ring2.b1 = BBuf;
      }
      else {
	buffer_storage.resize(r_size + o_size);
	Buf_ring2.b1 = buffer_storage.data();
      }
      Buf_ring2.b3 = Buf_ring2.b1 + r_size + o_size;

      Buf_ring.b1 = Buf_ring2.b1;
      Buf_ring.b3 = Buf_ring.b1 + r_size;

      buf_list.clear();
      buf_it = buf_list.insert(buf_list.end(),BufClass());
      buf_it->b1 = Buf_ring.b1;
      buf_it->b = buf_it->b1;
      buf_it->b3 = buf_it->b1+buf_size;
      //buf_it->flag=??;  // считаем, что сделаны Findlast + анализ


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

    if (*b_wrt) {
      trd_write = std::make_unique<std::thread>(&Encoder::Handle_write, this);
    }

    // if (opt.raw_write) {
    //   trd_raw_write = new TThread("trd_raw_write", handle_raw_write, (void*) 0);
    //   trd_raw_write->Run();
    // }

    //gSystem->Sleep(100);
  }
}

void Encoder::Encode_Stop(int end_ana, bool opt_wrt) {
  if (end_ana) {
    if (opt_wrt) {
      Flush3(end_ana);
    }
  }

  wrt_thread_run = 0;
  cv_buf.notify_one();  // Будим поток для завершения
  
  if (trd_write && trd_write->joinable()) {
    trd_write->join();
    trd_write.reset();
  }
}

void Encoder::Reset_Wrt() {
  sprintf(wr_opt,"wb%d",w_compr);

  // if (cpar.Trigger==1) //trigger on START channel
  //   mod=80;

  gzf = gzopen(wr_name.c_str(),wr_opt);
  if (gzf) {
    cout << "Writing parameters... : " << wr_name.c_str() << endl;
    crs->SaveParGz(gzf,w_module);
    gzclose(gzf);
  }
  else {
    TString msg=TString("Can't open file: ") + wr_name.c_str();
    EError(1,1,1,msg);
    *b_wrt=false;
  }

  sprintf(wr_opt,"ab%d",w_compr);

  wtime_prev = wtime_1 = wtime_0 = gSystem->Now();
  //wr_bytes_prev=0;
  size_prev=0;
  wrate_mean=0;

  // mdec1=0;
  // mdec2=0;
  // memset(b_decwrite,0,sizeof(b_decwrite));
} //Reset_Wrt

void Encoder::Flush3(int end_ana) {
  //сбрасывает заполненный буфер на диск

  if (opt.nthreads==1) { //single thread
    Write3(buf_it->b1, buf_it->b-buf_it->b1);
    buf_it->b1 = Buf_ring.b1;
    buf_it->b = Buf_ring.b1;
    buf_it->b3 = Buf_ring.b1+buf_size;
  }
  else { //multithreading
    std::unique_lock<std::mutex> wr_lock(wr_mut);

    //печатает на экран информацию о каждом 10-м буфере
    static int rep=0;
    rep++;
    if (rep>9) {
    // if (rep>p) {
      double rr = Buf_ring.b3-Buf_ring.b1;
      rr/=(buf_it->b-buf_it->b1);
      prnt("s d x d l l l f;","Flush3: ",buf_list.size(),buf_it->b1,
	   (buf_it->b-buf_it->b1)/1024,(buf_it->b1-Buf_ring.b1)/1024,
	   (Buf_ring.b3-buf_it->b1)/1024,(Buf_ring.b3-Buf_ring.b1)/1024,rr);
      rep=0;
    }

    auto prev = buf_it;

    if (!end_ana) {
      // если не конец -> задаем новый буфер
      buf_it = buf_list.insert(buf_list.end(),BufClass());
      if (prev->b < Buf_ring.b3) {
	buf_it->b1 = prev->b;
      }
      else {
	prnt("ss d x xs;",BYEL,"---end of Buf---: ",buf_it->bufnum,
	     prev->b,Buf_ring.b3,RST);
	buf_it->b1 = Buf_ring.b1;
      }
      buf_it->b = buf_it->b1;
      buf_it->b3 = buf_it->b1+buf_size;

      buf_it->bufnum = prev->bufnum+1;
      // bufnum всегда "линейный", независимо от многопоточности.
      // анализ событий идет всегда последовательно,
      // соответственно, и вызов Flush3 тоже последовательный
    }

    // устанавливаем флаг в старом буфере: можно писать
    prev->flag=9;
    
    // Разблокируем в правильном порядке
    wr_lock.unlock();
    cv_buf.notify_one();
  }

} //Flush3

int Encoder::Write3(UChar_t* buf, int len) {
  //return >0 if error
  //       0 - OK
  int rr=0;
  TString msg;

  if (len) { //пишем непустой буфер

    //sprintf(wr_opt,"ab%d",opt.dec_compr);
    gzf = gzopen(wr_name.c_str(),wr_opt);
    if (!gzf) {
      msg=TString("Can't open file: ") + wr_name.c_str();
      EError(1,1,1,msg);
      *b_wrt=false;
      //idec=0;
      rr=1;
    }

    int res=gzwrite(gzf,buf,len);
    if (res!=len) {
      msg=TString("Error writing to file: ")+wr_name.c_str()+" "+res+" "+len;
      EError(1,1,1,msg);
      wr_bytes+=res;
      *b_wrt=false;
      //gzclose(gzf);
      rr=2;
    }
    wr_bytes+=res;

    gzclose(gzf);
    gzf=0;
  } //if len

  Long64_t tt = gSystem->Now();
  double dt = (tt-wtime_prev)*0.001;
  Long64_t size=0;

  if (opt.wdog_timer>0 && dt>60) { //60 seconds
    try {
      std::filesystem::path file_path = wr_name.c_str();
      size = std::filesystem::file_size(file_path);
      //std::cout << "Размер: " << size << " байт" << std::endl;
    } catch (const std::filesystem::filesystem_error& ex) {
      msg=TString("Can't access file: ")+wr_name.c_str() + " " + ex.what();
      EError(1,1,1,msg);
    }

    double rate = (size-size_prev)/dt;
    double mdt = (tt-wtime_0)*0.001; //прошло с начала
    double dt1 = (tt-wtime_1)*0.001; //прошло с прошлого срабатывания wdog
    wrate_mean = size/mdt;
    wtime_prev=tt;
    size_prev = size;

    if (dt1>opt.wdog_timer) {
      //время с прошлого срабатывания wdog-a > opt.wdog_timer
      double dd = 100;
      if (wrate_mean>0)
	dd = rate/wrate_mean*100;

      if (dd<opt.wdog1 || dd>opt.wdog2) { //alert!
	wtime_1=tt;
	char msg[200];
	sprintf(msg,"File %s growth: %0.1f%% of average:"
		" %0.1f %0.1f",wr_name.c_str(),dd,rate,wrate_mean);
	EError(1,1,0,TString(msg));
      }
    }

    // cout << "Write3: " << len << " " << tt << " " << dt << " " << wr_bytes
    // 	 << " " << size << " " << rate << " " << wrate_mean << endl;
  }

  return rr;
} //Write3

void Encoder::Handle_write() {
  {
    std::lock_guard<std::mutex> lock(cmut);
    cout << "write thread started: " << endl;
  }

  while (wrt_thread_run.load() || !buf_list.empty()) {
    std::unique_lock<std::mutex> lock(wr_mut);
    
    cv_buf.wait(lock, [this]() {
      return !buf_list.empty() || !wrt_thread_run.load();
    });
    
    if (!buf_list.empty()) {
      for (auto it = buf_list.begin(); it != buf_list.end(); ++it) {
 	// проверка bufnum не нужна,
	// т.к. buf_list заполняется всегда последовательно
	if (it->flag == 9) {
	  lock.unlock(); // Сначала РАЗБЛОКИРОВАТЬ перед долгой операцией
          Write3(it->b1, it->b - it->b1);
 	  //gSystem->Sleep(50); // для искусственного замедления (тест)
          lock.lock();
          buf_list.erase(it);
	  break;
        }
      }
    }
    
    lock.unlock();
    
    //std::this_thread::sleep_for(std::chrono::milliseconds(1)); //ИЗБЫТОЧНО
  }

  {
    std::lock_guard<std::mutex> lock(cmut);
    cout << "write thread stopped: " << endl;
  }
}

//------------------------
EDec::EDec() {
  buf_size=Megabyte;
}

void EDec::Fill_Dec(event_iter evt) {
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

void EDec::Fill_Dec79(event_iter evt) {
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

  *buf_it->ul = 0x8000 | evt->Spin;
  *buf_it->ul <<= 48;
  *buf_it->ul |= evt->Tstmp & sixbytes;

  ++buf_it->ul;

  if (evt->Spin & 128) { //Counters
    //prnt("ss ls;",BRED,"Counter:",evt->Tstmp,RST);
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      //prnt("ss d d ls;",BGRN,"Ch:",ipls->Chan,ipls->Pos,ipls->Counter,RST);
      *buf_it->ul=ipls->Counter;
      //Short_t* Decbuf2 = (Short_t*) DecBuf8;
      buf_it->s[3] = ipls->Chan;
      ++buf_it->ul;
      //++DecBuf8;
    }
  }
  else { //Peaks
    for (pulse_vect::iterator ipls=evt->pulses.begin();
	 ipls!=evt->pulses.end();++ipls) {
      if (ipls->Pos>-32222) {
	*buf_it->ul=0;
	//Short_t* Decbuf2 = (Short_t*) DecBuf8;
	//UShort_t* Decbuf2u = (UShort_t*) Decbuf2;
	//UChar_t* Decbuf1u = (UChar_t*) DecBuf8;
	if (ipls->Area<0) {
	  *buf_it->us = 0;
	}
	else if (ipls->Area>13106){
	  *buf_it->us = 65535;
	}
	else {
	  *buf_it->us = ipls->Area*5+1;
	}
	if (ipls->Time>327.6)
	  buf_it->s[1] = 32767;
	else if (ipls->Time<-327)
	  buf_it->s[1] = -32767;
	else
	  buf_it->s[1] = ipls->Time*100;

	if (ipls->Width>32.76)
	  buf_it->s[2] = 32767;
	else if (ipls->Width<-32.76)
	  buf_it->s[2] = -32767;
	else
	  buf_it->s[2] = ipls->Width*1000;

	buf_it->s[3] = ipls->Chan;
	if (ipls->Height<0)
	  buf_it->b[7] = 0;
	else
	  buf_it->b[7] = ((int)ipls->Height)>>8;
	++buf_it->ul;
	//++DecBuf8;
      }
    }
  }

  //idec = (UChar_t*)DecBuf8-DecBuf;
  //if (idec>buf_size) {
  if (buf_it->b >= buf_it->b3) {
    Flush3(0);
  }

} //Fill_Dec79

//------------------------
ERaw::ERaw() {
  cout << "перенести Fill_Raw в ERaw (search Fill_Raw in libcrs.cpp)" << endl;
  buf_size=2*Megabyte;
}
