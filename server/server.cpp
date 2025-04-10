#include "server.hpp"

#include "../chatbot/chatbot.hpp"
#include "../client/client.hpp"
#include "../channel/channel.hpp"
#include "../parsing/Parsing.hpp"
#include "../log/log.hpp"
#include "responses.hpp"
#include "hasher.hpp"

Server::Server(int port, const std::string &password, volatile bool &stop) : _listen_fd(-1), _port(port), _password(Hasher::hash(password)), _stop(stop)
{
	// initialiser la map des clients
	_clients.clear();
	_channels.clear();
	_chatbot = new Chatbot(*this);
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		pollfd socketPfd = it->second.getSocketPfd();
		socketPfd.events = 0;
		socketPfd.revents = 0;
		socketPfd.fd = -1;
		it->second.setSocketPfd(socketPfd);
		it->second.setNickname("");
		it->second.setUserName("");
		it->second.setRealName("");
		it->second.setIp("");
		it->second.getBuffer().clear();
		it->second.setPasswordNotVerified();
		it->second.getSendBuffer().clear();
		_chatbot->deleteClient(&it->second);
	}
	delete _chatbot;
}

/**
 * @brief Démarre le serveur et attend les connexions des clients
 **/
void Server::run()
{
	while (true)
	{
		// Vérifier si le serveur doit être arrêté
		if (_stop)
		{
			log::log::write(log::log::INFO, "Arrêt du serveur...");
			break;
		}

		// Reconstruire le tableau de pollfd à partir de la map
		std::vector<pollfd> pollfds;
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			pollfds.push_back(it->second.getSocketPfd());

		int ret = poll(&pollfds[0], pollfds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		// parcourir la liste des clients et lle supprimer s'il est déconnecté
		std::map<int, Client>::iterator it = _clients.begin();
		while (it != _clients.end())
		{
			if (it->second.getSocketFd() == -1)
			{
				log::log::write(log::log::INFO, "Suppression du client : fd(" + log::toString(it->first) + ")");
				std::map<int, Client>::iterator toErase = it;
				++it;
				_clients.erase(toErase);
			}
			else
				++it;
		}

		// Parcourir les descripteurs ayant généré un événement
		for (size_t i = 0; i < pollfds.size(); ++i)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int fd = pollfds[i].fd;
				// Si le descripteur est celui du socket d'écoute, c'est une nouvelle connexion
				// Sinon, c'est un client qui envoie des données
				if (fd == _listen_fd)
					handleNewConnection();
				else
					handleClientData(fd);
			}
		}
	}
}

/**
 * @brief Initialisation du serveur
 * Création du socket d'écoute, configuration de l'adresse du serveur
 * et mise en mode non bloquant du socket d'écoute
 * @return true si l'initialisation a réussi, false sinon
 **/
bool Server::init()
{
	// Création du socket d'écoute
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd < 0)
	{
		perror("socket");
		return false;
	}

	// Mettre le socket en mode non bloquant
	if (!setNonBlocking(_listen_fd))
	{
		perror("setNonBlocking");
		close(_listen_fd);
		return false;
	}

	// Configuration de l'adresse du serveur
	sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(_port);

	// Permettre la réutilisation de l'adresse du serveur
	int opt = 1;
	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		close(_listen_fd);
		return false;
	}

	// Lier le socket d'écoute à l'adresse du serveur
	if (bind(_listen_fd, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		close(_listen_fd);
		return false;
	}

	// Mettre le socket en mode écoute
	if (listen(_listen_fd, 10) < 0)
	{
		perror("listen");
		close(_listen_fd);
		return false;
	}

	// Ajout du socket d'écoute dans la map
	pollfd listen_pfd;
	listen_pfd.fd = _listen_fd;
	listen_pfd.events = POLLIN;
	Client listenClient(listen_pfd, "listen");
	_clients.insert(std::make_pair(_listen_fd, listenClient));

	log::log::write(log::log::INFO, "Le serveur a démarré sur le port " + log::toString(_port));
	return true;
}

/**
 * @brief Mettre un descripteur de fichier en mode non bloquant
 * @param fd : le descripteur de fichier
 * @return true si l'opération a réussi, false sinon
 **/
bool Server::setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
		return false;
	return true;
}

/**
 * @brief Nouvelle connexion d'un client
 * Accepte la connexion et ajoute le client à la map
 **/
void Server::handleNewConnection()
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(_listen_fd, (sockaddr *)&client_addr, &client_len);
	if (client_fd < 0)
	{
		if (errno != EWOULDBLOCK)
			perror("accept");
		return;
	}
	if (!setNonBlocking(client_fd))
	{
		perror("setNonBlocking client");
		close(client_fd);
		return;
	}
	pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	Client client(client_pfd, inet_ntoa(client_addr.sin_addr));
	_clients.insert(std::make_pair(client_fd, client));
	_chatbot->addClient(&client);
	log::write(log::INFO, "Nouvelle connexion : fd(" + log::toString(client_fd) + ")");
}

/**
 * @brief Déconnexion d'un client
 * @param client : le client à déconnecter
 **/
void Server::DisconnectClient(Client *client)
{
	// parcourir les channels et supprimer le client
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.disconnectClientChannel(client, true))
		{
			it->second.broadcastMessage(LEAVE_CHANNEL(client->getUniqueName(), it->first, "Leaving"));
			if (it->second.getClientCount() == 0)
			{
				log::log::write(log::log::INFO, "Suppression du channel : " + it->first);
				_channels.erase(it);
			}
		}
	}
	close(client->getSocketFd());
	pollfd socketPfd = client->getSocketPfd();
	socketPfd.events = 0;
	socketPfd.revents = 0;
	socketPfd.fd = -1;
	client->setSocketPfd(socketPfd);
	client->setNickname("");
	client->setUserName("");
	client->setRealName("");
	client->setIp("");
	client->getBuffer().clear();
	client->setPasswordNotVerified();
	client->getSendBuffer().clear();
	_chatbot->deleteClient(client);
}

/**
 * @brief Déconnexion d'un client
 * @param client : le client à déconnecter
 * @param message : le message à envoyer au client avant de le déconnecter
 **/
void Server::DisconnectClient(Client *client, std::string message)
{
	if (message[0] == ':')
		message.erase(0, 1);
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second.disconnectClientChannel(client, true))
			it->second.broadcastMessage(LEAVE_CHANNEL(client->getUniqueName(), it->first, message));
	}
	close(client->getSocketFd());
	pollfd socketPfd = client->getSocketPfd();
	socketPfd.events = 0;
	socketPfd.revents = 0;
	socketPfd.fd = -1;
	client->setSocketPfd(socketPfd);
	client->setNickname("");
	client->setUserName("");
	client->setRealName("");
	client->setIp("");
	client->getBuffer().clear();
	client->setPasswordNotVerified();
	client->getSendBuffer().clear();
	_chatbot->deleteClient(client);
}

/**
 * @brief Traitement des données reçues d'un client
 * Les données reçues sont ajoutées au buffer du client
 * Tant qu'il y a une commande complète est présente dans le buffer (délimitée par '\n'),
 * elle est traitée par la classe Parsing
 * @param client_fd : le descripteur du client
 **/
void Server::handleClientData(int client_fd)
{
	// Lire les données reçues
	char tempBuffer[1024];
	int n = read(client_fd, tempBuffer, sizeof(tempBuffer) - 1);
	if (n <= 0)
	{
		if (n < 0)
			perror("read");
		this->DisconnectClient(&_clients[client_fd]);
		return;
	}
	tempBuffer[n] = '\0';

	// Ajouter les données reçues au buffer du client
	std::map<int, Client>::iterator it = _clients.find(client_fd);
	if (it == _clients.end())
	{
		std::cerr << "Erreur : client introuvable pour fd " << client_fd << std::endl;
		return;
	}
	Client &client = it->second;
	std::string &buffer = client.getBuffer();
	buffer.append(tempBuffer);

	// Tant qu'une commande complète (délimitée par '\n') est présente, on la traite
	std::string::size_type pos;
	log::write(log::RECEIVED, "fd(" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");
	while ((pos = buffer.find("\n")) != std::string::npos)
	{
		std::string command = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);
		Parsing parsing(*this);
		if (parsing.init_parsing(&client, command) == false)
		{
			// verifier que le client a bien été supprimé
			if (_clients.find(client_fd) != _clients.end())
				DisconnectClient(&client);
			return;
		}
	}
}

/**
 * @brief Envoi de données à un client
 * @param client_fd : le descripteur du client
 * @param data : les données à envoyer
 * @param server_name : true si le nom du serveur doit être ajouté devant les données
 * @param date : true si la date doit être ajoutée devant les données
 **/
void Server::send_data(int client_fd, std::string data, bool server_name, bool date)
{
	if (server_name)
		data.insert(0, std::string(SERVER_NAME));
	if (date)
	{
		std::string time = log::getTime("%Y-%m-%dT%H:%M:%S.000Z");
		data.insert(0, "@time=" + time + " ");
	}

	log::write(log::SENT, "fd(" + log::toString(client_fd) + ") : '" + data + "'");

	if (data.size() == 0 || data[data.size() - 1] != '\n')
		data += '\n';

	Client &client = _clients[client_fd];
	client.getSendBuffer() += data;

	std::string &sendBuffer = client.getSendBuffer();
	ssize_t bytes = write(client_fd, sendBuffer.c_str(), sendBuffer.size());

	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		else
		{
			perror("write");
			DisconnectClient(&client);
		}
	}
	else
		sendBuffer.erase(0, bytes);
}
