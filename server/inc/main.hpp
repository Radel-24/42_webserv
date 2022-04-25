#pragma once

#include <iostream>
#include <unistd.h>
#include <string>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdint>

#include "Request.hpp"
#include "utils.hpp"
#include "Config.hpp"
#include "PostResponder.hpp"

#include <sys/time.h>

#include <set>

#include <signal.h>

typedef struct s_accepter {
	int							highestSocket;
	fd_set						watching_read_sockets;
	fd_set						watching_write_sockets;
	std::map<int, Request *>	requests;
}	t_accepter;