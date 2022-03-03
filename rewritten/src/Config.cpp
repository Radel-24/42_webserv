#include "Config.hpp"

/*
separate separates the key - value pairs from each other
divide divides between the key and the value
comment is the identifier for a comment line
*/
void	Config::buildMap(std::string file, std::string divide, char separate, std::string comment) {
	std::ifstream fin(file);
	std::string input;
	std::string line;

	if (fin.is_open()) {
		while (getline(fin, line))
			input += line + "\n";
		fin.close();
		map = stringToMap(input, divide, separate, comment);
	}
	else {
		// TODO error handling, config not readable
		std::cout << "Config file not readable\n";
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

void	Config::readIntVars(std::string names[], int ** ints, int size) {
	std::map<std::string, std::string>::iterator iter;
	for (int i = 0; i < size; ++i) {
		iter = map.find(names[i]);
		if (iter != map.end()) {
			*ints[i] = atoi((iter->second).c_str());
		}
	}
}

void	Config::readDoubleVars(std::string names[], double ** doubles, int size) {
	std::map<std::string, std::string>::iterator iter;
	for (int i = 0; i < size; ++i) {
		iter = map.find(names[i]);
		if (iter != map.end()) {
			*doubles[i] = atof((iter->second).c_str());
		}
	}
}

void	Config::readStrVars(std::string names[], std::string ** strs, int size) {
	std::map<std::string, std::string>::iterator iter;
	for (int i = 0; i < size; ++i) {
		iter = map.find(names[i]);
		if (iter != map.end()) {
			*strs[i] = iter->second;
		}
	}
}

void	Config::printMap() {
	for (std::map<std::string, std::string>::iterator iter = map.begin(); iter != map.end(); ++iter) {
		std::cout << iter->first << " | " << iter->second << "\n";
	}
}