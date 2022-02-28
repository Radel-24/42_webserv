#include "../inc/utils.hpp"

std::pair<std::string, std::string>	divideString(std::string input, std::string divide) {
	size_t	break_pos = input.find(divide);
	std::string	key;
	std::string	value;
	if (break_pos != std::string::npos) {
		key = input.substr(0, break_pos);
		value = input.substr(break_pos + divide.length(), std::string::npos);
	}
	return (std::pair<std::string, std::string>(key, value));
}

std::map<std::string, std::string>	stringToMap(std::string input, std::string separate, std::string divide) {
	std::map<std::string, std::string> map;
	std::string substr;
	size_t found_pos = 0;
	size_t	start_pos = 0;
	while (found_pos != std::string::npos) {
		found_pos = input.find(separate, start_pos);
		substr = input.substr(start_pos, found_pos - start_pos);
		std::pair<std::string, std::string> pair;
		pair = divideString(substr, divide);
		if (pair.first != "")
			map.insert(pair);
		start_pos = found_pos + separate.length();
	}
	return map;
}