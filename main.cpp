#include <iostream>
#include "server/server.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <string>

int main(int ac, char **av)
{

	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int port = atoi(av[1]);
	if (port == 0)
	{
		std::cerr << "Error: Invalid port number." << std::endl;
		return 1;
	}

	std::string password(av[2]);
	Server server(port, password);
	if (!server.init())
		return 1;
	server.run();
	return 0;
}
