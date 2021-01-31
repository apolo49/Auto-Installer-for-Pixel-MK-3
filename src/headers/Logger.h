#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
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
	void WriteLog(std::string message, int severity);

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

	/**
	* Open a log file with the supplied name
	*/
	int open(std::string name = "log");

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
