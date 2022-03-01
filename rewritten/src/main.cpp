#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

#include <unistd.h>
#include <string>
#include <fstream>

#include "../inc/Request.hpp"
#include "../inc/utils.hpp"

#include "Config.hpp"

/* LISTENING SOCKET */
int					backlog = 10;
int					listening;
/* LISTENING SOCKET */

/* SIMPLE SOCKET */
int					connection;
int					sock;
struct sockaddr_in	g_address;
/* SIMPLE SOCKET */

/* TEST SERVER */
char				buffer[3000];
int					new_socket;
Request				request;
/* TEST SERVER */

//check if socket or connection has been properly established
void test_connection(int item_to_test)
{
	if (item_to_test < 0)
	{
		perror("Failed to connect!");
		exit(EXIT_FAILURE);
	}
}

void	findRequestType(std::string header) {
	if (header.length() < 3) { request.setRequestKey(NIL); }
	else if (header.find("GET") != std::string::npos) { request.setRequestKey(GET); } // if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") != std::string::npos) { request.setRequestKey(POST); }
	else if (header.find("PUT") != std::string::npos) { request.setRequestKey(PUT); }
	else if (header.find("DELETE") != std::string::npos) { request.setRequestKey(DELETE); }
}

void accepter()
{
	struct sockaddr_in address = g_address;
	int addrlen = sizeof(address);
	new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	read(new_socket, buffer, 3000);
	request.setHeader(buffer);
	usleep(100);
	read(new_socket, buffer, 3000);
	request.setBody(buffer);
	std::cout << buffer << std::endl;
	//std::pair<std::string, std::string> input_pair = divideString(buffer, "\n\r\n");
	//request.setHeader(input_pair.first);
	//request.setBody(input_pair.second);
	std::cout << "header: \n" << request.getHeader() << "\n";
	std::cout << "body: \n" << request.getBody() << "\n";
	std::string headerValues = request.getHeader().substr(request.getHeader().find("\n") + 1, std::string::npos);
	std::cout << "header values:\n" << headerValues << "\n";
	request.setHeaderValues(stringToMap(headerValues, "\n", ": "));
	request.printHeaderValues();
	findRequestType(request.getHeader());
}

void	createFile()
{
	std::ofstream	file;
	file.open("testfile.txt");
	file << request.getBody();
	file.close();
}

/* START RESPONDER */
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
/* END RESPONDER */

void handler()
{
	if (request.getRequestKey() == GET)
		responder();
	else if (request.getRequestKey() == POST)
		createFile();
}

int	main( void )
{
	//loadConfig(config);
	//printConfig(config);
	Config config;
	config.buildMap("setup.conf");
	config.printMap();
	std::string nec_vars[] = {"port", "necessary", "fail"};
	std::vector<std::string> vec(&(nec_vars[0]), &(nec_vars[3]));
	std::cout << "check: " << config.checkNecessaryKeys(vec) << "\n";

	/* SIMPLE SOCKET */
	//Define address structure
	g_address.sin_family = AF_INET;
	g_address.sin_port = htons(7000);
	g_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//Establish socket and test
	sock = socket(AF_INET, SOCK_STREAM, 0);
	test_connection(sock);
	/* SIMPLE SOCKET */


	/* BINDING SOCKET */
	connection = bind(sock, (struct sockaddr *) &g_address, sizeof(g_address));
	test_connection(connection);
	/* BINDING SOCKET */


	/* LISTENING SOCKET */
	listening = listen(sock, backlog);
	test_connection(listening);
	/* LISTENING SOCKET */


	/* LAUNCH */
	while (1)
	{
		std::cout << "===WAITING===" << std::endl;
		bzero(buffer, 3000);
		accepter();
		handler();
		responder();
		std::cout << "===DONE===" << std::endl;
	}
	/* LAUNCH */


	return 0;
}