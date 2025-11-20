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
#include <csignal>

using namespace std;

// Потокобезопасная очередь
template<typename T>
class ThreadSafeQueue {
private:
  queue<T> data_queue;
  mutable mutex mtx;
  condition_variable cv;
  atomic<bool> shutdown_flag{false};
  size_t max_size{1000};
    
public:
  ThreadSafeQueue(size_t max_sz = 1000) : max_size(max_sz) {}
    
  bool push(T&& item) {
    if (shutdown_flag) return false;
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this]() { return data_queue.size() < max_size || shutdown_flag; });
    if (shutdown_flag) return false;
    data_queue.push(std::move(item));
    lock.unlock();
    cv.notify_one();
    return true;
  }
    
  bool pop(T& item) {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this]() { return !data_queue.empty() || shutdown_flag; });
    if (shutdown_flag && data_queue.empty()) return false;
    if (data_queue.empty()) return false;
    item = std::move(data_queue.front());
    data_queue.pop();
    lock.unlock();
    cv.notify_one();
    return true;
  }
    
  void shutdown() {
    shutdown_flag = true;
    cv.notify_all();
  }
    
  size_t size() const {
    lock_guard<mutex> lock(mtx);
    return data_queue.size();
  }
    
  bool is_shutdown() const {
    return shutdown_flag;
  }
};

class MultiThreadedDecoder {
private:
  gzFile ff;
  atomic<bool> stop_flag{false};
  atomic<bool> emergency_stop{false};
  ThreadSafeQueue<vector<UChar_t>> buffer_queue;
  ThreadSafeQueue<EventClass> event_queue;
  const size_t BUFFER_SIZE = 4 * 1024 * 1024;
  atomic<int> active_decoders{0};
  atomic<int> max_events{0};
  atomic<int> total_events_stored{0};

    
  // Статический указатель для обработки сигналов
  static MultiThreadedDecoder* instance;
    
public:
  MultiThreadedDecoder(const char* fname) {
    ff = gzopen(fname, "rb");
    if (!ff) {
      cout << "Can't open file: " << fname << endl;
      return;
    }
    
    // Регистрируем обработчик сигналов
    instance = this;
    signal(SIGINT, SignalHandler);
    signal(SIGTERM, SignalHandler);
    
    SkipHeader();
  }
    
  ~MultiThreadedDecoder() {
    if (ff) gzclose(ff);
    instance = nullptr;
  }
    
  void Run(int num_decoder_threads = 2, int max_evt = 0) {
    cout << "Starting multi-threaded decoder with " 
	 << num_decoder_threads << " threads" << endl;
    cout << "Press Ctrl+C to stop processing gracefully" << endl;

    max_events = max_evt;
    if (max_events > 0) {
      cout << "Will process maximum " << max_events << " events" << endl;
    }
    
    thread reader_thread(&MultiThreadedDecoder::ReaderThread, this);
    vector<thread> decoder_threads;
        
    for (int i = 0; i < num_decoder_threads; ++i) {
      decoder_threads.emplace_back(&MultiThreadedDecoder::DecoderThread, this, i);
    }
        
    thread processor_thread(&MultiThreadedDecoder::ProcessorThread, this);
        
    // Ждем завершения reader thread
    reader_thread.join();
    cout << "Reader thread finished" << endl;
        
    // Ждем пока декодеры обработают оставшиеся буферы
    while (active_decoders > 0 && buffer_queue.size() > 0 && !emergency_stop) {
      this_thread::sleep_for(chrono::milliseconds(100));
    }
    
    if (emergency_stop) {
      cout << "EMERGENCY STOP: Shutting down queues..." << endl;
    }
        
    // Останавливаем очереди
    buffer_queue.shutdown();
    event_queue.shutdown();
        
    // Ждем завершения decoder threads
    for (auto& thread : decoder_threads) {
      if (thread.joinable()) thread.join();
    }
    cout << "All decoder threads finished" << endl;
        
    // Ждем завершения processor thread
    processor_thread.join();
    cout << "Processor thread finished" << endl;
    
    if (emergency_stop) {
      cout << "Processing stopped by user request" << endl;
    } else {
      cout << "Processing completed normally" << endl;
    }
  }
  
  // Метод для экстренной остановки
  void EmergencyStop() {
    cout << "\nEmergency stop requested..." << endl;
    emergency_stop = true;
    stop_flag = true;
    buffer_queue.shutdown();
    event_queue.shutdown();
  }
    
private:
  // Обработчик сигналов
  static void SignalHandler(int signal) {
    cout << "\nReceived signal " << signal << endl;
    if (instance) {
      instance->EmergencyStop();
    }
  }
    
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
      cout << "Header not found" << endl;
      exit(-1);
    }

    cout << "Header: " << fmt << " " << mod << " " << sz << endl;

    char* buf = new char[sz];
    gzread(ff, buf, sz);
        
    //F_start test
    Long64_t fstart=0;
    time_t tt=0;
    string str(buf,sz);
    size_t pos=str.find("F_start");
    if (pos!=string::npos) {
      char* buf2 = buf+pos;
      buf2 += strlen(buf2)+1+sizeof(short);
      fstart = *(Long64_t*) buf2;

      char txt[100];
      tt = (fstart+788907600000)*0.001;
      struct tm *ptm = localtime(&tt);
      strftime(txt,sizeof(txt),"%F %T",ptm);

      cout << "F_start: " << " " << txt << " " << fstart << " " << tt << endl;
    }
    else {
      cout << "F_start not found" << endl;
    }

    delete[] buf;
  }
    
  void ReaderThread() {
    cout << "Reader thread started" << endl;
    int read_count = 0;
        
    while (!stop_flag && !emergency_stop) {

      if (buffer_queue.size() > 800) {
	this_thread::sleep_for(chrono::milliseconds(50));
	continue;
      }

      auto buffer = vector<UChar_t>(BUFFER_SIZE);
      int nbytes = gzread(ff, buffer.data(), BUFFER_SIZE);
            
      if (nbytes <= 0) {
	cout << "End of file reached" << endl;
	break;
      }
            
      buffer.resize(nbytes);
      if (!buffer_queue.push(std::move(buffer))) {
        if (emergency_stop) {
          cout << "Reader thread stopped by emergency request" << endl;
        }
	break;
      }
            
      read_count++;
      if (read_count % 10 == 0) {
	cout << "Reader: read " << read_count << " buffers (" 
	     << nbytes << " bytes)" << endl;
      }
    }
        
    cout << "Reader thread finished, total buffers: " << read_count << endl;
  }
    
  void DecoderThread(int thread_id) {
    active_decoders++;
    cout << "Decoder thread " << thread_id << " started" << endl;
    int event_count = 0;
        
    while ((!buffer_queue.is_shutdown() || buffer_queue.size() > 0) && !emergency_stop) {
      vector<UChar_t> buffer;
      if (!buffer_queue.pop(buffer)) break;
            
      BufClass buf;
      buf.buffer_storage = std::move(buffer);
      buf.b1 = buf.buffer_storage.data();
      buf.b3 = buf.b1 + buf.buffer_storage.size();
      buf.u82.b = buf.b1;

      while (buf.u82.b + 7 < buf.b3 && !emergency_stop) {
	EventClass evt;
	if (!Dec79(buf, evt)) break;
                
	if (!event_queue.push(std::move(evt))) break;
	event_count++;
      }
            
      if (event_count % 1000 == 0 && !emergency_stop) {
	cout << "Decoder " << thread_id << ": processed " 
	     << event_count << " events" << endl;
      }
    }
        
    active_decoders--;
    cout << "Decoder thread " << thread_id << " finished, total events: " 
	 << event_count << endl;
  }

  void ProcessorThread() {
    cout << "Processor thread started" << endl;
    list<EventClass> all_events;
    int event_count = 0;
        
    while ((!event_queue.is_shutdown() || event_queue.size() > 0) && 
           !emergency_stop && 
           (max_events == 0 || event_count < max_events)) {
      
      EventClass evt;
      if (!event_queue.pop(evt)) break;
            
      all_events.push_back(std::move(evt));
      event_count++;
      total_events_stored = event_count;  // Обновляем глобальный счетчик
      
      // Если достигли лимита - останавливаем декодеры
      if (max_events > 0 && event_count >= max_events) {
        cout << "Reached event limit of " << max_events << " events. Stopping..." << endl;
        emergency_stop = true;
        buffer_queue.shutdown();
        event_queue.shutdown();
        break;
      }
            
      if (event_count % 5000 == 0 && !emergency_stop) {
        cout << "Processor: stored " << event_count << " events" << endl;
        if (max_events > 0) {
          cout << "Remaining: " << (max_events - event_count) << " events" << endl;
        }
      }
    }
        
    if (!emergency_stop) {
      cout << "=== PROCESSING COMPLETE ===" << endl;
      cout << "Total events processed: " << event_count << endl;
    } else if (max_events > 0 && event_count >= max_events) {
      cout << "=== PROCESSING STOPPED: Event limit reached ===" << endl;
      cout << "Processed " << event_count << " events (limit: " << max_events << ")" << endl;
    } else {
      cout << "Processing interrupted. Events processed: " << event_count << endl;
    }
    
    // Вывод sample events
    int nn = 0;
    for (auto evt = all_events.begin(); evt != all_events.end() && nn < 10; ++evt) {
      cout << "Sample event " << nn << ": Tstmp=" << evt->Tstmp 
	   << ", pulses=" << evt->pulses.size() << endl;
      nn++;
    }
  }
};

// Статическая переменная
MultiThreadedDecoder* MultiThreadedDecoder::instance = nullptr;

// основная функция
void decoder_mt(const char* fname="", int num_threads = 1, int max_events = 0) {
  if (strlen(fname)==0) {
    cout << "usage: .x decoder.C(\"filename\")" << endl;
    cout << "       .x decoder.C(\"filename\", N) for N threads" << endl;
    cout << "       .x decoder.C(\"filename\", N, M) for N threads and M events limit" << endl;
    return;
  }
  
  if (num_threads >= 1) {
    cout << "Using multi-threaded decoder with " << num_threads << " threads" << endl;
    if (max_events > 0) {
      cout << "Event limit: " << max_events << " events" << endl;
    }
    MultiThreadedDecoder decoder(fname);
    decoder.Run(num_threads, max_events);
  }
}
