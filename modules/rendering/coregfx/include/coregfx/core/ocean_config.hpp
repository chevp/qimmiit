#pragma once

#ifndef OCEAN_CONFIG_HPP_
#define OCEAN_CONFIG_HPP_

// Es ist möglich, sowohl das POC vom Server zu laden,
// als auch die POC-Datei. Z.B. um eine Sequenz aus dem
// POC zu laden!
// false Datei sowieso vom Server geladen wird, hat dieser Parameter kein Einfluss!
constexpr std::string_view DEFAULT_POC_FILEPATH = "C:/projects/structure8/3.6.0/ocean.sdk.renderer/resources/poc/Scene8-Sand.json.poc";
//constexpr std::string_view DEFAULT_POC_FILEPATH = "C:/projects/structure8/3.6.0/ocean.sdk.renderer/resources/bin/Box.bin.poc";

// To Do: Delete all
constexpr uint32_t APP_ID = 2260415837;
constexpr uint32_t VERSION_ID = 3752091614; // Bei jeder Versions-�nderung evtl eine andere Nummer
constexpr std::string_view VERSION = "0.1.39";
constexpr std::string_view MAIN_SERVER_ADDRESS = "localhost:63819";           // island-server
constexpr uint32_t TAKUU_PORT = 69030;                                     // takuuServer immer localhost
constexpr std::string_view USER_UUID = "7ea9852e-e6cd-4a2a-aec8-69e55f5eac4c";  // ohne Login
constexpr std::string_view USER_ID = "25a2aaf35058b63992ce78d1";  // ohne Login
constexpr std::string_view ROUTE = "monster-map"; // innerhalb des entsprechenden spezialisierten Servers
constexpr std::string_view STUB_ID = "65a2aaf35058b63992ce78d1";
constexpr std::string_view CMD_CLOSE = "close";
constexpr std::string_view CMD_PRINT_ALL = "printAll";
constexpr std::string_view CMD_CLEAR = "clear";

constexpr std::string_view USER_NAME = "otto";
constexpr int32_t DEFAULT_WINDOW_WIDTH = 320;
constexpr int32_t DEFAULT_WINDOW_HEIGHT = 180;

#include <coregfx/core/ConfigProvider.hpp>

// Use ConfigProvider for dynamic paths, fallback to these defaults if not configured
inline std::string getAssetPath() { return ConfigProvider::getAssetRoot() + "/"; }
inline std::string getShaderPath() { return ConfigProvider::getShaderDir() + "/"; }

constexpr std::string_view DEFAULT_GLTF_FILE = "models/DamagedHelmet/glTF/DamagedHelmet.gltf";
constexpr std::string_view ENVIRONMENT_MAP_FILE = "environments/papermill.ktx"; // "environments/gcanyon_cube.ktx";
constexpr std::string_view SKYBOX_FILE = "models/Box/glTF-Embedded/Box.gltf";
constexpr std::string_view TEXTURE_EMPTY_FILE = "textures/empty.ktx";
// constexpr std::string_view DEFAULT_GLTF_ERROR_PLACEHOLDER_FILENAME = "C:/3d-graphics/work/_playground2/placeholder/gltf/error_placeholder1x1x1.gltf";
// constexpr float CLEAR_COLOR[4] = { 0.05f, 0.05f, 0.05f, 1.0f };
constexpr float CLEAR_COLOR[4] = { 30.0f/255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f };

#endif