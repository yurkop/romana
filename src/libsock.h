#pragma once

#include <netinet/in.h>
#include <poll.h> // For poll and pollfd
#include <sys/socket.h>
#include "TString.h"

#include <deque>

enum Command {
    CMD_DEVICE = 0,
    CMD_B_ACQ,
    CMD_B_STOP,
    CMD_B_FANA,
    CMD_RESET,
    CMD_START,
    CMD_STOP,
    CMD_ANALYZE,
    CMD_COUNT // маркер конца
};

struct CommandEntry {
    Command cmd;
    const char* name;
};

class SockClass {
  // RQ_OBJECT("SockClass")
public:
  int sockfd = 0, //
      newsockfd = 0;
  socklen_t clilen;
  char buffer[0x10000];
  struct sockaddr_in serv_addr, cli_addr;
  struct pollfd fds[1]; // Or more if you're monitoring other sockets

  std::deque<TString> l_com;
  std::deque<TString> l_par;
  std::string answer;

const std::vector<CommandEntry> commands = {
    {CMD_DEVICE,  "device"},
    {CMD_B_ACQ,   "b_acq"},
    {CMD_B_STOP,  "b_stop"},
    {CMD_B_FANA,  "b_fana"},
    {CMD_RESET,   "reset"},
    {CMD_START,   "start"},
    {CMD_STOP,    "stop"},
    {CMD_ANALYZE, "analyze"}
};

public:
  SockClass(int portno);
  ~SockClass();
  void Poll();
  // void Handle();
  void Eval_Buf();
  int Eval_Par();
  void Eval_Com();
  void Run_Com();
  // void DoReset();
  // void Created() { Emit("Created()"); } //*SIGNAL*
};
