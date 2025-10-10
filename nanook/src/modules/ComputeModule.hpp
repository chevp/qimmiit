#pragma once
#include "../core/IModule.hpp"
namespace cryo { namespace engine {
class ComputeModule : public IModule {
public:
    bool initialize(CentralAgent& agent) override;
    void update(float) override;
    void shutdown() override;
    std::string getName() const override { return "ComputeModule"; }
    ModuleType getType() const override { return ModuleType::COMPUTE; }
    std::vector<std::string> getDependencies() const override;
    void handleMessage(const Message&) override;
    ModuleSpec getSpecification() const override;
private:
    CentralAgent* agent_ = nullptr;
};
}}
