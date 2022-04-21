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

void	initAccepter(std::map<int, Server *> & servers, fd_set & watching_read_sockets, fd_set & watching_write_sockets) {
	FD_ZERO(&watching_read_sockets);
	FD_ZERO(&watching_write_sockets);

	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		LOG_GREEN("Server " << iter->second->sock << " added to waching read sockets");
		FD_SET(iter->second->sock, &watching_read_sockets);
		//if (highestSocket < iter->second->sock) {
		//	highestSocket = iter->second->sock;
		//}
	}
}

void accepter(std::map<int, Server *> & serve)
{
	std::map<int, Server*> servers = serve;
	//Server server;
	fd_set	watching_read_sockets;
	fd_set	watching_write_sockets;

	std::set<int> set_read_sockets;
	std::set<int> set_write_sockets;
	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		set_read_sockets.insert(iter->second->sock);
		LOG_BLUE_INFO("server sock added " << iter->second->sock);
	}

	std::map<int, Request *> requests;

	//int	highestSocket = 0;

	initAccepter(servers, watching_read_sockets, watching_write_sockets);

	//LOG_RED_INFO("highest socket " << highestSocket);

	int amount_ready_socks = 0;

	timeval tv;
	tv.tv_sec = 10;

	int check_socket = 2;
	LOG_PINK_INFO("check socket " << &check_socket);
	LOG_PINK_INFO("amount ready socks " << &amount_ready_socks);
	//sleep(5);

	fd_set read_sockets;
	fd_set write_sockets;

	while (true){
		LABEL:
		FD_ZERO(&read_sockets);
		FD_ZERO(&write_sockets);
		for (std::set<int>::iterator iter = set_read_sockets.begin(); iter != set_read_sockets.end(); ++iter) {
			FD_SET(*iter, &read_sockets);
			//LOG_BLUE_INFO("read socket " << *iter);
		}
		for (std::set<int>::iterator iter = set_write_sockets.begin(); iter != set_write_sockets.end(); ++iter) {
			FD_SET(*iter, &write_sockets);
			//LOG_BLUE_INFO("write socket " << *iter);
		}
		//read_sockets = watching_read_sockets;
		//write_sockets = watching_write_sockets;

		// TODO speed this up
		// TODO can fcntl(...) be used to get currently highest fd?
		LOG_PINK("before select");
		
		amount_ready_socks = select(FD_SETSIZE, &read_sockets, &write_sockets, NULL, &tv); // TODO add &tv for timeout

		LOG_PINK("after select " << amount_ready_socks);

		if (amount_ready_socks == 0) {
			requests.clear();
			initAccepter(servers, watching_read_sockets, watching_write_sockets);
			LOG_GREEN_INFO("get the clients out of here");

			set_read_sockets.clear();
			set_write_sockets.clear();
			for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
				set_read_sockets.insert(iter->second->sock);
				LOG_BLUE_INFO("server sock added " << iter->second->sock);
			}
			continue ;
		}

		if (amount_ready_socks < 0)
		{
			perror("select error");
			exit(EXIT_FAILURE);
		}

		// TODO read steps should be: default server reads header, requested server checks header, and maybe assigns new server socket with accept (close old socket!) and processes request

		check_socket = 2;
		for (; check_socket < FD_SETSIZE; check_socket++) {
			if (FD_ISSET(check_socket, &read_sockets)) {
				LOG_BLUE_INFO("read chapter " << check_socket);
				std::map<int, Server *>::iterator server_elem;
				LOG_BLUE_INFO("iterator created");
				if (servers.empty()) {
					LOG_RED_INFO("shit");
				}
				LOG_RED_INFO("what?! " << servers.find(check_socket)->first);
				server_elem = servers.find(check_socket);
				LOG_BLUE_INFO("done find");
				if (server_elem != servers.end()) {
					LOG_BLUE_INFO("new client");
					struct sockaddr_in address = (server_elem->second)->g_address;
					int addrlen = sizeof(address);
					int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					//if (highestSocket < new_socket) {
					//	highestSocket = new_socket;
					//}
					FD_SET(new_socket, &watching_read_sockets);
					set_read_sockets.insert(new_socket);
					requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
					LOG_YELLOW_INFO("new connection set up " << requests.size());
				}
				else {
					LOG_BLUE_INFO("read client");
					//if ((requests[check_socket]) == NULL) {
					//	LOG_RED_INFO("finds null pointer");
					//	continue;
					//}
					Request &	request = *(requests[check_socket]);
					request.readRequest(servers);

					if (request.status >= 100 || request.status == DONE_READING) {
						LOG_RED_INFO("new request status " << request.status);
						FD_CLR(request.socket, &watching_read_sockets);
						set_read_sockets.erase(request.socket);
						FD_SET(request.socket, &watching_write_sockets);
						set_write_sockets.insert(request.socket);
					}
					else if (request.status == CLIENT_CLOSED_CONNECTION) {
						LOG_RED("request removed from map" << request.socket);
						FD_CLR(request.socket, &watching_read_sockets);
						set_read_sockets.erase(request.socket);
						delete &request;
						requests.erase(requests.find(check_socket));
					}
				}
				goto LABEL;
			}
		}
		check_socket = 2;
		for (; check_socket < FD_SETSIZE; check_socket++) {
			if (FD_ISSET(check_socket, &write_sockets)) {
				LOG_BLUE_INFO("write chapter");
				//if ((requests[check_socket]) == NULL) {
				//	LOG_RED_INFO("finds null pointer");
				//	continue;
				//}
				Request &	request = *(requests[check_socket]);
				LOG_RED_INFO("request status: " << request.status);
				if (request.status >= 100 && request.status < 200) {
					requests[check_socket]->writeRequest();
					FD_CLR(request.socket, &watching_write_sockets);
					set_write_sockets.erase(request.socket);
					FD_SET(request.socket, &watching_read_sockets);
					set_read_sockets.insert(request.socket);
					request.status = HEADER_READ;
				}
				else if (request.status == DONE_READING || (request.status >= 200 && request.status < 600)) {
					requests[check_socket]->writeRequest();
					//if (request.status == DONE_WRITING) {
					//	FD_CLR(request.socket, &watching_write_sockets);
					//	delete &request;
					//	requests.erase(requests.find(check_socket));
					//	LOG_RED("request removed from map");
					//}
					if (request.status == DONE_WRITING_CGI || request.status == DONE_WRITING) {
						FD_CLR(request.socket, &watching_write_sockets);
						set_write_sockets.erase(request.socket);
						FD_SET(request.socket, &watching_read_sockets);
						set_read_sockets.insert(request.socket);
						request.status = READING_HEADER;
						request.init();
					}
				}
				else if (request.status == CLIENT_CLOSED_CONNECTION) {
						FD_CLR(request.socket, &watching_write_sockets);
						set_write_sockets.erase(request.socket);
						delete &request;
						requests.erase(requests.find(check_socket));
						LOG_RED("request removed from map");
				}
			goto LABEL;
			}
		}
	}
}


int	main(int argc, char ** argv)
{
	LOG_RED_INFO(getcwd(NULL, FILENAME_MAX));
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