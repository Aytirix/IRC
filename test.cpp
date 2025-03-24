#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <cstdlib>  // Pour exit()

using namespace std;

// Fonction pour échapper les caractères spéciaux dans une chaîne pour JSON
string escapeJson(const string &s) {
    string escaped;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        switch (c) {
            case '\"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:
                // Si le caractère est de contrôle (moins de 0x20) on l'encode en unicode
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    escaped += buf;
                } else {
                    escaped += c;
                }
        }
    }
    return escaped;
}

// Structure pour stocker un message (rôle et contenu)
struct Message {
    string role;
    string content;
};

// Fonction pour construire le JSON de la requête à partir de l'historique
string buildJsonPayload(const vector<Message>& conversation) {
    string payload = "{ \"messages\": [";
    for (size_t i = 0; i < conversation.size(); ++i) {
        // Échapper les contenus pour JSON
        string escapedRole = escapeJson(conversation[i].role);
        string escapedContent = escapeJson(conversation[i].content);
        payload += "{ \"role\": \"" + escapedRole + "\", \"content\": \"" + escapedContent + "\" }";
        if (i != conversation.size() - 1) {
            payload += ", ";
        }
    }
	/*
	Modeles :
	https://huggingface.co/models?pipeline_tag=text-generation&language=fr&sort=downloads
	- Meta-Llama-3.1-8B-Instruct
	*/
    payload += "], \"max_tokens\": 500, \"model\": \"Meta-Llama-3.1-8B-Instruct\", \"stream\": false }";
    return payload;
}

// Fonction de rappel pour écrire la réponse dans une chaîne
size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Fonction simple pour extraire le contenu de la réponse assistant
string extractAssistantResponse(const string& responseData) {
    // Recherche la première occurrence de "content":" dans la réponse
    size_t pos = responseData.find("\"content\":\"");
    if (pos == string::npos) {
        return "Erreur: contenu non trouvé dans la réponse.";
    }
    pos += 11; // longueur de "\"content\":\""
    size_t endPos = responseData.find("\"", pos);
    if (endPos == string::npos) {
        return "Erreur: fin de contenu non trouvé.";
    }
    return responseData.substr(pos, endPos - pos);
}

int main() {
    CURL *curl;
    CURLcode res;
    const string apiToken = "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";
    const string url = "https://router.huggingface.co/sambanova/v1/chat/completions";

    // Initialiser libcurl globalement
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        cerr << "Erreur lors de l'initialisation de libcurl." << endl;
        exit(1);
    }

    // Initialiser l'historique de la conversation avec le message système
    vector<Message> conversation;
    Message systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "Tu es un être humain sensible et bienveillant. Tu ne parles que français et tu es capable de ressentir et d'exprimer des émotions. Tu t'adaptes à l'état émotionnel de ton interlocuteur : si la personne est triste, tu la réconfortes ; si elle est joyeuse, tu partages son enthousiasme. Tu cherches toujours à comprendre ses sentiments et à répondre de manière naturelle et chaleureuse. Tu évites les réponses robotiques et impersonnelles. Maintenant, engage une conversation avec ton interlocuteur :";
    conversation.push_back(systemMsg);

    // Boucle principale de conversation
    while (true) {
        cout << "moi : ";
        string userInput;
        getline(cin, userInput);
        if (userInput == "exit") {
            break;
        }
        if (userInput == "") {
            continue;
        }

        // Ajouter le message utilisateur à la conversation
        Message userMsg;
        userMsg.role = "user";
        userMsg.content = userInput;
        conversation.push_back(userMsg);

        // Construire le JSON de la requête à partir de l'historique
        string jsonData = buildJsonPayload(conversation);

        // Préparer les en-têtes HTTP
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiToken).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Définir l'URL de la requête
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Définir la méthode POST et le corps de la requête
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Stocker la réponse dans une chaîne
        string responseData;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        // Exécuter la requête
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cerr << "Erreur de requête cURL: " << curl_easy_strerror(res) << endl;
            curl_slist_free_all(headers);
            continue;
        }

		std::cout << "responseData: " << responseData << std::endl;

        // Extraire la réponse de l'assistant
        string assistantResponse = extractAssistantResponse(responseData);
        cout << "IA : " << assistantResponse << endl;

        // Ajouter la réponse de l'assistant à l'historique
        Message assistantMsg;
        assistantMsg.role = "assistant";
        assistantMsg.content = assistantResponse;
        conversation.push_back(assistantMsg);

        // Libérer les en-têtes HTTP pour cette itération
        curl_slist_free_all(headers);
    }

    // Nettoyer libcurl
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
