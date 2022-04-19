#include "Cgi.hpp"

// LOGIC: <request.body php-cgi >response -> socket
void	Cgi::init() {
	inFile = tmpfile();
	// outFile = tmpfile();
	tempFile = tmpfile();
	answer = "";
	answer.clear();
	body = "";
	body.clear();
}

Cgi::Cgi(Request & request) : request(request) {
	//if (request.file_created == false)
	//{
		init();
		setEnv();
		runCgi();
		request.file_created = true;
	//}
	parseCgi();
	answerCgi();
}

Cgi::~Cgi() {
	request.file_created = false;
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

void	emptyUploadFile( std::string path )
{
	//LOG_YELLOW("depug upload path: " << path);
	std::ofstream	file(path, std::ios_base::out);
	if (file.is_open()) {
		file << ""; // else error
		// LOG_YELLOW("upload file is opened");
	}
	//std::cout << "HEX\n" << ToHex(content, 0) << "\n";
	file.close();
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

	// std::string	pwd = std::string(getcwd(NULL, FILENAME_MAX));
	// std::string	fullPath = pwd + "/cgiOutput.txt";
	// emptyUploadFile(fullPath);
	// int fout = open(fullPath.c_str(), O_RDWR);
	int fout = fileno(tempFile);
	LOG_CYAN_INFO("cgi file opened");

	pid_t pid = fork();
	if (pid == -1) {
		LOG_RED_INFO("fork failed"); // TODO error handling
		request.status = 500;
	}
	if (pid == 0) {
		//LOG_CYAN_INFO(request.body);

		// !!!!!!!! Don't write log messages in here !!!!!!!

		dup2(fin, STDIN_FILENO);

		//fout = open("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt", O_RDWR); // TODO only for testing

		
		dup2(fout, STDOUT_FILENO); // TODO comment this line in to write back the answer to the client
		write(fin, request.body.c_str(), request.body.size());
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
		// close(fout);
		// TODO no clue what to do
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		LOG_GREEN_INFO("exit status " << WEXITSTATUS(exit_status));
		//if (WEXITSTATUS(exit_status) == 1)
		//	request.status = 500;
		//else
			//request.status = DONE_WRITING;
		LOG_GREEN_INFO("finished runCgi");
		//write(request.socket, fout, )
		//LOG_BLUE_INFO(fout);
	}
}

void	Cgi::parseCgi() {
//	std::ifstream instream("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt");
//	std::stringstream strStr;
//	strStr << instream;
	//std::string	pwd = std::string(getcwd(NULL, FILENAME_MAX));
	//std::string	fullPath = pwd + "/www/42testServer/upload/Felix";
	// std::string	pwd = std::string(getcwd(NULL, FILENAME_MAX));
	// std::string	fullPath = pwd + "/cgiOutput.txt";
	// std::ifstream t(fullPath);
	// std::stringstream buffer;
	// buffer << t.rdbuf();

	// int fout = fileno(request.tempFile);
	char *buffer;

	 // obtain file size:
	fseek (tempFile , 0 , SEEK_END);
	long lSize = ftell (tempFile);
	rewind (tempFile);

	buffer = (char*) calloc (lSize, sizeof(char));
	if (buffer == NULL)
		LOG_RED_INFO("MALLOC FAILED");

	long result = fread (buffer, 1,lSize ,tempFile);

	LOG_RED(lSize);
	LOG_RED(result);

  	if (result != lSize)
	  	LOG_RED("ERROR");
	answer = std::string(buffer);
	free(buffer);
	int fout = fileno(tempFile);
	close(fout);
	//LOG_RED_INFO("file read " << answer);
	size_t	bodyBegin = answer.find("\r\n\r\n") + 4;

	LOG_PINK("body begin: " << bodyBegin);
	//LOG_GREEN_INFO("body begin: " << bodyBegin);
	//LOG_GREEN_INFO("std::string::npos: " << std::string::npos);
	body = answer.substr(bodyBegin, std::string::npos);

	LOG_PINK(body.substr(0, 20));
	LOG_PINK(body.substr(body.length() - 20, std::string::npos));
	size_t pp = body.find_first_not_of("C");
	LOG_PINK(body.substr(pp, 30));

	//std::getline(std::ifstream("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt"), answer, '\0');
	//LOG_BLUE_INFO(body);
}

void	Cgi::answerCgi() {
	LOG_RED_INFO("body length " << body.length());
	std::string response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	response += std::to_string(body.length());
	response += "\r\n\r\n";
	response += body;
	//if (size_t pos = body.find("\r"))
	//	LOG_RED_INFO("found r " << pos);
	//if (size_t pos = body.find("\n"))
	//	LOG_RED_INFO("found n " << pos);
	//response += "\r\n\r\n";
	LOG_GREEN_INFO("cgi response length: " << response.length());
	char * tmp = const_cast<char *>(response.c_str());
	int bytes_written = writeToSocket(request.socket, tmp + request.bytes_written);
	LOG_BLACK_INFO("bytes written " << bytes_written);
	if (bytes_written == -1) {
		request.status = DONE_WRITING;
		LOG_BLACK_INFO("write failed");
		return ;
	}
	request.bytes_written += bytes_written;
	LOG_BLACK_INFO("total written " << request.bytes_written);
	if ((size_t)request.bytes_written >= response.length()) {
		request.init();
		request.status = DONE_WRITING_CGI;
		//request.file_created = false;
		fclose(tempFile);
		fclose(inFile);
		//request.body.clear();
		//request.bytes_written = 0;
		//request.header.clear();
		LOG_GREEN("FINISHED CGI");
	}

	//int fout = open("/Users/radelwar/Documents/42_webserv/server/cgiOutput.txt", O_RDWR);
	//writeToSocket(fout, response);
	//close(fout);
}