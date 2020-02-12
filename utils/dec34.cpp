//#include <stdlib.h>
//#include <stdio.h>
#include <iostream>
#include <zlib.h>
#include <vector>
#include <fstream>
#include <sstream>

typedef char           Char_t;      //Signed Character 1 byte (char)
typedef unsigned char  UChar_t;     //Unsigned Character 1 byte (unsigned char)
typedef short          Short_t;     //Signed Short integer 2 bytes (short)
typedef unsigned short UShort_t;    //Unsigned Short integer 2 bytes (unsigned short)
typedef long long          Long64_t; //Portable signed long integer 8 bytes
typedef unsigned long long ULong64_t;//Portable unsigned long integer 8 bytes
typedef double         Double_t;    //Double 8 bytes
typedef int            Int_t;       //Signed integer 4 bytes (int)
typedef unsigned int   UInt_t;      //Unsigned integer 4 bytes (unsigned int)
typedef float          Float_t;     //Float 4 bytes (float)

const ULong64_t sixbytes=0xFFFFFFFFFFFF;
const int MAX_ERR=7; //maximal number of error types;
Long64_t errors[MAX_ERR];
const unsigned char P_BADPULSE=128;


Long64_t npulses;

class pkclass {
public:
  Long64_t Tstamp;
  Double_t EE;
};


std::vector<pkclass> ort;
std::vector<pkclass> alp;

unsigned int iter=0;
int mwhat=0;

using namespace std;

class PulseClass {

 public:
  Long64_t Tstamp64; //64-bit timestamp (corrected for overflows)
  Long64_t Counter; //pulse counter
  std::vector <Float_t> sData; //(maybe smoothed) pulse data
  //std::vector <peak_type> Peaks;
  UChar_t Chan; //channel number
  UChar_t State; //channel state word (Control word - external input in crs32)
  UChar_t ptype; //pulse type: 0 - good pulse; (see P_* constants)
  ofstream *ofs;
public:
  PulseClass();// {};
  virtual ~PulseClass() {};

  void Ana(int what);
  void SavePulse(const char* fname);

  //ClassDef(PulseClass, 0)
};

PulseClass::PulseClass() {
  ptype=0;
  Tstamp64=0;
  State=0;
}

void PulseClass::Ana(int what) {
  switch (what) {
  case 2:
    if (iter>=alp.size()) {
      cout << "iter is too big: " << iter << " " << alp.size() << endl;
      return;
    }
    if (alp[iter].Tstamp==Tstamp64) {
      SavePulse("pulses.dat");
      // cout << "Pulse: " << iter << " " << alp[iter].Tstamp << " "
      // 	 << ort[iter].Tstamp << " " << Tstamp64 << " " << sData.size() << endl;
      ++iter;
    }
    break;
  case 3:
    cout << "Pulse: " << (int) Chan << " " << Counter << " " << Tstamp64 << " " << sData.size() << endl;
    // for (int i=0;i<(int)sData.size();i++) {
    //   printf("-- %d %f\n",i,sData[i]);
    // }
    break;
  default:
    ;
  }
}

void PulseClass::SavePulse(const char* fname) {
  if (iter==0) {//open file
    ofs=new ofstream(fname, std::ofstream::out);
    delete ofs;
    //ofs->close();
  }

  ofs=new ofstream(fname, std::ofstream::out | std::ofstream::app);
  *ofs << iter << " " << sData.size()
       << " " << ort[iter].Tstamp << " " << ort[iter].EE
       << " " << alp[iter].Tstamp << " " << alp[iter].EE
       << endl;
  for (unsigned int i=0;i<sData.size();i++) {
    *ofs << sData[i] << endl;
  }
  delete ofs;
  //ofs->close();

}

void read_da18(const char* fname) {
  char line[2000];
  double par[100];
  ifstream ifs(fname, std::ifstream::in);
  if (!ifs.good()) {
    printf("Can't open file: %s\n",fname);
    exit(0);
  }

  for (int j=0;j<22;j++) {
    ifs.getline(line,1999);
    //cout << j << " " << line << endl;
  }

  int i=0;
  while (!ifs.eof()) {
    ifs.getline(line,1999);
    istringstream ss(line);
    int j=0;
    while (!ss.eof()) {
      ss >> par[j];
      j++;
    }
    
    //cout << "line: " << i << " " << j << " " << par[0] << endl;
    if (j>5) {
      pkclass pk1,pk2;
      pk1.Tstamp = par[2]/5;
      pk1.EE = par[3];
      pk2.Tstamp = par[6]/5;
      pk2.EE = par[7];

      alp.push_back(pk1);
      ort.push_back(pk2);
      i++;
    }
    else {
      cout << "line2: " << i << " " << j << " " << par[0] << endl;
    }
  }
  cout << "sizes: " << ort.size() << " " << alp.size() << endl;

}

PulseClass ipls;

void print_buf8(char* buf, Long64_t size) {
  ULong64_t* buf8 = (ULong64_t*) buf;
  Long64_t idx8=0;

  ULong64_t data;
  unsigned short frmt,ch;

  while (idx8<size/8) {
    frmt = buf[idx8*8+6];
    frmt = (frmt & 0xF0)>>4;
    ch = buf[idx8*8+7];
    data = buf8[idx8] & sixbytes;
    
    printf("%6lld %4d %3d %16lld %20lld\n",idx8,ch,frmt,data,buf8[idx8]);
    //printf("%lld %lld %lld %lld\n",idx8,size,size/8,buf8[idx8]);
    ++idx8;
  }
}

void decode34(char* buf, Long64_t &idx1, Long64_t size) {

  ULong64_t* buf8 = (ULong64_t*) buf;

  unsigned short frmt;
  ULong64_t data;

  Int_t iii; // temporary var. to convert signed nbit var. to signed 32bit int
  int n_frm=0; //counter for frmt4 and frmt5

  /*
  Short_t sss; // temporary var. to convert signed nbit var. to signed 16bit int
  Long64_t lll; //Long temporary var.
  //peak_type *ipk=&dummy_peak; //pointer to the current peak in the current pulse;
  //Double_t QX=0,QY=0,RX,RY;
  Double_t QX=0,RX,AY;
  */

  while (idx1<size) {
    frmt = buf[idx1+6];
    frmt = (frmt & 0xF0)>>4;
    data = buf8[idx1/8] & sixbytes;
    unsigned char ch = buf[idx1+7];

    // Проверка на соответствие каналов, формата и т.п.
    // if (frmt && Blist->empty()) {
    //   ++errors[0];
    //   //cout << "dec34: bad buf start: " << idx1 << " " << (int) ch << " " << frmt << endl;
    //   idx1+=8;
    //   continue;
    // }
    // else if (ch==255) {
    //   //start signal
    //   idx1+=8;
    //   continue;      
    // }
    // else if (frmt<6) {
    //   if (ch>=opt.Nchan) { //bad channel
    // 	//Print_Buf(ibuf,"error.dat");
    // 	//exit(1);

    // 	++errors[1];
    // 	ipls.ptype|=P_BADCH;
    // 	idx1+=8;
    // 	continue;
    //   }
    //   if (frmt && ch!=ipls.Chan) { //channel mismatch
    // 	++errors[2];
    // 	ipls.ptype|=P_BADCH;
    // 	idx1+=8;
    // 	continue;
    //   }
    // }


    switch (frmt) {
    case 0:
      if (buf8[idx1/8]==0) {
	++errors[4];
	idx1+=8;
	continue;
      }
      
      //analyze previous pulse
      if (ipls.ptype==0) {
	//cout << "new pulse: " << npulses << " " << idx1 << endl;
	ipls.Ana(mwhat);
	//exit(1);
	//Event_Insert_Pulse(Blist,&ipls);
      }
      // create new pulse
      ipls=PulseClass();
      npulses++;
      ipls.Chan=ch;
      ipls.Tstamp64=data;
      //cout << "Tstmp64: " << ipls.Tstamp64 << " " << Blist->back().Tstmp << " " << Blist->size() << endl;
      n_frm=0;
      break;
    case 1:
      ipls.State = buf[idx1+5];
      ipls.Counter = data & 0xFFFFFFFFFF;
      break;
    case 2:
      //if (ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
      //ipls.ptype|=P_BADSZ;
      //}
      ////else {
      for (int i=0;i<4;i++) {
	iii = data & 0x7FF;
	ipls.sData.push_back((iii<<21)>>21);
	data>>=12;
      }
      //}
      break;
    case 3:
      //if (ipls.sData.size()>=cpar.durWr[ipls.Chan]) {
      //ipls.ptype|=P_BADSZ;
      //}
      ////else {
      for (int i=0;i<3;i++) {
	iii = data & 0xFFFF;
	ipls.sData.push_back((iii<<16)>>16);
	data>>=16;
      }
      //}
      break;
    case 4:
      /*
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [24]; A – [24]
	  //area
	  iii = data & 0xFFFFFF;
	  ipk->Area0=((iii<<8)>>8);
	  ipk->Area0/=p_len[ipls.Chan];
	  data>>=24;
	  //bkg
	  iii = data & 0xFFFFFF;
	  ipk->Base=((iii<<8)>>8);
	  ipk->Base/=b_len[ipls.Chan];
	  ipk->Area=ipk->Area0 - ipk->Base;
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }
	  break;
	case 1: //H – [12]; QX – [36]
	  lll = data & 0xFFFFFFFFF;
	  QX=((lll<<28)>>28);
	  data>>=36;
	  //height
	  iii = data & 0xFFF;
	  ipk->Height=((iii<<20)>>20);
	  break;
	// case 2: //QY – [36]
	//   lll = data & 0xFFFFFFFFF;
	//   QY=((lll<<28)>>28);
	//   break;
	case 2: //AY – [24]; RX – [20]
	  //RX
	  iii = data & 0xFFFFF;
	  RX=((iii<<12)>>12);
	  data>>=24;
	  //AY (Width)
	  iii = data & 0xFFFFFF;
	  AY=((iii<<8)>>8);

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  ipk->Width=AY/w_len[ipls.Chan]-ipk->Base;
	  ipk->Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Width +
	    opt.E2[ipls.Chan]*ipk->Width*ipk->Width;
	  // if (opt.Bc[ipls.Chan]) {
	  //   ipk->Width+=opt.Bc[ipls.Chan]*ipk->Base;
	  // }
	  ipk->Width/=ipk->Area;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
      */
      break;
    case 5:
      /*
      if (opt.dsp[ipls.Chan]) {
	if (ipls.Peaks.size()==0) {
	  ipls.Peaks.push_back(peak_type());
	  ipk=&ipls.Peaks[0];
	}
	switch (n_frm) {
	case 0: //C – [28]; H – [16]
	  //height
	  sss = data & 0xFFFF;
	  ipk->Height=sss;
	  data>>=16;
	  //bkg
	  iii = data & 0xFFFFFFF;
	  ipk->Base=((iii<<4)>>4);
	  ipk->Base/=b_len[ipls.Chan];
	  break;
	case 1: //A – [28]
	  //area
	  iii = data & 0xFFFFFFF;
	  ipk->Area0=((iii<<4)>>4);
	  ipk->Area0/=p_len[ipls.Chan];

	  ipk->Area=ipk->Area0 - ipk->Base;
	  //ipk->Area*=opt.emult[ipls.Chan];
	  ipk->Area=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Area +
	    opt.E2[ipls.Chan]*ipk->Area*ipk->Area;
	  if (opt.Bc[ipls.Chan]) {
	    ipk->Area+=opt.Bc[ipls.Chan]*ipk->Base;
	  }
	  break;
	case 2: //QX – [40]
	  lll = data & 0xFFFFFFFFFF;
	  QX=((lll<<24)>>24);
	  break;
	case 3: //AY – [28]
	  iii = data & 0xFFFFFFF;
	  AY=((iii<<4)>>4);
	  break;
	case 4: //reserved – [24]; RX – [24]
	  //RX
	  iii = data & 0xFFFFFF;
	  RX=((iii<<8)>>8);
	  data>>=24;

	  if (RX!=0)
	    ipk->Time=QX/RX;
	  else
	    ipk->Time=-999;

	  ipk->Width=AY/w_len[ipls.Chan]-ipk->Base;
	  ipk->Width=opt.E0[ipls.Chan] + opt.E1[ipls.Chan]*ipk->Width +
	    opt.E2[ipls.Chan]*ipk->Width*ipk->Width;
	  // if (opt.Bc[ipls.Chan]) {
	  //   ipk->Width+=opt.Bc[ipls.Chan]*ipk->Base;
	  // }
	  ipk->Width/=ipk->Area;

	  break;
	default:
	  ;
	} //switch
	n_frm++;
      } //if (opt.dsp[ipls.Chan])
      */
      break;
    case 6:
      //npulses3[ch]=data;
      break;
    case 7:
      break;
    default:
      //Print_Buf(ibuf,"error.dat");
      //exit(1);

      ++errors[3];
      //cout << "bad frmt: " << frmt << endl;
    } //switch (frmt);

    //idx8++;
    idx1+=8;
  } //while (idx1<buf_len)

} //decode34

int main(int argc, char **argv)
{
  const int SZ=1024*1024*100; //100Mb - buffer size
  char* buf = new char[SZ];


  //cout << argc << endl;
  if (argc <=2 ) {
    cout << "usage: " << argv[0] << " filename what" << endl;
    cout << "filename must be in raw format 34" << endl;
    cout << "what =" << endl;
    cout << "1 - print buf8+ words" << endl;
    cout << "2 - write file for Telezhnikov" << endl;
    cout << "3 - print pulses" << endl;
    exit(0);
  }
  gzFile zf = gzopen(argv[1],"rb");
  if (!zf) {
    cout << "can't open file: " << argv[1] << endl;
    exit(0);
  }

  std::istringstream ss(argv[2]);
  if (!(ss >> mwhat)) {
    std::cerr << "Invalid number: " << argv[2] << '\n';
  } else if (!ss.eof()) {
    std::cerr << "Trailing characters after number: " << argv[2] << '\n';
  }

  if (mwhat==2) {
    read_da18("da18.txt");
  }
  //exit(0);

  short mod,sz;
  gzread(zf,&mod,sizeof(mod));
  gzread(zf,&sz,sizeof(sz));
  gzread(zf,buf,sz);

  //cout << "mod: " << mod << " " << sz << endl;

  int length;
  int nbuf=0;

  ipls.ptype=P_BADPULSE;
  
  do {
    length=gzread(zf,buf,SZ);
    if (!length)
      break;
    //cout << "length: " << length << endl;
    long long idx1=0;
    if (mwhat==1) {
      print_buf8(buf,length);
    }
    else {
      decode34(buf,idx1,length);
    }
    //exit(1);
    nbuf++;
    //cout << "nbuf: " << nbuf << " " << length << endl;
    //cout << "idx1: " << idx1 << " " << SZ << endl;
  } while (length>0);

  ipls.Ana(mwhat);

  cout << "Total pulses: " << npulses << endl;
  cout << "Good pulses: " << iter << endl;

  delete[] buf;
  gzclose(zf);
  
}
