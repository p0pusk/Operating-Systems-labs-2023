#pragma once

#include <sched.h>

#include "conn.h"

class ConnSocket : public IConn {
 public:
  ConnSocket() = delete;
  ConnSocket(pid_t host_pid);
  ~ConnSocket();

  bool write(void* buf, size_t size) override;
  bool read(void* buf, size_t size) override;

 private:
  int m_host_fd;
  int m_client_fd;
};
