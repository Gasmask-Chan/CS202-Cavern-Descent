#include "TileMap.h"
#include "../core/EventBus.h"
#include <cmath>
#include <iostream>
#include "rlgl.h"

namespace Platformer {

TileMap::TileMap(int w, int h, int size) : width(w), height(h), tileSize(size) {
  tiles.assign(height, std::vector<TileType>(width, TileType::NOTHING));
  dsTileset = LoadTexture("assets/tilemaps/gfx_cavebg.png");
}

TileMap::~TileMap() { 
  UnloadTexture(dsTileset); 
}

TileType TileMap::getTile(int x, int y) const {
  if (!isInBounds(x, y))
    return TileType::CAVE_ROCK; // Treat out-of-bounds as solid dirt/rock
  return tiles[y][x];
}

void TileMap::setTile(int x, int y, TileType type) {
  if (x >= 0 && x < width && y >= 0 && y < height) {
    tiles[y][x] = type;
  }
}

void TileMap::destroyBlock(int x, int y) {
  // Whip already checks isCracked, Bomb destroys indiscriminately.
  setTile(x, y, TileType::NOTHING);
  EventData data;
  data.gridX = x;
  data.gridY = y;
  EventBus::getInstance()->publish(EventType::EVENT_TERRAIN_DESTROYED, data);
  // TODO: spawn rubble particle effects here in the future
}

bool TileMap::isSolid(int x, int y) const {
  TileType type = getTile(x, y);
  if ((type >= TileType::CAVE_ROCK && type <= TileType::CAVE_UP_DOWN_ORIENTED) || 
      type == TileType::CAVE_SMOOTH || type == TileType::STONE_BLOCK || type == TileType::SPIKE_TRAP || type == TileType::ARROW_TRAP_LEFT || type == TileType::ARROW_TRAP_RIGHT) {
      return true; 
  }
  return false;
}

bool TileMap::isOneWayPlatform(int x, int y) const {
    TileType type = getTile(x, y);
    return type == TileType::LADDER_DECK;
}

bool TileMap::isOpaque(int x, int y) const {
  TileType type = getTile(x, y);
  // Opaque blocks light
  if ((type >= TileType::CAVE_ROCK && type <= TileType::CAVE_UP_DOWN_ORIENTED && type != TileType::CAVE_DOWN_ORIENTED) || 
      type == TileType::CAVE_SMOOTH || type == TileType::ARROW_TRAP_LEFT || type == TileType::ARROW_TRAP_RIGHT) {
      return true; 
  }
  return false;
}

bool TileMap::isCracked(int x, int y) const {
  return getTile(x, y) == TileType::STONE_BLOCK;
}

bool TileMap::isLadder(int x, int y) const {
  TileType type = getTile(x, y);
  return type == TileType::LADDER || type == TileType::LADDER_DECK;
}

void TileMap::render(Camera2D &cam, const std::vector<std::vector<float>>& lightMap, bool foregroundPass) {
  Vector2 screenTL = GetScreenToWorld2D(Vector2{0, 0}, cam);
  Vector2 screenBR = GetScreenToWorld2D(Vector2{(float)GetScreenWidth(), (float)GetScreenHeight()}, cam);

  int startX = (int)(screenTL.x / tileSize) - 1;
  int startY = (int)(screenTL.y / tileSize) - 1;
  int endX = (int)(screenBR.x / tileSize) + 1;
  int endY = (int)(screenBR.y / tileSize) + 1;

  for (int y = startY; y <= endY; ++y) {
    for (int x = startX; x <= endX; ++x) {
      TileType type = getTile(x, y);

      auto getVertexLight = [&](int vx, int vy) -> float {
        float sum = 0.0f;
        for (int dy = -1; dy <= 0; dy++) {
            for (int dx = -1; dx <= 0; dx++) {
                int tx = vx + dx;
                int ty = vy + dy;
                if (ty >= 0 && static_cast<size_t>(ty) < lightMap.size() && tx >= 0 && static_cast<size_t>(tx) < lightMap[ty].size()) {
                    sum += lightMap[ty][tx];
                } else {
                    sum += 0.15f; // Ambient light for out-of-bounds tiles
                }
            }
        }
        return sum / 4.0f;
      };

      float lTL = getVertexLight(x, y);
      float lTR = getVertexLight(x + 1, y);
      float lBL = getVertexLight(x, y + 1);
      float lBR = getVertexLight(x + 1, y + 1);
      
      Color cTL = Color{(unsigned char)(255 * lTL), (unsigned char)(255 * lTL), (unsigned char)(255 * lTL), 255};
      Color cTR = Color{(unsigned char)(255 * lTR), (unsigned char)(255 * lTR), (unsigned char)(255 * lTR), 255};
      Color cBL = Color{(unsigned char)(255 * lBL), (unsigned char)(255 * lBL), (unsigned char)(255 * lBL), 255};
      Color cBR = Color{(unsigned char)(255 * lBR), (unsigned char)(255 * lBR), (unsigned char)(255 * lBR), 255};

      Rectangle dest = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };

      if (dsTileset.id != 0) {
        // Skip rendering foreground for invisible/special tiles (Entrance and Exit ARE in gfx_cavebg.png so we render them)
        if (type == TileType::NOTHING || type == TileType::ROPE_NODE || type == TileType::SPIKE_TRAP) {
            continue; 
        }

        bool isBackgroundTile = (type == TileType::ENTRANCE || type == TileType::EXIT || type == TileType::LADDER || type == TileType::LADDER_DECK);
        
        if (foregroundPass && isBackgroundTile) continue;
        if (!foregroundPass && !isBackgroundTile) continue;

        int dsIndex = static_cast<int>(type);
        if (dsIndex > 0 && dsIndex <= 42) {
            int logical_index = dsIndex - 1;
            int pixel_x = (logical_index % 2) * 16;
            int pixel_y = (logical_index / 2) * 16;
            Rectangle dsSrc = { (float)pixel_x, (float)pixel_y, 16.0f, 16.0f };
            
            rlSetTexture(dsTileset.id);
            rlBegin(RL_QUADS);
                rlColor4ub(cTL.r, cTL.g, cTL.b, cTL.a);
                rlTexCoord2f(dsSrc.x / dsTileset.width, dsSrc.y / dsTileset.height);
                rlVertex2f(dest.x, dest.y);

                rlColor4ub(cBL.r, cBL.g, cBL.b, cBL.a);
                rlTexCoord2f(dsSrc.x / dsTileset.width, (dsSrc.y + dsSrc.height) / dsTileset.height);
                rlVertex2f(dest.x, dest.y + dest.height);

                rlColor4ub(cBR.r, cBR.g, cBR.b, cBR.a);
                rlTexCoord2f((dsSrc.x + dsSrc.width) / dsTileset.width, (dsSrc.y + dsSrc.height) / dsTileset.height);
                rlVertex2f(dest.x + dest.width, dest.y + dest.height);

                rlColor4ub(cTR.r, cTR.g, cTR.b, cTR.a);
                rlTexCoord2f((dsSrc.x + dsSrc.width) / dsTileset.width, dsSrc.y / dsTileset.height);
                rlVertex2f(dest.x + dest.width, dest.y);
            rlEnd();
            rlSetTexture(0);
        }
      }
    }
  }
}

void TileMap::renderParallaxBackground(Camera2D &cam) {
    if (dsTileset.id == 0) return;
    
    // In Spelunky DS, the background is stored in gfx_cavebg.png from row 21 onwards.
    // It's a 32x128 pixel block starting at Y = 336 (21 * 16).
    Rectangle bgSrc = { 0.0f, 336.0f, 32.0f, 128.0f };
    
    // Scale by 2 to match our game's scale (tileSize 32 vs original 16)
    float bgWidth = 64.0f;
    float bgHeight = 256.0f;
    
    // Darken the background layer significantly to push it to the background
    // and make the foreground tiles pop out.
    Color bgTint = Color{ 80, 80, 80, 255 }; 

    float parallaxFactor = 0.5f;
    Vector2 screenTL = GetScreenToWorld2D(Vector2{0, 0}, cam);
    
    float offsetX = fmod(screenTL.x * parallaxFactor, bgWidth);
    float offsetY = fmod(screenTL.y * parallaxFactor, bgHeight);
    
    // Handle negative offsets from fmod properly
    if (offsetX < 0) offsetX += bgWidth;
    if (offsetY < 0) offsetY += bgHeight;
    
    float startX = screenTL.x - offsetX;
    float startY = screenTL.y - offsetY;
    
    float endX = screenTL.x + GetScreenWidth() + bgWidth;
    float endY = screenTL.y + GetScreenHeight() + bgHeight;
    
    for (float y = startY - bgHeight; y < endY; y += bgHeight) {
        for (float x = startX - bgWidth; x < endX; x += bgWidth) {
            Rectangle dest = { x, y, bgWidth, bgHeight };
            DrawTexturePro(dsTileset, bgSrc, dest, Vector2{0, 0}, 0.0f, bgTint);
        }
    }
}

Vector2i TileMap::worldToGrid(float wx, float wy) {
  return Vector2i{(int)(wx / tileSize), (int)(wy / tileSize)};
}

Vector2 TileMap::gridToWorld(int gx, int gy) {
  return Vector2{(float)(gx * tileSize), (float)(gy * tileSize)};
}

int TileMap::getWidth() { return width; }

int TileMap::getHeight() { return height; }

int TileMap::getTileSize() { return tileSize; }

bool TileMap::isInBounds(int x, int y) const {
  return x >= 0 && x < width && y >= 0 && y < height;
}

}