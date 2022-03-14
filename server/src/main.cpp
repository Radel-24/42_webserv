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


void accepter(std::vector<Server *> & servers)
{
	//Server server;
	fd_set	watching_read_sockets;
	fd_set	watching_write_sockets;
	fd_set	server_sockets;

	for (std::vector<Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter){
		FD_SET((*iter)->sock, &server_sockets);
	}

	while (1){

	struct sockaddr_in address = server->g_address;

	int addrlen = sizeof(address);

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
			if (FD_ISSET(i, &server_sockets)) {
				int new_socket = accept(server->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				FD_SET(new_socket, &watching_read_sockets);
				server->requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket)));
			}
			else {
				Request &	request = *(server->requests[i]);
				if (server->requests[i]->readRequest() == DONE) {
					FD_CLR(request.socket, &watching_read_sockets);
					FD_SET(request.socket, &watching_write_sockets);
				}
			}
		}
	}
	for (int i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET(i, &write_sockets)) {
			Request &	request = *(server->requests[i]);
			if (server->requests[i]->writeRequest() == DONE) {
				FD_CLR(request.socket, &watching_write_sockets);
				delete &request;
				server->requests.erase(server->requests.find(i));
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
	std::vector<Server *> servers;
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