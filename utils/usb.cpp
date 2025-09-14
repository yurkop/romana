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
void init_usb(int ndevice);
int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par, int tmout, int delay, int ans);

int main(int argc, char *argv[])
{
  int r;
  if (argc!=7) {
    printf("Usage: %s ndevice niter delay tmout ans\n",argv[0]);
    printf("ndevice - номер прибора (если их несколько)\n");
    printf("niter - число итераций\n");
    printf("cmd: команда (1,3,4,8,9,34=3+4\n");
    printf("delay - задержка в мсек\n");
    printf("tmout - в bulk_transfer (мсек)\n");
    printf("ans =1/0 - запрашивать/не запрашивать ответ на команду\n");
    return 1;
  }
  int ndevice = std::stoi(argv[1]);
  int niter = std::stoi(argv[2]);
  int cmd = std::stoi(argv[3]);
  int delay = std::stoi(argv[4]);
  int tmout = std::stoi(argv[5]);
  int ans = std::stoi(argv[6]);


  init_usb(ndevice);


  cout << "Reset usb (command7) start: " << endl;
  Command32(7,0,0,0,tmout,delay,ans); //reset usb command
  cyusb_close();
  cy_handle=0;

  std::this_thread::sleep_for(2000 * 1ms);
  cout << "Reset usb (command7) end: " << endl;

  init_usb(ndevice);



  for (int i=0;i<niter;i++) {
    cout << "Iter: " << i << endl;
    switch (cmd) {
    case 1:
    case 3:
    case 4:
    case 8:
    case 9: 
      //cout << "Пуск: " << i << endl;
      std::this_thread::sleep_for(delay * 1ms);
      Command32(cmd,0,0,0,tmout,delay,ans); //pusk
      break;
    case 34:
      //cout << "Стоп: " << i << endl;
      std::this_thread::sleep_for(delay * 1ms);
      Command32(3,0,0,0,tmout,delay,ans); //pusk
      std::this_thread::sleep_for(delay * 1ms);
      Command32(4,0,0,0,tmout,delay,ans); //stop
      break;
    default:
      cout << "wrong cmd: " << i << " " << cmd << endl;
    }
  }

}

void init_usb(int ndevice) {
  int r=0;
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

  Command32(1,0,0,0,0,0,1);
  memcpy(device,buf_in+1,4);
  for (auto i=0;i<4;i++) {
    cout << i << " " << (int) device[i] << endl;
  }
}

int Command32(UChar_t cmd, UChar_t ch, UChar_t type, int par, int tmout, int delay, int ans) {

  int transferred = 0;
  int r=0;

  int len_in,len_out;



  printf("Cmd32: %d %d %d %d %d %d\n",(int) cmd, (int) ch, (int) type, par, tmout, ans);

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
  // cout << "команда: " << (int)cmd << endl;
  r = cyusb_bulk_transfer(cy_handle, 0x01, buf_out, len_out, &transferred, tmout);
  // cout << "прошла: " << (int)cmd << endl;
  if (r) {
    oss << " Error_out";
    cyusb_error(r);
  }

  std::this_thread::sleep_for(delay*1ms);

  if (ans && cmd!=7 && cmd!=12) { // 7 и 12: сброс USB и Тест
    // std::this_thread::sleep_for(3000*1ms);
    // cout << "ответ: " << (int)cmd << endl;
    r = cyusb_bulk_transfer(cy_handle, 0x81, buf_in, len_in, &transferred, tmout);
    // cout << "прошел: " << (int)cmd << endl;
    if (r) {
      oss << " Error_in";
      cyusb_error(r);
    }
  }
  if (oss.str().size()) {
    cout << oss.str().data() << endl;
  }

  return len_in;
}
