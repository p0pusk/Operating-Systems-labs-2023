#include "client.h"

#include <assert.h>
#include <fcntl.h>
#include <linux/prctl.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/syslog.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "conn.h"
#include "conn_fifo.h"
#include "conn_pipe.h"

Client::Client(ConnectionType conn_type) {
  m_sem = sem_open("/host", 0);
  if (m_sem == SEM_FAILED) {
    sem_close(m_sem);
    syslog(LOG_ERR, "ERROR: failed to open semaphore");
    exit(1);
  }

  m_host_pid = getpid();
  m_conn_type = conn_type;
  switch (conn_type) {
    case ConnectionType::PIPE:
      m_conn = std::make_unique<ConnPipe>(m_host_pid);
      break;
    case ConnectionType::FIFO:
      m_conn = std::make_unique<ConnFifo>(m_host_pid);
      break;
    default:
      assert(1);
      break;
  }

  pid_t pid;
  switch (pid = fork()) {
    case -1:
      syslog(LOG_ERR, "ERROR: unable to fork(), host terminating");
      exit(1);
    case 0:
      prctl(PR_SET_NAME, "client");
      syslog(LOG_INFO, "INFO: client started");
      m_client_pid = getpid();
      break;
    default:
      m_client_pid = pid;
      break;
  }
}

Client::~Client() {
  sem_close(m_sem);
}

void Client::open_term() {
  if (getpid() != m_client_pid) return;

  m_term_stdin = std::format("/tmp/lab2/in{}.file", getpid());
  m_term_stdout = std::format("/tmp/lab2/out{}.file", getpid());
  std::ofstream _(m_term_stdin);
  _.close();
  std::ofstream fout(m_term_stdout);

  m_term_pid = fork();
  switch (m_term_pid) {
    case -1:
      syslog(LOG_ERR, "ERROR: in fork(), client terminating");
      exit(1);
      break;
    case 0:
      prctl(PR_SET_PDEATHSIG, SIGTERM);
      int res = execl("/usr/bin/kitty", "kitty", "--", "bash", "-c",
                      std::format("cp /dev/stdin {} | tail -f {}",
                                  m_term_stdin.c_str(), m_term_stdout.c_str())
                          .c_str(),
                      (char*)NULL);
      if (res < 0) {
        syslog(LOG_ERR, "ERROR: in execl(), client terminating");
        exit(1);
      }
      break;
  }

  syslog(LOG_INFO, "INFO: created terminal for client");
  std::string conn;
  switch (m_conn_type) {
    case ConnectionType::PIPE:
      conn = "PIPE";
      break;
    case ConnectionType::FIFO:
      conn = "FIFO";
      break;
  }
  fout << std::format(
              "You are in child process {}, connection type: \"{}\". Type "
              "something:",
              m_client_pid, conn)
       << std::endl;

  std::signal(SIGCHLD, SIG_IGN);

  std::signal(SIGCHLD, [](int) {
    syslog(LOG_INFO, "INFO: terminal closed, client terminating");
    kill(getpid(), SIGKILL);
  });

  std::ifstream fin(m_term_stdin);
  if (!fin.is_open()) {
    syslog(LOG_ERR,
           "ERROR: client can't open terminal stdin file, terminating");
    exit(1);
  }

  const int buf_size = 1000;
  char buf[buf_size];
  std::string line;
  while (true) {
    if (std::getline(fin, line)) {
      strcpy(buf, line.c_str());
      m_conn->write(buf, buf_size);
      syslog(LOG_DEBUG, "DEBUG: client[%d] wrote \"%s\" to host", m_client_pid,
             buf);
      sem_post(m_sem);
    }
    fin.clear();
  }
}
