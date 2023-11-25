#pragma once

#include <cstddef>

enum ConnectionType { PIPE, FIFO, SOCKET };

class IConn {
 public:
  virtual ~IConn(){};
  virtual void write(void* buf, size_t size) = 0;
  virtual void read(void* buf, size_t size) = 0;

 protected:
  const int BUFF_SIZE = 1000;
};
