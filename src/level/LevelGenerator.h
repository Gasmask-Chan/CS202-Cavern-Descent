#pragma once

#include <vector>
#include <memory>
#include "../Config.h"
#include "TileMap.h"
#include "RoomTemplate.h"
#include "../entities/DynamicEntity.h"
#include "../entities/items/Item.h"
#include "../entities/traps/Trap.h"
#include "../liquid/LiquidSimulator.h"

namespace Platformer {

struct LiquidSpawn {
    int gx;
    int gy;
    LiquidType type;
};

constexpr int ROOM_WIDTH  = 10;
constexpr int ROOM_HEIGHT = 10;
constexpr int MAP_ROOMS_X = 4;
constexpr int MAP_ROOMS_Y = 4;
constexpr int MAP_TILE_SIZE = 32;

struct GeneratedLevel {
    std::unique_ptr<TileMap> tileMap;
    std::vector<std::unique_ptr<DynamicEntity>> dynamicEntities;
    std::vector<std::unique_ptr<Item>> items;
    std::vector<std::unique_ptr<Trap>> traps;
    std::vector<std::unique_ptr<Entity>> decorations;
    Vector2 playerSpawn;
    Vector2 exitPos;
    DifficultyConfig difficulty;
    FloorModifier modifier;
    std::vector<LiquidSpawn> initialLiquids;
    Rectangle shopArea;
};

class LevelGenerator {
private:
    RoomRole macroGrid[MAP_ROOMS_Y][MAP_ROOMS_X];
    int roomVariations[MAP_ROOMS_Y][MAP_ROOMS_X];

    // Temporary entity lists built during generate()
    std::vector<std::unique_ptr<DynamicEntity>> tempEnemies;
    std::vector<std::unique_ptr<Item>>          tempItems;
    std::vector<std::unique_ptr<Trap>>          tempTraps;
    std::vector<std::unique_ptr<Entity>>        tempDecorations;
    Vector2 tempPlayerSpawn;
    Vector2 tempExitPos;

    // Spelunky-DS style global entity limits per level
    int snakesLeft   = 4;
    int batsLeft     = 4;
    int spidersLeft  = 4;
    int cavemenLeft  = 3;
    int skeletonsLeft= 3;
    int damselsLeft  = 1;
    int spikesLeft   = 4;
    int lastPlacement = 3; // cooldown counter to avoid clustering

    int startRoomX = 0;
    int startRoomY = 0;
    int exitRoomX  = 0;
    int exitRoomY  = 0;

    // ---- Macro-grid (Spelunky path algorithm) ----
    void generateMacroGrid();
    void obtainNewDirection(int currX, bool& movingLeft);
    void placeShop();
    void placeAltar();

    // ---- Template selection ----
    // Returns a random variation index for the given room role.
    int selectVariation(RoomRole role, bool isEntrance = false, ZoneType zone = ZoneType::CAVE) const;

    // ---- Room population ----
    void instantiateTiles(ZoneType zone, const int tileGrid[ROOM_HEIGHT][ROOM_WIDTH], int gx, int gy, RoomRole role, TileMap* map);
    void instantiateLakeRoom(int gx, int gy, TileMap* map, LiquidType lType, std::vector<LiquidSpawn>& initialLiquids);
    void populateEntities(const int npcGrid[ROOM_HEIGHT][ROOM_WIDTH], const int lootGrid[ROOM_HEIGHT][ROOM_WIDTH], int gx, int gy, RoomRole role, TileMap* map);

    // ---- Post-passes ----
    // (generateChunks and generateBorders removed for Spelunky DS native rendering)

    // ---- Validation ----
    bool validateLevel(TileMap* map, Vector2i start, Vector2i exit);
    bool bfsReachability(TileMap* map, Vector2i from, Vector2i to);

    // ---- Difficulty / modifiers ----
    DifficultyConfig getDifficultyConfig(int floor);
    FloorModifier    rollFloorModifier(int floor);

public:
    LevelGenerator() = default;
    ~LevelGenerator();

    GeneratedLevel generate(int floor, ZoneType zone);
};

} // namespace Platformer
