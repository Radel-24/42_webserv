#pragma once

#define LOG(x) (std::cout << x << std::endl)

#define LOG_RED(x) (std::cout << "\033[1;31m" << x << "\033[0m" << std::endl)
#define LOG_GREEN(x) (std::cout <<  "\033[1;32m" << x << "\033[0m" << std::endl)
#define LOG_YELLOW(x) (std::cout << "\033[1;33m" << x << "\033[0m" << std::endl)
#define LOG_BLUE(x) (std::cout << "\033[1;34m" << x << "\033[0m" << std::endl)
#define LOG_BLACK(x) (std::cout << "\033[1;30m" << x << "\033[0m" << std::endl)
#define LOG_PINK(x) (std::cout << "\033[1;35m" << x << "\033[0m" << std::endl)
#define LOG_CYAN(x) (std::cout << "\033[1;36m" << x << "\033[0m" << std::endl)
#define LOG_WHITE(x) (std::cout << "\033[1;37m" << x << "\033[0m" << std::endl)

#define LOG_RED_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;31m" << x << "\033[0m" << std::endl)
#define LOG_GREEN_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;32m" << x << "\033[0m" << std::endl)
#define LOG_YELLOW_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;33m" << x << "\033[0m" << std::endl)
#define LOG_BLUE_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;34m" << x << "\033[0m" << std::endl)
#define LOG_BLACK_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;30m" << x << "\033[0m" << std::endl)
#define LOG_PINK_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;35m" << x << "\033[0m" << std::endl)
#define LOG_CYAN_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;36m" << x << "\033[0m" << std::endl)
#define LOG_WHITE_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;37m" << x << "\033[0m" << std::endl)

#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "Server.hpp"
#include "utils.hpp"

class PostResponder {

	private:
		std::string		_header;
		std::string		_body;
		std::string		_boundary;
		int				_numOfBoundaries;
		Server *		server;

		std::string	extractBoundary( void );
		int			countBoundaries( void );
		void		uploadFiles( void );
		void		createUploadFile( std::string filename, std::string content );
		void	writeResponse(std::string response);

	public:
		PostResponder( std::string header, std::string body, int new_socket, Server * server );
};