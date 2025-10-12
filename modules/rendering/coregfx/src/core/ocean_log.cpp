/**
 * Copyright (C) by Patrice Chevillat
 */

#include <coregfx/core/ocean_log.hpp>
#include <filesystem>

namespace ocean
{
	// Static member definitions
	std::ofstream Logger::logFile;
	std::mutex Logger::logMutex;
	bool Logger::fileLoggingEnabled = false;

	bool Logger::initializeFileLogging(const std::string& filename) {
		std::lock_guard<std::mutex> lock(logMutex);

		try {
			logFile.open(filename, std::ios::out | std::ios::app);
			if (!logFile.is_open()) {
				return false;
			}

			fileLoggingEnabled = true;

			// Write session start marker
			logFile << "\n" << std::string(80, '=') << std::endl;
			logFile << "SESSION START: " << getCurrentTimestamp() << std::endl;
			logFile << std::string(80, '=') << std::endl;
			logFile.flush();
			return true;
		}
		catch (...) {
			fileLoggingEnabled = false;
			return false;
		}
	}

	void Logger::closeFileLogging() {
		std::lock_guard<std::mutex> lock(logMutex);

		if (logFile.is_open()) {
			logFile << std::string(80, '=') << std::endl;
			logFile << "SESSION END: " << getCurrentTimestamp() << std::endl;
			logFile << std::string(80, '=') << std::endl;
			logFile.close();
		}
		fileLoggingEnabled = false;
	}

	std::string Logger::getCurrentTimestamp() {
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			now.time_since_epoch()) % 1000;

		std::stringstream ss;
#ifdef _WIN32
		std::tm tm_buf;
		localtime_s(&tm_buf, &time_t);
		ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
#else
		ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
#endif
		ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
		return ss.str();
	}

	void Logger::writeToFile(const std::string& message, Severity severity) {
		if (!fileLoggingEnabled) {
			return;
		}

		std::lock_guard<std::mutex> lock(logMutex);

		if (logFile.is_open()) {
			std::string timestamp = getCurrentTimestamp();
			std::string severityStr = getSeverityString(severity);

			// Format: [TIMESTAMP] [SEVERITY] MESSAGE
			logFile << "[" << timestamp << "] [" << severityStr << "] " << message << std::endl;
			logFile.flush(); // Ensure immediate write
		}
	}
}