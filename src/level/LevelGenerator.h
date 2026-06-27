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
    std::unordered_map<int, std::vector<int>> adjacencyList;
    std::vector<int> goldenPath;
    std::vector<RoomTemplate> templates;

    // For keeping track of the spawn lists while generating
    std::vector<std::unique_ptr<DynamicEntity>> tempEnemies;
    std::vector<std::unique_ptr<Item>> tempItems;
    std::vector<std::unique_ptr<Trap>> tempTraps;
    Vector2 tempPlayerSpawn;
    Vector2 tempExitPos;

    void buildGraph();
    
    void generateGoldenPath();
    
    RoomTemplate selectRoomTemplate(RoomRole role);
    
    void populateRoom(const RoomTemplate& tpl, int gx, int gy, TileMap* map);
    
    void carvePathways(TileMap* map);
    
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
