#include "Config.hpp"

/*
separate separates the key - value pairs from each other
divide divides between the key and the value
comment is the identifier for a comment line
*/
//void	Config::buildMap(std::string file, std::string divide, char separate, std::string comment) {
//	std::ifstream fin(file);
//	std::string input;
//	std::string line;

//	if (fin.is_open()) {
//		while (getline(fin, line))
//			input += line + "\n";
//		fin.close();
//		map = stringToMap(input, divide, separate, comment);
//	}
//	else {
//		// TODO error handling, config not readable
//		std::cout << "Config file not readable\n";
//	}
//}


//		//std::istringstream sin(line.substr(line.find(divide) + divide.length()));
//		//if (line.find("port") != std::string::npos) { sin >> config.port; }
//		//else if (line.find("server_name") != std::string::npos) { sin >> config.server_name; }



//bool	Config::checkNecessaryKeys(std::vector<std::string> vec) {
//	for (std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter) {
//		std::cout << "search: " << *iter << "\n";
//		if (map.find(*iter) == map.end())
//			return false;
//	}
//	return true;
//}

//void	Config::readIntVars(std::string names[], int ** ints, int size) {
//	std::map<std::string, std::string>::iterator iter;
//	for (int i = 0; i < size; ++i) {
//		iter = map.find(names[i]);
//		if (iter != map.end()) {
//			*ints[i] = atoi((iter->second).c_str());
//		}
//	}
//}

//void	Config::readDoubleVars(std::string names[], double ** doubles, int size) {
//	std::map<std::string, std::string>::iterator iter;
//	for (int i = 0; i < size; ++i) {
//		iter = map.find(names[i]);
//		if (iter != map.end()) {
//			*doubles[i] = atof((iter->second).c_str());
//		}
//	}
//}

//void	Config::readStrVars(std::string names[], std::string ** strs, int size) {
//	std::map<std::string, std::string>::iterator iter;
//	for (int i = 0; i < size; ++i) {
//		iter = map.find(names[i]);
//		if (iter != map.end()) {
//			*strs[i] = iter->second;
//		}
//	}
//}

//void	Config::printMap() {
//	for (std::map<std::string, std::string>::iterator iter = map.begin(); iter != map.end(); ++iter) {
//		std::cout << iter->first << " | " << iter->second << "\n";
//	}
//}

size_t	is_parameter(std::string const & parameter, std::string const & line) {
	if (line.find(parameter) == 0) {
		return parameter.length();
	}
	return 0;
}

void	remove_comments(std::string &line) {
	size_t pos = line.find("#");
	if (pos != std::string::npos) {
		line = line.substr(0, pos);
	}
}

void	remove_whitespace(std::string &line) {
	size_t pos = line.find_first_not_of("\t ");
	if (pos != std::string::npos)
		line = line.substr(pos, std::string::npos);
}

int	location_parser(std::ifstream &fin, Location &location) {
	std::string line;
	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (line.find("}") != STR_END) { break; }
		else if (line.find("directory_listing: ") == 0) {
			line = line.substr(strlen("directory_listing: "), std::string::npos);
			if (line == "on") { location.directory_listing = true; }
			else if (line == "off") { location.directory_listing = false; }
			else { return FAILURE; }
		}
	}
	return SUCCESS;
}

int	server_parser(std::ifstream &fin) {
	Server server;
	// TODO add server to main data struct or something else
	std::string line;

	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (line.find("}") != STR_END) { break; }
		else if (size_t pos = is_parameter("server_name: ", line)) {server.server_name = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("client_max_body_size: ", line)) {server.client_max_body_size = atof(line.substr(pos).c_str()); }
		else if (is_parameter("location: ", line)) {
			std::string path = line.substr(strlen("location"), line.find("{"));
			if (line.find("{") != line.length() - 1) {
				std::cout << "Wrong formatting\n";
				// TODO error handling or in calling function/
				return FAILURE;
			}
			Location *location = new Location(path);
			location_parser(fin, *location);
		}

	}
	return SUCCESS; // TODO
}

int	main_parser(std::ifstream &fin) {
	std::string line;

	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (is_parameter("server{", line)) {
			server_parser(fin);
		}

	}
	return SUCCESS; // TODO
}

int	read_config(std::string file) {
	std::ifstream fin(file);
	std::string input;
	std::string line;

	if (fin.is_open()) {
		if (main_parser(fin) != SUCCESS) {
			std::cout << "Reading the config file failed\n";
			exit(EXIT_SUCCESS);
			// TODO other exit strategy?
		}
	}
	else {
		// TODO error handling, config not readable
		std::cout << "Config file not readable\n";
	}
	return SUCCESS; // TODO
}