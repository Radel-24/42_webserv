#include "Cgi.hpp"

void	Cgi::init() {
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	runCgi();
}

void	Cgi::setEnv() {
	env["REQUEST_METHOD"] = "POST";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["PATH_INFO"] = toAbsolutPath(request.server->cgi_path);
	//env["REDIRECT_STATUS"] = "200";
	env["REDIRECT_STATUS"] = "CGI";
	env["SCRIPT_NAME"] = toAbsolutPath(request.server->cgi_path);
	env["REQUEST_URI"] = toAbsolutPath(request.server->cgi_path);
	env["PATH_TRANSLATED"] = toAbsolutPath(request.server->cgi_path);
	env["CONTENT_TYPE"] = request.headerValues["Content-type"];
	env["CONTENT_LENGTH"] = std::to_string(request.getBody().length());
	env["QUERY_STRING"] = request.getBody();


	//for (std::map<std::string, std::string>::iterator iter = env.begin(); iter != env.end(); ++iter) {
	//	LOG_BLACK(iter->first << " | " << iter->second);
	//}

}

void	Cgi::runCgi() {
	char ** localEnv = mapToArray(env);



	pid_t pid = fork();
	if (pid == -1) {
		LOG_RED_INFO("fork failed"); // TODO error handling
		request.status = 500;
	}
	if (pid == 0) {
		if (execve(request.server->cgi_path.c_str(), NULL, localEnv) == -1) {
			LOG_RED_INFO("cgi failed");
			exit(1);
		}
	} else {
		// TODO no clue what to do
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		if (WEXITSTATUS(exit_status) == 1)
			request.status = 500;
		else
			request.status = DONE_WRITING;
	}
}