#include "Config.hpp"
#include "PostResponder.hpp"

size_t	is_parameter(std::string const & parameter, std::string const & line) {
	if (line.find(parameter) == 0) {
		return parameter.length();
	}
	return 0;
}

void	remove_comments(std::string &line) {
	size_t pos = line.find_first_of("#");
	if (pos != std::string::npos) {
		line.erase(pos);
	}
}

void	remove_whitespace(std::string &line) {
	size_t pos = line.find_first_not_of("\t ");
	if (pos != std::string::npos)
		line = line.substr(pos, std::string::npos);
	else {
		line.clear();
		return;
	}
	pos = line.find_last_not_of("\t ");
	if (pos != std::string::npos)
		line.erase(pos + 1);
}


/*
This function reads all the data which is inside of the parantheses after the location variable.
It checks for different parameter and puts them inside our location class.
That we can access them later, and check for possible error handling.
*/
int	location_parser(std::ifstream &fin, Location &location) {
	std::string line;
	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (line.find("}") != STR_END) { break; }
		else if (size_t pos = is_parameter("root: ", line)) { location.root = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("default_file: ", line)) { location.default_file = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("return: ", line)) { location.redirection = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("client_max_body_size: ", line)) { location.client_max_body_size = atol(line.substr(pos).c_str()); }
		else if (line.find("directory_listing: ") == 0) {
			line = line.substr(strlen("directory_listing: "), std::string::npos);
			if (line == "on") { location.directory_listing = true; }
			else if (line == "off") { location.directory_listing = false; }
			else { return FAILURE; }
		}
		else if (size_t pos = is_parameter("methods: ", line)) { location.methods = stringSplit(", ", line.substr(pos, STR_END)); }
		else if (line.empty()) { continue; }
		else {
			LOG_RED_INFO("Not a valid parameter in location line: " << line);
			exit(EXIT_SUCCESS);
		}
	}
	return SUCCESS;
}

/*
This function reads all the data which is inside of the parantheses after the server variable.
It checks for different parameter and puts them inside our server class.
That we can access them later, and check for possible error handling.
*/
int	server_parser(std::ifstream &fin, Server & server) {
	std::string line;
	std::cout << std::endl;
	LOG_YELLOW("server parser start");
	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (line.find("}") != STR_END) { break; }
		else if (size_t pos = is_parameter("server_name: ", line)) { server.server_name = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("root: ", line)) { server.root = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("upload: ", line)) { server.uploadPath = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("client_max_body_size: ", line)) { server.client_max_body_size = atoi(line.substr(pos).c_str()); }
		else if (size_t pos = is_parameter("cgi_extension: ", line)) { server.cgi_extension = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("cgi_path: ", line)) { server.cgi_path = line.substr(pos, STR_END); }
		else if (size_t pos = is_parameter("listen: ", line)) { server.port = atol(line.substr(pos).c_str()); }
		else if (is_parameter("location ", line)) {
			std::string var = "location ";
			std::string path = line.substr(var.length(), line.find(" {") - var.length());
			if (line.find("{") != line.length() - 1) {
				std::cout << "Wrong formatting\n";
				return FAILURE;
			}
			Location *location = new Location(path);
			server.locations.insert(std::pair<std::string, Location*>(path, location));
			location_parser(fin, *location);
		}
		else if (line.empty()) { continue; }
		else {
			LOG_RED_INFO("Not a valid parameter in location line: " << line);
			exit(EXIT_SUCCESS);
		}
	}
	LOG_PINK("server name: " << server.server_name);
	LOG_PINK("server port: " << server.port);
	LOG_PINK("server upload path: " << server.uploadPath);
	return SUCCESS;
}

/*
	This function created the main structure of the server. (directorys)
	It then copies all the data into the root directory(default server)
*/
// TODO why is root: /www/defualt server?
void	createServerDirectory(Server *server) {

	// std::string	serverWarehouse = "/www/";
	std::string	directory1 = "." + server->root + "/" + server->server_name;
	std::string	directory2 = "." + server->root + "/" + server->server_name + server->uploadPath;

	// create server diretory
	if (!dirExists(directory1.c_str())) {
		std::string	createDirectory1 = "mkdir " + directory1;
		system(createDirectory1.c_str());
	}
	if (!dirExists(directory2.c_str())) {
		std::string	createDirectory2 = "mkdir " + directory2;
		system(createDirectory2.c_str());
	}

	// copy neccesarry files over from prototype (/www/default_server)
	if (dirExists("./www/default_server")) {
		std::string	copy = "cp -n ./www/default_server/* " + directory1 + "/"; // -n option prevents overrides
		system(copy.c_str());
		if (dirExists("./www/default_server/img")) {
			std::string	copy2 = "cp -n -R ./www/default_server/img " + directory1 + "/";
			system(copy2.c_str());
		}
		else {
			LOG_RED_INFO("ERROR: NO DEFAULT SERVER GIVEN");
			// exit(0); // TODO what do do here?
		}
	}
	else {
		LOG_RED_INFO("ERROR: NO DEFAULT SERVER GIVEN");
		// exit(0); // TODO what do do here?
	}
	LOG_GREEN("created: server directory");
	// server->updateFilesHTML(); TODO is this neccesary?
	// LOG_GREEN("created: files.html");
}


/*
This function reads the input file line by line and deletes all the comments(#) and the whitespaces(\t ) out.
If the parameter SERVER is found it will create a new server class and parse it into the server parser to
extract the server data and write them into the newly created class.
Then it will be configured (starting server/binding ports etc.) in the configure function.
If there is a false line OR an empty file we exit with an error.
*/
int	main_parser(std::ifstream &fin, std::map<int, Server *> & servers) {
	std::string line;

	while (getline(fin, line)) {
		remove_comments(line);
		remove_whitespace(line);
		if (is_parameter("server{", line)) {
			Server *server = new Server();
			server_parser(fin, *server);
			server->configure(servers);
			servers.insert(std::pair<int, Server *>(server->sock, server));
			//for (std::map<std::string, Location*>::iterator iter = server->locations.begin(); iter != server->locations.end(); ++iter) {
			//	if (iter->second->directory_listing == true) {
			//		server->updateFilesHTML();
			//	}
			//}
			// createServerDirectory(server); // TODO unnessecary?
		}
		else if (line.empty()) { continue; }
		else {
			std::cout << "wrong line: " << line << "\n";
			exit(EXIT_SUCCESS);
		}
	}
	return SUCCESS; // TODO
}


/*
This function opens the config-file in an ifstream and then send it to the parser.
If the config is not readable we exit with an error.
*/
int	read_config(std::string file, std::map<int, Server *> & servers) {
	std::ifstream fin(file);
	std::string input;
	std::string line;


	if (fin.is_open()) {
		if (main_parser(fin, servers) != SUCCESS) {
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

void	check_config(std::map<int, Server *> & servers) {
	if (servers.empty()) {
		std::cout << "no server configured\n";
		exit(EXIT_SUCCESS);
	}
	// TODO check for several servers listening on same port //ALEX DID THIS I GUESS?
}