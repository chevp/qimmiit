/**
 * Cryo Engine - Event Bus Implementation
 */

#include "EventBus.hpp"
#include <chrono>

namespace cryo {
namespace engine {

bool EventBus::initialize() {
    return true;
}

void EventBus::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribers_.clear();
}

uint64_t EventBus::subscribe(EventType type, EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    uint64_t subscriptionId = nextSubscriptionId_++;
    subscribers_[type].push_back({subscriptionId, callback});

    return subscriptionId;
}

void EventBus::unsubscribe(uint64_t subscriptionId) {
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& [type, subs] : subscribers_) {
        auto it = std::remove_if(subs.begin(), subs.end(),
            [subscriptionId](const auto& pair) {
                return pair.first == subscriptionId;
            });
        subs.erase(it, subs.end());
    }
}

void EventBus::publish(const Event& event) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = subscribers_.find(event.type);
    if (it != subscribers_.end()) {
        for (const auto& [id, callback] : it->second) {
            callback(event);
        }
    }
}

void EventBus::publish(EventType type, const std::string& source, const Payload& data) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    Event event{type, source, data, static_cast<uint64_t>(timestamp)};
    publish(event);
}

} // namespace engine
} // namespace cryo