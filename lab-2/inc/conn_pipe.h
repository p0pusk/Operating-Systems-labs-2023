#pragma once

#include <sched.h>

#include <cstddef>

#include "conn.h"

class ConnPipe : public IConn {
 public:
  ConnPipe() = delete;
  ConnPipe(pid_t host_pid, pid_t client_pid);
  ~ConnPipe();
  void read(void* buf, size_t size) override;
  void write(void* buf, size_t size) override;
  void open_term();

 private:
  int m_ptoc_desc[2];
  int m_ctop_desc[2];

  pid_t m_host_pid;
  pid_t m_client_pid;
};
