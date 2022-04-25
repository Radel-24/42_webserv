#include "Request.hpp"

void	Request::init() {
	header.clear();
	body.clear();
	bytes_read = 0;
	location = NULL;
	status = READING_HEADER;
	requestKey = NIL;
	cgi_request = false;
	file_created = false;
	pr = NULL;
	headerValues.clear();
	path.clear();
	uploadPath.clear();
	filename.clear();
	closeConnection = false;
	clearResponse();
	newClient = false;
}

Request::Request() { init(); }

Request::Request(int socket, Server * server) : socket(socket), server(server) { init(); }

Request::~Request() { close(socket); }

void	Request::readRequest(std::map<int, Server *> & servers) {
	if (status == READING_HEADER) {
		readHeader();
		if (status == HEADER_READ) {
			LOG_GREEN_INFO("header read");
			processHeader(servers);
			if (status >= 100) {
				return ;
			}
			if (getRequestKey() == GET || getRequestKey() == HEAD || getRequestKey() == DELETE) {
				status = DONE_READING;
				return ;
			}
		}
	}
	else if (status == HEADER_READ && (getRequestKey() == POST || getRequestKey() == PUT)) {
		if (headerValues.find("Transfer-Encoding")->second == "chunked") {
			readBodyChunked();
		}
		else {
			readBodyLength();
		}
	}
}

void	Request::createCookie() {
	if (cookie.empty() || server->cookies.find(cookie) == server->cookies.end()) {
		uint64_t id = reinterpret_cast<uint64_t>(this);
		cookie = IntToHex(id);
		server->cookies.insert(cookie);
		newClient = true;
		LOG_YELLOW("NEWCLIENT RECOGNISED " << cookie);
	}
}

int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

int	Request::checkHeaderRead(void) {
	if (header.find("\r\n\r\n") != std::string::npos)
		return (1);
	return (0);
}

void	Request::appendHeader(char * input, size_t size) {
	std::string tmp(input, size);
	this->header += tmp;
}

void	Request::changePath() { // TODO make hacking save when relative path is given in request
	for (std::map<std::string, Location *>::reverse_iterator riter = server->locations.rbegin(); riter != server->locations.rend(); ++riter) {
		if (path.find(riter->first) == 0) {
			location = riter->second;
			if (location->root.empty() == false) {
				path = path.substr(riter->first.length(), std::string::npos);
				path = riter->second->root + path;
			}
			if (dirExists(path.c_str())) {
				path += "/" + location->default_file;
			}
			break ;
		}
	}
	path = server->root + path;
}

void	Request::setPath() {
	size_t posBegin = header.find("/");
	size_t posEnd = header.find_first_of(" \t?", posBegin + 1);
	if (posEnd != std::string::npos) {
		headerKeyValuePairs = header.substr(posEnd + 1, header.find_first_of(" \t", posEnd + 1) - posEnd - 1);
	}
	if (posBegin == std::string::npos || posEnd == std::string::npos) { // TODO usually not needed, except when header is wrong
		path = "";
		LOG_RED_INFO("shit path not found");
		return ;
	}
	path = header.substr(posBegin, posEnd - posBegin);
	uploadPath = path;
	if (path.length() >= server->cgi_extension.length() && (path.compare(path.length() - server->cgi_extension.length(), server->cgi_extension.length(), server->cgi_extension)) == 0){
		cgi_request = true;
	}
}

void	Request::setHeaderValues( const std::pair<std::string, std::string> &pair ) {
	this->headerValues.insert(pair);
}

std::string	&Request::leftTrim( std::string &str, char c ) {
	str.erase(0, str.find_first_not_of(c));
	return str;
}

std::pair<std::string, std::string>	Request::splitToken( std::string token ) {
	size_t	pos = 0;

	pos = token.find(':');
	std::string	key = token.substr(0, pos);
	std::string	value = token.substr(pos + 1, token.length() - pos - 1);
	key = leftTrim(key, ' ');
	value = leftTrim(value, ' ');
	return std::make_pair(key, value);
}


void	Request::checkHeaderValues( void ) {
	if (headerValues.find("Host") == headerValues.end()) {
		status = 400;
		LOG_RED_INFO("ERROR: request is missing host");
	}
	std::map<std::string, std::string>::iterator iter = headerValues.find("Cookie");
	if (iter != headerValues.end()) {
		cookie = iter->second.substr(iter->second.find_last_of("=") + 1, std::string::npos);
		LOG_RED_INFO("cookie already set: " << cookie);
	}
}

void	Request::parseHeader(std::string header) {
	std::string							delimiter = "\n";
	size_t								pos = 0;
	std::string							token;
	std::pair<std::string, std::string>	thePair;

	while ((pos = header.find(delimiter)) != std::string::npos) {
		token = header.substr(0, pos - 1);
		if (token.find(":") != std::string::npos)
		{
			thePair = splitToken(token);
			setHeaderValues(thePair);
		}
		header.erase(0, pos + delimiter.length());
	}
}

/* if there is a server that has a fitting name to the request, the request hast to get forwarded to that server */
/* else we use the default server, which is the first from the config file, that uses the same port */
void Request::detectCorrectServer(std::map<int, Server *> & servers) {
	std::map<int, Server *>::iterator	iter = servers.begin();
	while (iter != servers.end()) {
		if (iter->second->port == server->port && iter->second->server_name == getHostName()) {
			server = iter->second;
			return;
		}
		++iter;
	}
}

void	Request::hundredStatus() {
	std::map<std::string, std::string>::iterator iter = headerValues.find("Expect");
	if (iter != headerValues.end()) {
		LOG_CYAN_INFO("expect found");
		if (iter->second == "100-continue") {
			status = 100;
		}
	}
	iter = headerValues.find("Connection");
	if (iter != headerValues.end()) {
		if (iter->second == "close") {
			LOG_CYAN_INFO("Connection close found");
			closeConnection = true;
		}
	}
}

void Request::checkRequest() {
	if (requestKey == NIL) { status = 405; }
	else if (requestKey == GET || requestKey == HEAD) {
		if (!findInVector(location->methods, std::string("GET"))) {
			status = 405; // compulsory method mustn't be deactivated: https://developer.mozilla.org/de/docs/Web/HTTP/Status
		}
	}
	else if (requestKey == POST) {
		if (!findInVector(location->methods, std::string("POST"))) {
			LOG_RED("error: POST not allowed");
			status = 405;
		}
	}
	else if (requestKey == PUT) {
		if (!findInVector(location->methods, std::string("PUT"))) {
			status = 405;
		}
	}
	else if (requestKey == DELETE) {
		if (!findInVector(location->methods, std::string("DELETE"))) {
			status = 405;
		}
	}
}

void	Request::extractFilename() {
	if ((uploadPath.length() > 1)
		&& (uploadPath.find_first_not_of("/") == std::string::npos)) { // need to this, otherwise we cant redirect to default file on root
		filename = "/";
		return ;
	}
	size_t			pos = uploadPath.find_last_of("/") + 1;
	std::string		file = uploadPath.substr(pos, uploadPath.length() - pos);
	filename = file;
}

void	Request::getBodyOutOfHeader() {
	size_t	posHeaderEnd = header.find("\r\n\r\n");
	if (posHeaderEnd != header.size() - 4) {
		body = header;

		header.erase(posHeaderEnd, std::string::npos);

		body.erase(0, posHeaderEnd + 4);

		if ((int)body.size() == checkBodySize()) { // transfer encoding length
			status = DONE_READING;
		}
		if (body.find("\r\n\r\n") != std::string::npos) { // transfer encoding chunked
			status = DONE_READING;
		}
	}
}

void	Request::processHeader(std::map<int, Server *> & servers) {
	getBodyOutOfHeader();
	parseHeader(header);
	checkHeaderValues();
	createCookie();
	detectCorrectServer(servers);
	setPath();
	changePath();
	setType();
	extractFilename();
	hundredStatus();
	checkRequest();
}

int	Request::checkBodySize(void) {
	std::string	content_length;
	size_t		type_start = header.find("Content-Length: ") + strlen("Content-Length: ");
	size_t		type_end = type_start;
	if (type_start == std::string::npos)
		LOG_RED_INFO("No Centent-Length in header");
	while(header[type_end] != '\n')
		type_end++;
	content_length = header.substr(type_start, type_end - type_start - 1);
	return (std::atol(content_length.c_str()));
}

void	Request::appendBody(char *body_in, size_t size) {
	std::string tmp(body_in, size);
	this->body += tmp;
}

void	Request::setRequestKey(unsigned int KeyIn) {
	this->requestKey = KeyIn;
}

void	Request::setType() {
	if (header.find("GET") == 0 ) { setRequestKey(GET); }
	else if (header.find("POST") == 0) { setRequestKey(POST); }
	else if (header.find("PUT") == 0) { setRequestKey(PUT); }
	else if (header.find("DELETE") == 0) { setRequestKey(DELETE);}
	//else if (header.find("HEAD") == 0) { setRequestKey(HEAD); }
	else { setRequestKey(NIL); }
}

void Request::readHeader() {
	int		buffer_size = 200000;
	char	buffer[buffer_size]; // TODO dirty fix so that POST tester doesn't fail at / because of broken pipe
	memset(buffer, 0, buffer_size * sizeof(char));
	ssize_t	bytes_read = recv(socket, buffer, buffer_size, 0);
	if (bytes_read == -1) {
		status = CLOSE_CONNECTION;
		LOG_RED_INFO("bytes read -1: error");
		return;
	}
	if (bytes_read == 0) {
		status = CLOSE_CONNECTION;
		LOG_PINK_INFO("bytes read 0: client closed connection");
	}
	appendHeader(buffer, bytes_read);

	if (checkHeaderRead()) {
		status = HEADER_READ;
	}
}

void	Request::readBodyChunked() {
	int		buffer_size = 200000;
	char *	read_body = NULL;

	read_body = new char[buffer_size];

	if (body.find("\r\n\r\n") != std::string::npos) {
		status = DONE_READING;
		delete read_body;
		return;
	}
	ssize_t tmp_bytes_read = recv(socket, read_body, buffer_size, 0);
	if (tmp_bytes_read > 0) {
		appendBody(read_body, tmp_bytes_read);
		bytes_read += tmp_bytes_read;
	}
	else if (tmp_bytes_read == 0) {
		LOG_YELLOW_INFO("CLIENT CLOSED CONNECTION: " << socket);
		status = CLOSE_CONNECTION;
		delete read_body;
		return;
	}
	else {
		LOG_RED_INFO("ERROR: recv"); // TODO this is weird shit
	}
	if (body.find("\r\n\r\n") != std::string::npos) {
		status = DONE_READING;
	}
	delete read_body;
}

void Request::readBodyLength() {
	int max_size = checkBodySize();
	if (max_size < 1) {
		status = DONE_READING;
		return ;
	}

	char * read_body = NULL;
	read_body = new char[max_size];
	ssize_t tmp_bytes_read = recv(socket, read_body, max_size, 0);
	if (tmp_bytes_read > 0) {
		appendBody(read_body, tmp_bytes_read);
		bytes_read += tmp_bytes_read;
	}
	else {
		LOG_YELLOW_INFO("CLIENT CLOSED CONNECTION: " << socket);
		status = CLOSE_CONNECTION;
		delete read_body;
		return;
	}
	if ((int)body.size() == checkBodySize()) {
		status = DONE_READING;
	}
	delete read_body;
}

std::string	Request::getHostName( void ) const {
	std::map<std::string, std::string>::const_iterator	iter = headerValues.begin();
	while (iter != headerValues.end())
	{
		if (iter->first == "Host")
			return iter->second;
		iter++;
	}
	return "";
}
