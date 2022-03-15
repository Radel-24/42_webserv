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

		std::cout << "before select\n";
		int amount_ready_socks = select(FD_SETSIZE, &read_sockets, &write_sockets, NULL, NULL);
		std::cout << "after select; amount ready socks: " << amount_ready_socks << "\n";
		if (amount_ready_socks < 0)
		{
			perror("select error");
			exit(EXIT_FAILURE);
		}
		for (int i = 0; i < FD_SETSIZE; ++i) {
			if (FD_ISSET(i, &read_sockets)) {
				std::map<int, Server *>::iterator server_elem = servers.find(i);
				if (server_elem != servers.end()) {
					struct sockaddr_in address = (server_elem->second)->g_address;
					int addrlen = sizeof(address);
					int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
					FD_SET(new_socket, &watching_read_sockets);
					requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
				}
				else {
					Request &	request = *(requests[i]);
					int requestStatus = request.readRequest();

					if (requestStatus == DONE) {
						FD_CLR(request.socket, &watching_read_sockets);
						FD_SET(request.socket, &watching_write_sockets);
					}
					else if (requestStatus == DECLINE) { // TODO check if this is working
						FD_CLR(request.socket, &watching_read_sockets);
						close(request.socket);
						delete &request;
						requests.erase(requests.find(i));
						LOG_YELLOW("request removed from map");
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
					LOG_YELLOW("request removed from map");
				}
			}
		}
	}
}

void	updateFilesHTML() {
	if (!chdir("./files")) // else irgendein error
	{
		system("../tree -H './files' -T 'Your Files' -L 1 --noreport --charset utf-8 -o ../files.html"); // if == -1 error happened
		chdir("..");
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

		LOG_BLUE("==========================WAITING==========================");
		updateFilesHTML();
		accepter(servers);
		LOG_BLUE("============================DONE===========================");
	//}
	/* LAUNCH */

	return 0;
}


// curl -F 'name=@hund.txt' localhost:7000 -v