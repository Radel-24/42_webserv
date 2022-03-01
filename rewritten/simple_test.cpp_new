#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

int	main() {
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in sockaddr;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(9999);

	bind(socketfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr));

	listen(socketfd, 10);

	size_t addrlen = sizeof(sockaddr);
	int connection = accept(socketfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	//use select so that it's non blocking

	char buf[100];
	size_t bytes_read = read(connection, buf, 100);
	std::cout << "You have sent this string: " << buf << "\n";

	std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nGood evening Sir lord master\n";
	send(connection, response.c_str(), response.size(), 0);

	close(connection);
	close(socketfd);
}