#include "Parsing.hpp"
#include <unistd.h>

Parsing::Parsing() {}

Parsing::~Parsing() {}

void Parsing::init_parsing(Client client, std::string buffer)
{
	(void)buffer;
	// TYPE DE FORMAT :
	// Serveur démarré sur le port 6666
	// Nouvelle connexion : fd 4
	// cmd : fd client 4 : 'CAP LS'
	// cmd : fd client 4 : 'PASS test'
	// cmd : fd client 4 : 'NICK test2'
	// cmd : fd client 4 : 'USER thmouty thmouty localhost :Theo Mouty'

}