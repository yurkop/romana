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
#include "param.h"


using namespace std;

Coptions cpar;
Toptions opt;

int debug=0;

//-------------------------------------
UShort_t ClassToBuf(const char* name, const char* varname, char* var, char* buf) {
  //copies all data members to a buffer, returns size of the buffer
  //buffer should exist and have sufficient size to allocate all data

  //cout << "ClassToBuf: " << name << endl;
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

  strcpy(cname,"var");
  len = strlen(cname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,cname,len);
  sz+=len;

  len = strlen(varname)+1;
  memcpy(buf+sz,&len,sizeof(len));
  sz+=sizeof(len);
  memcpy(buf+sz,varname,len);
  sz+=len;

  if (debug&0x2)
    cout << "Save class: " << name << endl;

  TIter nextd(lst);
  TDataMember *dm;
  while ((dm = (TDataMember *) nextd())) {
    if (debug&0x2) {
      // if (!dm->GetDataType()) {
      cout << "member: " << dm->GetName() << " " << dm->GetDataType() << " " << dm->GetClass()->GetName() << endl;
      // }
    }
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
	cout << dm->GetName() << " " << len << " " << sz << endl;
    }
  }

  if (debug&0x2)
    cout << "size: " << sz << endl;
  return sz;

}

//------------------------------------

void BufToClass(const char* name, const char* varname, char* var, char* buf, int size) {
  //copies all data members from a buffer, size - size of the buffer
  //buffer should exist. Only data members with matching names are copied

  if (debug&0x2)
    cout <<"BufToClass:: " << size << endl;

  TList* lst = TClass::GetClass(name)->GetListOfDataMembers();
  if (!lst) {
    cout <<"Class " << name << " doesn't exist" << endl;
    return;
  }

  // TList* lst;
  // TIter nextd(lst);
  // TDataMember *dm;
  // while ((dm = (TDataMember *) nextd())) {
  // }

  //= (TList*)lst1->Clone("lst2");
  //cout << "lst: " << lst->GetName() << endl;
  //exit(1);

  Int_t sz=0;
  UShort_t len=0;
  UShort_t len2=0;
  TDataMember *dm;
  char memname[100];
  char clname[100]; //class name
  char vname[100]; //class var name
  const UShort_t mx=50000;
  char data[mx];

  if (TString(name).EqualTo("Toptions",TString::kIgnoreCase)) {
    strcpy(vname,"opt");
  }
  else if (TString(name).EqualTo("Coptions",TString::kIgnoreCase)) {
    strcpy(vname,"cpar");
  }
  else {
    strcpy(vname,"");
  }

  while (sz<size) {
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>size) {
      cout << "br1: " << endl;
      break;
    }
    memcpy(memname,buf+sz,len);
    sz+=len;
    if (sz>size) {
      cout << "br2: " << endl;
      break;
    }
    memcpy(&len,buf+sz,sizeof(len));
    sz+=sizeof(len);
    if (len==0 || len>=mx || sz>size) {
      cout << "br3: " << endl;
      break;
    }
    memcpy(data,buf+sz,len);
    sz+=len;
    if (sz>size) {
      cout << "br4: " << sz << " " << size << endl;
      break;
    }

    if (strcmp(memname,"class")==0) {
      strcpy(clname,data);
      if (debug&0x2)
	if (strcmp(clname,name)==0) { //the same class
	  cout << "Read class: " << clname << " " << name << endl;
	}
      continue;
    }

    if (strcmp(memname,"var")==0) {
      strcpy(vname,data);
      if (debug&0x2)
	if (strcmp(vname,varname)==0) { //the same class
	  cout << "Read var: " << vname << " " << varname << endl;
	}
      continue;
    }

    if (!strcmp(clname,name) && !strcmp(vname,varname)) { //the same class & var
      TIter nextd(lst);
      while ((dm = (TDataMember *) nextd())) {
	if (!strcmp(dm->GetName(),memname)) {
	  break;
	}
	else {
	  const char* s1 = strchr(dm->GetTitle(),'[');
	  const char* s2 = strchr(dm->GetTitle(),']');
	  if (s1 && s2) {
	    int len=s2-s1-1;
	    char str[999]="";
	    strncpy(str,s1+1,len);
	    // cout << "dm: " << dm << " " << str << " " << len << " " << dm->GetName() << " " << dm->GetTitle() << endl;
	    // exit(-1);
	    if (!strcmp(str,memname)) {
	      cout << "dm2: " << dm << " " << str << " " << len << " " << dm->GetName() << " " << dm->GetTitle() << endl;
	      //exit(-1);
	      break;
	    }
	  }
	}
	//cout << "dm: " << dm << " " << dm->GetName() << " " << dm->GetTitle() << endl;
      }
      // cout << "dm2: " << dm << endl;
      // if (dm) {
      //  	cout << "dm: " << dm << " " << dm->GetName() << " " << dm->GetTitle() << endl;
      // }
      //dm = (TDataMember*) lst->FindObject(memname);
      if (dm) {
	len2=dm->GetUnitSize();
	for (int i=0;i<dm->GetArrayDim();i++) {
	  len2*=dm->GetMaxIndex(i);
	}
	if (debug&0x4)
	  cout << "read: " << dm->GetName() << " " << len << " " << sz << endl;
	memcpy(var+dm->GetOffset(),data,TMath::Min(len,len2));
      }
      else {
	if (debug&0x2)
	  cout << "class member not found: " << dm << " " << memname << " " 
	       << clname << " " << name << endl;
      }
    }

  }

  if (debug&0x2)
    cout << "len: " << len << " " << sz << endl;
  
}
//------------------------------------

int ReadParGz(gzFile &ff, char* pname, int m1, int p1, int p2) {
  //m1 - read module (1/0); 1 - read from raw/dec file; 0 - read from par file
  //p1 - read cpar (1/0); p2 - read opt (1/0)
  UShort_t sz;

  UShort_t mod;
  gzread(ff,&mod,sizeof(mod));

  cout << "ReadParGz: " << pname << endl;
  //cout << "ReadParGz1 Fmode: " << Fmode << endl;

  gzread(ff,&sz,sizeof(sz));

  char buf[100000];
  gzread(ff,buf,sz);

  if (p1) 
    BufToClass("Coptions","cpar",(char*) &cpar,buf,sz);
  if (p2) {
    BufToClass("Toptions","opt",(char*) &opt,buf,sz);

    TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
    TIter nextd(lst);
    TDataMember *dm;
    char* popt = (char*)&opt;
    while ((dm = (TDataMember *) nextd())) {
      if (dm->GetDataType()==0 && TString(dm->GetName()).Contains("h_")) {
	//cout << "member: " << dm->GetName() << " " << dm->GetDataType() << " " << dm->GetOffset() << " " << (void*)popt+dm->GetOffset() << " " << &opt << " " << &(opt.h_time) << endl;
	BufToClass("Hdef",dm->GetName(),popt+dm->GetOffset(),buf,sz);
      }
    }
  }

  for (int i=0;i<MAX_CH+MAX_TP;i++) {
    if (opt.chtype[i]>MAX_TP) {
      opt.chtype[i]=1;
    }
  }
  //Make_prof_ch();

  // if (m1) {
  //   //T_start = opt.F_start;
  //   if (mod==2) {
  //     module=2;
  //     cout << "CRS2 File: " << Fname << " " << module << endl;
  //   }
  //   else if (mod>=32) {
  //     module=mod;
  //     cout << "CRS32 File: " << Fname << " " << module << endl;
  //   }
  //   else {
  //     Fmode=0;
  //     cout << "Unknown file type: " << Fname << " " << mod << endl;
  //     return 1;
  //   }
  // }

  //cout << "ReadParGz: " << sz << " " << pname << endl;

  //cout << "ReadParGz2: " << sz << " " << pname << " " << HiFrm << endl;

  //Set_Trigger();

  // if (p2) {
  //   //cout << "false_gz: " << endl;
  //   opt.raw_write=false;
  //   opt.dec_write=false;
  //   opt.root_write=false;
  // }

  // if (HiFrm)
  //   HiFrm->HiReset();

  return 0;
}

void SaveParGz(gzFile &ff, Short_t mod) {

  //cout << "savepargz: " << opt.tnames[0] << endl;
  char buf[100000];
  UShort_t sz=0;

  sz+=ClassToBuf("Coptions","cpar",(char*) &cpar, buf+sz);
  sz+=ClassToBuf("Toptions","opt",(char*) &opt, buf+sz);

  TList* lst = TClass::GetClass("Toptions")->GetListOfDataMembers();
  TIter nextd(lst);
  TDataMember *dm;
  char* popt = (char*)&opt;
  while ((dm = (TDataMember *) nextd())) {
    if (dm->GetDataType()==0 && TString(dm->GetName()).Contains("h_")) {
      //cout << "member: " << dm->GetName() << " " << dm->GetDataType() << endl;
      sz+=ClassToBuf("Hdef",dm->GetName(),popt+dm->GetOffset(),buf+sz);
    }
  }


  gzwrite(ff,&mod,sizeof(mod));
  gzwrite(ff,&sz,sizeof(sz));
  gzwrite(ff,buf,sz);

  //cout << "SavePar_gz: " << sz << endl;

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

  gzFile ff;

  if (argc<3) {
    cout << "usage: " << endl;
    cout << argv[0] << " r filename - read parameters" << endl;
    cout << argv[0] << " w filename - write parameters" << endl;
    return 1;
  }
  if (!strcmp(argv[1],"r")) {
    ff = gzopen(argv[2],"rb");
    if (ff) {
      if (ReadParGz(ff,argv[2],1,1,1)) {
	gzclose(ff);
	return 0;
      }
    }
    else {
      cout << "Can't open file: " << argv[2] << endl;
    }
  }
  else if (!strcmp(argv[1],"w")) {
    ff = gzopen(argv[2],"wb");
    if (ff) {
      SaveParGz(ff,1);
      gzclose(ff);
    }
    else {
      cout << "Can't open file: " << argv[2] << endl;
    }
  }
  else {
  }
  //change_gz_file2(argv[1],argv[2]);
}
