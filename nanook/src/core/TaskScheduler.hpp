/**
 * Nanook Engine - Task Scheduler
 */

#pragma once

#include "Types.hpp"
#include <chrono>

namespace cryo {
namespace engine {

class TaskScheduler {
public:
    bool initialize(size_t threadCount);
    void shutdown();

    uint64_t schedule(Task task, TaskPriority priority = TaskPriority::NORMAL);
    uint64_t scheduleDelayed(Task task, std::chrono::milliseconds delay,
                            TaskPriority priority = TaskPriority::NORMAL);
    uint64_t scheduleRecurring(Task task, std::chrono::milliseconds interval,
                              TaskPriority priority = TaskPriority::NORMAL);

    bool cancelTask(uint64_t taskId);
    size_t getPendingTaskCount() const;
};

} // namespace engine
} // namespace cryo
