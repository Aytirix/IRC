#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <sstream>

class log
{
public:
	enum Level
	{
		INFO,
		WARNING,
		ERROR
	};

	static std::string toString(int value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	static void write(Level level, const std::string &message)
	{
		std::string levelStr;
		switch (level)
		{
		case INFO:
			levelStr = "\033[34mINFO";
			break;
		case WARNING:
			levelStr = "\033[33mWARNING";
			break;
		case ERROR:
			levelStr = "\033[31mERROR";
			break;
		}

		std::time_t now = std::time(0);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		std::string logEntry = "\033[32m[" + std::string(buf) + "] " + levelStr + ": " + message + "\033[0m";

		std::cout << logEntry << std::endl;
	}
};

#endif