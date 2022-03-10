#include "Server.hpp"

void	Location::default_init() {

}

Location::Location() {
	default_init();
}

Location::Location(std::string path) : path(path) { default_init(); }

void	Server::default_init() {
	port = 80;
}

Server::Server() {
	default_init();
}