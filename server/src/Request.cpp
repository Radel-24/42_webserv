#include "../inc/Request.hpp"
#include "../inc/PostResponder.hpp"
#include <iostream>

Request::Request() : header_read(false), body_read(false) { }

Request::~Request() { }

int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

int	Request::checkHeader(void)
{
	if (header.find("\r\n\r\n",0) != std::string::npos)
		return (1);
	return (0);
}

void	Request::appendHeader(std::string input)
{
	if (this->header.empty())
	{
		this->header = input;
	}
	else
	{
		this->header = this->header + input;
	}
}

int	Request::checkBodySize(void)
{
	std::string content_length;
	size_t	type_start = header.find("Content-Length: ") + strlen("Content-Length: ");
	size_t	type_end = type_start;
	while(header[type_end] != '\n')
		type_end++;
	content_length = header.substr(type_start, type_end - type_start - 1);
	return (std::stoi(content_length));
}

void	Request::appendBody(char *body_in, int size)
{
	std::string tmp(body_in, size);
	this->body += tmp;
	//std::cout << this->body << std::endl;
}

void	Request::setRequestKey(unsigned int KeyIn)
{
	this->requestKey = KeyIn;
}

void	Request::clearBody()
{
	this->body = "";
}

void	Request::clearHeader()
{
	this->header = "";
}
