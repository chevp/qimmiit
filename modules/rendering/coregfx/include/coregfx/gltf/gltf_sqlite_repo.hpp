#pragma once
/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef GLTF_SQLITE_REPO_HPP_
#define GLTF_SQLITE_REPO_HPP_

#include <nlohmann/json.hpp>
#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <tinygltf/tiny_gltf.h>

constexpr std::string_view BASE_DIR = "C:/workspaces/media-store/resources/vkpbr2";

    class GltfSqliteRepo {
    public:
        // Public method to access the singleton instance
        static GltfSqliteRepo& getInstance() {
            static GltfSqliteRepo instance; // Guaranteed to be destroyed and instantiated on first use.
            return instance;
        }

        // Delete copy constructor and assignment operator
        GltfSqliteRepo(const GltfSqliteRepo&) = delete;
        GltfSqliteRepo& operator=(const GltfSqliteRepo&) = delete;

        tinygltf::Model findById(uint32_t id) {
            // Check if the resource is already loaded
            auto it = resourceCache.find(id);
            if (it != resourceCache.end()) {
                return it->second;
            }

            if (id == 0) {
                std::cerr << "id is 0" << std::endl;
                return tinygltf::Model();
            }

            // Load the resource from database
            auto gltfModel = loadFromSqliteDatabase(id);

            // Cache the loaded resource
            resourceCache[id] = gltfModel;
            return gltfModel;
        }
        

    private:
        // Private constructor for singleton
        GltfSqliteRepo() = default;

        // Resource cache
        std::map<uint32_t, tinygltf::Model> resourceCache;

        // TinyGLTF
        tinygltf::TinyGLTF tinygltfLoader = tinygltf::TinyGLTF();

        // Function to load text (GLTF structure) from SQLite
        std::string loadText(sqlite3* db, const std::string& sqlQuery, int gltfId) {
            sqlite3_stmt* stmt;
            std::string textData;

            // Prepare the SQL statement
            if (sqlite3_prepare_v2(db, sqlQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                // Bind the gltf_id to the query
                sqlite3_bind_int(stmt, 1, gltfId);

                // Execute the query and fetch the results
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    textData = text ? std::string(text) : "";
                }
            }
            else {
                std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            }

            sqlite3_finalize(stmt);
            return textData;
        }

        tinygltf::Model loadFromSqliteDatabase(uint32_t id) {
            sqlite3* db;
            int rc = sqlite3_open("gltf_embedded.db", &db);

            if (rc) {
                std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
                return tinygltf::Model();
            }

            // Load GLTF structure (JSON)
            std::string gltfStructureQuery = "SELECT gltf_structure FROM GLTF WHERE id = ?;";
            std::string gltfStructure = loadText(db, gltfStructureQuery, id);

            tinygltf::Model gltfModel;
            auto tStart = std::chrono::high_resolution_clock::now();
            std::string error;
            std::string warning;

            bool loaded = tinygltfLoader.LoadASCIIFromString(&gltfModel,
                &error, &warning, gltfStructure.c_str(), gltfStructure.size(), std::string(BASE_DIR));

            if (!loaded)
            {
                std::cerr << "Could not load gltf: " << error << std::endl;
            }

            // Close the database
            sqlite3_close(db);

            return gltfModel;
        }
    };
#endif