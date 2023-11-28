#include "conn_pipe.h"

#include <poll.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <unistd.h>

#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "conn.h"

ConnPipe::ConnPipe(pid_t host_pid) {
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
  syslog(LOG_INFO, "INFO: Pipe created");
}

ConnPipe::~ConnPipe() {
  close(m_ptoc_desc[0]);
  close(m_ptoc_desc[1]);
  close(m_ctop_desc[0]);
  close(m_ctop_desc[1]);
}

bool ConnPipe::write(void* buf, size_t size) {
  bool res;
  pid_t pid = getpid();
  int fd;
  std::string who, to;
  if (pid == m_host_pid) {
    fd = m_ptoc_desc[1];
  } else {
    fd = m_ctop_desc[1];
  }
  res = ::write(fd, buf, size);

  return res;
}

bool ConnPipe::read(void* buf, size_t size) {
  pid_t pid = getpid();
  bool res = false;
  int fd;
  std::string who, from;
  if (pid == m_host_pid) {
    fd = m_ctop_desc[0];
  } else {
    fd = m_ptoc_desc[0];
  }

  struct pollfd tmp;
  tmp.fd = fd;
  tmp.events = POLLIN;
  if (poll(&tmp, 1, 0) == 1) {
    res = ::read(fd, buf, size);
  }

  return res;
}
