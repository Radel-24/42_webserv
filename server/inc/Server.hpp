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
#include <set>

struct Location {
	public:
		int							port;
		std::string					path;
		std::string					root;
		std::vector<std::string>	methods;
		bool						directory_listing;
		std::string					default_file;
		long						client_max_body_size;
		std::string					redirection;

	private:
		void	default_init();

	public:
		Location();
		Location(std::string path);
};

class Server {
	public:
		int									port;
		std::string							server_name;
		std::map<std::string, Location*>	locations;
		std::string							root;
		std::string							uploadPath;

		std::string							cgi_extension;
		std::string							cgi_path;

		long								client_max_body_size;
		fd_set								watching_read_sockets;
		fd_set								watching_write_sockets;
		std::set<std::string>			cookies;
		int									sock;
		struct sockaddr_in					g_address;
		int									backlog;

		bool								websiteConfig;

	private:
		int									connection;
		int									listening;

		void		default_init();

	public:
		Server();

		void		configure( std::map<int, Server *> & servers );
		std::string	createFileTree( Location * location );
		std::string	buildTreeCommandLine( std::string webserverRoot, std::string nameTag );

};