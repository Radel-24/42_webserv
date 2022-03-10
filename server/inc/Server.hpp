#pragma once

#include <string>
#include <map>

class Server {
	private:
		int port;
		std::string server_name;
		std::map<std::string, std::string> locations;
};