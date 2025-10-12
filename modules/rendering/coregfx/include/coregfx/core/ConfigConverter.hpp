/**
 * ConfigConverter.hpp - Bidirectional XML ↔ Protobuf Config Conversion
 *
 * CRITICAL: This is the SINGLE SOURCE OF TRUTH for config parsing
 * - Converts arctic.config.xml → cgfx::CoregfxConfig (protobuf)
 * - Converts cgfx::CoregfxConfig → arctic.config.xml (for export/validation)
 * - Validates config values and provides defaults
 *
 * Purpose: Eliminate config ambiguity by making protobuf the canonical format
 */

#pragma once

#include "cgfx.pb.h"
#include <tinyxml2.h>
#include <string>
#include <filesystem>
#include <stdexcept>

namespace coregfx {
namespace config {

/**
 * Load and parse arctic.config.xml into protobuf message
 *
 * @param xml_path Path to arctic.config.xml
 * @return Parsed CoregfxConfig protobuf message
 * @throws std::runtime_error if XML is invalid or file not found
 */
inline cgfx::CoregfxConfig loadFromXML(const std::string& xml_path) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xml_path.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to load config XML: " + xml_path);
    }

    cgfx::CoregfxConfig config;

    // Get root element
    tinyxml2::XMLElement* root = doc.FirstChildElement("arctic-config");
    if (!root) {
        throw std::runtime_error("Invalid config XML: missing <arctic-config> root");
    }

    // Parse <variables>
    if (auto* varsNode = root->FirstChildElement("variables")) {
        auto* vars = config.mutable_variables();

        for (auto* var = varsNode->FirstChildElement("variable"); var; var = var->NextSiblingElement("variable")) {
            const char* name = var->Attribute("name");
            const char* value = var->Attribute("value");
            if (name && value) {
                if (strcmp(name, "THREAD_COUNT") == 0) {
                    vars->set_thread_count(std::atoi(value));
                } else {
                    (*vars->mutable_custom_vars())[name] = value;
                }
            }
        }
    }

    // Parse <paths>
    if (auto* pathsNode = root->FirstChildElement("paths")) {
        auto* paths = config.mutable_paths();

        if (auto* elem = pathsNode->FirstChildElement("asset-root")) {
            if (elem->GetText()) paths->set_asset_root(elem->GetText());
        }
        if (auto* elem = pathsNode->FirstChildElement("shader-dir")) {
            if (elem->GetText()) paths->set_shader_dir(elem->GetText());
        }
        if (auto* elem = pathsNode->FirstChildElement("log-dir")) {
            if (elem->GetText()) paths->set_log_dir(elem->GetText());
        }
        if (auto* elem = pathsNode->FirstChildElement("temp-dir")) {
            if (elem->GetText()) paths->set_temp_dir(elem->GetText());
        }
        if (auto* elem = pathsNode->FirstChildElement("cache-dir")) {
            if (elem->GetText()) paths->set_cache_dir(elem->GetText());
        }
    }

    // Parse <scene>
    if (auto* sceneNode = root->FirstChildElement("scene")) {
        auto* scene = config.mutable_scene();

        if (auto* elem = sceneNode->FirstChildElement("default-input-file")) {
            if (elem->GetText()) scene->set_default_input_file(elem->GetText());
        }
        if (auto* elem = sceneNode->FirstChildElement("auto-load")) {
            if (elem->GetText()) {
                scene->set_auto_load(strcmp(elem->GetText(), "true") == 0);
            }
        }
    }

    // Parse <settings>
    if (auto* settingsNode = root->FirstChildElement("settings")) {
        auto* settings = config.mutable_settings();

        if (auto* elem = settingsNode->FirstChildElement("headless-mode")) {
            if (elem->GetText()) settings->set_headless_mode(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = settingsNode->FirstChildElement("studio-mode")) {
            if (elem->GetText()) settings->set_studio_mode(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = settingsNode->FirstChildElement("auto-close-enabled")) {
            if (elem->GetText()) settings->set_auto_close_enabled(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = settingsNode->FirstChildElement("auto-close-timeout")) {
            if (elem->GetText()) settings->set_auto_close_timeout(std::atoi(elem->GetText()));
        }
        if (auto* elem = settingsNode->FirstChildElement("target-fps")) {
            if (elem->GetText()) settings->set_target_fps(std::atoi(elem->GetText()));
        }
        if (auto* elem = settingsNode->FirstChildElement("vsync-enabled")) {
            if (elem->GetText()) settings->set_vsync_enabled(strcmp(elem->GetText(), "true") == 0);
        }
    }

    // Parse <debug>
    if (auto* debugNode = root->FirstChildElement("debug")) {
        auto* debug = config.mutable_debug();

        if (auto* elem = debugNode->FirstChildElement("log-level")) {
            if (elem->GetText()) {
                std::string level = elem->GetText();
                if (level == "TRACE") debug->set_log_level(cgfx::LOG_LEVEL_TRACE);
                else if (level == "DEBUG") debug->set_log_level(cgfx::LOG_LEVEL_DEBUG);
                else if (level == "INFO") debug->set_log_level(cgfx::LOG_LEVEL_INFO);
                else if (level == "WARN") debug->set_log_level(cgfx::LOG_LEVEL_WARN);
                else if (level == "ERROR") debug->set_log_level(cgfx::LOG_LEVEL_ERROR);
                else if (level == "FATAL") debug->set_log_level(cgfx::LOG_LEVEL_FATAL);
            }
        }
        if (auto* elem = debugNode->FirstChildElement("log-to-console")) {
            if (elem->GetText()) debug->set_log_to_console(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = debugNode->FirstChildElement("log-to-file")) {
            if (elem->GetText()) debug->set_log_to_file(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = debugNode->FirstChildElement("json-logging")) {
            if (elem->GetText()) debug->set_json_logging(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = debugNode->FirstChildElement("vulkan-validation")) {
            if (elem->GetText()) debug->set_vulkan_validation(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = debugNode->FirstChildElement("compact-logging")) {
            if (elem->GetText()) debug->set_compact_logging(strcmp(elem->GetText(), "true") == 0);
        }
    }

    // Parse <camera>
    if (auto* cameraNode = root->FirstChildElement("camera")) {
        auto* camera = config.mutable_camera();

        if (auto* elem = cameraNode->FirstChildElement("default-position")) {
            const char* value = elem->Attribute("value");
            if (value) {
                float x, y, z;
                if (sscanf(value, "%f, %f, %f", &x, &y, &z) == 3) {
                    auto* pos = camera->mutable_default_position();
                    pos->set_x(x);
                    pos->set_y(y);
                    pos->set_z(z);
                }
            }
        }
        if (auto* elem = cameraNode->FirstChildElement("default-rotation")) {
            const char* value = elem->Attribute("value");
            if (value) {
                float x, y, z;
                if (sscanf(value, "%f, %f, %f", &x, &y, &z) == 3) {
                    auto* rot = camera->mutable_default_rotation();
                    rot->set_x(x);
                    rot->set_y(y);
                    rot->set_z(z);
                }
            }
        }
        if (auto* elem = cameraNode->FirstChildElement("default-fov")) {
            if (elem->GetText()) camera->set_default_fov(std::atof(elem->GetText()));
        }
        if (auto* elem = cameraNode->FirstChildElement("near-plane")) {
            if (elem->GetText()) camera->set_near_plane(std::atof(elem->GetText()));
        }
        if (auto* elem = cameraNode->FirstChildElement("far-plane")) {
            if (elem->GetText()) camera->set_far_plane(std::atof(elem->GetText()));
        }
    }

    // Parse <http-server>
    if (auto* httpNode = root->FirstChildElement("http-server")) {
        auto* http = config.mutable_http_server();

        if (auto* elem = httpNode->FirstChildElement("enabled")) {
            if (elem->GetText()) http->set_enabled(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = httpNode->FirstChildElement("port")) {
            if (elem->GetText()) http->set_port(std::atoi(elem->GetText()));
        }
        if (auto* elem = httpNode->FirstChildElement("host")) {
            if (elem->GetText()) http->set_host(elem->GetText());
        }
        if (auto* elem = httpNode->FirstChildElement("max-connections")) {
            if (elem->GetText()) http->set_max_connections(std::atoi(elem->GetText()));
        }
    }

    // Parse <state-dump>
    if (auto* dumpNode = root->FirstChildElement("state-dump")) {
        auto* dump = config.mutable_state_dump();

        if (auto* elem = dumpNode->FirstChildElement("enabled")) {
            if (elem->GetText()) dump->set_enabled(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = dumpNode->FirstChildElement("default-filename")) {
            if (elem->GetText()) dump->set_default_filename(elem->GetText());
        }
        if (auto* elem = dumpNode->FirstChildElement("default-delay")) {
            if (elem->GetText()) dump->set_default_delay(std::atoi(elem->GetText()));
        }
        if (auto* elem = dumpNode->FirstChildElement("include-buffers")) {
            if (elem->GetText()) dump->set_include_buffers(strcmp(elem->GetText(), "true") == 0);
        }
        if (auto* elem = dumpNode->FirstChildElement("include-textures")) {
            if (elem->GetText()) dump->set_include_textures(strcmp(elem->GetText(), "true") == 0);
        }
    }

    return config;
}

/**
 * Export protobuf CoregfxConfig to XML format
 *
 * @param config Protobuf config message
 * @param xml_path Output path for XML file
 * @throws std::runtime_error if write fails
 */
inline void saveToXML(const cgfx::CoregfxConfig& config, const std::string& xml_path) {
    tinyxml2::XMLDocument doc;

    // XML declaration
    auto* decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    // Root element
    auto* root = doc.NewElement("arctic-config");
    root->SetAttribute("version", "1.0");
    doc.InsertEndChild(root);

    // Variables section
    if (config.has_variables()) {
        auto* varsNode = doc.NewElement("variables");
        root->InsertEndChild(varsNode);

        auto* threadVar = doc.NewElement("variable");
        threadVar->SetAttribute("name", "THREAD_COUNT");
        threadVar->SetAttribute("value", config.variables().thread_count());
        varsNode->InsertEndChild(threadVar);

        for (const auto& [key, value] : config.variables().custom_vars()) {
            auto* var = doc.NewElement("variable");
            var->SetAttribute("name", key.c_str());
            var->SetAttribute("value", value.c_str());
            varsNode->InsertEndChild(var);
        }
    }

    // Paths section
    if (config.has_paths()) {
        auto* pathsNode = doc.NewElement("paths");
        root->InsertEndChild(pathsNode);

        auto addPath = [&](const char* name, const std::string& value) {
            if (!value.empty()) {
                auto* elem = doc.NewElement(name);
                elem->SetText(value.c_str());
                pathsNode->InsertEndChild(elem);
            }
        };

        addPath("asset-root", config.paths().asset_root());
        addPath("shader-dir", config.paths().shader_dir());
        addPath("log-dir", config.paths().log_dir());
        addPath("temp-dir", config.paths().temp_dir());
        addPath("cache-dir", config.paths().cache_dir());
    }

    // Scene section
    if (config.has_scene()) {
        auto* sceneNode = doc.NewElement("scene");
        root->InsertEndChild(sceneNode);

        if (!config.scene().default_input_file().empty()) {
            auto* elem = doc.NewElement("default-input-file");
            elem->SetText(config.scene().default_input_file().c_str());
            sceneNode->InsertEndChild(elem);
        }

        auto* autoLoad = doc.NewElement("auto-load");
        autoLoad->SetText(config.scene().auto_load() ? "true" : "false");
        sceneNode->InsertEndChild(autoLoad);
    }

    // Settings section
    if (config.has_settings()) {
        auto* settingsNode = doc.NewElement("settings");
        root->InsertEndChild(settingsNode);

        auto addBool = [&](const char* name, bool value) {
            auto* elem = doc.NewElement(name);
            elem->SetText(value ? "true" : "false");
            settingsNode->InsertEndChild(elem);
        };

        auto addInt = [&](const char* name, int value) {
            auto* elem = doc.NewElement(name);
            elem->SetText(value);
            settingsNode->InsertEndChild(elem);
        };

        addBool("headless-mode", config.settings().headless_mode());
        addBool("studio-mode", config.settings().studio_mode());
        addBool("auto-close-enabled", config.settings().auto_close_enabled());
        addInt("auto-close-timeout", config.settings().auto_close_timeout());
        addInt("target-fps", config.settings().target_fps());
        addBool("vsync-enabled", config.settings().vsync_enabled());
    }

    // Debug section
    if (config.has_debug()) {
        auto* debugNode = doc.NewElement("debug");
        root->InsertEndChild(debugNode);

        auto* logLevel = doc.NewElement("log-level");
        switch (config.debug().log_level()) {
            case cgfx::LOG_LEVEL_TRACE: logLevel->SetText("TRACE"); break;
            case cgfx::LOG_LEVEL_DEBUG: logLevel->SetText("DEBUG"); break;
            case cgfx::LOG_LEVEL_INFO: logLevel->SetText("INFO"); break;
            case cgfx::LOG_LEVEL_WARN: logLevel->SetText("WARN"); break;
            case cgfx::LOG_LEVEL_ERROR: logLevel->SetText("ERROR"); break;
            case cgfx::LOG_LEVEL_FATAL: logLevel->SetText("FATAL"); break;
        }
        debugNode->InsertEndChild(logLevel);

        auto addBool = [&](const char* name, bool value) {
            auto* elem = doc.NewElement(name);
            elem->SetText(value ? "true" : "false");
            debugNode->InsertEndChild(elem);
        };

        addBool("log-to-console", config.debug().log_to_console());
        addBool("log-to-file", config.debug().log_to_file());
        addBool("json-logging", config.debug().json_logging());
        addBool("vulkan-validation", config.debug().vulkan_validation());
        addBool("compact-logging", config.debug().compact_logging());
    }

    // Camera section
    if (config.has_camera()) {
        auto* cameraNode = doc.NewElement("camera");
        root->InsertEndChild(cameraNode);

        if (config.camera().has_default_position()) {
            auto* pos = doc.NewElement("default-position");
            pos->SetAttribute("type", "vec3");
            char buf[128];
            snprintf(buf, sizeof(buf), "%.1f, %.1f, %.1f",
                config.camera().default_position().x(),
                config.camera().default_position().y(),
                config.camera().default_position().z());
            pos->SetAttribute("value", buf);
            cameraNode->InsertEndChild(pos);
        }

        if (config.camera().has_default_rotation()) {
            auto* rot = doc.NewElement("default-rotation");
            rot->SetAttribute("type", "vec3");
            char buf[128];
            snprintf(buf, sizeof(buf), "%.1f, %.1f, %.1f",
                config.camera().default_rotation().x(),
                config.camera().default_rotation().y(),
                config.camera().default_rotation().z());
            rot->SetAttribute("value", buf);
            cameraNode->InsertEndChild(rot);
        }

        auto addFloat = [&](const char* name, float value) {
            auto* elem = doc.NewElement(name);
            elem->SetText(value);
            cameraNode->InsertEndChild(elem);
        };

        addFloat("default-fov", config.camera().default_fov());
        addFloat("near-plane", config.camera().near_plane());
        addFloat("far-plane", config.camera().far_plane());
    }

    // HTTP Server section
    if (config.has_http_server()) {
        auto* httpNode = doc.NewElement("http-server");
        root->InsertEndChild(httpNode);

        auto* enabled = doc.NewElement("enabled");
        enabled->SetText(config.http_server().enabled() ? "true" : "false");
        httpNode->InsertEndChild(enabled);

        auto* port = doc.NewElement("port");
        port->SetText(config.http_server().port());
        httpNode->InsertEndChild(port);

        if (!config.http_server().host().empty()) {
            auto* host = doc.NewElement("host");
            host->SetText(config.http_server().host().c_str());
            httpNode->InsertEndChild(host);
        }

        auto* maxConn = doc.NewElement("max-connections");
        maxConn->SetText(config.http_server().max_connections());
        httpNode->InsertEndChild(maxConn);
    }

    // State Dump section
    if (config.has_state_dump()) {
        auto* dumpNode = doc.NewElement("state-dump");
        root->InsertEndChild(dumpNode);

        auto addBool = [&](const char* name, bool value) {
            auto* elem = doc.NewElement(name);
            elem->SetText(value ? "true" : "false");
            dumpNode->InsertEndChild(elem);
        };

        addBool("enabled", config.state_dump().enabled());

        if (!config.state_dump().default_filename().empty()) {
            auto* filename = doc.NewElement("default-filename");
            filename->SetText(config.state_dump().default_filename().c_str());
            dumpNode->InsertEndChild(filename);
        }

        auto* delay = doc.NewElement("default-delay");
        delay->SetText(config.state_dump().default_delay());
        dumpNode->InsertEndChild(delay);

        addBool("include-buffers", config.state_dump().include_buffers());
        addBool("include-textures", config.state_dump().include_textures());
    }

    // Save to file
    if (doc.SaveFile(xml_path.c_str()) != tinyxml2::XML_SUCCESS) {
        throw std::runtime_error("Failed to write config XML: " + xml_path);
    }
}

/**
 * Apply defaults to config message for missing values
 */
inline void applyDefaults(cgfx::CoregfxConfig& config) {
    // Variables defaults
    if (!config.has_variables()) {
        auto* vars = config.mutable_variables();
        vars->set_thread_count(8);
    } else if (config.variables().thread_count() <= 0) {
        config.mutable_variables()->set_thread_count(8);
    }

    // Paths defaults
    if (!config.has_paths()) {
        auto* paths = config.mutable_paths();
        paths->set_asset_root("assets/vkpbr5");
        paths->set_shader_dir("assets/vkpbr5/shaders");
        paths->set_log_dir("log");
        paths->set_temp_dir("./temp");
        paths->set_cache_dir("./cache");
    }

    // Scene defaults
    if (!config.has_scene()) {
        auto* scene = config.mutable_scene();
        scene->set_default_input_file("index.arctic");
        scene->set_auto_load(false);
    }

    // Settings defaults
    if (!config.has_settings()) {
        auto* settings = config.mutable_settings();
        settings->set_headless_mode(false);
        settings->set_studio_mode(true);
        settings->set_auto_close_enabled(false);
        settings->set_auto_close_timeout(5);
        settings->set_target_fps(60);
        settings->set_vsync_enabled(true);
    }

    // Debug defaults
    if (!config.has_debug()) {
        auto* debug = config.mutable_debug();
        debug->set_log_level(cgfx::LOG_LEVEL_INFO);
        debug->set_log_to_console(true);
        debug->set_log_to_file(true);
        debug->set_json_logging(false);
        debug->set_vulkan_validation(false);
        debug->set_compact_logging(false);
    }

    // Camera defaults (NEVER 0,0,0 - shows blue background!)
    if (!config.has_camera()) {
        auto* camera = config.mutable_camera();
        auto* pos = camera->mutable_default_position();
        pos->set_x(0.0f);
        pos->set_y(3.0f);
        pos->set_z(15.0f);
        auto* rot = camera->mutable_default_rotation();
        rot->set_x(0.0f);
        rot->set_y(0.0f);
        rot->set_z(0.0f);
        camera->set_default_fov(60.0f);
        camera->set_near_plane(0.1f);
        camera->set_far_plane(1000.0f);
    }

    // HTTP Server defaults
    if (!config.has_http_server()) {
        auto* http = config.mutable_http_server();
        http->set_enabled(true);
        http->set_port(52009);
        http->set_host("0.0.0.0");
        http->set_max_connections(10);
    }

    // State Dump defaults
    if (!config.has_state_dump()) {
        auto* dump = config.mutable_state_dump();
        dump->set_enabled(false);
        dump->set_default_filename("state_dump.log");
        dump->set_default_delay(3);
        dump->set_include_buffers(false);
        dump->set_include_textures(false);
    }
}

/**
 * Validate config and return errors
 */
inline cgfx::ConfigValidationResponse validate(const cgfx::CoregfxConfig& config) {
    cgfx::ConfigValidationResponse response;
    response.set_valid(true);

    auto addError = [&](const std::string& field, const std::string& msg, cgfx::ConfigErrorSeverity severity) {
        auto* err = response.add_errors();
        err->set_field_path(field);
        err->set_error_message(msg);
        err->set_severity(severity);
        if (severity == cgfx::CONFIG_ERROR_ERROR || severity == cgfx::CONFIG_ERROR_FATAL) {
            response.set_valid(false);
        }
    };

    // Validate thread count
    if (config.has_variables() && config.variables().thread_count() < 1) {
        addError("variables.thread_count", "Thread count must be >= 1", cgfx::CONFIG_ERROR_ERROR);
    }

    // Validate paths exist
    if (config.has_paths()) {
        if (!config.paths().asset_root().empty() && !std::filesystem::exists(config.paths().asset_root())) {
            addError("paths.asset_root", "Asset root directory does not exist", cgfx::CONFIG_ERROR_WARNING);
        }
    }

    // Validate camera
    if (config.has_camera()) {
        if (config.camera().has_default_position()) {
            auto& pos = config.camera().default_position();
            if (pos.x() == 0.0f && pos.y() == 0.0f && pos.z() == 0.0f) {
                addError("camera.default_position", "Camera at origin (0,0,0) shows blue background! Use (0, 3, 15)", cgfx::CONFIG_ERROR_WARNING);
            }
        }
        if (config.camera().default_fov() <= 0.0f || config.camera().default_fov() > 180.0f) {
            addError("camera.default_fov", "FOV must be between 0 and 180 degrees", cgfx::CONFIG_ERROR_ERROR);
        }
    }

    // Validate HTTP server
    if (config.has_http_server()) {
        if (config.http_server().port() < 1024 || config.http_server().port() > 65535) {
            addError("http_server.port", "Port must be between 1024 and 65535", cgfx::CONFIG_ERROR_WARNING);
        }
    }

    return response;
}

} // namespace config
} // namespace coregfx
