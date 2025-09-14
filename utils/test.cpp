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
