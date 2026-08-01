#pragma once

#include "raylib.h"
#include "../Config.h"

#include <vector>
#include <cstdint>

namespace Platformer {

enum class TileType {
    NOTHING = 0,
    CAVE_ROCK = 1,
    CAVE_REGULAR = 2,
    STONE_BLOCK = 3,
    CAVE_DOWN_ORIENTED = 4,
    CAVE_SOME_GOLD = 5,
    CAVE_MUCH_GOLD = 6,
    CAVE_UP_ORIENTED = 7,
    CAVE_UP_DOWN_ORIENTED = 8,
    LADDER = 9,
    LADDER_DECK = 10,
    ARROW_TRAP_LEFT = 11,
    ARROW_TRAP_RIGHT = 12,
    ENTRANCE = 13,
    EXIT = 14,
    CAVE_SMOOTH = 24, // Shop floor (Col 1, Row 11)
    // (Other Spelunky DS tiles 15-42 omitted as they aren't used in cave generation, but can be added if needed)
    
    // We add a few custom ones we need for logic that aren't in DS tiles natively
    SPIKE_TRAP = 100, 
    ROPE_NODE = 101,
    CHEST = 102,
    ENEMY_SNAKE = 103,
    ENEMY_BAT = 104,
    ENEMY_SPIDER = 105,
    LAVA = 106,
    WATER = 107
};

class TileMap {
private:
    std::vector<std::vector<TileType>> tiles;
    int width;
    int height;
    int tileSize;

    Texture2D dsTileset;
    Color zoneTint = WHITE;

public:
    TileMap(int w, int h, int size);
    ~TileMap();

    /**
     * @brief Returns `tiles[y][x] if in bounds, else `TileType::NOTHING` (out-of-bounds treated as solid for safety).
     * 
     * @param x 
     * @param y 
     * @return TileType 
     */
    TileType getTile(int x, int y) const;

    /**
     * @brief Sets `tiles[y][x] = type`. Called by terrain destruction and level editor. Does bounds check first.
     * 
     * @param x 
     * @param y 
     * @param type 
     */
    void setTile(int x, int y, TileType type);

    /**
     * @brief Safely destroys a cracked block at the specified tile coordinates, replacing it with NOTHING.
     * 
     * @param x The X coordinate on the tile grid.
     * @param y The Y coordinate on the tile grid.
     */
    void destroyBlock(int x, int y);

    /**
     * @brief Returns `true` if tile at `(x,y)` is `WALL`, `CRACKED`. Used by physics for collision and by BFS for reachability.
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isSolid(int x, int y) const;

    /**
     * @brief Returns `true` if tile at `(x,y)` is a one-way platform (can be stood on, but passed through from below).
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isOneWayPlatform(int x, int y) const;

    /**
     * @brief Returns `true` if tile blocks light (`WALL`, `CRACKED`). Used by `LightingSystem` shadowcasting. `PLATFORM` tiles are NOT opaque (light passes through).
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isOpaque(int x, int y) const;

    /**
     * @brief Returns `true` only if tile is `TileType::CRACKED`. Only cracked blocks can be broken by whip attack.
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isCracked(int x, int y) const;

    /**
     * @brief Returns `true` if tile is a ladder.
     * 
     * @param x 
     * @param y 
     * @return true 
     * @return false 
     */
    bool isLadder(int x, int y) const;

    /**
     * @brief Iterates only tiles visible within the camera's viewport (culling). For each visible tile, draws the zone-appropriate sprite at grid position, tinted by `ColorTint(WHITE, lightMap[gy][gx])`. Empty tiles are not drawn (cave background is black).
     * 
     * @param cam 
     * @param lightMap 
     * @param foregroundPass
     */
    void render(Camera2D &cam, const std::vector<std::vector<Vector3>>& lightMap, bool foregroundPass);

    /**
     * @brief Renders a repeating background layer with parallax scrolling based on the camera.
     * 
     * @param cam 
     */
    void renderParallaxBackground(Camera2D &cam);

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

    void setZoneTint(Color tint);

    bool isInBounds(int x, int y) const;
};
}