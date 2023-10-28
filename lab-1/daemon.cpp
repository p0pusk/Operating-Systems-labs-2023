#include "daemon.h"

#include <csignal>
#include <fstream>
#include <sys/syslog.h>

#include <cstdlib>

Daemon &Daemon::getInstance(std::string config_path) {
  static Daemon instance(config_path);
  return instance;
}

Daemon::Daemon(std::string config_path) {}

void Daemon::run() {
  Daemon::killPrev();
  Daemon::forkProc();

  syslog(LOG_NOTICE, "kekis");
}

void Daemon::handleSignal(int signum) {
  switch (signum) {
  case SIGHUP:
    syslog(LOG_NOTICE, "sighup singal");
    break;
  case SIGTERM:
    syslog(LOG_NOTICE, "Daemon terminated");
    closelog();
    exit(EXIT_SUCCESS);
    break;
  default:
    syslog(LOG_NOTICE, "unknown signal");
    break;
  }
}

void Daemon::forkProc() {
  pid_t pid = 0;
  int fd;

  /* Fork off the parent process */
  pid = fork();

  /* An error occurred */
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  /* Let the parent terminate */
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

  /* Let the parent terminate */
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  chdir("/");

  /* Close all open file descriptors */
  for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    close(fd);
  }

  std::signal(SIGHUP, handleSignal);

  std::ofstream f(pid_fp, std::ios_base::trunc);
  f << getpid();
}

void Daemon::killPrev() {
  pid_t pid;
  std::fstream f(pid_fp);

  f >> pid;
  kill(pid, SIGTERM);
  syslog(LOG_NOTICE, "terminated previous daemon");
}
