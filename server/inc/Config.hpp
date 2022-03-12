#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include "utils.hpp"

#include "Server.hpp"

#define SUCCESS 0
#define FAILURE 1
#define STR_END std::string::npos

//class Config {
//	private:
//		std::map<std::string, std::string>	map;

//	public:

//		void	buildMap(std::string file, std::string divide = ": ", char separate = '\n', std::string comment = "#");
//		void	printMap();

//		bool	checkNecessaryKeys(std::vector<std::string> vec);

//		void	readIntVars(std::string names[], int ** ints, int size);
//		void	readDoubleVars(std::string names[], double ** doubles, int size);
//		void	readStrVars(std::string names[], std::string ** strs, int size);
//};

//void	loadConfig(Config & config);
//void	printConfig(Config & config);

int	read_config(std::string file, std::vector<Server *> servers);