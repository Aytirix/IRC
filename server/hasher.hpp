#ifndef HASHER_HPP
#define HASHER_HPP

#include <string>

class Hasher
{
public:
	static unsigned long hash(const std::string &str)
	{
		unsigned long hash = 5381;
		for (size_t i = 0; i < str.length(); ++i)
			hash = ((hash << 5) + hash) ^ (str[i] * 31);
		return hash ^ (hash >> 16);
	}

	static bool compare(const std::string &password, const unsigned long passwordserver)
	{
		return Hasher::hash(password) == passwordserver;
	}
};

#endif
