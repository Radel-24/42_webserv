#include "Config.hpp"

/*
separate separates the key - value pairs from each other
divide divides between the key and the value
comment is the identifier for a comment line
*/
void	Config::buildMap(std::string file, std::string divide, char separate, std::string comment) {
	std::ifstream	fin(file);
	std::string		line;

	while (std::getline(fin, line, separate)) {
		if (line.find(comment) == 0)
			continue ;
		std::string key = line.substr(0, line.find(divide));
		std::string value = line.substr(line.find(divide) + divide.length(), std::string::npos);
		if (key != "" && value != "")
			map.insert(std::pair<std::string, std::string>(key, value));
	}
}


		//std::istringstream sin(line.substr(line.find(divide) + divide.length()));
		//if (line.find("port") != std::string::npos) { sin >> config.port; }
		//else if (line.find("server_name") != std::string::npos) { sin >> config.server_name; }



bool	Config::checkNecessaryKeys(std::vector<std::string> vec) {
	for (std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter) {
		std::cout << "search: " << *iter << "\n";
		if (map.find(*iter) == map.end())
			return false;
	}
	return true;
}

void	Config::readIntVars(std::string names[], int & (&ints)[], int size) {
	std::map<std::string, std::string>::iterator iter;
	for (int i = 0; i < size; ++i) {
		iter = map.find(names[i]);
		if (iter != map.end()) {
			ints[i] = atoi((iter->second).c_str());
		}
	}
}

void	loadConfig(Config & config) {
	std::ifstream	fin("setup.conf");
	std::string	line;
	while (std::getline(fin, line)) {
		if (line[0] == '#')
			continue ;
		std::istringstream sin(line.substr(line.find(":") + 1));
		if (line.find("port") != std::string::npos) { sin >> config.port; }
		else if (line.find("server_name") != std::string::npos) { sin >> config.server_name; }

	}
}

void	printConfig(Config & config) {
	std::cout << "port: " << config.port << "\n";
	std::cout << "server name: " << config.server_name << "\n";
}


void	Config::printMap() {
	for (std::map<std::string, std::string>::iterator iter = map.begin(); iter != map.end(); ++iter) {
		std::cout << iter->first << " | " << iter->second << "\n";
	}
}