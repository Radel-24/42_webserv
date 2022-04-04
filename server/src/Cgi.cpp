#include "Cgi.hpp"

void	Cgi::init() {
	inFile = tmpfile();
	outFile = tmpfile();
}

Cgi::Cgi(Request & request) : request(request) {
	init();
	setEnv();
	runCgi();
}

void	Cgi::setEnv() {
	env["REQUEST_METHOD"] = "POST";
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["PATH_INFO"] = toAbsolutPath(request.server->cgi_path); // path to uploaded file?
	//env["PATH_INFO"] = "./";
	//env["REQUEST_URI"] = toAbsolutPath(request.server->cgi_path); // path to uploaded file?
	//env["REDIRECT_STATUS"] = "200";
	//env["REDIRECT_STATUS"] = "CGI";
	//env["SCRIPT_NAME"] = toAbsolutPath(request.server->cgi_path);
	//env["PATH_TRANSLATED"] = toAbsolutPath(request.server->cgi_path);
	//env["CONTENT_TYPE"] = request.headerValues["Content-type"];
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

	//size_t index = 0;
	//while (localEnv[index]) {
	//	LOG_BLUE(localEnv[index]);
	//	++index;
	//}
	//char * cgi_path = const_cast<char *>(toAbsolutPath(request.server->cgi_path).c_str());
	int fin = fileno(inFile);
	int fout = request.socket; // TODO maybe write to outfile and let host send the answer back to the client
	pid_t pid = fork();
	if (pid == -1) {
		LOG_RED_INFO("fork failed"); // TODO error handling
		request.status = 500;
	}
	if (pid == 0) {
		write(fin, request.body.c_str(), request.body.size());
		dup2(fin, STDIN_FILENO);

		//dup2(fout, STDOUT_FILENO); // TODO comment this line in to write back the answer to the client

		lseek(fin, 0, SEEK_SET); // TODO needed?
		close(fin);
		close(fout);
		LOG_RED_INFO("first " << toAbsolutPath(request.server->cgi_path).c_str());
		if (execve(toAbsolutPath(request.server->cgi_path).c_str(), NULL, localEnv) == -1) {
			LOG_RED_INFO("cgi failed");
			exit(1);
		}
	} else {
		close(fin);
		// TODO no clue what to do
		int exit_status;
		wait(&exit_status);
		LOG_GREEN_INFO("cgi ended with status " << exit_status);
		LOG_GREEN_INFO("exit status " << WEXITSTATUS(exit_status));
		if (WEXITSTATUS(exit_status) == 1)
			request.status = 500;
		else
			request.status = DONE_WRITING;

		//write(request.socket, fout, )
		//LOG_BLUE_INFO(fout);
	}
}