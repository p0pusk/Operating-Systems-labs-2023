#include "host.h"

#include <fcntl.h>
#include <semaphore.h>
#include <sys/syslog.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "client.h"
#include "conn.h"
#include "conn_pipe.h"

Host& Host::getInstance() {
  static Host instance;
  return instance;
}

Host::Host() {
  m_host_pid = getpid();
  m_semaphore = sem_open("/host", 0);
  if (m_semaphore == SEM_FAILED) {
    sem_close(m_semaphore);
    syslog(LOG_ERR, "ERROR: failed to open semaphore");
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

  signal(SIGCHLD, [](int) {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
      syslog(LOG_DEBUG, "DEBUG: caught terminating child %d", pid);
      delete s_clients[pid];
      s_clients.erase(pid);

      if (s_clients.size() == 0) {
        syslog(LOG_INFO, "INFO: No remaining clients, host terminating");
        exit(0);
      }
    }
  });
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
  const int buf_size = 1000;
  char buf[buf_size];
  while (true) {
    sem_wait(m_semaphore);
    syslog(LOG_DEBUG, "DEBUG: host aquired semaphore");
    for (auto& c : s_clients) {
      syslog(LOG_DEBUG, "DEBUG: host reading %d", c.first);
      if (c.second->m_conn->read(buf, buf_size)) {
        syslog(LOG_DEBUG, "DEBUG: host read \"%s\" from %d", buf, c.first);
      } else {
        syslog(LOG_DEBUG, "DEBUG: host read nothing from %d", c.first);
      }
    }
  }
}

auto main() -> int {
  openlog("host", LOG_NDELAY | LOG_PID, LOG_DAEMON);
  sem_unlink("/host");
  sem_t* sem =
      sem_open("/host", O_CREAT | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO, 0);
  errno = 0;
  if (sem == SEM_FAILED) {
    sem_close(sem);
    std::cerr << "sem_open() failed: " << errno << std::endl;
    exit(1);
  }

  syslog(LOG_INFO, "INFO: host starting...");
  Host& host = Host::getInstance();
  host.create_client(ConnectionType::PIPE);
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  host.create_client(ConnectionType::FIFO);

  host.run();
  sem_destroy(sem);
  syslog(LOG_INFO, "INFO: Timeout");
  closelog();
}
