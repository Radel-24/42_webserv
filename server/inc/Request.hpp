#pragma once

#include <sys/socket.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>

#include "Server.hpp"
#include "PostResponder.hpp"

enum ReqKeys {
	NIL,
	GET,
	POST,
	PUT,
	DELETE
};

enum Status {
	WORKING,
	DONE,
	DECLINE
};

class	Request {
	public:
		int									socket;

	private:
		std::string							header;
		std::string							body;
		std::map<std::string, std::string>	headerValues;
		unsigned int						requestKey;
		ssize_t								bytes_read;
		bool								header_read;
		bool								body_read;
		Server *							server;
		std::string							path;


	private:
		void	init();

	public:
		Request();
		Request(int socket, Server * server);
		~Request();
		int			getRequestKey() const;
		std::string	getHeader() const;
		std::string	getBody() const;
		int			checkHeaderRead(void);
		int			checkBodySize(void);

		void	appendHeader(std::string input);
		void	appendBody(char *, int);
		// void	setHeaderValues(std::pair<std::string, std::string> pair);

		//void	clearBody();
		//void	clearHeader();

		void	setRequestKey(unsigned int);

		int	readRequest();
		int writeRequest();

		void	setType();
		void	changePath();
		void	setPath();
		void	readHeader();
		void	readBody();

		void	responder();
		std::string	getFilename();

		void	printHeaderValues() {
			std::map<std::string, std::string>::iterator iter = headerValues.begin();
			while (iter != headerValues.end()) {
				std::cout << iter->first << " | " << iter->second << "\n";
				++iter;
			}
		}

		/* start alex new */
		void								deleteResponder( void );
		void								parseHeader( std::string header );
		std::pair<std::string, std::string>	splitToken( std::string token );
		std::string							&leftTrim( std::string &str, char c );
		void								setHeaderValues( const std::pair<std::string, std::string> &pair );
		std::map<std::string, std::string>	getHeaderValues( void ) const {
			return this->headerValues;
		}
		Server								*getServer( void ) const {
			return this->server;
		}
		int									checkHeaderValues( void );

		std::string							getHostName( void ) const {
			std::map<std::string, std::string>::const_iterator	iter = headerValues.begin();
			while (iter != headerValues.end())
			{
				if (iter->first == "Host")
					return iter->second;
				iter++;
			}
			return "";
		}
		/* end alex new */

};