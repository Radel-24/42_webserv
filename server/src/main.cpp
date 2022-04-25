#include "main.hpp"

void	initAccepter(std::map<int, Server *> & servers, t_accepter & acp) {
	FD_ZERO(&acp.watching_read_sockets);
	FD_ZERO(&acp.watching_write_sockets);

	acp.highestSocket = 0;

	for (std::map<int, Server *>::iterator iter = servers.begin(); iter != servers.end(); ++iter) {
		LOG_GREEN("Server " << iter->second->sock << " added to waching read sockets");
		FD_SET(iter->second->sock, &acp.watching_read_sockets);
		if (acp.highestSocket < iter->second->sock) {
			acp.highestSocket = iter->second->sock;
		}
	}
}

void	watchReadSockets(t_accepter & acp, fd_set & read_sockets, std::map<int, Server *> & servers) {
	for (int check_socket = 2; check_socket <= acp.highestSocket; check_socket++) {
		if (FD_ISSET(check_socket, &read_sockets)) {
			std::map<int, Server *>::iterator server_elem;
			server_elem = servers.find(check_socket);
			if (server_elem != servers.end()) {
				struct sockaddr_in address = (server_elem->second)->g_address;
				int addrlen = sizeof(address);
				int new_socket = accept((server_elem->second)->sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				if (acp.highestSocket < new_socket) {
					acp.highestSocket = new_socket;
				}
				FD_SET(new_socket, &acp.watching_read_sockets);
				acp.requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket, server_elem->second)));
			}
			else {
				Request &	request = *(acp.requests[check_socket]);
				request.readRequest(servers);

				if (request.status >= 100 || request.status == DONE_READING) {
					FD_CLR(request.socket, &acp.watching_read_sockets);
					FD_SET(request.socket, &acp.watching_write_sockets);
					LOG_GREEN_INFO("done reading request");
				}
				else if (request.status == CLOSE_CONNECTION) {
					FD_CLR(request.socket, &acp.watching_read_sockets);
					delete &request;
					acp.requests.erase(acp.requests.find(check_socket));
				}
			}
		}
	}
}

void	watchWriteSockets(t_accepter & acp, fd_set & write_sockets) {
	for (int check_socket = 2; check_socket <= acp.highestSocket; check_socket++) {
		if (FD_ISSET(check_socket, &write_sockets)) {
			Request &	request = *(acp.requests[check_socket]);
			if (request.status >= 100 && request.status < 200) {
				acp.requests[check_socket]->writeRequest();
				FD_CLR(request.socket, &acp.watching_write_sockets);
				FD_SET(request.socket, &acp.watching_read_sockets);
				request.status = HEADER_READ;
				request.clearResponse();

			}
			else if (request.status == DONE_READING && request.closeConnection == false){
				acp.requests[check_socket]->writeRequest();
				if (request.status == DONE_WRITING) {
					FD_CLR(request.socket, &acp.watching_write_sockets);
					FD_SET(request.socket, &acp.watching_read_sockets);
					request.init();
				}
			}
			else if ((request.status >= 400 && request.status < 600) || request.closeConnection == true) {
				acp.requests[check_socket]->writeRequest();
				FD_CLR(request.socket, &acp.watching_write_sockets);
				delete &request;
				acp.requests.erase(acp.requests.find(check_socket));
				LOG_RED("request removed from map");
			}
			else if (request.status == CLOSE_CONNECTION) {
				FD_CLR(request.socket, &acp.watching_write_sockets);
				delete &request;
				acp.requests.erase(acp.requests.find(check_socket));
				LOG_RED("request removed from map");
			}
		}
	}
}

void accepter(std::map<int, Server *> & servers)
{
	t_accepter	acp;

	initAccepter(servers, acp);

	int amount_ready_socks = 0;

	struct timeval	tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	fd_set	read_sockets;
	fd_set	write_sockets;

	while (true){
		FD_ZERO(&read_sockets);
		FD_ZERO(&write_sockets);

		read_sockets = acp.watching_read_sockets;
		write_sockets = acp.watching_write_sockets;

		amount_ready_socks = select(acp.highestSocket + 1, &read_sockets, &write_sockets, NULL, &tv);

		if (amount_ready_socks == 0) {
			LOG_GREEN_INFO("get the clients out of here");
			for (std::map<int, Request *>::iterator iter = acp.requests.begin(); iter != acp.requests.end(); ++iter) {
				delete (iter->second);
			}
			acp.requests.clear();
			initAccepter(servers, acp);
			continue ;
		}
		else if (amount_ready_socks < 0) {
			perror("select error");
			//exit(EXIT_FAILURE); // TODO maybe do the same like when amount_ready_socks == 0
		}
		watchReadSockets(acp, read_sockets, servers);
		watchWriteSockets(acp, write_sockets);
	}
}

int	main(int argc, char ** argv)
{
	signal(SIGPIPE, SIG_IGN);

	//check if there is a config file in argv, if not we take the default server config file
	std::string	configFile;
	if (argc == 2)
		configFile = argv[1];
	else
		configFile = "setup.conf";

	//create a map of servers(Key: serversocket, Value: server class)
	std::map<int, Server *>	servers;

	//read the config file and setup the servers
	read_config(configFile, servers);

	//check if the servers are correctly setup
	check_config(servers);

	accepter(servers);

	return 0;
}


/*
-g flag in Makefile
Terminal: lldb
target create webserv
r
*/