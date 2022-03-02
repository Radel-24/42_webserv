#include <sys/socket.h>
#include <stdio.h>
#include <iostream>
#include <netinet/in.h>

#include <unistd.h>
#include <string>
#include <fstream>

#include "Request.hpp"
#include "utils.hpp"

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

void accepter()
{
	struct sockaddr_in address = g_address;

	int addrlen = sizeof(address);

	new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen);

	bzero(buffer, 3000);
	read(new_socket, buffer, 3000);


	LOG_GREEN("---------------- START HEADER: ----------------");
	std::cout << buffer << std::endl;
	LOG_GREEN("----------------- END HEADER: -----------------");

	request.setHeader(buffer);

	request.setRequestKey();

	// LOG("\033[1;32m" << "------- REQUEST KEY: " << request.getRequestKey() << " -------" << "\033[0m");

	if (request.getRequestKey() == POST)
	{
		sleep(1);

		bzero(buffer, 3000);
		read(new_socket, buffer, 3000);

		LOG_RED("---------------- START BODY: ----------------");
		std::cout << buffer << std::endl;
		LOG_RED("----------------- END BODY: -----------------");

		request.setBody(buffer);
		bzero(buffer, 3000);
	}

	std::pair<std::string, std::string> input_pair = divideInput(buffer);
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
		updateFilesHTML();
		bzero(buffer, 3000);
		accepter();
		handler();
		LOG_BLUE("============================DONE===========================");
	}
	/* LAUNCH */


	return 0;
}