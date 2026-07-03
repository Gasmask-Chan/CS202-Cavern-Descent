#include "LevelGenerator.h"
#include "rooms/ClosedRooms.h"
#include "rooms/LeftRightRooms.h"
#include "rooms/LeftRightDownRooms.h"
#include "rooms/LeftRightUpRooms.h"
#include "rooms/ShopRooms.h"
#include "rooms/AltarRoom.h"
#include "../entities/EntityFactory.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <queue>
#include <random>
#include <cstring>  // memcpy

namespace Platformer {

// ---------------------------------------------------------------------------
// generate()
// ---------------------------------------------------------------------------
GeneratedLevel LevelGenerator::generate(int floor, ZoneType zone) {
  GeneratedLevel level;

  int maxRetries = 10;
  bool isValid   = false;

  for (int attempt = 0; attempt < maxRetries; ++attempt) {
    generateMacroGrid();

    int mapW = MAP_ROOMS_X * ROOM_WIDTH;
    int mapH = MAP_ROOMS_Y * ROOM_HEIGHT;
    level.tileMap = std::make_unique<TileMap>(mapW, mapH, MAP_TILE_SIZE);

    tempEnemies.clear();
    tempItems.clear();
    tempTraps.clear();
    tempPlayerSpawn = Vector2{0, 0};
    tempExitPos     = Vector2{0, 0};

    // Reset entity limits (Spelunky-DS style)
    snakesLeft    = 4;
    batsLeft      = 4;
    spidersLeft   = 4;
    cavemenLeft   = 3;
    skeletonsLeft = 3;
    damselsLeft   = 1;
    spikesLeft    = 4;
    lastPlacement = 3;

    for (int gy = 0; gy < MAP_ROOMS_Y; ++gy) {
      for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
        RoomRole role = macroGrid[gy][gx];
        int v         = selectVariation(role);

        // Copy chosen variation into local stack buffers (spelunky-ds style)
        int tileGrid[ROOM_HEIGHT][ROOM_WIDTH];
        int npcGrid[ROOM_HEIGHT][ROOM_WIDTH];
        int lootGrid[ROOM_HEIGHT][ROOM_WIDTH];

        switch (role) {
          case RoomRole::TYPE_1:
            memcpy(tileGrid, left_right_rooms[v], sizeof(tileGrid));
            memcpy(npcGrid,  left_right_npcs[v],  sizeof(npcGrid));
            memcpy(lootGrid, left_right_loot[v],  sizeof(lootGrid));
            break;
          case RoomRole::TYPE_2:
          case RoomRole::TYPE_2_DROP_THROUGH:
            memcpy(tileGrid, left_right_down_rooms[v], sizeof(tileGrid));
            memcpy(npcGrid,  left_right_down_npcs[v],  sizeof(npcGrid));
            memcpy(lootGrid, left_right_down_loot[v],  sizeof(lootGrid));
            break;
          case RoomRole::TYPE_3:
            memcpy(tileGrid, left_right_up_rooms[v], sizeof(tileGrid));
            memcpy(npcGrid,  left_right_up_npcs[v],  sizeof(npcGrid));
            memcpy(lootGrid, left_right_up_loot[v],  sizeof(lootGrid));
            break;
          case RoomRole::TYPE_SHOP:
            // Shop has 2 variations (left/right) in spelunky-ds arrays
            memcpy(tileGrid, shops[v % 2], sizeof(tileGrid));
            memcpy(npcGrid,  shops_npcs[v % 2],  sizeof(npcGrid));
            memcpy(lootGrid, shops_loot[v % 2],  sizeof(lootGrid));
            break;
          case RoomRole::TYPE_ALTAR:
            // Altar room has 1 variation
            memcpy(tileGrid, altar_room[0], sizeof(tileGrid));
            memcpy(npcGrid,  altar_room_npc[0], sizeof(npcGrid));
            memset(lootGrid, 0, sizeof(lootGrid)); // No loot array for altar
            break;
          case RoomRole::TYPE_0:
          default:
            memcpy(tileGrid, closed_rooms[v], sizeof(tileGrid));
            memcpy(npcGrid,  closed_rooms_npcs[v], sizeof(npcGrid));
            memcpy(lootGrid, closed_rooms_loot[v], sizeof(lootGrid));
            break;
        }

        populateRoom(tileGrid, npcGrid, lootGrid, gx, gy, role, level.tileMap.get());
      }
    }

    // generateChunks(level.tileMap.get());
    // generateBorders(level.tileMap.get());

    // ---- Spawn and exit placement ----
    int spawnGx = startRoomX * ROOM_WIDTH + 1;
    int spawnGy = startRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;
    while (spawnGy > startRoomY * ROOM_HEIGHT && level.tileMap->isSolid(spawnGx, spawnGy))
      spawnGy--;
    if (level.tileMap->isSolid(spawnGx, spawnGy))
      spawnGy = startRoomY * ROOM_HEIGHT + 1;

    tempPlayerSpawn = Vector2{(float)(spawnGx * MAP_TILE_SIZE),
                              (float)(spawnGy * MAP_TILE_SIZE)};
    level.tileMap->setTile(spawnGx, spawnGy, TileType::ENTRANCE);

    int exitGx = exitRoomX * ROOM_WIDTH + ROOM_WIDTH - 2;
    int exitGy = exitRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;
    while (exitGy > exitRoomY * ROOM_HEIGHT && level.tileMap->isSolid(exitGx, exitGy))
      exitGy--;
    if (level.tileMap->isSolid(exitGx, exitGy))
      exitGy = exitRoomY * ROOM_HEIGHT + 1;

    tempExitPos = Vector2{(float)(exitGx * MAP_TILE_SIZE),
                          (float)(exitGy * MAP_TILE_SIZE)};
    level.tileMap->setTile(exitGx, exitGy, TileType::EXIT);

    Vector2i startGrid = {spawnGx, spawnGy};
    Vector2i exitGrid  = {exitGx,  exitGy};

    if (tempPlayerSpawn.x == 0 && tempPlayerSpawn.y == 0 &&
        tempExitPos.x == 0    && tempExitPos.y == 0) {
      isValid = true; // bypass for empty fallback
    } else {
      isValid = validateLevel(level.tileMap.get(), startGrid, exitGrid);
    }

    if (isValid || attempt == maxRetries - 1) {
      level.dynamicEntities = std::move(tempEnemies);
      level.items           = std::move(tempItems);
      level.traps           = std::move(tempTraps);
      level.playerSpawn     = tempPlayerSpawn;
      level.exitPos         = tempExitPos;
      level.difficulty      = getDifficultyConfig(floor);
      level.modifier        = rollFloorModifier(floor);
      return level;
    }
  }

  level.dynamicEntities = std::move(tempEnemies);
  level.items           = std::move(tempItems);
  level.traps           = std::move(tempTraps);
  level.playerSpawn     = tempPlayerSpawn;
  level.exitPos         = tempExitPos;
  level.difficulty      = getDifficultyConfig(floor);
  level.modifier        = rollFloorModifier(floor);
  return level;
}

// ---------------------------------------------------------------------------
// generateMacroGrid() — Spelunky path-carving algorithm
// ---------------------------------------------------------------------------
void LevelGenerator::generateMacroGrid() {
  // Reset all rooms to TYPE_0 (closed)
  for (int y = 0; y < MAP_ROOMS_Y; ++y)
    for (int x = 0; x < MAP_ROOMS_X; ++x)
      macroGrid[y][x] = RoomRole::TYPE_0;

  // Start at a random column in the top row (Spelunky-style)
  int currX = GetRandomValue(0, MAP_ROOMS_X - 1);
  int currY = 0;
  bool movingLeft = false;
  obtainNewDirection(currX, movingLeft);

  startRoomX = currX;
  startRoomY = currY;

  bool exitPlaced = false;

  // Mark entrance room as a horizontal pass-through
  macroGrid[currY][currX] = RoomRole::TYPE_1;

  while (currY < MAP_ROOMS_Y) {
    bool atLeftWall  = movingLeft  && (currX == 0);
    bool atRightWall = !movingLeft && (currX == MAP_ROOMS_X - 1);
    // Drop down: forced at walls, or 1-in-3 random chance
    bool forceDown = atLeftWall || atRightWall || (GetRandomValue(1, 3) == 3);

    if (!forceDown) {
      // Walk horizontally
      currX += movingLeft ? -1 : 1;

      bool isBottomRow = (currY == MAP_ROOMS_Y - 1);
      if (isBottomRow && !exitPlaced && GetRandomValue(0, 1) == 0) {
        macroGrid[currY][currX] = RoomRole::TYPE_2;
        exitRoomX = currX;
        exitRoomY = currY;
        exitPlaced = true;
      } else {
        macroGrid[currY][currX] = RoomRole::TYPE_1;
      }
    } else {
      // Drop down
      if (currY < MAP_ROOMS_Y - 1) {
        // Current room gets a hole in the floor
        if (macroGrid[currY][currX] == RoomRole::TYPE_3)
          macroGrid[currY][currX] = RoomRole::TYPE_2_DROP_THROUGH;
        else
          macroGrid[currY][currX] = RoomRole::TYPE_2;

        currY++;
        // Room below gets a hole in the ceiling
        macroGrid[currY][currX] = RoomRole::TYPE_3;

        if (currY == MAP_ROOMS_Y - 1 && !exitPlaced && GetRandomValue(0, 1) == 0) {
          exitRoomX = currX;
          exitRoomY = currY;
          exitPlaced = true;
        }

        obtainNewDirection(currX, movingLeft);
      } else {
        // Already at the bottom row — guarantee exit
        if (!exitPlaced) {
          exitRoomX = currX;
          exitRoomY = currY;
          exitPlaced = true;
        }
        break;
      }
    }
  }

  // Post-process: convert first reachable closed room to altar, then shop
  placeAltar();
  placeShop();
}

void LevelGenerator::obtainNewDirection(int currX, bool& movingLeft) {
  if (currX == 0)
    movingLeft = false;
  else if (currX == MAP_ROOMS_X - 1)
    movingLeft = true;
  else
    movingLeft = (GetRandomValue(0, 1) == 0);
}

void LevelGenerator::placeAltar() {
  for (int y = 0; y < MAP_ROOMS_Y; ++y)
    for (int x = 0; x < MAP_ROOMS_X; ++x)
      if (macroGrid[y][x] == RoomRole::TYPE_0) {
        macroGrid[y][x] = RoomRole::TYPE_ALTAR;
        return;
      }
}

void LevelGenerator::placeShop() {
  for (int y = 0; y < MAP_ROOMS_Y; ++y) {
    for (int x = 0; x < MAP_ROOMS_X; ++x) {
      if (macroGrid[y][x] != RoomRole::TYPE_0) continue;
      bool leftOpen  = (x > 0)             && macroGrid[y][x - 1] != RoomRole::TYPE_0;
      bool rightOpen = (x < MAP_ROOMS_X-1) && macroGrid[y][x + 1] != RoomRole::TYPE_0;
      if (leftOpen || rightOpen) {
        macroGrid[y][x] = RoomRole::TYPE_SHOP;
        return;
      }
    }
  }
}

// ---------------------------------------------------------------------------
// selectVariation() — picks a random variation index for the given role
// ---------------------------------------------------------------------------
int LevelGenerator::selectVariation(RoomRole role) const {
  int numVars = 6; // Most rooms have 6 variations
  switch (role) {
    case RoomRole::TYPE_SHOP:          numVars = 2; break;
    case RoomRole::TYPE_ALTAR:         numVars = 1; break;
    default:                           break;
  }
  return GetRandomValue(0, numVars - 1);
}

// ---------------------------------------------------------------------------
// populateRoom() — writes tile and entity grids into the TileMap
// ---------------------------------------------------------------------------
void LevelGenerator::populateRoom(const int tileGrid[ROOM_HEIGHT][ROOM_WIDTH],
                                  const int npcGrid[ROOM_HEIGHT][ROOM_WIDTH],
                                  const int lootGrid[ROOM_HEIGHT][ROOM_WIDTH],
                                  int gx, int gy, RoomRole role, TileMap* map) {
  for (int cy = 0; cy < ROOM_HEIGHT; ++cy) {
    for (int cx = 0; cx < ROOM_WIDTH; ++cx) {
      int tx = gx * ROOM_WIDTH  + cx;
      int ty = gy * ROOM_HEIGHT + cy;

      // TYPE_2_DROP_THROUGH: punch a hole in the ceiling to connect to the room above
      // For Spelunky-DS 10x10, usually x=4,5 is open
      if (role == RoomRole::TYPE_2_DROP_THROUGH) {
        if (cy < 2 && cx >= 4 && cx <= 5) {
          map->setTile(tx, ty, TileType::NOTHING);
          continue; // Override hole logic entirely
        }
      }

      float px = tx * MAP_TILE_SIZE;
      float py = ty * MAP_TILE_SIZE;

      // ---- Tile layer (Spelunky MapTileType translation) ----
      // Spelunky DS arrays contain direct integer values for MapTileType.
      int tileVal = tileGrid[cy][cx];
      map->setTile(tx, ty, static_cast<TileType>(tileVal));

      // ---- NPC layer (Spelunky entity translation) ----
      // Matches Spelunky-DS populate_cave_npcs() logic:
      // only spawn on r==1 (1-in-3 chance) with global limits
      int npc = npcGrid[cy][cx];
      if (npc > 0) {
        lastPlacement++;
        int r = GetRandomValue(0, 2); // 0,1,2 — spawn only when r==1

        switch (npc) {
          case 1: { // Snake
            if (snakesLeft > 0 && r == 1 && lastPlacement >= 2) {
              auto enemy = EntityFactory::createEnemy('S', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); snakesLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 2: { // Bat
            if (batsLeft > 0 && r == 1 && lastPlacement >= 2) {
              auto enemy = EntityFactory::createEnemy('B', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); batsLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 3: { // Spider
            if (spidersLeft > 0 && r == 1 && lastPlacement >= 2) {
              auto enemy = EntityFactory::createEnemy('P', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); spidersLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 4: { // Spikes
            if (spikesLeft > 0 && r == 1 && lastPlacement >= 2) {
              map->setTile(tx, ty, TileType::SPIKE_TRAP);
              auto trap = EntityFactory::createTrap('^', px, py);
              if (trap) { tempTraps.push_back(std::move(trap)); spikesLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 5: { // Caveman
            if (cavemenLeft > 0 && r == 1 && lastPlacement >= 2) {
              auto enemy = EntityFactory::createEnemy('C', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); cavemenLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 6: { // Damsel
            if (damselsLeft > 0) { // always spawn damsel if present
              auto enemy = EntityFactory::createEnemy('D', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); damselsLeft--; }
            }
            break;
          }
          case 7: { // Lamp (decoration — skip for now)
            break;
          }
          case 8: { // Shopkeeper — always spawn
            auto enemy = EntityFactory::createEnemy('K', px, py);
            if (enemy) tempEnemies.push_back(std::move(enemy));
            break;
          }
          case 9: { // ArrowTrap Left (NPC entity, tile already set by tile grid)
            auto trap = EntityFactory::createTrap('<', px, py);
            if (trap) tempTraps.push_back(std::move(trap));
            break;
          }
          case 10: { // ArrowTrap Right
            auto trap = EntityFactory::createTrap('>', px, py);
            if (trap) tempTraps.push_back(std::move(trap));
            break;
          }
          case 12: { // Shop Item — always spawn
            auto item = EntityFactory::createItem('$', px, py);
            if (item) tempItems.push_back(std::move(item));
            break;
          }
          case 13: { // Skeleton
            if (skeletonsLeft > 0 && r == 1 && lastPlacement >= 2) {
              auto enemy = EntityFactory::createEnemy('X', px, py);
              if (enemy) { tempEnemies.push_back(std::move(enemy)); skeletonsLeft--; lastPlacement = 0; }
            }
            break;
          }
          case 20: { // Golden Idol — always spawn
            auto item = EntityFactory::createItem('I', px, py);
            if (item) tempItems.push_back(std::move(item));
            break;
          }
          default: break;
        }
      }

      // ---- Loot layer (Spelunky loot translation) ----
      int loot = lootGrid[cy][cx];
      if (loot > 0 && GetRandomValue(1, 100) <= 20) {
        char itemCode = 0;
        switch (loot) {
          case 1: itemCode = 'G'; break; // Goldbars
          case 2: itemCode = 'R'; break; // Rubies
          case 3: itemCode = 'J'; break; // Jar
          case 4: itemCode = 'C'; break; // Crate
          case 5: itemCode = 'L'; break; // Locked Chest
          case 6: itemCode = 'Y'; break; // Key
          default: break;
        }
        if (itemCode != 0) {
          auto item = EntityFactory::createItem(itemCode, px, py);
          if (item) tempItems.push_back(std::move(item));
        }
      }
    }
  }
}

// (Removed generateChunks and generateBorders as they are no longer used for Spelunky DS native rendering)

// ---------------------------------------------------------------------------
// validateLevel() + bfsReachability()
// ---------------------------------------------------------------------------
bool LevelGenerator::validateLevel(TileMap* map, Vector2i start, Vector2i exit) {
  return bfsReachability(map, start, exit);
}

bool LevelGenerator::bfsReachability(TileMap* map, Vector2i from, Vector2i to) {
  int width  = map->getWidth();
  int height = map->getHeight();

  std::vector<std::vector<int>> visited(height, std::vector<int>(width, -1));

  struct State { int x, y, jumpEnergy; };
  std::queue<State> q;
  q.push({from.x, from.y, 0});
  visited[from.y][from.x] = 0;

  auto pushState = [&](int nx, int ny, int nEnergy) {
    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
      if (!map->isSolid(nx, ny) && visited[ny][nx] < nEnergy) {
        visited[ny][nx] = nEnergy;
        q.push({nx, ny, nEnergy});
      }
    }
  };

  while (!q.empty()) {
    State curr = q.front(); q.pop();
    if (curr.x == to.x && curr.y == to.y) return true;

    bool onGround = (curr.y + 1 < height) && map->isSolid(curr.x, curr.y + 1);
    bool onLadder = (map->getTile(curr.x, curr.y) == TileType::LADDER);

    if (onGround || onLadder) {
      pushState(curr.x - 1, curr.y, 4);
      pushState(curr.x + 1, curr.y, 4);
      pushState(curr.x, curr.y - 1, 3);
      if (onLadder) pushState(curr.x, curr.y + 1, 0);
    } else {
      pushState(curr.x, curr.y + 1, 0);
      pushState(curr.x - 1, curr.y, 0);
      pushState(curr.x + 1, curr.y, 0);
      if (curr.jumpEnergy > 0) {
        pushState(curr.x, curr.y - 1, curr.jumpEnergy - 1);
        pushState(curr.x - 1, curr.y - 1, curr.jumpEnergy - 1);
        pushState(curr.x + 1, curr.y - 1, curr.jumpEnergy - 1);
      }
    }
  }
  return false;
}

// ---------------------------------------------------------------------------
// getDifficultyConfig() + rollFloorModifier()
// ---------------------------------------------------------------------------
DifficultyConfig LevelGenerator::getDifficultyConfig(int floor) {
  DifficultyConfig cfg;
  cfg.maxEnemiesPerRoom         = 3 + floor / 2;
  cfg.trapDensity               = 0.1f + (floor * 0.05f);
  cfg.treasureValueMultiplier   = 1;
  cfg.enemySpeedScale           = 1.0f + (floor * 0.1f);
  cfg.ghostTimerSeconds         = std::max(60.0f, 180.0f - floor * 10.0f);
  cfg.liquidProbability         = 0.2f;
  return cfg;
}

FloorModifier LevelGenerator::rollFloorModifier(int floor) {
  if (floor == 1) return FloorModifier::NONE;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine rng(seed);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  if (dist(rng) < 0.3f) {
    std::uniform_int_distribution<int> modDist(1, 3);
    int mod = modDist(rng);
    if (mod == 1) return FloorModifier::DARK_FLOOR;
    if (mod == 2) return FloorModifier::FLOODED_FLOOR;
    if (mod == 3) return FloorModifier::CURSED_FLOOR;
  }
  return FloorModifier::NONE;
}

} // namespace Platformer
