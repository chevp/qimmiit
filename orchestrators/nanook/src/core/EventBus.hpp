/**
 * Nanook Engine - Event Bus
 */

#pragma once

#include "Types.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace cryo {
namespace engine {

class EventBus {
public:
    bool initialize();
    void shutdown();

    uint64_t subscribe(EventType type, EventCallback callback);
    void unsubscribe(uint64_t subscriptionId);

    void publish(const Event& event);
    void publish(EventType type, const std::string& source, const Payload& data);

private:
    std::mutex mutex_;
    std::unordered_map<EventType, std::vector<std::pair<uint64_t, EventCallback>>> subscribers_;
    uint64_t nextSubscriptionId_ = 1;
};

} // namespace engine
} // namespace cryo
