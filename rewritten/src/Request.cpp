#include "Request.hpp"
#include <iostream>
int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

void	Request::setHeader(std::string input) { header = input; }
void	Request::setBody(std::string body) { this->body = body; }

void	Request::setRequestKey() {
	if (header.length() < 3) { requestKey = NIL; }
	else if (header.find("GET") != std::string::npos) { requestKey = GET; } // if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") != std::string::npos) { requestKey = POST; }
	else if (header.find("PUT") != std::string::npos) { requestKey = PUT; }
	else if (header.find("DELETE") != std::string::npos) { requestKey = DELETE; }
}