#pragma once

#include "../Config.h"
#include <unordered_map>

namespace Platformer {

class EventBus {
private:
    static EventBus *instance;
    // unordered_map<EventType, std::vector<EventCallback> listeners;

    EventBus();
    ~EventBus();
public:
    static EventBus* getInstance();

    EventBus(const EventBus &) = delete;
    EventBus& operator = (const EventBus &) = delete;

    // void subscribe(EventType type, EventCallback cb);

    // void publish(EventType type, EventData);
};

}