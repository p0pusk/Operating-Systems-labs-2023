#pragma once

#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include <csignal>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

class Daemon {
 public:
  static Daemon& getInstance(std::string config_path);
  void run();

  Daemon(Daemon const&) = delete;
  void operator=(Daemon const&) = delete;

 private:
  Daemon() = default;
  Daemon(std::string config_path);

  void loadConfig();
  std::string pid_fp = std::filesystem::absolute("/var/run/daemon.pid");
};
