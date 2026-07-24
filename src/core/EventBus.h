#pragma once

#include "../Config.h"
#include <unordered_map>
#include <vector>
#include <functional>

namespace Platformer {

enum class EventType {
    EVENT_TERRAIN_DESTROYED,
    EVENT_SPAWN_FLAME,
    EVENT_SPAWN_BOMB,
    EVENT_SPAWN_LAVA_DRIP,
    EVENT_BOMB_EXPLODE,
    EVENT_PLAYER_DAMAGED,
    EVENT_PLAYER_DEATH,
    EVENT_GOLD_COLLECTED,
    EVENT_GHOST_SPAWN,
    EVENT_ENEMY_KILLED,
    EVENT_SPAWN_ITEM,
    EVENT_SPAWN_ARROW,
    EVENT_SPAWN_BUBBLE,
    EVENT_ADD_LIQUID
};

struct EventData {
    int gridX = 0;
    int gridY = 0;
    float worldX = 0.0f;
    float worldY = 0.0f;
    int amount = 0;
    char entityCode = ' ';
    float vx = 0.0f;
    float vy = 0.0f;
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
    
    void clearListeners(EventType type);
    void clearAllListeners();

    void publish(EventType type, EventData data);
};

}