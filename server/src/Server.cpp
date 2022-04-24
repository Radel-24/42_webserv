#include "Server.hpp"
#include "PostResponder.hpp"

void	Location::default_init() {
	directory_listing = false;
	client_max_body_size = -1;
	redirection = "";
}

Location::Location() {
	default_init();
}

Location::Location(std::string path) : path(path) { default_init(); }

void	Server::default_init() {
	port = 80;
	backlog = 150;
	client_max_body_size = -1;
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

/*
Setting up the server with the parameters from the class.
socket = creates a network socket for tcp/ip connections
g_address.sin_family = PF_INET == TCP/IP;
g_address.sin_port = htons(port) == PORT to communicate over;
g_address.sin_addr.s_addr = htonl(INADDR_ANY) == accept every IPv4 address;
FD_ZERO = Clear all entries from the set. (sets them all to zero)
FD_SET = Add fd to the set.
fcntl = set the socket to nonblocking
setsockopt = sets the socket to be reusable, so when the server crash we can instantly restart it
bind = maps the socket to the corresponding port on the machine
listen = allows connections to the port/socket and logs them in a backlog, NOW we can accept them
test_connection checks if a function returns < 0 to see i there was an error.
*/
void	Server::configure( std::map<int, Server *> & servers ) {
	int on = 1;
	int tmp;

	//Establish socket and test
	sock = socket(PF_INET, SOCK_STREAM, 0);
	test_connection(sock);

	g_address.sin_family = PF_INET;
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

	// you can bind to a port only once, so we don't bind if there is already a server on the port
	/* BINDING SOCKET */
	bool	skip_bind = false;
	for (std::map<int, Server *>::iterator	iter = servers.begin(); iter != servers.end(); ++iter) {
		if (iter->second->port == this->port) {
			skip_bind = true;
			break;
		}
	}
	if (!skip_bind) {
		connection = bind(sock, (struct sockaddr *) &g_address, sizeof(g_address));
		test_connection(connection);
	}
	/* BINDING SOCKET */


	/* LISTENING SOCKET */
	listening = listen(sock, backlog);
	test_connection(listening);
	/* LISTENING SOCKET */

	//check for main client_max_body_size and adapt all location client_max_body_size with this value
	if (client_max_body_size != -1) {
		for (std::map<std::string, Location*>::iterator location = locations.begin(); location != locations.end(); ++location) {
			if (location->second->client_max_body_size == -1 || client_max_body_size < location->second->client_max_body_size)
				location->second->client_max_body_size = client_max_body_size;
		}
	}
}

std::string	Server::buildTreeCommandLine( std::string webserverRoot, std::string nameTag ) {
	std::string	execPath = webserverRoot;

	execPath += "/tree -H "; // tree executable
	execPath += "."; // which files do you want to show (cwd)
	execPath += " -T '";
	execPath += nameTag; // title inside the HTML
	execPath += "' -L 1 --nolinks --noreport --charset utf-8 -o ";
	execPath += webserverRoot + root; // where to create file
	execPath += "/tree.html"; // name of file
	return execPath;
}

std::string	Server::createFileTree( Location * location ) {
	std::string cwd = getPWD();
	std::string	fileContent = "";

	std::string	locationPath = cwd + root + "/" + location->path;

	if (!chdir(locationPath.c_str())) {
		std::string execPath = buildTreeCommandLine(cwd, location->path);
		if (system(execPath.c_str()) == -1)
			LOG_RED_INFO("ERROR: file tree went wrong");
		else {
			fileContent = fileToString(cwd + root + "/tree.html");
		}
		if (chdir(cwd.c_str())) {
			LOG_RED_INFO("ERROR: chdir: " << cwd);
		}
	}
	else {
		LOG_RED_INFO("ERROR: chdir: " << locationPath);
	}
	return fileContent;
}