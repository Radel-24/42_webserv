#pragma once

#include "Request.hpp"
#include "utils.hpp"
#include <map>
#include <unistd.h>

class Request;

class Cgi {
	private:
		std::map<std::string, std::string>	env;
		Request &	request;

		void	init();
		void	setEnv();
		void	runCgi();

	public:
		Cgi(Request & request);
};