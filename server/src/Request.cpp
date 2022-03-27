#include "Request.hpp"


void	Request::init() {
	header = "";
	body = "";
	bytes_read = 0;
	header_read = false;
	body_read = false;
}

Request::Request() { init(); }

Request::Request(int socket, Server * server) : socket(socket), server(server) { init(); }

Request::~Request() { }

int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

int	Request::checkHeaderRead(void) {
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

void	Request::changePath() {
	for (std::map<std::string, Location *>::reverse_iterator riter = server->locations.rbegin(); riter != server->locations.rend(); ++riter) {
		if (path.find(riter->first) == 0) {
			path = path.substr(riter->first.length(), std::string::npos);
			path = riter->second->root + path;
			LOG_BLUE("after replace: |" << path << "|");
			break ;
		}
	}
	path = server->root + path;
}

void	Request::setPath() {
	size_t posBegin = header.find("/");
	size_t posEnd = header.find_first_of(" \t", posBegin + 1);
	path = header.substr(posBegin, posEnd - posBegin);
	LOG_BLUE("before replace: |" << path << "|");
}

int	Request::readRequest() { // TODO check if request is allowed, otherwise return DECLINE
	//server->updateFilesHTML(); // TODO put to uesful position
	if (!header_read) {
		readHeader();
		if (header_read){
			setType();
			setPath();
			changePath();
			LOG("------- REQUEST KEY: " << getRequestKey() << " -------");
			LOG_RED(getHeader());
			std::cout << "HEADER END" << std::endl;
			if (body_read) {
				LOG_GREEN("read all in one");
				return DONE;
			}
		}
	}
	else if (header_read && getRequestKey() == POST) {
		if (!body_read)
			readBody();
		if (body_read) {
			return DONE;
		}
	}
	if (header_read && getRequestKey() == GET) {
		return DONE;
	}
	return WORKING;
}

int	Request::writeRequest() {
	if (header_read && getRequestKey() == POST) {
		PostResponder pR(getHeader(), getBody(), socket, server);
	}
	if (header_read && getRequestKey() == GET) {
		responder();
	}
	return DONE;
}

int	Request::checkBodySize(void) {
	std::string content_length;
	size_t	type_start = header.find("Content-Length: ") + strlen("Content-Length: ");
	size_t	type_end = type_start;
	while(header[type_end] != '\n')
		type_end++;
	content_length = header.substr(type_start, type_end - type_start - 1); // TODO protect when content_lengt not written in header
	return (std::stoi(content_length)); // TODO is std 11 function
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
	char	buffer[10000]; // TODO if buffer[5000] not possible to upload felix.jpg with browser
	memset(buffer, 0, 10000 * sizeof(char));
	ssize_t bytes_read = recv(socket, buffer, 10000, 0);
	std::cout << "header read bytes: " << bytes_read << "\n";
	appendHeader(buffer);

	if (checkHeaderRead()) {
		header_read = true;
	}
	size_t	posHeaderEnd = header.find("\r\n\r\n");
	if (posHeaderEnd != header.size() - 4) {
		LOG_YELLOW("takes body out of header");
		body = header;
		body.erase(0, posHeaderEnd + 4);

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
	//write(socket, "HTTP/1.1 100 Continue\r\n\r\n", 25); // much faster when sending huge files with curl; not allowed without checking with select for write rights
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
	if ((int)body.size() == checkBodySize()) {
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
	size_t			found;
	char			c;
	if ((found = filename.find("?",0)) != std::string::npos)
		filename = filename.substr(0,found);
	newFile.open(filename, std::ios::in);
	if (!newFile)
		return "";
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
	std::string	file_content;
	std::string	formatted;

	/* HARD CODED START */
	LOG_PINK("path of presented file: |" << path << "|");
	LOG_PINK(server->root);
	if (path == (server->root + "/"))
	{
		LOG_PINK("ENTERED HERE!");
		file_content = readFile("www/server7/index.html");
		formatted = formatString(file_content);
	}
	/* HARD CODED END */
	else
	{
		file_content = readFile(path.substr(1, std::string::npos));
		if (file_content.empty())
			formatted = formatString("error: 404");
		else
			formatted = formatString(file_content);
	}
	write(socket, formatted.c_str(), formatted.length());
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