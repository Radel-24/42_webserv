#pragma once

#define LOG(x) (std::cout << x << std::endl)

#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>

class PostResponder {

	private:
		std::string		_header;
		std::string		_body;
		std::string		_boundary;
		int				_numOfBoundaries;

		std::string	extractBoundary( void );
		int			countBoundaries( void );
		void		uploadFiles( void );
		void		createUploadFile( std::string filename, std::string content );

	public:
		PostResponder( std::string header, std::string body, int new_socket );
};