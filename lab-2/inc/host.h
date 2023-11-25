#pragma once

#include <sched.h>
#include <semaphore.h>

#include <unordered_map>
#include <vector>

#include "Iconn.h"

class Host {
 public:
  Host();
  ~Host();
  void create_client(ConnectionType id);
  void run();

  std::unordered_map<pid_t, IConn*> connections;

 private:
  sem_t* semaphore;
  pid_t host_pid;

  void open_term(pid_t pid);
  std::string await_line(std::string path);
};
