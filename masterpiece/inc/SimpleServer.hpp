#ifndef SIMPLESERVER_H
# define SIMPLESERVER_H

#include "SimpleSocket.hpp"
#include "BindingSocket.hpp"
#include "ConnectingSocket.hpp"
#include "ListeningSocket.hpp"

class SimpleServer
{
	private:

	ListeningSocket * socket;

	virtual void accepter() = 0;
	virtual void handler() = 0;
	virtual void responder() = 0;

	public:

	SimpleServer(int domain, int service, int protocol, int port, unsigned long interface, int bklg)
	{
		socket = new ListeningSocket(domain, service, protocol, port, interface, bklg);
	}

	virtual void launch() = 0;

	ListeningSocket * get_sock()
	{
		return socket;
	}

};

#endif
