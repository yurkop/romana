#include <TObject.h>
#include <TROOT.h>
//#include <TDatime.h>
#include <TClass.h>
#include <TDataMember.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include "gz_header.h"

using namespace std;

Coptions cpar;
Toptions opt;


//-------------------------------------
UShort_t ClassToBuf(const char* name, char* var, char* buf) {
  //copies all data members to a buffer, returns size of the buffer
  //buffer should exist and have sufficient size to allocate all data

  TList* lst = TClass::GetClass(name)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << name << " doesn't exist" << endl;
    return 0;
  }

  Int_t sz=0;
  Short_t len=0;

  char cname[100];
  strcpy(cname,"class");
  len = strlen(cname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,cname,len);
  sz+=len;

  len = strlen(name)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,name,len);
  sz+=len;

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
    if (dm->GetDataType()) {
      len = strlen(dm->GetName())+1;
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,dm->GetName(),len);
      sz+=len;
      len=1;
      for (int i=0;i<dm->GetArrayDim();i++) {
	len*=dm->GetMaxIndex(i)*dm->GetUnitSize();
      }
      memcpy(buf+sz,&len,sizeof(len));
      sz+=sizeof(len);
      memcpy(buf+sz,var+dm->GetOffset(),len);
      sz+=len;

    }
  }

  return sz;

}

//------------------------------------

void BufToClass(const char* name, char* var, char* buf, int size) {
  //copies all data members from a buffer, size - size of the buffer
  //buffer should exist. Only data members with matching names are copied

  TList* lst = TClass::GetClass(name)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << name << " doesn't exist" << endl;
    return;
  }

  Int_t sz=0;
  UShort_t len=0;
  UShort_t len2=0;
  //TIter nextd(lst);
  TDataMember *dm;
  char memname[100];
  char clname[100];
  const UShort_t mx=5000;
  char data[mx];
  while (sz<size) {
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>=size) break;
    memcpy(memname,buf+sz,len);
    sz+=len;
    if (sz>=size) break;
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>=size) break;
    memcpy(data,buf+sz,len);
    sz+=len;
    if (sz>=size) break;

    if (strcmp(memname,"class")==0) {
      strcpy(clname,data);
      continue;
    }

    dm = (TDataMember*) lst->FindObject(memname);
    if (dm && strcmp(clname,name)==0) {
      len2=1;
      for (int i=0;i<dm->GetArrayDim();i++) {
	len2*=dm->GetMaxIndex(i)*dm->GetUnitSize();
      }
      cout << dm->GetName() << " " << len << " " << len2 << endl;
      memcpy(var+dm->GetOffset(),data,TMath::Min(len,len2));
    }
    else {
      cout << "member not found: " << dm << " " << memname << " " 
	   << clname << " " << name << endl;
    }

  }

}
//--------------------------------

void change_gz_file2(char* name1, char* name2) {

  gzFile f1 = gzopen(name1,"rb");
  gzFile f2 = gzopen(name2,"wb");

  char buf[100000];
  Version_t ver;
  UShort_t mod2[2];
  UShort_t mod[2];
  UShort_t sz=0;

  cout << name1 << " " << name2 << " " << f1 << " " << f2 << endl;

  if (!f1 || !f2) {
    cout << "can't open file" << endl;
    exit(-1);
  }
  
  gzread(f1,mod2,sizeof(mod2));
  gzread(f1,&ver,sizeof(ver));
  gzread(f1,&opt,mod2[1]);

  memcpy(&cpar.smooth,   &opt.smooth,   sizeof(cpar.smooth));
  memcpy(&cpar.deadTime, &opt.deadTime, sizeof(cpar.deadTime));
  memcpy(&cpar.preWr,    &opt.preWr,    sizeof(cpar.preWr));
  memcpy(&cpar.durWr,    &opt.durWr,    sizeof(cpar.durWr));
  memcpy(&cpar.kderiv,   &opt.kderiv,   sizeof(cpar.kderiv));
  memcpy(&cpar.threshold,&opt.threshold,sizeof(cpar.threshold));
  memcpy(&cpar.adcGain,  &opt.adcGain,  sizeof(cpar.adcGain));
  memcpy(&cpar.acdc,     &opt.acdc,     sizeof(cpar.acdc));
  memcpy(&cpar.inv,      &opt.inv,      sizeof(cpar.inv));
  memcpy(&cpar.forcewr,  &opt.forcewr,  sizeof(cpar.forcewr));
  memcpy(&cpar.enabl,    &opt.enabl,    sizeof(cpar.enabl));
  memcpy(&cpar.chtype,   &opt.chtype,   sizeof(cpar.chtype));

  cout << "ver: " << ver << " " << opt.durWr[0] << endl;
  cout << "ver2: " << ver << " " << cpar.durWr[0] << endl;

  sz=0;
  sz+=ClassToBuf("Coptions",(char*) &cpar, buf+sz);
  sz+=ClassToBuf("Toptions",(char*) &opt, buf+sz);


  memset(mod,0,sizeof(mod));
  mod[0]=mod2[0];
  mod[1]=sz;

  gzwrite(f2,mod,sizeof(mod));
  gzwrite(f2,buf,sz);

  int res=1;
  do {
    res = gzread(f1,buf,sizeof(buf));
    gzwrite(f2,buf,sizeof(buf));
  } while (res>0);

  gzclose(f1);
  gzclose(f2);

  exit(-1);

}

int main(int argc, char **argv)
{

  /*
  char buf[100000];

  Coptions* par1 = new Coptions();
  Coptions* par2 = new Coptions();
  Toptions* opt1 = new Toptions();
  Toptions* opt2 = new Toptions();

  par2->adcGain[11]=7;
  //opt2->channels[11]=255;

  cout << "-------------" << endl;
  cout << memcmp(par1,par2,sizeof(*par1)) << " " << sizeof(*par1) << endl;
  cout << ClassToBuf("Coptions",(char*) par1, buf) << endl;
  BufToClass("Coptions",(char*) par2, buf, sizeof(buf));
  cout << memcmp(par1,par2,sizeof(*par1)) << " " << sizeof(*par1) << endl;
  cout << "-------------" << endl;

  cout << memcmp(opt1,opt2,sizeof(*opt1)) << " " << sizeof(*opt1) << endl;
  cout << ClassToBuf("Toptions",(char*) opt1, buf) << endl;
  BufToClass("Toptions",(char*) opt2, buf, sizeof(buf));
  cout << memcmp(opt1,opt2,sizeof(*opt1)) << " " << sizeof(*opt1) << endl;
  cout << "-------------" << endl;






  exit(1);
  */

  if (argc<3) {
    cout << "need two arguments" << endl;
    return 1;
  }
  change_gz_file2(argv[1],argv[2]);
}
