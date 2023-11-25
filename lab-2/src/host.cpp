#include "host.h"

#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <iostream>
#include <string>

#include "Iconn.h"
#include "conn_pipe.h"

const int BUFF_SIZE = 100;

Host::Host() {
  host_pid = getpid();
  semaphore = sem_open("semaphore", O_CREAT, 0644, 1);
  if (semaphore == SEM_FAILED) {
    std::cerr << "semaphore open failed\n";
    exit(1);
  }

  int t;
  sem_getvalue(semaphore, &t);
  syslog(LOG_INFO, "INFO: Semaphore value %d", t);

  std::filesystem::remove_all("/tmp/lab2");
  std::filesystem::create_directory("/tmp/lab2");
}

Host::~Host() {
  sem_close(semaphore);
  for (auto& c : connections) {
    delete c.second;
  }
}

void Host::create_client(ConnectionType id) {
  syslog(LOG_INFO, "INFO: Creating connection...");
  if (id == ConnectionType::PIPE) {
    auto conn = new ConnPipe();

    if (getpid() == host_pid) return;
    open_term(getpid());
    connections[conn->client_pid] = conn;
  }
}

std::string Host::await_line(std::string path) {
  std::ifstream fin(path);
  std::string res;
  fin >> res;
  return res;
}

void Host::open_term(pid_t) {
  std::ofstream fin;
  std::string fin_path = std::format("/tmp/lab2/log{}.file", getpid());
  std::string fout_path = std::format("/tmp/lab2/out{}.file", getpid());
  fin.open(fin_path);
  std::ofstream _(fout_path);
  system(std::format("kitty -- bash -c \"cp /dev/stdin {} | tail -f {};\"",
                     fout_path, fin_path)
             .c_str());  // открыли терминал, out которого соединен с
                         // дескриптором файла log.file, а stdin с
                         // дескриптором файла myfile.txt

  fin << "You are in child process. Type something: ";
  fin.close();

  std::chrono::minutes timeout(5);
  std::future<std::string> future = std::async(await_line, fout_path);
  if (future.wait_for(timeout) == std::future_status::ready) {
    syslog(LOG_INFO, "DEBUG: %s", future.get().c_str());
  }
}

auto main() -> int {
  syslog(LOG_INFO, "INFO: host starting...");
  Host host;
  host.create_client(ConnectionType::PIPE);
}
