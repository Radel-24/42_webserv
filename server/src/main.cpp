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

void accepter(std::map<int, Server *> & servers)
{
	//Server server;
	fd_set	watching_read_sockets;
	fd_set	watching_write_sockets;
	std::map<int, Request *> requests;

	FD_ZERO(&watching_read_sockets);
	FD_ZERO(&watching_write_sockets);

	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		LOG_GREEN("Server " << iter->second->sock << " added to waching read sockets");
		FD_SET(iter->first, &watching_read_sockets);
	}



	while (true){
		fd_set read_sockets = watching_read_sockets;
		fd_set write_sockets = watching_write_sockets;

		std::cout << std::endl;
		LOG_YELLOW("before select ---------------------");
		int amount_ready_socks = select(FD_SETSIZE, &read_sockets, &write_sockets, NULL, NULL);
		LOG_YELLOW("after select: amount ready socks: " << amount_ready_socks);
		if (amount_ready_socks < 0)
		{
			perror("select error");
			exit(EXIT_FAILURE);
		}

		// TODO read steps should be: default server reads header, requested server checks header, and maybe assigns new server socket with accept (close old socket!) and processes request

		for (int check_socket = 0; check_socket < FD_SETSIZE; ++check_socket) {
			if (FD_ISSET(check_socket, &read_sockets)) {
				std::map<int, Server *>::iterator server_elem = servers.find(check_socket);
				if (server_elem != servers.end()) {
					struct sockaddr_in address = (server_elem->second)->g_address;
					int addrlen = sizeof(address);
					int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					FD_SET(new_socket, &watching_read_sockets);
					requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
				}
				else {
					Request &	request = *(requests[check_socket]);
					int requestStatus = request.readRequest(servers);

					/* start alex new */
					// printServerMap(servers);
					// LOG_RED(server_elem->first << " | " << server_elem->second);
					// LOG_RED(request.getServer()->server_name);
					// LOG_RED(request.getServer()->port);
					// request.printHeaderValues();
					/* end alex new */

					if (requestStatus == DONE) {
						FD_CLR(request.socket, &watching_read_sockets);
						FD_SET(request.socket, &watching_write_sockets);
					}
					else if (requestStatus == DECLINE) { // TODO check if this is working
						// TODO should try to write: 413 Payload Too Large
						request.status = DECLINE;
						FD_CLR(request.socket, &watching_read_sockets);
						FD_SET(request.socket, &watching_write_sockets);
						//close(request.socket);
						//delete &request;
						//requests.erase(requests.find(check_socket));
						//LOG_RED("request removed from map");
					}
				}
			}
		}
		for (int check_socket = 0; check_socket < FD_SETSIZE; ++check_socket) {
			if (FD_ISSET(check_socket, &write_sockets)) {
				Request &	request = *(requests[check_socket]);
				if (requests[check_socket]->writeRequest() == DONE) {
					FD_CLR(request.socket, &watching_write_sockets);
					close(request.socket);
					delete &request;
					requests.erase(requests.find(check_socket));
					LOG_RED("request removed from map");
				}
			}
		}
	}
}


int	main(int argc, char ** argv)
{
	LOG_RED_INFO(getcwd(NULL, FILENAME_MAX));
	std::string configFile;
	if (argc == 2)
		configFile = argv[1];
	else
		configFile = "setup.conf";
	std::map<int, Server *> servers;
	read_config(configFile, servers);
	check_config(servers);

	/* LAUNCH */
		accepter(servers);

	return 0;
}


// curl -F 'name=@hund.txt' localhost:7000 -v