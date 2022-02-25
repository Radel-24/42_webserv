#include "utils.hpp"

std::pair<std::string, std::string>	divideString(std::string input, std::string divide) {
	size_t	break_pos = input.find(divide);
	std::string	header;
	std::string	body;
	if (break_pos != std::string::npos) {
		header = input.substr(0, break_pos);
		body = input.substr(break_pos + divide.length(), std::string::npos);
	}
	return (std::pair<std::string, std::string>(header, body));
}