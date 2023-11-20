#include <sys/syslog.h>

#include <cstdint>
#include <iostream>

#include "daemon.h"

int32_t main(int argc, char* argv[]) {
  openlog("daemon_test", LOG_PID, LOG_DAEMON);

  std::string config_file;
  Daemon& daemon = Daemon::getInstance();

  if (argc == 2) {
    config_file = argv[1];
    daemon.setConfigPath(config_file);
  } else if (argc > 2) {
    std::cout << "Too much arguments" << std::endl;
    return 1;
  }

  daemon.run();

  return 0;
}
