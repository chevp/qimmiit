/*
 * Vulkan utilities
 *
 * This code is licensed under the MIT license(MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#ifndef OCEAN_UTIL_HPP_
#define OCEAN_UTIL_HPP_

#ifndef OCEAN_DEPENDENCIES_HPP_
#include "ocean_dependencies.hpp"
#endif

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#elif defined(__linux__)
#include <dirent.h>
#endif

#include <codecvt>
#include <locale>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstdlib>

namespace oceanutil
{
	void readDirectory(const std::string_view& directory, const std::string_view& pattern, std::map<std::string, std::string>& filelist, bool recursive);

	VkPipelineShaderStageCreateInfo loadShader(VkDevice device, const std::string_view filename, VkShaderStageFlagBits stage);

	inline std::wstring to_wstring(const std::string_view& string)
	{
		if (string.empty())
		{
			return L"";
		}

		const auto size_needed = MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), nullptr, 0);
		if (size_needed <= 0)
		{
			throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
		}

		std::wstring result(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &string.at(0), (int)string.size(), &result.at(0), size_needed);
		return result;
	}

	inline std::string to_string(const std::wstring& wide_string)
	{
		if (wide_string.empty())
		{
			return "";
		}

		const auto size_needed = WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(),
			nullptr, 0, nullptr, nullptr);
		if (size_needed <= 0)
		{
			throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
		}

		std::string result(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wide_string.at(0), (int)wide_string.size(), &result.at(0), size_needed, nullptr, nullptr);
		return result;
	}
}
#endif