#pragma once

#include "../Config.h"
#include "../level/TileMap.h"
#include "../core/EventBus.h"
#include "raylib.h"
#include <vector>
#include <cstdint>
#include <queue>
#include <algorithm>

namespace Platformer {

enum class LiquidType {
    NONE = 0,
    WATER,
    LAVA
};

class LiquidSimulator {
    std::vector<std::vector<bool>> hasLiquid;
    std::vector<std::vector<LiquidType>> typeGrid;
    bool checkLiquid;
    int width;
    int height;
    TileMap* tileMap;
    
    bool isWaterDirty;
    std::vector<std::vector<bool>> isSpurtBlock;
    std::vector<std::vector<float>> spurtTimer;

    Texture2D waterTex;
    Texture2D lavaTex;
    Texture2D lavaTopTex;

    void onTerrainDestroyed(EventData data);

public:
    LiquidSimulator(TileMap* map);
    ~LiquidSimulator();

    void update(float dt);
    void render(Camera2D& cam);
    
    void addLiquid(int gx, int gy, uint8_t amount, LiquidType type);
    void removeLiquid(int gx, int gy);
    bool hasLiquidAt(int gx, int gy) const;
    bool isWaterAt(Rectangle rect) const;
    bool isLavaAt(Rectangle rect) const;
    void applyFloodedFloorModifier(int bottomRows, LiquidType type = LiquidType::WATER);
    void updateSpurts(float dt, float playerX, float playerY);
};

} // namespace Platformer
