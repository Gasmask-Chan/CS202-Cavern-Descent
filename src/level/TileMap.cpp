#include "TileMap.h"

namespace Platformer {

TileMap::TileMap(int w, int h, int size) : width(w), height(h), tileSize(size) {
    tiles.assign(height, std::vector<TileType>(width, TileType::EMPTY));
}

TileType TileMap::getTile(int x, int y) {
    if (!isInBounds(x, y)) return TileType::WALL;
    return tiles[y][x];
}

void TileMap::setTile(int x, int y, TileType type) {
    if (isInBounds(x, y)) {
        tiles[y][x] = type;
    }
}

bool TileMap::isSolid(int x, int y) {
    TileType type = getTile(x, y);
    return type == TileType::WALL || type == TileType::CRACKED || type == TileType::PLATFORM;
}

bool TileMap::isOpaque(int x, int y) {
    TileType type = getTile(x, y);
    return type == TileType::WALL || type == TileType::CRACKED;
}

bool TileMap::isCracked(int x, int y) {
    return getTile(x, y) == TileType::CRACKED;
}

void TileMap::render(Camera2D &cam, std::vector<std::vector<float>> lightMap) {
    // For now, render solid color rectangles based on tile type.
    // Later this will be replaced with sprite rendering (from Spelunky assets).
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            TileType type = tiles[y][x];
            if (type == TileType::EMPTY) continue;

            float light = 1.0f;
            if (y < lightMap.size() && x < lightMap[y].size()) {
                light = lightMap[y][x];
            }

            Color baseColor = WHITE;
            switch(type) {
                case TileType::WALL: baseColor = GRAY; break;
                case TileType::CRACKED: baseColor = DARKGRAY; break;
                case TileType::PLATFORM: baseColor = BROWN; break;
                case TileType::SPIKE_TRAP: baseColor = RED; break;
                case TileType::ARROW_TRAP: baseColor = MAROON; break;
                case TileType::BOULDER_TRAP: baseColor = ORANGE; break;
                case TileType::ROPE_NODE: baseColor = YELLOW; break;
                case TileType::EXIT_DOOR: baseColor = GOLD; break;
                default: baseColor = MAGENTA; break;
            }

            unsigned char r = (unsigned char)(baseColor.r * light);
            unsigned char g = (unsigned char)(baseColor.g * light);
            unsigned char b = (unsigned char)(baseColor.b * light);
            Color tintedColor = {r, g, b, baseColor.a};

            DrawRectangle(x * tileSize, y * tileSize, tileSize, tileSize, tintedColor);
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

bool TileMap::isInBounds(int x, int y) {
    return x >= 0 && x < width && y >= 0 && y < height;
}

}