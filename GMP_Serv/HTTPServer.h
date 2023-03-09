#pragma once

#include <httplib.h>
class HTTPServer {
private:
  int port;
  httplib::Server server;

public:
  HTTPServer();
  ~HTTPServer();

  void Start(int);
};
