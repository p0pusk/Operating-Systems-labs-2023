#pragma once

#include <sched.h>

#include <cstddef>

#include "conn.h"

class ConnPipe : public IConn {
 public:
  ConnPipe() = delete;
  ConnPipe(pid_t host_pid);
  ~ConnPipe();
  bool read(void* buf, size_t size) override;
  bool write(void* buf, size_t size) override;
  void open_term();

 private:
  int m_ptoc_desc[2];
  int m_ctop_desc[2];
};
