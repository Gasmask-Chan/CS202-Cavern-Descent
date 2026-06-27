#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include "../Config.h"
#include "TileMap.h"
#include "RoomTemplate.h"
#include "../entities/DynamicEntity.h"
#include "../entities/Item.h"
#include "../entities/Trap.h"

namespace Platformer {

constexpr int ROOM_WIDTH = 10;
constexpr int ROOM_HEIGHT = 8;
constexpr int MAP_ROOMS_X = 4;
constexpr int MAP_ROOMS_Y = 4;
constexpr int MAP_TILE_SIZE = 32;

struct GeneratedLevel {
    std::unique_ptr<TileMap> tileMap;
    std::vector<std::unique_ptr<DynamicEntity>> dynamicEntities;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<Trap>> traps;
    Vector2 playerSpawn;
    Vector2 exitPos;
    DifficultyConfig difficulty;
    FloorModifier modifier;
};

class LevelGenerator {
private:
    RoomRole macroGrid[MAP_ROOMS_Y][MAP_ROOMS_X];
    std::vector<RoomTemplate> templates;

    // For keeping track of the spawn lists while generating
    std::vector<std::unique_ptr<DynamicEntity>> tempEnemies;
    std::vector<std::unique_ptr<Item>> tempItems;
    std::vector<std::unique_ptr<Trap>> tempTraps;
    Vector2 tempPlayerSpawn;
    Vector2 tempExitPos;

    int startRoomX = 0;
    int startRoomY = 0;
    int exitRoomX = 0;
    int exitRoomY = 0;

    void generateMacroGrid();
    
    RoomTemplate selectRoomTemplate(RoomRole role);
    
    void populateRoom(const RoomTemplate& tpl, int gx, int gy, RoomRole role, TileMap* map);
    
    void generateChunks(TileMap* map);
    
    void generateBorders(TileMap* map); // New pass for autotiling decals
    
    bool validateLevel(TileMap* map, Vector2i start, Vector2i exit);
    
    bool bfsReachability(TileMap* map, Vector2i from, Vector2i to);
    
    DifficultyConfig getDifficultyConfig(int floor);
    
    FloorModifier rollFloorModifier(int floor);

public:
    LevelGenerator();
    
    GeneratedLevel generate(int floor, ZoneType zone);
};

} // namespace Platformer
