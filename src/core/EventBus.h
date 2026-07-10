#pragma once

#include "../Config.h"
#include <unordered_map>
#include <vector>
#include <functional>

namespace Platformer {

enum class EventType {
    EVENT_TERRAIN_DESTROYED,
    EVENT_BOMB_EXPLODE,
    EVENT_PLAYER_DAMAGED,
    EVENT_PLAYER_DEATH,
    EVENT_GOLD_COLLECTED,
    EVENT_GHOST_SPAWN,
    EVENT_ENEMY_KILLED,
    EVENT_SPAWN_ITEM
};

struct EventData {
    int gridX = 0;
    int gridY = 0;
    float worldX = 0.0f;
    float worldY = 0.0f;
    int amount = 0;
    char entityCode = ' ';
};

using EventCallback = std::function<void(EventData)>;

class EventBus {
private:
    std::unordered_map<EventType, std::vector<EventCallback>> listeners;

    EventBus();
    ~EventBus();
public:
    static EventBus* getInstance();

    EventBus(const EventBus &) = delete;
    EventBus& operator = (const EventBus &) = delete;

    void subscribe(EventType type, EventCallback cb);

    void publish(EventType type, EventData data);
};

}