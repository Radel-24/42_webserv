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
	FAILURE
};

class	Request {
	public:
		int		socket;

	private:
		std::string	header;
		std::string	body;
		std::map<std::string, std::string>	headerValues;
		unsigned int			requestKey;
		ssize_t		bytes_read;
		bool	header_read;
		bool	body_read;
		Server *	server;


	private:
		void	init();

	public:
		Request();
		Request(int socket, Server * server);
		~Request();
		int			getRequestKey() const;
		std::string	getHeader() const;
		std::string	getBody() const;
		int			checkHeader(void);
		int			checkBodySize(void);

		void	appendHeader(std::string input);
		void	appendBody(char *, int);
		void	setHeaderValues(std::map<std::string, std::string> mappe);

		//void	clearBody();
		//void	clearHeader();

		void	setRequestKey(unsigned int);

		int	readRequest();
		int writeRequest();

		void	setType();
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

};