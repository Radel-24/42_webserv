#include "../inc/Request.hpp"
#include <iostream>
int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

void	Request::setHeader(std::string input) { header = input; }
void	Request::setBody(std::string body) { this->body = body; }
void	Request::setHeaderValues(std::map<std::string, std::string> mappe) { headerValues = mappe; }

void	Request::setRequestKey(unsigned int key) { requestKey = key; }

