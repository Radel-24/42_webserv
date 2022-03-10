#include "ConfigToken.hpp"

const std::string ConfigToken::general_tokens[] = { "#", "{", "}", "location", "listen", "server_name", "root", "return"};
const std::string ConfigToken::location_tokens[] = {"path", "root" "listen", "charset", "server_name", "client_max_body_size", "proxy_pass"};

ConfigToken::ConfigToken() { }

ConfigToken::~ConfigToken() { }


