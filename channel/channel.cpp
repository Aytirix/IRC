#include "channel.hpp"
#include <algorithm>

/**
 * @brief Constructeur de la classe Channel.
 *
 * Ce constructeur initialise un nouvel objet Channel avec le serveur donné.
 * Le nom du canal et le client initial. Le client est ajouté à la liste des
 * opérateurs et à la liste des clients du canal.
 *
 * @param server Référence vers l'objet Server associé à ce canal.
 * @param name Référence vers une chaîne de caractères représentant le nom du canal.
 * @param client Référence vers l'objet Client initial qui crée le canal.
 **/
Channel::Channel(Server &server, std::string &name, Client &client) : _server(server), _name(name)
{
	_operators.push_back(client.getSocketFd());
	addClient(client);
}

Channel::~Channel() {}

/**
 * @brief Ajoute un client au canal.
 *
 * Cette fonction ajoute un client à la liste des clients du canal et diffuse un message
 * indiquant que le client a rejoint le canal.
 *
 * @param client L'objet Client représentant le client à ajouter.
 **/
void Channel::addClient(Client client)
{

	_clients[client.getSocketFd()] = client;
	this->broadcastMessage(client, USER_JOIN_CHANNEL(client.getUniqueName(), _name));
}

/**
 * @brief Ajoute un opérateur au canal.
 *
 * Cette fonction ajoute un client en tant qu'opérateur du canal en
 * ajoutant le descripteur de socket du client à la liste des opérateurs.
 *
 * @param client Référence vers l'objet Client à ajouter en tant qu'opérateur.
 **/
void Channel::addOperator(Client &client)
{
	_operators.push_back(client.getSocketFd());
}

/**
 * @brief Supprime un opérateur du canal.
 *
 * Cette fonction retire un client de la liste des opérateurs du canal
 * en utilisant le descripteur de socket du client.
 *
 * @param client Référence vers l'objet Client à retirer des opérateurs.
 **/
void Channel::removeOperator(Client &client)
{
	_operators.remove(client.getSocketFd());
}

/**
 * @brief Supprime un client du canal.
 *
 * Cette fonction tente de supprimer un client spécifique du canal en utilisant
 * son descripteur de socket. Si le client n'est pas trouvé dans la liste des
 * clients, la fonction retourne false. Sinon, elle supprime le client et retourne true.
 *
 * @param client L'objet Client à supprimer du canal.
 * @return true si le client a été supprimé avec succès, false sinon.
 **/
bool Channel::removeClient(Client client)
{
	if (_clients.find(client.getSocketFd()) == _clients.end())
		return false;
	_clients.erase(client.getSocketFd());
	return true;
}

/**
 * @brief Récupère une chaîne de caractères contenant les pseudos de tous les clients du canal.
 *
 * Cette fonction parcourt tous les clients du canal et construit une chaîne de caractères
 * contenant les pseudos de chaque client. Si un client est un opérateur, son pseudo est
 * précédé d'un '@'.
 *
 * @return std::string Une chaîne de caractères contenant les pseudos de tous les clients,
 * séparés par des espaces. Les pseudos des opérateurs sont précédés d'un '@'.
 **/
std::string Channel::getAllClientsString()
{
	std::string clients;
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (std::find(_operators.begin(), _operators.end(), it->first) != _operators.end())
			clients += "@" + it->second.getNickname() + " ";
		else
			clients += it->second.getNickname() + " ";
	}
	clients.substr(0, clients.size() - 1);
	return clients;
}

/**
 * @brief Diffuse un message à tous les clients connectés au canal.
 *
 * Cette fonction envoie le message spécifié à tous les clients actuellement
 * connectés au canal.
 *
 * @param message Le message à diffuser à tous les clients.
 **/
void Channel::broadcastMessage(std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		_server.send_data(it->first, message, false, true);
}

/**
 * @brief Diffuse un message à tous les clients du canal sauf l'expéditeur.
 *
 * Cette fonction envoie un message à tous les clients connectés au canal,
 * à l'exception du client qui a envoyé le message.
 *
 * @param client Le client qui envoie le message.
 * @param message Le message à diffuser.
 **/
void Channel::broadcastMessage(Client client, std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != client.getSocketFd())
			_server.send_data(it->first, message, false, true);
	}
}
