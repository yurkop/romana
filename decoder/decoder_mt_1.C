// в этих 3 строчках нужно указать путь к папке romana/src
#include "../src/dec.h"
#include "../src/pulseclass.h"
#include "../src/dec.cpp"

#include <iostream>
#include <zlib.h>
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TRandom.h"
#include <bitset>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <vector>

// Потокобезопасная очередь
template<typename T>
class ThreadSafeQueue {
private:
  std::queue<T> queue;
  mutable std::mutex mtx;
  std::condition_variable cv;
  std::atomic<bool> shutdown_flag{false};
  size_t max_size{1000};
    
public:
  ThreadSafeQueue(size_t max_sz = 1000) : max_size(max_sz) {}
    
  bool push(T&& item) {
    if (shutdown_flag) return false;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return queue.size() < max_size || shutdown_flag; });
    if (shutdown_flag) return false;
    queue.push(std::move(item));
    lock.unlock();
    cv.notify_one();
    return true;
  }
    
  bool pop(T& item) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return !queue.empty() || shutdown_flag; });
    if (shutdown_flag && queue.empty()) return false;
    if (queue.empty()) return false;
    item = std::move(queue.front());
    queue.pop();
    lock.unlock();
    cv.notify_one();
    return true;
  }
    
  void shutdown() {
    shutdown_flag = true;
    cv.notify_all();
  }
    
  size_t size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.size();
  }
    
  bool is_shutdown() const {
    return shutdown_flag;
  }
};

class MultiThreadedDecoder {
private:
  gzFile ff;
  std::atomic<bool> stop_flag{false};
  ThreadSafeQueue<std::vector<UChar_t>> buffer_queue;
  ThreadSafeQueue<EventClass> event_queue;
  const size_t BUFFER_SIZE = 4 * 1024 * 1024;
  std::atomic<int> active_decoders{0};
    
public:
  MultiThreadedDecoder(const char* fname) {
    ff = gzopen(fname, "rb");
    if (!ff) {
      std::cout << "Can't open file: " << fname << std::endl;
      return;
    }
    SkipHeader();
  }
    
  ~MultiThreadedDecoder() {
    if (ff) gzclose(ff);
  }
    
  void Run(int num_decoder_threads = 2) {
    std::cout << "Starting multi-threaded decoder with " 
	      << num_decoder_threads << " threads" << std::endl;
        
    std::thread reader_thread(&MultiThreadedDecoder::ReaderThread, this);
    std::vector<std::thread> decoder_threads;
        
    for (int i = 0; i < num_decoder_threads; ++i) {
      decoder_threads.emplace_back(&MultiThreadedDecoder::DecoderThread, this, i);
    }
        
    std::thread processor_thread(&MultiThreadedDecoder::ProcessorThread, this);
        
    reader_thread.join();
    std::cout << "Reader thread finished" << std::endl;
        
    while (active_decoders > 0 && buffer_queue.size() > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
        
    buffer_queue.shutdown();
    event_queue.shutdown();
        
    for (auto& thread : decoder_threads) {
      if (thread.joinable()) thread.join();
    }
    std::cout << "All decoder threads finished" << std::endl;
        
    processor_thread.join();
    std::cout << "Processor thread finished" << std::endl;
  }
    
private:
  void SkipHeader() {
    UShort_t fmt, mod;
    Int_t sz;

    gzread(ff, &fmt, sizeof(Short_t));
    if (fmt == 129) {
      gzread(ff, &mod, sizeof(Short_t));
      gzread(ff, &sz, sizeof(Int_t));
    } else {
      mod = fmt;
      fmt = 129;
      gzread(ff, &sz, sizeof(UShort_t));
    }

    if (fmt != 129 || mod > 100 || sz > 5e5) {
      std::cout << "Header not found" << std::endl;
      exit(-1);
    }

    std::cout << "Header: " << fmt << " " << mod << " " << sz << std::endl;

    char* buf = new char[sz];
    gzread(ff, buf, sz);
        
    //F_start test
    Long64_t fstart=0;
    time_t tt=0;
    std::string str(buf,sz);
    size_t pos=str.find("F_start");
    if (pos!=std::string::npos) {
      char* buf2 = buf+pos;
      buf2 += strlen(buf2)+1+sizeof(short);
      fstart = *(Long64_t*) buf2;

      char txt[100];
      tt = (fstart+788907600000)*0.001;
      struct tm *ptm = localtime(&tt);
      strftime(txt,sizeof(txt),"%F %T",ptm);

      std::cout << "F_start: " << " " << txt << " " << fstart << " " << tt << std::endl;
    }
    else {
      std::cout << "F_start not found" << std::endl;
    }

    delete[] buf;
  }
    
  void ReaderThread() {
    std::cout << "Reader thread started" << std::endl;
    int read_count = 0;
        
    while (!stop_flag) {
      auto buffer = std::vector<UChar_t>(BUFFER_SIZE);
      int nbytes = gzread(ff, buffer.data(), BUFFER_SIZE);
            
      if (nbytes <= 0) {
	std::cout << "End of file reached" << std::endl;
	break;
      }
            
      buffer.resize(nbytes);
      if (!buffer_queue.push(std::move(buffer))) {
	break;
      }
            
      read_count++;
      if (read_count % 10 == 0) {
	std::cout << "Reader: read " << read_count << " buffers (" 
		  << nbytes << " bytes)" << std::endl;
      }
    }
        
    std::cout << "Reader thread finished, total buffers: " << read_count << std::endl;
  }
    
  void DecoderThread(int thread_id) {
    active_decoders++;
    std::cout << "Decoder thread " << thread_id << " started" << std::endl;
    int event_count = 0;
        
    while (!buffer_queue.is_shutdown() || buffer_queue.size() > 0) {
      std::vector<UChar_t> buffer;
      if (!buffer_queue.pop(buffer)) break;
            
      BufClass buf;
      buf.buffer_storage = std::move(buffer);
      buf.b1 = buf.buffer_storage.data();
      buf.b3 = buf.b1 + buf.buffer_storage.size();
      buf.u82.b = buf.b1;
            
      while (buf.u82.b + 7 < buf.b3) {
	EventClass evt;
	if (!Dec79(buf, evt)) break;
                
	if (!event_queue.push(std::move(evt))) break;
	event_count++;
      }
            
      if (event_count % 1000 == 0) {
	std::cout << "Decoder " << thread_id << ": processed " 
		  << event_count << " events" << std::endl;
      }
    }
        
    active_decoders--;
    std::cout << "Decoder thread " << thread_id << " finished, total events: " 
	      << event_count << std::endl;
  }
    
  void ProcessorThread() {
    std::cout << "Processor thread started" << std::endl;
    std::list<EventClass> all_events;
    int event_count = 0;
        
    while (!event_queue.is_shutdown() || event_queue.size() > 0) {
      EventClass evt;
      if (!event_queue.pop(evt)) break;
            
      all_events.push_back(std::move(evt));
      event_count++;
            
      if (event_count % 5000 == 0) {
	std::cout << "Processor: stored " << event_count << " events" << std::endl;
      }
    }
        
    std::cout << "=== PROCESSING COMPLETE ===" << std::endl;
    std::cout << "Total events processed: " << event_count << std::endl;
        
    int nn = 0;
    for (auto evt = all_events.begin(); evt != all_events.end() && nn < 10; ++evt) {
      std::cout << "Sample event " << nn << ": Tstmp=" << evt->Tstmp 
		<< ", pulses=" << evt->pulses.size() << std::endl;
      nn++;
    }
  }
};

// ОСНОВНАЯ ФУНКЦИЯ С МНОГОПОТОЧНОСТЬЮ КАК ОПЦИЯ
void decoder_mt(const char* fname="", int num_threads = 1) {
  if (strlen(fname)==0) {
    std::cout << "usage: .x decoder.C(\"filename\")" << std::endl;
    std::cout << "       .x decoder.C(\"filename\", N) for N threads (N>1 for multi-threaded)" << std::endl;
    return;
  }
  
  if (num_threads >= 1) {
    std::cout << "Using multi-threaded decoder with " << num_threads << " threads" << std::endl;
    MultiThreadedDecoder decoder(fname);
    decoder.Run(num_threads);
  }
}
//***************************************************************
