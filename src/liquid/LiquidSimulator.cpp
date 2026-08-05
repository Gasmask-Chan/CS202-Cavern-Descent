#include "LiquidSimulator.h"
#include <algorithm>
#include <iostream>

namespace Platformer {

LiquidSimulator::LiquidSimulator(TileMap *map)
    : tileMap(map), isWaterDirty(false) {
  width = map->getWidth();
  height = map->getHeight();
  hasLiquid.resize(height, std::vector<bool>(width, false));
  typeGrid.resize(height, std::vector<LiquidType>(width, LiquidType::NONE));
  isSpurtBlock.resize(height, std::vector<bool>(width, false));
  spurtTimer.resize(height, std::vector<float>(width, 0.0f));
  checkLiquid = false;

  EventBus::getInstance()->subscribe(
      EventType::EVENT_TERRAIN_DESTROYED,
      [this](EventData data) { this->onTerrainDestroyed(data); });

  auto loadChroma = [](const char *path) -> Texture2D {
    Image img = LoadImage(path);
    if (img.data != nullptr) {
      ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
      Color chroma = GetImageColor(img, 0, 0); // top-left pixel color
      ImageColorReplace(&img, chroma, BLANK);
      Texture2D tex = LoadTextureFromImage(img);
      UnloadImage(img);
      return tex;
    }
    return Texture2D{0};
  };

  waterTex = LoadTexture("assets/sprites/16x16/water.png");
  lavaTex = LoadTexture(
      "assets/sprites/lava/Lava.png"); // No chroma key for the solid lava body!
  lavaTopTex = loadChroma("assets/sprites/lava/LavaTop.png");

  // Fix blurry lines by enforcing point filtering
  SetTextureFilter(waterTex, TEXTURE_FILTER_POINT);
  SetTextureFilter(lavaTex, TEXTURE_FILTER_POINT);
  SetTextureFilter(lavaTopTex, TEXTURE_FILTER_POINT);
}

LiquidSimulator::~LiquidSimulator() {
  UnloadTexture(waterTex);
  UnloadTexture(lavaTex);
  UnloadTexture(lavaTopTex);
}

static float liquidUpdateTimer = 0.0f;

void LiquidSimulator::update(float dt) {
  if (!checkLiquid)
    return;

  liquidUpdateTimer += dt;
  // Run exactly at 60 FPS (every frame) but we can gate it if we want.
  // For cascade drainage, frame-by-frame is good.
  // Let's use the frame delta to limit extremely fast loops if dt is variable,
  // but in a fixed step engine, just running every frame is fine.

  int blocksDestroyed = 0;
  std::vector<std::pair<int, int>> toDestroy;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (hasLiquid[y][x]) {
        // Check 3 spots: Left, Right, Bottom
        bool leftEmpty =
            (x - 1 >= 0) && !tileMap->isSolid(x - 1, y) && !hasLiquid[y][x - 1];
        bool rightEmpty = (x + 1 < width) && !tileMap->isSolid(x + 1, y) &&
                          !hasLiquid[y][x + 1];
        bool bottomEmpty = (y + 1 < height) && !tileMap->isSolid(x, y + 1) &&
                           !hasLiquid[y + 1][x];

        if (leftEmpty || rightEmpty || bottomEmpty) {
          toDestroy.push_back({x, y});
        }
      }
    }
  }

  for (auto &pos : toDestroy) {
    int x = pos.first;
    int y = pos.second;

    hasLiquid[y][x] = false;
    isSpurtBlock[y][x] = false;

    // SPAWN LAVA DRIP when lava drains
    if (typeGrid[y][x] == LiquidType::LAVA && GetRandomValue(1, 100) <= 35) {
      EventData dripData;
      dripData.worldX = x * tileMap->getTileSize() + GetRandomValue(4, 28);
      dripData.worldY = y * tileMap->getTileSize() + GetRandomValue(4, 28);
      EventBus::getInstance()->publish(EventType::EVENT_SPAWN_LAVA_DRIP,
                                       dripData);
    }

    typeGrid[y][x] = LiquidType::NONE;
    blocksDestroyed++;
    isWaterDirty = true;
  }

  if (blocksDestroyed == 0) {
    checkLiquid = false;
  }
}

void LiquidSimulator::addLiquid(int gx, int gy, uint8_t amount,
                                LiquidType type) {
  if (!tileMap->isInBounds(gx, gy))
    return;

  // Stack upwards if the cell is already occupied by liquid
  while (gy >= 0 && hasLiquid[gy][gx]) {
    // If the block above is solid, we can't stack anymore (volume is
    // lost/compressed)
    if (gy - 1 >= 0 && tileMap->isSolid(gx, gy - 1)) {
      return;
    }
    gy--;
  }

  if (gy < 0 || tileMap->isSolid(gx, gy))
    return;

  hasLiquid[gy][gx] = true;
  typeGrid[gy][gx] = type;
  if (type == LiquidType::LAVA && GetRandomValue(1, 4) == 1) {
    isSpurtBlock[gy][gx] = true;
    spurtTimer[gy][gx] = (float)GetRandomValue(60, 180) / 60.0f;
  }
  isWaterDirty = true;
  checkLiquid = true;
}

void LiquidSimulator::removeLiquid(int gx, int gy) {
  if (!tileMap->isInBounds(gx, gy))
    return;
  hasLiquid[gy][gx] = false;
  typeGrid[gy][gx] = LiquidType::NONE;
}

bool LiquidSimulator::hasLiquidAt(int gx, int gy) const {
  if (!tileMap->isInBounds(gx, gy))
    return false;
  return hasLiquid[gy][gx];
}

bool LiquidSimulator::isWaterAt(Rectangle rect) const {
  int tileSize = tileMap->getTileSize();
  int startX = std::max(0, (int)(rect.x / tileSize));
  int startY = std::max(0, (int)(rect.y / tileSize));
  int endX =
      std::min(width - 1, (int)((rect.x + rect.width - 0.1f) / tileSize));
  int endY =
      std::min(height - 1, (int)((rect.y + rect.height - 0.1f) / tileSize));

  for (int y = startY; y <= endY; y++) {
    for (int x = startX; x <= endX; x++) {
      if (hasLiquid[y][x] && typeGrid[y][x] == LiquidType::WATER)
        return true;
    }
  }
  return false;
}

bool LiquidSimulator::isLavaAt(Rectangle rect) const {
  int tileSize = tileMap->getTileSize();
  int startX = std::max(0, (int)(rect.x / tileSize));
  int startY = std::max(0, (int)(rect.y / tileSize));
  int endX =
      std::min(width - 1, (int)((rect.x + rect.width - 0.1f) / tileSize));
  int endY =
      std::min(height - 1, (int)((rect.y + rect.height - 0.1f) / tileSize));

  for (int y = startY; y <= endY; y++) {
    for (int x = startX; x <= endX; x++) {
      if (hasLiquid[y][x] && typeGrid[y][x] == LiquidType::LAVA)
        return true;
    }
  }
  return false;
}

void LiquidSimulator::updateSpurts(float dt, float playerX, float playerY) {
  int tileSize = tileMap->getTileSize();
  float maxDistSq = 240.0f * 240.0f;

  int startX = std::max(0, (int)((playerX - 240.0f) / tileSize));
  int endX = std::min(width - 1, (int)((playerX + 240.0f) / tileSize));
  int startY = std::max(0, (int)((playerY - 240.0f) / tileSize));
  int endY = std::min(height - 1, (int)((playerY + 240.0f) / tileSize));

  for (int y = startY; y <= endY; y++) {
    for (int x = startX; x <= endX; x++) {
      if (hasLiquid[y][x] && typeGrid[y][x] == LiquidType::LAVA &&
          isSpurtBlock[y][x]) {
        // Check if it's a surface cell (no lava above it)
        if (y == 0 || !hasLiquid[y - 1][x]) {

          // SPAWN LAVA DRIP randomly on the surface
          if (GetRandomValue(1, 1000) <= 5) {
            EventData dripData;
            dripData.worldX = x * tileSize + GetRandomValue(4, 28);
            dripData.worldY = y * tileSize;
            EventBus::getInstance()->publish(EventType::EVENT_SPAWN_LAVA_DRIP,
                                             dripData);
          }

          float cellCX = x * tileSize + tileSize / 2.0f;
          float cellCY = y * tileSize + tileSize / 2.0f;
          float dx = playerX - cellCX;
          float dy = playerY - cellCY;

          if (dx * dx + dy * dy <= maxDistSq) {
            spurtTimer[y][x] -= dt;
            if (spurtTimer[y][x] <= 0.0f) {
              EventData data;
              data.worldX = cellCX - 8.0f; // Center horizontally
              data.worldY = y * tileSize;
              data.vy =
                  -100.0f * (float)GetRandomValue(
                                3, 6); // -300 to -600 (simulate -rand(1,4))

              // Publish event
              EventBus::getInstance()->publish(EventType::EVENT_SPAWN_FLAME,
                                               data);

              // Reset timer
              spurtTimer[y][x] = (float)GetRandomValue(60, 180) / 60.0f;
            }
          }
        }
      }
    }
  }
}

void LiquidSimulator::applyFloodedFloorModifier(int bottomRows,
                                                LiquidType type) {
  for (int y = height - 1; y >= std::max(0, height - bottomRows); y--) {
    for (int x = 0; x < width; x++) {
      if (!tileMap->isSolid(x, y)) {
        hasLiquid[y][x] = true;
        typeGrid[y][x] = type;
      }
    }
  }
}

void LiquidSimulator::onTerrainDestroyed(EventData data) {
  isWaterDirty = true;
  checkLiquid = true;
}

void LiquidSimulator::render(Camera2D &cam) {
  int tileSize = tileMap->getTileSize();

  int screenW = GetScreenWidth();
  if (screenW == 0)
    screenW = 800; // fallback
  int screenH = GetScreenHeight();
  if (screenH == 0)
    screenH = 600; // fallback

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
            bool isSurface = (y - 1 < 0) || (!tileMap->isSolid(x, y - 1) &&
                                             !hasLiquid[y - 1][x]);
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

            // Determine animation frame based on time and position
            float time = GetTime();
            int frame = (int)(time * 6.0f + (x * 0.2f)) % 4;
            float sourceY = isSurface ? (frame * 16.0f) : 0.0f;

            Rectangle source = {sourceX, sourceY, 16.0f, 16.0f};
            Rectangle dest = {(float)x * tileSize, rectY, (float)tileSize,
                              rectHeight};

            DrawTexturePro(waterTex, source, dest, {0, 0}, 0.0f,
                           Color{255, 255, 255, 100});
          } else {
            DrawRectangleRec(
                {(float)(x * tileSize), rectY, (float)tileSize, rectHeight},
                {0, 100, 255, 120});
          }
        } else if (typeGrid[y][x] == LiquidType::LAVA) {
          bool isSurface = (y - 1 < 0) || (!tileMap->isSolid(x, y - 1) &&
                                           !hasLiquid[y - 1][x]);

          float time = GetTime();
          int frame = (int)(time * 6.0f + (x * 0.2f)) % 4; // basic animation

          if (isSurface && lavaTopTex.id != 0) {
            float sourceY = frame * 17.0f; // LavaTop is 16x119 (17px stride)
            Rectangle source = {0.0f, sourceY, 16.0f, 16.0f};
            Rectangle dest = {x * (float)tileSize, rectY, (float)tileSize,
                              rectHeight};
            DrawTexturePro(lavaTopTex, source, dest, {0, 0}, 0.0f,
                           Color{255, 255, 255, 120});
          } else if (lavaTex.id != 0) {
            Rectangle source = {0.0f, 0.0f, 16.0f, 16.0f};
            Rectangle dest = {x * (float)tileSize, rectY, (float)tileSize,
                              rectHeight};
            DrawTexturePro(lavaTex, source, dest, {0, 0}, 0.0f,
                           Color{255, 255, 255, 120});
          } else {
            DrawRectangleRec(
                {(float)(x * tileSize), rectY, (float)tileSize, rectHeight},
                {255, 60, 0, 150});
          }
        }
      }
    }
  }
}

} // namespace Platformer
