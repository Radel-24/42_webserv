#pragma once

#include <sys/socket.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>
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
	private:
		std::string	header;
		std::string	body;
		std::map<std::string, std::string>	headerValues;
		unsigned int			requestKey;
		ssize_t		bytes_read;
		bool	header_read;
		bool	body_read;

	public:
		int		socket;

	private:
		void	init();

	public:
		Request();
		Request(int);
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