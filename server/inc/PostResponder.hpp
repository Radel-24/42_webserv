#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "Server.hpp"
#include "utils.hpp"
#include "Request.hpp"
#include "Cgi.hpp"

class Request;
class Cgi;
class Server;

class PostResponder {

	private:

		std::string		_boundary;
		int				_numOfBoundaries;

		Request &		request;
		Cgi * cgi;

		std::string	extractBoundary( void );
		int			countBoundaries( void );
		void		uploadFiles( void );
		void		writeResponse(std::string response);
		int			checkBodySizeChuncked();
		int			checkBodyStart();
		int			extractStartChunk();
		int			extractEndChunk();

	public:
		// TODO pass server and request by reference?
		PostResponder(Request & request );
		void	run();
};