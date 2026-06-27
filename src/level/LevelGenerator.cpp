#include "LevelGenerator.h"
#include "../entities/EntityFactory.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <queue>
#include <random>

namespace Platformer {


LevelGenerator::LevelGenerator() {
  std::string roomsDir = "assets/rooms/";
  if (std::filesystem::exists(roomsDir)) {
    for (const auto &entry : std::filesystem::directory_iterator(roomsDir)) {
      if (entry.path().extension() == ".txt") {
        RoomTemplate tpl(entry.path().string());
        if (tpl.load()) {
          templates.push_back(tpl);
        }
      }
    }
  } else {
    std::cerr
        << "WARNING: " << roomsDir
        << " does not exist. LevelGenerator will use fallback empty rooms."
        << std::endl;
  }
}

GeneratedLevel LevelGenerator::generate(int floor, ZoneType zone) {
  GeneratedLevel level;

  int maxRetries = 10;
  bool isValid = false;

  for (int attempt = 0; attempt < maxRetries; ++attempt) {
    generateMacroGrid();

    int mapW = MAP_ROOMS_X * ROOM_WIDTH;
    int mapH = MAP_ROOMS_Y * ROOM_HEIGHT;
    level.tileMap = std::make_unique<TileMap>(mapW, mapH, MAP_TILE_SIZE);

    tempEnemies.clear();
    tempItems.clear();
    tempTraps.clear();
    tempPlayerSpawn = Vector2{0, 0};
    tempExitPos = Vector2{0, 0};

    for (int gy = 0; gy < MAP_ROOMS_Y; ++gy) {
      for (int gx = 0; gx < MAP_ROOMS_X; ++gx) {
        RoomRole role = macroGrid[gy][gx];

        RoomTemplate tpl = selectRoomTemplate(role);
        populateRoom(tpl, gx, gy, role, level.tileMap.get());
      }
    }

    // Carving pass removed; Spelunky templates naturally have open walls.

    generateChunks(level.tileMap.get());
    generateBorders(level.tileMap.get());

    // Dynamically place Spawn and Exit
    // Put spawn in top-left-most floor tile of start room
    int spawnGx = startRoomX * ROOM_WIDTH + 1;
    int spawnGy = startRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;
    while (spawnGy > startRoomY * ROOM_HEIGHT && level.tileMap->isSolid(spawnGx, spawnGy)) {
      spawnGy--;
    }
    
    // If we couldn't find a non-solid, default to center of room
    if (level.tileMap->isSolid(spawnGx, spawnGy)) {
        spawnGy = startRoomY * ROOM_HEIGHT + 1;
    }
    tempPlayerSpawn = Vector2{(float)(spawnGx * MAP_TILE_SIZE),
                              (float)(spawnGy * MAP_TILE_SIZE)};
    level.tileMap->setTile(spawnGx, spawnGy, TileType::EMPTY);

    // Put exit in bottom-right-most floor tile of exit room
    int exitGx = exitRoomX * ROOM_WIDTH + ROOM_WIDTH - 2;
    int exitGy = exitRoomY * ROOM_HEIGHT + ROOM_HEIGHT - 2;

    // Find the lowest non-solid tile in that column
    while (exitGy > exitRoomY * ROOM_HEIGHT && level.tileMap->isSolid(exitGx, exitGy)) {
      exitGy--;
    }
    
    if (level.tileMap->isSolid(exitGx, exitGy)) {
        exitGy = exitRoomY * ROOM_HEIGHT + 1;
    }

    tempExitPos = Vector2{(float)(exitGx * MAP_TILE_SIZE),
                          (float)(exitGy * MAP_TILE_SIZE)};
    level.tileMap->setTile(exitGx, exitGy, TileType::EXIT_DOOR);

    Vector2i startGrid = {spawnGx, spawnGy};
    Vector2i exitGrid = {exitGx, exitGy};

    if (tempPlayerSpawn.x == 0 && tempPlayerSpawn.y == 0 &&
        tempExitPos.x == 0 && tempExitPos.y == 0) {
      isValid = true; // Bypass for empty fallback maps during testing
    } else {
      isValid = validateLevel(level.tileMap.get(), startGrid, exitGrid);
    }

    if (isValid || attempt == maxRetries - 1) {
      level.dynamicEntities = std::move(tempEnemies);
      level.items = std::move(tempItems);
      level.traps = std::move(tempTraps);
      level.playerSpawn = tempPlayerSpawn;
      level.exitPos = tempExitPos;
      level.difficulty = getDifficultyConfig(floor);
      level.modifier = rollFloorModifier(floor);
      return level;
    }
  }

  level.dynamicEntities = std::move(tempEnemies);
  level.items = std::move(tempItems);
  level.traps = std::move(tempTraps);
  level.playerSpawn = tempPlayerSpawn;
  level.exitPos = tempExitPos;
  level.difficulty = getDifficultyConfig(floor);
  level.modifier = rollFloorModifier(floor);

  return level;
}

void LevelGenerator::generateMacroGrid() {
  for (int y = 0; y < MAP_ROOMS_Y; ++y) {
    for (int x = 0; x < MAP_ROOMS_X; ++x) {
      macroGrid[y][x] = RoomRole::TYPE_0;
    }
  }

  int currX = GetRandomValue(0, MAP_ROOMS_X - 1);
  int currY = 0;
  bool movingLeft = (GetRandomValue(0, 1) == 0);

  startRoomX = currX;
  startRoomY = currY;
  
  while (currY < MAP_ROOMS_Y) {
    if (macroGrid[currY][currX] == RoomRole::TYPE_0) {
      macroGrid[currY][currX] = RoomRole::TYPE_1;
    }

    int roll = GetRandomValue(1, 5);
    bool forcedDown = false;
    
    if (roll <= 4) {
      if (movingLeft) {
        if (currX == 0) forcedDown = true;
        else currX--;
      } else {
        if (currX == MAP_ROOMS_X - 1) forcedDown = true;
        else currX++;
      }
    } else {
      forcedDown = true;
    }
    
    if (forcedDown) {
      if (currY < MAP_ROOMS_Y - 1) {
        if (macroGrid[currY][currX] == RoomRole::TYPE_3) {
            macroGrid[currY][currX] = RoomRole::TYPE_2_DROP_THROUGH;
        } else {
            macroGrid[currY][currX] = RoomRole::TYPE_2;
        }
        currY++;
        macroGrid[currY][currX] = RoomRole::TYPE_3;
        movingLeft = (GetRandomValue(0, 1) == 0);
      } else {
        exitRoomX = currX;
        exitRoomY = currY;
        break;
      }
    }
  }
}

RoomTemplate LevelGenerator::selectRoomTemplate(RoomRole role) {
  if (templates.empty()) {
    // Generate a fallback empty 10x8 shell room
    RoomTemplate shell("");
    return shell;
  }

  RoomRole searchRole = role;
  if (role == RoomRole::TYPE_2_DROP_THROUGH) {
      searchRole = RoomRole::TYPE_2;
  }

  // Filter templates by role
  std::vector<RoomTemplate> filtered;
  for (const auto &t : templates) {
    if (t.getRole() == searchRole)
      filtered.push_back(t);
  }

  if (filtered.empty())
    return templates[0];

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine rng(seed);
  std::uniform_int_distribution<int> dist(0, filtered.size() - 1);

  return filtered[dist(rng)];
}

void LevelGenerator::populateRoom(const RoomTemplate &tpl, int gx, int gy, RoomRole role,
                                  TileMap *map) {
  auto grid = tpl.getGrid();

  // If empty grid (fallback), generate a basic bordered room
  bool isFallback =
      grid.empty() || grid.size() < ROOM_HEIGHT || grid[0].size() < ROOM_WIDTH;

  bool skip[ROOM_HEIGHT][ROOM_WIDTH] = {false};

  for (int cy = 0; cy < ROOM_HEIGHT; ++cy) {
    for (int cx = 0; cx < ROOM_WIDTH; ++cx) {
      if (skip[cy][cx]) continue;

      int tx = gx * ROOM_WIDTH + cx;
      int ty = gy * ROOM_HEIGHT + cy;

      char code = '0';
      if (isFallback) {
        if (cx == 0 || cx == ROOM_WIDTH - 1 || cy == 0 || cy == ROOM_HEIGHT - 1)
          code = '1';
        else
          code = '0';
      } else {
        code = grid[cy][cx];
      }

      // If this is a drop-through room, punch a hole in the ceiling to connect to the drop room above
      if (role == RoomRole::TYPE_2_DROP_THROUGH) {
          if (cy < 2 && cx >= 4 && cx <= 6) {
              code = '0';
          }
      }

      float px = tx * MAP_TILE_SIZE;
      float py = ty * MAP_TILE_SIZE;

      switch (code) {
      case '1':
        map->setTile(tx, ty, TileType::WALL);
        break;
      case 'C':
        map->setTile(tx, ty, TileType::CRACKED);
        break;
      case 'P':
        map->setTile(tx, ty, TileType::PLATFORM);
        break;
      case 'L':
        map->setTile(tx, ty, TileType::LADDER);
        break;
      case '4':
        // 25% pushblock (using WALL for now until pushblock entity exists)
        if (GetRandomValue(1, 100) <= 25) {
            map->setTile(tx, ty, TileType::WALL);
        } else {
            map->setTile(tx, ty, TileType::EMPTY);
        }
        break;
      case '5':
      case '6': {
        std::vector<std::string> block;
        if (code == '5') {
            block = {
                "00000",
                "00202",
                "71177"
            };
        } else {
            block = {
                "00000",
                "22222",
                "00000"
            };
        }

        for (int dy = 0; dy < 3; ++dy) {
            for (int dx = 0; dx < 5; ++dx) {
                if (cy + dy >= ROOM_HEIGHT || cx + dx >= ROOM_WIDTH) continue;
                
                char bc = block[dy][dx];
                int btx = tx + dx;
                int bty = ty + dy;
                float bpx = btx * MAP_TILE_SIZE;
                float bpy = bty * MAP_TILE_SIZE;
                
                skip[cy + dy][cx + dx] = true;
                
                // Probabilistic evaluations
                if (bc == '2') {
                    bc = (GetRandomValue(0, 1) == 0) ? '1' : '0';
                } else if (bc == '7') {
                    bc = (GetRandomValue(1, 10) <= 3) ? '^' : '0'; // 30% Spikes Trap
                }

                if (bc == '1') {
                    map->setTile(btx, bty, TileType::WALL);
                } else if (bc == '0') {
                    map->setTile(btx, bty, TileType::EMPTY);
                }

                auto trap = EntityFactory::createTrap(bc, bpx, bpy);
                if (trap) tempTraps.push_back(std::move(trap));
            }
        }
        break;
      }
      case '@':
        map->setTile(tx, ty, TileType::EMPTY);
        tempPlayerSpawn = Vector2{px, py};
        break;
      case 'X':
        map->setTile(tx, ty, TileType::EXIT_DOOR);
        tempExitPos = Vector2{px, py};
        break;
      case '0':
      default:
        map->setTile(tx, ty, TileType::EMPTY);
        break;
      }

      // Check entities
      auto enemy = EntityFactory::createEnemy(code, px, py);
      if (enemy)
        tempEnemies.push_back(std::move(enemy));

      auto item = EntityFactory::createItem(code, px, py);
      if (item)
        tempItems.push_back(std::move(item));

      auto trap = EntityFactory::createTrap(code, px, py);
      if (trap)
        tempTraps.push_back(std::move(trap));
    }
  }
}


void LevelGenerator::generateChunks(TileMap *map) {
  int width = map->getWidth();
  int height = map->getHeight();

  std::vector<std::vector<bool>> visited(height,
                                         std::vector<bool>(width, false));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (map->getTile(x, y) != TileType::WALL || visited[y][x]) {
        continue;
      }

      ChunkInfo chunk;
      chunk.isOrigin = true;

      // Try to form a 2x2 chunk
      bool can2x2 =
          (x + 1 < width && y + 1 < height) &&
          (map->getTile(x + 1, y) == TileType::WALL && !visited[y][x + 1]) &&
          (map->getTile(x, y + 1) == TileType::WALL && !visited[y + 1][x]) &&
          (map->getTile(x + 1, y + 1) == TileType::WALL &&
           !visited[y + 1][x + 1]);

      // Introduce some random bias (60% chance) to form 2x2 to avoid repetitive
      // patterns
      if (can2x2 && GetRandomValue(1, 10) <= 5) {
        chunk.width = 2;
        chunk.height = 2;
        int r = GetRandomValue(0, 3);
        if (r == 0) {
          chunk.offsetX = 0;
          chunk.offsetY = 256;
        } else if (r == 1) {
          chunk.offsetX = 128;
          chunk.offsetY = 256;
        } else if (r == 2) {
          chunk.offsetX = 0;
          chunk.offsetY = 384;
        } else {
          chunk.offsetX = 128;
          chunk.offsetY = 384;
        }

        visited[y][x] = true;
        visited[y][x + 1] = true;
        visited[y + 1][x] = true;
        visited[y + 1][x + 1] = true;

        map->setChunk(x, y, chunk);

        ChunkInfo child;
        child.isOrigin = false;
        map->setChunk(x + 1, y, child);
        map->setChunk(x, y + 1, child);
        map->setChunk(x + 1, y + 1, child);
        continue;
      }

      // Try 1x2 or 2x1
      bool can1x2 =
          (y + 1 < height) &&
          (map->getTile(x, y + 1) == TileType::WALL && !visited[y + 1][x]);
      bool can2x1 =
          (x + 1 < width) &&
          (map->getTile(x + 1, y) == TileType::WALL && !visited[y][x + 1]);

      bool do2Block = (can1x2 || can2x1) && (GetRandomValue(1, 10) <= 5);

      if (do2Block && can1x2 && (!can2x1 || GetRandomValue(0, 1) == 0)) {
        chunk.width = 1;
        chunk.height = 2;
        int r = GetRandomValue(0, 1);
        if (r == 0) {
          chunk.offsetX = 128;
          chunk.offsetY = 64;
        } else {
          chunk.offsetX = 192;
          chunk.offsetY = 64;
        }

        visited[y][x] = true;
        visited[y + 1][x] = true;

        map->setChunk(x, y, chunk);

        ChunkInfo child;
        child.isOrigin = false;
        map->setChunk(x, y + 1, child);
        continue;
      } else if (do2Block && can2x1) {
        chunk.width = 2;
        chunk.height = 1;
        int r = GetRandomValue(0, 1);
        if (r == 0) {
          chunk.offsetX = 0;
          chunk.offsetY = 192;
        } else {
          chunk.offsetX = 128;
          chunk.offsetY = 192;
        }

        visited[y][x] = true;
        visited[y][x + 1] = true;

        map->setChunk(x, y, chunk);

        ChunkInfo child;
        child.isOrigin = false;
        map->setChunk(x + 1, y, child);
        continue;
      }

      // Fallback to 1x1
      chunk.width = 1;
      chunk.height = 1;
      int r = GetRandomValue(0, 3);
      if (r == 0) {
        chunk.offsetX = 0;
        chunk.offsetY = 64;
      } else if (r == 1) {
        chunk.offsetX = 0;
        chunk.offsetY = 128;
      } else if (r == 2) {
        chunk.offsetX = 64;
        chunk.offsetY = 64;
      } else {
        chunk.offsetX = 64;
        chunk.offsetY = 128;
      }

      visited[y][x] = true;
      map->setChunk(x, y, chunk);
    }
  }
}

void LevelGenerator::generateBorders(TileMap *map) {
  int width = map->getWidth();
  int height = map->getHeight();
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (map->getTile(x, y) == TileType::WALL) {
        ChunkInfo chunk = map->getChunk(x, y);
        uint8_t mask = 0;

        // Check Top
        if (y > 0 && !map->isSolid(x, y - 1))
          mask |= 1;
        else if (y == 0)
          mask |= 1; // Map edge treated as exposed

        // Check Right
        if (x < width - 1 && !map->isSolid(x + 1, y))
          mask |= 2;

        // Check Bottom
        if (y < height - 1 && !map->isSolid(x, y + 1))
          mask |= 4;

        // Check Left
        if (x > 0 && !map->isSolid(x - 1, y))
          mask |= 8;

        chunk.borderMask = mask;
        map->setChunk(x, y, chunk);
      }
    }
  }
}

bool LevelGenerator::validateLevel(TileMap *map, Vector2i start,
                                   Vector2i exit) {
  return bfsReachability(map, start, exit);
}

bool LevelGenerator::bfsReachability(TileMap *map, Vector2i from, Vector2i to) {
  int width = map->getWidth();
  int height = map->getHeight();

  // Visited array storing the maximum jump energy we had at this cell.
  // Initialize to -1 (unvisited).
  std::vector<std::vector<int>> visited(height, std::vector<int>(width, -1));

  struct State {
    int x, y, jumpEnergy;
  };

  std::queue<State> q;
  q.push({from.x, from.y, 0});
  visited[from.y][from.x] = 0;

  auto pushState = [&](int nx, int ny, int nenergy) {
    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
      if (!map->isSolid(nx, ny) && visited[ny][nx] < nenergy) {
        visited[ny][nx] = nenergy;
        q.push({nx, ny, nenergy});
      }
    }
  };

  while (!q.empty()) {
    State curr = q.front();
    q.pop();

    if (curr.x == to.x && curr.y == to.y)
      return true;

    bool onGround = (curr.y + 1 < height) && map->isSolid(curr.x, curr.y + 1);
    bool onLadder = (map->getTile(curr.x, curr.y) == TileType::LADDER);

    if (onGround || onLadder) {
      // Walk left/right
      pushState(curr.x - 1, curr.y, 4);
      pushState(curr.x + 1, curr.y, 4);
      // Start jump or climb up
      pushState(curr.x, curr.y - 1, 3);
      
      // If on ladder, we can climb down without gravity
      if (onLadder) {
          pushState(curr.x, curr.y + 1, 0); // Climb down
      }
    } else {
      // Falling / Gravity
      pushState(curr.x, curr.y + 1, 0);

      // Air strafe
      pushState(curr.x - 1, curr.y, 0);
      pushState(curr.x + 1, curr.y, 0);

      // Continue jump up
      if (curr.jumpEnergy > 0) {
        pushState(curr.x, curr.y - 1, curr.jumpEnergy - 1);
        pushState(curr.x - 1, curr.y - 1, curr.jumpEnergy - 1);
        pushState(curr.x + 1, curr.y - 1, curr.jumpEnergy - 1);
      }
    }
  }

  return false; // Exit is not reachable
}

DifficultyConfig LevelGenerator::getDifficultyConfig(int floor) {
  DifficultyConfig cfg;
  cfg.maxEnemiesPerRoom = 3 + floor / 2;
  cfg.trapDensity = 0.1f + (floor * 0.05f);
  cfg.treasureValueMultiplier = 1;
  cfg.enemySpeedScale = 1.0f + (floor * 0.1f);
  cfg.ghostTimerSeconds = std::max(60.0f, 180.0f - floor * 10.0f);
  cfg.liquidProbability = 0.2f;
  return cfg;
}

FloorModifier LevelGenerator::rollFloorModifier(int floor) {
  if (floor == 1)
    return FloorModifier::NONE;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine rng(seed);
  std::uniform_real_distribution<float> dist(0.0f, 1.0f);

  if (dist(rng) < 0.3f) {
    std::uniform_int_distribution<int> modDist(1, 3);
    int mod = modDist(rng);
    if (mod == 1)
      return FloorModifier::DARK_FLOOR;
    if (mod == 2)
      return FloorModifier::FLOODED_FLOOR;
    if (mod == 3)
      return FloorModifier::CURSED_FLOOR;
  }
  return FloorModifier::NONE;
}

} // namespace Platformer
