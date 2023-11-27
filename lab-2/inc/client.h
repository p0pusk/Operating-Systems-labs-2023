#pragma once

#include <memory>

#include "conn.h"

class Client {
 public:
  Client(ConnectionType connection_type);
  Client() = delete;
  ~Client();

  void open_term();

  pid_t m_client_pid;
  std::unique_ptr<IConn> m_conn;

 protected:
  pid_t m_host_pid;
  sem_t* m_semaphore;
  std::filesystem::path m_term_stdin;
  std::filesystem::path m_term_stdout;

  void get_input();
};
