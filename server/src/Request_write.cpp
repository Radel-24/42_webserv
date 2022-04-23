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
	header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ";
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

void	Request::doDirectoryListing( Location * locationToList ) {
	LOG_GREEN_INFO("entering createFileTree");
	std::string fileTree = server->createFileTree(locationToList);
	std::string formattedTree = formatString(fileTree);
	writeToSocket(socket, formattedTree);
}

Location *	Request::checkDirectoryListing( std::string requestedPath ) {
	LOG_BLUE_INFO("requestedPath: " << requestedPath);
	for (std::map<std::string, Location*>::iterator it = server->locations.begin(); it != server->locations.end(); it++)
	{
		std::string	locationPath = server->root + it->second->path;

		requestedPath = convertDoubleSlashToSingle(requestedPath);
		locationPath = convertDoubleSlashToSingle(locationPath);

		// use location pointer
		if (it->second->directory_listing == true && (requestedPath == locationPath)) {
			if (dirExists(toAbsolutPath(requestedPath).c_str())) {
				LOG_GREEN_INFO("Requested Directory exists");
				return it->second;
			}
		}
	}
	return nullptr;
}

void	Request::responder() {
	std::string	file_content;
	std::string	formatted;

	// TODO return deafault file when neseecary
	// TODO create files always in www
	LOG_CYAN("path: " << path);
	LOG_CYAN("filename: " << filename);
	LOG_CYAN("header: " << header);
	LOG_CYAN("================================================");
	std::string	requestedPath = server->root + "/" + filename;
	Location *	locationToList = checkDirectoryListing(requestedPath);
	if (locationToList != nullptr) {
		doDirectoryListing(locationToList);
		LOG_CYAN("================================================");
		return ;
	}

	LOG_PINK_INFO("test:	" << path);
	struct stat path_stat;
	std::string	temp = "." + path;
	stat(temp.c_str(), &path_stat);
	LOG_PINK_INFO("redirection " << location->redirection << "");
	if (location->redirection != "") { // if redirection exixsts
		LOG_RED_INFO("will redirect");
		std::string ret = "HTTP/1.1 301 Moved Permanently\r\nLocation: ";
		ret += location->redirection;
		ret += "\r\n\r\n";
		LOG_PINK_INFO(ret);
		writeToSocket(socket, ret);
		return;
	}
	if (S_ISREG(path_stat.st_mode)) {
		LOG_CYAN_INFO("default file request: " << path);
	}
	if (S_ISDIR(path_stat.st_mode)) {
		path += "/" + location->default_file;
		LOG_CYAN_INFO("default dir request: " << path);
	}
	if (path == (server->root + "/")) {
		file_content = readFile( "." + server->root + "/index.html");
	}
	else
	{
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