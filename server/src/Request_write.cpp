#include "Request.hpp"

// only for website
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
				LOG_RED("file tree went wrong");
			if (chdir(cwd.c_str()))
				LOG_RED_INFO("error: chdir went wrong: " << cwd);
		}
		else {
			LOG_RED_INFO("error: chdir went wrong: " << (cwd + server->root + server->uploadPath));
		}
	}
	else {
		LOG_RED_INFO("error: upload directory doesnt exists: " << (cwd + server->root + server->uploadPath));
	}
}

void	Request::postResponder() {
		if (!pr)
			pr = new PostResponder(*this);
		pr->run();
		if (status == DONE_WRITING_CGI) {
			delete (pr);
			pr = NULL;
		}
}

void	Request::writeResponse() {
	ssize_t bytes_written = writeToSocket(socket, response.c_str() + bytes_written);
	LOG_BLACK_INFO("bytes written " << bytes_written);
	if (bytes_written == -1) {
		status = CLOSE_CONNECTION;
		LOG_BLACK_INFO("write failed");
		return ;
	}
	bytes_written += bytes_written;
	if ((size_t)bytes_written >= response.length()) {
		status = DONE_WRITING;
		LOG_GREEN_INFO("done writing");
	}

}

void	Request::writeRequest() {
	LOG_RED_INFO("request key " << requestKey);
	if (responseCreated == false) {
		if (status >= 100 && status < 600) {
			LOG_RED_INFO("request status " << status);
			writeStatus(status, socket);
			status =  DONE_WRITING;
		}
		else if (status == DONE_READING && (getRequestKey() == POST || getRequestKey() == PUT)) {
			postResponder();
			refreshFilesHTML(); // only for website
		}
		else if (status == DONE_READING && cgi_request == false && (getRequestKey() == GET || getRequestKey() == HEAD)) {
			responder();
			LOG_YELLOW_INFO("END responder ----------------");
			status =  DONE_WRITING;
		}
		else if (status == DONE_READING && getRequestKey() == DELETE) {
			deleteResponder();
			status =  DONE_WRITING;
		}
		else if (status == DONE_READING && getRequestKey() == GET && cgi_request == true) {
			Cgi * cgi = new Cgi(*this);
			(void)cgi;
		}
		responseCreated = true;
	}
	writeResponse();
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
	header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
	length = std::to_string(file_content.length()) + "\r\n\r\n";
	full_header = header.append(length);
	if (requestKey == HEAD)
		return full_header;
	ret = full_header.append(file_content);
	//LOG_RED_INFO("response: " << ret);
	return ret;
}

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

void	Request::doDirectoryListing( void ) {
	std::string fileTree = server->createFileTree(location);
	std::string formattedTree = formatString(fileTree);
	writeToSocket(socket, formattedTree);
}

bool	Request::checkDirectoryListing( void ) {
	LOG_YELLOW_INFO("filename:\t" << filename);
	if (("/" + filename) != location->path) {
		LOG_RED_INFO("requested Location doesnt exist in config");
		return false;
	}
	if (location->directory_listing == false) {
		LOG_RED_INFO("error: requested Location doesnt allow listing");
		return false;
	}
	if (!dirExists(toAbsolutPath(server->root + location->path).c_str())) {
		LOG_RED_INFO("error: requested Location doesnt have directory");
		return false;
	}
	return true;
}

void	Request::responder() {
	std::string	file_content;
	std::string	formatted;

	if (checkDirectoryListing()) {
		LOG_GREEN_INFO("EXECUTING DIRECTORY LISTING");
		doDirectoryListing();
		return ;
	}

	LOG_PINK_INFO("request.path:\t" << path);
	struct stat path_stat;
	std::string	temp = "." + path;
	stat(temp.c_str(), &path_stat);
	LOG_PINK_INFO("redirection:\t" << location->redirection << "");
	if (location->redirection != "") { // if redirection exixsts
		LOG_RED_INFO("will redirect");
		std::string ret = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		ret += location->redirection;
		ret += "\r\n\r\n";
		LOG_PINK_INFO(ret);
		writeToSocket(socket, ret);
		return;
	}
	if (S_ISDIR(path_stat.st_mode)) {
		path += "/" + location->default_file;
		LOG_CYAN_INFO("default dir request: " << path);
	}
	if (path == (server->root + "/")) {
		file_content = readFile( "." + server->root + "/index.html");
	}
	else {
		file_content = readFile(path.substr(1, std::string::npos));
		if (status == 404){
			writeStatus(404, socket);
			return ;
		}
	}
	formatted = formatString(file_content);
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