#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

#include <unistd.h>
#include <string>
#include <fstream>
#include <fcntl.h>

#include "../inc/Request.hpp"
#include "../inc/utils.hpp"

#include "Config.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

#include "PostResponder.hpp"

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
char				buffer[4096];
char				*read_body;
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


void accepter()
{
	struct sockaddr_in address = g_address;

	int addrlen = sizeof(address);

	new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);

	//START READING HEADER -> needs to be changed because in normal post request can be header and body in one
	std::cout << "START READLOOP HEADER" << std::endl;
	while (request.checkHeader() == 0)
	{
		recv(new_socket, buffer, 4096, MSG_TRUNC);
		request.appendHeader(buffer);
	}
	setRequestType(request.getHeader());
	LOG("------- REQUEST KEY: " << request.getRequestKey() << " -------");
	LOG_RED(request.getHeader());
	//std::cout << request.getHeader() << std::endl;
	std::cout << "HEADER END" << std::endl;
	std::cout << "START READLOOP" << std::endl;
	if (request.getRequestKey() == POST)
	{
		usleep(100);
		//FILE * fd = fopen("binary.file", "wb");
		int max_size = request.checkBodySize();
		std::cout << max_size << std::endl;
		read_body = NULL;
		read_body = (char *)malloc(max_size);
		recv(new_socket, read_body, max_size, MSG_WAITALL);
		request.appendBody(read_body, max_size);
		//fwrite (read_body , sizeof(char), max_size, fd);
		free(read_body);
		//fclose(fd);
	}
	std::cout << "END READLOOP" << std::endl;
	//if (request.getHeader().find("boundary=") != std::string::npos)
	//	request.createFileFlex();
	//LOG_GREEN(request.getBody());
	//std::cout << "END BODY" << std::endl;
	//std::pair<std::string, std::string> input_pair = divideInput(&buffer);
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
	else if (request.getRequestKey() == POST)
		PostResponder pR(request.getHeader(), request.getBody(), new_socket);
}

void	readConfigFile() {
	Config config;
	config.buildMap("setup.conf");

	config.printMap();

	std::string nec_vars[] = {"port", "necessary"};
	std::vector<std::string> vec(&(nec_vars[0]), &(nec_vars[2]));
	//std::cout << "check: " << config.checkNecessaryKeys(vec) << "\n";

	int port = 0;
	int necessary = 0;
	int * ints[] = {&port, &necessary};
	config.readIntVars(nec_vars, ints, 2);
	//std::cout << "port: " << port << " necessary: " << necessary << "\n";
}

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
	readConfigFile();
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
		LOG_BLUE("==========================WAITING==========================");
		request.clearHeader();
		request.clearBody();
		updateFilesHTML();
		accepter();
		handler();
		//responder();
		LOG_BLUE("============================DONE===========================");
	}
	/* LAUNCH */


	return 0;
}
