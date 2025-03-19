
#ifndef PARSING_HPP
#define PARSING_HPP

#include <list>
#include <string>
#include <map>
#include "../client/client.hpp"

class Parsing
{
public:
	Parsing();

	~Parsing();

	std::map<std::string, std::string> init_parsing(std::string buffer);
};

#endif