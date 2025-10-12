/**
 * Cryo Engine - Cache Module
 */

#pragma once

#include "../core/IModule.hpp"

namespace cryo {
namespace engine {

class CacheModule : public IModule {
public:
    CacheModule() = default;
    ~CacheModule() override = default;

    // IModule interface
    bool initialize(CentralAgent& agent) override;
    void update(float deltaTime) override;
    void shutdown() override;

    std::string getName() const override { return "CacheModule"; }
    ModuleType getType() const override { return ModuleType::CACHE; }
    std::vector<std::string> getDependencies() const override;
    void handleMessage(const Message& msg) override;
    ModuleSpec getSpecification() const override;

private:
    CentralAgent* agent_ = nullptr;
};

} // namespace engine
} // namespace cryo