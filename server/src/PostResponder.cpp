#include "PostResponder.hpp"

PostResponder::PostResponder(Request & request ) : request(request) { run(); }

void	PostResponder::uploadFiles( void )
{
	int				pos = 0;
	int				pos2 = -1;
	std::string		cutBody;
	std::string		filename;
	std::string		name;
	std::string		content_type;
	std::string		bodyContent;

	while (_numOfBoundaries > 1)
	{
		pos = request.body.find(_boundary, pos2 + 1) + _boundary.length() + 2;
		pos2 = request.body.find(_boundary, pos + 1);
		while (request.body[--pos2] == '-')
			continue ;
		pos2 -= 1;

		cutBody = request.body.substr(pos, pos2 - pos);

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

		std::string	cwd = getPWD();
		std::string path = cwd + request.server->root + request.server->uploadPath + "/" + filename;
		createUploadFile(path, bodyContent);

		_numOfBoundaries--;

		std::string fileExtension = filename.substr(filename.find_last_of(".") + 1, std::string::npos);
		if (fileExtension == request.server->cgi_extension) {
			LOG_CYAN_INFO("this should be processed with cgi");
			std::string executionString = request.server->cgi_path + " " + "./cgi/cgi_tester";
			system(executionString.c_str());
			return ;
		}
	}
}

int	PostResponder::countBoundaries( void )
{
	size_t	count = 0;
	size_t	pos = request.body.find(_boundary, 0);

	while(pos != std::string::npos) {
		pos = request.body.find(_boundary, pos + 1);
		count++;
	}
	return count;
}

std::string	PostResponder::extractBoundary( void )
{
	size_t	start = request.header.find("boundary=") + strlen("boundary=");
	if (start == std::string::npos)
		return "error";

	while (!isalpha(request.header[start]) && !isdigit(request.header[start]))
		start++;

	size_t end = start;
	while ((isalpha(request.header[end]) || isdigit(request.header[end])) && request.header[end])
		end++;
	return request.header.substr(start, end - start);
}

// search body for first char until \r\n and then convert it from hex to decimal
int	PostResponder::checkBodySizeChuncked(void) {
	std::string content_length_in_hex;
	size_t	type_start = request.body.find("\r\n");
	content_length_in_hex = request.body.substr(0, type_start);
	return (hex_to_decimal(content_length_in_hex));
}

int	PostResponder::checkBodyStart(void) {
	std::string content_length_in_hex;
	size_t	type_start = request.body.find("\r\n");
	content_length_in_hex = request.body.substr(0, type_start);
	return (content_length_in_hex.length());
}

int	PostResponder::extractStartChunk(void) {
	std::string content_length_in_hex;
	size_t	type_start = request.body.find("\r\n") + 2;
	content_length_in_hex = request.body.substr(0, type_start);
	return (content_length_in_hex.length());
}

int	PostResponder::extractEndChunk(void) {
	size_t	type_end = checkBodySizeChuncked();
	return (type_end);
}

void PostResponder::run() {
	if (request.header.find("Transfer-Encoding: chunked") != std::string::npos && request.file_created == false)
	{
		LOG_YELLOW_INFO("chunked body");
		std::string cleanBody;

		int i = 0;
		std::string tmp;
		std::istringstream tmp_body;
		tmp_body.str(request.body);
		std::getline(tmp_body, cleanBody);
		LOG_YELLOW_INFO("START LOOP");
		while(cleanBody.front() != '0' && cleanBody.size() != 1) {
			if (i % 2) {
				cleanBody.pop_back();
				tmp.append(cleanBody);
			}
			i++;
			std::getline(tmp_body, cleanBody);
		}
		request.body.clear();
		request.body = tmp;
		LOG_YELLOW_INFO("END LOOP");

		if (request.location && request.location->client_max_body_size != -1 && request.body.length() > (unsigned long)request.location->client_max_body_size) {
			LOG_RED_INFO("error: BODY TO BIG!");
			request.response = writeStatus(413, request);
			return ;
		}

		if (!request.cgi_request) {
			LOG_YELLOW_INFO("CREATE FILE");
			std::string filename = request.filename;
			std::string cwd = getPWD();
			std::string path = cwd + request.server->root + request.server->uploadPath + "/" + filename;
			emptyUploadFile(path);
			createUploadFile(path, request.body);
			LOG_YELLOW_INFO("FILE CREATED");
		}

		if (request.cgi_request) {
			LOG_GREEN_INFO("RUN CGI");
			Cgi cgi(request);
			LOG_GREEN_INFO("END CGI");
			return ;
		}
		request.response = writeStatus(201, request);
		return ;
	}
	if (request.body.size() == 0) {
		LOG_RED_INFO("empty body in post request");
		request.response = writeStatus(204, request);
		return ;
	}
	_boundary = extractBoundary();
	if (request.location && request.location->client_max_body_size != -1 && request.body.length() > (unsigned long)request.location->client_max_body_size) {
		LOG_RED_INFO("error: BODY TO BIG!");
		request.response = writeStatus(413, request);
		return ;
	}
	if (_boundary == "error") {
		request.response = writeStatus(200, request);
		return ;
	}

	_numOfBoundaries = countBoundaries();
	if (!_numOfBoundaries) {
		request.response = request.response = writeStatus(200, request);
		return ;
	}

	if (_numOfBoundaries > 0) {
		uploadFiles();
		if (request.cgi_request) {
			Cgi cgi(request);
			return ;
		}
		request.response = writeStatus(201, request);
		return ;
	}
}
