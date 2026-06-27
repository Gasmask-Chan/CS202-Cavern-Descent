#include "TileMap.h"

namespace Platformer {

TileMap::TileMap(int w, int h, int size) : width(w), height(h), tileSize(size) {
  tiles.assign(height, std::vector<TileType>(width, TileType::EMPTY));
  chunks.assign(height, std::vector<ChunkInfo>(width, ChunkInfo{}));
  tileset = LoadTexture("assets/textures/tileset.png");
}

TileMap::~TileMap() { UnloadTexture(tileset); }

TileType TileMap::getTile(int x, int y) const {
  if (!isInBounds(x, y))
    return TileType::WALL;
  return tiles[y][x];
}

void TileMap::setTile(int x, int y, TileType type) {
  if (isInBounds(x, y)) {
    tiles[y][x] = type;
  }
}

ChunkInfo TileMap::getChunk(int x, int y) const {
  if (!isInBounds(x, y)) return ChunkInfo{};
  return chunks[y][x];
}

void TileMap::setChunk(int x, int y, const ChunkInfo& c) {
  if (isInBounds(x, y)) {
    chunks[y][x] = c;
  }
}

bool TileMap::isSolid(int x, int y) const {
  TileType type = getTile(x, y);
  // Note: PLATFORM is NOT solid because players can jump up through it.
  return type == TileType::WALL || type == TileType::CRACKED;
}

bool TileMap::isOpaque(int x, int y) const {
  TileType type = getTile(x, y);
  return type == TileType::WALL || type == TileType::CRACKED;
}

bool TileMap::isCracked(int x, int y) const {
  return getTile(x, y) == TileType::CRACKED;
}

void TileMap::render(Camera2D &cam, const std::vector<std::vector<float>>& lightMap) {
  auto applyLight = [](Color c, float l) {
      return Color{(unsigned char)(c.r * l), (unsigned char)(c.g * l), (unsigned char)(c.b * l), 255};
  };

  Vector2 screenTL = GetScreenToWorld2D(Vector2{0, 0}, cam);
  Vector2 screenBR = GetScreenToWorld2D(Vector2{(float)GetScreenWidth(), (float)GetScreenHeight()}, cam);

  int startX = std::max(0, (int)(screenTL.x / tileSize) - 1);
  int startY = std::max(0, (int)(screenTL.y / tileSize) - 1);
  int endX = std::min(width - 1, (int)(screenBR.x / tileSize) + 1);
  int endY = std::min(height - 1, (int)(screenBR.y / tileSize) + 1);

  // PASS 1: Render Base Terrain Chunks
  for (int y = startY; y <= endY; ++y) {
    for (int x = startX; x <= endX; ++x) {
      TileType type = getTile(x, y);
      float light = 1.0f;
      if (static_cast<size_t>(y) < lightMap.size() && static_cast<size_t>(x) < lightMap[y].size()) {
        light = lightMap[y][x];
      }
      
      if (type == TileType::WALL) {
        ChunkInfo chunk = getChunk(x, y);
        if (!chunk.isOrigin) continue; // Only draw chunks from their top-left origin
        
        Rectangle src = { (float)chunk.offsetX, (float)chunk.offsetY, 
                          (float)(chunk.width * 64), (float)(chunk.height * 64) };
        Rectangle dest = { (float)x * tileSize, (float)y * tileSize, (float)(chunk.width * tileSize), (float)(chunk.height * tileSize) };
        Color tint = Color{255, 255, 255, (unsigned char)(255 * light)};
        
        if (tileset.id != 0) {
            DrawTexturePro(tileset, src, dest, Vector2{0, 0}, 0.0f, tint);
        } else {
            DrawRectangleRec(dest, applyLight(GRAY, light));
        }
      }
    }
  }

  // PASS 2: Render Decals (Borders) per 1x1 tile & Other Entities
  for (int y = startY; y <= endY; ++y) {
    for (int x = startX; x <= endX; ++x) {
      TileType type = getTile(x, y);
      float light = 1.0f;
      if (static_cast<size_t>(y) < lightMap.size() && static_cast<size_t>(x) < lightMap[y].size()) {
        light = lightMap[y][x];
      }
      
      Color tint = Color{255, 255, 255, (unsigned char)(255 * light)};
      Rectangle dest = { (float)x * tileSize, (float)y * tileSize, (float)tileSize, (float)tileSize };

      if (type == TileType::WALL && tileset.id != 0) {
        unsigned int hash = (unsigned int)(x * 73856093 ^ y * 19349663);

        bool top = (y == 0 || getTile(x, y - 1) != TileType::WALL);
        bool bottom = (y == height - 1 || getTile(x, y + 1) != TileType::WALL);
        bool left = (x == 0 || getTile(x - 1, y) != TileType::WALL);
        bool right = (x == width - 1 || getTile(x + 1, y) != TileType::WALL);

        if (top) {
            unsigned int idx = hash % 3;
            Rectangle decalSrc = {320.0f + idx * 64.0f, 0.0f, 64.0f, 64.0f}; 
            Rectangle decalDest = dest;
            decalDest.y -= tileSize / 2.0f;
            DrawTexturePro(tileset, decalSrc, decalDest, Vector2{0, 0}, 0.0f, tint);
        }
        if (right) {
            Rectangle decalSrc = {448.0f, 64.0f, 64.0f, 64.0f}; 
            Rectangle decalDest = dest;
            decalDest.x += tileSize / 2.0f;
            DrawTexturePro(tileset, decalSrc, decalDest, Vector2{0, 0}, 0.0f, tint);
        }
        if (bottom) {
            unsigned int idx = (hash >> 4) % 2;
            Rectangle decalSrc = {320.0f + idx * 64.0f, 64.0f, 64.0f, 64.0f}; 
            Rectangle decalDest = dest;
            decalDest.y += tileSize / 2.0f;
            DrawTexturePro(tileset, decalSrc, decalDest, Vector2{0, 0}, 0.0f, tint);
        }
        if (left) {
            Rectangle decalSrc = {448.0f, 128.0f, 64.0f, 64.0f}; 
            Rectangle decalDest = dest;
            decalDest.x -= tileSize / 2.0f;
            DrawTexturePro(tileset, decalSrc, decalDest, Vector2{0, 0}, 0.0f, tint);
        }
      } else if (type != TileType::WALL && type != TileType::EMPTY) {
        bool drawTexture = false;
        Rectangle src = {0, 0, 64, 64};
        
        if (type == TileType::CRACKED) {
            src = {64, 0, 64, 64};
            drawTexture = true;
        } else if (type == TileType::PLATFORM) {
            src = {192, 0, 64, 64}; // Ladder top
            drawTexture = true;
        } else if (type == TileType::LADDER) {
            src = {128, 0, 64, 64}; // Ladder
            drawTexture = true;
        } else if (type == TileType::SPIKE_TRAP) {
            src = {320, 448, 64, 64};
            drawTexture = true;
        } else if (type == TileType::ROPE_NODE) {
            src = {128, 0, 64, 64};
            drawTexture = true;
        } else if (type == TileType::EXIT_DOOR) {
            src = {448, 448, 64, 64};
            drawTexture = true;
        }

        if (drawTexture && tileset.id != 0) {
            DrawTexturePro(tileset, src, dest, Vector2{0, 0}, 0.0f, tint);
        } else if (!drawTexture) {
            DrawRectangleRec(dest, applyLight(MAGENTA, light)); // Fallback missing texture
        }
      }
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