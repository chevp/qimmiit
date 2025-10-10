#pragma once
#include "../core/IModule.hpp"

namespace cryo {
namespace engine {

class AssetModule : public IModule {
public:
    bool initialize(CentralAgent& agent) override;
    void update(float deltaTime) override;
    void shutdown() override;
    std::string getName() const override { return "AssetModule"; }
    ModuleType getType() const override { return ModuleType::ASSET; }
    std::vector<std::string> getDependencies() const override;
    void handleMessage(const Message& msg) override;
    ModuleSpec getSpecification() const override;
private:
    CentralAgent* agent_ = nullptr;
};

}}