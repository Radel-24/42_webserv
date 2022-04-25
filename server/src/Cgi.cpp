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
	while (it != request.headerValues.end()) {
		env["HTTP_" + it->first] = it->second;
		it++;
	}
}

void	Cgi::setInput() {
	std::vector<std::string> inVec;

	inVec.push_back(toAbsolutPath(request.server->cgi_path).c_str());
	inVec.push_back(toAbsolutPath("cgi/" + request.filename));
	inVec.push_back(request.headerKeyValuePairs);
	input = vectorToArray(inVec);
}

void	Cgi::runCgi() {
	char **	localEnv = mapToArray(env);
	int		fin = fileno(inFile);
	int		fout = fileno(tempFile);
	pid_t	pid = fork();

	if (pid == -1) {
		LOG_RED_INFO("fork failed");
		request.status = 500;
		close(fin);
		close(fout);
		return ;
	}
	if (pid == 0) {
		if (dup2(fin, STDIN_FILENO) == -1) {
			write(STDERR_FILENO, "error: dup2 failed\n", 20);
			request.status = 500;
		}
		if (dup2(fout, STDOUT_FILENO) == -1) {
			write(STDERR_FILENO, "error: dup2 failed\n", 20);
			request.status = 500;
		}
		write(fin, request.body.c_str(), request.body.size());
		if (lseek(fin, 0, SEEK_SET) == -1) {
			write(STDERR_FILENO, "error: lseek failed\n", 21);
			request.status = 500;
		}
		close(fin);
		close(fout);
		if (execve(toAbsolutPath(request.server->cgi_path).c_str(), input, localEnv) == -1) {
			write(STDERR_FILENO, "error: cgi failed\n", 19);
			exit(1);
		}
	}
	else {
		close(fin);
		int exit_status;
		wait(&exit_status);
		for (int i = 0; localEnv[i] != NULL; ++i) {
			free(localEnv[i]);
		}
		for (int i = 0; input[i] != NULL; ++i) {
			free(input[i]);
		}
		free(localEnv);
		free(input);
	}
}

//TO-DO remove file creation to get more performance
void	Cgi::parseCgi() {
	// if error happened while cgi ran
	if (request.status >= 500) {
		request.response = writeStatus(request.status, request);
		return ;
	}

	// obtain file size:
	fseek(tempFile , 0 , SEEK_END);
	long	lSize = ftell(tempFile);
	rewind(tempFile);

	// allocate file size for buffer
	char	*buffer = (char*)calloc(lSize, sizeof(char));
	if (buffer == NULL)
		LOG_RED_INFO("ERROR: malloc failed");

	// read file into buffer
	long	result = fread(buffer, 1,lSize ,tempFile);
	if (result != lSize)
		LOG_RED_INFO("ERROR: fread failed");

	// convert buffer to std::string
	answer = std::string(buffer);
	free(buffer);

	// close tempfile
	int	fout = fileno(tempFile);
	close(fout);

	// fill response
	size_t		bodyBegin = answer.find("\r\n\r\n") + 4;
	std::string	body = answer.substr(bodyBegin, std::string::npos);
	request.response = "HTTP/1.1 200 OK\r\nContent-Length: ";
	request.response += std::to_string(body.length());
	request.response += "\r\n\r\n";
	request.response += body;
}