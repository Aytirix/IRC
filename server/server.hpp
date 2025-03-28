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
#include <arpa/inet.h>


class Parsing;
class Channel;
class Chatbot;
class Client;

class Server
{
public:
	Server(int port, const std::string &password);
	~Server();

	bool init();
	void run();
	
	void send_data(int client_fd, std::string data, bool server_name = true, bool date = true);

	// Getters
	int getPort() const { return _port; }

private:
	int _listen_fd;
	int _port;
	unsigned long _password;
	std::map<int, Client> _clients;
	std::map<std::string, Channel> _channels;
	Chatbot *_chatbot;

	bool setNonBlocking(int fd);
	void handleNewConnection();
	void DisconnectClient(Client &client, std::string message);
	void DisconnectClient(Client &client);
	void handleClientData(int client_fd);

	// Permet a parsing d'acceder au attribut privé de server
	friend class Parsing;
	friend class Chatbot;
};

#endif