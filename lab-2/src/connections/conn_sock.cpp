#include "conn_sock.h"

#include <sched.h>
#include <sys/socket.h>

ConnSocket::ConnSocket(pid_t host_pid) {
  m_host_pid = host_pid;

  int new_socket;
  struct sockaddr address;
}
