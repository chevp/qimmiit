/**
 * Copyright (C) by chevp
 */

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <string_view>
#include <sstream>
#include <vector>
#include <map>
#include <random>
#include <limits>
#include <stdexcept>
#include <algorithm>
#include <filesystem>

namespace util {

inline bool starts_with(const std::string_view str, const std::string_view prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool ends_with(const std::string_view str, const std::string_view suffix)
{
	return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline std::string load_file_content(const std::string_view& fullpath)
{
	std::ifstream myFile(std::string(fullpath), std::ios::in | std::ios::binary);

	if (!myFile) {
		// Handle error: file could not be opened
		throw std::runtime_error("Failed to open file: " + std::string(fullpath));
	}

	// Use std::ostringstream to read the file content
	std::ostringstream tmp;
	tmp << myFile.rdbuf();
	myFile.close();

	// Return the content as a std::string
	return tmp.str();
}

// Function to save a string to a file
inline bool save_string_to_file(const std::string& filename, const std::string& data) {
	std::ofstream outFile(filename, std::ios::binary); // Open file in binary mode
	if (!outFile) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return false;
	}

	outFile.write(data.data(), data.size()); // Write the string data to the file
	if (!outFile) {
		std::cerr << "Error writing to file: " << filename << std::endl;
		return false;
	}

	outFile.close(); // Close the file
	return true;
}

inline std::string load_file_content_with_stringstream(const std::string_view& filename) {
	std::ifstream file(std::string(filename), std::ios::in | std::ios::binary);
	if (!file) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return buffer.str();
}

inline uint32_t to_uint32(std::string value) {
	return static_cast<uint32_t>(std::stoul(value));
}

inline bool is_uint32(const std::string& s) {
	if (s.empty()) return false;
	// jeder Character muss eine Ziffer sein
	for (char c : s) {
		if (!std::isdigit(static_cast<unsigned char>(c)))
			return false;
	}
	// optional: Bereich pr�fen, damit es in uint32_t passt
	if (s.length() > 10) // 2^32-1 hat 10 Stellen
		return false;
	// kurze stoul-Pr�fung ohne Ausnahmebehandlung
	unsigned long v = std::stoul(s);
	return v <= std::numeric_limits<uint32_t>::max();
}

inline std::string load_file_content_with_ostringstream(const std::string_view& fullpath)
{
	std::ifstream myFile(std::string(fullpath), std::ios::in | std::ios::binary);

	if (!myFile) {
		// Handle error: file could not be opened
		throw std::runtime_error("Failed to open file: " + std::string(fullpath));
	}

	// Use std::ostringstream to read the file content
	std::ostringstream tmp;
	tmp << myFile.rdbuf();
	myFile.close();

	// Return the content as a std::string
	return tmp.str();
}

// Function to convert const char* to std::wstring (wide string)
inline std::wstring to_wide_string(const char* ansiString) {
	std::wstring wideString = std::wstring(ansiString, ansiString + strlen(ansiString));
	return wideString;
}

inline uint32_t get_random_uint32(uint32_t min = 0,
	uint32_t max = std::numeric_limits<uint32_t>::max()) {
	if (min > max) {
		throw std::invalid_argument("get_random_int: min must be <= max");
	}

	static std::random_device rd;
	static std::mt19937 gen(rd()); // 32-bit Mersenne Twister

	std::uniform_int_distribution<uint32_t> dist(min, max);
	return dist(gen);
}
inline bool isValidHttpUri(const std::string& uri)
{
	return uri.rfind("http://", 0) == 0 || uri.rfind("https://", 0) == 0;
}
inline bool isValidAssetUri(const std::string& uri) {
	return uri.rfind("asset://", 0) == 0;
}
inline bool isValidFileUri(const std::string& uri) {
	const std::string filePrefix = "file:///";

	if (uri.rfind(filePrefix, 0) != 0) {
		return false; // Does not start with file:///
	}

	// Extract path portion and convert to platform-specific format
	std::string path = uri.substr(filePrefix.length());

	// Convert forward slashes to backslashes (Windows specific)
#ifdef _WIN32
	std::replace(path.begin(), path.end(), '/', '\\');
#endif

	// Check if the path exists and is a regular file
	return std::filesystem::is_regular_file(std::filesystem::path(path));
}
}
