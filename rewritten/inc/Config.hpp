#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include "utils.hpp"


class Config {
	private:
		std::map<std::string, std::string>	map;

	public:
		std::string	server_name;
		std::string	root;
		std::string	index;
		std::string	try_files;
		int	port;

	void	buildMap(std::string file, std::string divide = ": ", char separate = '\n', std::string comment = "#");
	void	printMap();

	bool	checkNecessaryKeys(std::vector<std::string> vec);
};

void	loadConfig(Config & config);
void	printConfig(Config & config);
