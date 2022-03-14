#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

#include <unistd.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../inc/Request.hpp"
#include "../inc/utils.hpp"

#include "Config.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

#include "PostResponder.hpp"
#include "ConfigToken.hpp"

#include "general.hpp"
///* LISTENING SOCKET */
//int					backlog = 10;
//int					listening;
///* LISTENING SOCKET */

/* SIMPLE SOCKET */
//int					connection;
//int					sock;
//struct sockaddr_in	g_address;
struct pollfd		fds;
/* SIMPLE SOCKET */


std::string ToHex(const std::string & s, bool upper_case /* = true */)
{
	std::stringstream ret;

	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		if (i % 2 == 0)
			ret << " ";
		if (i % 16 == 0)
			ret << "\n";
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)s[i];
	}

	return ret.str();
}


void accepter(Server &server)
{
	struct sockaddr_in address = server.g_address;

	int addrlen = sizeof(address);

	fd_set read_sockets = server.watching_read_sockets;
	fd_set write_sockets = server.watching_write_sockets;

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
			if (i == server.sock) {
				int new_socket = accept(server.sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				FD_SET(new_socket, &server.watching_read_sockets);
				server.requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket)));
			}
			else {
				Request &	request = *(server.requests[i]);
				if (server.requests[i]->readRequest() == DONE) {
					FD_CLR(request.socket, &server.watching_read_sockets);
					FD_SET(request.socket, &server.watching_write_sockets);
				}
			}
		}
	}
	for (int i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET(i, &write_sockets)) {
			Request &	request = *(server.requests[i]);
			if (server.requests[i]->writeRequest() == DONE) {
				FD_CLR(request.socket, &server.watching_write_sockets);
				delete &request;
				server.requests.erase(server.requests.find(i));
				LOG_YELLOW("request removed from map");
			}
		}
	}
}

void	updateFilesHTML() {
	if (!chdir("./files")) // else irgendein error
	{
		system("tree -H './files' -T 'Your Files' -L 1 --noreport --charset utf-8 -o ../files.html"); // if == -1 error happened
		chdir("..");
	}
}

int	main( )
{
	std::vector<Server *> servers;
	read_config("setup.conf", servers);

	/* LAUNCH */

	Server server;

	while (1) {

		LOG_BLUE("==========================WAITING==========================");
		updateFilesHTML();
		accepter(server);
		LOG_BLUE("============================DONE===========================");
	}
	/* LAUNCH */

	return 0;
}


// curl -F 'name=@hund.txt' localhost:7000 -v