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
    std::vector<std::vector<int8_t>> flowDir;
    int width;
    int height;
    TileMap* tileMap;
    
    bool isWaterDirty;

    Texture2D waterTex;

    void onTerrainDestroyed(EventData data);

public:
    LiquidSimulator(TileMap* map);
    ~LiquidSimulator();

    void update(float dt);
    void render(Camera2D& cam);
    
    void addLiquid(int gx, int gy, uint8_t amount, LiquidType type);
    void removeLiquid(int gx, int gy);
    bool isWaterAt(Rectangle rect) const;
    void applyFloodedFloorModifier(int bottomRows);
};

} // namespace Platformer
