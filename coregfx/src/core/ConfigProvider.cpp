#include <coregfx/core/ConfigProvider.hpp>

std::map<std::string, std::string> ConfigProvider::paths_;
bool ConfigProvider::initialized_ = false;

void ConfigProvider::initialize(const std::map<std::string, std::string>& pathValues) {
    paths_ = pathValues;
    initialized_ = true;
}

std::string ConfigProvider::getAssetRoot() {
    return getPath("ASSET_ROOT", "assets/vkpbr5");
}

std::string ConfigProvider::getShaderDir() {
    return getPath("SHADER_DIR", "assets/vkpbr5/shaders");
}

std::string ConfigProvider::getPath(const std::string& key, const std::string& defaultValue) {
    if (!initialized_) {
        return defaultValue;
    }

    auto it = paths_.find(key);
    if (it != paths_.end()) {
        return it->second;
    }

    return defaultValue;
}