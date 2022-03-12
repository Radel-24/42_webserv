#pragma once

#include <string>
#include <map>
#include <vector>
#include "Request.hpp"
#include "Client.hpp"



struct Location {
	public:
		int	port;
		std::string path;
		std::vector<std::string> methods;
		std::string	redirect;
		bool	directory_listing;
		std::string	default_file;
		std::string	cgi_extension;
		std::string	cgi_path;

		std::map<std::string, Location*> sub_locations;

	private:
		void	default_init();

	public:
		Location();
		Location(std::string path);
};

class Server {
	public:
		int port;
		std::string server_name;
		std::map<std::string, Location*> locations;
		std::map<int, Request *> requests; // TODO store clients or requests?
		double client_max_body_size;

	private:
		void	default_init();

	public:
		Server();

};