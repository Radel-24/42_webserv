#include "PostResponder.hpp"
std::string ToHex(const std::string & s, bool upper_case /* = true */);

void	PostResponder::emptyUploadFile( std::string filename )
{
	char * buf = getcwd(NULL, FILENAME_MAX);
	std::string cwd(buf);
	std::string path = cwd + request.server->root + request.server->uploadPath + "/" + filename;
	//LOG_YELLOW("depug upload path: " << path);
	std::ofstream	file(path, std::ios_base::out);
	if (file.is_open()) {
		file << ""; // else error
		// LOG_YELLOW("upload file is opened");
	}
	//std::cout << "HEX\n" << ToHex(content, 0) << "\n";
	file.close();
}

void	PostResponder::createUploadFile( std::string filename, std::string content )
{
	char * buf = getcwd(NULL, FILENAME_MAX);
	std::string cwd(buf);
	std::string path = cwd + request.server->root + request.server->uploadPath + "/" + filename;
	//LOG_YELLOW("depug upload path: " << path);
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

		// remove new
		createUploadFile(filename, bodyContent);

		_numOfBoundaries--;

		std::string fileExtension = filename.substr(filename.find_last_of(".") + 1, std::string::npos);
		if (fileExtension == request.server->cgi_extension) {
			LOG_CYAN_INFO("this should be processed with cgi");
			std::string executionString = request.server->cgi_path + " " + "./cgi/cgi_tester";
			system(executionString.c_str());
			return ;
		}
	}
	request.server->updateFilesHTML();
}

int	PostResponder::countBoundaries( void )
{
	size_t	count = 0;
	size_t	pos = request.body.find(_boundary, 0);

	while(pos != std::string::npos)
	{
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

//search body for first char until \r\n and hen convert it from hex to decimal
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
	//LOG_RED(content_length_in_hex);
	return (content_length_in_hex.length());
}

int	PostResponder::extractStartChunk(void) {
	std::string content_length_in_hex;
	size_t	type_start = request.body.find("\r\n") + 2;
	content_length_in_hex = request.body.substr(0, type_start);
	//LOG_RED(content_length_in_hex);
	return (content_length_in_hex.length());
}

int	PostResponder::extractEndChunk(void) {
	size_t	type_end = checkBodySizeChuncked();
	//LOG_YELLOW(type_end);
	return (type_end);
}

/*				TIMINGS
AFTER 50-60 SEC CHUNKED BODY IS PRINTED
AFTER 3-5 SEC CHUNKED BODY WITH THIS LOGIC IS OVER
WRITE BODY TO FILE TAKES 20 SECS
-> SUM OF 1:25 MINUTES TO UPLOAD 100MB -> needs to be like max 5-10 secs
*/
PostResponder::PostResponder(Request & request ) : request(request)
{
}

void PostResponder::run() {
	//LOG_YELLOW("START");
	//LOG_YELLOW(request.header);
	//LOG_YELLOW("END");
	//----------------------------------------------------------------------------------------------------------------------------
	if (request.header.find("Transfer-Encoding: chunked") != std::string::npos && request.file_created == false)
	{
		//TO-DO create the right file name, and create new file per reuest atm im appending it
		LOG_YELLOW("chunked body!!!!");
		std::string cleanBody;

		int i = 0;
		std::string tmp;
		std::istringstream tmp_body;
		tmp_body.str(request.body);
		std::getline(tmp_body, cleanBody);
		LOG_YELLOW("START LOOP");
		while(cleanBody.front() != '0' && cleanBody.size() != 1)
		{
			if (i % 2)
			{
				cleanBody.pop_back();
				tmp.append(cleanBody);
			}
			i++;
			std::getline(tmp_body, cleanBody);
		}
		request.body.clear();
		request.body = tmp;
		LOG_YELLOW("END LOOP");



		//TO-DO check server config file(server class) if something is forbidden/allowd and return error code!
		int method = 0;
		std::map<std::string, Location*>::const_iterator it = request.server->locations.begin();
		while (it != request.server->locations.end())
		{
			// LOG_BLUE("First " << it->first);
			// LOG_BLUE("Sec size " << it->second->client_max_body_size);
			// LOG_BLUE("Sec Method " << it->second->methods[0]);
			if (it->second->methods[0] == "GET")
				method = GET;
			if (it->second->methods[0] == "POST")
				method = POST;
			if (it->second->methods[0] == "PUT")
				method = PUT;
			if (it->second->methods[0] == "DELETE")
				method = DELETE;
			// if ( request.getUploadPath() == it->first)
			// 	LOG_BLACK("PATH TRUE");
			// if ( it->second->client_max_body_size != -1)
			// 	LOG_BLACK("NON DEFAULT BODY TRUE");
			// if ( tmp.size() > (unsigned long)it->second->client_max_body_size)
			// 	LOG_BLACK("MAX BODY TRUE");
			// if ( request.getRequestKey() == method)
			// 	LOG_BLACK("REQUEST TRUE");
			if ( request.getUploadPath() == it->first
				&& it->second->client_max_body_size != -1
				&& tmp.size() > (unsigned long)it->second->client_max_body_size
				&& request.getRequestKey() == method)
			{
				LOG_YELLOW("TRUE");
				writeStatus(413, request.socket);
				request.status = DONE_WRITING;
				return;
			}
			//LOG_BLUE(it->first);
			method = 0;
			it++;
		}
		//LOG_YELLOW("MAX SIZE: " << request.server->locations["client_max_body_size"]);
			//tmp.size() > (unsigned long)request.server->client_max_body_size



		LOG_YELLOW("CREATE FILE");
		if (!request.cgi_request)
		{
			std::string filename = "Felix";
			emptyUploadFile(filename);
			createUploadFile(filename, tmp);
		}
		LOG_YELLOW("FILE CREATED");

		if (request.cgi_request) {
			//request.status = DONE_READING;
			LOG_GREEN("RUN CGI");
			cgi = new Cgi(request);
			//Cgi cgi(request);
			LOG_GREEN("END CGI");
			return ;
		}
		//------------------------------------------------------------------------------------------------------------------------------------------
		//LOG_BLACK("FLEX");
		writeStatus(201, request.socket);
		request.status = DONE_WRITING;
		return ;
	}
	if (request.cgi_request && request.file_created) {
			request.status = DONE_READING;
			LOG_GREEN("RUN CGI");
			cgi->answerCgi();
			if (request.status == DONE_WRITING_CGI) {
				delete cgi;
				LOG_GREEN("END CGI");
			}
			//Cgi cgi(request);
			return ;
		}
	if (request.body.size() == 0) {
		LOG_RED_INFO("empty body in post request");
		//LOG_RED_INFO(request.header);
		writeStatus(204, request.socket);
		request.status = DONE_WRITING;
		return ;
	}
	_boundary = extractBoundary();
	if (_boundary == "error")
	{
		// es gibt kein boundary, also wuden keine files geschickt und ich muss irgendwas anderes tun
		writeToSocket(request.socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: extractBoundary");
		request.status = DONE_WRITING;
		return ;
	}

	_numOfBoundaries = countBoundaries();
	if (!_numOfBoundaries)
	{
		writeToSocket(request.socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: countBoundaries");
		request.status = DONE_WRITING;
		return ;
	}

	if (_numOfBoundaries > 0)
	{
		uploadFiles();
		if (request.cgi_request) {
			Cgi cgi(request);
			return ;
		}
		writeStatus(201, request.socket);
		request.status = DONE_WRITING;
		return ;
	}

	//write(request.socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 16\n\nfile was created", 80);
	char redirection[] = "HTTP/1.1 301 Moved Permanently\nLocation: http://127.0.0.1:1000/index.html\n\n"; //TODO include path from rerouting

	writeToSocket(request.socket, redirection);
	request.status = DONE_WRITING;
}

