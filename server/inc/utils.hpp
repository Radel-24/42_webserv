#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <istream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <unistd.h>

std::pair<std::string, std::string>	divideString(std::string input, std::string divide);
std::map<std::string, std::string>	stringToMap(std::string input, std::string divide = ": ",
											char separate = '\n', std::string comment = "#");
std::vector<std::string> stringSplit(std::string sep, std::string str);

std::string	readFile(std::string filename);

std::string ToHex(const std::string & s, bool upper_case /* = true */);

template <class T>
bool	findInVector(std::vector<T> vec, T needle) {
	for (typename std::vector<T>::iterator elem = vec.begin(); elem != vec.end(); ++elem) {
		if (*elem == needle)
			return true;
	}
	return false;
}

void	writeToSocket(int socket, std::string text);