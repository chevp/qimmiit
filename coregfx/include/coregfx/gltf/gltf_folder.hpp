/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <string>
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <mutex>
#include <cassert>
#include <limits>   // For SIZE_MAX
#include <tinygltf/tiny_gltf.h>
#include <coregfx/util/thread_pool.hpp>
#include <fstream>
#include "../../../adapters/curl_stubs.h"
#include <coregfx/core/ocean_dependencies.hpp>
#include <coregfx/core/ocean_log.hpp>

namespace GltfEngine
{
	class TinyGltfFolder
	{
	private:
		tinygltf::TinyGLTF *tinyGltfLoader;

		std::vector<std::pair<uint32_t, tinygltf::Model>> gltfModelsInt;

		std::vector<std::pair<std::string, tinygltf::Model>> gltfModelsString;

	public:
		std::mutex m;

		TinyGltfFolder()
		{
			tinyGltfLoader = new tinygltf::TinyGLTF();
			
			gltfModelsInt = std::vector<std::pair<uint32_t, tinygltf::Model>>();
		}

		void clear()
		{
			ocean::info("GltfFolder", "Clearing GLTF models cache - " + std::to_string(gltfModelsInt.size()) + " int models, " + std::to_string(gltfModelsString.size()) + " string models");

			size_t totalMemory = (gltfModelsInt.size() + gltfModelsString.size()) * sizeof(tinygltf::Model);
			// // ocean::info("GltfFolder", "Freeing approximately " + std::to_string(totalMemory) + " of GLTF model memory");

			gltfModelsInt.resize(0);
			gltfModelsString.resize(0);

			ocean::info("GltfFolder", "GLTF models cache cleared successfully");
		}

		void loadGltfFromFile(uint32_t id, std::string_view filename)
		{
			ocean::info("GltfFolder", "Starting GLTF load - ID: " + std::to_string(id) + ", File: " + std::string(filename));
			
			auto loadStart = std::chrono::high_resolution_clock::now();
			
			auto model = folderBuildTinyModel(filename);
			
			auto loadEnd = std::chrono::high_resolution_clock::now();
			auto loadDuration = std::chrono::duration<double, std::milli>(loadEnd - loadStart).count();
			
			gltfModelsInt.push_back(std::pair<uint32_t, tinygltf::Model>(id, std::move(model)));
			
			// ocean::info("GltfFolder", "GLTF loaded");
			
			// ocean::info("GltfFolder", "Model info loaded");
		}

		void loadGltfFromFileWithMultithreading(std::vector<std::pair<std::uint32_t, std::string>> files)
		{
			// ocean::info("GltfFolder", "Starting multithreaded GLTF loading session");
			// ocean::info("GltfFolder", "Loading " + std::to_string(files.size()) + " GLTF files with multithreading");
			
			auto sessionStart = std::chrono::high_resolution_clock::now();
			
			// Log all files being loaded
			for (const auto& file : files) {
				// ocean::info("GltfLoader", "Queued for loading - ID: " + std::to_string(file.first) + ", File: " + file.second);
			}
			
			setupThreadsWithMultithreading(files);
			
			auto sessionEnd = std::chrono::high_resolution_clock::now();
			auto sessionDuration = std::chrono::duration<double, std::milli>(sessionEnd - sessionStart).count();
			
			// ocean::info("GltfFolder", "Multithreaded loading session completed");
		}

		tinygltf::Model* getGltfModelById(uint32_t id)
		{
			auto searchStart = std::chrono::high_resolution_clock::now();
			
			uint32_t a = getGltfModelIndexById(id);
			
			auto searchEnd = std::chrono::high_resolution_clock::now();
			auto searchDuration = std::chrono::duration<double, std::micro>(searchEnd - searchStart).count();

			if (a == SIZE_MAX) {
				ocean::warn("GltfFolder", "Model not found - ID: " + std::to_string(id) + " | Search took " + std::to_string(static_cast<int>(searchDuration)) + "us");
				return nullptr;
			}

			// ocean::info("GltfFolder", "Model retrieved - ID: " + std::to_string(id) + " at index " + std::to_string(a) + " | Search took " + std::to_string(static_cast<int>(searchDuration)) + "μs");
			return &gltfModelsInt[a].second;
		}

		std::pair<uint32_t, tinygltf::Model>* findGltfModelById(uint32_t id) {
			// Use std::find_if to find the element with the given id
			auto it = std::find_if(gltfModelsInt.begin(), gltfModelsInt.end(),
				[id](const std::pair<uint32_t, tinygltf::Model>& element) {
					// Compare the first part of the pair (the uint32_t id)
					return element.first == id;
				});

			// Check if an element was found
			if (it != gltfModelsInt.end()) {
				return &(*it);  // Return pointer to the found element
			}

			return nullptr;  // Return nullptr if no element was found
		}

		uint32_t getGltfModelIndexById(uint32_t id)
		{
			for (size_t i = 0; i < gltfModelsInt.size(); i++)
			{
				if (gltfModelsInt[i].first == id)
				{
					return i;
				}
			}

			return SIZE_MAX;
		}

		uint32_t getGltfModelIndexById(std::string_view id)
		{
			for (size_t i = 0; i < gltfModelsString.size(); i++)
			{
				if (gltfModelsString[i].first == id)
				{
					return i;
				}
			}

			return SIZE_MAX;
		}
		// Return true if the element was found, false otherwise
		bool existsByGltfId(uint32_t id) {	
			return findGltfModelById(id) != nullptr;
		}

		bool existsByGltfId(std::string_view gltfId)
		{
			return getGltfModelIndexById(gltfId) != SIZE_MAX;
		}
		void addModelByModelId(uint32_t modelIdDest, uint32_t modelIdSrc,
			uint32_t copyNodeIndexSrc, float px, float py, float pz,
			float rx, float ry, float rz)
		{
			auto src = &findGltfModelById(modelIdSrc)->second;
			auto dest = &findGltfModelById(modelIdDest)->second;

			// Copy all Buffers from SRC to DEST
			size_t bufferOffsetDest = dest->buffers.size();
			dest->buffers.insert(dest->buffers.end(), src->buffers.begin(), src->buffers.end());

			// Copy all nodes from SRC to DEST
			if (!src->nodes.empty()) {
				dest->nodes.push_back(src->nodes[0]);
			}

			tinygltf::Node n;
			n.translation = { px, py, pz };
			n.rotation = { 0.7071, 0, 0, 0.7071 };
			n.scale = { 4.757, 0.480, 4.798 };

			// n.mesh = gltfModels[modelIndexDest].second.nodes[copyNodeIndexSrc].mesh;
			n.mesh = 0;	 // src->nodes[copyNodeIndexSrc].mesh + meshOffsetDest;
			n.skin = -1; // gltfModels[modelIndexDest].second.nodes[copyNodeIndexSrc].skin;

			dest->nodes.push_back(n);

			n.name = "Plane";

			// gltfModelsInt[modelIndexDest].second.scenes[0].nodes.push_back(gltfModelsInt[modelIndexDest].second.nodes.size() - 1);
			dest->scenes[0].nodes.push_back(dest->nodes.size() - 1);
		}
		void addModelByModelId(std::string modelIdDest, std::string modelIdSrc,
			uint32_t copyNodeIndexSrc, float px, float py, float pz, float rx, float ry, float rz)
		{
			size_t modelIndexSrc = getGltfModelIndexById(modelIdSrc);
			size_t modelIndexDest = getGltfModelIndexById(modelIdDest);

			tinygltf::Model *src = &gltfModelsString[modelIndexSrc].second;
			tinygltf::Model *dest = &gltfModelsString[modelIndexDest].second;

			// Copy all Buffers from SRC to DEST
			size_t bufferOffsetDest = dest->buffers.size();
			dest->buffers.insert(dest->buffers.end(), src->buffers.begin(), src->buffers.end());

			// Copy all nodes from SRC to DEST
			if (!src->nodes.empty()) {
				dest->nodes.push_back(src->nodes[0]);
			}

			tinygltf::Node n;
			n.translation = { px, py, pz };
			n.rotation = { 0.7071, 0, 0, 0.7071 };
			n.scale = { 4.757, 0.480, 4.798 };

			// n.mesh = gltfModels[modelIndexDest].second.nodes[copyNodeIndexSrc].mesh;
			n.mesh = 0;	 // src->nodes[copyNodeIndexSrc].mesh + meshOffsetDest;
			n.skin = -1; // gltfModels[modelIndexDest].second.nodes[copyNodeIndexSrc].skin;

			dest->nodes.push_back(n);

			n.name = "Plane";

			gltfModelsString[modelIndexDest].second.scenes[0].nodes.push_back(gltfModelsString[modelIndexDest].second.nodes.size() - 1);
		}

		void addNodeByModelId(uint32_t modelId, uint32_t copyNodeIndex, float px, float py, float pz, float rx, float ry, float rz)
		{
			addNodeByModelIntIndex(getGltfModelIndexById(modelId), copyNodeIndex, px, py, pz, rx, ry, rz);
		}

		void addNodeByModelId(std::string modelId, uint32_t copyNodeIndex, float px, float py, float pz, float rx, float ry, float rz)
		{
			addNodeByModelStringIndex(getGltfModelIndexById(modelId), copyNodeIndex, px, py, pz, rx, ry, rz);
		}

		void addNodeByModelIntIndex(uint32_t modelIndex, uint32_t copyNodeIndex, float px, float py, float pz,
								 float rx, float ry, float rz)
		{
			assert(copyNodeIndex < gltfModelsInt[modelIndex].second.nodes.size());

			tinygltf::Node n = tinygltf::Node();

			n.translation.push_back(px);
			n.translation.push_back(py);
			n.translation.push_back(pz);

			// n.rotation.push_back(rx);
			// n.rotation.push_back(ry);
			// n.rotation.push_back(rz);

			n.rotation.push_back(0.7071);
			n.rotation.push_back(0);
			n.rotation.push_back(0);
			n.rotation.push_back(0.7071);

			n.mesh = gltfModelsInt[modelIndex].second.nodes[copyNodeIndex].mesh;
			n.skin = gltfModelsInt[modelIndex].second.nodes[copyNodeIndex].skin;

			gltfModelsInt[modelIndex].second.nodes.push_back(n);

			gltfModelsInt[modelIndex].second.scenes[0].nodes.push_back(gltfModelsInt[modelIndex].second.nodes.size() - 1);
		}
		void addNodeByModelStringIndex(uint32_t modelIndex, uint32_t copyNodeIndex, float px, float py, float pz,
			float rx, float ry, float rz)
		{
			assert(copyNodeIndex < gltfModelsString[modelIndex].second.nodes.size());

			tinygltf::Node n = tinygltf::Node();

			n.translation.push_back(px);
			n.translation.push_back(py);
			n.translation.push_back(pz);

			// n.rotation.push_back(rx);
			// n.rotation.push_back(ry);
			// n.rotation.push_back(rz);

			n.rotation.push_back(0.7071);
			n.rotation.push_back(0);
			n.rotation.push_back(0);
			n.rotation.push_back(0.7071);

			n.mesh = gltfModelsString[modelIndex].second.nodes[copyNodeIndex].mesh;
			n.skin = gltfModelsString[modelIndex].second.nodes[copyNodeIndex].skin;

			gltfModelsString[modelIndex].second.nodes.push_back(n);

			gltfModelsString[modelIndex].second.scenes[0].nodes.push_back(gltfModelsString[modelIndex].second.nodes.size() - 1);
		}
		// Schreibcallback: speichert geladene Daten in einen std::vector
		static size_t writeToBuffer(void* ptr, size_t size, size_t nmemb, void* userdata) {
			auto* buffer = reinterpret_cast<std::vector<unsigned char>*>(userdata);
			size_t totalSize = size * nmemb;
			buffer->insert(buffer->end(), (unsigned char*)ptr, (unsigned char*)ptr + totalSize);
			return totalSize;
		}

		bool downloadToMemory(const std::string& url, std::vector<unsigned char>& outBuffer) {
			// ocean::info("HttpDownload", "Starting download from: " + url);
			auto downloadStart = std::chrono::high_resolution_clock::now();
			
			CURL* curl = curl_easy_init();
			if (!curl) {
				ocean::error("HttpDownload", "Failed to initialize CURL for URL: " + url);
				return false;
			}

			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToBuffer);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outBuffer);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			
			// Add timeout settings to prevent hanging on unreachable servers
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);         // Total timeout: 5 seconds
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);  // Connection timeout: 3 seconds

			// ocean::info("HttpDownload", "CURL configured with 5s total timeout, 3s connection timeout");

			CURLcode res = curl_easy_perform(curl);
			
			auto downloadEnd = std::chrono::high_resolution_clock::now();
			auto downloadDuration = std::chrono::duration<double, std::milli>(downloadEnd - downloadStart).count();
			
			// Get response code and download info
			long response_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			
			double download_size = 0;
			curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &download_size);
			
			curl_easy_cleanup(curl);
			
			bool success = (res == CURLE_OK);
			if (success) {
				// ocean::info("HttpDownload", "Download successful");
			} else {
				ocean::error("HttpDownload", "Download failed from " + url + 
					" | CURL error: " + std::string(curl_easy_strerror(res)) + 
					" | Duration: " + std::to_string(downloadDuration) + 
					" | HTTP: " + std::to_string(response_code));
			}
			
			return success;
		}

		// Lädt und parst ein GLB-Modell aus dem RAM
		bool loadGlbFromHttpToModel(const std::string& url, tinygltf::Model& model) {
			std::vector<unsigned char> buffer;
			if (!downloadToMemory(url, buffer)) {
				std::cerr << "Download failed: " << url << "\n";
				return false;
			}

			tinygltf::TinyGLTF loader;
			std::string error, warning;

			bool result = loader.LoadBinaryFromMemory(
				&model, &error, &warning, buffer.data(), buffer.size(), /* base_dir */ ""
			);

			if (!warning.empty()) std::cerr << "GLTF warning: " << warning << "\n";
			if (!error.empty())   std::cerr << "GLTF error: " << error << "\n";

			return result;
		}
		bool shouldLoadFromHttp(const std::string& src) {
			return src.rfind("http://", 0) == 0 || src.rfind("https://", 0) == 0;
		}
		tinygltf::Model folderBuildTinyModel(std::string_view uri, float scale = 1.0f)
		{
			tinygltf::Model gltfModel;
			auto tStart = std::chrono::high_resolution_clock::now();
			std::string error;
			std::string warning;

			bool binary = false;
			size_t extpos = uri.rfind('.', uri.length());
			if (extpos != std::string::npos)
			{
				binary = (uri.substr(extpos + 1, uri.length() - extpos) == "glb");
			}

			bool fileLoaded = false;

			if (shouldLoadFromHttp(std::string(uri).c_str())) {

				ocean::info("Using libcurl version: (curl info disabled for compilation)");

				if (loadGlbFromHttpToModel(std::string(uri).c_str(), gltfModel)) {
					ocean::info("GLB successfully loaded from HTTP: " + std::string(uri));
					fileLoaded = true;
				}
				else {
					ocean::error("Failed to load GLB from HTTP: " + std::string(uri) + ". Server may be unreachable.");
					// Return empty model instead of hanging - this allows the program to continue
					return tinygltf::Model();
				}
			}
			else {
				fileLoaded = binary ? tinyGltfLoader->LoadBinaryFromFile(&gltfModel,
					&error, &warning, std::string(uri).c_str()) :
					tinyGltfLoader->LoadASCIIFromFile(&gltfModel, &error, &warning, std::string(uri).c_str());			
			}

			if (!fileLoaded)
			{
				m.lock();
				ocean::error("Could not load gltf uri: " + error);
				m.unlock();
			}
			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			m.lock();
			ocean::info("GltfFolder.loadingFromFile() Loaded file=" + std::string(uri) + " in " + std::to_string(tDiff) + "ms");
			m.unlock();
			return gltfModel;
		}

	private:
		// Function to read a file into a string
		std::string readFileToString(const std::string& filePath)
		{
			std::ifstream file(filePath, std::ios::binary);
			if (!file.is_open())
			{
				throw std::runtime_error("Could not open file: " + filePath);
			}

			// Move to end of file to get size
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			// Read the file into a string
			std::string fileContents(fileSize, '\0');
			file.read(&fileContents[0], fileSize);

			return fileContents;
		}

		// Function to load files into memory
		/*std::vector<std::pair<uint32_t, std::string>> loadFilesIntoMemory(const std::string& directoryPath)
		{
			std::vector<std::pair<uint32_t, std::string>> files;

			// Iterate over files in the directory
			uint32_t fileIndex = 0;
			for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
			{
				if (entry.is_regular_file())
				{
					try
					{
						std::string filePath = entry.path().string();
						std::string fileContent = readFileToString(filePath);

						// Add file index and content to the vector
						files.emplace_back(fileIndex++, fileContent);
						std::cout << "Loaded file: " << filePath << " (Size: " << fileContent.size() << " bytes)" << std::endl;
					}
					catch (const std::exception& e)
					{
						std::cerr << "Error reading file: " << e.what() << std::endl;
					}
				}
			}

			return files;
		}*/
		// eigentlich zum sehr viele Dateien hinzuzuf�gen!
		void buildTinyModelThreadExecution(uint32_t id, std::string filename)
		{
			auto a = folderBuildTinyModel(filename);

			m.lock();
			gltfModelsInt.push_back(std::make_pair(id, a));
			m.unlock();
			// std::cout << "GltfFolder.buildTinyModelThreadExecution() loaded id=" << id
			//	<< " filename = " << filename << std::endl;
		}

		void buildTinyModelThreadExecution(std::string id, std::string filename)
		{
			if (id == "")
				return;

			auto a = folderBuildTinyModel(filename);

			m.lock();
			gltfModelsString.push_back(std::make_pair(id, a));
			m.unlock();
			// std::cout << "GltfFolder.buildTinyModelThreadExecution() loaded id=" << id
			//	<< " filename = " << filename << std::endl;
		}

		/*void setupThreadsWithMultithreading(std::vector<std::pair<uint32_t, std::string>> files)
		{
			auto tStart = std::chrono::high_resolution_clock::now();

			oceanthreadpool::ThreadPool threadPool;

			auto maxThreads = std::thread::hardware_concurrency();

			if (maxThreads == 0)
				std::cerr << "maxThreads = 0" << std::endl;

			std::cout << "maxThreads = " << maxThreads << std::endl;

			// TODO: Replace with proper ThreadPool implementation
			// threadPool.setThreadCount(maxThreads);

			for (size_t count = 0; count < files.size(); ++count)
			{
				buildTinyModelThreadExecution(files.at(count).first, files.at(count).second);
			}

			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			std::cout << "GltfFolder.setupThreadsWithMultithreading() job is done in " << tDiff << "ms" << std::endl;
		}*/

		void setupThreadsWithMultithreading(std::vector<std::pair<uint32_t, std::string>> files)
		{
			// Start timer
			auto tStart = std::chrono::high_resolution_clock::now();
			
			// ocean::info("ThreadPool", "Initializing multithreaded GLTF loading");
			// ocean::info("ThreadPool", "Processing " + std::to_string(files.size()) + " files for multithreaded loading");

			// Get the number of hardware threads
			auto maxThreads = std::thread::hardware_concurrency();

			if (maxThreads == 0)
			{
				ocean::error("ThreadPool", "Hardware concurrency detection failed - falling back to single thread");
				maxThreads = 1;
			}

			// ocean::info("ThreadPool", "Hardware threads detected: " + std::to_string(maxThreads));

			// TODO: Implement proper ThreadPool integration
			// For now, using single-threaded approach to ensure compilation
			// ocean::info("ThreadPool", "Using single-threaded loading for " + std::to_string(files.size()) + " files");

			// Process files sequentially for now
			for (size_t i = 0; i < files.size(); ++i)
			{
				auto fileStart = std::chrono::high_resolution_clock::now();
				buildTinyModelThreadExecution(files[i].first, files[i].second);
				auto fileEnd = std::chrono::high_resolution_clock::now();
				auto fileDuration = std::chrono::duration<double, std::milli>(fileEnd - fileStart).count();
				// ocean::info("GLTF", "Completed file " + std::to_string(i) + " (ID:" + std::to_string(files[i].first) + ") in " + std::to_string(fileDuration) + "ms");
			}

			// End timer and log the execution time
			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

			double averagePerFile = files.size() > 0 ? tDiff / files.size() : 0.0;

			// ocean::info("GLTF", "Sequential loading completed in " + std::to_string(tDiff) + "ms" +
			//	" | Average per file: " + std::to_string(averagePerFile) + "ms");
		}
		/**
			Use a Hash Set: Instead of doing a linear search inside the inner loop, 
			you can use a hash set to store the first elements of gltfModelsString 
			and then check if the files.at(i).first exists in the set. 
			This will reduce the inner loop's time complexity from 
			𝑂(𝑚)O(m) to 𝑂(1)O(1), making the overall complexity 𝑂(𝑛)O(n).
		*/
		void removeGltfsIfAlreadyExists(std::vector<std::pair<std::uint32_t, std::string>>& files)
		{
			// Assuming `gltfModelsString` contains pairs with the first element as a string
			std::unordered_set<std::uint32_t> gltfSet;

			// Populate the set with the first elements of gltfModelsString
			for (const auto& model : gltfModelsInt) {
				gltfSet.insert(model.first);
			}

			// Iterate over files and modify the first element if it exists in the set
			for (auto& file : files) {
				if (gltfSet.find(file.first) != gltfSet.end()) {
					file.first = 0;  // Clear the string if found in gltfModelsString
				}
			}
		}
		/*void setupThreadsWithMultithreading(std::vector<std::pair<std::string, std::string>> files)
		{
			auto tStart = std::chrono::high_resolution_clock::now();

			removeGltfsIfAlreadyExists(files);

			threadpool::ThreadPool threadPool;

			auto maxThreads = std::thread::hardware_concurrency();

			if (maxThreads == 0)
				std::cerr << "maxThreads = 0" << std::endl;

			std::cout << "maxThreads = " << maxThreads << std::endl;

			// TODO: Replace with proper ThreadPool implementation
			// threadPool.setThreadCount(maxThreads);

			for (size_t count = 0; count < files.size(); ++count)
			{
				buildTinyModelThreadExecution(files.at(count).first, files.at(count).second);
			}

			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			std::cout << "GltfFolder.setupThreadsWithMultithreading() job is done in " << tDiff << "ms" << std::endl;
		}*/
	};
}