#include "server.hpp"
#include "../parsing/Parsing.hpp"
#include "../client/client.hpp"

Server::Server(int port, const std::string &password)
	: listen_fd_(-1), port_(port), password_(password)
{
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it)
		close(it->first);
}

bool Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;
	return true;
}

bool Server::init()
{
	// Création du socket d'écoute
	listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd_ < 0)
	{
		perror("socket");
		return false;
	}
	// Mettre le socket en mode non bloquant
	if (!setNonBlocking(listen_fd_))
	{
		perror("setNonBlocking");
		close(listen_fd_);
		return false;
	}
	// Configuration de l'adresse du serveur
	sockaddr_in serv_addr;
	std::memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_);

	int opt = 1;
	if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		close(listen_fd_);
		return false;
	}

	if (bind(listen_fd_, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		close(listen_fd_);
		return false;
	}

	if (listen(listen_fd_, 10) < 0)
	{
		perror("listen");
		close(listen_fd_);
		return false;
	}

	// Ajout du socket d'écoute dans la map
	pollfd listen_pfd;
	listen_pfd.fd = listen_fd_;
	listen_pfd.events = POLLIN;
	Client listenClient(listen_pfd);
	clients_.insert(std::make_pair(listen_fd_, listenClient));

	std::cout << "Serveur démarré sur le port " << port_ << std::endl;
	return true;
}

void Server::handleNewConnection()
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(listen_fd_, (sockaddr *)&client_addr, &client_len);
	if (client_fd < 0)
	{
		if (errno != EWOULDBLOCK)
			perror("accept");
		return;
	}
	// Configuration du socket client en mode non bloquant
	if (!setNonBlocking(client_fd))
	{
		perror("setNonBlocking client");
		close(client_fd);
		return;
	}
	pollfd client_pfd;
	client_pfd.fd = client_fd;
	client_pfd.events = POLLIN;
	Client client(client_pfd);
	clients_.insert(std::make_pair(client_fd, client));
	std::cout << "Nouvelle connexion : fd " << client_fd << std::endl;
}

void Server::handleClientData(int client_fd)
{
	char tempBuffer[2048];
	int n = read(client_fd, tempBuffer, sizeof(tempBuffer) - 1);
	if (n <= 0)
	{
		if (n == 0)
			std::cout << "Client déconnecté : fd " << client_fd << std::endl;
		else
			perror("read");
		close(client_fd);
		clients_.erase(client_fd);
		return;
	}
	tempBuffer[n] = '\0';

	std::map<int, Client>::iterator it = clients_.find(client_fd);
	if (it == clients_.end())
	{
		std::cerr << "Erreur : client introuvable pour fd " << client_fd << std::endl;
		return;
	}
	Client &client = it->second;
	std::string &buffer = client.getBuffer();
	
	buffer.append(tempBuffer);

	// Tant qu'une commande complète (délimitée par '\n') est présente, on la traite
	std::string::size_type pos;
	while ((pos = buffer.find('\n')) != std::string::npos)
	{
		// Extraction de la commande (on peut également gérer '\r' si nécessaire)
		std::string command = buffer.substr(0, pos);
		// Supprimer la commande traitée du buffer
		buffer.erase(0, pos + 1);

		std::cout << "Commande complète reçue du fd " << client_fd << " : '"
				  << command << "'" << std::endl;

		Parsing parsing;
		std::map<std::string, std::string> result = parsing.init_parsing(command);
	}
}

void Server::run()
{
	while (true)
	{
		// Reconstruire le tableau de pollfd à partir de la map
		std::vector<pollfd> pollfds;
		for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it)
		{
			pollfds.push_back(it->second.getSocketPfd());
		}
		int ret = poll(&pollfds[0], pollfds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}
		// Parcourir les descripteurs ayant généré un événement
		for (size_t i = 0; i < pollfds.size(); ++i)
		{
			if (pollfds[i].revents & POLLIN)
			{
				int fd = pollfds[i].fd;
				if (fd == listen_fd_)
					handleNewConnection();
				else
					handleClientData(fd);
			}
		}
	}
}
