#include "utils.hpp"

std::pair<std::string, std::string>	divideInput(std::string input) {
	size_t	break_pos = input.find("\n\r\n");
	std::string	header;
	std::string	body;
	if (break_pos != std::string::npos) {
		header = input.substr(0, break_pos);
		body = input.substr(break_pos, std::string::npos);
	}
	return (std::pair<std::string, std::string>(header, body));
}