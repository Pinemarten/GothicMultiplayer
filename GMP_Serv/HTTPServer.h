#pragma once
#include "../cpp-httplib/httplib.h"
class HTTPServer
{
private:
	int port;
	httplib::Server server;
public:
	HTTPServer();
	~HTTPServer();

	void Start(int);
};

