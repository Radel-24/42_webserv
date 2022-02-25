#ifndef LISTENINGSOCKET_H
# define LISTENINGSOCKET_H

#include "BindingSocket.hpp"

class ListeningSocket: public BindingSocket
{
	private:

	int backlog;
	int listening;

	public:

	ListeningSocket(int domain, int service, int protocol, int port, unsigned long interface, int bklg) : BindingSocket(domain, service, protocol, port, interface)
	{
		//set backlog variable
		backlog = bklg;
		//start listening to the network using listen() from sys/socket.h
		start_listening();
		//confirm connection was successful
		test_connection(listening);
	}

	void start_listening()
	{
		//start listening on the network
		listening = listen(get_socket(), backlog);
	}

	int get_listening()
	{
		return listening;
	}

	int get_backlog()
	{
		return backlog;
	}

};

#endif
