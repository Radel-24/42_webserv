#include "Server.hpp"
#include "PostResponder.hpp"
void	Location::default_init() {
	directory_listing = false;
}

Location::Location() {
	default_init();
}

Location::Location(std::string path) : path(path) { default_init(); }

void	Server::default_init() {
	port = 80;
	backlog = 10;
}

//check if socket or connection has been properly established
void test_connection(int item_to_test)
{
	if (item_to_test < 0)
	{
		perror("Failed to connect!");
		exit(EXIT_FAILURE);
	}
}

Server::Server() {
	default_init();
}

void	Server::configure() {
	int on = 1;
	int tmp;

	//Establish socket and test
	sock = socket(AF_INET, SOCK_STREAM, 0);
	test_connection(sock);

	g_address.sin_family = AF_INET;
	g_address.sin_port = htons(port);
	g_address.sin_addr.s_addr = htonl(INADDR_ANY);

	FD_ZERO(&watching_read_sockets);
	FD_ZERO(&watching_write_sockets);
	FD_SET(sock, &watching_read_sockets);

	fcntl(sock, F_SETFL, O_NONBLOCK);

	/* set reusable*/
	tmp = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
	if (tmp < 0) {
		perror("setsockopt() failed");
		close(sock);
		exit(-1);
	}
	/* set reusable*/

	/* BINDING SOCKET */
	connection = bind(sock, (struct sockaddr *) &g_address, sizeof(g_address));
	test_connection(connection);
	/* BINDING SOCKET */


	/* LISTENING SOCKET */
	listening = listen(sock, backlog);
	test_connection(listening);
	/* LISTENING SOCKET */
}


void	Server::updateFilesHTML() {
	std::string path = "./" + root + uploadPath;
	char * buf = getcwd(NULL, FILENAME_MAX);
	std::string execPath(buf);
	execPath += "/tree -H ";
	execPath += uploadPath;
	execPath += " -T 'Your Files' -L 1 --noreport --charset utf-8 -o ";
	execPath += root;
	execPath += "/files.html";
	//LOG_GREEN("exec Path " << execPath);
	if (!chdir(path.c_str())) // else irgendein error
	{
		system(execPath.c_str()); // if == -1 error happened
		chdir("..");
	}
	free(buf);
}