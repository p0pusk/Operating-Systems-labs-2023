#include <sys/syslog.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "daemon.h"

namespace fs = std::filesystem;

int32_t main(int argc, char* argv[]) {
  openlog("daemon_test", LOG_PID, LOG_DAEMON);

  auto prev_path = fs::current_path();
  fs::current_path("/tmp/");
  fs::create_directories("test_folder/1.0/1.1/1.2/1.3/1.4/1.5");
  fs::create_directories("test_folder/2.0/2.1/2.2/2.3/2.4/2.5");
  std::ofstream give_me_a_name("test_folder/1.0/1.1/file");
  fs::current_path(prev_path);

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
