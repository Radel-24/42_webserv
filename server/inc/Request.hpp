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
	CLOSE_CONNECTION
};

class	Request {
	public:
		int									file_created;
		int									status;
		int									socket;
		Server *							server;
		bool								cgi_request;
		std::map<std::string, std::string>	headerValues;
		std::string							body;
		std::string							header;
		size_t								bytes_written;
		PostResponder *						pr;
		Location *							location;
		std::string							path;

		std::string							headerKeyValuePairs;
		bool								responseCreated;



		std::string							filename;

		bool								closeConnection;

		std::string							response;

	private:
		unsigned int						requestKey;
		ssize_t								bytes_read;
		std::string							uploadPath;

		void								extractFilename();
		void								writeResponse();
						
		void								processHeader(std::map<int, Server *> & servers);
		void								getBodyOutOfHeader();
						
	public:						
		Request();
		Request(int socket, Server * server);
		~Request();
		void								init();
		int									getRequestKey() const;
		std::string							getHeader() const;
		std::string							getBody() const;
		int									checkHeaderRead(void);
		int									checkBodySize(void);

		void								clearResponse();

		void								appendHeader(char * input, size_t size);
		void								appendBody( char *, size_t);
		void								setRequestKey(unsigned int);

		void								detectCorrectServer(std::map<int, Server *> & servers);

		void								readRequest(std::map<int, Server *> & servers);
		void								writeRequest();

		void								setType();
		void								changePath();
		void								setPath();
		void								readHeader();
		void								readBodyLength();
		void								readBodyChunked();

		void								hundredStatus();

		void								postResponder();

		int									checkBodySizeChunk();
		int									chunkSize();

		std::string							formatString( std::string file_content);


		void								responder();
		std::string							getFilename();

		void								deleteResponder( void );
		void								parseHeader(std::string header);
		std::pair<std::string, std::string>	splitToken( std::string token );
		std::string							&leftTrim( std::string &str, char c );
		void								setHeaderValues( const std::pair<std::string, std::string> &pair );
		void								checkHeaderValues( void );

		void								checkRequest();
		std::string							readFile( std::string filename );

		std::string							getHostName( void ) const;

		bool								checkDirectoryListing( void );
		void								doDirectoryListing( void );
		void								refreshFilesHTML( void );
};
