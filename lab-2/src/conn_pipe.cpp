#include "conn_pipe.h"

#include <fcntl.h>
#include <sys/syslog.h>
#include <unistd.h>

#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "conn.h"

ConnPipe::ConnPipe(pid_t host_pid, pid_t client_pid) {
  int status = pipe(m_ptoc_desc);
  if (status == -1) {
    syslog(LOG_ERR, "ERROR: creating connection, in pipe()");
    exit(1);
  }

  status = pipe(m_ctop_desc);
  if (status == -1) {
    syslog(LOG_ERR, "ERROR: creating connection, in pipe()");
    close(m_ptoc_desc[0]);
    close(m_ptoc_desc[1]);
    exit(1);
  }

  m_host_pid = host_pid;
  m_client_pid = client_pid;
  syslog(LOG_INFO, "INFO: Pipe created");
}

ConnPipe::~ConnPipe() {
  close(m_ptoc_desc[0]);
  close(m_ptoc_desc[1]);
  close(m_ctop_desc[0]);
  close(m_ctop_desc[1]);
}

void ConnPipe::write(void* buf, size_t size) {
  pid_t pid = getpid();
  if (pid == m_host_pid) {
    ::write(m_ptoc_desc[1], buf, size);
    syslog(LOG_INFO, "INFO: host wrote: %s", (char*)buf);
  } else if (pid == m_client_pid) {
    ::write(m_ctop_desc[1], buf, size);
    syslog(LOG_INFO, "INFO: client wrote: %s", (char*)buf);
  }
}

void ConnPipe::read(void* buf, size_t size) {
  pid_t pid = getpid();
  if (pid == m_host_pid) {
    ::read(m_ctop_desc[0], buf, size);
    syslog(LOG_INFO, "INFO: host read: %s", (char*)buf);
  } else if (pid == m_client_pid) {
    ::read(m_ptoc_desc[0], buf, size);
    syslog(LOG_INFO, "INFO: client read: %s", (char*)buf);
  }
}
