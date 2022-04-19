#include "Cgi.hpp"

// LOGIC: <request.body php-cgi >response -> socket
void	Cgi::init() {
	inFile = tmpfile();
	outFile = tmpfile();
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	runCgi();
	parseCgi();
	answerCgi();
}

void	Cgi::setEnv() {
	env["REQUEST_METHOD"] = "POST";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["PATH_INFO"] = toAbsolutPath(request.server->cgi_path); // path to uploaded file?
	//env["PATH_INFO"] = "./";
	//env["REQUEST_URI"] = toAbsolutPath(request.server->cgi_path); // path to uploaded file?
	env["REDIRECT_STATUS"] = "200";
	//env["REDIRECT_STATUS"] = "CGI";
	//env["SCRIPT_NAME"] = toAbsolutPath(request.server->cgi_path);
	//env["PATH_TRANSLATED"] = toAbsolutPath(request.server->cgi_path);
	env["CONTENT_TYPE"] = request.headerValues["Content-type"]; //empty??
	//env["CONTENT_TYPE"] = "test/file";
	//env["CONTENT_LENGTH"] = std::to_string(request.getBody().length());
	//env["QUERY_STRING"] = request.getBody();


	//for (std::map<std::string, std::string>::iterator iter = env.begin(); iter != env.end(); ++iter) {
	//	LOG_BLACK(iter->first << " | " << iter->second);
	//}

	LOG_GREEN_INFO("path info " << env["PATH_INFO"]);
	LOG_GREEN_INFO("request uri " << env["REQUEST_URI"]);

}

void	Cgi::runCgi() {
	char ** localEnv = mapToArray(env);

	size_t index = 0;
	while (localEnv[index]) {
		LOG_BLUE(localEnv[index]);
		++index;
	}
	//char * cgi_path = const_cast<char *>(toAbsolutPath(request.server->cgi_path).c_str());
	int fin = fileno(inFile);
	//int fout = fileno(outFile); // TODO maybe write to outfile and let host send the answer back to the client
	int fout = open("/Users/fharing/42/webserv/server/cgiOutput.txt", O_RDWR);
	LOG_CYAN_INFO("cgi file opened");

	pid_t pid = fork();
	if (pid == -1) {
		LOG_RED_INFO("fork failed"); // TODO error handling
		request.status = 500;
	}
	if (pid == 0) {

		// !!!!!!!! Don't write log messages in here !!!!!!!

		write(fin, request.body.c_str(), request.body.size());
		dup2(fin, STDIN_FILENO);

		//fout = open("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt", O_RDWR); // TODO only for testing

		dup2(fout, STDOUT_FILENO); // TODO comment this line in to write back the answer to the client

		lseek(fin, 0, SEEK_SET);
		close(fin);
		close(fout);
		//LOG_RED_INFO("first " << toAbsolutPath(request.server->cgi_path).c_str());
		if (execve(toAbsolutPath(request.server->cgi_path).c_str(), NULL, localEnv) == -1) {
			LOG_RED_INFO("cgi failed");
			exit(1);
		}
	} else {
		close(fin);
		close(fout);
		// TODO no clue what to do
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		LOG_GREEN_INFO("exit status " << WEXITSTATUS(exit_status));
		//if (WEXITSTATUS(exit_status) == 1)
		//	request.status = 500;
		//else
			request.status = DONE_WRITING;
		LOG_GREEN_INFO("finished runCgi");
		//write(request.socket, fout, )
		//LOG_BLUE_INFO(fout);
	}
}

std::string	readFile( std::string filename ) {
	std::ifstream	newFile;
	std::string		ret = "";
	std::string		binary = "/Users/fharing/42/webserv/server/";
	std::string		values;
	std::string		execute = "/Users/fharing/42/webserv/server/cgi/php-cgi -f ";
	size_t			found;
	//char			c;
	if ((found = filename.find("cgi/", 0)) != std::string::npos)
	{
		if ((found = filename.find("?",0)) != std::string::npos)
		{
			binary = binary + filename.substr(0,found);
			values = filename.substr(found + 1,filename.length());
			std::replace(values.begin(),values.end(), '&', ' ');
		}
		execute = execute + binary + " " + values + " > out";

		//std::cout << execute << std::endl;
		system(execute.c_str());
		return "EXEC";
	}

	LOG_CYAN_INFO("trying to open: " << filename);
	if (open(filename.c_str(), std::ios::in) == -1) {
		//status = 404;
		return "";
	}
	newFile.open(filename, std::ios::in);
	if (!newFile){
		LOG_RED_INFO("openeing " << filename << " failed");
		return "";
	}
	std::string buffer;
	while (!newFile.eof())
	{
		std::getline(newFile, buffer);
		ret.append(buffer);
	}
	// LOG_YELLOW("i: " << i);
	// LOG_WHITE("ret1 = " << ret.substr(0, 80));

	// unsigned long pos = ret.find("\r\n");
	// LOG_GREEN("pos = " << pos);
	// if (pos != std::string::npos)
	// {
	// 	LOG_GREEN("pos = " << pos);
	// 	ret = ret.substr(pos + 2, ret.length());
	// }
	newFile.close();
	return ret;
}


void	Cgi::parseCgi() {
//	std::ifstream instream("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt");
//	std::stringstream strStr;
//	strStr << instream;
	answer = readFile("/Users/fharing/42/webserv/server/www/42testServer/upload/Felix");
	//LOG_RED_INFO("file read " << answer);
	//size_t	bodyBegin = answer.find("\r\n") + 2;
	//LOG_GREEN_INFO("body begin: " << bodyBegin);
	//LOG_GREEN_INFO("std::string::npos: " << std::string::npos);
	body = answer;
	//std::getline(std::ifstream("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt"), answer, '\0');
	//LOG_BLUE_INFO(body);
}

void	Cgi::answerCgi() {
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	response += std::to_string(body.length());
	response += "\r\n\r\n";
	response += body;
	//if (size_t pos = body.find("\r"))
	//	LOG_RED_INFO("found r " << pos);
	//if (size_t pos = body.find("\n"))
	//	LOG_RED_INFO("found n " << pos);
	//response += "\r\n\r\n";
	LOG_GREEN_INFO("cgi body length: " << body.length());
	int bytes_written = writeToSocket(request.socket, response.c_str() + request.bytes_written);
	LOG_BLACK_INFO("bytes written " << bytes_written);
	if (bytes_written == -1) {
		request.status = DONE_WRITING;
		LOG_BLACK_INFO("write failed");
		return ;
	}
	request.bytes_written += bytes_written;
	if ((size_t)request.bytes_written >= response.length())
		request.status = DONE_WRITING;
	LOG_GREEN("FINISHED CGI");

	//int fout = open("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt", O_RDWR);
	//writeToSocket(fout, response);
	//close(fout);
}