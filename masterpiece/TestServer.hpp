
#ifndef TESTSERVER_H
# define TESTSERVER_H

#include "SimpleServer.hpp"
#include <unistd.h>

class TestServer: public SimpleServer
{
	private:

	char buffer[3000];
	int new_socket;

	void accepter()
	{
		struct sockaddr_in address = get_sock()->get_address();
		int addrlen = sizeof(address);
		new_socket = accept(get_sock()->get_socket(), (struct sockaddr *)&address, (socklen_t *)&addrlen);
		read(new_socket, buffer, 3000);
	}

	void handler()
	{
		std::cout << buffer << std::endl;
	}

	void responder()
	{
		char *hello = strdup("HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 29\n\nGood evening Sir lord master\n");
		write(new_socket, hello, strlen(hello));
		close(new_socket);
	}

	public:

	
	void launch()
	{
		while (1)
		{
			std::cout << "===WAITING===" << std::endl;
			bzero(buffer,3000); // vllt muell
			accepter();
			handler();
			responder();
			std::cout << "===DONE===" << std::endl;
		}
	}

	TestServer() : SimpleServer(AF_INET, SOCK_STREAM, 0, 7000, INADDR_ANY, 10)
	{
		launch();
	}

};

#endif
