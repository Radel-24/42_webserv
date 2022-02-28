#pragma once

#include <string>
#include <iostream>
#include <map>

std::pair<std::string, std::string>	divideString(std::string input, std::string divide);
std::map<std::string, std::string>	stringToMap(std::string input, std::string separate, std::string divide);