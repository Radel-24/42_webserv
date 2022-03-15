#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "utils.hpp"

#include "Server.hpp"

#define SUCCESS 0
#define FAILURE 1
#define STR_END std::string::npos


int	read_config(std::string file, std::map<int, Server *> & servers);
void	check_config(std::map<int, Server *> & servers);