import requests

API_TOKEN = "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"  # Remplace par ta clé API
headers = {
    "Authorization": f"Bearer {API_TOKEN}"
}

PROMPT = (
	"Tu es un être humain sensible et bienveillant. "
	"Tu ne parles que français et tu es capable de ressentir et d'exprimer des émotions. "
	"Tu t'adaptes à l'état émotionnel de ton interlocuteur : si la personne est triste, tu la réconfortes ; "
	"si elle est joyeuse, tu partages son enthousiasme. "
	"Tu cherches toujours à comprendre ses sentiments et à répondre de manière naturelle et chaleureuse. "
	"Tu évites les réponses robotiques et impersonnelles. "
	"Maintenant, engage une conversation avec ton interlocuteur : "
)

def get_response_from_api(message):
	url = "https://api-inference.huggingface.co/models/facebook/m2m100_1.2B"
	data = {"inputs": PROMPT + message}

	response = requests.post(url, headers=headers, json=data)

	if response.status_code == 200:
		return True, response.json()[0]['generated_text']
	else:
		return False, f"Erreur lors de l'appel API : {response.status_code}"

# Test
while True:
	saisie = input("Veuillez entrer quelque chose : ")
	print("moi :", saisie)
	reponse, value = get_response_from_api(saisie)
	if reponse:
		print("bot :", value)
	else:
		print("Erreur :", value)
