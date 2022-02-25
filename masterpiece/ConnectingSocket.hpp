#ifndef CONNECTING_SOCKET_H
# define CONNECTING_SOCKET_H

#include "SimpleSocket.hpp"

class ConnectingSocket: public SimpleSocket
{

	public:

	ConnectingSocket(int domain, int service, int protocol, int port, unsigned long interface) : SimpleSocket(domain, service, protocol, port, interface)
	{
		//Establish connection and test
		set_connection(connect_to_network(get_socket(), get_address()));
		test_connection(get_connection());
	}

	int connect_to_network(int sock, struct sockaddr_in address)
	{
		return connect(sock, (struct sockaddr *) &address, sizeof(address));
	}

};

#endif
