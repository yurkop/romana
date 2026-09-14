#include <unistd.h>
#include <string>
#include "libsock.h"
#include "romana.h"

extern Coptions cpar;
extern Toptions opt;
extern MyMainFrame *myM;
extern ParParDlg *parpar;
extern ChanParDlg *chanpar;
extern HistParDlg *histpar;
extern CRS *crs;


SockClass::SockClass(int portno) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    perror("ERROR opening socket");

  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  }

  bzero((char *)&serv_addr, sizeof(serv_addr));
  // portno = 55555;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    perror("ERROR on binding");
  // cout << "here1: " << endl;
  listen(sockfd, 128);
  // cout << "here2: " << endl;
  clilen = sizeof(cli_addr);

  /*
  // Если нужен неблокирующий режим (скорее всего, нет)
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags == -1) {
    // Handle error
    perror("ERROR on getting flags");
  }
  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    // Handle error
    perror("ERROR on setting flags");
  }
  */

  fds[0].fd = sockfd;
  fds[0].events = POLLIN;

  // Connect("Created()", "MyMainFrame", myM, "DoReset()");
  // Connect("Created()", "SockClass", this, "DoReset()");
  // Created();

  // trd_sock = new TThread("trd_sock", handle_sock_func, (void*) 0);
  // trd_sock->Run();
}

SockClass::~SockClass() {
  if (newsockfd)
    close(newsockfd);
  if (sockfd)
    close(sockfd);
}

// void SockClass::DoReset() {
//   myM->DoReset();
// }

void SockClass::Poll() {
  // cout << "Poll1: " << endl;
  int ret = poll(fds, 1, 1); // последний параметр - timeout(ms)
  // cout << "Poll2: " << ret << endl;

  if (ret == -1) {
    perror("ERROR on poll");
    // Handle poll error
  } else if (ret == 0) {
    // Timeout occurred, no events
  } else {
    // Check for events
    if (fds[0].revents & POLLIN) {
      newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
      if (newsockfd < 0)
        perror("ERROR on accept");
      bzero(buffer, sizeof(buffer));
      int n1;
      n1 = read(newsockfd, buffer, 0xFFFF);
      if (n1 < 0)
        perror("ERROR reading from socket");

      // gSystem->Sleep(3000);


      // const char *answer = "OK";
      // std::string buf2(buffer);
      // int n2 = write(newsockfd, answer, strlen(answer));
      // if (n2 < 0)
      //   perror("ERROR writing to socket");


        // prnt("ssd s sd ss;",BGRN,"Command: ",n1,buffer,"answer:
      // ",n2,answer,RST);
      Eval_Buf();

      close(newsockfd); // Закрываем соединение после обработки
      newsockfd = 0; // Сбрасываем дескриптор
    }
  }
}

/*
void SockClass::Handle() {
  while (true) {
    newsockfd = accept(sockfd,
                       (struct sockaddr *) &cli_addr,
                       &clilen);
    cout << "here3: " << newsockfd << endl;
    if (newsockfd < 0)
      error("ERROR on accept");
    bzero(buffer,sizeof(buffer));
    int n1;
    n1 = read(newsockfd,buffer,0xFFFF);
    if (n1 < 0) error("ERROR reading from socket");

    //printf("Here is the message: %s\n",buffer);
    const char* answer = "OK";
    string buf2(buffer);
    int n2 = write(newsockfd,answer,strlen(answer));
    if (n2 < 0) error("ERROR writing to socket");
    prnt("ssd s sd ss;",BGRN,"Command: ",n1,buffer,"answer: ",n2,answer,RST);
    //cout << "sdfsafd: " << buffer << " " << buf2 << " " << n2 << endl;
    // for (int i=0;i<strlen(buffer);i++) {
    //   cout << i << " " << int(buffer[i]) << endl;
    // }
    EvalBuf();
  }
}
*/

void SockClass::Eval_Buf() {
  std::vector<TString> zz; // all commands/parameters separated by " " or ";"

  const char *delim = "; ";
  char *token = strtok(buffer, delim);
  while (token) {
    zz.push_back(token);
    // cout << std::quoted(token) << ' ';
    token = strtok(nullptr, delim);
  }
  // cout << endl;

  l_par.clear();
  l_com.clear();

  for (auto it = zz.begin(); it != zz.end(); ++it) {
    // cout << "zz: " << *it << endl;
    if (it->Contains("="))
      l_par.push_back(*it);
    // l_par.push_back(*it);
    else
      l_com.push_back(*it);
    // l_com.push_back(*it);
  }

  if (!l_par.empty()) {
    int nn = Eval_Par();
    if (nn && myM) {
      parpar->Update();
      chanpar->Update();
      histpar->Update();
    }
  }
  if (!l_com.empty())
    Eval_Com();

  //const char *answer = "OK";
  int n2 = write(newsockfd, answer.c_str(), answer.size());
  if (n2 < 0)
    perror("ERROR writing to socket");

  if (!l_com.empty())
    Run_Com();

    // cout << "EndEval: " << l_par.size() << " " << l_com.size() << endl;
}

int SockClass::Eval_Par() {
  // cout << "Eval_Par: " << l_par.size() << " " << l_par.front() << endl;
  int npar = 0;
  answer="OK";
  while (!l_par.empty()) {
    TString ss = l_par.front();
    l_par.pop_front();

    int res = 0;
    res += evalpar(ss, (char *)&opt, "Toptions");
    if (res == 0)
      npar++;
    res += evalpar(ss, (char *)&cpar, "Coptions");
    if (res == 0)
      npar++;
    if (res >= 200) {
      prnt("ssssds;", BRED, "Parameter ", ss.Data(), " not found: ", res, RST);
      answer = "ERROR";
    }
  }

  return npar;

}

void SockClass::Eval_Com() {
  //cout << "Eval_Com: " << l_com.size() << " " << l_com.front() << endl;

  // Проверка: количество команд совпадает с CMD_COUNT
  // static_assert(commands.size() == CMD_COUNT, "commands and enum mismatch");

  for (const auto &cmd : l_com) {
    size_t index = CMD_COUNT;
    for (size_t i = 0; i < commands.size(); ++i) {
      if (cmd.EqualTo(commands[i].name, TString::kIgnoreCase)) {
      //if (cmd == commands[i].name) {
        index = i;
        break;
      }
    }

    switch (index) {
    case CMD_DEVICE:
      if (crs->Fmode == 1)
        answer = cpar.GetDevice(crs->module, 0);
      else
        answer = "NONE";
      break;
    case CMD_B_ACQ:
      answer = std::to_string(crs->b_acq);
      break;
    case CMD_B_STOP:
      answer = std::to_string(crs->b_stop);
      break;
    case CMD_B_FANA:
      answer = std::to_string(crs->b_fana);
      break;
    case CMD_RESET:
    case CMD_START:
    case CMD_STOP:
    case CMD_ANALYZE:
      answer = "OK";
      break;
    default: /* не найдено */
      answer = "Not found";
      break;
    }
    //cout << "commands: " << *cmd << endl;
  }

  //cout << "Eval_Com_end: " << l_com.size() << endl;
}

void SockClass::Run_Com() {

  for (const auto &cmd : l_com) {
    size_t index = CMD_COUNT;
    for (size_t i = 0; i < commands.size(); ++i) {
      if (cmd.EqualTo(commands[i].name, TString::kIgnoreCase)) {
        index = i;
        break;
      }
    }

    switch (index) {
    case CMD_RESET:
      myM->fReset2->Emit("Clicked()");
      break;
    case CMD_START:
      myM->fStart->Emit("Clicked()");
      break;
    case CMD_STOP:
      myM->fStart->Emit("Clicked()");
      break;
    case CMD_ANALYZE:
      myM->fAna->Emit("Clicked()");
      break;
    default: /* не найдено */
      break;
    }
  }
}
