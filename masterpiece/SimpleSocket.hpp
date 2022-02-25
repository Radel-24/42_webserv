#ifndef SIMPLESOCKET_H
# define SIMPLESOCKET_H

#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

class SimpleSocket
{
	private:

	int connection;
	int sock;
	struct sockaddr_in address;

	public:

	SimpleSocket(int domain, int service, int protocol, int port, unsigned long interface)
	{
		//Define address structure
		address.sin_family = domain;
		address.sin_port = htons(port);
		address.sin_addr.s_addr = htonl(interface);
		//Establish socket and test
		sock = socket(domain, service, protocol);
		test_connection(sock);
	}

	virtual int connect_to_network(int sock, struct sockaddr_in address) = 0;

	//check if socket or connection has been properly established
	void test_connection(int item_to_test)
	{
		if (item_to_test < 0)
		{
			perror("Failed to connect!");
			exit(EXIT_FAILURE);
		}
	}

	//getter functions
	struct sockaddr_in get_address()
	{
		return address;
	}

	int get_socket()
	{
		return sock;
	}

	int get_connection()
	{
		return connection;
	}

	//setter functions
	void set_connection(int connection_in)
	{
		connection = connection_in;
	}

};

#endif
