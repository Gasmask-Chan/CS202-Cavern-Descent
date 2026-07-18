#include "LevelGenerator.h"
#include "rooms/ClosedRooms.h"
#include "rooms/EntranceRooms.h"
#include "rooms/LeftRightRooms.h"
#include "rooms/LeftRightDownRooms.h"
#include "rooms/LeftRightUpRooms.h"
#include "rooms/ShopRooms.h"
#include "../entities/EntityFactory.h"
#include "../entities/enemies/Enemy.h"
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

    // Step 2: Template Selection
    for (int gy = 0; gy < MAP_ROOMS_Y; ++gy) {
      for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
        RoomRole role = macroGrid[gy][gx];
        bool isEntrance = (gx == startRoomX && gy == startRoomY);
        roomVariations[gy][gx] = selectVariation(role, isEntrance);
      }
    }

    // Step 3: Tile Instantiation
    for (int gy = 0; gy < MAP_ROOMS_Y; ++gy) {
      for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
        RoomRole role = macroGrid[gy][gx];
        int v         = roomVariations[gy][gx];
        bool isEntrance = (gx == startRoomX && gy == startRoomY);

        // Procedurally turn some unused closed rooms (TYPE_0) into lake/lava rooms
        if (role == RoomRole::TYPE_0 && gy >= 2) {
            float r = GetRandomValue(1, 100) / 100.0f;
            if (r <= level.difficulty.liquidProbability) {
                LiquidType lType = (zone == ZoneType::TEMPLE) ? LiquidType::LAVA : LiquidType::WATER;
                instantiateLakeRoom(gx, gy, level.tileMap.get(), lType, level.initialLiquids);
                continue; // Skip the normal room instantiation
            }
        }

        int tileGrid[ROOM_HEIGHT][ROOM_WIDTH];
        
        if (isEntrance) {
            memcpy(tileGrid, entrance_room[v], sizeof(tileGrid));
        } else {
            switch (role) {
              case RoomRole::TYPE_1:
                memcpy(tileGrid, left_right_rooms[v], sizeof(tileGrid));
                break;
              case RoomRole::TYPE_2:
              case RoomRole::TYPE_2_DROP_THROUGH:
                memcpy(tileGrid, left_right_down_rooms[v], sizeof(tileGrid));
                break;
              case RoomRole::TYPE_3:
                memcpy(tileGrid, left_right_up_rooms[v], sizeof(tileGrid));
                break;
              case RoomRole::TYPE_SHOP:
                memcpy(tileGrid, shops[v % 2], sizeof(tileGrid));
                break;
              case RoomRole::TYPE_0:
              default:
                memcpy(tileGrid, closed_rooms[v], sizeof(tileGrid));
                break;
            }
        }
        instantiateTiles(tileGrid, gx, gy, role, level.tileMap.get());
      }
    }

    // ---- Spawn and exit placement ----
    int spawnGx = startRoomX * ROOM_WIDTH + 1;
    int spawnGy = startRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;
    bool foundEntrance = false;

    // Scan the entrance room for a pre-placed ENTRANCE tile (13) from the template
    for (int y = startRoomY * ROOM_HEIGHT; y < startRoomY * ROOM_HEIGHT + ROOM_HEIGHT; ++y) {
        for (int x = startRoomX * ROOM_WIDTH; x < startRoomX * ROOM_WIDTH + ROOM_WIDTH; ++x) {
            if (level.tileMap->getTile(x, y) == TileType::ENTRANCE) {
                spawnGx = x;
                spawnGy = y;
                foundEntrance = true;
                break;
            }
        }
        if (foundEntrance) break;
    }

    if (!foundEntrance) {
        // 1. Drop down if we start in mid-air
        while (spawnGy < startRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 1 && !level.tileMap->isSolid(spawnGx, spawnGy + 1)) {
          spawnGy++;
        }
        // 2. Go up if we are inside a solid block
        while (spawnGy > startRoomY * ROOM_HEIGHT && level.tileMap->isSolid(spawnGx, spawnGy)) {
          spawnGy--;
        }
        // 3. Fallback if trapped
        if (level.tileMap->isSolid(spawnGx, spawnGy))
          spawnGy = startRoomY * ROOM_HEIGHT + 1;
        
        level.tileMap->setTile(spawnGx, spawnGy, TileType::ENTRANCE);
    }

    // Offset by +8 to center the 16x24 player inside the 32x32 Entrance tile
    tempPlayerSpawn = Vector2{(float)(spawnGx * MAP_TILE_SIZE + 8),
                              (float)(spawnGy * MAP_TILE_SIZE + 8)};


    // Apply the same robust floor-finding logic to the exit placement
    int exitGx = exitRoomX * ROOM_WIDTH + ROOM_WIDTH - 2;
    int exitGy = exitRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;
    bool foundExit = false;

    // Scan the exit room for a pre-placed EXIT tile (14) from the template
    for (int y = exitRoomY * ROOM_HEIGHT; y < exitRoomY * ROOM_HEIGHT + ROOM_HEIGHT; ++y) {
        for (int x = exitRoomX * ROOM_WIDTH; x < exitRoomX * ROOM_WIDTH + ROOM_WIDTH; ++x) {
            if (level.tileMap->getTile(x, y) == TileType::EXIT) {
                exitGx = x;
                exitGy = y;
                foundExit = true;
                break;
            }
        }
        if (foundExit) break;
    }

    if (!foundExit) {
        while (exitGy < exitRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 1 && !level.tileMap->isSolid(exitGx, exitGy + 1)) {
          exitGy++;
        }
        while (exitGy > exitRoomY * ROOM_HEIGHT && level.tileMap->isSolid(exitGx, exitGy)) {
          exitGy--;
        }
        if (level.tileMap->isSolid(exitGx, exitGy))
          exitGy = exitRoomY * ROOM_HEIGHT + 1;

        level.tileMap->setTile(exitGx, exitGy, TileType::EXIT);
    }

    tempExitPos = Vector2{(float)(exitGx * MAP_TILE_SIZE),
                          (float)(exitGy * MAP_TILE_SIZE)};

    // Step 3.5: Golden Path Lake Generation
    int lakeGx = -1, lakeGy = -1;
    if (GetRandomValue(1, 100) <= 70) { // 70% chance for a lake on the map
        std::vector<std::pair<int, int>> validLakeRooms;
        
        for (int gy = 1; gy < MAP_ROOMS_Y; ++gy) { // Don't put it in the very top row
            for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
                bool isEntrance = (gx == startRoomX && gy == startRoomY);
                bool isExit = (gx == exitRoomX && gy == exitRoomY);
                if (macroGrid[gy][gx] == RoomRole::TYPE_1 && !isEntrance && !isExit) {
                    validLakeRooms.push_back({gx, gy});
                }
            }
        }
        
        if (!validLakeRooms.empty()) {
            TraceLog(LOG_INFO, "Found %zu valid lake rooms. Spawning lake.", validLakeRooms.size());
            int idx = GetRandomValue(0, validLakeRooms.size() - 1);
            auto room = validLakeRooms[idx];
            lakeGx = room.first;
            lakeGy = room.second;
            
            int startX = lakeGx * ROOM_WIDTH;
            int startY = lakeGy * ROOM_HEIGHT;
            int endX = startX + ROOM_WIDTH;
            int endY = startY + ROOM_HEIGHT;
            
            // Randomly choose height 3 or 4 for the lake
            int targetHeight = GetRandomValue(3, 4);
            
            // The floor is at endY - 1. We want the walls to go UP from the floor.
            int floorY = endY - 1;
            
            // Ensure the floor is solid across the room
            for (int x = startX; x < endX; ++x) {
                level.tileMap->setTile(x, floorY, TileType::CAVE_REGULAR);
            }
            
            // Build solid walls on the left and right to hold the water
            // Walls start at floorY - 1 and go up to floorY - targetHeight
            for (int y = floorY - 1; y >= floorY - targetHeight; --y) {
                level.tileMap->setTile(startX, y, TileType::CAVE_REGULAR);
                level.tileMap->setTile(endX - 1, y, TileType::CAVE_REGULAR);
                
                // Clear the inside to ensure a deep lake
                for (int x = startX + 1; x < endX - 1; ++x) {
                    level.tileMap->setTile(x, y, TileType::NOTHING);
                }
            }
            
            // Fill water basin. Water fills the inside from floorY - 1 up to floorY - targetHeight
            LiquidType lType = (zone == ZoneType::TEMPLE) ? LiquidType::LAVA : LiquidType::WATER;
            for (int y = floorY - 1; y >= floorY - targetHeight; --y) {
                for (int x = startX + 1; x < endX - 1; ++x) {
                    if (!level.tileMap->isSolid(x, y)) {
                        level.initialLiquids.push_back(LiquidSpawn{x, y, lType});
                    }
                }
            }
            
            // 50% chance to spawn treasure in the lake (as requested)
            if (GetRandomValue(1, 100) <= 50) {
                int tx = startX + GetRandomValue(2, ROOM_WIDTH - 3);
                int ty = floorY - 1;
                while (ty > startY && level.tileMap->isSolid(tx, ty)) {
                    ty--;
                }
                float px = tx * MAP_TILE_SIZE;
                float py = ty * MAP_TILE_SIZE;
                auto treasure = EntityFactory::createItem('$', px, py);
                if (treasure) {
                    tempItems.push_back(std::move(treasure));
                }
            }
        } else {
            TraceLog(LOG_INFO, "No valid lake rooms found!");
        }
    } else {
        TraceLog(LOG_INFO, "Failed 70%% chance to spawn lake.");
    }

    // Step 4: Populating
    for (int gy = 0; gy < MAP_ROOMS_Y; ++gy) {
      for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
        RoomRole role = macroGrid[gy][gx];
        int v         = roomVariations[gy][gx];
        bool isEntrance = (gx == startRoomX && gy == startRoomY);

        int npcGrid[ROOM_HEIGHT][ROOM_WIDTH];
        int lootGrid[ROOM_HEIGHT][ROOM_WIDTH];
        
        if (isEntrance) {
            memset(npcGrid, 0, sizeof(npcGrid)); // entrance_room has no NPCs
            memcpy(lootGrid, entrance_room_loot[v], sizeof(lootGrid));
        } else {
            switch (role) {
              case RoomRole::TYPE_1:
                memcpy(npcGrid,  left_right_npcs[v],  sizeof(npcGrid));
                memcpy(lootGrid, left_right_loot[v],  sizeof(lootGrid));
                break;
              case RoomRole::TYPE_2:
              case RoomRole::TYPE_2_DROP_THROUGH:
                memcpy(npcGrid,  left_right_down_npcs[v],  sizeof(npcGrid));
                memcpy(lootGrid, left_right_down_loot[v],  sizeof(lootGrid));
                break;
              case RoomRole::TYPE_3:
                memcpy(npcGrid,  left_right_up_npcs[v],  sizeof(npcGrid));
                memcpy(lootGrid, left_right_up_loot[v],  sizeof(lootGrid));
                break;
              case RoomRole::TYPE_SHOP:
                memcpy(npcGrid,  shops_npcs[v % 2],  sizeof(npcGrid));
                memcpy(lootGrid, shops_loot[v % 2],  sizeof(lootGrid));
                break;
              case RoomRole::TYPE_0:
              default:
                memcpy(npcGrid,  closed_rooms_npcs[v], sizeof(npcGrid));
                memcpy(lootGrid, closed_rooms_loot[v], sizeof(lootGrid));
                break;
            }
        }
        
        if (gx == lakeGx && gy == lakeGy) {
            // It's a lake room! No enemies, no traps.
            memset(npcGrid, 0, sizeof(npcGrid));
            
            // 50% chance for treasure per bottom tile
            memset(lootGrid, 0, sizeof(lootGrid));
            for (int x = 0; x < ROOM_WIDTH; ++x) {
                if (GetRandomValue(1, 100) <= 50) {
                    lootGrid[ROOM_HEIGHT - 2][x] = 1; // 1 usually means gold
                }
            }
        }
        
        populateEntities(npcGrid, lootGrid, gx, gy, role, level.tileMap.get());
      }
    }

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

  // Post-process: convert first reachable closed room to shop
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
int LevelGenerator::selectVariation(RoomRole role, bool isEntrance) const {
  if (isEntrance) return GetRandomValue(0, 5); // 6 variations for entrance rooms

  int numVars = 6; // Most rooms have 6 variations
  switch (role) {
    case RoomRole::TYPE_SHOP:          numVars = 2; break;
    default:                           break;
  }
  return GetRandomValue(0, numVars - 1);
}

// ---------------------------------------------------------------------------
void LevelGenerator::instantiateTiles(const int tileGrid[ROOM_HEIGHT][ROOM_WIDTH], int gx, int gy, RoomRole role, TileMap* map) {
  for (int cy = 0; cy < ROOM_HEIGHT; ++cy) {
    for (int cx = 0; cx < ROOM_WIDTH; ++cx) {
      int tx = gx * ROOM_WIDTH  + cx;
      int ty = gy * ROOM_HEIGHT + cy;

      if (role == RoomRole::TYPE_2_DROP_THROUGH) {
        if (cy < 2 && cx >= 4 && cx <= 5) {
          map->setTile(tx, ty, TileType::NOTHING);
          continue;
        }
      }

      int tileVal = tileGrid[cy][cx];
      map->setTile(tx, ty, static_cast<TileType>(tileVal));
    }
  }
}

void LevelGenerator::instantiateLakeRoom(int gx, int gy, TileMap* map, LiquidType lType, std::vector<LiquidSpawn>& initialLiquids) {
  // String template parsing using Spelunky Classic style ('w' for water, '3' for 50/50 block/water)
  std::string lakeTemplate = 
      "0000000000"
      "0000000000"
      "0000000000"
      "11wwwwww11"
      "11wwwwww11"
      "113wwww311"
      "113wwww311"
      "113wwww311"
      "1133331111"
      "1111111111";

  for (int cy = 0; cy < ROOM_HEIGHT; ++cy) {
    for (int cx = 0; cx < ROOM_WIDTH; ++cx) {
      int tx = gx * ROOM_WIDTH  + cx;
      int ty = gy * ROOM_HEIGHT + cy;
      
      char c = lakeTemplate[cy * ROOM_WIDTH + cx];
      
      if (c == '0') {
          map->setTile(tx, ty, TileType::NOTHING);
      } else if (c == '1') {
          map->setTile(tx, ty, TileType::CAVE_ROCK);
      } else if (c == 'w') {
          map->setTile(tx, ty, TileType::NOTHING);
          initialLiquids.push_back({tx, ty, lType});
      } else if (c == '3') {
          if (GetRandomValue(0, 1) == 0) {
              map->setTile(tx, ty, TileType::CAVE_ROCK);
          } else {
              map->setTile(tx, ty, TileType::NOTHING);
              initialLiquids.push_back({tx, ty, lType});
          }
      }
    }
  }
}

void LevelGenerator::populateEntities(const int npcGrid[ROOM_HEIGHT][ROOM_WIDTH],
                                      const int lootGrid[ROOM_HEIGHT][ROOM_WIDTH],
                                      int gx, int gy, RoomRole role, TileMap* map) {
  for (int cy = 0; cy < ROOM_HEIGHT; ++cy) {
    for (int cx = 0; cx < ROOM_WIDTH; ++cx) {
      int tx = gx * ROOM_WIDTH  + cx;
      int ty = gy * ROOM_HEIGHT + cy;
      
      // Do not spawn anything on Entrance or Exit tiles
      TileType tile = map->getTile(tx, ty);
      if (tile == TileType::ENTRANCE || tile == TileType::EXIT) {
          continue;
      }

      float px = tx * MAP_TILE_SIZE;
      float py = ty * MAP_TILE_SIZE;

      int npc = npcGrid[cy][cx];
      if (npc > 0) {
        lastPlacement++;
        int r = GetRandomValue(1, 100);

        switch (npc) {
          case 1: { // Snake
            if (snakesLeft > 0 && r <= 50) {
              auto enemyObj = EntityFactory::createEnemy('S', px, py);
              if (enemyObj) {
                  auto* e = static_cast<Enemy*>(enemyObj.get());
                  e->setTileMap(map);
                  tempEnemies.push_back(std::move(enemyObj));
                  snakesLeft--; lastPlacement = 0;
              }
            }
            break;
          }
          case 2: { // Bat
            if (batsLeft > 0 && r <= 50) {
              auto enemyObj = EntityFactory::createEnemy('B', px, py);
              if (enemyObj) {
                  auto* e = static_cast<Enemy*>(enemyObj.get());
                  e->setTileMap(map);
                  tempEnemies.push_back(std::move(enemyObj));
                  batsLeft--; lastPlacement = 0;
              }
            }
            break;
          }
          case 3: { // Spider
            if (spidersLeft > 0 && r <= 50) {
              auto enemyObj = EntityFactory::createEnemy('P', px, py);
              if (enemyObj) {
                  auto* e = static_cast<Enemy*>(enemyObj.get());
                  e->setTileMap(map);
                  tempEnemies.push_back(std::move(enemyObj));
                  spidersLeft--; lastPlacement = 0;
              }
            }
            break;
          }
          case 4: { // Spikes
            int r3 = GetRandomValue(0, 2);
            if (spikesLeft > 0 && r3 == 1) {
              int tx = static_cast<int>(px / MAP_TILE_SIZE);
              int ty = static_cast<int>(py / MAP_TILE_SIZE);
              while (ty < (MAP_ROOMS_Y * ROOM_HEIGHT) && !map->isSolid(tx, ty)) {
                  ty++;
              }
              if (ty >= (MAP_ROOMS_Y * ROOM_HEIGHT)) {
                  ty = static_cast<int>(py / MAP_TILE_SIZE) + 1; // fallback
              }
              float actualY = (ty - 1) * MAP_TILE_SIZE;
              // Y offset + 8.0f (so the bottom 8 pixels of transparent space are covered by the tile below)
              auto enemyObj = EntityFactory::createEnemy('^', px, actualY + 8.0f);
              if (enemyObj) {
                  auto* e = static_cast<Enemy*>(enemyObj.get());
                  e->setTileMap(map);
                  tempEnemies.push_back(std::move(enemyObj));
                  spikesLeft--;
              }
            }
            break;
          }
          case 9: { // ArrowTrap Left
            map->setTile(tx, ty, TileType::ARROW_TRAP_LEFT);
            auto trap = EntityFactory::createTrap('<', px, py);
            if (trap) tempTraps.push_back(std::move(trap));
            break;
          }
          case 10: { // ArrowTrap Right
            map->setTile(tx, ty, TileType::ARROW_TRAP_RIGHT);
            auto trap = EntityFactory::createTrap('>', px, py);
            if (trap) tempTraps.push_back(std::move(trap));
            break;
          }
          case 12: { // Shop Item
            char shopCodes[] = {'$', 'I', 'Y', 'L'};
            char itemCode = shopCodes[GetRandomValue(0, 3)];
            auto item = EntityFactory::createItem(itemCode, px, py + (32 - 16));
            if (item) {
                item->isShopItem = true;
                tempItems.push_back(std::move(item));
            }
            break;
          }
          case 20: { // Golden Idol
            auto item = EntityFactory::createItem('I', px, py);
            if (item) tempItems.push_back(std::move(item));
            break;
          }
          default: break;
        }
      }

      int loot = lootGrid[cy][cx];
      if (loot > 0 && GetRandomValue(1, 100) <= 5) {
        char itemCode = 0;
        if (loot == 1) {
            int r = GetRandomValue(1, 100);
            if (r <= 50) itemCode = 'G'; // 50% Gold
            else if (r <= 65) itemCode = 'R'; // 15% Ruby
            else if (r <= 80) itemCode = 'O'; // 15% Bomb
            else if (r <= 95) itemCode = 'U'; // 15% Rope
            else itemCode = 'C'; // 5% Chest
        } else {
            switch (loot) {
              case 2: itemCode = 'R'; break;
              case 3: itemCode = 'C'; break;
              case 4: itemCode = 'O'; break;
              case 5: itemCode = 'U'; break;
              default: break;
            }
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
    bool onLadder = map->isLadder(curr.x, curr.y);

    if (onGround || onLadder) {
      pushState(curr.x - 1, curr.y, 4);
      pushState(curr.x + 1, curr.y, 4);
      pushState(curr.x, curr.y - 1, 3);
      if (onLadder) {
          pushState(curr.x, curr.y - 1, 0); // Climb up
          pushState(curr.x, curr.y + 1, 0); // Climb down
      }
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
