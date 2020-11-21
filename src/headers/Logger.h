#include <string>
#include <fstream>

class Logger {
private:
	std::ofstream log;
public:

	/**
	*
	* Create a log file with the current name.
	*
	* @param std::string name- name of the log file without the extension
	*
	*/
	Logger(std::string name = "log") {
		Logger::open(name);
	}

	int open(std::string name = "log") {
		log.open(name.append(".log"), std::ofstream::out | std::ofstream::app);
		if (!log.good()) return 1;
		else return 0;
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
		switch (severity) {
		case 0: log << "INFO: " << message << std::endl; break;
		case 1: log << "WARN: " << message << std::endl; break;
		case 2: log << "ERROR: " << message << std::endl; break;
		case 3: log << "SEVERE: " << message << std::endl; break;
		}
	}

	~Logger() {
		write("Deleting Logger object");
		log.close();
	}
};