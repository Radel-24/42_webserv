#pragma once

#include <sys/time.h>
#include "Request.hpp"

class Client {
	private:
		timeval		logout_time;
		Request		request;
		int			socket;

	public:
		Client();
		~Client();
};