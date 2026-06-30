#include "EventBus.h"

namespace Platformer {

EventBus::EventBus() {
}

EventBus::~EventBus() {
}

EventBus* EventBus::getInstance() {
    static EventBus instance;
    return &instance;
}

void EventBus::subscribe(EventType type, EventCallback cb) {
    listeners[type].push_back(cb);
}

void EventBus::publish(EventType type, EventData data) {
    auto it = listeners.find(type);
    if (it != listeners.end()) {
        for (auto& cb : it->second) {
            cb(data);
        }
    }
}

}
