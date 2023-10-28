#include "daemon.h"

#include <sys/syslog.h>

#include <cstdlib>

Daemon& Daemon::getInstance(std::string config_path) {
  static Daemon instance(config_path);
  return instance;
}

Daemon::Daemon(std::string config_path) {}

void Daemon::run() {
  pid_t pid = 0;
  int fd;

  /* Fork off the parent process */
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* On success: The child process becomes session leader */
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }

  /* Ignore signal sent from child to parent process */
  signal(SIGCHLD, SIG_IGN);

  /* Fork off for the second time*/
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Success: Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  /* or another appropriated directory */
  chdir("/");

  /* Close all open file descriptors */
  for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    close(fd);
  }

  openlog("daemon_test", LOG_PID, LOG_DAEMON);

  std::ofstream f(pid_fp, std::ios_base::trunc);
  f << getpid();

  syslog(LOG_NOTICE, "kekis");
}
