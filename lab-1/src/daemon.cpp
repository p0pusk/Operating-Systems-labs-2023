#include "daemon.h"

#include <sys/syslog.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace fs = std::filesystem;

Daemon& Daemon::getInstance() {
  static Daemon instance;
  return instance;
}

void Daemon::setConfigPath(std::string const& path) {
  config_path = fs::absolute(path);
}

void Daemon::run() {
  Daemon::killPrev();
  Daemon::forkProc();

  try {
    getInstance().loadConfig();
  } catch (std::exception e) {
    syslog(LOG_ERR, "%s", e.what());
    closelog();
    exit(EXIT_FAILURE);
  }

  syslog(LOG_NOTICE, "Daemon started");

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(interval * 1000));
    for (auto& path : trackedFolders) {
      std::stack<fs::path> s;
      for (auto i = fs::recursive_directory_iterator(path.first);
           i != fs::recursive_directory_iterator(); ++i) {
        if (i.depth() >= path.second && fs::is_directory(fs::status(*i))) {
          s.push(i->path());
        }
      }

      while (!s.empty()) {
        syslog(LOG_NOTICE, "%s",
               ("removed:" + s.top().generic_string()).c_str());
        fs::remove_all(s.top());
        s.pop();
      }
    }
  }
}

void Daemon::handleSignal(int signum) {
  switch (signum) {
    case SIGHUP:
      getInstance().loadConfig();
      break;
    case SIGTERM:
      syslog(LOG_NOTICE, "Daemon terminated");
      closelog();
      exit(EXIT_SUCCESS);
      break;
    default:
      syslog(LOG_NOTICE, "Unknown signal");
      break;
  }
}

void Daemon::forkProc() {
  pid_t pid = 0;
  int fd;

  pid = fork();

  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);
  if (setsid() < 0) exit(EXIT_FAILURE);

  signal(SIGCHLD, SIG_IGN);

  pid = fork();
  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);

  umask(0);
  chdir("/");

  for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
    close(fd);
  }

  std::signal(SIGHUP, handleSignal);
  std::signal(SIGTERM, handleSignal);

  std::ofstream fs(pid_path, std::ios_base::trunc);
  fs << getpid();
  fs.close();
}

void Daemon::killPrev() {
  pid_t pid;
  std::fstream fs(pid_path);
  fs >> pid;
  fs.close();

  /* if process exists */
  if (0 == kill(pid, 0)) {
    kill(pid, SIGTERM);
  }
}

void Daemon::loadConfig() {
  if (!fs::exists(config_path)) {
    throw std::runtime_error("Config file not found in: " +
                             config_path.generic_string());
  }

  std::fstream fs(config_path);
  std::string line;
  while (std::getline(fs, line)) {
    std::istringstream iss(line);
    fs::path path;
    int depth;
    if (!(iss >> path >> depth)) {
      throw std::runtime_error("Invalid config format");
    }

    if (!fs::exists(path)) {
      throw std::runtime_error(
          "Invalid directory path: " + path.generic_string() +
          "\nin config: " + config_path.generic_string());
    }

    trackedFolders.emplace_back(std::pair<fs::path, int>(path, depth));
  }

  syslog(LOG_NOTICE, "Loaded config");
  fs.close();
}
