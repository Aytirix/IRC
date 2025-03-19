
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

	void init_parsing(Client client, std::string buffer);
};

#endif