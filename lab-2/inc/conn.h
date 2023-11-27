#pragma once

#include <sched.h>

#include <cstddef>
#include <filesystem>
#include <thread>

enum ConnectionType { PIPE, FIFO, SOCKET };

class IConn {
 public:
  virtual ~IConn(){};
  virtual void write(void* buf, size_t size) = 0;
  virtual void read(void* buf, size_t size) = 0;

  const int BUFF_SIZE = 1000;
};
