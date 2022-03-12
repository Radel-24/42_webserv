#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

#include <unistd.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../inc/Request.hpp"
#include "../inc/utils.hpp"

#include "Config.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

#include "PostResponder.hpp"
#include "ConfigToken.hpp"

#include "general.hpp"
///* LISTENING SOCKET */
//int					backlog = 10;
//int					listening;
///* LISTENING SOCKET */

/* SIMPLE SOCKET */
//int					connection;
//int					sock;
//struct sockaddr_in	g_address;
struct pollfd		fds;
/* SIMPLE SOCKET */




void handler(Request & request)
{
	LOG_RED("REQUEST TYPE:		" << request.getRequestKey());
	request.getBody().size(); // needed, so that body is ready when going on with processing?
	if (request.getRequestKey() == GET) {
		responder(request);
	}
	else if (request.getRequestKey() == POST)
		PostResponder pR(request.getHeader(), request.getBody(), request.socket);
}


std::string ToHex(const std::string & s, bool upper_case /* = true */)
{
	std::stringstream ret;

	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		if (i % 2 == 0)
			ret << " ";
		if (i % 16 == 0)
			ret << "\n";
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)s[i];
	}

	return ret.str();
}



void	setRequestType(std::string header, Request & request) {
	if (header.length() < 3) { request.setRequestKey(NIL);}
	else if (header.find("GET") != std::string::npos) { request.setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") != std::string::npos) { request.setRequestKey(POST); }
	else if (header.find("PUT") != std::string::npos) { request.setRequestKey(PUT); }
	else if (header.find("DELETE") != std::string::npos) { request.setRequestKey(DELETE);}
}


void readHeader(Request & request)
{
	std::cout << "HEADER START" << std::endl;
	char	buffer[1000];
	ssize_t bytes_read = recv(request.socket, buffer, 1000, 0);
	std::cout << "header read bytes: " << bytes_read << "\n";
	request.appendHeader(buffer);

	if (request.checkHeader()) {
		request.header_read = true;
		setRequestType(request.getHeader(), request);
		LOG("------- REQUEST KEY: " << request.getRequestKey() << " -------");
		LOG_RED(request.getHeader());
		std::cout << "HEADER END" << std::endl;
	}
}

void readBody(Request &request)
{
	std::cout << "BODY START" << std::endl;
	if (request.getRequestKey() == POST)
	{
		int max_size = request.checkBodySize();
		std::cout << "body size: " << max_size << std::endl;
		char * read_body = NULL;
		read_body = new char[max_size];
		write(request.socket, "HTTP/1.1 100 Continue\r\n\r\n", 25); // much faster when sending huge files with curl
			ssize_t bytes_read = recv(request.socket, read_body, max_size, 0);
			std::cout << "bytes read: " << bytes_read << std::endl;
			if (bytes_read > 0) {
				request.appendBody(read_body, bytes_read);
				request.bytes_read += bytes_read;
			}
		if (request.bytes_read == max_size) {
			request.body_read = true;
			std::cout << request.getBody() << std::endl;
			LOG_YELLOW("body read true");
		}
		delete read_body;
	}
	std::cout << "BODY END" << std::endl;
}

void accepter(Server &server)
{
	struct sockaddr_in address = server.g_address;

	int addrlen = sizeof(address);

	fd_set ready_sockets = server.watching_sockets;

	std::cout << "before select\n";
	int amount_ready_socks = select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL);
	std::cout << "after select; amount ready socks: " << amount_ready_socks << "\n";
	if (amount_ready_socks < 0)
	{
		perror("select error");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < FD_SETSIZE; i++) {
		if (FD_ISSET(i, &ready_sockets)) {
			if (i == server.sock) {
				int new_socket = accept(server.sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				FD_SET(new_socket, &server.watching_sockets);
				server.requests.insert(std::pair<int, Request *>(new_socket, new Request(new_socket)));
			}
			else {
				Request &	request = *(server.requests[i]);

				if (!request.header_read)
					readHeader(request);
				else if (request.header_read)
					readBody(request);

				if (request.body_read) {
					FD_CLR(request.socket, &server.watching_sockets);
					handler(request);
					close(request.socket);
					delete &request;
					server.requests.erase(server.requests.find(i));
					LOG_YELLOW("request removed from map");
				}
			}
		}
	}
}

/* START RESPONDER */
std::string	getFilename(Request & request)
{
	LOG("FILENAME START");
	std::string	converted = std::string(request.getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	LOG(file);
	return file;
}

std::string	readFile( std::string filename ) {
	std::ifstream	newFile;
	std::string		ret;
	char			c;
	newFile.open(filename, std::ios::in);
	if (!newFile)
		return "error: opening file: " + filename;
	while (!newFile.eof())
	{
		newFile >> std::noskipws >> c;
		ret.push_back(c);
	}
	newFile.close();
	return ret;
}

std::string	formatString( std::string file_content ) {
	std::string	header;
	std::string	length;
	std::string	full_header;
	std::string	ret;
	header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	length = std::to_string(file_content.length()) + "\n\n";
	full_header = header.append(length);
	ret = full_header.append(file_content);
	return ret;
}

void responder(Request & request) {
	std::string	filename;
	std::string	file_content;
	std::string	formatted;
	filename = getFilename(request);

	if (filename.empty()) {
		file_content = "alex ist sehr toll und du leider nicht so :(\n";
	}
	else {
		file_content = readFile(filename);
	}
	formatted = formatString(file_content);

	write(request.socket, formatted.c_str(), formatted.length());
	close(request.socket); // TODO is this good?
}

void	updateFilesHTML() {
	if (!chdir("./files")) // else irgendein error
	{
		system("tree -H './files' -T 'Your Files' -L 1 --noreport --charset utf-8 -o ../files.html"); // if == -1 error happened
		chdir("..");
	}
}

int	main( )
{
	std::vector<Server *> servers;
	read_config("setup.conf", servers);

	/* LAUNCH */

	Server server;

	while (1)
	{

		LOG_BLUE("==========================WAITING==========================");
		updateFilesHTML();
		accepter(server);
		LOG_BLUE("============================DONE===========================");
	}
	/* LAUNCH */


	return 0;
}
