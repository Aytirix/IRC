#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <stdio.h>
#include "../parsing/Parsing.hpp"
#include "../client/client.hpp"

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();

		bool init();
		void run();

		void Server::send_data(int client_fd, std::string data);

	private:
		int listen_fd_;
		int port_;
		std::string password_;
		std::map<int, Client> clients_;

		bool setNonBlocking(int fd);
		void handleNewConnection();
		void handleClientData(int client_fd);
};

#endif