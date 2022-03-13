#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <istream>
#include <map>
#include <vector>

std::pair<std::string, std::string>	divideString(std::string input, std::string divide);
std::map<std::string, std::string>	stringToMap(std::string input, std::string divide = ": ",
											char separate = '\n', std::string comment = "#");
std::vector<std::string> stringSplit(std::string sep, std::string str);

std::string	readFile(std::string filename);