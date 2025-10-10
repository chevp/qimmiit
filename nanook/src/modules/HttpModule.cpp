#include "HttpModule.hpp"
#include "../core/CentralAgent.hpp"

namespace cryo { namespace engine {

bool HttpModule::initialize(CentralAgent& agent) {
    agent_ = &agent;
    agent.getLogManager().info("HttpModule", "Initialized");
    return true;
}
void HttpModule::update(float) {}
void HttpModule::shutdown() { if(agent_) agent_->getLogManager().info("HttpModule", "Shutdown"); }
std::vector<std::string> HttpModule::getDependencies() const { return {}; }
void HttpModule::handleMessage(const Message&) {}
ModuleSpec HttpModule::getSpecification() const {
    ModuleSpec s; s.name="HttpModule"; s.version="1.0.0"; s.type=ModuleType::HTTP;
    s.description="REST API server"; return s;
}

}}