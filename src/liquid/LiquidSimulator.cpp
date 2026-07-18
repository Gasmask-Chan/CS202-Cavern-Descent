#include "LiquidSimulator.h"
#include <algorithm>
#include <iostream>

namespace Platformer {

LiquidSimulator::LiquidSimulator(TileMap* map) : tileMap(map), isWaterDirty(false) {
    width = map->getWidth();
    height = map->getHeight();
    hasLiquid.resize(height, std::vector<bool>(width, false));
    typeGrid.resize(height, std::vector<LiquidType>(width, LiquidType::NONE));
    flowDir.resize(height, std::vector<int8_t>(width, 0)); // Initialize flowDir
    
    EventBus::getInstance()->subscribe(EventType::EVENT_TERRAIN_DESTROYED, 
        [this](EventData data) { this->onTerrainDestroyed(data); });

    waterTex = LoadTexture("assets/sprites/16x16/water.png");
}

LiquidSimulator::~LiquidSimulator() {
    UnloadTexture(waterTex);
}

static float liquidUpdateTimer = 0.0f;

void LiquidSimulator::update(float dt) {
    liquidUpdateTimer += dt;
    if (liquidUpdateTimer >= 0.05f) { // 20 FPS for liquids
        liquidUpdateTimer = 0.0f;
    } else {
        return;
    }

    static bool flowLeftToRight = true;
    flowLeftToRight = !flowLeftToRight;

    bool changed = false;
    std::vector<std::vector<bool>> moved(height, std::vector<bool>(width, false));

    // Iterate bottom-to-top, and alternate left-to-right/right-to-left
    for (int y = height - 1; y >= 0; y--) {
        for (int i = 0; i < width; i++) {
            int x = flowLeftToRight ? i : (width - 1 - i);
            
            if (hasLiquid[y][x] && !moved[y][x]) {
                bool canMoveDown = (y + 1 < height) && !tileMap->isSolid(x, y + 1) && !hasLiquid[y + 1][x];
                
                if (canMoveDown) {
                    hasLiquid[y + 1][x] = true;
                    typeGrid[y + 1][x] = typeGrid[y][x];
                    flowDir[y + 1][x] = flowDir[y][x];
                    moved[y + 1][x] = true;
                    hasLiquid[y][x] = false;
                    typeGrid[y][x] = LiquidType::NONE;
                    flowDir[y][x] = 0;
                    changed = true;
                } else {
                    bool canMoveLeft = (x - 1 >= 0) && !tileMap->isSolid(x - 1, y) && !hasLiquid[y][x - 1];
                    if (canMoveLeft) {
                        bool destHole = (y + 1 < height) && !tileMap->isSolid(x - 1, y + 1) && !hasLiquid[y + 1][x - 1];
                        canMoveLeft = destHole;
                    }
                    
                    bool canMoveRight = (x + 1 < width) && !tileMap->isSolid(x + 1, y) && !hasLiquid[y][x + 1];
                    if (canMoveRight) {
                        bool destHole = (y + 1 < height) && !tileMap->isSolid(x + 1, y + 1) && !hasLiquid[y + 1][x + 1];
                        canMoveRight = destHole;
                    }
                    
                    if (canMoveLeft && canMoveRight) {
                        // Use flow direction memory to prevent oscillation
                        if (flowDir[y][x] == -1) canMoveRight = false;
                        else if (flowDir[y][x] == 1) canMoveLeft = false;
                        else {
                            if (GetRandomValue(0, 1) == 0) canMoveRight = false;
                            else canMoveLeft = false;
                        }
                    }

                    if (canMoveLeft) {
                        hasLiquid[y][x - 1] = true;
                        typeGrid[y][x - 1] = typeGrid[y][x];
                        flowDir[y][x - 1] = -1; // Moving left
                        moved[y][x - 1] = true;
                        hasLiquid[y][x] = false;
                        typeGrid[y][x] = LiquidType::NONE;
                        flowDir[y][x] = 0;
                        changed = true;
                    } else if (canMoveRight) {
                        hasLiquid[y][x + 1] = true;
                        typeGrid[y][x + 1] = typeGrid[y][x];
                        flowDir[y][x + 1] = 1; // Moving right
                        moved[y][x + 1] = true;
                        hasLiquid[y][x] = false;
                        typeGrid[y][x] = LiquidType::NONE;
                        flowDir[y][x] = 0;
                        changed = true;
                    } else {
                        // If it couldn't move left or right, it stops flowing
                        flowDir[y][x] = 0;
                    }
                }
            }
        }
    }
    
    isWaterDirty = changed;
}

void LiquidSimulator::addLiquid(int gx, int gy, uint8_t amount, LiquidType type) {
    if (!tileMap->isInBounds(gx, gy)) return;
    hasLiquid[gy][gx] = true;
    typeGrid[gy][gx] = type;
    isWaterDirty = true;
}

void LiquidSimulator::removeLiquid(int gx, int gy) {
    if (!tileMap->isInBounds(gx, gy)) return;
    hasLiquid[gy][gx] = false;
    typeGrid[gy][gx] = LiquidType::NONE;
}

bool LiquidSimulator::isWaterAt(Rectangle rect) const {
    int tileSize = tileMap->getTileSize();
    int startX = std::max(0, (int)(rect.x / tileSize));
    int startY = std::max(0, (int)(rect.y / tileSize));
    int endX = std::min(width - 1, (int)((rect.x + rect.width) / tileSize));
    int endY = std::min(height - 1, (int)((rect.y + rect.height) / tileSize));
    
    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (hasLiquid[y][x]) return true;
        }
    }
    return false;
}

void LiquidSimulator::applyFloodedFloorModifier(int bottomRows) {
    for (int y = height - 1; y >= std::max(0, height - bottomRows); y--) {
        for (int x = 0; x < width; x++) {
            if (!tileMap->isSolid(x, y)) {
                hasLiquid[y][x] = true;
                typeGrid[y][x] = LiquidType::WATER;
            }
        }
    }
}

void LiquidSimulator::onTerrainDestroyed(EventData data) {
    isWaterDirty = true;
}

void LiquidSimulator::render(Camera2D& cam) {
    int tileSize = tileMap->getTileSize();

    int screenW = GetScreenWidth();
    if (screenW == 0) screenW = 800; // fallback
    int screenH = GetScreenHeight();
    if (screenH == 0) screenH = 600; // fallback

    int startX = std::max(0, (int)(cam.target.x - cam.offset.x) / tileSize);
    int startY = std::max(0, (int)(cam.target.y - cam.offset.y) / tileSize);
    int endX = std::min(width, startX + (screenW / tileSize) + 2);
    int endY = std::min(height, startY + (screenH / tileSize) + 2);

    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            if (hasLiquid[y][x]) {
                float rectHeight = tileSize;
                float rectY = (y * tileSize);

                if (typeGrid[y][x] == LiquidType::WATER) {
                    if (waterTex.id != 0) {
                        // Check block above for surface logic
                        bool isSurface = (y - 1 < 0) || (!tileMap->isSolid(x, y - 1) && !hasLiquid[y - 1][x]);
                        bool isBottom = (y + 1 >= height) || tileMap->isSolid(x, y + 1);

                        float sourceX = 16.0f; // default to col 1 (Water)

                        if (isSurface) {
                            sourceX = 3.0f * 16.0f; // col 3 (Water Top)
                        } else if (isBottom) {
                            // Pseudo-random pick between 4 bottom textures (e.g., 0, 2, 4, 5)
                            int cols[] = {0, 2, 4, 5};
                            int pseudoRandom = (x * 37 + y * 13) % 4;
                            sourceX = cols[pseudoRandom] * 16.0f;
                        } else {
                            sourceX = 1.0f * 16.0f; // col 1 (Water Middle)
                        }
                        
                        Rectangle source = {sourceX, 0.0f, 16.0f, 16.0f};
                        Rectangle dest = {(float)(x * tileSize), rectY, (float)tileSize, rectHeight};
                        Color tint = WHITE;
                        tint.a = 200; // Transparency
                        DrawTexturePro(waterTex, source, dest, {0,0}, 0.0f, tint);
                    } else {
                        DrawRectangleRec({(float)(x * tileSize), rectY, (float)tileSize, rectHeight}, {0, 100, 255, 120});
                    }
                } else if (typeGrid[y][x] == LiquidType::LAVA) {
                    DrawRectangleRec({(float)(x * tileSize), rectY, (float)tileSize, rectHeight}, {255, 80, 0, 160});
                }
            }
        }
    }
}

} // namespace Platformer
