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
/* LISTENING SOCKET */
int					backlog = 10;
int					listening;
/* LISTENING SOCKET */

/* SIMPLE SOCKET */
int					connection;
int					sock;
struct sockaddr_in	g_address;
struct pollfd		fds;
/* SIMPLE SOCKET */

/* TEST SERVER */
char				buffer;
char				*read_body;
int					new_socket;
Request				request;
/* TEST SERVER */

/* SELECT */
fd_set current_socket;
fd_set ready_sockets;
/* SELECT */


//check if socket or connection has been properly established
void test_connection(int item_to_test)
{
	if (item_to_test < 0)
	{
		perror("Failed to connect!");
		exit(EXIT_FAILURE);
	}
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



void	setRequestType(std::string header) {
	if (header.length() < 3) { request.setRequestKey(NIL);}
	else if (header.find("GET") != std::string::npos) { request.setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") != std::string::npos) { request.setRequestKey(POST); }
	else if (header.find("PUT") != std::string::npos) { request.setRequestKey(PUT); }
	else if (header.find("DELETE") != std::string::npos) { request.setRequestKey(DELETE);}
	}


void readHeader()
{
	std::cout << "HEADER START" << std::endl;
	while (request.checkHeader() == 0)
	{
		recv(new_socket, &buffer, 1, 0);
		request.appendHeader(&buffer);
	}
	setRequestType(request.getHeader());
	LOG("------- REQUEST KEY: " << request.getRequestKey() << " -------");
	LOG_RED(request.getHeader());
	std::cout << "HEADER END" << std::endl;
}

void readBody()
{
	static ssize_t total_bytes_read = 0;
	std::cout << "BODY START" << std::endl;
	if (request.getRequestKey() == POST)
	{
		//usleep(100);
		//FILE * fd = fopen("binary.file", "wb");
		int max_size = request.checkBodySize();
		std::cout << max_size << std::endl;
		read_body = NULL;
		read_body = new char[max_size];
		write(new_socket, "HTTP/1.1 100 Continue\r\n\r\n", 25); // much faster when sending huge files with curl
		while (total_bytes_read < max_size) {
			ssize_t bytes_read = recv(new_socket, read_body, max_size, 0);
			if (bytes_read > 0) {
				request.appendBody(read_body, bytes_read);
				total_bytes_read += bytes_read;
				std::cout << "bytes read: " << bytes_read << std::endl;
			}
		}
		std::cout << request.getBody() << std::endl;
		//if (bytes_read == 0) {
		//	std::cout << "socket gets closed\n";
		//	close(new_socket);
		//}
		//request.appendBody(read_body, max_size);
		//std::cout << "debug: " << request.getBody() << std::endl;
		//close(new_socket);
		//fwrite (read_body , sizeof(char), max_size, fd);
		delete read_body;
		//fclose(fd);
	}
	std::cout << "BODY END" << std::endl;
}

void accepter()
{
	struct sockaddr_in address = g_address;

	int addrlen = sizeof(address);

	//new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);

	//readHeader();
	//readBody();

	ready_sockets = current_socket;
	//if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
	if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
	{
		perror("select error");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < FD_SETSIZE; i++)
	{
		if (FD_ISSET(i, &ready_sockets))
		{
			if (i == sock)
			{
				new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);
				FD_SET(new_socket, &current_socket);
			}
			else
			{
				//handle
				readHeader();
				readBody();
				//char * place = new char[3000];
				//recv(i, place, 3000, 0);
				//write(STDOUT_FILENO, place, 3000);
				FD_CLR(i, &current_socket);
			}
		}
	}
}

/* START RESPONDER */
std::string	getFilename( void )
{
	LOG("FILENAME START");
	std::string	converted = std::string(request.getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	LOG(file);
	return file;
}

std::string	readFile( std::string filename )
{
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

std::string	formatString( std::string file_content )
{
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

void responder()
{
	std::string	filename;
	std::string	file_content;
	std::string	formatted;
	filename = getFilename();

	if (filename.empty())
	{
		file_content = "alex ist sehr toll und du leider nicht so :(\n";
	}
	else
	{
		file_content = readFile(filename);
	}
	formatted = formatString(file_content);

	write(new_socket, formatted.c_str(), formatted.length());
	close(new_socket);
}

void handler()
{
	LOG_RED("REQUEST TYPE:		" << request.getRequestKey());
	if (request.getRequestKey() == GET)
	{
		responder();
	}
	else if (request.getRequestKey() == POST && request.getBody().size() > 0) // TODO after && quick fix!!!
		PostResponder pR(request.getHeader(), request.getBody(), new_socket);
	LOG_GREEN(request.getBody());
}

//void	readConfigFile() {
//	Config config;
//	config.buildMap("setup.conf");

//	config.printMap();

//	std::string nec_vars[] = {"port", "necessary"};
//	std::vector<std::string> vec(&(nec_vars[0]), &(nec_vars[2]));
//	//std::cout << "check: " << config.checkNecessaryKeys(vec) << "\n";

//	int port = 0;
//	int necessary = 0;
//	int * ints[] = {&port, &necessary};
//	config.readIntVars(nec_vars, ints, 2);
//	//std::cout << "port: " << port << " necessary: " << necessary << "\n";
//}

void	updateFilesHTML()
{
	if (!chdir("./files")) // else irgendein error
	{
		system("tree -H './files' -T 'Your Files' -L 1 --noreport --charset utf-8 -o ../files.html"); // if == -1 error happened
		chdir("..");
	}
}

int	main( )
{
	//ConfigToken config;
	//readConfigFile();
	read_config("setup.conf");
	/* SIMPLE SOCKET */
	//Define address structure
	g_address.sin_family = AF_INET;
	g_address.sin_port = htons(7000);
	g_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//Establish socket and test
	sock = socket(AF_INET, SOCK_STREAM, 0);
	test_connection(sock);
	/* SIMPLE SOCKET */

	fcntl(sock, F_SETFL, O_NONBLOCK);

	/* BINDING SOCKET */
	connection = bind(sock, (struct sockaddr *) &g_address, sizeof(g_address));
	test_connection(connection);
	/* BINDING SOCKET */


	/* LISTENING SOCKET */
	listening = listen(sock, backlog);
	test_connection(listening);
	/* LISTENING SOCKET */

	/* SELECT */
	//innit sockets sets
	FD_ZERO(&current_socket);
	FD_SET(sock, &current_socket);
	/* SELECT */

	/* LAUNCH */
	while (1)
	{

		LOG_BLUE("==========================WAITING==========================");
		request.clearHeader();
		request.clearBody();
		updateFilesHTML();
		accepter();
		handler();
		LOG_BLUE("============================DONE===========================");
	}
	/* LAUNCH */


	return 0;
}
