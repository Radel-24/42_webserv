
#ifndef TESTSERVER_H
# define TESTSERVER_H

#include "SimpleServer.hpp"
#include "Request.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <fstream>

class TestServer: public SimpleServer
{
	private:

	char buffer[3000];
	int new_socket;
	Request	request;

	void accepter()
	{
		struct sockaddr_in address = get_sock()->get_address();
		int addrlen = sizeof(address);
		new_socket = accept(get_sock()->get_socket(), (struct sockaddr *)&address, (socklen_t *)&addrlen);
		read(new_socket, buffer, 3000);
		std::cout << buffer << std::endl;
		std::pair<std::string, std::string> input_pair = divideInput(buffer);
		request.setHeader(input_pair.first);
		request.setBody(input_pair.second);
		std::cout << "header: " << request.getHeader() << "\n";
		std::cout << "body: " << request.getBody() << "\n";
		request.setRequestKey();
		//request.setHeader(buffer);
		//request.setRequestKey();
		//if (request.getRequestKey() == POST)
		//	request.setBody(buffer);
	}

	void	createFile() {
		std::ofstream	file;
		file.open("testfile.txt");
		file << request.getBody();
		file.close();
	}

	void handler()
	{
		if (request.getRequestKey() == GET)
			responder();
		else if (request.getRequestKey() == POST)
			createFile();
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
			//responder();
			std::cout << "===DONE===" << std::endl;
		}
	}

	TestServer() : SimpleServer(AF_INET, SOCK_STREAM, 0, 7000, INADDR_ANY, 10)
	{
		launch();
	}

};

#endif
