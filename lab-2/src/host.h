#pragma once

#include <sched.h>
#include <semaphore.h>

#include <string>
#include <unordered_map>

#include "client.h"
#include "conn.h"

class Host {
 public:
  static Host& getInstance();
  void create_client(ConnectionType id);
  void run();

  Host(Host const&) = delete;
  void operator=(Host const&) = delete;

  inline static std::unordered_map<pid_t, Client*> s_clients;

 private:
  Host();
  ~Host();

  sem_t* m_semaphore;
  pid_t m_host_pid;
};
