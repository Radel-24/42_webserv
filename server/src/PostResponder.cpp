#include "PostResponder.hpp"
std::string ToHex(const std::string & s, bool upper_case /* = true */);

void	PostResponder::createUploadFile( std::string filename, std::string content )
{
	std::ofstream	file(".//files//" + filename);
	if (file.is_open())
		file << content; // else error
	//std::cout << "HEX\n" << ToHex(content, 0) << "\n";
	file.close();
}

void	PostResponder::uploadFiles( void )
{
	int				pos = 0;
	int				pos2 = -1;
	std::string		cutBody;
	std::string		filename;
	std::string		name;
	std::string		content_type;
	std::string		bodyContent;

	LOG_RED("_numOfBoundaries :	" << _numOfBoundaries);
	LOG_RED("_boundary :		" << _boundary);

	while (_numOfBoundaries > 1)
	{
		pos = _body.find(_boundary, pos2 + 1) + _boundary.length() + 2;
		pos2 = _body.find(_boundary, pos + 1);
		while (_body[--pos2] == '-')
			continue ;
		pos2 -= 1;


		cutBody = _body.substr(pos, pos2 - pos);


		size_t	file_start = cutBody.find("filename=") + strlen("filename=") + 1;
		size_t	file_end = file_start;
		while(cutBody[file_end] != '\n')
			file_end++;
		filename = cutBody.substr(file_start, file_end - file_start - 2);


		size_t	name_start = cutBody.find("name=") + strlen("name=") + 1;
		size_t	name_end = name_start;
		while(cutBody[name_end] != ';')
			name_end++;
		name = cutBody.substr(name_start, name_end - name_start - 1);


		size_t	type_start = cutBody.find("Content-Type: ") + strlen("Content-Type: ");
		size_t	type_end = type_start;
		while(cutBody[type_end] != '\n')
			type_end++;
		content_type = cutBody.substr(type_start, type_end - type_start - 1);


		size_t	dblNewline = cutBody.find("\n\r\n");
		bodyContent = cutBody.substr(dblNewline + 3, cutBody.length() - dblNewline - 3);

		LOG_RED("content_type :		" << content_type);
		LOG_RED("filename :		" << filename);
		LOG_RED("name :			" << name);

		// remove new
		createUploadFile(filename, bodyContent);

		_numOfBoundaries--;
	}
}

int	PostResponder::countBoundaries( void )
{
	size_t	count = 0;
	size_t	pos = _body.find(_boundary, 0);

	while(pos != std::string::npos)
	{
		pos = _body.find(_boundary, pos + 1);
		count++;
	}
	return count;
}

std::string	PostResponder::extractBoundary( void )
{
	size_t	start = _header.find("boundary=") + strlen("boundary=");
	if (start == std::string::npos)
		return "error";

	while (!isalpha(_header[start]) && !isdigit(_header[start]))
		start++;

	size_t end = start;
	while ((isalpha(_header[end]) || isdigit(_header[end])) && _header[end])
		end++;

	return _header.substr(start, end - start);
}

PostResponder::PostResponder( std::string header, std::string body, int new_socket ) : _header(header), _body(body)
{
	_boundary = extractBoundary();
	if (_boundary == "error")
	{
		// es gibt kein boundary, also wuden keine files geschickt und ich muss irgendwas anderes tun
		write(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: extractBoundary", 101);
		close(new_socket);
		return ;
	}

	_numOfBoundaries = countBoundaries();
	if (!_numOfBoundaries)
	{
		//createUploadFile("FELIX_new", body);
		// kann eigentlich nicht sein, keine ahnung was dann passieren soll mrrrrrrkkk
		write(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 37\n\nerror: PostResponder: countBoundaries", 101);
		close(new_socket);
		return ;
	}

	if (_numOfBoundaries > 0)
		uploadFiles();

	//write(new_socket, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 16\n\nfile was created", 80);
	char redirection[] = "HTTP/1.1 301 Moved Permanently\nLocation: http://127.0.0.1:7000/index.html\n\n";
	write(new_socket, redirection, strlen(redirection));
	close(new_socket);
}