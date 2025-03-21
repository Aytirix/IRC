#include <iostream>
#include "server/server.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <string>

int main(int ac, char **av)
{
	system("clear");
	std::cout << "\033[2J\033[1;1H" << "\033[32m" << "IRC Server" << "\033[0m" << std::endl;
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
