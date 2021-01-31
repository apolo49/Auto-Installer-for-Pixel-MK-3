#include "../headers/Logger.h"

void Logger::WriteLog(std::string message, int severity) {
	message.append("\n");
	struct tm* timeinfo;
	time_t rawTime = std::time(NULL);
	timeinfo = localtime(&rawTime);
	//I'm sure there's a better way of doing this but for now this will do
	//Get current date and time in format dd/mm/yyyy hh:mm
	std::string date = std::to_string(timeinfo->tm_mday).append("/" + std::to_string(timeinfo->tm_mon + 1)).append("/" + std::to_string(timeinfo->tm_year + 1900));
	date.append(" " + std::to_string(timeinfo->tm_hour) + ":" + std::to_string(timeinfo->tm_min) + ":" + std::to_string(timeinfo->tm_sec));
	std::string completeMessage = "";
	switch (severity) {
	case 0: completeMessage = date + std::string(" INFO: ").append(message); break;
	case 1: completeMessage = date + std::string(" WARN: ").append(message); break;
	case 2: completeMessage = date + std::string(" ERROR: ").append(message); break;
	case 3: completeMessage = date + std::string(" SEVERE: ").append(message); break;
	}
	log.write(completeMessage.c_str(), strlen(completeMessage.c_str()));
}

int Logger::open(std::string name) {
	if (!HasBeenOpened) {
		log.open(name.append(".log"), std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
		HasBeenOpened = 1;
	}
	else {
		log.open(name.append(".log"), std::ofstream::out | std::ofstream::app | std::ofstream::binary);
	}
	if (!log.good()) { std::cout << "Failed to open log" << std::endl; return 1; }
	else {
		log.exceptions(std::ifstream::badbit);
		return 0;
	}
}