#pragma once

#include <string>

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
		unsigned int			requestKey;

	public:
		int		getRequestKey() const;
		std::string	getHeader() const;
		std::string	getBody() const;
		int			checkHeader(void);
		int			checkBodySize(void);

		void	setHeader(std::string input);
		void	setBody(std::string body);

		void	setRequestKey();
};