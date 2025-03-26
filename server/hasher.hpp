#ifndef HASHER_HPP
#define HASHER_HPP

#include <string>

/**
 * @class Hasher
 * @brief Une classe utilitaire pour hacher et comparer des chaînes de caractères.
 *
 * La classe Hasher fournit des méthodes statiques pour hacher des chaînes de caractères
 * en utilisant un algorithme de hachage personnalisé et pour comparer des valeurs hachées.
 */
class Hasher
{
public:
	/**
	 * @brief Hache une chaîne de caractères donnée.
	 *
	 * Cette méthode prend une chaîne de caractères en entrée et retourne sa valeur hachée
	 * en utilisant un algorithme de hachage personnalisé basé sur l'algorithme djb2.
	 *
	 * @param str La chaîne de caractères à hacher.
	 * @return La valeur hachée de la chaîne de caractères d'entrée.
	 */
	static unsigned long hash(const std::string &str)
	{
		unsigned long hash = 5381;
		for (size_t i = 0; i < str.length(); ++i)
			hash = ((hash << 5) + hash) ^ (str[i] * 31);
		return hash ^ (hash >> 16);
	}

	/**
	 * @brief Compare un mot de passe en clair avec un mot de passe haché.
	 *
	 * Cette méthode hache le mot de passe en clair fourni en entrée et le compare
	 * avec le mot de passe haché fourni.
	 *
	 * @param password Le mot de passe en clair à hacher et à comparer.
	 * @param passwordserver Le mot de passe haché à comparer.
	 * @return True si le mot de passe haché correspond au mot de passe haché fourni, false sinon.
	 */
	static bool compare(const std::string &password, const unsigned long passwordserver)
	{
		return Hasher::hash(password) == passwordserver;
	}

	/**
	 * @brief Compare deux mots de passe hachés.
	 *
	 * Cette méthode compare deux valeurs de mot de passe hachées pour vérifier leur égalité.
	 *
	 * @param password Le premier mot de passe haché à comparer.
	 * @param passwordserver Le second mot de passe haché à comparer.
	 * @return True si les mots de passe hachés correspondent, false sinon.
	 */
	static bool compare(const unsigned long password, const unsigned long passwordserver)
	{
		return password == passwordserver;
	}
};

#endif // HASHER_HPP
