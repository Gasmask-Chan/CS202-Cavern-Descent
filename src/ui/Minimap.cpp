#include "Minimap.h"
#include <algorithm>
#include <cmath>

namespace Platformer {

Minimap::Minimap(Vector2 exitPos) {
    for (int y = 0; y < MAP_ROOMS_Y; y++) {
        for (int x = 0; x < MAP_ROOMS_X; x++) {
            visited[y][x] = false;
        }
    }
    
    currentRoomX = 0;
    currentRoomY = 0;
    
    // Map the absolute pixel position of the exit to room coordinates
    float roomPixelWidth = ROOM_WIDTH * MAP_TILE_SIZE;
    float roomPixelHeight = ROOM_HEIGHT * MAP_TILE_SIZE;
    
    exitRoomX = static_cast<int>(std::floor(exitPos.x / roomPixelWidth));
    exitRoomY = static_cast<int>(std::floor(exitPos.y / roomPixelHeight));
    
    // Clamp to valid range just in case
    exitRoomX = std::max(0, std::min(exitRoomX, MAP_ROOMS_X - 1));
    exitRoomY = std::max(0, std::min(exitRoomY, MAP_ROOMS_Y - 1));
}

void Minimap::update(float playerWorldX, float playerWorldY) {
    // 1 tile = MAP_TILE_SIZE (32px).
    // 1 room = 10 tiles = 320px width/height.
    float roomPixelWidth = ROOM_WIDTH * MAP_TILE_SIZE;
    float roomPixelHeight = ROOM_HEIGHT * MAP_TILE_SIZE;
    
    currentRoomX = static_cast<int>(std::floor(playerWorldX / roomPixelWidth));
    currentRoomY = static_cast<int>(std::floor(playerWorldY / roomPixelHeight));
    
    currentRoomX = std::max(0, std::min(currentRoomX, MAP_ROOMS_X - 1));
    currentRoomY = std::max(0, std::min(currentRoomY, MAP_ROOMS_Y - 1));
    
    // Reveal the room the player is in
    visited[currentRoomY][currentRoomX] = true;
}

void Minimap::render() {
    float startX = mapX;
    float startY = mapY;
    
    // Draw background panel for the minimap
    float bgWidth = MAP_ROOMS_X * (roomSize + margin) + margin;
    float bgHeight = MAP_ROOMS_Y * (roomSize + margin) + margin;
    DrawRectangle(startX - margin, startY - margin, bgWidth, bgHeight, Color{20, 20, 20, 200});
    DrawRectangleLines(startX - margin, startY - margin, bgWidth, bgHeight, DARKGRAY);

    for (int y = 0; y < MAP_ROOMS_Y; y++) {
        for (int x = 0; x < MAP_ROOMS_X; x++) {
            float rx = startX + x * (roomSize + margin);
            float ry = startY + y * (roomSize + margin);
            
            if (!visited[y][x]) {
                // Fog of War: Unexplored rooms are black
                DrawRectangle(rx, ry, roomSize, roomSize, BLACK);
                DrawRectangleLines(rx, ry, roomSize, roomSize, Color{40, 40, 40, 255});
            } else {
                // Explored room: Dark Gray
                DrawRectangle(rx, ry, roomSize, roomSize, Color{80, 80, 80, 255});
                DrawRectangleLines(rx, ry, roomSize, roomSize, LIGHTGRAY);
                
                // Draw Exit indicator if this room has the exit
                if (x == exitRoomX && y == exitRoomY) {
                    // Draw slightly larger so it isn't completely hidden by the player
                    DrawRectangle(rx + roomSize/2 - 4, ry + roomSize/2 - 4, 8, 8, GREEN);
                }
                
                // Draw Player indicator if this is the current room
                if (x == currentRoomX && y == currentRoomY) {
                    // Using a perfectly centered 6x6 square
                    DrawRectangle(rx + roomSize/2 - 3, ry + roomSize/2 - 3, 6, 6, WHITE);
                }
            }
        }
    }
}

}
