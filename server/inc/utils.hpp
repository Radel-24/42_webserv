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

#define LOG(x) (std::cout << x << std::endl)

#define LOG_RED(x) (std::cout << "\033[1;31m" << x << "\033[0m" << std::endl)
#define LOG_GREEN(x) (std::cout <<  "\033[1;32m" << x << "\033[0m" << std::endl)
#define LOG_YELLOW(x) (std::cout << "\033[1;33m" << x << "\033[0m" << std::endl)
#define LOG_BLUE(x) (std::cout << "\033[1;34m" << x << "\033[0m" << std::endl)
#define LOG_BLACK(x) (std::cout << "\033[1;30m" << x << "\033[0m" << std::endl)
#define LOG_PINK(x) (std::cout << "\033[1;35m" << x << "\033[0m" << std::endl)
#define LOG_CYAN(x) (std::cout << "\033[1;36m" << x << "\033[0m" << std::endl)
#define LOG_WHITE(x) (std::cout << "\033[1;37m" << x << "\033[0m" << std::endl)

#define LOG_RED_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;31m" << x << "\033[0m" << std::endl)
#define LOG_GREEN_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;32m" << x << "\033[0m" << std::endl)
#define LOG_YELLOW_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;33m" << x << "\033[0m" << std::endl)
#define LOG_BLUE_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;34m" << x << "\033[0m" << std::endl)
#define LOG_BLACK_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;30m" << x << "\033[0m" << std::endl)
#define LOG_PINK_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;35m" << x << "\033[0m" << std::endl)
#define LOG_CYAN_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;36m" << x << "\033[0m" << std::endl)
#define LOG_WHITE_INFO(x) (std::cout << "(file: " << __FILE__ << ", line: " << __LINE__ << ") " << "\033[1;37m" << x << "\033[0m" << std::endl)


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

int	writeToSocket(int socket, std::string text);

char **	mapToArray(std::map<std::string, std::string> map);

std::string	toAbsolutPath(std::string path);

void	writeStatus(int status, int socket);

int hex_to_decimal(std::string hex);