#include "Request.hpp"
#include "PostResponder.hpp"


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
			// LOG_BLUE("after replace: |" << path << "|");
			break ;
		}
	}
	path = server->root + path;
}

void	Request::setPath() {
	size_t posBegin = header.find("/");
	size_t posEnd = header.find_first_of(" \t", posBegin + 1);
	path = header.substr(posBegin, posEnd - posBegin);
	// LOG_BLUE("before replace: |" << path << "|");
}

/* START ALEX NEW */
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

/* currently only checking if host is included */
int	Request::checkHeaderValues( void )
{
	bool											flag = false;
	std::map<std::string, std::string>::iterator	iter = headerValues.begin();

	while (iter != headerValues.end()) {
		if (iter->first == "Host")
			flag = true;
		++iter;
	}
	if (flag == false)
	{
		LOG_RED("error: request is missing host");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void	Request::parseHeader( std::string header )
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
	int check = checkHeaderValues();
	LOG_BLACK("check: " << check);
	// if check == 1 muessen error pages returned werden, je nachdm was falsch am header ist
}
/* END ALEX NEW */

int	Request::readRequest() { // TODO check if request is allowed, otherwise return DECLINE
	//server->updateFilesHTML(); // TODO put to uesful position
	LOG_PINK_INFO("server name: " << getServer()->server_name);
	LOG_PINK_INFO("sock: " << getServer()->sock);
	if (!header_read) {
		readHeader();
		if (header_read) {
			setType();
			setPath();
			changePath();
			// LOG_RED_INFO(getRequestKey());
			LOG_WHITE(getHeader());
			// start new alex
			// LOG_GREEN("START HEADER VALUES");
			parseHeader(getHeader());
			// printHeaderValues();
			// LOG_GREEN("END HEADER VALUES");
			LOG_BLUE("HEADER END ------------------------");
			// end new alex
			if (body_read) {
				LOG_GREEN("read all in one");
				return DONE;
			}
		}
	}
	if (header_read && getRequestKey() == DELETE) {
		return DONE;
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
	if (header_read && getRequestKey() == DELETE) {
		deleteResponder();
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
	// LOG_RED("find: " << header.find("DELETE"));
	if (header.length() < 3) { setRequestKey(NIL); }
	else if (header.find("GET") == 0 ) { setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") == 0) { setRequestKey(POST); }
	else if (header.find("PUT") == 0) { setRequestKey(PUT); }
	else if (header.find("DELETE") == 0) { setRequestKey(DELETE);}
	else { setRequestKey(NIL); }
}

void Request::readHeader() {
	std::cout << std::endl;
	LOG_BLUE("HEADER START ----------------------");
	char	buffer[10000]; // TODO if buffer[5000] not possible to upload felix.jpg with browser
	memset(buffer, 0, 10000 * sizeof(char));
	ssize_t bytes_read = recv(socket, buffer, 10000, 0);
	LOG_BLACK("header read bytes: " << bytes_read << std::endl);
	appendHeader(buffer);

	if (checkHeaderRead()) {
		header_read = true;
	}
	size_t	posHeaderEnd = header.find("\r\n\r\n");
	if (posHeaderEnd != header.size() - 4) {
		LOG_BLACK("takes body out of header");
		body = header;
		body.erase(0, posHeaderEnd + 4);

		LOG_BLACK("body size " << body.size() << "check size " << checkBodySize());
		if ((int)body.size() == checkBodySize()) {
			body_read = true;
		}
	}
}

void Request::readBody() {
	LOG_CYAN(std::endl << "BODY START ----------------------");

	int max_size = checkBodySize();
	LOG_BLACK("body size: " << max_size);
	if (max_size > server->client_max_body_size)
	{
		LOG_RED("error: BODY TO BIG!");
		// give back some type of error page
	}
	char * read_body = NULL;
	read_body = new char[max_size];
	write(socket, "HTTP/1.1 100 Continue\r\n\r\n", 25); // much faster when sending huge files with curl // TODO only send when required
	LOG_BLACK("bytes read before recv: " << bytes_read);
	ssize_t tmp_bytes_read = recv(socket, read_body, max_size, 0);
	if (tmp_bytes_read > 0) {
		appendBody(read_body, tmp_bytes_read);
		bytes_read += tmp_bytes_read;
	}
	else {
		LOG_RED("weird shit going on with select");
	}
	LOG_BLACK("bytes read after recv: " << bytes_read);
	if ((int)body.size() == checkBodySize()) {
		body_read = true;
		//std::cout << getBody() << std::endl;
		LOG_BLACK("body read true");
	}
	delete read_body;

	LOG_CYAN(std::endl << "BODY END ------------------------");
}


// TODO put following 2 functions into utils file
std::string	readFile( std::string filename ) {
	std::ifstream	newFile;
	std::string		ret;
	std::string		binary = "/Users/fharing/42/webserv/server/";
	std::string		values;
	std::string		execute = "/Users/fharing/42/webserv/server/cgi/php-cgi -f ";
	size_t			found;
	char			c;
	if ((found = filename.find("cgi/", 0)) != std::string::npos)
	{
		if ((found = filename.find("?",0)) != std::string::npos)
		{
			binary = binary + filename.substr(0,found);
			values = filename.substr(found + 1,filename.length());
			std::replace(values.begin(),values.end(), '&', ' ');
		}
		execute = execute + binary + " " + values + " > out";
		std::cout << execute << std::endl;
		system(execute.c_str());
		return "EXEC";
	}

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

/* start alex new */
// can only delete one file
void	Request::deleteResponder( void ) {
	LOG_RED_INFO("deleteRequest starts here ------------------");
	std::string	filename = getFilename();
	std::string	deleteRoute = "." + server->root + server->uploadPath + "/" + filename;
	LOG_CYAN_INFO("deleteRoute: " << deleteRoute);
	if (std::ifstream(deleteRoute)) {
		std::remove(deleteRoute.c_str());
		if (std::ifstream(deleteRoute))
			LOG_RED_INFO("error: deleting file");
		else
			LOG_GREEN_INFO("file deleted");
	}
	else
		LOG_RED_INFO("error: file not found");
	LOG_RED_INFO("deleteRequest ends here --------------------");
}
/* end alex new */

void	Request::responder() {
	std::string	file_content;
	std::string	formatted;

	if (path == (server->root + "/"))
	{
		file_content = readFile( "." + server->root + "/index.html");
		formatted = formatString(file_content);
	}
	else
	{
		file_content = readFile(path.substr(1, std::string::npos));
		if (file_content.empty())
			formatted = formatString("error: 404");
		else
			formatted = formatString(file_content);
	}
	if (file_content == "EXEC")
	{
		file_content = readFile("/Users/fharing/42/webserv/server/out");
		formatted = formatString(file_content);
		write(socket, formatted.c_str(), formatted.length());
		close(socket); // TODO is this good?
		return;
	}
	formatted = formatString(file_content);
	std::cout << formatted << std::endl;
	write(socket, formatted.c_str(), formatted.length());
}

std::string	Request::getFilename() {
	std::string	converted = std::string(getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	LOG_CYAN_INFO("filename: " << file);
	return file;
}