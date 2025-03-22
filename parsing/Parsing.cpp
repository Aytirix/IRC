#include "Parsing.hpp"
#include <string>

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}

/*
** split
** Split une string en fonction d'un délimiteur
** @param str : la string à split
** @param delim : le délimiteur
** @return un vector contenant les morceaux de la string
*/
std::vector<std::string> Parsing::split(const std::string &str, char delim)
{
	std::vector<std::string> elems;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim))
	{
		if (item.size() > 0)
			elems.push_back(item);
	}
	return elems;
}

/*
** RemoveHiddenChar
** Supprime les caractères cachés '\r' et '\n' d'une string
** @param str : la string à nettoyer
** @return la string nettoyée
*/
std::string Parsing::RemoveHiddenChar(std::string &str)
{
	if (str.find("\r") != std::string::npos)
		str.erase(str.find("\r"), 1);
	if (str.find("\n") != std::string::npos)
		str.erase(str.find("\n"), 1);
	return str;
}

/*
** init_parsing
** Initialise le parsing des commandes
** @param client : le client qui a envoyé la commande
** @param buffer : la commande à parser
*/
bool Parsing::init_parsing(Client &client, std::string &buffer)
{
	buffer = RemoveHiddenChar(buffer);
	log::write(log::RECEIVED, "fd (" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");
	return true;
}
