#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <ctime>
#include <iostream>
#include <sstream>
#include "Colors.hpp"

class log
{
public:
	enum Level
	{
		INFO,
		WARNING,
		ERROR,
		RECEIVED,
		SENT,
		DEBUG
	};

	static std::string toString(int value)
	{
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

	static int toInt(std::string value)
	{
		std::stringstream ss(value);
		int intValue;
		ss >> intValue;
		return intValue;
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
			levelStr = BLUE "INFO    ";
			break;
		case WARNING:
			levelStr = YELLOW "WARNING ";
			break;
		case ERROR:
			levelStr = RED "ERROR   ";
			break;
		case RECEIVED:
			levelStr = MAGENTA "RECEIVED";
			break;
		case SENT:
			levelStr = BRIGHT_CYAN "SENT    ";
			break;
		case DEBUG:
			levelStr = BRIGHT_YELLOW "DEBUG   ";
			break;
		}

		std::string logEntry = GREEN "[" + getTime() + "] " + levelStr + " : " + message + RESET;

		std::cout << logEntry << std::endl;
	}
};

#endif