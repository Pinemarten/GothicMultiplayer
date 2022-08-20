#include "HTTPServer.h"
#include <fstream>

using namespace std;
using namespace httplib;
HTTPServer::HTTPServer()
{

}

HTTPServer::~HTTPServer()
{
	server.stop();
}

void HTTPServer::Start(int port)
{
	server.Get("/wb_file", [](const Request& req, Response& res) {
		ifstream file("world.wbm", ios::binary|ios::in);
		if(!file.is_open()) {
			res.set_content("EMPTY", "text/plain");
			return;
		}
		stringstream buffer;
		buffer << file.rdbuf();
		res.set_content(buffer.str(), "text/plain");
	});

	server.Get("/class", [](const Request& req, Response& res) {
		ifstream file("class.xml");
		if (!file.is_open()) {
			res.set_content("EMPTY", "text/plain");
			return;
		}
		stringstream buffer;
		buffer << file.rdbuf();
		res.set_content(buffer.str(), "text/plain");
	});

	server.Get("/spawnpoint", [](const Request& req, Response& res) {
		ifstream file("spawnpoint");
		if (!file.is_open()) {
			res.set_content("EMPTY", "text/plain");
			return;
		}
		stringstream buffer;
		buffer << file.rdbuf();
		res.set_content(buffer.str(), "text/plain");
	});

	bool result = server.listen("0.0.0.0", port);
	if (!result) {
		puts("File server cannot start");
	}
}