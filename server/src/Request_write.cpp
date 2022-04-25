#include "Request.hpp"

void	Request::writeRequest() {
	if (responseCreated == false) {
		if (status >= 100 && status < 600) {
			if (newClient == true)
			{
				LOG_RED_INFO("writeStatusCookie");
				response = writeStatusCookie(status, cookie, *this);
				newClient = false;
			}
			else
				response = writeStatus(status, *this);
			}
		else if (status == DONE_READING && (getRequestKey() == POST || getRequestKey() == PUT)) {
			LOG_BLUE_INFO("POST Responder");
			PostResponder pr(*this);
		}
		else if (status == DONE_READING && cgi_request == false && (getRequestKey() == GET || getRequestKey() == HEAD)) {
			LOG_BLUE_INFO("GET Responder");
			responder();
		}
		else if (status == DONE_READING && getRequestKey() == DELETE) {
			LOG_BLUE_INFO("DELETE Responder");
			deleteResponder();
		}
		else if (status == DONE_READING && getRequestKey() == GET && cgi_request == true) {
			LOG_BLUE_INFO("CGI GET Responder");
			Cgi * cgi = new Cgi(*this);
			(void)cgi;
		}
		responseCreated = true;
	}
	if (server->websiteConfig == true)
		refreshFilesHTML(); // only for website
	writeResponse();
}

// only for website
// when called, change dir into upload directory and list all the files, that the user uploaded via POST
// files.html gets created in sever root and gets returned when user clicks "Files" on Front End
void	Request::refreshFilesHTML() {
	std::string	cwd = getPWD();
	std::string	execPath = cwd;

	execPath += "/tree -H ";
	execPath += ".";
	execPath += " -T 'Your Files' -L 1 --nolinks --noreport --charset utf-8 -o ";
	execPath += cwd + server->root;
	execPath += "/files.html";

	if (dirExists((cwd + server->root + server->uploadPath).c_str())) {
		if (!chdir((cwd + server->root + server->uploadPath).c_str())) {
			if (system(execPath.c_str()) == -1)
				LOG_RED_INFO("ERROR: file tree went wrong");
			if (chdir(cwd.c_str()))
				LOG_RED_INFO("ERROR: chdir went wrong: " << cwd);
		}
		else {
			LOG_RED_INFO("ERROR: chdir went wrong: " << (cwd + server->root + server->uploadPath));
		}
	}
	else {
		LOG_RED_INFO("ERROR: upload directory doesnt exists: " << (cwd + server->root + server->uploadPath));
	}
}

void	Request::writeResponse() {
	ssize_t tmp_bytes_written = writeToSocket(socket, response.substr(bytes_written, std::string::npos));
	if (tmp_bytes_written == -1 || tmp_bytes_written == 0) { // TODO only check for -1 or even 0
		status = CLOSE_CONNECTION;
		LOG_BLACK_INFO("ERROR: write failed");
		return ;
	}
	bytes_written += (size_t)tmp_bytes_written;
	if (bytes_written >= response.length()) {
		status = DONE_WRITING;
		LOG_GREEN_INFO("done writing response");
	}
}

void	Request::clearResponse() {
	responseCreated = false;
	response.clear();
	bytes_written = 0;
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
	while (!newFile.eof()) {
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
	if (newClient == true) {
		header = "HTTP/1.1 200 OK\nSet-Cookie: I_Like_Cookies=" + cookie + "\nContent-Type: text/html\nContent-Length: ";
	}
	else {
		header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	}
	length = std::to_string(file_content.length()) + "\r\n\r\n";
	full_header = header.append(length);
	if (requestKey == HEAD)
		return full_header;
	ret = full_header.append(file_content);
	return ret;
}

void	Request::deleteResponder( void ) {
	std::string	filename = getFilename();
	std::string	deleteRoute = "." + server->root + server->uploadPath + "/" + filename;
	if (std::ifstream(deleteRoute)) {
		std::remove(deleteRoute.c_str());
		if (std::ifstream(deleteRoute))
			LOG_RED_INFO("error: deleting file");
		else
			LOG_GREEN_INFO("file deleted");
	}
	else
		LOG_RED_INFO("error: file not found");
}

// directory listing of location
// filling the response with the tree
void	Request::doDirectoryListing( void ) {
	std::string fileTree = server->createFileTree(location);
	response = formatString(fileTree);
}

// check if directory listing is allowed on requested location
// location has to exists
bool	Request::checkDirectoryListing( void ) {
	if (("/" + filename) != location->path) {
		return false;
	}
	if (location->directory_listing == false) {
		return false;
	}
	if (!dirExists(toAbsolutPath(server->root + location->path).c_str())) {
		return false;
	}
	return true;
}

void	Request::responder() {
	std::string	file_content;

	if (checkDirectoryListing()) {
		LOG_GREEN_INFO("EXECUTING DIRECTORY LISTING");
		doDirectoryListing();
		return ;
	}
	if (location->redirection != "") { // if redirection exixsts
		LOG_CYAN_INFO("will redirect");
		response = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		response += location->redirection;
		response += "\r\n\r\n";
		return;
	}
	std::string	temp = "." + path;
	if (dirExists(temp.c_str())) {
		path += "/" + location->default_file;
	}
	if (path == (server->root + "/")) {
		file_content = readFile( "." + server->root + "/index.html");
	}
	else {
		file_content = readFile(path.substr(1, std::string::npos));
		if (status == 404) {
			response = writeStatus(404, *this);
			return ;
		}
	}
	response = formatString(file_content);
	if (newClient == true) 
		newClient = false;
}

std::string	Request::getFilename() {
	std::string	converted = std::string(getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	return file;
}
