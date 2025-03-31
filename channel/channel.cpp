#include "channel.hpp"
#include "../server/server.hpp"
#include "../server/responses.hpp"
#include "../log/log.hpp"

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
Channel::Channel(Server &server, std::string &name, Client &client) : _server(server), _name(name), _topic("")
{
	addClient(client, true);
	password = "";
	limit = 0;
	restrict_topic = false;
	join_only_invite = false;
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
void Channel::addClient(Client client, bool _operator)
{
	// Si le channel est sur invite seulement
	if (join_only_invite && _clients.size() > 0)
	{
		if (_clients.find(client.getSocketFd()) == _clients.end())
			return _server.send_data(client.getSocketFd(), ERR_INVITE_ONLY(client.getNickname(), _name));
	}

	// Si la limite est atteinte
	if (limit > 0 && this->getClientCount() >= limit)
	{
		if (_clients.find(client.getSocketFd()) == _clients.end())
			return _server.send_data(client.getSocketFd(), ERR_CHANNELISFULL(client.getNickname(), _name));
	}

	Client_channel client_it = _clients[client.getSocketFd()];
	if (_clients.find(client.getSocketFd()) == _clients.end())
	{
		_clients[client.getSocketFd()]._client = client;
		_clients[client.getSocketFd()]._operator = _operator;
	}
	_clients[client.getSocketFd()]._connected = true;
	_clients[client.getSocketFd()]._invited = false;
	this->broadcastMessage(client, USER_JOIN_CHANNEL(client.getUniqueName(), _name));
	_server.send_data(client.getSocketFd(), USER_JOIN_CHANNEL(client.getUniqueName(), _name), false, true);
	if (_topic.size())
		_server.send_data(client.getSocketFd(), INIT_TOPIC(client.getUniqueName(), _name, _topic));
}

/**
 * @brief Ajoute un opérateur au canal.
 *
 * Cette fonction ajoute un client en tant qu'opérateur du canal en
 * ajoutant le descripteur de socket du client à la liste des opérateurs.
 *
 * @param client Référence vers l'objet Client à ajouter en tant qu'opérateur.
 **/
void Channel::addOperator(Client_channel &client)
{
	client._operator = true;
}

/**
 * @brief Supprime un opérateur du canal.
 *
 * Cette fonction retire un client de la liste des opérateurs du canal
 * en utilisant le descripteur de socket du client.
 *
 * @param client Référence vers l'objet Client à retirer des opérateurs.
 **/
void Channel::removeOperator(Client_channel &client)
{
	client._operator = false;
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
bool Channel::disconnectClientChannel(Client &client)
{
	if (_clients.find(client.getSocketFd()) == _clients.end())
		return false;
	_clients[client.getSocketFd()]._connected = false;
	_clients[client.getSocketFd()]._operator = false;
	_clients[client.getSocketFd()]._invited = false;
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
	for (std::map<int, Client_channel>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second._operator)
			clients += "@" + it->second._client.getNickname() + " ";
		else
			clients += it->second._client.getNickname() + " ";
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
	for (std::map<int, Client_channel>::iterator it = _clients.begin(); it != _clients.end(); ++it)
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
void Channel::broadcastMessage(Client &client, std::string message)
{
	for (std::map<int, Client_channel>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != client.getSocketFd())
			_server.send_data(it->first, message, false, true);
	}
}

Client_channel Channel::getClientByNickname(std::string &nickname)
{
	for (std::map<int, Client_channel>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second._client.getNickname() == nickname)
			return it->second;
	}
	throw std::runtime_error("Client not found");
}

void Channel::kickClient(Client &client, std::string &client_kick, std::string &message)
{
	if (_clients.find(client.getSocketFd()) == _clients.end())
		return _server.send_data(client.getSocketFd(), USER_NOT_IN_CHANNEL(client.getNickname(), _name));
	if (_clients[client.getSocketFd()]._operator == false)
		return _server.send_data(client.getSocketFd(), NOT_OPERATOR(client.getUniqueName(), _name));
	Client_channel kick;
	try
	{
		kick = this->getClientByNickname(client_kick);
		if (kick._connected == false)
			return _server.send_data(client.getSocketFd(), ERR_NOSUCH_NICK(client.getUniqueName(), client_kick, _name));
	}
	catch (std::exception &e)
	{
		return _server.send_data(client.getSocketFd(), ERR_NOSUCH_NICK(client.getUniqueName(), client_kick, _name));
	}
	kick._connected = false;
	this->broadcastMessage(KICK(client.getUniqueName(), _name, kick._client.getNickname(), message));
}

/**
 * @brief Change le sujet du canal.
 *
 * Cette fonction change le sujet du canal et diffuse un message à tous les clients
 * pour les informer du changement de sujet.
 *
 * @param client Le client qui change le sujet.
 * @param topic Le nouveau sujet du canal.
 **/
void Channel::setTopic(Client &client, std::string &topic)
{
	if (_clients.find(client.getSocketFd()) == _clients.end())
		return _server.send_data(client.getSocketFd(), USER_NOT_IN_CHANNEL(client.getNickname(), _name));
	if (_clients[client.getSocketFd()]._operator == false)
		return _server.send_data(client.getSocketFd(), NOT_OPERATOR(client.getUniqueName(), _name));
	this->_topic = topic;
	this->broadcastMessage(SET_TOPIC(client.getUniqueName(), _name, topic));
}


/**
 * @brief Récupère le nombre de clients connectés au canal.
**/
int Channel::getClientCount()
{
	int count = 0;
	for (std::map<int, Client_channel>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second._connected)
			count++;
	}
	return count;
}


/** 
 * @brief Envoie une invitation à un client pour rejoindre le canal.
 *  
 * Cette fonction envoie une invitation à un client cible pour rejoindre le canal.
 * Si le client qui envoie l'invitation n'est pas dans le canal, ou s'il n'est pas
 * un opérateur, ou si le client cible est déjà dans le canal, un message d'erreur
 * est envoyé. Sinon, un message d'invitation est envoyé au client cible.
 *
 * @param client Le client qui envoie l'invitation.
 * @param target Le client cible qui reçoit l'invitation.
**/
void Channel::sendInvite(Client &client, Client &target)
{
	// Si le client n'est pas dans le channel
	Client_channel client_it = _clients[client.getSocketFd()];
	Client_channel target_it = _clients[target.getSocketFd()];
	if (client_it._connected == false)
		return _server.send_data(client.getSocketFd(), USER_NOT_IN_CHANNEL(client.getNickname(), _name));

	// Si le client n'est pas un opérateur
	if (client_it._operator == false)
		return _server.send_data(client.getSocketFd(), NOT_OPERATOR(client.getUniqueName(), _name));
	
	// Si le target est déjà dans le channel
	if (target_it._connected == true)
		return _server.send_data(client.getSocketFd(), ERR_USER_ON_CHANNEL(client.getNickname(), target.getNickname(), _name));
	
	_server.send_data(client.getSocketFd(), INVITE_CALLBACK(client.getNickname(), target.getNickname(), _name));
	_server.send_data(target.getSocketFd(), INVITE_TO_TARGET(client.getUniqueName(), target.getNickname(), _name), false);
}