#include "../inc/Request.hpp"
#include "../inc/PostResponder.hpp"
#include <iostream>
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

void	Request::appendBody(std::string body_in)
{
	if (this->body.empty())
	{
		this->body = body_in;
	}
	else
	{
		//body_in = body_in.substr(0, body_in.size()-1);
		//if (body_in.empty())
		//	body_in = "NULL";
		this->body = this->body + body_in;
	}
	const char *tmp = body_in.c_str();
	this->body_flex.push_back(tmp[0]);
}

void	Request::setRequestKey(unsigned int KeyIn)
{
	this->requestKey = KeyIn;
}

void	Request::setBody()
{
	this->body = "";
	this->body_flex.clear();
}

void	Request::setHeader()
{
	this->header = "";
}

int Request::checkBoundaryStart() const
{
	int i = 0, count = 0;
	while (count <= 26)
	{
		if (this->body_flex[i] == '\r' && this->body_flex[i + 1] == '\n'
			&& this->body_flex[i + 2] == '\r' && this->body_flex[i + 3] == '\n')
		return (i + 3);
		i++;
	}
	return (0);
}

//calculate boundary from the end and return the offset to write my file
int Request::checkBoundaryEnd() const
{
	int max_size = this->body_flex.size();
	int i = max_size - 1, count = 0;
	while (count <= 26)
	{
		if (this->body_flex[i] == '-')
			count++;
		i--;
	}
	return (i - 2);
}

void	Request::createFileFlex() const
{
	FILE * fd = fopen("binary_boundary.file", "wb");
	LOG_BLUE("START BODY FLEX");
	int max_size = this->body_flex.size();
	char tmp;
	int minimum = checkBoundaryStart();
	int maximum = checkBoundaryEnd();
	for (int i = 0; i < max_size; i++)
	{
		std ::cout << body_flex[i];
		tmp = body_flex[i];
		if (i > minimum && i < maximum)
			fwrite (&tmp , 1, 1, fd);
	}
	LOG_BLUE("END BODY FLEX");

	fclose(fd);
}