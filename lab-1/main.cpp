#include <sys/syslog.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include "daemon.h"

int32_t main(int argc, char* argv[]) {
  openlog("daemon_test", LOG_PID, LOG_DAEMON);

  std::string config_file;
  if (argc == 1) {
    config_file = "config.txt";
  } else if (argc == 2) {
    config_file = argv[1];
  } else {
    std::cout << "Uncorrent count of arguments" << std::endl;
    return 1;
  }

  Daemon& daemon = Daemon::getInstance(config_file);
  daemon.run();

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
