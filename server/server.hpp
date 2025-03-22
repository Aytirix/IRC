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
#include "../log/log.hpp"
#include "../channel/channel.hpp"
#include "hasher.hpp"

class Parsing;
class Channel;

class Server
{
public:
	Server(int port, const std::string &password);
	~Server();

	bool init();
	void run();
	
	void send_data(int client_fd, std::string data, bool server_name = true, bool date = true);
	void joinChannel(Client &client, std::string &channelName);

	// Getters
	int getPort() const { return port_; }

private:
	int listen_fd_;
	int port_;
	unsigned long password_;
	std::map<int, Client> clients_;
	std::map<std::string, Channel> channels_;

	bool setNonBlocking(int fd);
	void handleNewConnection();
	void DisconnectClient(Client &client);
	void handleClientData(int client_fd);

	// Permet a parsing d'acceder au attribut privé de server
	friend class Parsing;
};

#endif