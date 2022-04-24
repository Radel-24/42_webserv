#include <iostream>
#include <unistd.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdint>

#include "Request.hpp"
#include "utils.hpp"
#include "Config.hpp"
#include "PostResponder.hpp"
#include "general.hpp"

#include <sys/time.h>

#include <set>

#include <signal.h>

/* start alex new */
void	printServerMap(std::map<int, Server *> & servers) {
	std::map<int, Server *>::iterator iter = servers.begin();
	while (iter != servers.end()) {
		LOG_CYAN("SERVER START ------------");
		LOG_CYAN(iter->first << " | " << iter->second);
		LOG_CYAN("name: " << iter->second->server_name);
		LOG_CYAN("port: " << iter->second->port);
		LOG_CYAN("SERVER END --------------");
		++iter;
	}
}

void	initAccepter(std::map<int, Server *> & servers, fd_set & watching_read_sockets, fd_set & watching_write_sockets, int & highestSocket) {
	FD_ZERO(&watching_read_sockets);
	FD_ZERO(&watching_write_sockets);

	highestSocket = 0;

	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		LOG_GREEN("Server " << iter->second->sock << " added to waching read sockets");
		FD_SET(iter->second->sock, &watching_read_sockets);
		if (highestSocket < iter->second->sock) {
			highestSocket = iter->second->sock;
		}
	}
}

void accepter(std::map<int, Server *> & servers)
{
	int	highestSocket;
	//std::map<int, Server*> servers = serve;
	//Server server;
	fd_set	watching_read_sockets;
	fd_set	watching_write_sockets;

	//std::set<int> set_read_sockets;
	//std::set<int> set_write_sockets;
	//for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
	//	set_read_sockets.insert(iter->second->sock);
	//	LOG_BLUE_INFO("server sock added " << iter->second->sock);
	//}

	std::map<int, Request *> requests;



	initAccepter(servers, watching_read_sockets, watching_write_sockets, highestSocket);

	//LOG_RED_INFO("highest socket " << highestSocket);

	int amount_ready_socks = 0;

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	fd_set read_sockets;
	fd_set write_sockets;

	while (true){
		FD_ZERO(&read_sockets);
		FD_ZERO(&write_sockets);

		read_sockets = watching_read_sockets;
		write_sockets = watching_write_sockets;

		amount_ready_socks = select(highestSocket + 1, &read_sockets, &write_sockets, NULL, &tv);

		if (amount_ready_socks == 0) {
			LOG_GREEN_INFO("get the clients out of here");
			for (std::map<int, Request *>::iterator iter = requests.begin(); iter != requests.end(); ++iter) {
				delete (iter->second);
			}
			requests.clear();
			initAccepter(servers, watching_read_sockets, watching_write_sockets, highestSocket);
			continue ;
		}

		if (amount_ready_socks < 0)
		{
			perror("select error");
			exit(EXIT_FAILURE); // TODO maybe do the same like when amount_ready_socks == 0
		}

		// TODO read steps should be: default server reads header, requested server checks header, and maybe assigns new server socket with accept (close old socket!) and processes request

		for (int check_socket = 2; check_socket <= highestSocket; check_socket++) {
			if (FD_ISSET(check_socket, &read_sockets)) {
				std::map<int, Server *>::iterator server_elem;
				server_elem = servers.find(check_socket);
				if (server_elem != servers.end()) {
					struct sockaddr_in address = (server_elem->second)->g_address;
					int addrlen = sizeof(address);
					int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					if (highestSocket < new_socket) {
						highestSocket = new_socket;
					}
					FD_SET(new_socket, &watching_read_sockets);
					requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
				}
				else {
					Request &	request = *(requests[check_socket]);
					request.readRequest(servers);

					if (request.status >= 100 || request.status == DONE_READING) {
						FD_CLR(request.socket, &watching_read_sockets);
						FD_SET(request.socket, &watching_write_sockets);
					}
					else if (request.status == CLOSE_CONNECTION) {
						FD_CLR(request.socket, &watching_read_sockets);
						delete &request;
						requests.erase(requests.find(check_socket));
					}
				}
			}
		}
		for (int check_socket = 2; check_socket <= highestSocket; check_socket++) {
			if (FD_ISSET(check_socket, &write_sockets)) {
				Request &	request = *(requests[check_socket]);
				if (request.status >= 100 && request.status < 200) {
					requests[check_socket]->writeRequest();
					FD_CLR(request.socket, &watching_write_sockets);
					FD_SET(request.socket, &watching_read_sockets);
					request.status = HEADER_READ;
				}
				else if (request.status == DONE_READING && request.closeConnection == false){
					requests[check_socket]->writeRequest();
					if (request.status == DONE_WRITING_CGI || request.status == DONE_WRITING) {
						FD_CLR(request.socket, &watching_write_sockets);
						FD_SET(request.socket, &watching_read_sockets);
						request.init();
					}
				}
				else if ((request.status >= 400 && request.status < 600) || request.closeConnection == true) {
					requests[check_socket]->writeRequest();
					//usleep(1000);
					FD_CLR(request.socket, &watching_write_sockets);
					delete &request;
					requests.erase(requests.find(check_socket));
					LOG_RED("request removed from map");
				}
				else if (request.status == CLOSE_CONNECTION) {
						FD_CLR(request.socket, &watching_write_sockets);
						delete &request;
						requests.erase(requests.find(check_socket));
						LOG_RED("request removed from map");
				}
			}
		}
	}
}

//void	sig_handler(int signum) {
//	(void)signum;
//	LOG_GREEN_INFO("Hah, you can't harm our server!");
//}

int	main(int argc, char ** argv)
{
	//signal(SIGINT, SIG_IGN); // TODO doesn't work

	LOG_RED_INFO(getPWD());
	std::string configFile;
	//check if there is a config file in argv, if not we take the default server config file
	if (argc == 2)
		configFile = argv[1];
	else
		configFile = "setup.conf";
	//create a map of servers(Key: serversocket, Value: server class)
	std::map<int, Server *> servers;
	//read the config file and setup the servers
	read_config(configFile, servers);
	//check if the servers are correctly setup
	check_config(servers);

	//hardest thing evaaaaaaa
	accepter(servers);

	return 0;
}

// TODO check file sizes after put, maybe clearing file at first call and then appending

/*
-g flag in Makefile
Terminal: lldb
target create webserv
r
*/