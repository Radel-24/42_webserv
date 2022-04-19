#include "Request.hpp"
#include "PostResponder.hpp"

#include <sys/stat.h>

void	Request::init() {
	header = "";
	body = "";
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

void	Request::changePath() { // TODO make hacking save when relative path is given in request
	for (std::map<std::string, Location *>::reverse_iterator riter = server->locations.rbegin(); riter != server->locations.rend(); ++riter) {
		if (path.find(riter->first) == 0) {
			LOG_CYAN_INFO("path: " << path << " riter first " << riter->first);
			location = riter->second;
			LOG_CYAN_INFO("root: " << riter->second->root);
			LOG_CYAN_INFO("path: " << path);
			path = path.substr(riter->first.length(), std::string::npos);
			path = riter->second->root + path;
			LOG_CYAN_INFO("modified path: " << path);
			struct stat path_stat; // TODO, I don't think this is allowed
			stat(path.c_str(), &path_stat);
			if (S_ISDIR(path_stat.st_mode)) {
				path += "/" + location->default_file;
				LOG_CYAN_INFO("default file request: " << path);
			}
			// LOG_BLUE("after replace: |" << path << "|");
			break ;
		}
	}
	path = server->root + path;
}

void	Request::setPath() {
	size_t posBegin = header.find("/");
	size_t posEnd = header.find_first_of(" \t", posBegin + 1);
	if (posBegin == std::string::npos || posEnd == std::string::npos) { // TODO usually not needed, except when header is wrong
		path = "";
		LOG_RED_INFO("shit path not found");
		return ;
	}
	path = header.substr(posBegin, posEnd - posBegin);
	LOG_GREEN_INFO("path: |" << path << "|");
	LOG_GREEN_INFO("find: " << path.find_last_of(server->cgi_extension));
	LOG_GREEN_INFO("length path: " << path.length() << " cgi Length " << server->cgi_extension.length());
	if (path.find_last_of(server->cgi_extension) == (path.length() - 1)) {
		cgi_request = true;
	}
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


void	Request::checkHeaderValues( void )
{
	LOG_GREEN_INFO("request key: " << requestKey);
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

	// if check == 1 muessen error pages returned werden, je nachdm was falsch am header ist
}
/* END ALEX NEW */


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

	if (requestKey == NIL) { status = 405; }
	else if (requestKey == GET) {
		if (!findInVector(location->methods, std::string("GET")))
			status = 405; // TODO compulsory method mustn't be deactivated: https://developer.mozilla.org/de/docs/Web/HTTP/Status
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

void	Request::readRequest(std::map<int, Server *> & servers) { // TODO check if request is allowed, otherwise return DECLINE
	//server->updateFilesHTML(); // TODO put to uesful position
	//LOG_PINK_INFO("server name: " << getServer()->server_name);
	//LOG_PINK_INFO("sock: " << getServer()->sock);
	//LOG_RED_INFO("request status " << status << " request key " << requestKey);
	if (status == READING_HEADER) {
		readHeader();
		if (status == HEADER_READ) {
			//LOG_GREEN_INFO(header);
			parseHeader(header);
			checkHeaderValues();
			printHeaderValues();
			if (status >= 100)
				return ;
			detectCorrectServer(servers);
			setPath();
			changePath();
			setType();
			//LOG_YELLOW("START");
			//LOG_YELLOW(getHeader());
			//LOG_BLACK(getBody());
			//LOG_YELLOW("END");
			checkRequest();
			if (status >= 100)
				return ;
			// LOG_RED_INFO(getRequestKey());
			//LOG_WHITE(getHeader());
			// start new alex
			LOG_GREEN("START HEADER VALUES");
			printHeaderValues();
			LOG_GREEN("END HEADER VALUES");
			LOG_BLUE("HEADER END ------------------------");
			// end new alex
			if (status == DONE_READING) {
				LOG_GREEN("read all in one");
				return ;
			}
		}
	}
	else if (status == HEADER_READ && getRequestKey() == DELETE) {
		status = DONE_READING;
		return ;
	}
	if (status == HEADER_READ && (getRequestKey() == POST || getRequestKey() == PUT)) {
		if (headerValues.find("Transfer-Encoding")->second == "chunked") {
			//readBodyChunked();
			//LOG_GREEN_INFO("READ BODY CHUNKED");
			readBodyChunked();
		}
		else {
			LOG_GREEN_INFO("READ BODY LENGTH");
			readBodyLength();
		}
		if (status == DONE_READING) {
			return ;
		}
	}
	if (status == HEADER_READ && getRequestKey() == GET) {
		status = DONE_READING;
		return ;
	}
	//if (status == HEADER_READ && getRequestKey() == PUT) {
	//	status = DONE_READING;
	//	return ;
	//}
}

void	Request::writeRequest() {
	//LOG_RED_INFO("request status " << status);
	if (status >= 100 && status < 600) {
		writeStatus(status, socket);
		status =  DONE_WRITING;
	}
	else if (status == DONE_READING && (getRequestKey() == POST || getRequestKey() == PUT)) {
		if (!postResponder)
			postResponder = new PostResponder(*this);
		postResponder->run();
		//PostResponder pR(*this);
		return ;
	}
	else if (status == DONE_READING && getRequestKey() == GET) {
		responder();
		status =  DONE_WRITING;
	}
	else if (status == DONE_READING && getRequestKey() == DELETE) {
		deleteResponder();
		status =  DONE_WRITING;
	}
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

void	Request::appendBody(char *body_in, int size) {
	std::string tmp(body_in, size);
	this->body += tmp;
}

void	Request::setRequestKey(unsigned int KeyIn) {
	this->requestKey = KeyIn;
}

void	Request::setType() {
	// LOG_RED_INFO("find: " << header);
	if (header.find("GET") == 0 ) { setRequestKey(GET); }// if keyword not always at the beginning, us find("GET")
	else if (header.find("POST") == 0) { setRequestKey(POST); }
	else if (header.find("PUT") == 0) { setRequestKey(PUT); }
	else if (header.find("DELETE") == 0) { setRequestKey(DELETE);}
	else { setRequestKey(NIL); }
}

void Request::readHeader() {
	LOG_BLUE("HEADER START ----------------------");
	char	buffer[10000]; // TODO if buffer[5000] not possible to upload felix.jpg with browser
	memset(buffer, 0, 10000 * sizeof(char));
	ssize_t bytes_read = recv(socket, buffer, 10000, 0);
	LOG_BLACK("header read bytes: " << bytes_read << std::endl);
	appendHeader(buffer);

	if (checkHeaderRead()) {
		status = HEADER_READ;
		//LOG_WHITE_INFO(header);

		size_t	posHeaderEnd = header.find("\r\n\r\n");
		if (posHeaderEnd != header.size() - 4) {
			LOG_BLACK("takes body out of header; pos header end: " << posHeaderEnd);
			body = header;
			body.erase(0, posHeaderEnd + 4);
			//LOG_RED_INFO("header: " << header << "\nbody: " << body);

			LOG_BLACK("body size " << body.size() << "check size " << checkBodySize());
			if ((int)body.size() == checkBodySize()) {
				status = DONE_READING;
				//LOG_WHITE_INFO(header);
			}
		}
		else {
			LOG_BLACK_INFO("only header sent");
		}
	}
}

void	Request::readBodyChunked() {
	int buffer_size = 200000;
	//if (chunk_size == -1)
	//	buffer_size = 4096;
	//else
	//	buffer_size = chunk_size;
	char * read_body = NULL;
	read_body = new char[buffer_size];

	if (body.find("\r\n\r\n") != std::string::npos)
	{
		status = DONE_READING;
		return;
	}
	ssize_t tmp_bytes_read = recv(socket, read_body, buffer_size, 0);
	//if ()
	//std::string	sizeInfo =
	if (tmp_bytes_read > 0) {
		appendBody(read_body, tmp_bytes_read);
		bytes_read += tmp_bytes_read;
	}
	else {
		LOG_YELLOW("CLIENT CLOSED CONNECTION: " << socket);
		status = CLIENT_CLOSED_CONNECTION;
		//TO-DO close socket and delete out of socket list
		return;
	}
	//LOG_BLACK("bytes read after recv: " << bytes_read);
	if (body.find("\r\n\r\n") != std::string::npos) {
		status = DONE_READING;
		//std::cout << getBody() << std::endl;
		//LOG_BLACK("body read true" << body.size());
	}
	delete read_body;
	//LOG_BLUE_INFO(body);
	//LOG_CYAN(std::endl << "BODY END ------------------------");
}

void Request::readBodyLength() {
	//LOG_CYAN(std::endl << "BODY START ----------------------");

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
		LOG_YELLOW("CLIENT CLOSED CONNECTION: " << socket);
		status = CLIENT_CLOSED_CONNECTION;
		//TO-DO close socket and delete out of socket list
		return;
	}
	LOG_BLACK("bytes read after recv: " << bytes_read);
	LOG_BLACK("body size " << body.size());
	if ((int)body.size() == checkBodySize()) {
		status = DONE_READING;
		//std::cout << getBody() << std::endl;
		LOG_BLACK("body read true" << body.size());
	}
	delete read_body;
	//LOG_BLUE_INFO(body);
	//LOG_CYAN(std::endl << "BODY END ------------------------");
}


// TODO put following 2 functions into utils file
std::string	Request::readFile( std::string filename ) {
	std::ifstream	newFile;
	std::string		ret;
	// std::string		binary = "/Users/radelwar/42/webserv/server/";
	std::string		values;
	// std::string		execute = "/Users/radelwar/42/webserv/server/cgi/php-cgi -f ";
	// size_t			found;
	char			c;
	// if ((found = filename.find("cgi/", 0)) != std::string::npos)
	// {
	// 	if ((found = filename.find("?",0)) != std::string::npos)
	// 	{
	// 		binary = binary + filename.substr(0,found);
	// 		values = filename.substr(found + 1,filename.length());
	// 		std::replace(values.begin(),values.end(), '&', ' ');
	// 	}
	// 	execute = execute + binary + " " + values + " > out";
	// 	//std::cout << execute << std::endl;
	// 	system(execute.c_str());
	// 	return "EXEC";
	// }

	LOG_CYAN_INFO("trying to open: " << filename);
	if (open(filename.c_str(), std::ios::in) == -1) {
		status = 404;
		return "";
	}
	newFile.open(filename, std::ios::in);
	if (!newFile){
		return "";
	}
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
	//LOG_RED_INFO("response: " << ret);
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

	LOG_PINK_INFO("test:	" << path);
	struct stat path_stat; // TODO, I don't think this is allowed
	std::string	temp = "." + path;
	stat(temp.c_str(), &path_stat);
	if (S_ISDIR(path_stat.st_mode)) {
		path += "/" + location->default_file;
		LOG_CYAN_INFO("default dir request: " << path);
	}
	if (S_ISREG(path_stat.st_mode)) {
		//path += "/" + location->default_file;
		LOG_CYAN_INFO("default file request: " << path);
	}
	if (path == (server->root + "/"))
	{
		file_content = readFile( "." + server->root + "/index.html");
		formatted = formatString(file_content);
	}
	else
	{
		file_content = readFile(path.substr(1, std::string::npos));
		if (status == 404){
			LOG_RED_INFO("404 gets sent");
			writeToSocket(socket, "HTTP/1.1 404 Method Not Allowed\r\nContent-Length: 0\r\n\r\n");
			return ;
		}
		if (file_content.empty()) {
			formatted = formatString("error: 404"); //TODO check if this is reached
		}
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
	//std::cout << formatted << std::endl;
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