#include <iostream>
#include <csignal>
#include "server/server.hpp"
#include "log/Colors.hpp"
#include <stdlib.h>
#include <stdexcept>
#include <string>

volatile bool stop = false;

void handleSignal(int signal)
{
	if (signal == SIGINT)
		stop = true;
}

void font()
{
	std::cout << BRIGHT_RED "██ ██████   ██████" << std::endl;
	std::cout << "██ ██   ██ ██" << std::endl;
	std::cout << "██ ██████  ██" << std::endl;
	std::cout << "██ ██   ██ ██" << std::endl;
	std::cout << "██ ██   ██  ██████" << std::endl
			  << std::endl
			  << std::endl;

	std::cout << "███████ ███████ ██████  ██    ██ ███████ ██████" << std::endl;
	std::cout << "██      ██      ██   ██ ██    ██ ██      ██   ██" << std::endl;
	std::cout << "███████ █████   ██████  ██    ██ █████   ██████" << std::endl;
	std::cout << "     ██ ██      ██   ██  ██  ██  ██      ██   ██" << std::endl;
	std::cout << "███████ ███████ ██   ██   ████   ███████ ██   ██" RESET << std::endl
			  << std::endl;
}

int main(int ac, char **av)
{
	// Associer le signal SIGINT à la fonction handleSignal
	std::signal(SIGINT, handleSignal);

	system("clear");
	font();
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
	Server server(port, password, stop);
	if (!server.init())
		return 1;

	server.run();
	return 0;
}
