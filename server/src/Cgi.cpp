#include "Cgi.hpp"

// LOGIC: <request.body php-cgi >response -> socket
void	Cgi::init() {
	inFile = tmpfile();
	tempFile = tmpfile();
	if (inFile == NULL || tempFile == NULL) {
		LOG_RED_INFO("tempfile failed");
	}
	answer = "";
	answer.clear();
	body = "";
	body.clear();
	input = NULL;
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	setInput();
	runCgi();
	request.file_created = true;
	parseCgi();
	answerCgi();
}

Cgi::~Cgi() {
	request.file_created = false;
}

void	Cgi::setEnv() {
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	if (request.getRequestKey() == POST)
		env["REQUEST_METHOD"] = "POST";
	if (request.getRequestKey() == GET)
		env["REQUEST_METHOD"] = "GET";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["PATH_INFO"] = toAbsolutPath(request.server->cgi_path);
	//env["PATH_INFO"] = toAbsolutPath("cgi/" + request.filename);
	env["REDIRECT_STATUS"] = "200";
	env["PATH_TRANSLATED"] = toAbsolutPath("cgi/" + request.filename);
	env["CONTENT_TYPE"] = request.headerValues["Content-Type"];
	
	//env["CONTENT_TYPE"] = "test/file";
	//env["CONTENT_LENGTH"] = std::to_string(request.getBody().length());
	//env["QUERY_STRING"] = request.getBody();

	std::map<std::string, std::string>::const_iterator it = request.headerValues.begin();
	while (it != request.headerValues.end())
	{
		env["HTTP_" + it->first] = it->second;
		it++;
	}

	//it = env.begin();
	//while (it != env.end()) {
	//	LOG_BLUE(it->first << "|" << it->second);
	//	++it;
	//}

}

void	Cgi::setInput() {
	std::vector<std::string> inVec;

	inVec.push_back(toAbsolutPath("cgi/" + request.filename));
	//inVec.push_back("-f");
	std::map<std::string, std::string> headerInfos;
	while (request.headerKeyValuePairs.find("&") != std::string::npos) {
		std::string strPair = request.headerKeyValuePairs.substr(0, request.headerKeyValuePairs.find("&"));
		inVec.push_back(strPair);
		request.headerKeyValuePairs.erase(0, request.headerKeyValuePairs.find("&") + 1);
	}
	inVec.push_back(request.headerKeyValuePairs);
	input = vectorToArray(inVec);
}

void	Cgi::runCgi() {
	char ** localEnv = mapToArray(env);
	
	//int i = 0;
	//while (input[i]) {
	//	LOG_YELLOW_INFO("|" << input[i] << "|");
	//	++i;
	//}

	int fin = fileno(inFile);

	int fout = fileno(tempFile);
	LOG_CYAN_INFO("cgi file opened");


	pid_t pid = fork();

	if (pid == -1) {
		LOG_RED_INFO("fork failed"); // TODO error handling
		request.status = 500;
	}
	if (pid == 0) {
		// !!!!!!!! Don't write log messages in here !!!!!!!

		if (dup2(fin, STDIN_FILENO) == -1) {
			LOG_RED_INFO("dup2 failed");
		}

		if (dup2(fout, STDOUT_FILENO) == -1) {
			LOG_RED_INFO("dup2 failed");
		}
		write(fin, request.body.c_str(), request.body.size());
		if (lseek(fin, 0, SEEK_SET) == -1) {
			LOG_RED_INFO("lseek failed");
		}
		close(fin);
		close(fout);
		if (execve(toAbsolutPath(request.server->cgi_path).c_str(), input, localEnv) == -1) {
			LOG_RED_INFO("cgi failed");
			exit(1);
		}
	} else {
		close(fin);
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		free(localEnv);
		free(input);
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

  	if (result != lSize)
	  	LOG_RED("ERROR");
	answer = std::string(buffer);
	free(buffer);
	int fout = fileno(tempFile);
	close(fout);

	size_t	bodyBegin = answer.find("\r\n\r\n") + 4;

	body = answer.substr(bodyBegin, std::string::npos);

	response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	response += std::to_string(body.length());
	response += "\r\n\r\n";
	response += body;
}

void	Cgi::answerCgi() {
	ssize_t bytes_written = writeToSocket(request.socket, response.c_str() + request.bytes_written);
	LOG_BLACK_INFO("bytes written " << bytes_written);
	if (bytes_written == -1) {
		request.status = CLOSE_CONNECTION;
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