#pragma once

#include <sys/socket.h>
#include <string>
#include <map>
#include <iostream>
#include <vector>

#include "Server.hpp"
#include "PostResponder.hpp"
#include "utils.hpp"

#include <sys/stat.h>

class Server;
class PostResponder;

enum ReqKeys {
	NIL,
	GET,
	HEAD,
	POST,
	PUT,
	DELETE
};

enum Status {
	NILL,
	READING_HEADER,
	HEADER_READ,
	READING_BODY,
	DONE_READING,
	WRITING,
	DONE_WRITING,
	DONE_WRITING_CGI,
	CLOSE_CONNECTION
};

class	Request {
	public:
		int file_created;
		int	status;
		int									socket;
		Server *							server;
		bool	cgi_request;
		std::map<std::string, std::string>	headerValues;
		std::string							body;
		std::string							header;
		ssize_t								bytes_written;
		PostResponder *	postResponder;



		std::string							filename;
	private:
		unsigned int						requestKey;
		ssize_t								bytes_read;
		std::string							path;
		std::string							uploadPath;
		Location *							location;
		std::string	chunk;


		void					extractFilename();


	private:
		void	processHeader(std::map<int, Server *> & servers);
		void	getBodyOutOfHeader();

	public:
		void	init();
		Request();
		Request(int socket, Server * server);
		~Request();
		int			getRequestKey() const;
		std::string	getHeader() const;
		std::string	getBody() const;
		int			checkHeaderRead(void);
		int			checkBodySize(void);

		void	appendHeader(char * input, size_t size);
		void	appendBody( char *, size_t);
		// void	setHeaderValues(std::pair<std::string, std::string> pair);

		//void	clearBody();
		//void	clearHeader();

		void	setRequestKey(unsigned int);

		void	detectCorrectServer(std::map<int, Server *> & servers);

		void	readRequest(std::map<int, Server *> & servers);
		void writeRequest();

		void	setType();
		void	changePath();
		void	setPath();
		std::string	getPath();
		std::string	getUploadPath();
		void	readHeader();
		void	readBodyLength();
		void	readBodyChunked();

		int	checkBodySizeChunk();
		int	chunkSize();

		std::string	formatString( std::string file_content);


		void	responder();
		std::string	getFilename();

		void	printHeaderValues() {
			std::map<std::string, std::string>::iterator iter = headerValues.begin();
			while (iter != headerValues.end()) {
				LOG_GREEN(iter->first << " | " << iter->second);
				++iter;
			}
		}

		/* start alex new */
		void								deleteResponder( void );
		void								parseHeader(std::string header);
		std::pair<std::string, std::string>	splitToken( std::string token );
		std::string							&leftTrim( std::string &str, char c );
		void								setHeaderValues( const std::pair<std::string, std::string> &pair );
		std::map<std::string, std::string>	getHeaderValues( void ) const {
			return this->headerValues;
		}
		Server								*getServer( void ) const {
			return this->server;
		}
		void									checkHeaderValues( void );

		void	checkRequest();
		std::string	readFile( std::string filename );

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

		// Location *	checkDirectoryListing( std::string requestedPath );
		// void		doDirectoryListing( Location * locationToList );
		bool		checkDirectoryListing( void );
		void		doDirectoryListing( void );
		/* end alex new */

};


