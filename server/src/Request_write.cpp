#include "Request.hpp"

void	Request::writeRequest() {
	LOG_RED_INFO("request key " << requestKey);
	if (status >= 100 && status < 600) {
		LOG_RED_INFO("request status " << status);
		writeStatus(status, socket);
		status =  DONE_WRITING;
	}
	else if (status == DONE_READING && (getRequestKey() == POST || getRequestKey() == PUT || cgi_request == true)) {
		if (!postResponder)
			postResponder = new PostResponder(*this);
		postResponder->run();
		if (status == DONE_WRITING_CGI) {
			delete (postResponder);
			postResponder = NULL;
		}
		return ;
	}
	else if (status == DONE_READING && (getRequestKey() == GET || getRequestKey() == HEAD)) {
		responder();
		status =  DONE_WRITING;
	}
	else if (status == DONE_READING && getRequestKey() == DELETE) {
		deleteResponder();
		status =  DONE_WRITING;
	}
}

std::string	Request::readFile( std::string filename ) {
	std::ifstream	newFile;
	std::string		ret;
	std::string		values;
	char			c;

	int fd = open(filename.c_str(), std::ios::in);
	if (fd == -1) {
		status = 404;
		return "";
	}
	else {
		close(fd);
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

std::string	Request::formatString( std::string file_content ) {
	std::string	header;
	std::string	length;
	std::string	full_header;
	std::string	ret;
	header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	length = std::to_string(file_content.length()) + "\r\n\r\n";
	full_header = header.append(length);
	if (requestKey == HEAD)
		return full_header;
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
	struct stat path_stat;
	std::string	temp = "." + path;
	stat(temp.c_str(), &path_stat);
	LOG_PINK_INFO("redirection " << location->redirection << "");
	if (location->redirection != "") {
		LOG_RED_INFO("will redirect");
		std::string ret = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		ret += location->redirection;
		ret += "\r\n\r\n";
		LOG_PINK_INFO(ret);
		writeToSocket(socket, ret);
		return;
	}
	if (S_ISREG(path_stat.st_mode)) {
		//path += "/" + location->default_file;
		LOG_CYAN_INFO("default file request: " << path);
	}
	if (S_ISDIR(path_stat.st_mode)) {
		path += "/" + location->default_file;
		LOG_CYAN_INFO("default dir request: " << path);
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
			writeStatus(404, socket);
			return ;
		}
		if (file_content.empty()) {
			formatted = formatString("error: 404"); //TODO check if this is reached
		}
		else
			formatted = formatString(file_content);
	}
	formatted = formatString(file_content);
	//LOG_RED_INFO("writes " << formatted);
	writeToSocket(socket, formatted);
}

std::string	Request::getFilename() {
	std::string	converted = std::string(getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	LOG_CYAN_INFO("filename: " << file);
	return file;
}

std::string	Request::getPath()
{
	return this->path;
}

std::string	Request::getUploadPath()
{
	return this->uploadPath;
}