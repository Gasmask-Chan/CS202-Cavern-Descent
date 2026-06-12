#pragma once

#include "raylib.h"
#include "../Config.h"

#include <vector>

namespace Platformer {

class TileMap {

enum TileType { //Just for example, modify it later if you need to
    EMPTY,
    WALL,
    CRACKED,
    PLATFORM,
    
    SPIKE_TRAP,
    ARROW_TRAP,
    BOULDER_TRAP,

    ROPE_NODE,
    EXIT_DOOR
};

private:
    std::vector<std::vector<TileType>> tiles;
    int width;
    int height;
    int tileSize;

public:
    TileMap(int w, int h, int size);

    /**
     * @brief Returns `tiles[y][x]` if in bounds, else `TileType::WALL` (out-of-bounds treated as solid for safety).
     * 
     * @param x 
     * @param y 
     * @return TileType 
     */
    TileType getTile(int x, int y);

    /**
     * @brief Sets `tiles[y][x] = type`. Called by terrain destruction and level editor. Does bounds check first.
     * 
     * @param x 
     * @param y 
     * @param type 
     */
    void setTile(int x, int y, TileType type);

    /**
     * @brief Returns `true` if tile at `(x,y)` is `WALL`, `CRACKED`, or `PLATFORM`. Used by physics for collision and by BFS for reachability.
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isSolid(int x, int y);

    /**
     * @brief Returns `true` if tile blocks light (`WALL`, `CRACKED`). Used by `LightingSystem` shadowcasting. `PLATFORM` tiles are NOT opaque (light passes through).
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isOpaque(int x, int y);

    /**
     * @brief Returns `true` only if tile is `TileType::CRACKED`. Only cracked blocks can be broken by whip attack.
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isCracked(int x, int y);

    /**
     * @brief Iterates only tiles visible within the camera's viewport (culling). For each visible tile, draws the zone-appropriate sprite at grid position, tinted by `ColorTint(WHITE, lightMap[gy][gx])`. Empty tiles are not drawn (cave background is black).
     * 
     * @param cam 
     * @param lightMap 
     */
    void render(Camera2D &cam, std::vector<std::vector<float>> lightMap);

    /**
     * @brief Returns `Vector2i{(int)(wx / tileSize), (int)(wy / tileSize)}`. Converts pixel coordinates to grid indices.
     * 
     * @param wx 
     * @param wy 
     * @return Vector2i 
     */
    Vector2i worldToGrid(float wx, float wy);   

    /**
     * @brief Returns `Vec2f{gx * tileSize, gy * tileSize}`. Converts grid indices to pixel coordinates (top-left corner of tile).
     * 
     * @param gx 
     * @param gy 
     * @return Vector2 
     */
    Vector2 gridToWorld(int gx, int gy);

    int getWidth();

    int getHeight();

    int getTileSize();

    bool isInBounds(int x, int y);
};
}