#include <string>
#include <fstream>
#include <time.h>
#include <thread>

class LogException : public std::exception {
public:
	const char* data;
	LogException(const char* message) : data(message) {}
	virtual const char* what() const throw()
	{
		return data;
	}
};

class Logger {
private:
	std::ofstream log;
	bool HasBeenOpened = 0;

	void WriteLog(std::string message, int severity) {
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

public:

	/**
	*
	* Create a log file with the current name.
	*
	* @param std::string name- name of the log file without the extension
	*
	*/
	Logger(std::string name = "log") {
		if (Logger::open(name)) throw LogException("Failed to create Log");
	}

	int open(std::string name = "log") {
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

	/**
	*
	* Write into the log file.
	*
	* @param std::string message - the message to write to the log.
	*
	* @param int severity - OPTIONAL, no input means just supply info. 1 is warn, 2 is error, 3 is severe.
	*
	*/
	void write(std::string message, int severity = 0) {
		std::thread(&Logger::WriteLog, this, message, severity).detach();
	}

	~Logger() {
		write("Deleting Logger object");
		try {
			log.close();
		}
		catch (...) {
			std::cout << "Exception caught." << std::endl;
		}
	}
};
