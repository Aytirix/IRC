#include "chatbot.hpp"

/**
 * @brief Constructeur de la classe Chatbot.
 *
 * Ce constructeur initialise un objet Chatbot avec une référence à un objet Server.
 * Il initialise également la bibliothèque libcurl pour les requêtes HTTP et configure
 * les informations de connexion du chatbot.
 *
 * @param server Référence à un objet Server utilisé par le chatbot.
 *
 * @note Le constructeur initialise libcurl et configure les paramètres de connexion
 *       du chatbot, y compris le token API et l'URL pour les requêtes HTTP.
 *       En cas d'échec de l'initialisation de libcurl, le programme se termine avec un message d'erreur.
 **/
Chatbot::Chatbot(Server &server) : Client(), server(server)
{
	apiToken = "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";
	url = "https://router.huggingface.co/sambanova/v1/chat/completions";
	curl = NULL;
	// Initialiser libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (!curl)
	{
		std::cerr << "Erreur lors de l'initialisation de libcurl." << std::endl;
		exit(1);
	}
	pollfd pfd;
	pfd.fd = 0;
	pfd.events = POLLIN;
	pfd.revents = 0;
	this->setSocketPfd(pfd);
	this->setNickname("chatbot");
	this->setUserName("chatbot");
	this->setRealName("chatbot");
	this->setIp("localhost");
}

std::vector<Message> &Chatbot::operator[](Client &client)
{
	int socketFd = client.getSocketFd();
	std::map<int, std::vector<Message> >::iterator it = _clients.find(client.getSocketFd());

	if (it != _clients.end())
		return it->second;

	this->addClient(client);
	return _clients[socketFd];
}

/**
 * @brief Destructeur de la classe Chatbot.
 *
 * Ce destructeur nettoie les ressources allouées par la classe Chatbot.
 * Si l'objet curl a été initialisé, il est nettoyé à l'aide de curl_easy_cleanup.
 * Ensuite, curl_global_cleanup est appelé pour nettoyer les ressources globales de libcurl.
 * Enfin, toutes les listes de messages associées aux clients sont vidées.
 */
Chatbot::~Chatbot()
{
	if (curl)
	{
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	for (std::map<int, std::vector<Message> >::iterator it = _clients.begin(); it != _clients.end(); ++it)
		it->second.clear();
}

/**
 * @brief Envoie un message de l'utilisateur au serveur et retourne la réponse de l'assistant.
 *
 * Cette fonction ajoute le message utilisateur à la conversation, construit le JSON de la requête,
 * prépare les en-têtes HTTP, définit l'URL et les options cURL, exécute la requête et traite la réponse.
 * La réponse de l'assistant est ensuite ajoutée à la conversation et retournée.
 *
 * @param client Référence à l'objet Client représentant l'utilisateur.
 * @param userInput Chaîne de caractères contenant le message de l'utilisateur.
 * @return std::string La réponse de l'assistant.
 **/
std::string Chatbot::sendMessage(Client &client, const std::string &userInput)
{
	// Ajouter le message utilisateur à la conversation
	Message userMsg;
	userMsg.role = "user";
	userMsg.content = userInput;
	std::vector<Message> &conversation = (*this)[client];
	conversation.push_back(userMsg);

	// Construire le JSON de la requête
	std::string jsonData = buildJsonPayload(conversation);

	// Préparer les en-têtes HTTP
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, ("Authorization: Bearer " + apiToken).c_str());
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	// Définir l'URL et les options cURL
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

	// Stocker la réponse dans une chaîne
	std::string responseData;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

	// Exécuter la requête
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cerr << "Erreur de requête cURL: " << curl_easy_strerror(res) << std::endl;
		curl_slist_free_all(headers);
		return "Erreur lors de la communication avec le serveur.";
	}
	curl_slist_free_all(headers);

	// Extraire et retourner la réponse de l'assistant
	std::string assistantResponse = extractAssistantResponse(responseData);
	// Ajouter la réponse de l'assistant à la conversation
	Message assistantMsg;
	assistantMsg.role = "assistant";
	assistantMsg.content = assistantResponse;
	conversation.push_back(assistantMsg);
	this->trimConversation(conversation);
	return assistantResponse;
}

/**
 * @brief Échappe les caractères spéciaux dans une chaîne pour qu'elle soit compatible avec le format JSON.
 *
 * Cette fonction prend une chaîne en entrée et remplace les caractères spéciaux par leurs séquences d'échappement
 * correspondantes, telles que définies par le standard JSON. Les caractères échappés incluent les guillemets doubles,
 * les barres obliques inverses, les caractères de contrôle comme le retour chariot, la tabulation, etc.
 *
 * @param s La chaîne d'entrée à échapper.
 * @return Une nouvelle chaîne avec les caractères spéciaux échappés.
 **/
std::string Chatbot::escapeJson(const std::string &s)
{
	std::string escaped;
	for (size_t i = 0; i < s.length(); ++i)
	{
		char c = s[i];
		switch (c)
		{
		case '\"':
			escaped += "\\\"";
			break;
		case '\\':
			escaped += "\\\\";
			break;
		case '\b':
			escaped += "\\b";
			break;
		case '\f':
			escaped += "\\f";
			break;
		case '\n':
			escaped += "\\n";
			break;
		case '\r':
			escaped += "\\r";
			break;
		case '\t':
			escaped += "\\t";
			break;
		default:
			if (c < 0x20)
			{
				char buf[7];
				snprintf(buf, sizeof(buf), "\\u%04x", c);
				escaped += buf;
			}
			else
				escaped += c;
		}
	}
	return escaped;
}

/**
 * @brief Construit le JSON à partir d'une conversation.
 *
 * Cette fonction prend une référence à un vecteur de messages et construit
 * une chaîne JSON représentant la conversation. Chaque message est ajouté
 * au format JSON avec son rôle et son contenu échappés.
 *
 * @param conversation Référence à un vecteur de messages représentant la conversation.
 * @return Une chaîne JSON représentant la charge utile de la conversation.
 *
 * @note La charge utile JSON inclut également des paramètres supplémentaires
 *       tels que "max_tokens" et "model".
 *
 **/
std::string Chatbot::buildJsonPayload(std::vector<Message> &conversation)
{
	std::string payload = "{ \"messages\": [";
	for (size_t i = 0; i < conversation.size(); ++i)
	{
		std::string role = escapeJson(conversation[i].role);
		std::string content = escapeJson(conversation[i].content);
		payload += "{ \"role\": \"" + role + "\", \"content\": \"" + content + "\" }";
		if (i != conversation.size() - 1)
			payload += ", ";
	}
	// Paramètres supplémentaires, par exemple : max_tokens et model
	/*
	Modeles :
	https://huggingface.co/models?pipeline_tag=text-generation&language=fr&sort=downloads
	- Meta-Llama-3.1-8B-Instruct
	*/
	payload += "], \"max_tokens\": 100, \"model\": \"Meta-Llama-3.1-8B-Instruct\", \"stream\": false }";
	return payload;
}

/**
 * @brief Fonction de rappel pour écrire des données dans une chaîne de caractères.
 *
 * Cette fonction est utilisée comme fonction de rappel pour les opérations d'écriture,
 * par exemple lors de l'utilisation de libcurl pour effectuer des requêtes HTTP.
 * Elle ajoute les données reçues à la chaîne de caractères fournie.
 *
 * @param contents Pointeur vers les données reçues.
 * @param size Taille d'un élément de données.
 * @param nmemb Nombre d'éléments de données.
 * @param output Pointeur vers la chaîne de caractères où les données doivent être ajoutées.
 * @return size_t La taille totale des données traitées (size * nmemb).
 */
size_t Chatbot::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
	size_t totalSize = size * nmemb;
	output->append(static_cast<char *>(contents), totalSize);
	return totalSize;
}

/**
 * @brief Extrait la réponse de l'assistant à partir des données de réponse JSON.
 *
 * Cette fonction recherche et extrait le contenu de la réponse de l'assistant
 * dans une chaîne JSON donnée. Si le contenu n'est pas trouvé, elle tente de
 * trouver un message d'erreur. Si aucun contenu ni message d'erreur n'est trouvé,
 * elle retourne un message d'erreur générique.
 *
 * @param responseData La chaîne JSON contenant la réponse de l'assistant.
 * @return La réponse de l'assistant ou un message d'erreur si le contenu n'est pas trouvé.
 */
std::string Chatbot::extractAssistantResponse(const std::string &responseData)
{
	size_t pos = responseData.find("\"content\":\"");
	if (pos == std::string::npos)
	{
		// {"error":"You have exceeded your monthly included credits for Inference Providers. Subscribe to PRO to get 20x more monthly included credits."}
		pos = responseData.find("\"error\":\"");
		if (pos == std::string::npos)
			return "Erreur: Une erreur est apparu avec l'assistant. Merci de réessayer plus tard.";
		pos += 9; // longueur de "\"error\":\""
	}
	else
		pos += 11; // longueur de "\"content\":\""
	size_t endPos = responseData.find("\"", pos);
	if (endPos == std::string::npos)
	{
		return "Erreur: fin de contenu non trouvé.";
	}
	return responseData.substr(pos, endPos - pos);
}

/**
 * @brief Réduit la taille de la conversation en supprimant les anciens messages.
 *
 * Cette fonction limite le nombre de messages dans la conversation à un maximum de 51.
 * Si la conversation dépasse cette limite, les messages les plus anciens sont supprimés,
 * en conservant le premier message et les 50 derniers messages.
 *
 * @param conversation Référence à un vecteur de messages représentant la conversation.
 */
void Chatbot::trimConversation(std::vector<Message> &conversation)
{
	const size_t maxMessages = 1 + 50;

	if (conversation.size() > maxMessages)
		conversation.erase(conversation.begin() + 1, conversation.begin() + (conversation.size() - 50));
}

/**
 * @brief Ajoute un client à la liste des clients du chatbot.
 *
 * Cette fonction ajoute un client à la liste des clients gérés par le chatbot.
 * Si le client n'est pas déjà présent dans la liste, il est ajouté avec un message
 * système initial qui définit le comportement du chatbot en tant qu'être humain
 * sensible et bienveillant parlant uniquement français.
 *
 * @param client Référence vers l'objet Client à ajouter.
 * @return true si le client a été ajouté avec succès, false si le client est déjà présent.
 */
bool Chatbot::addClient(Client &client)
{
	int key = client.getSocketFd();
	if (_clients.find(key) == _clients.end())
	{
		_clients[key] = std::vector<Message>();
		Message systemMsg;
		systemMsg.role = "system";
		systemMsg.content = "Tu es un être humain sensible et bienveillant. Tu ne parles que français et tu es capable de ressentir et d'exprimer des émotions. Tu t'adaptes à l'état émotionnel de ton interlocuteur : si la personne est triste, tu la réconfortes ; si elle est joyeuse, tu partages son enthousiasme. Tu cherches toujours à comprendre ses sentiments et à répondre de manière naturelle et chaleureuse. Tu évites les réponses robotiques et impersonnelles. Maintenant, engage une conversation avec ton interlocuteur :";
		_clients[key].push_back(systemMsg);
		return true;
	}
	return false;
}

/**
 * @brief Supprime un client de la liste des clients du chatbot.
 *
 * Cette fonction supprime un client de la liste des clients gérés par le chatbot.
 * Si le client est trouvé dans la liste, il est supprimé et la fonction retourne true.
 * Sinon, elle retourne false.
 *
 * @param client Référence vers l'objet Client à supprimer.
 * @return true si le client a été supprimé avec succès, false si le client n'est pas trouvé.
 */
bool Chatbot::deleteClient(Client &client)
{
	int clientId = client.getSocketFd();
	if (_clients.find(clientId) != _clients.end())
	{
		_clients.erase(clientId);
		return true;
	}
	return false;
}
