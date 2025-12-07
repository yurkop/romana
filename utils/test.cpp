#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class BufferWriter {
private:
  std::vector<int> buffer;
  std::mutex mtx;
  std::condition_variable cv;
  bool buffer_ready = false;
  bool stop_requested = false;
  const size_t buffer_size = 1000;
  std::ofstream file;

public:
  BufferWriter(const std::string &filename) : file(filename) {
    buffer.reserve(buffer_size);
  }

  // Поток для добавления данных в буфер
  void producerThread() {
    for (int i = 0; i < 10000; ++i) {
      {
        std::unique_lock<std::mutex> lock(mtx);
        buffer.push_back(i);

        if (buffer.size() >= buffer_size) {
          buffer_ready = true;
          cv.notify_one(); // Уведомляем потребителя
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Завершаем работу
    {
      std::unique_lock<std::mutex> lock(mtx);
      stop_requested = true;
      cv.notify_one();
    }
  }

  // Поток для записи в файл
  void consumerThread() {
    while (true) {
      std::vector<int> local_buffer;

      {
        std::unique_lock<std::mutex> lock(mtx);
        // Ждем, пока буфер не заполнится или не поступит команда остановки
        cv.wait(lock, [this]() { return buffer_ready || stop_requested; });

        if (stop_requested && !buffer_ready) {
          break;
        }

        // Копируем данные для записи
        local_buffer = std::move(buffer);
        buffer.clear();
        buffer_ready = false;
      }

      // Записываем в файл (вне критической секции)
      writeToFile(local_buffer);
    }

    // Записываем оставшиеся данные при остановке
    if (!buffer.empty()) {
      writeToFile(buffer);
    }
  }

  void writeToFile(const std::vector<int> &data) {
    for (const auto &item : data) {
      file << item << "\n";
    }
    file.flush();
  }

  void run() {
    std::thread producer(&BufferWriter::producerThread, this);
    std::thread consumer(&BufferWriter::consumerThread, this);

    producer.join();
    consumer.join();

    file.close();
  }
};

int main() {
  BufferWriter writer("output.txt");
  writer.run();
  return 0;
}

/*

#include <list>

typedef unsigned char UChar_t;
typedef long long Long64_t;

class BufClass {
public:
  UChar_t* b1=0; // указатель на начало буфера
  UChar_t* b2=0; // указатель на конец "полных" событий в буфере
  UChar_t* b3=0; // указатель на физический конец буфера
  // всегда должно быть: b1 <= b2 < b3

  //size_t Size=0;
  Long64_t bufid=0; // номер буфера (=идентификатор)
  UChar_t flag=0;
  //0 - empty, can be filled;
  //1 - filled, can be analyzed;
  //2 - analyzed, can be deleted.


  // public:
  //   BufClass(size_t sz);
  //   ~BufClass();
};

std::list<BufClass> dec_list; // выходные данные (dec)

int main(int argc, char **argv)
{
  BufClass dbuf=dec_list.front();
  dec_list.pop_front();

}
*/
