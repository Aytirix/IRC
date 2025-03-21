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
		ERROR,
		RECEIVED,
		SENT
	};

	static std::string toString(int value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	static std::string getTime(std::string format = "%Y-%m-%d %H:%M:%S")
	{
		std::time_t now = std::time(0);
		char *buf = new char[80];
		std::strftime(buf, 80, format.c_str(), std::localtime(&now));
		return std::string(buf);
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
		case RECEIVED:
			levelStr = "\033[35mRECEIVED";
			break;
		case SENT:
			levelStr = "\033[96mSENT";
			break;
		}

		std::string logEntry = "\033[32m[" + getTime() + "] " + levelStr + " : " + message + "\033[0m";

		std::cout << logEntry << std::endl;
	}
};

#endif