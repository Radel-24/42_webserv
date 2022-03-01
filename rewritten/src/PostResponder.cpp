#include "PostResponder.hpp"

void	PostResponder::createUploadFile( std::string filename, std::string content )
{
	std::ofstream	file;
	file.open(filename);
	file << content;
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
		name = cutBody.substr(name_start, name_end - name_start - 2);


		size_t	type_start = cutBody.find("Content-Type: ") + strlen("Content-Type: ");
		size_t	type_end = type_start;
		while(cutBody[type_end] != '\n')
			type_end++;
		content_type = cutBody.substr(type_start, type_end - type_start - 1);


		size_t	dblNewline = cutBody.find("\n\r\n");
		bodyContent = cutBody.substr(dblNewline + 3, cutBody.length() - dblNewline - 3);

		// remove new
		createUploadFile(filename + "_new", bodyContent);

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
		write(new_socket, "error: PostResponder: extractBoundary", 37);
		close(new_socket);
		return ;
	}

	_numOfBoundaries = countBoundaries();
	if (!_numOfBoundaries)
	{
		// kann eigentlich nicht sein, keine ahnung was dann passieren soll mrrrrrrkkk
		write(new_socket, "error: PostResponder: countBoundaries", 37);
		close(new_socket);
		return ;
	}

	if (_numOfBoundaries > 0)
		uploadFiles();

	write(new_socket, "file was created", 16);
	close(new_socket);
}