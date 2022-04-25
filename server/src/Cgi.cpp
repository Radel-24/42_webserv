#include "Cgi.hpp"

// LOGIC: <request.body php-cgi -> response -> socket
void	Cgi::init() {
	inFile = tmpfile();
	tempFile = tmpfile();
	if (inFile == NULL || tempFile == NULL) {
		LOG_RED_INFO("tempfile failed");
	}
	answer = "";
	answer.clear();
	input = NULL;
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	setInput();
	runCgi();
	request.file_created = true;
	parseCgi();
}

Cgi::~Cgi() { request.file_created = false; }

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

	//inVec.push_back("-f");
	inVec.push_back(toAbsolutPath(request.server->cgi_path).c_str());
	inVec.push_back(toAbsolutPath("cgi/" + request.filename));
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

	int i = 0;
	while (input[i]) {
		LOG_YELLOW_INFO("|" << input[i] << "|");
		++i;
	}
	LOG_BLACK(toAbsolutPath(request.server->cgi_path).c_str());

	int fin = fileno(inFile);
	int fout = fileno(tempFile);

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
	}
	else {
		close(fin);
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		free(localEnv);
		free(input);
	}
}

//TO-DO remove file creation to get more performance
void	Cgi::parseCgi() {
	char *buffer;

	// obtain file size:
	fseek(tempFile , 0 , SEEK_END);
	long	lSize = ftell(tempFile);
	rewind(tempFile);

	buffer = (char*)calloc(lSize, sizeof(char));
	if (buffer == NULL)
		LOG_RED_INFO("ERROR: malloc failed");

	long	result = fread(buffer, 1,lSize ,tempFile);

  	if (result != lSize)
	  	LOG_RED_INFO("ERROR: fread failed");
	answer = std::string(buffer);
	free(buffer);
	int		fout = fileno(tempFile);
	close(fout);

	size_t	bodyBegin = answer.find("\r\n\r\n") + 4;

	std::string	body = answer.substr(bodyBegin, std::string::npos);

	request.response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	request.response += std::to_string(body.length());
	request.response += "\r\n\r\n";
	request.response += body;

	LOG_BLACK(request.response);
}