/**
 * Cryo Engine - Asset Module (stub)
 */

#include "AssetModule.hpp"
#include "../core/CentralAgent.hpp"

namespace cryo {
namespace engine {

bool AssetModule::initialize(CentralAgent& agent) {
    agent_ = &agent;
    agent.getLogManager().info("AssetModule", "Initialized");
    return true;
}

void AssetModule::update(float deltaTime) {}

void AssetModule::shutdown() {
    if (agent_) agent_->getLogManager().info("AssetModule", "Shutdown");
}

std::vector<std::string> AssetModule::getDependencies() const {
    return {"CacheModule"};
}

void AssetModule::handleMessage(const Message& msg) {}

ModuleSpec AssetModule::getSpecification() const {
    ModuleSpec spec;
    spec.name = "AssetModule";
    spec.version = "1.0.0";
    spec.type = ModuleType::ASSET;
    spec.description = "Asset processing and validation";
    return spec;
}

} // namespace engine
} // namespace cryo