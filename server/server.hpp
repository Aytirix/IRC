#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include "../client/client.hpp"

class Server {
private:
    int socket_fd;
	std::map<int, Client *> clients;
    int port;
    std::string password;

public:
    Server(int port, const std::string& password);

    ~Server();

    void start();

	void loop();

    void acceptClient();

    void removeClient(Client* client);

    void handleConnections();

    bool verifyPassword(const std::string& password);
};

#endif