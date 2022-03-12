#pragma once

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

class	Request {
	private:
		std::string	header;
		std::string	body;
		std::map<std::string, std::string>	headerValues;
		unsigned int			requestKey;

	public:
		int		socket;
		ssize_t		bytes_read;
		bool	header_read;
		bool	body_read;

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

		void	clearBody();
		void	clearHeader();

		void	setRequestKey(unsigned int);

		void	printHeaderValues()
		{
			std::map<std::string, std::string>::iterator iter = headerValues.begin();
			while (iter != headerValues.end()) {
				std::cout << iter->first << " | " << iter->second << "\n";
				++iter;
			}
		}

};