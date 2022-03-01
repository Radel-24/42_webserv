#include "Request.hpp"
#include <iostream>
int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

void	Request::setHeader(std::string input) { header = input; }
void	Request::setBody(std::string body_in)
{
	if (this->body.empty())
	{
		body_in = body_in.substr(0, body_in.size()-1);
		this->body = body_in;
	}
	else
	{
		body_in = body_in.substr(0, body_in.size()-1);
		this->body = this->body + body_in;
	}
}

void	Request::setRequestKey() {
	if (header.length() < 3) { requestKey = NIL; }
	else if (header.find("GET") != std::string::npos) { requestKey = GET; } // if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") != std::string::npos) { requestKey = POST; }
	else if (header.find("PUT") != std::string::npos) { requestKey = PUT; }
	else if (header.find("DELETE") != std::string::npos) { requestKey = DELETE; }
}