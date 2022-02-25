#ifndef BINDING_SOCKET_H
# define BINDING_SOCKET_H

#include "SimpleSocket.hpp"

class BindingSocket: public SimpleSocket
{

	public:

	BindingSocket(int domain, int service, int protocol, int port, unsigned long interface) : SimpleSocket(domain, service, protocol, port, interface)
	{
		//Establish connection and test
		set_connection(connect_to_network(get_socket(), get_address()));
		test_connection(get_connection());
	}

	int connect_to_network(int sock, struct sockaddr_in address)
	{
		return bind(sock, (struct sockaddr *) &address, sizeof(address));
	}

};

#endif
