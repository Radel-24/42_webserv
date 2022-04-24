#include "Request.hpp"

void	Request::writeRequest() {
	if (responseCreated == false) {
		if (status >= 100 && status < 600) {
			response = writeStatus(status);
		}
		else if (status == DONE_READING && (getRequestKey() == POST || getRequestKey() == PUT)) {
			LOG_BLUE_INFO("POST Responder");
			PostResponder pr(*this);
			refreshFilesHTML(); // only for website
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
	writeResponse();
}

// only for website
// TODO is this necessary ? if directory listing is off might be a problem
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
	if (tmp_bytes_written == -1) { // TODO: what do do here?
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
	header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	length = std::to_string(file_content.length()) + "\r\n\r\n";
	full_header = header.append(length);
	if (requestKey == HEAD)
		return full_header;
	ret = full_header.append(file_content);
	return ret;
}

// can only delete one file
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

void	Request::doDirectoryListing( void ) {
	std::string fileTree = server->createFileTree(location);
	response = formatString(fileTree);
}

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
		LOG_CYAN_INFO("default dir request: " << path);
	}
	if (path == (server->root + "/")) {
		file_content = readFile( "." + server->root + "/index.html");
	}
	else {
		file_content = readFile(path.substr(1, std::string::npos));
		if (status == 404) {
			response = writeStatus(404);
			return ;
		}
	}
	response = formatString(file_content);
}

std::string	Request::getFilename() {
	std::string	converted = std::string(getHeader());
	int			start = converted.find("/") + 1;
	int			end = converted.find("HTTP") - 1;
	std::string	file = converted.substr(start, end - start);
	return file;
}
