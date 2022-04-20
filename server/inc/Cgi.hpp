#pragma once

#include "Request.hpp"
#include "utils.hpp"
#include <map>
#include <unistd.h>
#include <fstream>
#include <string>

class Request;

class Cgi {
	private:
		std::map<std::string, std::string>	env;
		Request &	request;
		FILE *	inFile; // TODO change to pipe?
		FILE *	outFile;
		std::string	answer;
		std::string body;
		std::string response;

		void	init();
		void	setEnv();
		void	runCgi();
		void	parseCgi();

	public:
		Cgi(Request & request);
		~Cgi();
		void	answerCgi();
};