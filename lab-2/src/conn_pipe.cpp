#include "conn_pipe.h"

#include <fcntl.h>
#include <sys/syslog.h>
#include <unistd.h>

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "Iconn.h"

ConnPipe::ConnPipe() {
  int status = pipe(filedes);
  if (status == -1) {
    throw std::runtime_error("cannot create pipe");
  }

  host_pid = getpid();

  pid_t pid = fork();
  if (pid == 0) {
    client_pid = getpid();
    syslog(LOG_INFO, "INFO: created client");
  } else {
    client_pid = pid;
  }
}

ConnPipe::~ConnPipe() {
  close(filedes[0]);
  close(filedes[1]);
}

void ConnPipe::write(void* buf, size_t size) {
  ::write(filedes[1], buf, size);
  syslog(LOG_INFO, "INFO: host wrote: %s", buf);
}

void ConnPipe::read(void* buf, size_t size) {
  ::read(filedes[0], buf, size);
  syslog(LOG_INFO, "INFO: client read: %s", buf);
}
