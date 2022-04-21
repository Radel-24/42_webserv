#pragma once

#include "Request.hpp"
#include "utils.hpp"
#include <map>
#include <unistd.h>

#include <string>

class Request;

class Cgi {
	private:
		std::map<std::string, std::string>	env;
		Request &							request;
		std::string							answer;
		std::string 						body;

		FILE *								inFile; 
		FILE *								tempFile;
		std::string							response;

		void	init();
		void	setEnv();
		void	runCgi();
		void	parseCgi();

	public:
		Cgi(Request & request);
		~Cgi();
		void	answerCgi();
};