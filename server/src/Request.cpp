#include "../inc/Request.hpp"
#include "../inc/PostResponder.hpp"
#include <iostream>

void	Request::init() {
	header = "";
	body = "";
	bytes_read = 0;
	header_read = false;
	body_read = false;
}

Request::Request() { init(); }

Request::Request(int socket) : socket(socket) { init(); }

Request::~Request() { }

int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

int	Request::checkHeader(void) {
	if (header.find("\r\n\r\n") != std::string::npos)
		return (1);
	return (0);
}

void	Request::appendHeader(std::string input) {
	if (this->header.empty()) {
		this->header = input;
	}
	else {
		this->header = this->header + input;
	}
}

int	Request::process() {
	if (!header_read) {
		readHeader();
		if (header_read){
			setType();
			LOG("------- REQUEST KEY: " << getRequestKey() << " -------");
			LOG_RED(getHeader());
			std::cout << "HEADER END" << std::endl;
		}
	}
	if (header_read && getRequestKey() == POST) {
		if (!body_read)
			readBody();
		if (body_read) {
			LOG_YELLOW("doing this");
			PostResponder pR(getHeader(), getBody(), socket);
			return DONE;
		}
	}
	if (header_read && getRequestKey() == GET) {
		responder();
		return DONE;
	}
	return WORKING;
}

int	Request::checkBodySize(void) {
	std::string content_length;
	size_t	type_start = header.find("Content-Length: ") + strlen("Content-Length: ");
	size_t	type_end = type_start;
	while(header[type_end] != '\n')
		type_end++;
	content_length = header.substr(type_start, type_end - type_start - 1);
	return (std::stoi(content_length));
}

void	Request::appendBody(char *body_in, int size) {
	std::string tmp(body_in, size);
	this->body += tmp;
}

void	Request::setRequestKey(unsigned int KeyIn) {
	this->requestKey = KeyIn;
}

void	Request::setType() {
	if (header.length() < 3) { setRequestKey(NIL); }
	else if (header.find("GET") == 0 ) { setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") == 0) { setRequestKey(POST); }
	else if (header.find("PUT") == 0) { setRequestKey(PUT); }
	else if (header.find("DELETE") == 0) { setRequestKey(DELETE);}
	else { setRequestKey(NIL); }
}

void Request::readHeader() {
	std::cout << "HEADER START" << std::endl;
	char	buffer[5000];
	memset(buffer, 0, 5000 * sizeof(char));
	ssize_t bytes_read = recv(socket, buffer, 5000, 0);
	std::cout << "header read bytes: " << bytes_read << "\n";
	appendHeader(buffer);

	if (checkHeader()) {
		header_read = true;
	}
	if (header.find("\r\n\r\n") != header.size() - 4) {
		LOG_YELLOW("takes body out of header");
		body = header.substr(header.find("\r\n\r\n") + 4, std::string::npos);
		LOG_YELLOW("body size " << body.size() << "check size " << checkBodySize());
		if ((int)body.size() == checkBodySize()) {
			body_read = true;
		}
	}
}

void Request::readBody() {
	std::cout << "BODY START" << std::endl;

	int max_size = checkBodySize();
	std::cout << "body size: " << max_size << std::endl;
	char * read_body = NULL;
	read_body = new char[max_size];
	write(socket, "HTTP/1.1 100 Continue\r\n\r\n", 25); // much faster when sending huge files with curl
	LOG_YELLOW("bytes read before recv: " << bytes_read);
	ssize_t tmp_bytes_read = recv(socket, read_body, max_size, 0);
	if (tmp_bytes_read > 0) {
		appendBody(read_body, tmp_bytes_read);
		bytes_read += tmp_bytes_read;
	}
	else {
		LOG_YELLOW("weird shit going on with select");
	}
	LOG_YELLOW("bytes read after recv: " << bytes_read);
	if (bytes_read == max_size) {
		body_read = true;
		//std::cout << getBody() << std::endl;
		LOG_YELLOW("body read true");
	}
	delete read_body;

	std::cout << "BODY END" << std::endl;
}


// TODO put following 2 functions into utils file
std::string	readFile( std::string filename ) {
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

std::string	formatString( std::string file_content ) {
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


void Request::responder() {
	std::string	filename;
	std::string	file_content;
	std::string	formatted;
	filename = getFilename();
	if (filename.empty()) {
		file_content = "alex ist sehr toll und du leider nicht so :(\n";
	}
	else {
		file_content = readFile(filename);
	}
	formatted = formatString(file_content);
	std::cout << formatted;
	write(socket, formatted.c_str(), formatted.length());
	close(socket); // TODO is this good?
}

std::string	Request::getFilename() {
	LOG("FILENAME START");
	std::string	converted = std::string(getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	LOG(file);
	return file;
}