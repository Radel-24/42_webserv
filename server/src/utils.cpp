#include "utils.hpp"

std::vector<std::string>	stringSplit(std::string sep, std::string str) {
	std::vector<std::string>	elems;

	size_t pos_start = 0;
	size_t pos_end = str.find(sep);
	while (pos_end != std::string::npos) {
		elems.push_back(str.substr(pos_start, pos_end - pos_start));
		pos_start = pos_end + sep.length();
		pos_end = str.find(sep, pos_start);
	}
	elems.push_back(str.substr(pos_start, pos_end - pos_start));
	return elems;
}

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

std::map<std::string, std::string>	stringToMap(std::string input, std::string divide,
												char separate, std::string comment) {
	std::map<std::string, std::string> map;
	std::istringstream	istr(input);
	std::string		line;

	while (std::getline(istr, line, separate)) {
		if (size_t pos = line.find(comment) != std::string::npos)
			line = line.substr(0, pos);
		std::string key = line.substr(0, line.find(divide));
		std::string value = line.substr(line.find(divide) + divide.length(), std::string::npos);
		if (key != "" && value != "")
			map.insert(std::pair<std::string, std::string>(key, value));
	}
	return map;
}

std::string findBlock(std::string input, std::string blockBegin, std::string blockEnd) {
	std::string blockContent;

	if (size_t posBegin = input.find(blockBegin) != std::string::npos) {
		if (size_t posEnd = input.find(blockEnd) != std::string::npos) {
			return input.substr(posBegin, posEnd);
		}
		else {
			// TODO error handling syntax error
			std::cout << "Syntax error near " << blockBegin << "\n";
		}
	}
	return NULL;
}