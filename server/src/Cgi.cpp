#include "Cgi.hpp"

// LOGIC: <request.body php-cgi >response -> socket
void	Cgi::init() {
	inFile = tmpfile();
	tempFile = tmpfile();
	answer = "";
	answer.clear();
	body = "";
	body.clear();
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	runCgi();
	request.file_created = true;
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
	env["CONTENT_TYPE"] = request.headerValues["Content-Type"]; //empty??
	//env["CONTENT_TYPE"] = "test/file";
	//env["CONTENT_LENGTH"] = std::to_string(request.getBody().length());
	//env["QUERY_STRING"] = request.getBody();

	std::map<std::string, std::string>::const_iterator it = request.headerValues.begin();
	while (it != request.headerValues.end())
	{
		env["HTTP_" + it->first] = it->second;
		it++;
	}


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
	int fin = fileno(inFile);

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
		LOG_GREEN_INFO("finished runCgi");
	}
}

void	Cgi::parseCgi() {
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

	body = answer.substr(bodyBegin, std::string::npos);

	response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	response += std::to_string(body.length());
	response += "\r\n\r\n";
	response += body;

	LOG_GREEN_INFO("body length " << body.length());
	if (body.length() < 200000) {
		LOG_BLACK(request.header);
	//	LOG_GREEN(request.body);
	//	LOG_BLUE_INFO(request.body.length());
	}
}

void	Cgi::answerCgi() {
	LOG_GREEN_INFO("cgi response length: " << response.length());
	if (request.body.length() < 200000) {
		LOG_BLUE(response);
	//	LOG_GREEN(request.body);
	//	LOG_BLUE_INFO(request.body.length());
	}
	ssize_t bytes_written = writeToSocket(request.socket, response.c_str() + request.bytes_written);
	LOG_BLACK_INFO("bytes written " << bytes_written);
	if (bytes_written == -1) {
		request.status = DONE_WRITING;
		LOG_BLACK_INFO("write failed");
		return ;
	}
	request.bytes_written += bytes_written;
	if ((size_t)request.bytes_written >= response.length()) {
		request.init();
		request.status = DONE_WRITING_CGI;
		fclose(tempFile);
		fclose(inFile);
		LOG_GREEN("FINISHED CGI");
	}
}