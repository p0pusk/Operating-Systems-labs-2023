#pragma once

#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

using folders = std::vector<std::pair<std::filesystem::path, int>>;

class Daemon {
 public:
  static Daemon& getInstance();
  void setConfigPath(std::string const& path);
  void run();

  Daemon(Daemon const&) = delete;
  void operator=(Daemon const&) = delete;

 private:
  Daemon() = default;

  void forkProc();
  void killPrev();
  void loadConfig();
  static void handleSignal(int signum);

  uint32_t interval = 50;
  folders trackedFolders;

  const std::filesystem::path pid_path =
      std::filesystem::absolute("/var/run/daemon.pid");
  std::filesystem::path config_path = std::filesystem::absolute("config.txt");
};
