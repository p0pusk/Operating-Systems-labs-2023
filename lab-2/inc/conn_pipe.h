#pragma once

#include <sched.h>

#include <cstddef>

#include "Iconn.h"

class ConnPipe : public IConn {
 public:
  ConnPipe();
  ~ConnPipe();
  void read(void* buf, size_t size) override;
  void write(void* buf, size_t size) override;

  pid_t client_pid;

 private:
  pid_t host_pid;

  int filedes[2];
};
