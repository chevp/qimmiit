/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef OCEAN_LOG_HPP_
#define OCEAN_LOG_HPP_

#include <chrono>
#include <string>
#include <windows.h>
#include <ctime>      // For time_t
#include <inttypes.h> // For PRId64
#include <fstream>
#include <mutex>
#include <sstream>
#include <iomanip>

namespace ocean
{
	enum class Severity
	{
		WARN,
		INFO,
		EXCEPTION
	};

	// Global logger state for file logging
	class Logger {
	private:
		static std::ofstream logFile;
		static std::mutex logMutex;
		static bool fileLoggingEnabled;

	public:
		static bool initializeFileLogging(const std::string& filename);
		static void closeFileLogging();
		static std::string getCurrentTimestamp();
		static void writeToFile(const std::string& message, Severity severity);
	};

	static void setupConsoleColor(Severity severity)
	{
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		// you can loop k higher to see more color choices
		/*for (int k = 1; k < 255; k++)
		{
			SetConsoleTextAttribute(hConsole, k);
			printf("%i Hello World\n", k);
		}*/
		int k = 15;
		switch (severity)
		{
		case Severity::EXCEPTION:
			k = 12;
			break;
		case Severity::WARN:
			k = 14;
			break;
		}
		SetConsoleTextAttribute(hConsole, k);
	}
	static void print(std::string comp, std::string msg, Severity severity)
	{
		// auto duration = std::chrono::duration<double, std::milli>(
		//	std::chrono::high_resolution_clock::now() - last).count();
		setupConsoleColor(severity);
		// printf("%-20s%-40s%-10i%\n", comp.c_str(), msg.c_str(), duration);
		printf("%-40s%-40s%-10i\n", comp.c_str(), msg.c_str(), 0);
		// last = std::chrono::high_resolution_clock::now();
	}
	static std::string getSeverityString(Severity s) {
		switch (s) {
		case Severity::INFO:
			return "Info";
		case Severity::WARN:
			return "Warn";
		case Severity::EXCEPTION:
		default:
			return "Exception";
		}
	}
	static void print(std::string msg, Severity severity)
	{
		setupConsoleColor(severity);

		// Get the current time point
		auto now = std::chrono::system_clock::now();

		// Convert to time_t for getting seconds
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);

		// Get the current time in milliseconds since the epoch
		auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

		// Format timestamp as human-readable: YYYY-MM-DD HH:MM:SS.mmm
		std::tm tm_buf;
#ifdef _WIN32
		localtime_s(&tm_buf, &now_time);
#else
		localtime_r(&now_time, &tm_buf);
#endif

		// Print message with human-readable timestamp
		printf("[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\n",
			tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
			tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
			static_cast<int>(now_ms.count()), msg.c_str());

		setupConsoleColor(Severity::INFO);

		// Also write to file if file logging is enabled
		Logger::writeToFile(msg, severity);
	}

	// static std::chrono::high_resolution_clock::time_point last;

	static void info(std::string comp, std::string msg)
	{
		print(comp, msg, Severity::INFO);
	}
	static void warn(std::string comp, std::string msg)
	{
		print(comp, msg, Severity::WARN);
	}
	static void error(std::string comp, std::string msg)
	{
		print(comp, msg, Severity::EXCEPTION);
	}
	static void info(std::string msg)
	{
		print(msg, Severity::INFO);
	}
	static void warn(std::string msg)
	{
		print(msg, Severity::WARN);
	}
	static void error(std::string msg)
	{
		print(msg, Severity::EXCEPTION);
	}
}
#endif