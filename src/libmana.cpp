//#define LONGSTAMP 1

#ifndef LINUX
#include <direct.h>
#endif

//#include "colors.h"
#include <signal.h>
#include <iomanip>
//#include <malloc.h>

#include "romana.h"
#include "popframe.h"
//#include "peditor.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>

//#include <TTree.h>
#include <TClass.h>
#include <TH1.h>
//#include <TH2.h>
#include <TApplication.h>
#include <TFile.h>
#include <TKey.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TColor.h>
#include <TBrowser.h>

#include <TPolyMarker.h>
#include <TF1.h>
#include <TText.h>
#include <TString.h>

#include <TGResourcePool.h>
#include <TGDockableFrame.h>

#include <TDataMember.h>
#include <TDataType.h>
#include "TThread.h"
#include "TImage.h"
#include "TMutex.h"

//#include <TGColorDialog.h>

#include "libcrs.h"

const double MB = 1024*1024;

MemInfo_t minfo;
ProcInfo_t pinfo;
// double rmem;

std::list<VarClass> varlist;
//typedef std::list<VarClass>::iterator v_iter;

extern TMutex cmut;

GlbClass* glb;

Coptions cpar;
Toptions opt;

CRS* crs;

MyMainFrame* myM=0;
EventFrame* EvtFrm=0;
HistFrame* HiFrm=0;
ErrFrame* ErrFrm=0;
HClass* hcl=0;

ParParDlg *parpar=0;
HistParDlg *histpar=0;
ChanParDlg *chanpar=0;

Pixel_t fWhite;
Pixel_t fGrey;
Pixel_t fYellow;
Pixel_t fGreen;
Pixel_t fGreen2;
Pixel_t fRed;
Pixel_t fCyan;
Pixel_t fMagenta;
Pixel_t fOrng;
Pixel_t fBlue;
Pixel_t fRed10;

Pixel_t fCol[7];// = {fYellow,fGreen,fRed,fRed10,fCyan,fOrng,fBlue};

//const int maxsamp = 16500;// константу 16500 надо будет заменить на переменную

gzFile ff;

//TString parfile,lastpar;
char* parfile=(char*)"romana.par";;
char* parfile2=0; //for -p option
char* datfname=0;
char* batch_fname=0;

bool b_raw=false,b_dec=false,b_root=false,b_force=false;
bool rd_root=false; //readroot from comand line
bool b_resetusb=false;

char startdir[200];
char pr_name[200];
char maintitle[200];
char mainname[200];
//char rootname[200]="";

struct stat statb;
const char* msg_exists = "Output file already exists.\nPress OK to overwrite it";
const char* msg_ident = "Input and output files are identical";

std::list<TString> listpar;
//std::list<TString> listshow;
typedef std::list<TString>::iterator l_iter;

std::vector<Mdef> list_mdef;
//TList listmap2;

int debug=0; //2|4; //=1 or 2 or 6// for printing debug messages

//int *opt_id[MXNUM];

using namespace std;

void prnt(const char* fmt...)
{
  //format:
  // d - integer
  // l - Long64_t
  // x - ULong64_t hex
  // s - char*
  // f - double
  // [0..9] - set width (only 1 digit) of next output
  // . - set precision (e.g 3.2 - width3.precision2)
  // ; - endl;
  cmut.Lock();

  va_list args;
  va_start(args, fmt);

  int ww=0;
  int pp=-1;


  while (*fmt != '\0') {
    if (*fmt == ' ') {
      std::cout << " ";
    }
    else if (*fmt == 'd') {
      int i = va_arg(args, int);
      std::cout << i;
      ww=0;pp=-1;
    }
    else if (*fmt == 'l') {
      Long64_t i = va_arg(args, Long64_t);
      std::cout << i;
      ww=0;pp=-1;
    }
    else if (*fmt == 'x') {
      ULong64_t i = va_arg(args, ULong64_t);
      std::cout << hex << i << dec;
      ww=0;pp=-1;
    }
    else if (*fmt == 's') {
      char* s = va_arg(args, char*);
      std::cout << s;
      ww=0;pp=-1;
    } 
    else if (*fmt == 'f') {
      double d = va_arg(args, double);
      std::cout << d;
      ww=0;pp=-1;
    }
    else if (isdigit(*fmt)) {
      char c = *fmt - '0';
      if (pp<0) { //width
        ww=ww*10+c;
        std::cout << std::setw(ww);
      }
      else { //precision
        pp=pp*10+c;
        std::cout << std::fixed;
        std::cout << std::setprecision(pp);
      }
    }
    else if (*fmt == '.') {
      pp=0;
    }
    else if (*fmt == ';') {
      std::cout << endl;
    }
    ++fmt;
  }

  std::cout << std::setprecision(-1);
  va_end(args);
  cmut.UnLock();
}

void debug_mess(bool cond, const char* mess, double par1, int par2) {
  if (cond) {
    cout << mess << par1;
    if (par2!=-9999)
      cout << " " << par2;
    cout << endl;
  }
}

void print_var(int tp, TDataMember *dm, TString vname, TString memname, int len=0, int len2=0) {
  const char* dmname = "--";
  const char* col=BGRN;
  if (tp) { //найдено
    dmname=dm->GetName();
    if (tp==2)
      col=BCYN;
    prnt("ssss",col,vname.Data(),".",dmname);
    for (int i=0;i<dm->GetArrayDim();i++) {
      int mx=dm->GetMaxIndex(i);
      //cout << " dm: " << dm->GetArrayDim() << " " << mx << " ";
      prnt("sds","[",mx,"]");
    }
    if (len!=len2) {
      len2/=dm->GetUnitSize();
      len/=dm->GetUnitSize();
      prnt("ssd d dss",BMAG,"[",len2,len,dm->GetMaxIndex(0),"]",col);      
    }
    int tt=dm->GetDataType()->GetType();
    const char* nn = dm->GetDataType()->GetTypeName();
    prnt(" d ss;",tt,nn,RST);
  }
  else { //не найдено
    col=BRED;
    prnt("ss ssss;",col,"#",vname.Data(),".",memname.Data(),RST);
  }	
}

int evalpar(l_iter it, char* var, const char* varname) {
  //returns 0 if OK

  //cout << "listpar: " << listpar.size() << endl;
  //for (l_iter it=listpar.begin(); it!=listpar.end(); ++it) {
    //try {
    TString par, // parameter name
      p0, //parameter with []
      p2, //index
      sdata; //value
    int index;
    Ssiz_t ll,len;
    char sbuf[1024];
    int buflen=0;
    //char* var = (char*) &opt;

    ll = it->First("=");
    p0 = (*it)(0,ll);
    sdata=(*it)(ll+1,it->Length());

    ll = p0.First("[");
    len = p0.First("]");

    if (ll==kNPOS || len==kNPOS) {
      par=p0;
      index=-1;
    }
    else {
      len=len-ll-1;
      par=p0(0,ll);
      p2=p0(ll+1,len);
      index=p2.Atoi();
    }

    //cout << "par: " << par << " " << index << endl;

    TList* lst = TClass::GetClass(varname)->GetListOfDataMembers();
    TDataMember* dm = (TDataMember*) lst->FindObject(par.Data());
    if (dm) {
      TString tp = dm->GetTypeName();
      int dim=dm->GetArrayDim();
      int maxindex = dm->GetMaxIndex(0);
      int unit = dm->GetUnitSize();
      Long_t off = dm->GetOffset();
      if (tp.Contains("int",TString::kIgnoreCase)) {
	int d=sdata.Atoi();
	buflen=sizeof(int);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("bool",TString::kIgnoreCase)) {
	bool d=sdata.Atoi();
	buflen=sizeof(bool);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("float",TString::kIgnoreCase)) {
	float d=sdata.Atof();
	buflen=sizeof(float);
	memcpy(sbuf,&d,buflen);
      }
      else if (tp.Contains("char",TString::kIgnoreCase)) {
	buflen=sdata.Length();
	memset(var+off,0,maxindex);
	memcpy(var+off,sdata.Data(),TMath::Min(buflen,maxindex));
	//continue;
	return 0;
      }
      else {
	cout << "Error: unknown type of parameter: "
	     << par << " " << tp << endl;
	//continue;
	return 1;
      }

      if (dim==0) { //only one parameter
	memcpy(var+off,sbuf,TMath::Min(buflen,unit));
	cout << "par: " << par << "=" << sdata << endl;
      }
      else if (dim==1) { //array [text was copied in the if("char") ]
	if (index<0) {
	  for (int j=0;j<maxindex;j++) {
	    memcpy(var+off+j*unit,sbuf,TMath::Min(buflen,unit));
	  }
	  cout << "par: " << par << "[]=" << sdata << endl;
	}
	else if (index<maxindex) {
	  memcpy(var+off+index*unit,sbuf,TMath::Min(buflen,unit));
	  cout << "par: " << par << "[" <<index<<"]=" << sdata << endl;
	}
	else {
	  cout << "Error: index is out of range : "
	       << par << " " << index << " " << maxindex << endl;
	  //continue;
	  return 2;
	}
      }
      else { //2-dim of more - ignore
	cout << "Error: parameter with dimension >1 : "
	     << par << " " << dim << endl;
	//continue;
	return 3;
      }
    } //if dm
    else {
      //cout << "Parameter " << par.Data() << " not found" << endl;
      return 100;
    }
    //} //for l_iter
    return 0;
}

//-------------------------------------
Int_t ClassToBuf(const char* clname, const char* varname, char* var, char* buf) {
  //copies all data members to a buffer, returns size of the buffer
  //buffer should exist and have sufficient size to allocate all data

  //cout << "ClassToBuf: " << name << endl;
  TList* lst = TClass::GetClass(clname)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << clname << " doesn't exist" << endl;
    return 0;
  }

  Int_t sz=0;
  Short_t len=0;

  char vname[100];
  strcpy(vname,"class");

  len = strlen(vname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,vname,len);
  sz+=len;

  len = strlen(clname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,clname,len);
  sz+=len;

  strcpy(vname,"var");
  len = strlen(vname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,vname,len);
  sz+=len;

  len = strlen(varname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,varname,len);
  sz+=len;

  if (debug&0x2)
    cout << "\033[1;31mSave class var: \033[0m" << clname << " " << varname << endl;

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
    //prnt("ss ss;",BGRN,"CTB:",dm->GetName(),RST);
    if (dm->GetDataType()) {
      len = strlen(dm->GetName())+1;
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,dm->GetName(),len);
      sz+=len;
      len=dm->GetUnitSize();

      for (int i=0;i<dm->GetArrayDim();i++) {
	len*=dm->GetMaxIndex(i);
      }
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,var+dm->GetOffset(),len);
      sz+=len;
      if (debug&0x2)
	cout << "member: " << dm->GetName() << " " << len << " " << sz << " " << dm->GetDataType()->GetType() << " " << dm->GetDataType()->GetTypeName() << endl;
    }
    // else {
    //   cout << "member: " << dm->GetName() << " " << len << " " << sz << " " << dm->GetDataType() << " " << dm->GetTypeName() << endl;
    // }
  }

  return sz;

}

//----------------------------------------
void MakeVarList(int cp, int op) {
  //cp!=0 - add cpar
  //op!=0 - add opt

  varlist.clear();
  VarClass vv;
  if (cp) { //cpar
    vv.Var = (char*) &cpar;
    TList* l1 = TClass::GetClass("Coptions")->GetListOfDataMembers();
    TDataMember *d1 = (TDataMember*) l1->First();
    while (d1) {
      if (d1->GetDataType()) {
	vv.name=d1->GetName();
	vv.Dm=d1;
	varlist.push_back(vv);
      }
      d1 = (TDataMember*)l1->After(d1);
    }
  }

  if (op) { //opt
    TList* l1 = TClass::GetClass("Toptions")->GetListOfDataMembers();
    TDataMember *d1 = (TDataMember*) l1->First();
    while (d1) {
      if (strcmp(d1->GetTypeName(),"Hdef")) { //not Hdef
	vv.name=d1->GetName();
	vv.Var = (char*) &opt;
	vv.Dm=d1;
	varlist.push_back(vv);
      }
      else { //Hdef
	TList* l2 = TClass::GetClass("Hdef")->GetListOfDataMembers();
	TDataMember *d2 = (TDataMember*) l2->First();
	while (d2) {
	  if (d2->GetDataType()) {
	    vv.name=d1->GetName();
	    vv.name+='.';
	    vv.name+=d2->GetName();
	    vv.Var = (char*) &opt+d1->GetOffset();
	    vv.Dm=d2;
	    varlist.push_back(vv);
	  }
	  d2 = (TDataMember*)l2->After(d2);
	}
      }
      d1 = (TDataMember*)l1->After(d1);
    }
  }
  // for (v_iter it=varlist.begin();it!=varlist.end();++it) {
  //   cout << "varlist: " << it->name << " " << it->Dm->GetName() << endl;
  // }
  //cout << varlist.size() << endl;
  // exit(1);
}

void OneVar(char* var, TDataMember* dm, char* data, UShort_t len,
	    TString & memname) {
  //копирует одну переменную из буфера *data в переменную var+dm

  UShort_t len2=0;
  len2=dm->GetUnitSize();
  for (int i=0;i<dm->GetArrayDim();i++) {
    len2*=dm->GetMaxIndex(i);
  }
  if (len2==len) {
    memcpy(var+dm->GetOffset(),data,len);
  }
  else {
    if (memname.EqualTo("ch_name")) {
      if (strcmp(opt.gitver,"v0.73")<0) { //gitver is earlier than 0.73
	int z1 = 6;
	int z2 = len2/MAX_TP;
	for (int i=0;i<6;i++) {
	  memcpy(var+dm->GetOffset()+z2*i,data+z1*i,z1);
	}
      }
    } //ch_name
    else {
      memcpy(var+dm->GetOffset(),data,TMath::Min(len,len2));
    }
  }
  // if (debug&0x4) {
  //   print_var(dm,varname,memname,len,len2);
  // }
}

void Read_Hdef(char* data, UShort_t len, TString &varname, TString &memname) {
  Mdef* md=0;
  for (auto it=list_mdef.rbegin();it!=list_mdef.rend();++it) {
    if (it->h_name.EqualTo(varname)) {
      md = &*it;
      //prnt("ssss;",BGRN,"Found: ",varname.Data(),RST);
      break;
    }
  }
  if (!md) {
    list_mdef.push_back(Mdef());
    md = &list_mdef.back();
    md->hd = new Hdef();
    md->h_name = varname;
    //prnt("ssss;",BRED,"Found: ",varname.Data(),RST);
  }

  TList* l1 = TClass::GetClass("Hdef")->GetListOfDataMembers();
  TDataMember *d1 = (TDataMember*) l1->First();
  while (d1) {
    if (d1->GetDataType()) {
      if (memname.EqualTo(d1->GetName())) {
	//prnt("ssss;",BBLU,"Vname: ",d1->GetName(),RST);
	OneVar((char*)(md->hd),d1,data,len,memname);
      }
    }
    d1 = (TDataMember*)l1->After(d1);
  }

  //Hdef* 
  //cout << "Hdef2: " << varname << " " << memname << endl;
}

void check_old_2d(Mdef* it) {
  TString h2old[]
    = {
       "h_axay",         "Area.Area",
       "h_area_base",    "Area.Base",
       "h_area_sl1",     "Area.Sl1",
       "h_area_sl2",     "Area.Sl2",
       "h_slope_12",     "Sl1.Sl2",
       "h_area_time",    "Area.Time",
       "h_area_width",   "Area.Width",
       "h_area_width2",  "xxx",
       "h_area_ntof",    "Area.Ntof",
       ""
  };

  if (!it->hd->b) return;
  //prnt("sss d ds;",BYEL,"Old1: ",it->h_name.Data(),it->hd->b,it->hnum,RST);
  int i=0;
  do {
    if (h2old[i].EqualTo(it->h_name)) {
      it->h_name = h2old[i+1];
      break;
    }
    //cout << i << " " << h2old[i] << " " << h2old[i].Length() << endl;
    i+=2;
  } while (h2old[i].Length()>0);

  //prnt("sss d ds;",BBLU,"Old2: ",it->h_name.Data(),it->hd->b,it->hnum,RST);
}

Mdef* Create_2d(Mdef* it) {
  Mdef* res=0;
  int sz = it->h_name.First('.');
  TString s1 = it->h_name(0,sz);
  TString s2 = it->h_name(sz+1,9999);
  //Mdef *m1=0, *m2=0;
  //mdef_iter m1, m2;
  int id1=0,id2=0;

  for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
    if (it->name.EqualTo(s1))
      id1 = it->hnum;
      //m1 = &*it;
    if (it->name.EqualTo(s2))
      id2 = it->hnum;
      //m2 = &*it;
  }

  if (id1 && id2) {
    res=hcl->Add_h2(id1,id2);
    //prnt("ss d ds;",BGRN,"C2d: ",id1,id2,RST);
  }
  return res;
  //cout << "C2d: " << s1 << " " << s2 << " " << m1 << " " << m2 << endl;
}

int BufToClass(char* buf, char* buf2, int op) {
  // Считывает все параметры в буфере buf
  // прекращает считывание в конце буфера (buf2 должно быть = buf+sz),
  // Сравнивает имя найденного параметра со всеми именами в varlist
  // (varlist) должен быть создан до вызова BufToClass
  // Если находит, записывает значение найденного параметра
  // в соответствующую переменную

  // op=1 - read opt -> читаем все hdef
  // op=0 - не читаем hdef

  // Возвращает 1 - если копирование успешно
  //            0 - если произошла ошибка

  for (auto it=list_mdef.rbegin();it!=list_mdef.rend();++it) {
    delete it->hd;
  }
  list_mdef.clear();

  TString classname;
  TString varname;
  TString memname;
  UShort_t len=0;
  const UShort_t mx=50000;
  char data[mx];

  while (buf+sizeof(len)<buf2) {

    memcpy(&len,buf,sizeof(len)); //считываем длину записи имени в буфере
    //prnt("ssd d ls;",BYEL,"bf2: ",len,mx,buf+len-buf2,RST);

    //могут быть нули в конце буфера из-за кратности 8
    if (len==0 && buf2-buf<10) {
      //cout << "len==0: " << len << " " << buf2-buf << endl;
      break;
      //return 1;
    }
    else if (len==0 || len>=mx || buf+len>buf2) {
      prnt("ssd d ls;",BRED,"errlen1: ",len,mx,buf+len-buf2,RST);
      //cout << "len1 error: " << len << " " << buf2-buf << endl;
      return 0;
    }
    buf+=sizeof(len);
    memname = TString(buf,len-1); //считываем имя в буфере
    buf+=len;

    // data
    memcpy(&len,buf,sizeof(len)); //считываем длину данных в буфере
    if (len==0 || len>=mx || buf+len>buf2) {
      cout << "len2 error: " << len << " " << buf2-buf << endl;
      return 0;
    }
    buf+=sizeof(len);
    memcpy(data,buf,len); //считываем данные в буфере
    buf+=len;

    if (memname.EqualTo("class")) {
      classname = TString(data,len-1); //считываем имя класса в буфере
      continue;
    }
    else if (memname.EqualTo("var")) {
      varname = TString(data,len-1); //считываем имя переменной в буфере
      continue;
    }

    if (classname.EqualTo("Hdef")) { // read Hdef variables
      //memname.Prepend('.');
      //memname.Prepend(varname);

      //cout << "Hdef: " << classname << " " << varname << " " << memname << endl;
      if (op)
	Read_Hdef(data,len,varname,memname);
    }
    else { //read all other variables

      // ищем соответствующий член класса
      int tp=0;
      // тип находки:
      // tp==0 - не найдено
      // tp==1 - найдено
      // tp==2 - найдено старое название //[] (в комментариях)

      v_iter it;
      for (it=varlist.begin();it!=varlist.end();++it) {
	if (memname.EqualTo(it->name)) {
	  tp=1;
	  break;
	}
	else {
	  // ищем старое название члена класа (в комментариях []),
	  // для обратной совместимости
	  TString s1 = it->Dm->GetTitle();
	  int a,b;

	  do {
	    a=s1.First('[');
	    b=s1.First(']');
	    if (a>=0 && b>=0) {
	      if (memname.EqualTo(s1(a+1,b-a-1))) {
		tp=2;
		break;
	      }
	    }
	    s1 = s1(b+1,999);
	  } while (b>=0);
	  if (tp) break;
	} //else
      }

      TDataMember* dm = 0;
      if (tp) { //найдено
	OneVar(it->Var,it->Dm,data,len,memname);
	varlist.erase(it);
	if (debug&0x4) {
	  print_var(tp,it->Dm,varname,memname,len);
	}
      }
      else { //не найдено
	if (debug&0x4) {
	  print_var(tp,dm,varname,memname);
	}
      }
    } //else -> read other varialbes

  } //while sz<size

  // копируем Hdef, попутно создаем 2d-гистограммы
  for (auto it=list_mdef.begin();it!=list_mdef.end();++it) {
    Mdef* target = 0;
    //bool found=0;
    for (auto it2 = hcl->Mlist.begin();it2!=hcl->Mlist.end();++it2) {
      if (it->h_name.EqualTo(it2->h_name)) { //найдено
	target = &*it2;
	//*(it2->hd) = *(it->hd);
    	//found=1;
    	break;
      }
    }
    if (!target) { // не найдено - создаем?
      check_old_2d(&*it);
      //prnt("sss ds;",BRED,"Hdef-: ",it->h_name.Data(),it->hd->b,RST);
      target = Create_2d(&*it);
    }
    else {// найдено - ничего не делаем
      //prnt("sss ds;",BYEL,"Hdef+: ",it->h_name.Data(),it->hd->b,RST);
    }

    if (target) {
      //копируем
      *(target->hd) = *(it->hd);
    }
  } //for it

  // for (auto it = hcl->Mlist.begin();it!=hcl->Mlist.end();++it) {
  //   prnt("sss d ds;",BMAG,"Mlist: ",it->h_name.Data(),it->hd->b,it->hnum,RST);
  // }
  
  return 1;
} //BufToClass
//--------------------------------

void SaveParTxt(string fname) {

  string txtname = fname+".txt";
  ofstream fout(txtname);

  MakeVarList(1,1);
  for (v_iter it=varlist.begin();it!=varlist.end();++it) {
    char* cc = it->Var+it->Dm->GetOffset();
    Int_t* ivar = (Int_t*) cc;
    Float_t* fvar = (Float_t*) cc;
    int usz = it->Dm->GetUnitSize();
    int adim = it->Dm->GetArrayDim();
    int len=1;
    
    int tp=0;
    if (it->Dm->GetDataType()) {
      tp = it->Dm->GetDataType()->GetType();

      if (tp==1 && adim>0) { //char
	--adim;
	usz=it->Dm->GetMaxIndex(adim);
      }
      fout << it->name;
      for (int i=0;i<adim;i++) {
	fout << '[' << it->Dm->GetMaxIndex(i) << ']';
	len*=it->Dm->GetMaxIndex(i);
      }
      if (!strcmp(it->Dm->GetName(),"F_start")) {
	fout << " " << cpar.F_start << " " << crs->txt_start << endl;
	continue;
      }
      //cout << " " << len << " " << adim << " " << usz << endl;
      for (int i=0;i<len;i++) {
	switch (tp) {
	case 1: { //char*
	  fout << " \"" << cc+i*usz << "\"";
	  break; }
	case 3: { //int
	  fout << " " << ivar[i];
	  break; }
	case 5: { //float
	  fout << " " << fvar[i];
	  break; }
	case 18: { //bool
	  fout << " " << (int)cc[i];
	  break; }
	default: {
	  fout << " # unknown type of class member" << endl; }
	}
      }
      fout << endl;
    }

  }
}

//--------------------------------
void ReadParTxt(string fname) {

  ifstream fin(fname);
  ofstream fout;
  string line;

  if (!fin.good()) {
    cout << "File " << fname << " not found" << endl;
    return;
  }

  MakeVarList(1,1);

  while (!fin.eof()) {
    getline(fin,line);
    istringstream iss(line);
    string name,dd;
    std::vector<string> data;
    iss>>name;
    name = name.substr(0,name.find_first_of('['));
    //cout << name;
    while (!iss.eof()) {
      iss>>dd;
      data.push_back(dd);
      //cout << " " << dd;
    }
    //cout << endl;
    v_iter it;
    for (it=varlist.begin();it!=varlist.end();++it) {
      if (it->name==name) {
	//cout << "Found: " << name << " " << varlist.size() << endl;
	break;
      }
    }
    if (it!=varlist.end()) { //found

      char* cc = it->Var+it->Dm->GetOffset();
      Int_t* ivar = (Int_t*) cc;
      Float_t* fvar = (Float_t*) cc;
      int usz = it->Dm->GetUnitSize();
      int adim = it->Dm->GetArrayDim();
      size_t len=1;    
      int tp=0;
      if (it->Dm->GetDataType()) {
	if (it->name=="F_start") {
	  //crs->Text_time();
	  fout << " " << cpar.F_start << " " << crs->txt_start << endl;
	  //cout << "F_start: " << opt.F_start << " " << crs->txt_start << " !!" << endl;
	}
	else { //not F_start
	  tp = it->Dm->GetDataType()->GetType();
	  if (tp==1 && adim>0) { //char
	    --adim;
	    usz=it->Dm->GetMaxIndex(adim);
	  }
	  for (int i=0;i<adim;i++) {
	    len*=it->Dm->GetMaxIndex(i);
	  }
	  size_t len2 = std::min(len,data.size());
	  //cout << name << ": " << len2 << " " << adim << " " << usz << endl;
	  for (size_t i=0;i<len2;i++) {
	    switch (tp) {
	    case 1: { //char*
	      string* s = &data[i];
	      s->erase(std::remove(s->begin(),s->end(),'\"'),s->end());
	      strncpy(cc+i*usz,s->c_str(),usz);
	      //fout << " " << cc+i*usz;
	      break; }
	    case 3: { //int
	      ivar[i]=stoi(data[i]);
	      //fout << " " << ivar[i];
	      break; }
	    case 5: { //float
	      fvar[i]=stof(data[i]);
	      //fout << " " << fvar[i];
	      break; }
	    case 18: { //bool
	      cc[i]=stoi(data[i]);
	      //fout << " " << (int)cc[i];
	      break; }
	    default: {
	      ;//fout << "# unknown type of class member" << endl; }
	    }
	    } //switch
	    //fout << endl;
	  } //for i
	}
      }

      varlist.erase(it);
    } //if found
  } //while
  for (v_iter it=varlist.begin();it!=varlist.end();++it) {
    cout << "varlist: " << it->name << endl;
  }
  return;

  for (v_iter it=varlist.begin();it!=varlist.end();++it) {
    char* cc = it->Var+it->Dm->GetOffset();
    Int_t* ivar = (Int_t*) cc;
    Float_t* fvar = (Float_t*) cc;
    int usz = it->Dm->GetUnitSize();
    int adim = it->Dm->GetArrayDim();
    int len=1;    
    int tp=0;
    if (it->Dm->GetDataType()) {
      tp = it->Dm->GetDataType()->GetType();

      if (tp==1 && adim>0) { //char
	--adim;
	usz=it->Dm->GetMaxIndex(adim);
      }
      fout << it->name;
      for (int i=0;i<adim;i++) {
	fout << '[' << it->Dm->GetMaxIndex(i) << ']';
	len*=it->Dm->GetMaxIndex(i);
      }
      if (!strcmp(it->Dm->GetName(),"F_start")) {
	//crs->Text_time();
	fout << " " << cpar.F_start << " " << crs->txt_start << endl;
	//cout << "F_start: " << opt.F_start << " " << crs->txt_start << " !!" << endl;
	continue;
      }
      //cout << " " << len << " " << adim << " " << usz << endl;
      for (int i=0;i<len;i++) {
	switch (tp) {
	case 1: { //char*
	  fout << " " << cc+i*usz;
	  break; }
	case 3: { //int
	  fout << " " << ivar[i];
	  break; }
	case 5: { //float
	  fout << " " << fvar[i];
	  break; }
	case 18: { //bool
	  fout << " " << (int)cc[i];
	  break; }
	default: {
	  fout << "# unknown type of class member" << endl; }
	}
      }
      fout << endl;
    } //if

  } //for
}

//--------------------------------
int FindVar(char* buf, int sz, const char* name, char* var) {
  std::string str(buf,sz);
  size_t pos=str.find(name);
  if (pos!=std::string::npos) {
    char* buf2 = buf+pos;
    buf2 += strlen(buf2)+1;
    short len = *(short*) buf2;;
    buf2+=sizeof(len);
    memcpy(var,buf2,len);
    return 1;
  }
  else {
    return 0;
    //cout << "Var not found: " << name << endl;
  }
}

// void out_of_memory(void)
// {
//   std::cerr << "Out of memory. Please go out and buy some more." << std::endl;
//   exit(-1);
// }

void ctrl_c_handler(int s){
  printf("Caught signal %d\n",s);

  if (crs->b_acq && crs->Fmode==1) {
    myM->DoStartStop(0);
    gSystem->Sleep(300);
    return;
  }

  if (crs->b_fana) {
    myM->DoAna();
    gSystem->Sleep(300);
    return;
  }

  gApplication->Terminate(0);
  // delete myM;
  // exit(1); 
}

void segfault_c_handler(int signal, siginfo_t *si, void *arg) {
  printf("Caught segfault %d\n",signal);
  gApplication->Terminate(0);
  // delete myM;
  // exit(-1);
  //exit(1); 
}

void duplcheck() {
  TList* l1 = TClass::GetClass("Coptions")->GetListOfDataMembers();
  TList* l2 = TClass::GetClass("Toptions")->GetListOfDataMembers();
  TDataMember *d1 = (TDataMember*) l1->First();
  while (d1) {
    TDataMember *d2 = (TDataMember*) l2->First();
    if (d1->GetDataType()) {
      //cout << d1->GetName() << " " << d1->GetDataType() << endl;
      while (d2) {
	if (d1->GetDataType()) {
	  if (!strcmp(d1->GetName(),d2->GetName())) {
	    cout << "Duplicate names: " << d1->GetName() << " " << d2->GetName() << endl;
	    exit(1);
	  }
	}
	d2 = (TDataMember*)l2->After(d2);
      }; //d2
    }
    d1 = (TDataMember*)l1->After(d1);
  }
}

int main(int argc, char **argv)
{

  //common_init();
  string s_name, dir, name, ext;

  //ROOT::EnableThreadSafety();

  //cross check for duplicate names in opt and cpar
  duplcheck();
  
  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = ctrl_c_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  // void segfault_sigaction(int signal, siginfo_t *si, void *arg)
  // {
  //     printf("Caught segfault at address %p\n", si->si_addr);
  //     exit(0);
  // }

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = segfault_c_handler;
  sa.sa_flags   = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);

  // cout << "sizeof(TDatime): " << sizeof(TDatime) << endl;
  // cout << "sizeof(Toptions): " << sizeof(Toptions) << endl;
  // cout << "sizeof(opt): " << sizeof(opt) << endl;
	
  hcl = new HClass();
  crs = new CRS();

  // #ifdef CYUSB
  //   crs->Detect_device();
  // #endif

#ifdef LINUX
  if (getcwd(startdir,150)) {
    strcat(startdir,"/");
  }
#else
  _getcwd(startdir,150);
  strcat(startdir,"\\");
#endif

  const char* help =
    "----------------------------------------------\n"
    "Usage: romana [filename] [options] (or romana [options] [filename])\n"
    "filename: read data and parameters from filename\n"
    "options: \n"
    "-h: print usage and exit\n"
    "-u: reset USB after detecting device\n"
    "-p parfile: read parameters from parfile, parameters from filename are ignored\n"
    "-t parfile: save parameters from parfile to text file parfile.txt and exit\n"
    "-m module: set module type (see Expert -> module); assume input file without header\n"
    "-a filename: start acquisition in batch mode (without gui)\n"
    "-b [filename]: analyze file in batch mode (without gui) and exit\n"
    "   Data are saved in filename[.raw/.dec/.root] depending on batch parameters\n"
    "   [filename] is optional for -b. If omitted, the input filename is used.\n"
    "   Program exits after Tstop time is reached\n"

    "-s [n] (only in batch mode): screen output frequency (0 - no output; n - every n-th buffer) \n"
    "-w (only in batch mode): create processed .raw file in subdirectory Raw/\n"
    "-d (only in batch mode): create decoded .dec file in subdirectory Dec/\n"
    "-r (only in batch mode): create .root file in subdirectory Root/\n"
    "-f (only in batch mode): force overwriting .raw/.dec/.root files\n"
    "\n"
    "   Parameters <Filename>, <Write raw/decoded/root data> from input file\n"
    "   are ignored in batch mode\n"
    "[par=val] - set parameter par to val (see toptions.h for parameter names)\n"
    "   examples: Tstop=10 (set time limit to 10 sec)\n"
    "             Thr[5]=20 (set threshold for ch5 to 20)\n"
    "             Thr=20 (set threshold for all channels to 20)\n"
    "   works only with Toptions class; doesn't work with 2d/3d hist parameters\n"
    //"[par:] - print value(s) of the parameter par\n"
    "----------------------------------------------";

  cout << help << endl;

  strcpy(pr_name,argv[0]);
  //strcpy(maintitle,pr_name);

  crs->abatch=true; //default: acquisition (for detect_device)

  listpar.clear();
  //listshow.clear();
  //process command line parameters
  if (argc > 1) {
    for (int i=1;i<argc;i++) {
      //int argnn=1;
      //while (argnn<argc) {
      // cout << "argnn: " << argc << " " << argnn << " " << argv[argnn] << " "
      //     << argv[argnn]+1 << endl;
      TString sarg=TString(argv[i]);

      if (sarg[0]=='-') {
	//cout << "sarg: " << i << " " << sarg << " " << (int) sarg[1] << endl;
	switch (tolower(sarg[1])) {
	case 'h':
	  exit(0);
	case 'u': //reset usb
	  b_resetusb=true;
	  continue;
	case 'm': //module
	  crs->b_noheader=true;

	  ++i;
	  if (i<argc) {
	    char* p;
	    crs->module = strtol(argv[i], &p, 10);
	    if (*p) {
	      //crs->scrn=-1;
	      --i;
	    }
	  }
	  cout << "modl: " << crs->module << endl;

	  continue;
	case 'a': //acquisition in batch
	  ++i;
	  if (i<argc) {
	    batch_fname = argv[i];
	    if (batch_fname[0]!='-') {
	      crs->batch=true;
	      crs->abatch=true;
	      continue;
	    }
	  }
	  prnt("sss;",BRED,"Parameter -a must be followed by a filename",RST);
	  exit(-1);

	case 'b': //file in batch
	  crs->batch=true;
	  crs->abatch=false;

	  ++i;
	  if (i<argc) {
	    if (argv[i][0]=='-') {
	      --i;
	    }
	    else {
	      batch_fname = argv[i];
	      //strcpy(opt.Filename,argv[i]);
	    }	    
	  }

	  continue;
	case 's': {
	  ++i;
	  //crs->scrn=0;
	  if (i<argc) {
	    char* p;
	    crs->scrn = strtol(argv[i], &p, 10);
	    if (*p) {
	      //crs->scrn=-1;
	      --i;
	    }
	    // try {
	    //   crs->scrn = std::stoi(std::string(argv[i]));
	    // }
	    // catch(std::invalid_argument&) {
	    //   cout << "catch: " << endl;
	    //   --i;
	    // }
	  }
	  continue;
	}
	case 'w':
	  b_raw=true;
	  continue;
	case 'd':
	  b_dec=true;
	  continue;
	case 'r':
	  b_root=true;
	  continue;
	case 'f':
	  b_force=true;
	  continue;
	case 'p':
	  ++i;
	  if (i<argc) {
	    parfile2 = argv[i];
	  }
	  //cout << "parfile2: " << parfile2 << endl;
	  continue;
	case 't':
	  ++i;
	  if (i<argc) {
	    parfile2 = argv[i];
	  }
	  //cout << "parfile2: " << parfile2 << endl;
	  if (parfile2) {
	    ff = gzopen(parfile2,"rb");
	    if (!ff) {
	      cout << "Can't open par file: " << parfile2 << endl;
	    }
	    else {
	      crs->ReadParGz(ff,parfile2,0,1,1);
	      gzclose(ff);
	      SaveParTxt(string(parfile2));

	      // string ff = string(parfile2)+".txt";
	      // ReadParTxt(ff);
	      // gzFile f2 = gzopen("dum.par","wb");
	      // crs->SaveParGz(f2,crs->module);
	      // gzclose(f2);
	    }
	  }
	  exit(0);
	  continue;
	default:
	  continue;
	}
      } //if (sarg[0]=='-')
      //else if (sarg.find("=")!=string::npos) {
      else if (sarg.First("=")>=0) {
	listpar.push_back(sarg);
      }
      // else if (sarg.First(":")>=0) {
      // 	listshow.push_back(sarg);
      // }
      else {
	datfname = argv[i];
      }
      /*
	char cc = argv[i][0];
	if (cc=='-') { //control character
	char pp = argv[i][1];
	}
	else if (cc=='+') { //read file of parameters
	parfile2 = argv[argnn]+1;
	}
	else { //read file
	datfname = argv[argnn];
	}
	argnn++;
      */
    } //for i
  }





  //сначала читаем -p parfile2
  //потом читаем параметры из datfname - если parfile2 был считан, то из datfname читаем только cpar
  //в конце переписываем cpar.on из parfile2 если они были считаны

  char* cparon=0;
  int rdopt=1; //read opt from file
  if (parfile2) { //read -p par file
    ff = gzopen(parfile2,"rb");
    if (!ff) {
      cout << "Can't open par file: " << parfile2 << endl;
      exit(-1);
    }
    else {
      crs->ReadParGz(ff,parfile2,0,1,1);
      gzclose(ff);
      rdopt=0; // if parfile2 is OK -> don't read opt from file
      cparon=new char[sizeof(cpar.on)];
      memcpy(cparon,cpar.on,sizeof(cpar.on));
    }
  }
  else { //read default romana.par file
    ff = gzopen(parfile,"rb");
    if (!ff) {
      cout << "Can't open " << parfile << " file. Using default parameters." << endl;
    }
    else {
      crs->ReadParGz(ff,parfile,0,1,1);
      gzclose(ff);
    }
  }

  //cout << "rd_root0: " << rd_root << endl;
  //cout << "cpar.on: " << sizeof(cpar.on) << endl;

  if (datfname) {
    string dir, name, ext;
    SplitFilename(string(datfname),dir,name,ext);
    //cout << dir << " " << name << " " << ext << endl;
    if (!ext.compare(".root")) { //root file
      if (!parfile2) {
	if (readpar_root(datfname,1))
	  exit(-1);
      }
      rd_root=true;
      //prnt("ss d ds;",BBLU,"root:",rd_root,(bool)parfile2,RST);
    }
    else { //.raw or .dec file
      if (crs->DoFopen(datfname,1,rdopt)) //read file and parameters from it
	exit(-1);
    }
  }
  else {
    datfname=(char*)"";
  }

  if (cparon) {
    memcpy(cpar.on,cparon,sizeof(cpar.on));
    delete[] cparon;
  }

  //exit(1);
  //change individual parameters if listpar is not empty
  for (l_iter it=listpar.begin(); it!=listpar.end(); ++it) {
    int res=0;
    res+=evalpar(it,(char*)&opt,"Toptions");
    res+=evalpar(it,(char*)&cpar,"Coptions");
    if (res>=200) {
      prnt("ssssds;",BRED,"Parameter ",it->Data()," not found: ",res,RST);
    }
  }

  //show individual parameters if listshow is not empty
  //showpar();

  //cout << "strlen: " << strlen(datfname) << endl;
  //cout << "gStyle1: " << gStyle << endl;
  //hcl->Make_hist();
  //cout << "gStyle2: " << gStyle << endl;

  //cout << "detect: " << crs->abatch << " " << endl;

  int ret=1;
#ifdef CYUSB
  if (crs->abatch) {
    if (crs->Fmode!=2 && !rd_root) {
      //if (crs->Fmode!=2) {
      bool d = opt.directraw;
      bool w = opt.raw_write;
      opt.directraw=1;
      opt.raw_write=0;
      ret=crs->Detect_device();
      if (b_resetusb)
	crs->DoResetUSB();
      opt.directraw=d;
      opt.raw_write=w;
    }
  }
#endif

  //batch loop
  EvtFrm = 0;
  if (crs->batch) {
    if (strlen(datfname)==0 && !crs->abatch) {
      cout << "No input file. Exiting..." << endl;
      exit(0);
    }

    if (crs->abatch && ret) {
      cout << "Device is not connected. Exiting..." << endl;
      exit(0);
    }

    if (batch_fname) {
      strcpy(opt.Filename,batch_fname);
    }
    else {
      strcpy(opt.Filename,crs->Fname);
    }
    // if (crs->abatch) {
    //   strcpy(crs->Fname,batch_fname);
    // }

    if (!TestFile()) {
      exit(0);
    }

    hcl->Make_hist();
    if (CheckMem(1)>700) { //>70%
      cout << "Not enough memory. Reduce the size of histograms. Exitting... " << pinfo.fMemResident*1e-3 << " " << minfo.fMemTotal << endl;
      crs->DoExit();
      exit(-1);
      // gApplication->Terminate(0);
    }

    if (crs->abatch) {
      prnt("sss;",BGRN,"Starting in batch mode.",RST);
      //prnt("ssss;",BGRN,"File = ",opt.Filename,RST);
      prnt("ssss;",BGRN,"Output file: ",opt.Filename,RST);
      prnt("ssfs;",BGRN,"Tstop=",opt.Tstop,RST);
      //prnt("ssss;",BGRN,"rawname=",crs->rawname.c_str(),RST);
#ifdef CYUSB
      crs->DoStartStop(1);
#endif
      crs->b_acq=false;
      crs->b_stop=true;
    }
    else {
      crs->b_fana=true;
      crs->b_stop=false;
      crs->FAnalyze2(false);
      crs->b_fana=false;
      crs->b_stop=true;
    }

    if (b_root) {
      cout << "Histograms are saved in: " << crs->rootname << endl;
      saveroot(crs->rootname.c_str());
    }

    for (int i=0;i<MAX_ERR;i++) {
      cout << crs->errlabel[i] << " " << crs->errors[i] << endl;
    }

    return 0;
  }

  TApplication theApp("App",&argc,argv);

  myM = new MyMainFrame(gClient->GetRoot(),800,600);

  glb = new GlbClass();
  TCollection* tglb = gROOT->GetListOfGlobalFunctions();
  tglb->Add(glb);

  //gSystem->Sleep(100);
  //crs->Dummy_trd();

  //EvtFrm->StartThread();
  //gClient->GetColorByName("yellow", yellow);
  //cout << "Init end2" << endl;

  //prtime("theRun");
  theApp.Run();

  //cout << "Init end3" << endl;
  //return 0;

  //fclose(fp);
  //gzclose(fp);

  /*
  //printf("%d buffers\n",nbuf);
  printf("%lld bytes\n",recd*2);
  printf("%d events\n",nevent);
  printf("%d bad events\n",bad_events);
  */
  //HRPUT(0,(char*)"spectra.hbook",(char*)" ");

  //memcpy(buf2,buf[0],BSIZE);

}

void SplitFilename (string str, string &folder, string &name)
{
  size_t found;
  found=str.find_last_of("/\\");

  folder = str.substr(0,found+1);
  name = str.substr(found+1);
  //cout << "spl1: " << str << ";" << folder << ";" << name << endl;
  //strcpy(folder,str.substr(0,found+1).c_str());
  //strcpy(name,str.substr(found+1).c_str());

}

void SplitFilename (string str, string &folder, string &name, string &ext)
{

  SplitFilename(str, folder, name);
  string fname(name);
  size_t found = fname.find_last_of(".");
  //cout << "spl22: " << found << " " << string::npos << endl;
  //return;
  name = fname.substr(0,found);
  if (found!=string::npos) {
    ext = fname.substr(found);
  }
  //cout << "spl2: " << str << ";" << folder << ";" << name << ";" << ext << endl;
  //strcpy(name,fname.substr(0,found).c_str());
  //strcpy(ext,fname.substr(found+1).c_str());
}

/*
void saveroot_old(const char *name) {

  TFile * tf = new TFile(name,"RECREATE");

  if (!tf->IsOpen()) {
    cout << "Can't open file: " << name << endl;
    return;
  }
  //cout << "saveroot11: " << name << " " << tf << " " << tf->IsOpen() << endl;

  int col;

  gROOT->cd();

  TIter next(gDirectory->GetList());
  //TIter next(hcl->map_list;);

  tf->cd();

  TH1F *h;
  TH2F *h2;
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (obj->InheritsFrom(TH2::Class())) {
      h2=(TH2F*) obj;
      //h2->Print();
      if (h2->GetEntries() > 0) {
	//printf("saveroot2: %s\n",h2->GetName());
	h2->Write();
      }
    }
    else if (obj->InheritsFrom(TH1::Class())) {
      h=(TH1F*) obj;
      //h->Print();
      if (h->GetEntries() > 0) {
	col=h->GetLineColor();
	//printf("saveroot1: %d %s\n",col,h->GetName());
	if (col==0) {
	  h->SetLineColor(50);
	}
	//tf->WriteTObject(obj);
	h->Write();
	h->SetLineColor(col);
      }
    }
  }

  //opt.Nevt=nevent;
  //opt.Tof=tof;
  cpar.Write();
  opt.Write();

  cout << "Histograms and parameters are saved in file: " << name << endl;

  tf->Close();
}
*/

void saveroot(const char *name) {

  TFile * tf = new TFile(name,"RECREATE");

  if (!tf->IsOpen()) {
    cout << "Can't open file: " << name << endl;
    return;
  }

  //gROOT->cd();

  TIter next(hcl->allmap_list);

  tf->cd();

  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    HMap* map = (HMap*) obj;
    if (map->hst->GetEntries() > 0) {
      map->hst->Write();
    }
  }

  cpar.Write();
  opt.Write();

  cout << "Histograms and parameters are saved in file: " << name << endl;

  tf->Close();
}

int readroot(const char *name) {
  //return 0 - OK; 1 - error

  gROOT->cd();
  // TList *list = hcl->hist_list;
  TList *list = hcl->allmap_list;
  //list->ls();

  TFile *tf = new TFile(name,"READ");
  if (!tf->IsOpen())
    return 1;

  TIter next(tf->GetListOfKeys());

  TKey *key;
  TObject *obj;
  TH1* obj2;
  while ((key = (TKey*)next())) {
    //key->Print();

    //cout << "key: " << key->GetClassName() << endl;
    //continue;

    //if (strcmp(key->GetClassName(),"Toptions")) {
    obj=key->ReadObj();
    if (obj->InheritsFrom(TH1::Class())) {
      //obj->Print();
      //cout << "obj: " << obj->GetName() << endl;
      HMap* map = (HMap*) list->FindObject(obj->GetName());
      if (map) {
	//cout << "map: " << map->GetName() << endl;
	//continue;
	obj2 = map->hst;
	//printf("%d\n",obj2);
	// cout << "readroot1: " << obj2 << " " << obj2->GetName() << " "
	//      << ((TH1*) obj2)->Integral() << endl;
	TDirectory* dir = obj2->GetDirectory();
	obj->Copy(*obj2);
	obj->Delete();
	obj2->SetDirectory(dir);
	// cout << "readroot2: " << obj2 << " " << obj2->GetName() << " "
	//      << ((TH1*) obj2)->Integral() << endl;
      }
    }
      //}

  }

  //opt.Read("Toptions");

  tf->Close();

  strcpy(mainname,name);
  if (myM) {
    myM->SetTitle((char*)name);
    //daqpar->AllEnabled(false);
  }

  crs->chan_changed = true;

  //strcpy(maintitle,pr_name);
  //strcat(maintitle," ");
  //strcat(maintitle,name);

  //cout << opt.channels[0] << endl;
  return 0;
}

void clear_hist() {

  //memset(f_long_t,0,sizeof(f_long_t));

  TIter next(gDirectory->GetList());

  TH1 *h;
  //TH2F *h2;
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    if (obj->InheritsFrom(TH1::Class())) {
      h=(TH1*) obj;
      h->Reset();
    }
  }

}

int readpar_root(const char* pname, int ropt)
{
  TFile *f2 = new TFile(pname,"READ");
  if (!f2->IsOpen())
    return 1;

  cpar.Read("Coptions");
  if (ropt)
    opt.Read("Toptions");

  opt.raw_write=false;
  opt.dec_write=false;
  opt.root_write=false;

  f2->Close();
  delete f2;
  return 0;
}

/*
  void savepar_root(const char* pname)
  {

  #ifdef LINUX
  if (chdir(startdir)) {}
  #else
  _chdir(startdir);
  #endif

  TFile *f2 = new TFile(pname,"RECREATE");

  cpar.Write();
  opt.Write();

  f2->Close();
  delete f2;

  }
*/

void swap_bytes(unsigned short* buf)
{
  //int i,j;
  unsigned char *buf1;
  unsigned char tmp;

  buf1 = (unsigned char*) buf;

  tmp = *(buf1);
  *(buf1) = *(buf1+1);
  *(buf1+1) = tmp;
}

short int bits(int n, int i1, int i2)
{
  int ll=i2-i1+1;
  unsigned int mask = ((1 << ll)-1);
  //unsigned int mask = (0xFFFF << (32-i2)) >> (32-i2);
  short int r = (n>>i1) & mask;
  //printf("%x %d %d %d %x %d\n",n,i1,i2,ll,mask,r);
  //r = r >> i1;
  return r;
}

Bool_t getbit(int n, int bit) {
  return n & (1<<bit);
}
void setbit(int &n, int bit, int set) {
  if (set) {
    n |= 1<<bit;
  }
  else {
    n = n & ~(1<<bit);
  }
}

string numstr(string inpstr) { //convert str to string containing only digits
  string res;
  for (UInt_t i=0;i<inpstr.size();i++) {
    if (isdigit(inpstr[i]))
      res+=inpstr[i];
  }
  if (!res.size())
    res="-1";
  return res;
}
/*
//----- MFileDialog ----------------

//MFileDialog::MFileDialog(int &fType, const TGWindow* p, const TGWindow* main, EFileDialogMode dlg_type, TGFileInfo* file_info):TGFileDialog(p,main,dlg_type,file_info) {
MFileDialog::MFileDialog(const TGWindow* p, const TGWindow* main, EFileDialogMode dlg_type, TGFileInfo* file_info):TGFileDialog(p,main,dlg_type,file_info) {
cout << "finside: " << p << " " << main << " " << dlg_type << " " << file_info << endl;
}
*/

bool TestFile() {

  string dir, name, ext;

  SplitFilename(string(opt.Filename),dir,name,ext);

  if (crs->batch) {
    //SplitFilename(string(crs->Fname),dir,name,ext);
    dir = TString(startdir);
    //cout << "Root_dir: " << dir << endl;

    crs->decname=dir;
    crs->rootname=dir;
    crs->rawname=dir;
    crs->decname.append("Dec/");
    crs->rootname.append("Root/");
    crs->rawname.append("Raw/");

    opt.dec_write=b_dec;
    opt.root_write=b_root;
    if (b_raw) {
      opt.raw_write=true;
      opt.fProc=true;
    }

    if (b_dec) {
#ifdef LINUX
      mkdir(crs->decname.c_str(),0755);
#else
      _mkdir(crs->decname.c_str());
#endif
    }

    if (b_root) {
#ifdef LINUX
      mkdir(crs->rootname.c_str(),0755);
#else
      _mkdir(crs->rootname.c_str());
#endif
    }

    if (b_raw) {
#ifdef LINUX
      mkdir(crs->rawname.c_str(),0755);
#else
      _mkdir(crs->rawname.c_str());
#endif
    }

    crs->decname.append(name);
    crs->rootname.append(name);
    crs->rawname.append(name);
  } //batch
  else { //not batch
    //SplitFilename(string(opt.Filename),dir,name,ext);
    dir.append(name);
    crs->rawname=dir;
    crs->decname=dir;
    crs->rootname=dir;
  }

  crs->decname.append(".dec");
  crs->rootname.append(".root");
  crs->rawname.append(".raw");

  if (!crs->juststarted) return true;

  bool b1 = opt.raw_write && !stat(crs->rawname.c_str(), &statb);
  bool b2 = opt.dec_write && !stat(crs->decname.c_str(), &statb);
  bool b3 = opt.root_write && !stat(crs->rootname.c_str(), &statb);

  bool b_ident=false;
  if (crs->Fmode==2) {//file analysis - test for identical filename
    bool c1=b1 && !crs->rawname.compare(crs->Fname);
    bool c2=b2 && !crs->decname.compare(crs->Fname);
    bool c3=b3 && !crs->rootname.compare(crs->Fname);
    b_ident=c1||c2||c3;
  }

  if (b1 || b2 || b3) {

    if (crs->batch) {
      if (!b_force) {
	cout << "flags: " << opt.raw_write << opt.dec_write << opt.root_write << endl;
	cout << crs->rawname << endl;
	cout << "\033[1;31mFile exists. Exiting...\033[0m" << endl;
	exit(0);
      }
      else if (b_ident) {
	prnt("sss;",BRED,msg_ident,RST);
	exit(0);
      }
      else {
	return true;
      }
    }

    Int_t retval;
    //new ColorMsgBox(gClient->GetRoot(), myM,
    if (b_ident) {
      new TGMsgBox(gClient->GetRoot(), myM,
		   "File exists",
		   msg_ident, kMBIconAsterisk, kMBCancel, &retval);
      retval=kMBCancel;
    }
    else {
      new TGMsgBox(gClient->GetRoot(), myM,
		   "File exists",
		   msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    }

    if (retval == kMBOk) {
      return true;
    }
    else {
      return false;
    }
  } //at least one file exists
  else { //no existing files
    return true;
  }

}

//---------------------------------
int CheckMem(bool pr) {
  //returns fraction of memory (in ppm) used by the program
  /*
  //returns true if memory is too low
  //t=50 (default) - check 50%
  //otherwise (t=70) - check 70%
  */

  gSystem->GetMemInfo(&minfo);
  gSystem->GetProcInfo(&pinfo);
  int rmem=pinfo.fMemResident/minfo.fMemTotal; // fraction of memory used by romana in %
  if (pr) {
    cout << "CheckMem: " << pinfo.fMemResident << " " << minfo.fMemTotal
	 << " " << rmem << " " << (1-rmem)*minfo.fMemTotal
	 << endl;
  }  return rmem;
  /*
    if (rmem>t) {
    return true;
    }
    else {
    return false;
    }
  */
  /*
    if (t==0) {
    if (minfo.fMemTotal<4000) {
    if (rmem>0.5)
    return true;
    }
    else if ((1-rmem)*minfo.fMemTotal<4000)
    return true;
    }
    else { //if (t==1) {
    if (minfo.fMemTotal<4000) {
    if (rmem>0.7)
    return true;
    }
    else if ((1-rmem)*minfo.fMemTotal<1500)
    return true;		
    }
    return false;
  */

}
//---------------------------------
TTimeStamp prtime(const char* txt, int set, const char* col) {
  static TTimeStamp tt1,tt2,tt0;
  if (set<0) {
    tt0.Set();
    tt1 = tt0;
    tt2 = tt0;
  }
  tt1=tt2;
  tt2.Set();
  if (set!=0) {
    double dt0 = tt2.AsDouble()-tt0.AsDouble(),
      dt1 = tt2.AsDouble()-tt1.AsDouble();
    prnt("ss   f   fs;",col,txt,dt0,dt1,RST);
  }
  //printf("%s: %8.3f\n",txt, tt2.AsDouble()-tt0.AsDouble());

  // printf("%s: %9d %8.3f\n",txt,int((tt2.AsDouble()-tt1.AsDouble())*1e6),
  // 	   tt2.AsDouble()-tt0.AsDouble());
  // cout << txt << " " << tt2.AsDouble()-tt1.AsDouble() << " "
  //      << tt2.AsDouble()-tt0.AsDouble() << endl;
  if (set>0 && debug==99)
    gSystem->Sleep(set);
  return tt2;
}

//----- GlbClass ----------------

GlbClass::GlbClass() {
  SetName("GLB");
  g_opt= &opt;

  g_cpar = &cpar;
  g_crs = crs;
  g_myM = myM;
  g_EvtFrm = EvtFrm;
  g_HiFrm = HiFrm;
  g_ErrFrm = ErrFrm;
  g_hcl = hcl;

}


//----- MainFrame ----------------

MainFrame::MainFrame(const TGWindow *p,UInt_t w,UInt_t h)
  : TGMainFrame(p,w,h) {
  // Create a main frame

  fTimer = new TTimer(opt.tsleep);
  fTimer->Connect("Timeout()", "MainFrame", this, "UpdateTimer()");

  gClient->GetColorByName("white", fWhite);
  fGrey=gROOT->GetColor(18)->GetPixel();

  gClient->GetColorByName("yellow", fYellow);
  gClient->GetColorByName("green", fGreen);
  //gClient->GetColorByName("kGreen", fGreen2);
  fGreen2 = gROOT->GetColor(kSpring+1)->GetPixel();
  //cout << "gr: " << fGreen2 << endl;
  gClient->GetColorByName("red", fRed);
  gClient->GetColorByName("cyan", fCyan);
  gClient->GetColorByName("magenta", fMagenta);
  //gClient->GetColorByName("BlueViolet",fBluevio);
  fOrng = TColor::RGB2Pixel(255,114,86);
  fBlue = TColor::RGB2Pixel(135,92,231);
  fRed10=gROOT->GetColor(kPink-9)->GetPixel();

  Pixel_t fOrng2 = TColor::RGB2Pixel(230,139,70);

  fCol[0] = fYellow; // 1
  fCol[1] = fGreen;  // 2
  fCol[2] = fRed;    // 3
  fCol[3] = fCyan;   // 4
  fCol[4] = fOrng;   // 5
  fCol[5] = fBlue;   // 6
  fCol[6] = fRed10;  // 7
  //cout << "fCol1: " << gROOT->GetColor(kWhite)->GetPixel() << endl;

  // Float_t rr[3];
  // for (int i=0;i<MAX_CH+NGRP;i++) {
  //   for (int j=0;j<3;j++) {
  //     rr[j]=EF::RGB[i%32][j];
  //   }
  //   chcol[i]=TColor::GetColor(TColor::RGB2Pixel(rr[0],rr[1],rr[2]));
  //   gcol[i]=gROOT->GetColor(chcol[i])->GetPixel();
  // }

  TGLayoutHints* LayCT1 = new TGLayoutHints(kLHintsCenterX|kLHintsTop,1,1,5,2);
  TGLayoutHints* LayE1 = new TGLayoutHints(kLHintsExpandX,1,1,0,0);
  TGLayoutHints* LayET0  = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,0);
  TGLayoutHints* LayET1 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,5,5);
  TGLayoutHints* LayET1a = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,5,0);
  TGLayoutHints* LayET1b = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,0,5);
  TGLayoutHints* LayET2=new TGLayoutHints(kLHintsExpandX|kLHintsTop,15,15,2,2);
  TGLayoutHints* LayET3 = new TGLayoutHints(kLHintsExpandX|kLHintsTop,0,0,5,15);
  //TGLayoutHints* LayLT3 = new TGLayoutHints(kLHintsLeft|kLHintsTop,1,1,1,1);
  TGLayoutHints* LayL1 = new TGLayoutHints(kLHintsLeft,1,1,0,0);
  LayEE1 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1);
  LayEE2 = new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,3,3,3,3);

  //cout << "gStyle: " << gStyle << endl;
  gStyle->SetOptStat(kFALSE);
  gStyle->SetPalette(1,0);
  gStyle->SetTitleFontSize(0.07);
  gStyle->SetTitleSize(0.05,"xyz");
  //gStyle->SetTitleOffset(0.95,"xy"); 
  gStyle->SetTitleOffset(0.95,"x"); 
  gStyle->SetTitleOffset(1.2,"yz"); 

  gStyle->SetLabelSize(0.05,"xyz");
  gStyle->SetNdivisions(606,"xyz");
  gStyle->SetPadLeftMargin(0.13);
  gStyle->SetPadRightMargin(0.05);
  //gStyle->SetPadBottomMargin(0.15);
  //gStyle->SetPadTopMargin(0.05);

  /*
    fDNDTypeList = new Atom_t[3];
    fDNDTypeList[0] = gVirtualX->InternAtom("application/root", kFALSE);
    fDNDTypeList[1] = gVirtualX->InternAtom("text/uri-list", kFALSE);
    fDNDTypeList[2] = 0;
    if (gDNDManager) delete gDNDManager;
    gDNDManager = new TGDNDManager(this, fDNDTypeList);
  */


  //int nn=2;
  //double xx[nn];
  //double yy[nn];

  //fEv=NULL;

  //bRun = false;

  Connect("CloseWindow()", "MainFrame", this, "CloseWindow()");

  fMenuBar = new TGMenuBar(this, 35, 50, kHorizontalFrame);

  TGPopupMenu* fMenuFile = new TGPopupMenu(gClient->GetRoot());

  fMenuBar->AddPopup("&File", fMenuFile, 
		     new TGLayoutHints(kLHintsLeft|kLHintsTop,0,4,0,0));

  fMenuFile->AddEntry("Read Parameters", M_READINIT);
  fMenuFile->AddEntry("Save Parameters", M_SAVEINIT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Read ROOT file", M_READROOT);
  fMenuFile->AddEntry("Save ROOT file", M_SAVEROOT);
  fMenuFile->AddSeparator();
  //fMenuFile->AddEntry("Save ASCII file", M_SAVEASCII);
  //fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Export...", M_EXPORT);
  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("Browser\tCtrl+B", M_FILE_BROWSE);
  fMenuFile->AddEntry("Reset USB", M_RESET_USB);
  //fMenuFile->AddEntry("New Canvas\tCtrl+N", M_FILE_NEWCANVAS);

  //fMenuFile->AddEntry("&Open...", M_FILE_OPEN);
  //fMenuFile->AddEntry("&Save", M_FILE_SAVE);

  fMenuFile->AddSeparator();
  fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

  //fMenuFile->AddPopup("&Cascaded menus", fCascadeMenu);
  fMenuFile->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  TGPopupMenu* fMenuDet = new TGPopupMenu(gClient->GetRoot());

  fMenuBar->AddPopup("&Detectors", fMenuDet, 
		     new TGLayoutHints(kLHintsLeft|kLHintsTop,0,4,0,0));

  fMenuDet->AddEntry("Profilometer 8x8", M_EDIT_PROF8);
  fMenuDet->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  fMenuDet->AddEntry("Profilometer 64x64", M_EDIT_PROF64);
  fMenuDet->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  // fMenuProf->AddEntry("Profilometer time calibration", M_PROF_TIME);
  // fMenuProf->Connect("Activated(Int_t)", "MainFrame", this,
  // 		     "HandleMenu(Int_t)");


  TGPopupMenu* fMenuAnalysis = new TGPopupMenu(gClient->GetRoot());

  fMenuBar->AddPopup("&Analysis", fMenuAnalysis, 
		     new TGLayoutHints(kLHintsLeft|kLHintsTop,0,4,0,0));

  // fMenuAnalysis->AddEntry("Energy Pre-calibration", M_PRECALIBR);
  // fMenuAnalysis->Connect("Activated(Int_t)", "MainFrame", this,
  // 		     "HandleMenu(Int_t)");
  fMenuAnalysis->AddEntry("Energy calibration", M_ECALIBR);
  // fMenuAnalysis->Connect("Activated(Int_t)", "MainFrame", this,
  // 		     "HandleMenu(Int_t)");
  fMenuAnalysis->AddEntry("Time calibration", M_TCALIBR);
  // fMenuAnalysis->Connect("Activated(Int_t)", "MainFrame", this,
  // 		     "HandleMenu(Int_t)");
  fMenuAnalysis->AddEntry("Peak Search", M_PEAKS);
#ifdef P_LIBUSB
  fMenuAnalysis->AddEntry("Test", M_TEST);
#endif

  fMenuAnalysis->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");



  TGPopupMenu* fMenuHelp = new TGPopupMenu(gClient->GetRoot());
  fMenuHelp->AddEntry("Display Help file", M_HELP);
  //fMenuHelp->AddEntry("Test", M_TEST);
  fMenuHelp->Connect("Activated(Int_t)", "MainFrame", this,
		     "HandleMenu(Int_t)");

  fMenuBar->AddPopup("&Help", fMenuHelp,
		     new TGLayoutHints(kLHintsTop|kLHintsRight,0,4,0,0));

  AddFrame(fMenuBar, 
	   new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 2, 2, 5));

  //TGLabel *ver = new TGLabel(fMenuBar,GITVERSION);
  //fMenuBar->AddFrame(ver,new TGLayoutHints(kLHintsCenterY|kLHintsRight,0,4,0,0));

  //fcanvas=NULL;
  //fAna=NULL;

  //fMain = new TGMainFrame(p,w,h);

  // Create a vertical frame for everything
  TGHorizontalFrame *hframe1 = new TGHorizontalFrame(this);
  AddFrame(hframe1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  // Create a left vertical frame with buttons
  TGVerticalFrame *vframe1 = new TGVerticalFrame(hframe1);
  hframe1->AddFrame(vframe1, new TGLayoutHints(kLHintsLeft | kLHintsExpandY,1,1,2,2));

  TGFontPool *pool = gClient->GetFontPool();

  //pool->Print();

  // family , size (minus value - in pixels, positive value - in points), weight, slant
  // kFontWeightNormal, kFontSlantRoman are defined in TGFont.h
  //font = pool->GetFont("helvetica", -18, kFontWeightMedium, kFontSlantRoman);
  //const TGFont *font = pool->GetFont("helvetica", -18, kFontWeightBold, kFontSlantRoman);
  //const TGFont *font = pool->GetFont("helvetica", -18, 4, kFontSlantRoman);
  const TGFont *font = pool->GetFont("-*-helvetica-bold-r-*-*-16-*-*-*-*-*-iso8859-1",true);
  const TGFont *font14 = pool->GetFont("-*-helvetica-bold-r-*-*-14-*-*-*-*-*-iso8859-1",true);

  //cout << "Font: " << font << endl;
  //font->Print();


  if (!font)
    font = gClient->GetResourcePool()->GetDefaultFont();
  if (!font14)
    font14 = gClient->GetResourcePool()->GetDefaultFont();

  FontStruct_t tfont = font->GetFontStruct();
  FontStruct_t tfont14 = font14->GetFontStruct();


  fTab = new TGTab(hframe1, 300, 300);
  hframe1->AddFrame(fTab, new TGLayoutHints(kLHintsExpandX |
					    kLHintsExpandY,3,3,2,2));

  fTab->Connect("Selected(Int_t)", "MainFrame", this, "DoTab(Int_t)");

  MakeTabs();

  const int butx=70,buty=30;

  fGr1 = new TGGroupFrame(vframe1, "Acquisition", kVerticalFrame);
  fGr1->SetTitlePos(TGGroupFrame::kCenter); // right aligned
  vframe1->AddFrame(fGr1, LayCT1);

  fStart = new TGTextButton(fGr1,"Start");
  fStart->SetToolTipText("Start/Stop acquisition");
  fStart->SetFont(tfont,false);
  fStart->Resize(butx,buty);
  fStart->ChangeOptions(fStart->GetOptions() | kFixedSize);
  fStart->ChangeBackground(fGreen);
  fStart->Connect("Clicked()","MainFrame",this,"DoStartStop(=1)");
  fGr1->AddFrame(fStart, LayET1);

  fContinue = new TGTextButton(fGr1,"Continue");
  fContinue->SetToolTipText("Continue/Pause acquisition");
  fContinue->SetFont(tfont14,false);
  fContinue->Resize(butx,buty);
  fContinue->ChangeOptions(fContinue->GetOptions() | kFixedSize);
  fContinue->ChangeBackground(fGreen2);
  fContinue->Connect("Clicked()","MainFrame",this,"DoStartStop(=0)");
  fGr1->AddFrame(fContinue, LayET1);

  /*
  fContinue = new TGTextButton(fGr1,"Clear");
  fContinue->SetToolTipText("Clear histograms (works during acquisition/analysis)");
  fContinue->SetFont(tfont14,false);
  fContinue->Resize(butx,buty);
  fContinue->ChangeOptions(fContinue->GetOptions() | kFixedSize);
  fContinue->ChangeBackground(fCyan);
  fContinue->Connect("Clicked()","HistFrame",HiFrm,"DoClear()");
  fGr1->AddFrame(fContinue, LayET1);
  */

  fGr2 = new TGGroupFrame(vframe1, "Analysis", kVerticalFrame);
  fGr2->SetTitlePos(TGGroupFrame::kCenter);
  vframe1->AddFrame(fGr2, LayCT1);

  TGTextButton *fOpen = new TGTextButton(fGr2,new TGHotString("Open +"));
  fOpen->SetFont(tfont,false);
  fOpen->SetToolTipText("Open data file with parameters");
  fOpen->Resize(butx,buty*2/3);
  fOpen->ChangeOptions(fOpen->GetOptions() | kFixedSize);
  fOpen->ChangeBackground(fOrng);
  fOpen->Connect("Clicked()","MainFrame",this,"DoOpen(=1)");
  fGr2->AddFrame(fOpen, LayET1a);

  TGTextButton *fOpen2 = new TGTextButton(fGr2,new TGHotString("Open -"));
  fOpen2->SetFont(tfont,false);
  fOpen2->SetToolTipText("Open data file without parameters");
  fOpen2->Resize(butx,buty*2/3);
  fOpen2->ChangeOptions(fOpen2->GetOptions() | kFixedSize);
  fOpen2->ChangeBackground(fOrng2);
  fOpen2->Connect("Clicked()","MainFrame",this,"DoOpen(=0)");
  fGr2->AddFrame(fOpen2, LayET1b);

  TGTextButton *fClose = new TGTextButton(fGr2,new TGHotString("&Close"));
  fClose->SetToolTipText("Close data file");
  fClose->SetFont(tfont,false);
  fClose->Resize(butx,buty);
  fClose->ChangeOptions(fClose->GetOptions() | kFixedSize);
  fClose->ChangeBackground(fBlue);
  fClose->Connect("Clicked()","MainFrame",this,"DoClose()");
  fGr2->AddFrame(fClose, LayET1);

  fAna = new TGTextButton(fGr2,"&Analyze");
  fAna->SetToolTipText("Analyze data file");
  fAna->SetFont(tfont,false);
  fAna->Resize(butx,buty);
  fAna->ChangeOptions(fAna->GetOptions() | kFixedSize);
  fAna->ChangeBackground(fGreen);
  fAna->Connect("Clicked()","MainFrame",this,"DoAna()");
  fGr2->AddFrame(fAna, LayET1);

  TGTextButton* f1b = new TGTextButton(fGr2,new TGHotString("&1 buf"));
  f1b->SetToolTipText("Analyze one buffer");
  f1b->SetFont(tfont,false);
  f1b->Resize(butx,buty);
  f1b->ChangeOptions(f1b->GetOptions() | kFixedSize);
  f1b->ChangeBackground(fGreen);
  f1b->Connect("Clicked()","MainFrame",this,"Do1buf()");
  fGr2->AddFrame(f1b, LayET1);

  int id = parpar->Plist.size()+1;
  TGNumberEntry* fNum1 = new TGNumberEntry(fGr2, 0, 0, id,
					   TGNumberFormat::kNESInteger,
					   TGNumberFormat::kNEAAnyNumber,
					   TGNumberFormat::kNELLimitMinMax,
					   1,(Long64_t(1)<<31)-1);
  // 					   1,100000);
  //opt.num_buf=(Long64_t(1)<<31)-1;
  parpar->DoMap(fNum1->GetNumberEntry(),&opt.num_buf,p_inum,0);
  fNum1->GetNumberEntry()->SetToolTipText("Number of buffers to analyze");
  //fNum1->Resize(butx,buty);
  //fNum1->Resize(65, fNum1->GetDefaultHeight());
  fNum1->GetNumberEntry()->Connect("TextChanged(char*)", "ParDlg", parpar,
				   "DoNum()");
  fGr2->AddFrame(fNum1,LayET1a);

  fNb = new TGTextButton(fGr2,new TGHotString("&N buf"));
  fNb->SetToolTipText("Analyze N buffers");
  fNb->SetFont(tfont,false);
  fNb->Resize(butx,buty);
  //fNb->Resize(35,122);
  fNb->ChangeOptions(fNb->GetOptions() | kFixedSize);
  fNb->ChangeBackground(fGreen);
  fNb->Connect("Clicked()","MainFrame",this,"DoNbuf()");
  fGr2->AddFrame(fNb, LayET1b);

  // TGTextButton *fReset2 = new TGTextButton(fGr2,new TGHotString("&Reset"));
  // fGr2->AddFrame(fReset2, LayET1);

  fReset2 = new TGTextButton(vframe1,new TGHotString("&Reset"));
  vframe1->AddFrame(fReset2, LayET2);

  fReset2->SetToolTipText("Reset/clear everything (doesn't work during acquisition/analysis)");
  fReset2->SetFont(tfont,false);
  fReset2->Resize(butx,buty);
  fReset2->ChangeOptions(fReset2->GetOptions() | kFixedSize);
  fReset2->ChangeBackground(fCyan);
  fReset2->Connect("Clicked()","MainFrame",this,"DoReset()");
  //fReset2->Connect("Clicked()","CRS",crs,"Reset()");

  TGHorizontal3DLine* v3d = new TGHorizontal3DLine(vframe1);
  vframe1->AddFrame(v3d,LayET3);

  //TGHorizontalFrame *hfr1 = new TGHorizontalFrame(fGr2);
  //fGr2->AddFrame(hfr1, LayET1);


  TGLabel *ver = new TGLabel(vframe1,GITVERSION);
  //cout << "gitversion: " << GITVERSION << " " << strlen(GITVERSION) << endl;
	
  vframe1->AddFrame(ver,new TGLayoutHints(kLHintsBottom|kLHintsCenterX,0,0,0,4));

  if (rd_root) {
    if (readroot(datfname))
      exit(-1);
    crs->Fmode=0;
    //SetTitle(datfname);
  }

  if (crs->Fmode!=1) { //no CRS present
    //daqpar->AllEnabled(false);
    EnableBut(fGr1,0);

    //fStart->SetEnabled(false);
    //fContinue->SetEnabled(false);
  }

  //EnableBut(fGr1,1);
  //EnableBut(fGr2,1);


  parpar->Update();

  //HiFrm->DoReset();
  //HiFrm->Update();

  //HiFrm->Update();
  fTab->SetTab(opt.seltab);

  tfont=gClient->GetResourcePool()->GetStatusFont()->GetFontStruct();

  TGHorizontalFrame *fStatFrame1 = new TGHorizontalFrame(this,10,10);
  TGHorizontalFrame *fStatFrame2 = new TGHorizontalFrame(this,10,10);
  AddFrame(fStatFrame1, LayET0);
  AddFrame(fStatFrame2, LayET0);

  const int fwid=120;
  const int fwid2=42;

  const char* txtlab[n_stat] = {"Start","AcqTime","Events","Ev/sec","MTrig","MTrig/sec","Buffers","MB in","MB/sec","Raw MB out","Dec MB out","CPU%:","Mem%:"};

  const char* st_tip[n_stat] = {
    "Acquisition start",
    //"Total Acquisition Time",
    "Acquisition Time",
    "Total number of events received",
    "Event rate received (in Hz)",
    "Total number of main trigger events",
    "Main trigger event rate (in Hz)",
    "Number of buffers received",
    "Megabytes received",
    "Received megabytes per second",
    "Raw megabytes saved",
    "Decoded megabytes saved",
    "CPU used by the program",
    "Memory used by the program"
  };

  TGTextEntry* fLab[n_stat];
  TGHorizontalFrame *hfr1=0;

  //TGTextEntry* fStat[10];
  for (int i=0;i<n_stat;i++) {
    if (i<n_stat2) {
      fLab[i] = new TGTextEntry(fStatFrame1, txtlab[i]);
      fStat[i] = new TGTextEntry(fStatFrame2, " ");
    }
    else {
      hfr1 = new TGHorizontalFrame(vframe1);
      vframe1->AddFrame(hfr1,LayET0);
      fLab[i] = new TGTextEntry(hfr1, txtlab[i]);
      fStat[i] = new TGTextEntry(hfr1, " ");
    }
    if (tfont) {
      fLab[i]->SetFont(tfont,false);
      fStat[i]->SetFont(tfont,false);
    }
    fLab[i]->SetHeight(18);
    fLab[i]->SetState(false);
    fLab[i]->ChangeOptions(fLab[i]->GetOptions()|kSunkenFrame|kFixedHeight);
    fLab[i]->SetToolTipText(st_tip[i]);

    fStat[i]->SetHeight(18);
    fStat[i]->SetState(false);
    fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kSunkenFrame|kFixedHeight);
    fStat[i]->SetToolTipText(st_tip[i]);


    if (i==0) {
      fLab[i]->SetWidth(fwid);
      fLab[i]->ChangeOptions(fLab[i]->GetOptions()|kFixedWidth);
      fStatFrame1->AddFrame(fLab[i],LayL1);
      fStat[i]->SetWidth(fwid);
      fStat[i]->ChangeOptions(fStat[i]->GetOptions()|kFixedWidth);
      fStatFrame2->AddFrame(fStat[i],LayL1);
    }
    else if (i>=n_stat2) {
      fLab[i]->SetWidth(fwid2);
      fLab[i]->ChangeOptions(fLab[i]->GetOptions()|kFixedWidth);
      hfr1->AddFrame(fLab[i],LayL1);
      hfr1->AddFrame(fStat[i],LayE1);
    }
    else {
      fStatFrame1->AddFrame(fLab[i],LayE1);
      fStatFrame2->AddFrame(fStat[i],LayE1);
    }

  }
	
  //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  

  UpdateTimer(1);
  //UpdateStatus(1);

  // Set a name to the main frame
  SetTitle(mainname);

  //Rebuild();
  // Map all subwindows of main frame

  MapSubwindows();
  // Initialize the layout algorithm
  //Rebuild();
  Resize(GetDefaultSize());
  // Map main frame
  MapWindow();

  //cout << "Init end" << endl;
  //Rebuild();

  Move(-100,-100);

  fTimer->TurnOn();
  //p_ed=0;
  //p_pop=0;
}

MainFrame::~MainFrame() {
  delete fTimer;
}

void MainFrame::Rebuild() {

  std::vector<TGCompositeFrame*>::iterator it;
  for (it=tabfr.begin();it!=tabfr.end();++it) {
    (*it)->RemoveAll();
    (*it)->Delete();
    fTab->RemoveTab();
  }
  tabfr.clear();

  MakeTabs(true);

  Resize(GetDefaultSize());
  MapSubwindows();
  Layout();

  // cout << "main::Rebuild3: " << endl;
}

void MainFrame::MakeTabs(bool reb) {

  int ntab=0;

  TGCompositeFrame* tb;

  tb = fTab->AddTab("Parameters");
  tabfr.push_back(tb);
  parpar = new ParParDlg(tb, 1, MAIN_HEIGHT-5); //-5 ->иначе появляется слайдер
  tb->AddFrame(parpar, LayEE1);
  parpar->Update();
  ntab++;

  //chanpar;
  //YKWheel
  tb = fTab->AddTab("Channels");
  tabfr.push_back(tb);
  chanpar = new ChanParDlg(tb, 1, MAIN_HEIGHT);
  tb->AddFrame(chanpar, LayEE2);
  ntab++;
  //YKWheel

  //cout << "tab3: " << ntab << endl;

  tb = fTab->AddTab("Events");
  tabfr.push_back(tb);
  EvtFrm = new EventFrame(tb, MAIN_WIDTH, MAIN_HEIGHT,ntab);
  tb->AddFrame(EvtFrm, LayEE1);
  ntab++;
  //cout << "tab2: " << ntab << endl;

  tb = fTab->AddTab("Histograms");
  tabfr.push_back(tb);
  //histpar = new HistParDlg(tb, 600, MAIN_HEIGHT);
  histpar = new HistParDlg(tb, 400, MAIN_HEIGHT);
  tb->AddFrame(histpar, LayEE1);
  ntab++;
  //cout << "tab3: " << ntab << endl;

  tb = fTab->AddTab("Plots");
  tabfr.push_back(tb);
  HiFrm = new HistFrame(tb, 1, MAIN_HEIGHT,ntab);
  tb->AddFrame(HiFrm, LayEE1);
  ntab++;
  //cout << "tab4: " << ntab << endl;

  tb = fTab->AddTab("Errors");
  tabfr.push_back(tb);
  ErrFrm = new ErrFrame(tb, 250, MAIN_HEIGHT);
  tb->AddFrame(ErrFrm, LayEE1);
  ntab++;
  //cout << "tab5: " << ntab << endl;


  // prtime("tab01",1,BGRN);
  // TThread *tr1 = new TThread("tr1", run_build, (void*) 1);
  // tr1->Run();

  // prtime("tab02",1,BGRN);
  // TThread *tr2 = new TThread("tr2", run_build, (void*) 2);
  // tr2->Run();

  // prtime("tab03",1,BGRN);
  // TThread *tr3 = new TThread("tr3", run_build, (void*) 3);
  // tr3->Run();


  chanpar->Build();
  chanpar->Update();

  //prtime("tab04",1,BGRN);
  histpar->Update();

  //prtime("tab05",1,BGRN);
  HiFrm->HiReset();

  //prtime("tab06",1,BGRN);
  local_nch=opt.Nchan;
  local_nrows=opt.Nrows;

  // tr1->Join();
  // tr2->Join();
  // tr3->Join();
  // tr1->Delete();
  // tr2->Delete();
  // tr3->Delete();
  //TThread::Ps();

  //parpar->DaqDisable();

  // MapSubwindows();
  // Resize(GetDefaultSize());
  // MapWindow();
  //cout << "tabs11: " << endl;
}

void MainFrame::SetTitle(char* fname) {
  strcpy(maintitle,pr_name);
  strcat(maintitle," ");
  strcat(maintitle,fname);
  SetWindowName(maintitle);
}

void MainFrame::EnableBut(TGGroupFrame* fGr, bool enbl) {
  //cout << "Gr: " << fGr->GetName() << endl;
  TIter next(fGr->GetList());
  TObject* obj;
  while ( (obj=(TObject*)next()) ) {
    //TGFrameElement* el = (TGFrameElement*) obj;
    //TGTextButton* but = (TGTextButton*) el->fFrame;

    TGTextButton* but = (TGTextButton*) ((TGFrameElement*) obj)->fFrame;

    //cout << "but: " << but->GetName() << " " << but->ClassName() << endl;
    //cout << "but2: " << but->InheritsFrom(TGButton::Class()) << endl;

    if (but->InheritsFrom(TGButton::Class())) {
      but->SetEnabled(enbl);
    }
    else {
      TGNumberEntry* ent = (TGNumberEntry*) but;
      ent->SetState(enbl);
    }
  }  
  
}

void MainFrame::DoStartStop(int rst) {
  //rst=1 - start/stop is pressed
  //rst=0 - continue/pause is pressed

  //cout << "Dostartstop1: " << gROOT->FindObject("Start") << endl;

#ifdef CYUSB
  if (crs->b_acq) { //STOP or PAUSE is pressed here
    if (!crs->batch) {
      fStart->ChangeBackground(fGreen);
      fStart->SetText("Start");
      fContinue->SetEnabled(true);

      fReset2->SetEnabled(true);
      EnableBut(fGr2,1);
      //fContinue->ChangeBackground(fGreen2);
      //fContinue->SetText("Continue");
    }
    //crs->b_stop=false;
    //crs->Show();
    crs->DoStartStop(rst);

    if (opt.root_write || b_root) {
      saveroot(crs->rootname.c_str());
    }

  }
  else { // START is pressed here
    if (TestFile()) {
      //ParLock();
      fStart->ChangeBackground(fRed);
      fStart->SetText("Stop");
      fContinue->SetEnabled(false);

      fReset2->SetEnabled(false);
      EnableBut(fGr2,0);
      //TList* l2 = fGr2->GetList();

      
      //fContinue->ChangeBackground(fRed);
      //fContinue->SetToolTipText("Stop acquisition");
      //fContinue->SetText("Stop");

      crs->DoStartStop(rst);
      //cout << "Start7: " << endl;


      fStart->ChangeBackground(fGreen);
      fStart->SetText("Start");
      fContinue->SetEnabled(true);

      fReset2->SetEnabled(true);
      EnableBut(fGr2,1);
      //fContinue->ChangeBackground(fGreen2);
      //fContinue->SetText("Continue");

      crs->b_stop=true;
      crs->b_fana=false;
      crs->b_acq=false;
      crs->b_run=0;

      if (opt.root_write) {
	saveroot(crs->rootname.c_str());
      }

    }
    //crs->b_stop=true;
  }
#endif

  //cout << "Dostartstop2: " << endl;
}

void MainFrame::DoOpen(Int_t popt) {

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "all files",     "*",
    "adcm raw or deecoded files",     "*.dat",
    "crs raw files",     "*.raw",
    "crs dec files",     "*.dec",
    "root files",        "*.root",
    //"crs32 files",     "*.32gz",
    0,               0
  };

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {

    string dir, name, ext2;
    SplitFilename(string(fi.fFilename),dir,name,ext2);
    TString ext(ext2);
    ext.ToLower();

    if (ext.EqualTo(".root")) {
      readpar_root(fi.fFilename,popt);
      DoReset();

      readroot(fi.fFilename);
      HiFrm->HiUpdate();
    }
    else {
      crs->DoFopen(fi.fFilename,1,popt); //1 - read cpar; popt - read toptions
    }

    parpar->Update();
    chanpar->Update();

  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::DoClose() {

  if (!crs->b_stop) return;

  if (crs->f_read) {
    gzclose(crs->f_read);
    crs->f_read=0;
  }

  myM->SetTitle((char*)"");
  //daqpar->AllEnabled(true);

  parpar->Update();
  chanpar->Update();

#ifdef CYUSB
  crs->Detect_device();
  if (crs->Fmode==1) { //CRS is present
    fStart->SetEnabled(true);
    fContinue->SetEnabled(true);
    //fReset->SetEnabled(true);

    //opt.raw_write=false;
    //parpar->Update();
    /*
      TGCheckButton *te = (TGCheckButton*) parpar->FindWidget(&opt.raw_write);
      if (te) 
      te->SetEnabled(true);
    */
  }
#endif

}

void MainFrame::DoAna() {

  
  //cout << "DoAna: " << gROOT->FindObject("Start") << endl;
  //cout << "Pmapsz: " << sizeof(Pmap) << " " << chanpar->Plist.size() << endl;


  if (crs->Fmode<2) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    crs->b_fana=false;
    crs->b_stop=true;
    gSystem->Sleep(100);
    if (!crs->batch) {
      //fAna->ChangeBackground(fGreen);
      fAna->ChangeText("&Analyze");
      //fAna->SetText(hAna);
    }

    if (opt.root_write) {
      saveroot(crs->rootname.c_str());
    }

  }
  else { //start analysis
    if (TestFile()) {
      //cout << "hAna: " << hAna->GetString() << " " << hPause->GetString() << endl;
      if (!crs->batch) {
	//fAna->SetText(new TGHotString("&Pause"));
	fAna->ChangeText("P&ause");
	fAna->ChangeBackground(fRed);
      }
      crs->b_fana=true;
      crs->b_stop=false;

      crs->FAnalyze2(true);

      if (!crs->batch) {
	//fAna->SetText(new TGHotString("&Analyze"));
	fAna->ChangeText("&Analyze");
	fAna->ChangeBackground(fGreen);
      }
      crs->b_fana=false;
      crs->b_stop=true;

      if (opt.root_write) {
	saveroot(crs->rootname.c_str());
      }

    }
  }

  //cout << "mainframe::doana: " << endl;
	
  //crs->DoFAna();
}

void MainFrame::Do1buf() {

  //cout << "DoNbuf" << endl;

  if (crs->Fmode<2) {
    cout << "File not open" << endl;
    return;
  }

  if (TestFile()) {
    crs->b_fana=true;
    crs->b_stop=false;
    crs->DoNBuf2(1);
    crs->b_fana=false;
    crs->b_stop=true;
  }  

}

void MainFrame::DoNbuf() {

  //cout << "DoNbuf" << endl;

  if (crs->Fmode<2) {
    cout << "File not open" << endl;
    return;
  }

  if (crs->b_fana) { //analysis is running -> stop it
    fAna->ChangeBackground(fGreen);
    fAna->SetText("&Analyse");
    fNb->ChangeBackground(fGreen);
    gSystem->Sleep(100);
    crs->b_fana=false;
    crs->b_stop=true;
  }
  else { //start analysis of n buffers
    if (TestFile()) {
      fAna->ChangeBackground(fRed);
      fAna->SetText("P&ause");
      fNb->ChangeBackground(fRed);
      crs->b_fana=true;
      crs->b_stop=false;
      crs->DoNBuf2(opt.num_buf);
      fAna->ChangeBackground(fGreen);
      fAna->SetText("&Analyse");
      fNb->ChangeBackground(fGreen);
      crs->b_fana=false;
      crs->b_stop=true;
    }
  }

}

void MainFrame::DoRWinit(EFileDialogMode nn) {

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "par files",     "*.par",
    "All files",     "*",
    0,               0
  };

  char pname[200];

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, nn, &fi);

  if (fi.fFilename != NULL) {
    strcpy(pname,fi.fFilename);
    printf("TGFile: %s\n",pname);

    if (nn==kFDOpen) {
      //readinit(pname);
      gzFile ff = gzopen(pname,"rb");
      if (!ff) {
	cout << "Can't open par file: " << pname << endl;
      }
      else {
	crs->ReadParGz(ff,pname,0,1,1);
	gzclose(ff);
      }

      parpar->Update();
      chanpar->Update();

    }
    else { //Save pars
      gzFile ff = gzopen(pname,"wb");
      if (ff) {
	crs->SaveParGz(ff,crs->module);
	gzclose(ff);
      }
      else {
	cout << "Can't open file: " << pname << endl;
      }

    } //else (save Pars)
    //newfile();
  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::DoReadRoot() {

  char rname[255];

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "par files",     "*.root",
    "All files",     "*",
    0,               0
  };

  static TString dir(".");
  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir);

  //printf("TGFile:\n");

  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);

  if (fi.fFilename != NULL) {

    //rootname=new char[200];

    strcpy(rname,fi.fFilename);

    readpar_root(rname,1);
    DoReset();
    //new_hist();

    readroot(rname);

    parpar->Update();
    chanpar->Update();
    HiFrm->HiUpdate();

    //nevent=opt.Nevt;
    //tof=opt.Tof;

    //fBar1->SetText(TString("Stop: ")+opt.F_stop.AsSQLString(),2);  
    UpdateTimer(1);
    //UpdateStatus(1);

    //if (fPar!=NULL) {
    //delete fPar;
    //fPar = new MParDlg(gClient->GetRoot(), fMain, "Parameters");
    //fPar->Map();
    //}
    //if (fChan!=NULL) {
    //delete fChan;
    //fChan = new MChanDlg(gClient->GetRoot(), fMain, "Channels");
    //fChan->Map();
    //}

    //DoDraw();

  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

void MainFrame::Export() {
  const char *dnd_types[] = {
    "pdf files",     "*.pdf",
    "png files",     "*.png",
    "jpg files",     "*.jpg",
    "gif files",     "*.gif",
    "all files",     "*",
    0,               0
  };

  //TString name = TString(fTab->GetCurrentTab()->GetString());
  //if (name.EqualTo("DAQ",TString::kIgnoreCase))

  TCanvas* cv=0;
  TString name = TString(fTab->GetCurrentTab()->GetString());
  if (name.EqualTo("Events",TString::kIgnoreCase)) {
    cv=EvtFrm->fCanvas->GetCanvas();
  }
  else if (name.Contains("Plots",TString::kIgnoreCase)) {
    cv=HiFrm->fEc->GetCanvas();
  }
  else {
    return;
  }

  TGFileInfo fi;
  fi.fFileTypes = dnd_types;
  //fi.fIniDir    = StrDup(".");
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);

  if (fi.fFilename) {
    cv->SaveAs(fi.fFilename);
    // Int_t retval=kMBOk;
    // if (!stat(fi.fFilename, &statb)) {
    //   new TGMsgBox(gClient->GetRoot(), this,
    // 		   "File exists",
    // 		   msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    // }

    // if (retval == kMBOk) {
    //   cout << "OK: " << fi.fFilename <<  " " << TImage::GetImageFileTypeFromFilename(fi.fFilename) << endl;
    //   //TGCompositeFrame* frame = fTab->GetCurrentContainer();
    //   //frame->SaveAs(fi.fFilename);
    //   cv->SaveAs(fi.fFilename);
    // }
  }
	
}

void MainFrame::DoReset() {

  //prtime("Main::DoReset");
	
  // if (HiFrm)
  //   cout << "DoReset_main: " << HiFrm->h_time[1]->GetName() << endl;

  //if (HiFrm)
  //cout << "DoReset_main: " << HiFrm->hlist->GetSize() << endl;

  if (!crs->b_stop) return;

  //cout << "Reset1: " << endl;
  crs->DoReset();
  //cout << "Reset2: " << endl;

  if (local_nch!=opt.Nchan || local_nrows!=opt.Nrows) {
    // cout << "Rebuild: " << endl;
    //for (int k=0;k<100000;k++) {
    //debug=99;
    Rebuild();
    //}

    //Resize(GetDefaultSize());
    //MapSubwindows();
    //MapWindow();  
    //Layout();
    local_nch=opt.Nchan;
    local_nrows=opt.Nrows;
  }

  ErrFrm->Reset();
  //if (crs->nchan_on != crs->CountChan()) {
  if (crs->chan_changed) {
    HiFrm->HiReset();
    crs->chan_changed = false;
  }
  parpar->Update();
  chanpar->Update();

  UpdateTimer(1);
  //UpdateStatus(1);

}

void MainFrame::UpdateTimer(int rst) {
  int ii=0;

  static Long64_t bytes1=0;
  static Long64_t nevents_old=0;
  static Long64_t mtrig_old=0;
  static double mb_rate,ev_rate,trig_rate;

  static clock_t clk_old, proc_old;

  struct tms timeSample;

  static Long64_t t_old=0;

  Long64_t tot;
  clock_t clk, proc;
  static double percent=0;
  static double pmem=0;

  //gSystem->GetProcInfo(&pinfo);
  
  if (rst) {
    bytes1=0;
    nevents_old=0;
    mtrig_old=0;
    mb_rate=0;
    ev_rate=0;
    trig_rate=0;

    clk_old=0;
    proc_old=0;
    percent=0;
    pmem=0;

    opt.T_acq=0;
    t_old = gSystem->Now();
  }

  Long64_t tt = gSystem->Now();
  double dt = (tt - t_old)*0.001;

  //cout << "timer: " << dt << " " << tt << " " << t_old << " " << (tt-t_old) << endl;

  t_old = tt;

  if (dt>0.1) {
    mb_rate = (crs->inputbytes-bytes1)/MB/dt;
    ev_rate = (crs->nevents-nevents_old)/dt;
    trig_rate = (crs->mtrig-mtrig_old)/dt;
    //cout << "trig_rate: " << trig_rate << " " << dt << endl;

    bytes1=crs->inputbytes;
    nevents_old=crs->nevents;
    mtrig_old=crs->mtrig;

    //cpu
    clk = times(&timeSample);
    proc = timeSample.tms_stime + timeSample.tms_utime;
    tot = clk-clk_old;
    if (tot>10) {
      percent = 100.*(proc-proc_old)/tot;
      clk_old=clk;
      proc_old=proc;
    }
    //else
    //percent = 0;

    //mem
    gSystem->GetMemInfo(&minfo);
    gSystem->GetProcInfo(&pinfo);
    pmem = pinfo.fMemResident/minfo.fMemTotal*0.1;
  }

  fStat[ii++]->SetText(crs->txt_start,0);

  fStat[ii++]->SetText(TGString::Format("%0.1f",opt.T_acq),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents),0);
  fStat[ii++]->SetText(TGString::Format("%0.3f",ev_rate),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->mtrig),0);
  fStat[ii++]->SetText(TGString::Format("%0.3f",trig_rate),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nbuffers),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->inputbytes/MB),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",mb_rate),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->rawbytes/MB),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->decbytes/MB),0);

  fStat[ii++]->SetText(TGString::Format("%0.2f",percent),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",pmem),0);

  // cout << "Pinfo: " << clk << " " << proc << " " << percent
  //      << " " << tot << " " << proc-proc_old
  //      << " " << timeSample.tms_stime << " " << timeSample.tms_utime
  //      << " " << timeSample.tms_cstime << " " << timeSample.tms_cutime
  //      << endl;

  //cout << "proc: " << rst << " " << percent << " " << tot << endl;
}

/*
void MainFrame::UpdateStatus(int rst) {

  int ii=0;

  static Long64_t bytes1=0;
  static Long64_t nevents_old=0;
  static Long64_t mtrig_old=0;
  static double t1=0;
  static double mb_rate,ev_rate,trig_rate;

  static clock_t clk_old, proc_old;

  struct tms timeSample;

  Long64_t tot;
  clock_t clk, proc;
  static double percent=0;
  static double pmem=0;

  //gSystem->GetProcInfo(&pinfo);
  
  //cout << "updatestatus: " << rst << endl;

  if (rst) {
    bytes1=0;
    nevents_old=0;
    mtrig_old=0;
    mb_rate=0;
    ev_rate=0;
    trig_rate=0;

    clk_old=0;
    proc_old=0;
    percent=0;
    pmem=0;

    t1=0;
    opt.T_acq=0;
  }

  //char txt[100];
  //time_t tt = opt.F_start.GetSec();

  //cout << "T_acq2: " << opt.T_acq << " " << crs->Tstart64 << endl;
  // if (opt.Tstop && opt.T_acq>opt.Tstop) {
  //   DoStartStop();
  // }

  double dt = opt.T_acq - t1;

  if (dt>0.1) {
    mb_rate = (crs->inputbytes-bytes1)/MB/dt;
    ev_rate = (crs->nevents-nevents_old)/dt;
    trig_rate = (crs->mtrig-mtrig_old)/dt;
    //cout << "trig_rate: " << trig_rate << " " << dt << endl;

    bytes1=crs->inputbytes;
    nevents_old=crs->nevents;
    mtrig_old=crs->mtrig;
    t1=opt.T_acq;

    //cpu
    clk = times(&timeSample);
    proc = timeSample.tms_stime + timeSample.tms_utime;
    tot = clk-clk_old;
    if (tot>10) {
      percent = 100.*(proc-proc_old)/tot;
      clk_old=clk;
      proc_old=proc;
    }
    //else
    //percent = 0;

    //mem
    gSystem->GetMemInfo(&minfo);
    gSystem->GetProcInfo(&pinfo);
    pmem = pinfo.fMemResident/minfo.fMemTotal*0.1;
  }

  fStat[ii++]->SetText(crs->txt_start,0);

  fStat[ii++]->SetText(TGString::Format("%0.1f",opt.T_acq),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nevents),0);
  fStat[ii++]->SetText(TGString::Format("%0.3f",ev_rate),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->mtrig),0);
  fStat[ii++]->SetText(TGString::Format("%0.3f",trig_rate),0);
  fStat[ii++]->SetText(TGString::Format("%lld",crs->nbuffers),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->inputbytes/MB),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",mb_rate),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->rawbytes/MB),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",crs->decbytes/MB),0);

  fStat[ii++]->SetText(TGString::Format("%0.2f",percent),0);
  fStat[ii++]->SetText(TGString::Format("%0.2f",pmem),0);

  // cout << "Pinfo: " << clk << " " << proc << " " << percent
  //      << " " << tot << " " << proc-proc_old
  //      << " " << timeSample.tms_stime << " " << timeSample.tms_utime
  //      << " " << timeSample.tms_cstime << " " << timeSample.tms_cutime
  //      << endl;

  //cout << "proc: " << rst << " " << percent << " " << tot << endl;
}
*/

void MainFrame::CloseWindow() {

  static int cc=0;
  // cout << "CloseWindow: " << cc << endl;

  if (cc) {
    cc=0;
    return;
  }

  if (crs->b_acq && crs->Fmode==1) {
    cc=1;
    // fStart->Emit("Clicked()");
    // cout << "dostartstop7: " << endl;
#ifdef CYUSB
    crs->DoStartStop(0);
    gSystem->Sleep(300);
#endif
    // cout << "dostartstop8: " << endl;
    return;
  }

  if (crs->b_fana) {
    cc=1;
    DoAna();
    gSystem->Sleep(300);
    return;
  }

  // return;

  // delete crs;
  // delete EvtFrm;

  // Clean up used widgets: frames, buttons, layouthints
  //printf("end\n");
  // Cleanup();
  //DoExit();
  delete this;
  gApplication->Terminate(0);
}

void MainFrame::DoExit() {
  //int i;
  //double it[4];
  //double sum;

  //cout << "DoExit" << endl;

  //saveinit(parfile);
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
  //parfile = (char*)"romana.par";
  gzFile ff = gzopen(parfile,"wb");
  if (ff) {
    crs->SaveParGz(ff,crs->module);
    gzclose(ff);
  }
  else {
    cout << "Can't open file: " << parfile << endl;
  }

  // printf("%lld bytes\n",recd*2);
  // printf("%d events\n",nevent);
  // printf("%d bad events\n",bad_events);

  delete this;
  gApplication->Terminate(0);
}

void MainFrame::DoSaveRoot() {

  //cout << "Dosave: " << endl;
  //cout << "Dosave: " << datfname << endl;

  if (!crs->b_stop) return;

  const char *dnd_types[] = {
    "root files",     "*.root",
    "all files",      "*",
    0,               0
  };

  //char s_name[100];

  string s_name, dir, name, ext;
  TGFileInfo fi;

  s_name = string();

  //   SplitFilename (string(datfname),dir,name,ext);
  //   dir.append("Root/");
  // #ifdef LINUX
  //   mkdir(dir.c_str(),0755);
  // #else
  //   _mkdir(dir.c_str());
  // #endif

  //s_name = dir;
  s_name.append(name);
  s_name.append(".root");

  //strcat(dir,"save/");
  //strcpy(s_name,name);
  //strcat(s_name,".root");

  fi.fFileTypes = dnd_types;
  fi.fIniDir    = StrDup(dir.c_str());
  fi.fFilename  = StrDup(s_name.c_str());

  //TGFileDialog *gfsave = 
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);

  if (fi.fFilename) {
    saveroot(fi.fFilename);

    // Int_t retval=kMBOk;
    // if (!stat(fi.fFilename, &statb)) {
    //   new TGMsgBox(gClient->GetRoot(), this,
    // 		   "File exists",
    // 		   msg_exists, kMBIconAsterisk, kMBOk|kMBCancel, &retval);
    // }

    // if (retval == kMBOk) {
    //   saveroot(fi.fFilename);
    // }
  }

#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif

}

/*
  void MainFrame::DoSaveAscii() {

  if (!crs->b_stop) return;




  #ifdef LINUX
  if (chdir(startdir)) {}
  #else
  _chdir(startdir);
  #endif

  }
*/

void MainFrame::DoTab(Int_t num) {
  //cout << "DoTab: " << num << endl;
  TGTab *tab = (TGTab*) gTQSender;

  TString name = TString(tab->GetCurrentTab()->GetString());

  opt.seltab = tab->GetCurrent();

  //cout << "dotab: " << tab->GetCurrent() << " " 
  //     << tab->GetCurrentTab()->GetString() << endl;
  //cout << "dotab: " << name.EqualTo("Parameters",TString::kIgnoreCase) << endl;


  if (name.EqualTo("Parameters",TString::kIgnoreCase)) {
    //cout << "DoTab1: " << name << endl;
    parpar->Update();
  }
  else if (name.EqualTo("Channels",TString::kIgnoreCase)) {
    //cout << "DoTab2: " << name << endl;
    chanpar->Update();
  }
  else if (name.EqualTo("Events",TString::kIgnoreCase)) {
    parpar->Update();
    EvtFrm->ChkpkUpdate();
    //cout << "DoTab4: " << name << endl;
    if (crs->b_stop)
      EvtFrm->DrawEvent2();
  }
  else if (name.EqualTo("histograms",TString::kIgnoreCase)) {
    histpar->Update();
  }
  else if (name.Contains("Plots",TString::kIgnoreCase)) {
    parpar->Update();
    //cout << "DoTab5: " << name << endl;
    if (!crs->b_acq)
      HiFrm->HiUpdate();
    //HiFrm->ReDraw();
  }
  else if (name.Contains("Errors",TString::kIgnoreCase)) {
  }
}

void MainFrame::EventInfo(Int_t event, Int_t px, Int_t py, TObject *selected)
{
  //  Writes the event status in the status bar parts

  //const char *text0, *text1, *text3;
  //char text2[50];
  //char ttt[100];
  int nn;

  double x1,x2,y1,y2;

  TVirtualPad *fp = fEcanvas->GetCanvas()->GetSelectedPad();
  if (fp) {
    nn = fp->GetNumber();
    //TVirtualPad *fc1 = gPad->GetCanvas()->cd(1);;
    //TVirtualPad *fc2 = gPad->GetCanvas()->cd(2);;
    TVirtualPad *fc = gPad->GetCanvas()->cd(nn);

    //cout << fc << " " << fp << endl;

    //fc-> AbsCoordinates(true);
    x1=fc->PixeltoX(px);
    x2=fc->AbsPixeltoX(px);
    y1=fc->PixeltoY(py);
    y2=fc->AbsPixeltoY(py);
    printf("R: %d %d %d %f %f %f %f\n",nn,px,py,x1,y1,x2,y2);
  }

  //gPad->GetRangeAxis(x1,y1,x2,y2);

  //printf("A: %f %f %f %f\n",x1,y1,x2,y2);

  //cout << gPad->GetUxmin() << " " << gPad->GetUxmax() << endl; 

  //gPad-> AbsCoordinates(true);

  //x1=gPad->PixeltoX(px);
  //y1=gPad->PixeltoY(py);

  /*

    text0 = selected->GetTitle();
    //fBar->SetText(text0,0);
    //SetStatusText(text0,0);
    text1 = selected->GetName();
    //SetStatusText(text1,1);
    if (event == kKeyPress)
    sprintf(text2, "%c", (char) px);
    else
    sprintf(text2, "%d :   %d,%d %f %f", nn, px, py, x1,y1);
    //SetStatusText(text2,2);
    text3 = selected->GetObjectInfo(px,py);
    //SetStatusText(text3,3);

    cout << text1 << " " << text2 << " " << text3 << endl;

  */
}

/*
  void MainFrame::DoCross() {

  static bool bcross=false;

  bcross = !bcross;

  cout << "bcross: " << bcross << endl;

  if (bcross) {
  fEcanvas->GetCanvas()->
  Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","MainFrame",
  this, "EventInfo(Int_t,Int_t,Int_t,TObject*)");
  }
  else {
  fEcanvas->GetCanvas()->
  Disconnect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)");
  }

  }
*/

void MainFrame::HandleMenu(Int_t menu_id)
{

  char command[128];
  int status;

  if (!crs->b_stop && menu_id!=M_HELP) {
    switch (menu_id) {
    case M_READINIT:
    case M_SAVEINIT:
    case M_SAVEROOT:
    case M_READROOT:
    case M_RESET_USB:
    case M_FILE_EXIT:
      return;
    default:
      break;
    }
  }

  //cout << menu_id << endl;

  // Handle menu events.

  // TRootHelpDialog *hd;
  // static TString dir(".");
  // TGFileInfo fi;
  // fi.fFileTypes = dnd_types;
  // fi.fIniDir    = StrDup(dir);

  switch (menu_id) {

  case M_READINIT:
    DoRWinit(kFDOpen);
    break;
  case M_SAVEINIT:
    DoRWinit(kFDSave);
    break;
  case M_SAVEROOT:
    DoSaveRoot();
    break;
    // case M_SAVEASCII:
    //   DoSaveAscii();
    //   break;
  case M_READROOT:
    DoReadRoot();
    break;
  case M_FILE_BROWSE:
    new TBrowser();
    break;
  case M_RESET_USB:
    crs->DoResetUSB();
    break;
  case M_EXPORT:
    Export();
    break;
    // case M_FILE_NEWCANVAS:
    //   gROOT->MakeDefCanvas();
    //   break;

    //case M_PARAM:
    //if (fPar!=NULL) {
    //delete fPar;
    //}
    //fPar = new MParDlg(gClient->GetRoot(), fMain, "Parameters");
    //fPar->Map();
    //break;

    //case M_CHANNELS:

    //if (fChan==NULL) {
    //fChan = new MChanDlg(gClient->GetRoot(), fMain, "Channels");
    //fChan->Map();
    //}

    //break;

  case M_EDIT_PROF8:
    new PEditor(this, M_EDIT_PROF8, 400, 600);
    break;

  case M_EDIT_PROF64:
    new PEditor(this, M_EDIT_PROF64, 400, 600);
    break;

  // case M_PROF_TIME:
  //   {
  //     //cout << "cal1: " << p_time_cal << endl;
  //     new PopFrame(this,800,600,M_PROF_TIME);
  //     //cout << "cal2: " << p_pop << endl;
  //   }
  //   break;

  // case M_PRECALIBR:
  //   {
  //     if (!p_pop) {
  // 	p_pop = new PopFrame(this,800,600,M_PRECALIBR);
  //     }
  //   }
  //   break;

  case M_ECALIBR:
    new PopFrame(this,800,600,M_ECALIBR);
    break;

  case M_TCALIBR:
    //cout << "ecalibr: " << fTab->GetCurrent() << endl;
    fTab->SetTab("Plots");
    new PopFrame(this,100,600,M_TCALIBR);
    break;

  case M_PEAKS:
    fTab->SetTab("Plots");
    new PopFrame(this,100,600,M_PEAKS);
    break;

#ifdef P_LIBUSB
  case M_TEST:
    new PopFrame(this,100,600,M_TEST);
    break;
#endif

  case M_HELP:

    strcpy(command,"xdg-open ");
    strcat(command,HELP);
    status = system( command );
    if (status == -1) {
      cout << "Return value of system(command): " << status << endl;
    }

    break;

  case M_FILE_EXIT:
    DoExit();   // terminate theApp no need to use SendCloseMessage()
    break;
  }
}

//______________________________________________________________________________

/*
  void MainFrame::HandleHelp()
  {

  if (!crs->b_stop) return;

  cout << "test" << endl;

  char command[128];

  strcpy(command,"evince ");
  strcat(command,"help.pdf");
  int status = system( command );

  cout << status << endl;

  }
*/
//______________________________________________________________________________

/*
  void example() {
  // Popup the GUI...

  //gSystem->Sleep(1000);
  //HiFrm->Update();
  //myM->Move(-100,-100);
  }
*/

/*
  void dumpbuf(int nn)
  {
  int i;

  unsigned int *ibuf= (unsigned int*) buf;

  //ftmp=fopen("buf.dat","w");
  //printf("%llx \n",(recd-Buffer->r_buf+idx)*2);
  for (i=0;i<nn;i++) {
  printf("dump: %d %d %d %x\n",i,buf[2*i],buf[2*i+1],ibuf[i]);
  }
  //close(ftmp);
  }
*/

//-----------------------------
//#############################################
// END
//
//
//
//
//
//


/*
  struct MyClass {
  MyClass() {std::cout <<"MyClass constructed\n";}
  ~MyClass() {std::cout <<"MyClass destroyed\n";}
  };

  void pointer_test() {
  MyClass * pt;
  int size = 3;

  pt = new MyClass[size];
  delete[] pt;

  }
*/

//-------------------------
// TGMsgBox2::TGMsgBox2(const TGWindow* p, const TGWindow* main, const char* title, const char* msg, EMsgBoxIcon icon, Int_t buttons, Int_t* ret_code, UInt_t options, Int_t text_align)
//   : TGMsgBox(p, main, title, msg, icon, buttons,ret_code,options, text_align)
// {
//   cout << "TGM: " << endl;
// }
// TGMsgBox2::TGMsgBox2(const TGWindow* p, const TGWindow* main, const char* title, const char* msg, EMsgBoxIcon icon, Int_t buttons = kMBDismiss, Int_t* ret_code = 0, UInt_t options = kVerticalFrame, Int_t text_align = kTextCenterX|kTextCenterY)
// {}
//-------------------------
ColorMsgBox::ColorMsgBox(const TGWindow *p, const TGWindow *main,
			 const char *title, const char *msg, EMsgBoxIcon icon,
			 Int_t buttons, Int_t *ret_code)
  : TGTransientFrame(p, main, 10, 10)
{
  TGLayoutHints* LayCB5 = new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5);
  TGLayoutHints* LayEC3 = new TGLayoutHints(kLHintsExpandX|kLHintsCenterY, 3, 3, 0, 0);
  TGLayoutHints* LayCLE2 = new TGLayoutHints(kLHintsCenterY|kLHintsLeft|kLHintsExpandX, 4, 2, 2, 2);

  Pixel_t fOrng = TColor::RGB2Pixel(255,114,86);

  UInt_t width, height;

  //Pixel_t fBluevio;
  //fBluevio=TColor::RGB2Pixel(255,114,86);

  cout << "ColorBox: " << endl;

  TGHorizontalFrame* fButtonFrame = new TGHorizontalFrame(this, 100, 20, kFixedWidth);
  AddFrame(fButtonFrame, LayCB5);
  TGVerticalFrame* fLabelFrame = new TGVerticalFrame(this, 60, 20);
  AddFrame(fLabelFrame, LayCB5);

  TGTextButton* fOK = new TGTextButton(fButtonFrame, new TGHotString("&OK"), 1);
  //fOK->Associate(this);
  fButtonFrame->AddFrame(fOK, LayEC3);
  //width = TMath::Max(width, fOK->GetDefaultWidth());
  TGTextButton* fCancel = new TGTextButton(fButtonFrame, new TGHotString("&Cancel"), 2);
  //fCancel->Associate(this);
  fButtonFrame->AddFrame(fCancel, LayEC3);
  //    width = TMath::Max(width, fCancel->GetDefaultWidth()); ++nb;

  //width = TMath::Max(width, fOK->GetDefaultWidth());


  TGLabel *label;
  label = new TGLabel(fLabelFrame, msg);
  label->SetTextJustify(kTextCenterX);

  this->SetBackgroundColor(fOrng);

  fLabelFrame->AddFrame(label, LayCLE2);

  MapSubwindows();

  width  = GetDefaultWidth();
  height = GetDefaultHeight();

  Resize(width, height);

  // position relative to the parent's window

  CenterOnParent();

  // make the message box non-resizable

  SetWMSize(width, height);
  SetWMSizeHints(width, height, width, height, 0, 0);

  // set names

  SetWindowName(title);
  SetIconName(title);
  SetClassHints("MsgBox", "MsgBox");

  SetMWMHints(kMWMDecorAll | kMWMDecorResizeH  | kMWMDecorMaximize |
	      kMWMDecorMinimize | kMWMDecorMenu,
	      kMWMFuncAll  | kMWMFuncResize    | kMWMFuncMaximize |
	      kMWMFuncMinimize,
	      kMWMInputModeless);

  MapRaised();

  fClient->WaitFor(this);
}
//-------------------------
ColorMsgBox::~ColorMsgBox()
{
}


//------------------------------

TGMatrixLayout2::TGMatrixLayout2(TGCompositeFrame *main, UInt_t r, UInt_t c,
				 Int_t s, Int_t h)
{
  // TGMatrixLayout2 constructor.

  fMain    = main;
  fList    = fMain->GetList();
  fSep     = s;
  fHints   = h;
  fRows    = r;
  fColumns = c;
}

//______________________________________________________________________________
void TGMatrixLayout2::Layout()
{
  // Make a matrix layout of all frames in the list.
  //cout << "MatrixLayout1: " << endl;


  TGFrameElement *ptr;
  TGDimension csize, maxsize(0,0);
  Int_t bw = fMain->GetBorderWidth();
  Int_t x = fSep+2, y = fSep + bw;
  UInt_t rowcount = fRows, colcount = fColumns;
  fModified = kFALSE;

  TIter next(fList);
  while ((ptr = (TGFrameElement *) next())) {
    if (ptr->fState & kIsVisible) {
      csize = ptr->fFrame->GetDefaultSize();
      maxsize.fWidth  = TMath::Max(maxsize.fWidth, csize.fWidth);
      maxsize.fHeight = TMath::Max(maxsize.fHeight, csize.fHeight);
    }
  }

  next.Reset();
  int nn=0;
  while ((ptr = (TGFrameElement *) next())) {
    //cout << "Layout11: " << nn << " " << ptr->fState << " " << kIsVisible << endl;
    if (ptr->fState & kIsVisible) {
      nn++;
      //cout << "Layout12: " << nn << " " << x << " " << y << endl;
      //ptr->fFrame->Move(x, y);
      ptr->fFrame->MoveResize(x, y, csize.fWidth, csize.fHeight);
      fModified = fModified || (ptr->fFrame->GetX() != x) || 
	(ptr->fFrame->GetY() != y);

      ptr->fFrame->Layout();

      if (fColumns == 0) {
	y += maxsize.fHeight + fSep;
	rowcount--;
	if (rowcount <= 0) {
	  rowcount = fRows;
	  y = fSep + bw; x += maxsize.fWidth + fSep;
	}
      } else if (fRows == 0) {
	x += maxsize.fWidth + fSep;
	colcount--;
	if (colcount <= 0) {
	  colcount = fColumns;
	  x = fSep; y += maxsize.fHeight + fSep;
	}
      } else {
	x += maxsize.fWidth + fSep;
	colcount--;
	if (colcount <= 0) {
	  rowcount--;
	  if (rowcount <= 0) return;
	  else {
	    colcount = fColumns;
	    x = fSep; y += maxsize.fHeight + fSep;
	  }
	}
      }
    }
  }
  //cout << "MatrixLayout2: " << nn << endl;

}

//______________________________________________________________________________
TGDimension TGMatrixLayout2::GetDefaultSize() const
{
  // Return default dimension of the matrix layout.


  TGFrameElement *ptr;
  TGDimension     size, csize, maxsize(0,0);
  Int_t           count = 0;
  Int_t           bw = fMain->GetBorderWidth();

  TIter next(fList);
  while ((ptr = (TGFrameElement *) next())) {
    if (ptr->fState & kIsVisible) {
      count++;
      csize = ptr->fFrame->GetDefaultSize();
      maxsize.fWidth  = TMath::Max(maxsize.fWidth, csize.fWidth);
      maxsize.fHeight = TMath::Max(maxsize.fHeight, csize.fHeight);
    }
  }
  Int_t rows=0;
  Int_t cols=0;

  if (fRows == 0) {
    rows = (count % fColumns) ? (count / fColumns + 1) : (count / fColumns);
    size.fWidth  = fColumns * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = rows * (maxsize.fHeight + fSep) + fSep + bw;
  } else if (fColumns == 0) {
    cols = (count % fRows) ? (count / fRows + 1) : (count / fRows);
    size.fWidth  = cols * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  } else {
    size.fWidth  = fColumns * (maxsize.fWidth + fSep) + fSep;
    size.fHeight = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  }

  size.fWidth+=5;
  size.fHeight+=10;
  // int sz2 = fRows * (maxsize.fHeight + fSep) + fSep + bw;
  // cout << "GetDefaultSize: " << size.fWidth << " " << size.fHeight
  //      << " " << count << " " << rows << " " << cols
  //      << " " << sz2 << " " << fSep << " " << bw << endl;
  return size;
}
// __________________________________________________________________________
void TGMatrixLayout2::SavePrimitive(ostream &out, Option_t *)
{

  // Save matrix layout manager as a C++ statement(s) on output stream

  out << "new TGMatrixLayout2(" << fMain->GetName() << ","
      << fRows << ","
      << fColumns << ","
      << fSep << ","
      << fHints <<")";

}
