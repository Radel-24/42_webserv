#include "Request.hpp"

void	Request::init() {
	//header = "";
	//body = "";
	header.clear();
	body.clear();
	bytes_read = 0;
	bytes_written = 0;
	location = NULL;
	status = READING_HEADER;
	requestKey = NIL;
	cgi_request = false;
	file_created = false;
	postResponder = NULL;
	headerValues.clear();
	path.clear();
	uploadPath.clear();
	chunk.clear();
	filename.clear();
}

Request::Request() { init(); }

Request::Request(int socket, Server * server) : socket(socket), server(server) { init(); }

Request::~Request() {
	close(socket);
}

void	Request::readRequest(std::map<int, Server *> & servers) {
	if (status == READING_HEADER) {
		readHeader();
		if (status == HEADER_READ) {
			processHeader(servers);
			if (status >= 100) {
				return ;
			}
			if (getRequestKey() == GET || getRequestKey() == HEAD || getRequestKey() == DELETE) {
				status = DONE_READING;
				LOG_GREEN("read all in one");
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

int	Request::getRequestKey() const { return requestKey; }

std::string	Request::getHeader() const { return header; }

std::string	Request::getBody() const { return body; }

int	Request::checkHeaderRead(void) {
	if (header.find("\r\n\r\n") != std::string::npos)
		return (1);
	return (0);
}

void	Request::appendHeader(char * input, size_t size) {
	//if (this->header.empty()) {
	//	this->header = input;
	//}
	//else {
		std::string tmp(input, size);
		//this->header = this->header + input;
		this->header += tmp;
	//}
}

void	Request::changePath() { // TODO make hacking save when relative path is given in request
	for (std::map<std::string, Location *>::reverse_iterator riter = server->locations.rbegin(); riter != server->locations.rend(); ++riter) {
		if (path.find(riter->first) == 0) {
			location = riter->second;
			path = path.substr(riter->first.length(), std::string::npos);
			path = riter->second->root + path;
			struct stat path_stat; // TODO, I don't think this is allowed
			stat(path.c_str(), &path_stat);
			if (S_ISDIR(path_stat.st_mode)) {
				path += "/" + location->default_file;
			}
			break ;
		}
	}
	path = server->root + path;
}

void	Request::setPath() {
	LOG_PINK_INFO("IN SET PATH" << header);
	size_t posBegin = header.find("/");
	size_t posEnd = header.find_first_of(" \t?", posBegin + 1);
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

/* maybe map needs to be reset after handling */
void	Request::setHeaderValues( const std::pair<std::string, std::string> &pair )
{
	this->headerValues.insert(pair);
}

std::string	&Request::leftTrim( std::string &str, char c )
{
	str.erase(0, str.find_first_not_of(c));
	return str;
}

std::pair<std::string, std::string>	Request::splitToken( std::string token )
{
	size_t	pos = 0;

	pos = token.find(':');
	std::string	key = token.substr(0, pos);
	std::string	value = token.substr(pos + 1, token.length() - pos - 1);
	key = leftTrim(key, ' ');
	value = leftTrim(value, ' ');
	return std::make_pair(key, value);
}


void	Request::checkHeaderValues( void )
{
	if (headerValues.find("Host") == headerValues.end()) {
		status = 400; // TODO don't know error code
		LOG_RED("error: request is missing host");
	}
	//if (headerValues.find("Content-Length") == headerValues.end()) {
	//	status = 411;
	//	LOG_RED("error: request is missing content length");
	//}
}

void	Request::parseHeader(std::string header)
{
	std::string							delimiter = "\n";
	size_t								pos = 0;
	std::string							token;
	std::pair<std::string, std::string>	thePair;

	while ((pos = header.find(delimiter)) != std::string::npos) {
		token = header.substr(0, pos - 1);
		// LOG_PINK("|" << token << "|");
		if (token.find(":") != std::string::npos)
		{
			thePair = splitToken(token);
			setHeaderValues(thePair);
		}
		header.erase(0, pos + delimiter.length());
	}

	// TODO if check == 1 muessen error pages returned werden, je nachdm was falsch am header ist
}


/* if there is a server that has a fitting name to the request, the request hast to get forwarded to that server */
/* else we use the default server, which is the first from the config file, that uses the same port */
/* PORT HAS TO FIT AS WELL !!!!!!!!!!!!!!!!!!!!! */
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
/* end alex new */

void Request::checkRequest() {
	LOG_RED_INFO("request key " << requestKey);
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
		else if (headerValues.find("Expect") != headerValues.end()) {
			LOG_CYAN_INFO("expect found");
			std::map<std::string, std::string>::iterator iter = headerValues.find("Expect");
			if (iter->second == "100-continue")
				status = 100;
		}

	}
	else if (requestKey == PUT) {
		if (!findInVector(location->methods, std::string("PUT")))
			status = 405;
	}
	else if (requestKey == DELETE) {
		if (!findInVector(location->methods, std::string("DELETE")))
			status = 405;
	}

	else if (location->client_max_body_size != -1 && checkBodySize() > location->client_max_body_size) {
		LOG_RED("error: BODY TO BIG!");
		status = 413; // TODO is this the right status??
	}

}

void	Request::extractFilename() {
	// LOG_WHITE("ABC: uploadPath: " << uploadPath);
	size_t	pos = uploadPath.find_last_of("/") + 1;
	std::string		file = uploadPath.substr(pos, uploadPath.length() - pos);
	// LOG_WHITE("ABC: filename: " << filename);
	// LOG_GREEN("ABC -----------");
	filename = file;
}

void	Request::getBodyOutOfHeader() {
	size_t	posHeaderEnd = header.find("\r\n\r\n");
	if (posHeaderEnd != header.size() - 4) {
		LOG_BLACK("takes body out of header; pos header end: " << posHeaderEnd);
		body = header;

		header.erase(posHeaderEnd, std::string::npos);

		body.erase(0, posHeaderEnd + 4);
		//LOG_RED_INFO("header: " << header << "\nbody: " << body);

		LOG_BLACK("body size " << body.size() << "check size " << checkBodySize());

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
	detectCorrectServer(servers);
	setPath();
	changePath();
	setType();
	extractFilename();
	checkRequest();
}

int	Request::checkBodySize(void) {
	std::string content_length;
	size_t	type_start = header.find("Content-Length: ") + strlen("Content-Length: ");
	size_t	type_end = type_start;

	while(header[type_end] != '\n')
		type_end++;
	content_length = header.substr(type_start, type_end - type_start - 1); // TODO protect when content_lengt not written in header
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
	if (header.find("GET") == 0 ) { setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") == 0) { setRequestKey(POST); }
	else if (header.find("PUT") == 0) { setRequestKey(PUT); }
	else if (header.find("DELETE") == 0) { setRequestKey(DELETE);}
	//else if (header.find("HEAD") == 0) { setRequestKey(HEAD); }
	else { setRequestKey(NIL); }
}

void Request::readHeader() {
	LOG_BLUE("HEADER START ----------------------");
	int buffer_size = 20000;
	char	buffer[buffer_size]; // TODO dirty fix so that POST tester doesn't fail at / because of broken pipe
	memset(buffer, 0, buffer_size * sizeof(char));
	ssize_t bytes_read = recv(socket, buffer, buffer_size, 0);
	LOG_BLACK("header read bytes: " << bytes_read << std::endl);
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
		//LOG_WHITE_INFO(header);

		
	}
}

void	Request::readBodyChunked() {
	int buffer_size = 200000;
	char * read_body = NULL;

	read_body = new char[buffer_size];

	if (body.find("\r\n\r\n") != std::string::npos)
	{
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
		LOG_YELLOW_INFO("CLIENT CLOSED CONNECTION: " << socket << " bytes read " << tmp_bytes_read);
		status = CLOSE_CONNECTION;
		delete read_body;
		return;
	}
	else {
		LOG_RED_INFO("recv error occured"); // TODO this is weird shit
	}
	if (body.find("\r\n\r\n") != std::string::npos) {
		status = DONE_READING;
	}
	delete read_body;
}

void Request::readBodyLength() {
	int max_size = checkBodySize();
	LOG_BLACK("body size: " << max_size);
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
		LOG_YELLOW_INFO("CLIENT CLOSED CONNECTION: " << socket << " bytes read " << tmp_bytes_read);
		status = CLOSE_CONNECTION;
		return;
	}
	if ((int)body.size() == checkBodySize()) {
		status = DONE_READING;
		//std::cout << getBody() << std::endl;
		LOG_BLACK("body read true" << body.size());
	}
	//delete read_body; hhh
}