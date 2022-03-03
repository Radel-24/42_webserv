#pragma once

#include <string>
#include <map>
#include <iostream>
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
		int		getRequestKey() const;
		std::string	getHeader() const;
		std::string	getBody() const;
		int			checkHeader(void);
		int			checkBodySize(void);

		void	appendHeader(std::string input);
		void	appendBody(std::string body);
		void	setHeaderValues(std::map<std::string, std::string> mappe);

		void	setBody();
		void	setHeader();

		void	setRequestKey(unsigned int);

		void	printHeaderValues() {
			std::map<std::string, std::string>::iterator iter = headerValues.begin();
			while (iter != headerValues.end()) {
				std::cout << iter->first << " | " << iter->second << "\n";
				++iter;
			}
		}
};