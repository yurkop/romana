//#include <stdio.h>
//#include <stdlib.h>
#include <iostream>
#include "../src/cyusb.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>

using namespace std;
typedef unsigned char UChar_t;
UChar_t buf_out[64];
UChar_t buf_in[64];
UChar_t device[4] = {}; // device_code, Serial_n, nplates, ver_po

cyusb_handle *cy_handle;
int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par);

int main(int argc, char *argv[])
{
  int r;
  if (argc!=3) {
    printf("Usage: %s ndevice niter \n",argv[0]);
    return 1;
  }
  int ndevice = std::stoi(argv[1]);
  int niter = std::stoi(argv[2]);

  int ndev = cyusb_open();

  // if ( ndev != 1 ) {
  //   printf("number of devices not equal to one: %d\n",ndev);
  //   return -1;
  // }

  cy_handle = cyusb_gethandle(ndevice);

  if (cyusb_getvendor(cy_handle) != 0x04b4 ) {
    printf("Cypress chipset not detected\n");
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -4;
  }

  r=cyusb_reset_device(cy_handle);
  if ( r != 0 ) {
    printf("Can't reset device. Exitting: %d\n",r);
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -5;
  }

  r = cyusb_kernel_driver_active(cy_handle, 0);
  if ( r != 0 ) {
    printf("kernel driver active. Exitting\n");
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -6;
  }

  r = cyusb_claim_interface(cy_handle, 0);
  if ( r != 0 ) {
    printf("Error in claiming interface\n");
    cyusb_close();
    cy_handle=0;
    exit(-1);
    //return -7;
  }
  else
    printf("Successfully claimed interface\n");

  Command32(1,0,0,0);
  memcpy(device,buf_in+1,4);
  for (auto i=0;i<4;i++) {
    cout << i << " " << (int) device[i] << endl;
  }

  for (int i=0;i<niter;i++) {
    // cout << "Пуск: " << i << endl;
    // std::this_thread::sleep_for(1000ms);
    // Command32(3,0,0,0); //pusk
    cout << "Стоп: " << i << endl;
    std::this_thread::sleep_for(1000ms);
    Command32(4,0,0,0); //stop
  }

}

int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par) {

  int transferred = 0;
  int r;

  int len_in,len_out;



  printf("Cmd32: %d %d %d %d\n",(int) cmd, (int) ch, (int) type, par);

  buf_out[0] = cmd;
  buf_out[1] = ch;
  buf_out[2] = type;
  buf_out[3] = (UChar_t) (par >> 16);
  buf_out[4] = (UChar_t) (par >> 8);
  buf_out[5] = (UChar_t) par;

  switch (cmd) {
  case 1:
    len_out=1;
    len_in=5;
    break;
  case 2:
    len_out=6;
    len_in=1;
    break;
  case 3:
  case 4:
    len_out=1;
    len_in=1;
    break;
  case 5:
    len_out=2;
    len_in=7;
    break;
  case 6:
  case 7:
  case 8:
  case 9:
    len_out=1;
    len_in=2;
    break;
  case 10:
    len_out=1;
    len_in=9;
    break;
  case 11:
    len_out=6;
    len_in=1;
    break;
  default:
    cout << "Wrong CRS32 command: " << (int) cmd << endl;
    return 0;
  }

  std::ostringstream oss;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, 0);
  if (r) {
    oss << " Error_out";
    //printf("Error6! %d: \n",buf_out[1]);
    cyusb_error(r);
    //cyusb_close();
  }

  if (cmd==8) { // если сброс сч./буф. -> задержка
    std::this_thread::sleep_for(100ms);
  }

  if (cmd!=7 && cmd!=12) { //сброс USB и Тест
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, 0);
    if (r) {
      oss << " Error_in";
	//sprintf(ss,"Error_in");
      //printf("Error7! %d: \n",buf_out[1]);
      cyusb_error(r);
      //cyusb_close();
    }
    // else
    //   oss << " " << len_in << " " << (int) buf_in[0];
	//sprintf(ss,"%d %d",len_in,buf_in[0]);
  }
  // else {
  //   oss << " none";
  // }
  if (oss.str().size()) {
    cout << oss.str().data() << endl;
  }

  return len_in;
}
