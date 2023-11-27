#include "host.h"

#include <fcntl.h>
#include <semaphore.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <thread>

#include "client.h"
#include "conn.h"
#include "conn_pipe.h"

Host& Host::getInstance() {
  static Host instance;
  return instance;
}

Host::Host() {
  m_host_pid = getpid();
  m_semaphore = sem_open("global", 0);
  if (m_semaphore == SEM_FAILED) {
    sem_close(m_semaphore);
    syslog(LOG_ERR, "ERROR: failed to open semaphore");
    exit(1);
  }
  int val;
  sem_getvalue(m_semaphore, &val);
  syslog(LOG_DEBUG, "host semaphore value %d", val);
  if (m_semaphore == SEM_FAILED) {
    syslog(LOG_ERR, "ERROR: in opening semaphore");
    exit(1);
  }

  std::filesystem::remove_all("/tmp/lab2");
  if (!std::filesystem::exists("/tmp/lab")) {
    std::filesystem::create_directory("/tmp/lab2");
  }

  std::signal(SIGTERM, [](int signum) {
    for (auto& c : s_clients) {
      syslog(LOG_DEBUG, "%s",
             std::format("DEBUG: killing {}", c.first).c_str());
      kill(c.first, SIGTERM);
    }

    syslog(LOG_INFO, "INFO: Host terminating");
    exit(0);
  });

  signal(SIGCHLD, SIG_IGN);
}

Host::~Host() {
  sem_close(m_semaphore);

  for (auto& client : s_clients) {
    delete client.second;
  }
}

void Host::create_client(ConnectionType id) {
  syslog(LOG_INFO, "INFO: Creating connection...");
  Client* client = new Client(id);

  client->open_term();

  if (getpid() == m_host_pid) {
    s_clients[client->m_client_pid] = client;
  }
}

void Host::run() {
  char buf[1000];
  while (true) {
    for (auto& c : s_clients) {
      int val;
      sem_getvalue(m_semaphore, &val);
      syslog(LOG_DEBUG, "before wait host semaphore value %d", val);
      sem_wait(m_semaphore);
      c.second->m_conn->read(buf, 1000);
      sem_post(m_semaphore);
    }
  }
}

auto main() -> int {
  openlog("host", LOG_NDELAY | LOG_PID, LOG_DAEMON);
  sem_t* sem = sem_open("global", O_CREAT | O_EXCL, 0644, 1);
  if (sem == SEM_FAILED) {
    sem_destroy(sem);
    std::cerr << "sem_open() failed" << std::endl;
    exit(1);
  }
  syslog(LOG_INFO, "INFO: host starting...");
  Host& host = Host::getInstance();
  host.create_client(ConnectionType::PIPE);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  host.create_client(ConnectionType::PIPE);

  host.run();
  sem_destroy(sem);
  syslog(LOG_INFO, "INFO: Timeout");
  closelog();
}
