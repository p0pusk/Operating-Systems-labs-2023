#include "conn_sock.h"

#include <netinet/in.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include <unistd.h>

ConnSocket::ConnSocket(pid_t host_pid) {
  int new_socket;
  struct sockaddr_in address;

  // Creating socket file descriptor
  if ((m_host_domain = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_ERR, "ERROR: socket() failed");
    exit(1);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(m_host_domain, (struct sockaddr*)&address, sizeof(address)) < 0) {
    syslog(LOG_ERR, "ERROR: [Socket] bind() failed");
    exit(1);
  }

  if (listen(m_host_domain, 5) < 0) {
    syslog(LOG_ERR, "ERROR: [Socket] listen() failed");
    exit(1);
  }

  socklen_t addrlen = sizeof(address);
  if ((new_socket =
           accept(m_host_domain, (struct sockaddr*)&address, &addrlen)) < 0) {
    syslog(LOG_ERR, "ERROR: [Socket] accept() failed");
    exit(1);
  }
}
