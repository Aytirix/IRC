#include "chatbot.hpp"

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

// Destructeur pour nettoyer libcurl
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

// Envoie un message utilisateur et retourne la réponse de l'assistant
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

// Construit le payload JSON à partir de l'historique
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
	payload += "], \"max_tokens\": 500, \"model\": \"Meta-Llama-3.1-8B-Instruct\", \"stream\": false }";
	return payload;
}

// Fonction de rappel pour écrire la réponse dans une chaîne
size_t Chatbot::WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
	size_t totalSize = size * nmemb;
	output->append(static_cast<char *>(contents), totalSize);
	return totalSize;
}

// Extrait le contenu de la réponse de l'assistant
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

void Chatbot::trimConversation(std::vector<Message> &conversation)
{
	const size_t maxMessages = 1 + 50;

	if (conversation.size() > maxMessages)
		conversation.erase(conversation.begin() + 1, conversation.begin() + (conversation.size() - 20));
}

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
