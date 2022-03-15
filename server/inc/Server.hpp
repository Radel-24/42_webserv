#pragma once

#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/select.h>
#include <unistd.h>


#include <string>
#include <map>
#include <vector>




struct Location {
	public:
		int	port;
		std::string path;
		std::string root;
		std::vector<std::string> methods;
		std::string	redirect;
		bool	directory_listing;
		std::string	default_file;
		std::string	cgi_extension;
		std::string	cgi_path;

		//std::map<std::string, Location*> sub_locations; //maybe remove this

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
		std::string root;

		double client_max_body_size;
		fd_set	watching_read_sockets;
		fd_set	watching_write_sockets;
		int	sock;
		struct sockaddr_in g_address;
		int	backlog;

	private:
		int connection;
		int listening;

	private:
		void	default_init();

	public:
		Server();

		void	configure();

};