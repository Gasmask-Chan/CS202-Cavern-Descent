#pragma once

#include "raylib.h"
#include "../level/LevelGenerator.h"

namespace Platformer {

class Minimap {
private:
    bool visited[MAP_ROOMS_Y][MAP_ROOMS_X];
    int currentRoomX;
    int currentRoomY;
    int exitRoomX;
    int exitRoomY;

    // Visual configurations
    float mapX = 16.0f;
    float mapY = 16.0f;
    float roomSize = 16.0f;
    float margin = 2.0f;

public:
    Minimap(Vector2 exitPos);
    
    // Updates the player's current room based on their world position
    void update(float playerWorldX, float playerWorldY);
    
    // Renders the minimap UI overlay
    void render();
};

}
