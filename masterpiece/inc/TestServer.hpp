
#ifndef TESTSERVER_H
# define TESTSERVER_H

#include "SimpleServer.hpp"
#include "Request.hpp"
#include "utils.hpp"
#include <unistd.h>
#include <string>
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

	std::string	getFilename( void )
	{
		std::string	converted = std::string(buffer);
		int			start = converted.find("/") + 1;
		int			end = converted.find("HTTP") - 1;
		std::string	file = converted.substr(start, end - start);

		return file;
	}

	std::string	readFile( std::string filename )
	{
		std::ifstream	newFile;
		std::string		ret;
		char			c;

		newFile.open(filename, std::ios::in);
		if (!newFile)
			return "eror: opening file: " + filename;
		while (!newFile.eof())
		{
			newFile >> std::noskipws >> c;
			ret.push_back(c);
		}
		newFile.close();

		return ret;
	}

	std::string	formatString( std::string file_content )
	{
		std::string	header;
		std::string	length;
		std::string	full_header;
		std::string	ret;

		header = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
		length = std::to_string(file_content.length()) + "\n\n";
		full_header = header.append(length);
		ret = full_header.append(file_content);

		return ret;
	}

	void responder()
	{
		std::string	filename;
		std::string	file_content;
		std::string	formatted;

		filename = getFilename();
		if (filename.empty())
			file_content = "alex ist sehr toll und du leider nicht so :(\n";
		else
			file_content = readFile(filename);
		formatted = formatString(file_content);

		write(new_socket, formatted.c_str(), formatted.length());
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
