#include "ComputeModule.hpp"
#include "../core/CentralAgent.hpp"

namespace cryo { namespace engine {
bool ComputeModule::initialize(CentralAgent& agent) {
    agent_ = &agent;
    agent.getLogManager().info("ComputeModule", "Initialized");
    return true;
}
void ComputeModule::update(float) {}
void ComputeModule::shutdown() { if(agent_) agent_->getLogManager().info("ComputeModule", "Shutdown"); }
std::vector<std::string> ComputeModule::getDependencies() const { return {}; }
void ComputeModule::handleMessage(const Message&) {}
ModuleSpec ComputeModule::getSpecification() const {
    ModuleSpec s; s.name="ComputeModule"; s.version="1.0.0"; s.type=ModuleType::COMPUTE;
    s.description="Background task processing"; return s;
}
}}
