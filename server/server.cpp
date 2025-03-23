#include "server.hpp"
#include <arpa/inet.h>

Server::Server(int port, const std::string &password)
	: listen_fd_(-1), port_(port), password_(Hasher::hash(password))
{
	// initialiser la map des clients
	clients_.clear();
	channels_.clear();
}

Server::~Server()
{
	for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it)
		close(it->first);
}

void Server::run()
{
	while (true)
	{
		// Reconstruire le tableau de pollfd à partir de la map
		std::vector<pollfd> pollfds;
		for (std::map<int, Client>::iterator it = clients_.begin(); it != clients_.end(); ++it)
			pollfds.push_back(it->second.getSocketPfd());

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

/*
 * Initialisation du serveur
 * Création du socket d'écoute, configuration de l'adresse du serveur
 * et mise en mode non bloquant du socket d'écoute
 * @return true si l'initialisation a réussi, false sinon
*/
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

	// Permettre la réutilisation de l'adresse du serveur
	int opt = 1;
	if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt");
		close(listen_fd_);
		return false;
	}

	// Lier le socket d'écoute à l'adresse du serveur
	if (bind(listen_fd_, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("bind");
		close(listen_fd_);
		return false;
	}

	// Mettre le socket en mode écoute
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

	log::log::write(log::log::INFO, "Le serveur a démarré sur le port " + log::toString(port_));
	return true;
}

/*
 * Mettre un descripteur de fichier en mode non bloquant
 * @param fd : le descripteur de fichier
 * @return true si l'opération a réussi, false sinon
*/
bool Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return false;
	return true;
}

/*
 * Nouvelle connexion d'un client
 * Accepte la connexion et ajoute le client à la map
*/
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
	client.setIp(inet_ntoa(client_addr.sin_addr));
	clients_.insert(std::make_pair(client_fd, client));
	log::write(log::INFO, "Nouvelle connexion : fd(" + log::toString(client_fd) + ")");
}

/*
 * Déconnexion d'un client
 * @param client : le client à déconnecter
 */
void Server::DisconnectClient(Client &client)
{
	log::log::write(log::log::INFO, "Client déconnecté : fd(" + log::toString(client.getSocketFd()) + ")");
	close(client.getSocketFd());
	clients_.erase(client.getSocketFd());
}

/*
 * Traitement des données reçues d'un client
 * Les données reçues sont ajoutées au buffer du client
 * Si une commande complète est présente dans le buffer (délimitée par '\n'),
 * elle est traitée par la classe Parsing
 * @param client_fd : le descripteur du client
 */
void Server::handleClientData(int client_fd)
{
	// Lire les données reçues
	char tempBuffer[1024];
	int n = read(client_fd, tempBuffer, sizeof(tempBuffer) - 1);
	if (n <= 0)
	{
		if (n < 0)
			perror("read");
		this->DisconnectClient(clients_[client_fd]);
		return;
	}
	tempBuffer[n] = '\0';

	// Ajouter les données reçues au buffer du client
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
	while ((pos = buffer.find("\n")) != std::string::npos)
	{
		std::string command = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);
		Parsing parsing(*this);
		if (parsing.init_parsing(client, command) == false)
		{
			// verifier que le client a bien été supprimé
			if (clients_.find(client_fd) != clients_.end())
				DisconnectClient(client);
			return;
		}
	}
}

/*
 * Envoi de données à un client
 * @param client_fd : le descripteur du client
 * @param data : les données à envoyer
 * @param server_name : true si le nom du serveur doit être ajouté devant les données
 * @param date : true si la date doit être ajoutée devant les données
 */
void Server::send_data(int client_fd, std::string data, bool server_name, bool date)
{
	if (server_name)
		data.insert(0, std::string(SERVER_NAME));
	if (date)
	{
		std::string time = log::getTime("%Y-%m-%dT%H:%M:%S.000Z");
		data.insert(0, "@time=" + time + " ");
	}

	log::write(log::SENT, " fd(" + log::toString(client_fd) + ") : '" + data + "'");

	if (data.size() == 0 || data[data.size() - 1] != '\n')
		data += '\n';

	ssize_t bytes = write(client_fd, data.c_str(), data.size());
	if (bytes < 0)
		perror("write");
	else if (bytes < (ssize_t)data.size())
		std::cerr << "Erreur : données partiellement envoyées" << std::endl;
}
