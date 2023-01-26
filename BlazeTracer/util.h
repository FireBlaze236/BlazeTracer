#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <unordered_map>
#include <ctime>

enum LOG_TYPE {
	WARNING, 
	ERROR,
	INFO
};

std::unordered_map<LOG_TYPE, std::string> logHeaders = {
	{LOG_TYPE::WARNING, "[WARNING]"},
	{LOG_TYPE::ERROR, "[ERROR]"},
	{LOG_TYPE::INFO, "[INFO]"},
};

void LOG(LOG_TYPE type, const std::string& message) {
	std::time_t time_now = std::time(nullptr);
	
	
	std::cout << "(" << time_now << ")"<< logHeaders[type] << " : " << message << std::endl;
}

#endif