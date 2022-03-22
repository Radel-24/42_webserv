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

/* if there is a server that has a fitting name to the request, the request hast to get forwarded to that server */
/* else we use the default server, which is the first from the config file, that uses the same port */
/* PORT HAS TO FIT AS WELL !!!!!!!!!!!!!!!!!!!!! */
std::map<int, Server *>::iterator detectCorrectServer( std::map<int, Server *> & servers, Request & request ) {
	std::map<int, Server *>::iterator	iter = servers.begin();
	while (iter != servers.end()) {
		if (iter->second->server_name == request.getHostName())
			return iter;
		++iter;
	}
	return servers.begin();
}
/* end alex new */

void accepter(std::map<int, Server *> & servers)
{
	//Server server;
	fd_set	watching_read_sockets;
	fd_set	watching_write_sockets;
	std::map<int, Request *> requests;

	FD_ZERO(&watching_read_sockets);
	FD_ZERO(&watching_write_sockets);

	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		LOG_GREEN("Server added to waching read sockets");
		FD_SET(iter->first, &watching_read_sockets);
	}

	

	while (1){
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
		for (int i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &read_sockets)) {
				std::map<int, Server *>::iterator server_elem = servers.find(i);
				if (server_elem != servers.end()) {
					LOG_RED("I AM HERE 1");
					// HIER WEITERMACHEN !!!!!
					// muss name von request mit name servern vergleichen um an richtigen zu schicken
					struct sockaddr_in address = (server_elem->second)->g_address;
					int addrlen = sizeof(address);
					int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					FD_SET(new_socket, &watching_read_sockets);
					requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
				}
				else {
					LOG_RED("I AM HERE 2");
					Request &	request = *(requests[i]);
					int requestStatus = request.readRequest();

					/* start alex new */
					std::map<int, Server *>::iterator iter = detectCorrectServer(servers, request);
					LOG_GREEN_INFO("IT'S THIS SERVER: " << iter->second->server_name);
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
						FD_CLR(request.socket, &watching_read_sockets);
						close(request.socket);
						delete &request;
						requests.erase(requests.find(i));
						LOG_RED("request removed from map");
					}
				}
			}
		}
		for (int i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &write_sockets)) {
				Request &	request = *(requests[i]);
				if (requests[i]->writeRequest() == DONE) {
					FD_CLR(request.socket, &watching_write_sockets);
					close(request.socket);
					delete &request;
					requests.erase(requests.find(i));
					LOG_RED("request removed from map");
				}
			}
		}
	}
}


int	main(int argc, char ** argv)
{
	std::string configFile;
	if (argc == 2)
		configFile = argv[1];
	else
		configFile = "setup.conf";
	std::map<int, Server *> servers;
	read_config(configFile, servers);
	check_config(servers);

	/* LAUNCH */

	//while (1) {

		accepter(servers);
	//}
	/* LAUNCH */

	return 0;
}


// curl -F 'name=@hund.txt' localhost:7000 -v