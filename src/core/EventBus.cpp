#include "EventBus.h"

namespace Platformer {

EventBus* EventBus::instance = nullptr;

EventBus::EventBus() {
}

EventBus::~EventBus() {
}

EventBus* EventBus::getInstance() {
    if (instance == nullptr) {
        instance = new EventBus();
    }
    return instance;
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
