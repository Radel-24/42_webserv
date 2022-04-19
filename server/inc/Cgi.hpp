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

		void	init();
		void	setEnv();
		void	runCgi();
		void	parseCgi();
		void	answerCgi();

	public:
		Cgi(Request & request);
};