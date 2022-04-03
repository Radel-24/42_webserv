#include "PostResponder.hpp"
std::string ToHex(const std::string & s, bool upper_case /* = true */);

void	PostResponder::createUploadFile( std::string filename, std::string content )
{
	char * buf = getcwd(NULL, FILENAME_MAX);
	std::string cwd(buf);
	std::string path = cwd + server->root + server->uploadPath + "/" + filename;
	LOG_YELLOW("depug upload path: " << path);
	std::ofstream	file(path, std::ios_base::app);
	if (file.is_open()) {
		file << content; // else error
		// LOG_YELLOW("upload file is opened");
	}
	//std::cout << "HEX\n" << ToHex(content, 0) << "\n";
	file.close();
}

void	PostResponder::uploadFiles( void )
{
	int				pos = 0;
	int				pos2 = -1;
	std::string		cutBody;
	std::string		filename;
	std::string		name;
	std::string		content_type;
	std::string		bodyContent;

	// LOG_RED("_numOfBoundaries :	" << _numOfBoundaries);
	// LOG_RED("_boundary :		" << _boundary);

	while (_numOfBoundaries > 1)
	{
		pos = _body.find(_boundary, pos2 + 1) + _boundary.length() + 2;
		pos2 = _body.find(_boundary, pos + 1);
		while (_body[--pos2] == '-')
			continue ;
		pos2 -= 1;


		cutBody = _body.substr(pos, pos2 - pos);


		size_t	file_start = cutBody.find("filename=") + strlen("filename=") + 1;
		size_t	file_end = file_start;
		while(cutBody[file_end] != '\n')
			file_end++;
		filename = cutBody.substr(file_start, file_end - file_start - 2);


		size_t	name_start = cutBody.find("name=") + strlen("name=") + 1;
		size_t	name_end = name_start;
		while(cutBody[name_end] != ';')
			name_end++;
		name = cutBody.substr(name_start, name_end - name_start - 1);


		size_t	type_start = cutBody.find("Content-Type: ") + strlen("Content-Type: ");
		size_t	type_end = type_start;
		while(cutBody[type_end] != '\n')
			type_end++;
		content_type = cutBody.substr(type_start, type_end - type_start - 1);


		size_t	dblNewline = cutBody.find("\r\n\r\n");
		bodyContent = cutBody.substr(dblNewline + 4, cutBody.length() - dblNewline - 4);

		// LOG_RED("content_type :		" << content_type);
		// LOG_RED("filename :		" << filename);
		// LOG_RED("name :			" << name);

		// remove new
		createUploadFile(filename, bodyContent);

		_numOfBoundaries--;

		std::string fileExtension = filename.substr(filename.find_last_of(".") + 1, std::string::npos);
		if (fileExtension == server->cgi_extension) {
			LOG_CYAN_INFO("this should be processed with cgi");
			std::string executionString = server->cgi_path + " " + "./cgi/cgi_tester";
			system(executionString.c_str());
			return ;
		}
	}
	server->updateFilesHTML();
}

int	PostResponder::countBoundaries( void )
{
	size_t	count = 0;
	size_t	pos = _body.find(_boundary, 0);

	while(pos != std::string::npos)
	{
		pos = _body.find(_boundary, pos + 1);
		count++;
	}
	return count;
}

std::string	PostResponder::extractBoundary( void )
{
	size_t	start = _header.find("boundary=") + strlen("boundary=");
	if (start == std::string::npos)
		return "error";

	while (!isalpha(_header[start]) && !isdigit(_header[start]))
		start++;

	size_t end = start;
	while ((isalpha(_header[end]) || isdigit(_header[end])) && _header[end])
		end++;
	return _header.substr(start, end - start);
}

int hex_to_decimal(std::string hex)
{
	std::transform(hex.begin(), hex.end(),hex.begin(), ::toupper);
	int len = hex.size();
	// Initializing base value to 1, i.e 16^0
	int base = 1;
	int dec_val = 0;
	// Extracting characters as digits from last
	// character
	for (int i = len - 1; i >= 0; i--) {
		// if character lies in '0'-'9', converting
		// it to integral 0-9 by subtracting 48 from
		// ASCII value
		if (hex[i] >= '0' && hex[i] <= '9') {
			dec_val += (int(hex[i]) - 48) * base;
			// incrementing base by power
			base = base * 16;
		}
		// if character lies in 'A'-'F' , converting
		// it to integral 10 - 15 by subtracting 55
		// from ASCII value
		else if (hex[i] >= 'A' && hex[i] <= 'F') {
			dec_val += (int(hex[i]) - 55) * base;
			// incrementing base by power
			base = base * 16;
		}
	}
	return dec_val;
}

//search body for first char until \r\n and hen convert it from hex to decimal
int	PostResponder::checkBodySizeChuncked(void) {
	std::string content_length_in_hex;
	size_t	type_start = _body.find("\r\n");
	content_length_in_hex = _body.substr(0, type_start);
	LOG_CYAN_INFO(content_length_in_hex);
	LOG_CYAN_INFO(hex_to_decimal(content_length_in_hex));
	return (hex_to_decimal(content_length_in_hex));
}

int	PostResponder::checkBodyStart(void) {
	std::string content_length_in_hex;
	size_t	type_start = _body.find("\r\n");
	content_length_in_hex = _body.substr(0, type_start);
	//LOG_RED(content_length_in_hex);
	return (content_length_in_hex.length());
}

int	PostResponder::extractStartChunk(void) {
	std::string content_length_in_hex;
	size_t	type_start = _body.find("\r\n") + 2;
	content_length_in_hex = _body.substr(0, type_start);
	//LOG_RED(content_length_in_hex);
	return (content_length_in_hex.length());
}

int	PostResponder::extractEndChunk(void) {
	size_t	type_end = checkBodySizeChuncked();
	//LOG_YELLOW(type_end);
	return (type_end);
}

PostResponder::PostResponder( std::string header, std::string body, int new_socket, Server * server ) : _header(header), _body(body), server(server)
{
	if (header.find("Transfer-Encoding: chunked") != std::string::npos)
	{
		//TO-DO create the right file name, and create new file per reuest atm im appending it
		LOG_YELLOW("chunked body!!!!");
		int start;
		int end;
		while (body.find("\r\n\r\n") != std::string::npos)
		{
			start = extractStartChunk();
			//LOG_RED(start);
			end = extractEndChunk();
			//LOG_RED(end);
			if (end == 3)
				break;
			createUploadFile("Felix", body.substr(start, end));
			//LOG_BLACK(body);
			body = body.substr(end + 2 + extractStartChunk(),body.length());
			//LOG_BLACK(body);
			_body = body;
		}
		writeToSocket(new_socket, "HTTP/1.1 201 Created\r\n\r\n");
		return ;
	}
	if (body.size() == 0) {
		LOG_RED_INFO("empty body in post request");
		LOG_RED_INFO(header);
		writeToSocket(new_socket, "HTTP/1.1 204 No Content");
		return ;
	}
	_boundary = extractBoundary();
	if (_boundary == "error")
	{
		// es gibt kein boundary, also wuden keine files geschickt und ich muss irgendwas anderes tun
		writeToSocket(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: extractBoundary");
		return ;
	}

	_numOfBoundaries = countBoundaries();
	if (!_numOfBoundaries)
	{
		// kann eigentlich nicht sein, keine ahnung was dann passieren soll mrrrrrrkkk
		writeToSocket(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: countBoundaries");
		return ;
	}

	if (_numOfBoundaries > 0)
	{
		uploadFiles();
		writeToSocket(new_socket, "HTTP/1.1 201 Created\r\n\r\n");
		return ;
	}

	//write(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 16\n\nfile was created", 80);
	char redirection[] = "HTTP/1.1 301 Moved Permanently\nLocation: http://127.0.0.1:1000/index.html\n\n"; //TODO include path from rerouting

	writeToSocket(new_socket, redirection);
}

