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

std::string ToHex(const std::string & s, bool upper_case /* = true */)
{
	std::stringstream ret;

	for (std::string::size_type i = 0; i < s.length(); ++i)
	{
		if (i % 2 == 0)
			ret << " ";
		if (i % 16 == 0)
			ret << "\n";
		ret << std::hex << std::setfill('0') << std::setw(2) << (upper_case ? std::uppercase : std::nouppercase) << (int)s[i];
	}

	return ret.str();
}

void	writeToSocket(int socket, std::string text) {
	write(socket, text.c_str(), text.length());
}

char ** mapToArray(std::map<std::string, std::string> map) {
	char ** array = (char **)calloc(sizeof(char *), map.size() + 1);
	size_t index = 0;

	for (std::map<std::string, std::string>::iterator iter = map.begin(); iter != map.end(); ++iter) {
		std::string str = iter->first + "=" + iter->second;
		char * c_str = (char *)calloc(sizeof(char *), str.length());
		char * helpStr = const_cast<char *>(str.c_str());
		size_t i = 0;
		while (helpStr[i]) {
			c_str[i] = helpStr[i];
			++i;
		}
		array[index] = c_str;
		++index;
	}
	array[index] = NULL;
	return array;
}

std::string toAbsolutPath(std::string path) {
	char * buf = getcwd(NULL, FILENAME_MAX);
	std::string retStr = buf;
	retStr += "/" + path;
	return (retStr);
}

int hex_to_decimal(std::string hex)
{
	std::transform(hex.begin(), hex.end(),hex.begin(), ::toupper);
	int len = hex.size();
	// Initializing base value to 1, i.e 16^0
	int base = 1;
	int dec_val = 0;
	// Extracting characters as digits from last
	// character
	for (int i = len - 1; i >= 0; i--) {
		// if character lies in '0'-'9', converting
		// it to integral 0-9 by subtracting 48 from
		// ASCII value
		if (hex[i] >= '0' && hex[i] <= '9') {
			dec_val += (int(hex[i]) - 48) * base;
			// incrementing base by power
			base = base * 16;
		}
		// if character lies in 'A'-'F' , converting
		// it to integral 10 - 15 by subtracting 55
		// from ASCII value
		else if (hex[i] >= 'A' && hex[i] <= 'F') {
			dec_val += (int(hex[i]) - 55) * base;
			// incrementing base by power
			base = base * 16;
		}
	}
	return dec_val;
}