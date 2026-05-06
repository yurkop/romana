#pragma once

#include <netinet/in.h>
#include <poll.h> // For poll and pollfd
#include <sys/socket.h>
#include "TString.h"

#include <queue>

class SockClass {
  // RQ_OBJECT("SockClass")
public:
  int sockfd = 0, //
      newsockfd = 0;
  socklen_t clilen;
  char buffer[0x10000];
  struct sockaddr_in serv_addr, cli_addr;
  struct pollfd fds[1]; // Or more if you're monitoring other sockets

  std::queue<TString> l_com;
  std::queue<TString> l_par;

public:
  SockClass(int portno);
  ~SockClass();
  void Poll();
  // void Handle();
  void Eval_Buf();
  int Eval_Par();
  void Eval_Com();
  // void DoReset();
  // void Created() { Emit("Created()"); } //*SIGNAL*
};
