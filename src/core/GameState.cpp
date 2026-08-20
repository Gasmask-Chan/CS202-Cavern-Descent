#include "GameState.h"
#include "../audio/AudioManager.h"
#include "EventBus.h"
#include "../entities/projectiles/Arrow.h"
#include "../entities/items/Bomb.h"
#include "../entities/EntityFactory.h"
#include "../entities/items/Item.h"
#include "../entities/effects/LavaDrip.h"
#include "../entities/traps/Trap.h"
#include "../entities/enemies/Enemy.h"
#include "../entities/enemies/NemesisGhost.h"
#include "../entities/enemies/Spike.h"
#include "../entities/effects/Explosion.h"
#include "../entities/effects/Particle.h"
#include "../entities/projectiles/RopeProjectile.h"
#include "../liquid/LiquidSimulator.h"
#include "../level/LevelGenerator.h"
#include "../player/Player.h"
#include "../shop/ShopSystem.h"
#include "../ui/ComboSystem.h"
#include "../ui/Minimap.h"
#include "../ui/MenuBackground.h"
#include "../vendor/tinyfiledialogs.h"
#include "Game.h"
#include "GameManager.h"
#include "raymath.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>


namespace Platformer {

void GameState::setGame(Game *game) { this->game = game; }

void GameState::drawCenteredText(const char *text, float y, float fontSize,
                                 Color color) {
  Vector2 size = MeasureTextEx(game->getFont(), text, fontSize, 2.0f);
  DrawTextEx(game->getFont(), text, {(1280.0f - size.x) / 2.0f, y}, fontSize,
             2.0f, color);
}

void GameState::drawCenteredAt(const char *text, float centerX, float y,
                               float fontSize, Color color) {
  Vector2 size = MeasureTextEx(game->getFont(), text, fontSize, 2.0f);
  DrawTextEx(game->getFont(), text, {centerX - size.x / 2.0f, y}, fontSize,
             2.0f, color);
}

void GameState::drawLeftText(const char *text, float x, float y,
                             float fontSize, Color color) {
  DrawTextEx(game->getFont(), text, {x, y}, fontSize, 2.0f, color);
}

/*
=======================================================
=========================MENU==========================
=======================================================
*/

void MenuState::enter() {
  selectedOption = 0;
  AudioManager::getInstance()->playBGM("mTitle");
}

void MenuState::exit() {
}

void MenuState::handleInput() {
  if (IsKeyPressed(KEY_UP)) {
    AudioManager::getInstance()->playSFX("xclick");
    selectedOption = (selectedOption + 2) % 3;
  }
  if (IsKeyPressed(KEY_DOWN)) {
    AudioManager::getInstance()->playSFX("xclick");
    selectedOption = (selectedOption + 1) % 3;
  }
  if (IsKeyPressed(KEY_ENTER)) {
    AudioManager::getInstance()->playSFX("xclick");
    switch (selectedOption) {
    case 0:
      game->changeState(GameStateType::CHAR_SELECT);
      break;
    case 1:
      game->changeState(GameStateType::EDITOR_MENU);
      break;
    case 2:
      game->quit();
      break;
    }
  }
}

void MenuState::update(float dt) {
  // Placeholder: Update background animation or effects if added later
}

void MenuState::render() {
  ClearBackground(BLACK);

  MenuBackground::render();

  // Draw semi-transparent left panel
  DrawRectangle(0, 0, 600, 720, {0, 0, 0, 200});

  drawLeftText("CAVERN", 50.0f, 60.0f, 60.0f, RAYWHITE);
  drawLeftText("DESCENT", 50.0f, 110.0f, 60.0f, RAYWHITE);

  Color startColor = (selectedOption == 0) ? YELLOW : LIGHTGRAY;
  Color editorColor = (selectedOption == 1) ? YELLOW : LIGHTGRAY;
  Color quitColor = (selectedOption == 2) ? YELLOW : LIGHTGRAY;

  drawLeftText("START GAME", 50.0f, 350.0f, 40.0f, startColor);
  drawLeftText("LEVEL EDITOR", 50.0f, 420.0f, 40.0f, editorColor);
  drawLeftText("QUIT", 50.0f, 490.0f, 40.0f, quitColor);

  drawLeftText("UP/DOWN: Navigate", 50.0f, 650.0f, 20.0f, GRAY);
  drawLeftText("ENTER: Select", 50.0f, 680.0f, 20.0f, GRAY);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

PlayState::PlayState() = default;
PlayState::~PlayState() = default;

void PlayState::enter() {
  deathTimer = -1.0f;
  EntityFactory::preloadTextures();

  Image hudImg = LoadImage("assets/sprites/16x16/gfx_hud.png");
  ImageFormat(&hudImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
  hudIcons = EntityFactory::getTexture("assets/sprites/16x16/gfx_hud.png");
  shopkeeperTex =
      EntityFactory::getTexture("assets/sprites/16x16/gfx_shopkeeper.png");

  // Initialize Camera
  camera.offset = Vector2{1280.0f / 2.0f, 720.0f / 2.0f}; // Center screen
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;

  tempGenerator = std::make_unique<LevelGenerator>();

  Color zoneTint = WHITE;

  if (GameManager::getInstance()->getIsCustomLevel()) {
    tempLevel.tileMap = std::make_unique<TileMap>(40, 32, 32);
    tempLevel.playerSpawn = {32.0f, 32.0f}; // default fallback
    tempLevel.exitPos = {200.0f, 200.0f};
    tempLevel.shopArea = {0, 0, 0, 0};
    
    // Initialize default difficulty since generator isn't called
    tempLevel.difficulty.ghostTimerSeconds = 180.0f;
    tempLevel.difficulty.treasureValueMultiplier = 1;

    std::ifstream in(GameManager::getInstance()->getCustomLevelPath());
    if (in.is_open()) {
      int tileVal;
      int x = 0, y = 0;
      while (in >> tileVal && y < 32) {
        TileType type = (TileType)tileVal;

        if (type == TileType::ENTRANCE) {
          tempLevel.playerSpawn = {(float)x * 32.0f + 16.0f,
                                   (float)y * 32.0f + 16.0f};
        } else if (type == TileType::EXIT) {
          tempLevel.exitPos = {(float)x * 32.0f, (float)y * 32.0f};
        }

        tempLevel.tileMap->setTile(x, y, type);

        x++;
        if (x >= 40) {
          x = 0;
          y++;
        }
      }
      in.close();

      // Spawn Custom Entities pass
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 40; x++) {
          TileType type = tempLevel.tileMap->getTile(x, y);
          float px = x * 32.0f;
          float py = y * 32.0f;

          if (type == TileType::CAVE_SOME_GOLD || type == TileType::CAVE_MUCH_GOLD) {
            tempLevel.tileMap->setTile(x, y, TileType::CAVE_ROCK);
            auto gold = EntityFactory::createItem('G', px, py);
            if (gold) tempLevel.items.push_back(std::move(gold));
          } else if (type == TileType::CHEST) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto chest = EntityFactory::createItem('C', px, py);
            if (chest) tempLevel.items.push_back(std::move(chest));
          } else if (type == TileType::ENEMY_SNAKE) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto enemy = EntityFactory::createEnemy('S', px, py);
            if (enemy) tempLevel.dynamicEntities.push_back(std::move(enemy));
          } else if (type == TileType::ENEMY_BAT) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto enemy = EntityFactory::createEnemy('B', px, py);
            if (enemy) tempLevel.dynamicEntities.push_back(std::move(enemy));
          } else if (type == TileType::ENEMY_SPIDER) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto enemy = EntityFactory::createEnemy('P', px, py);
            if (enemy) tempLevel.dynamicEntities.push_back(std::move(enemy));
          } else if (type == TileType::SPIKE_TRAP) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto enemy = EntityFactory::createEnemy('^', px, py + 8.0f); // Spikes act like enemies
            if (enemy) tempLevel.dynamicEntities.push_back(std::move(enemy));
          } else if (type == TileType::ARROW_TRAP_LEFT) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto trap = EntityFactory::createTrap('<', px, py);
            if (trap) tempLevel.traps.push_back(std::move(trap));
          } else if (type == TileType::ARROW_TRAP_RIGHT) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            auto trap = EntityFactory::createTrap('>', px, py);
            if (trap) tempLevel.traps.push_back(std::move(trap));
          } else if (type == TileType::LAVA) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            tempLevel.initialLiquids.push_back({x, y, LiquidType::LAVA});
          } else if (type == TileType::WATER) {
            tempLevel.tileMap->setTile(x, y, TileType::NOTHING);
            tempLevel.initialLiquids.push_back({x, y, LiquidType::WATER});
          }
        }
      }
    }

    // Load tilesets so tiles render properly when playing custom levels
    tempLevel.tileMap->setTileset(EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png"));
    tempLevel.tileMap->setJungleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png"));
    tempLevel.tileMap->setTempleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png"));
    tempLevel.tileMap->setRopeTexture(EntityFactory::getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"));
  } else {
    int currentFloor = GameManager::getInstance()->getFloor();
    ZoneType currentZone = GameManager::getInstance()->getZone();

    if (currentZone == ZoneType::JUNGLE) {
        zoneTint = Color{180, 255, 180, 255};
        AudioManager::getInstance()->playBGM("mLush");
    } else if (currentZone == ZoneType::TEMPLE) {
        zoneTint = Color{255, 200, 150, 255};
        AudioManager::getInstance()->playBGM("mTemple");
    } else {
        AudioManager::getInstance()->playBGM("mCave");
        if (currentFloor == 1) {
            AudioManager::getInstance()->playSFX("xletsexplore");
        }
    }

    tempLevel = tempGenerator->generate(currentFloor, currentZone);
  }

  // Apply tilesets to the loaded or generated map
  if (tempLevel.tileMap) {
      tempLevel.tileMap->setZoneTint(zoneTint);
      tempLevel.tileMap->setTileset(EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png"));
      tempLevel.tileMap->setJungleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png"));
      tempLevel.tileMap->setTempleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png"));
      tempLevel.tileMap->setRopeTexture(EntityFactory::getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"));
  }

  // Auto-Beautify CAVE_ROCK (Turn into Cave Up, Down, Gold, etc)
  auto isSolid = [&](int cx, int cy) {
    if (cx < 0 || cx >= tempLevel.tileMap->getWidth() || cy < 0 || cy >= tempLevel.tileMap->getHeight())
      return true;
    TileType t = tempLevel.tileMap->getTile(cx, cy);
    return t != TileType::NOTHING && t != TileType::ENTRANCE &&
           t != TileType::EXIT && t != TileType::ENEMY_SNAKE &&
           t != TileType::ENEMY_BAT && t != TileType::ENEMY_SPIDER &&
           t != TileType::CHEST && t != TileType::LAVA &&
           t != TileType::WATER && t != TileType::SPIKE_TRAP &&
           t != TileType::ARROW_TRAP_LEFT && t != TileType::ARROW_TRAP_RIGHT &&
           !(t >= TileType::LUSH_TOP_1 && t <= TileType::LUSH_LEFT) &&
           !(t >= TileType::VINE && t <= TileType::VINE_SOURCE) &&
           !(t >= TileType::LEAVE && t <= TileType::LEAVE_RIGHT);
  };

  std::vector<std::pair<Vector2i, TileType>> borderTiles;


  for (int y = 0; y < tempLevel.tileMap->getHeight(); y++) {
    for (int x = 0; x < tempLevel.tileMap->getWidth(); x++) {
      TileType type = tempLevel.tileMap->getTile(x, y);

      if (type == TileType::VINE || type == TileType::VINE_TOP) {
        TileType above = (y > 0) ? tempLevel.tileMap->getTile(x, y - 1) : TileType::NOTHING;
        if (above != TileType::VINE && above != TileType::VINE_TOP && 
            above != TileType::VINE_SOURCE && above != TileType::VINE_BOTTOM) {
            
            int topY = y;
            while (topY > 0) {
                TileType t = tempLevel.tileMap->getTile(x, topY - 1);
                if (t == TileType::NOTHING || t == TileType::VINE || t == TileType::VINE_TOP || t == TileType::VINE_SOURCE || t == TileType::VINE_BOTTOM) {
                    topY--;
                } else {
                    TraceLog(LOG_INFO, "Vine at x=%d upwards trace blocked by tile %d at y=%d", x, (int)t, topY - 1);
                    break;
                }
            }
            int bottomY = y;
            while (bottomY + 1 < tempLevel.tileMap->getHeight()) {
                TileType t = tempLevel.tileMap->getTile(x, bottomY + 1);
                if (t == TileType::VINE || t == TileType::VINE_TOP || t == TileType::VINE_SOURCE || t == TileType::VINE_BOTTOM) {
                    bottomY++;
                } else {
                    break;
                }
            }
            for (int cy = topY; cy <= bottomY; cy++) {
                if (cy == topY) {
                    tempLevel.tileMap->setTile(x, cy, TileType::VINE_SOURCE);
                } else if (cy == bottomY) {
                    tempLevel.tileMap->setTile(x, cy, TileType::VINE_BOTTOM);
                } else {
                    tempLevel.tileMap->setTile(x, cy, TileType::VINE);
                }
            }
        }
        type = tempLevel.tileMap->getTile(x, y);
      }

      if (type == TileType::CAVE_ROCK) {
        bool top = isSolid(x, y - 1);
        bool bottom = isSolid(x, y + 1);

        TileType newTile = TileType::CAVE_ROCK;
        if (!top && !bottom) {
          newTile = TileType::CAVE_UP_DOWN_ORIENTED;
        } else if (!top) {
          newTile = TileType::CAVE_UP_ORIENTED;
        } else if (!bottom) {
          newTile = TileType::CAVE_DOWN_ORIENTED;
        } else {
          newTile = TileType::CAVE_REGULAR; // Inner dirt
          bool isOuterBoundary = (x == 0 || x == tempLevel.tileMap->getWidth() - 1 || 
                                  y == 0 || y == tempLevel.tileMap->getHeight() - 1);
          if (!isOuterBoundary) {
              int r = GetRandomValue(1, 100);
              if (r <= 5)
                newTile = TileType::CAVE_SOME_GOLD;
              else if (r <= 7)
                newTile = TileType::CAVE_MUCH_GOLD;
          }
        }
        tempLevel.tileMap->setTile(x, y, newTile);
      } else if (type == TileType::LUSH_ROCK) {
        bool top = (y > 0 && isSolid(x, y - 1));
        bool bottom = (y < tempLevel.tileMap->getHeight() - 1 && isSolid(x, y + 1));
        bool left = (x > 0 && isSolid(x - 1, y));
        bool right = (x < tempLevel.tileMap->getWidth() - 1 && isSolid(x + 1, y));

        TileType newTile = TileType::LUSH_ROCK;

        if (!top) {
          // The block itself gets a grassy top
          int r = GetRandomValue(1, 3);
          if (r == 1) newTile = TileType::LUSH_UP_1;
          else if (r == 2) newTile = TileType::LUSH_UP_2;
          else newTile = TileType::LUSH_UP_3;
          
          // We also have a chance to spawn tall grass in the empty space above
          int r2 = GetRandomValue(1, 2);
          if (r2 == 1) borderTiles.push_back({{x, y - 1}, TileType::LUSH_TOP_1});
          else if (r2 == 2) borderTiles.push_back({{x, y - 1}, TileType::LUSH_TOP_2});
        } 
        if (!bottom) {
          // The block itself gets a bottom texture
          newTile = TileType::LUSH_DOWN;
          
          // We also have a chance to spawn hanging vines in the empty space below
          int r = GetRandomValue(1, 2);
          if (r == 1) borderTiles.push_back({{x, y + 1}, TileType::LUSH_BOTTOM_1});
          else borderTiles.push_back({{x, y + 1}, TileType::LUSH_BOTTOM_2});
        } 
        if (!left) {
          borderTiles.push_back({{x - 1, y}, TileType::LUSH_LEFT});
        } 
        if (!right) {
          borderTiles.push_back({{x + 1, y}, TileType::LUSH_RIGHT});
        }
        
        if (top && bottom && left && right) {
          newTile = TileType::LUSH_ROCK; // Inner dirt
          bool isOuterBoundary = (x == 0 || x == tempLevel.tileMap->getWidth() - 1 || 
                                  y == 0 || y == tempLevel.tileMap->getHeight() - 1);
          if (!isOuterBoundary) {
              int r = GetRandomValue(1, 100);
              if (r <= 5)
                newTile = TileType::LUSH_SOME_GOLD;
              else if (r <= 7)
                newTile = TileType::LUSH_MUCH_GOLD;
          }
        }
        tempLevel.tileMap->setTile(x, y, newTile);
      } else if (type == TileType::TEMPLE_ROCK) {
        bool top = (y > 0 && isSolid(x, y - 1));
        bool bottom = (y < tempLevel.tileMap->getHeight() - 1 && isSolid(x, y + 1));
        bool left = (x > 0 && isSolid(x - 1, y));
        bool right = (x < tempLevel.tileMap->getWidth() - 1 && isSolid(x + 1, y));

        TileType newTile = TileType::TEMPLE_ROCK;

        if (!top) {
          int r = GetRandomValue(1, 8);
          if (r == 1) newTile = TileType::TEMPLE_UP_1;
          else if (r == 2) newTile = TileType::TEMPLE_UP_2;
          else if (r == 3) newTile = TileType::TEMPLE_UP_3;
          else if (r == 4) newTile = TileType::TEMPLE_UP_4;
          else if (r == 5) newTile = TileType::TEMPLE_UP_5;
          else if (r == 6) newTile = TileType::TEMPLE_UP_6;
          else if (r == 7) newTile = TileType::TEMPLE_UP_7;
          else newTile = TileType::TEMPLE_UP_8;
          
          int r2 = GetRandomValue(1, 2);
          if (r2 == 1) borderTiles.push_back({{x, y - 1}, TileType::TEMPLE_TOP_1});
          else borderTiles.push_back({{x, y - 1}, TileType::TEMPLE_TOP_2});
        } 
        if (!bottom) {
          newTile = TileType::TEMPLE_DOWN;
          borderTiles.push_back({{x, y + 1}, TileType::TEMPLE_BOTTOM});
        } 
        if (!left) {
          borderTiles.push_back({{x - 1, y}, TileType::TEMPLE_LEFT});
        } 
        if (!right) {
          borderTiles.push_back({{x + 1, y}, TileType::TEMPLE_RIGHT});
        }
        
        if (top && bottom && left && right) {
          newTile = TileType::TEMPLE_ROCK; // Inner dirt
          bool isOuterBoundary = (x == 0 || x == tempLevel.tileMap->getWidth() - 1 || 
                                  y == 0 || y == tempLevel.tileMap->getHeight() - 1);
          if (!isOuterBoundary) {
              int r = GetRandomValue(1, 100);
              if (r <= 5)
                newTile = TileType::TEMPLE_SOME_GOLD;
              else if (r <= 7)
                newTile = TileType::TEMPLE_MUCH_GOLD;
          }
        }
        tempLevel.tileMap->setTile(x, y, newTile);
      }
    }
  }

  TraceLog(LOG_INFO, "BorderTiles total generated: %d", (int)borderTiles.size());
      int placedBorders = 0;
      for (auto& border : borderTiles) {
          TileType currentTile = tempLevel.tileMap->getTile(border.first.x, border.first.y);
          if (currentTile == TileType::NOTHING) {
              tempLevel.tileMap->setTile(border.first.x, border.first.y, border.second);
              placedBorders++;
          }
      }
  TraceLog(LOG_INFO, "BorderTiles actually placed on NOTHING: %d", placedBorders);

  // Spawn Custom Entities pass
  for (int y = 0; y < tempLevel.tileMap->getHeight(); y++) {
    for (int x = 0; x < tempLevel.tileMap->getWidth(); x++) {
      TileType type = tempLevel.tileMap->getTile(x, y);
      float px = x * 32.0f;
      float py = y * 32.0f;


      if (type == TileType::CHEST) {
        type = TileType::NOTHING;
        auto chest = EntityFactory::createItem('C', px, py);
        if (chest)
          tempLevel.items.push_back(std::move(chest));
      } else if (type == TileType::ENEMY_SNAKE) {
        type = TileType::NOTHING;
        auto enemy = EntityFactory::createEnemy('S', px, py);
        if (enemy)
          tempLevel.dynamicEntities.push_back(std::move(enemy));
      } else if (type == TileType::ENEMY_BAT) {
        type = TileType::NOTHING;
        auto enemy = EntityFactory::createEnemy('B', px, py);
        if (enemy)
          tempLevel.dynamicEntities.push_back(std::move(enemy));
      } else if (type == TileType::ENEMY_SPIDER) {
        type = TileType::NOTHING;
        auto enemy = EntityFactory::createEnemy('P', px, py);
        if (enemy)
          tempLevel.dynamicEntities.push_back(std::move(enemy));
      } else if (type == TileType::LAVA) {
        type = TileType::NOTHING;
        tempLevel.initialLiquids.push_back({x, y, LiquidType::LAVA});
      } else if (type == TileType::WATER) {
        type = TileType::NOTHING;
        tempLevel.initialLiquids.push_back({x, y, LiquidType::WATER});
      } else if (type == TileType::SPIKE_TRAP) {
        type = TileType::NOTHING;
        auto spike = EntityFactory::createEnemy('^', px, py + 8.0f);
        if (spike)
          tempLevel.dynamicEntities.push_back(std::move(spike));
      } else if (type == TileType::ARROW_TRAP_LEFT) {
        auto trap = EntityFactory::createTrap('<', px, py);
        if (trap)
          tempLevel.traps.push_back(std::move(trap));
      } else if (type == TileType::ARROW_TRAP_RIGHT) {
        auto trap = EntityFactory::createTrap('>', px, py);
        if (trap)
          tempLevel.traps.push_back(std::move(trap));
      }

      tempLevel.tileMap->setTile(x, y, type);
    }
  }

  physics = std::make_unique<PhysicsSystem>(tempLevel.tileMap.get());
  player = std::make_unique<Player>(
      tempLevel.playerSpawn.x, tempLevel.playerSpawn.y,
      GameManager::getInstance()->getSelectedCharacter());
  player->setTileMap(tempLevel.tileMap.get());
  player->startDoorSpawnAnim();

  minimap = std::make_unique<Minimap>(tempLevel.exitPos);

  lighting = std::make_unique<LightingSystem>(tempLevel.tileMap->getWidth(),
                                              tempLevel.tileMap->getHeight());
                                              
  // Base light increased by 6% (+0.06f) from original. Decreases by 10% (0.10f) per floor.
  int currentFloor = GameManager::getInstance()->getFloor();
  float decrement = (currentFloor - 1) * 0.10f;
  Vector3 baseLight = {0.21f - decrement, 0.21f - decrement, 0.31f - decrement};
  
  // Ensure it doesn't go completely pitch black
  baseLight.x = std::max(0.05f, baseLight.x);
  baseLight.y = std::max(0.05f, baseLight.y);
  baseLight.z = std::max(0.05f, baseLight.z);
  
  lighting->setAmbientLight(baseLight);

  liquids = std::make_unique<LiquidSimulator>(tempLevel.tileMap.get());
  player->setLiquidSimulator(liquids.get());
  for (const auto &liq : tempLevel.initialLiquids) {
    liquids->addLiquid(liq.gx, liq.gy, 255, liq.type);
  }

  if (tempLevel.modifier == FloorModifier::FLOODED_FLOOR) {
    liquids->applyFloodedFloorModifier(15);
  }

  camera.target = Vector2{player->getX(), player->getY()};

  cameraShakeTimer = 0.0f;
  cameraShakeIntensity = 0.0f;

  combo = std::make_unique<ComboSystem>();
  // Initialize shop
  shop = std::make_unique<ShopSystem>();
  shop->initializeFromItems(tempLevel.items,
                            GameManager::getInstance()->getFloor());

  if (tempLevel.modifier == FloorModifier::CURSED_FLOOR) {
    tempLevel.difficulty.treasureValueMultiplier *= 2;
    tempLevel.difficulty.ghostTimerSeconds = std::max(30.0f, tempLevel.difficulty.ghostTimerSeconds / 2.0f);
  }

  GameManager::getInstance()->setGhostTimer(tempLevel.difficulty.ghostTimerSeconds);

  EventBus::getInstance()->clearListeners(EventType::EVENT_GOLD_COLLECTED);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_GOLD_COLLECTED, [this](EventData data) {
        if (this->combo)
          this->combo->onTreasureCollected(
              data.amount * this->tempLevel.difficulty.treasureValueMultiplier,
              data.worldX, data.worldY);
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_ENEMY_KILLED);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_ENEMY_KILLED, [this](EventData data) {
        AudioManager::getInstance()->playSFX("xdie");
        if (this->combo)
          this->combo->onEnemyKilled(data.amount, data.worldX, data.worldY);
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_ITEM);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_ITEM, [this](EventData data) {
        auto item =
            EntityFactory::createItem(data.amount, data.worldX, data.worldY);
        if (item) {
          item->setVelocity(data.vx, data.vy);
          this->pendingItems.push_back(std::move(item));
        }
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_BOMB);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_BOMB, [this](EventData data) {
        auto bomb =
            std::make_unique<Bomb>(data.worldX, data.worldY, data.vx, data.vy);
        this->pendingEntities.push_back(std::move(bomb));
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_ROPE);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_ROPE, [this](EventData data) {
        auto ropeProj =
            std::make_unique<RopeProjectile>(data.worldX, data.worldY, data.vy, this->tempLevel.tileMap.get());
        this->pendingEntities.push_back(std::move(ropeProj));
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_ARROW);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_ARROW, [this](EventData data) {
        auto arrow =
            EntityFactory::createArrow(data.worldX, data.worldY, data.vx);
        this->pendingEntities.push_back(std::move(arrow));
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_FLAME);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_FLAME, [this](EventData data) {
        AudioManager::getInstance()->playSFX("xignite");
        auto flame = EntityFactory::createEnemy('F', data.worldX, data.worldY);
        if (flame) {
          if (data.vy != 0.0f) {
            flame->setVelocity(0.0f, data.vy);
          }
          this->pendingEntities.push_back(std::move(flame));
        }
      });
      
  auto spawnBloodParticles = [this](EventData data) {
    int numParticles = GetRandomValue(4, 8);
    for (int i = 0; i < numParticles; i++) {
        this->pendingEntities.push_back(EntityFactory::createBloodParticle(data.worldX, data.worldY));
    }
  };
  
  EventBus::getInstance()->clearListeners(EventType::EVENT_PLAYER_DAMAGED);
  EventBus::getInstance()->subscribe(EventType::EVENT_PLAYER_DAMAGED, spawnBloodParticles);
  
  EventBus::getInstance()->clearListeners(EventType::EVENT_ENEMY_DAMAGED);
  EventBus::getInstance()->subscribe(EventType::EVENT_ENEMY_DAMAGED, spawnBloodParticles);

  EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_LAVA_DRIP);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_SPAWN_LAVA_DRIP, [this](EventData data) {
        auto drip = std::make_unique<LavaDrip>(data.worldX, data.worldY);
        this->pendingEntities.push_back(std::move(drip));
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_ADD_LIQUID);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_ADD_LIQUID, [this](EventData data) {
        if (this->liquids) {
          this->liquids->addLiquid(data.gridX, data.gridY, 0,
                                   (LiquidType)data.amount);
        }
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_TERRAIN_DESTROYED);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_TERRAIN_DESTROYED, [this](EventData data) {
          int type = data.tileType;
          bool someGold = (type == 5 || type == 53 || type == 78);
          bool muchGold = (type == 6 || type == 52 || type == 77);

          if (someGold || muchGold) {
              Texture2D tex = EntityFactory::getTexture("assets/sprites/8x8/gold.png");
              
              // Spawn 3 Gold Chunks
              for (int i = 0; i < 3; i++) {
                  auto chunk = std::make_unique<LootPickup>(data.gridX * 32.0f + 14.0f, data.gridY * 32.0f + 14.0f, 4.0f, 4.0f, 100);
                  chunk->setSprite(tex, Rectangle{4, 4, 4, 4});
                  chunk->setVelocity(GetRandomValue(-150, 150), GetRandomValue(-300, -100));
                  pendingItems.push_back(std::move(chunk));
              }

              // Spawn 1 Gold Nugget if much gold
              if (muchGold) {
                  auto nugget = std::make_unique<LootPickup>(data.gridX * 32.0f + 12.0f, data.gridY * 32.0f + 12.0f, 8.0f, 8.0f, 500);
                  nugget->setSprite(tex, Rectangle{0, 0, 8, 8});
                  nugget->setVelocity(GetRandomValue(-100, 100), GetRandomValue(-350, -150));
                  pendingItems.push_back(std::move(nugget));
              }
          }
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_BOMB_EXPLODE);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_BOMB_EXPLODE, [this](EventData data) {
        AudioManager::getInstance()->playSFX("xexplosion");
        float explosionRadius = 80.0f; // roughly 2.5 tiles (32 * 2.5 = 80)

        // 1. Destroy Terrain and Evaporate Liquids
        int tx = (int)(data.worldX / 32.0f);
        int ty = (int)(data.worldY / 32.0f);
        for (int y = ty - 2; y <= ty + 2; y++) {
          for (int x = tx - 2; x <= tx + 2; x++) {
            if (x > 0 && x < tempLevel.tileMap->getWidth() - 1 && y > 0 &&
                y < tempLevel.tileMap->getHeight() - 1) {
              if (tempLevel.tileMap->isSolid(x, y)) {
                tempLevel.tileMap->destroyBlock(x, y);
              }
              // Evaporate liquid inside the blast radius
              liquids->removeLiquid(x, y);
            }
          }
        }

        // 2. Damage Player
        if (player) {
          float dx =
              player->getX() + player->getAABB().width / 2.0f - data.worldX;
          float dy =
              player->getY() + player->getAABB().height / 2.0f - data.worldY;
          float dist = std::sqrt(dx * dx + dy * dy);
          if (dist < explosionRadius) {
            player->takeDamage(1);
            player->setVelocity(dx > 0 ? 300.0f : -300.0f, -200.0f);
          }
        }

        // 3. Damage Entities
        for (auto &entity : tempLevel.dynamicEntities) {
          if (entity && entity->isAlive()) {
            if (Enemy *enemy = dynamic_cast<Enemy *>(entity.get())) {
              float dx =
                  enemy->getX() + enemy->getAABB().width / 2.0f - data.worldX;
              float dy =
                  enemy->getY() + enemy->getAABB().height / 2.0f - data.worldY;
              float dist = std::sqrt(dx * dx + dy * dy);
              if (dist < explosionRadius) {
                enemy->takeDamage(10);
                enemy->setVelocity(dx > 0 ? 300.0f : -300.0f, -200.0f);
              }
            }
          }
        }
        
        // 4. Add visual explosion flash and sprite
        explosionFlashes.push_back({data.worldX, data.worldY, 0.3f}); // 0.3s lifetime
        
        // Spawn the explosion visual entity (centered on bomb)
        pendingEntities.push_back(EntityFactory::createExplosion(data.worldX - 80.0f, data.worldY - 80.0f));

        // 5. Camera Shake
        if (player) {
            float dx = player->getX() - data.worldX;
            float dy = player->getY() - data.worldY;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < 600.0f) {
                this->cameraShakeTimer = 0.5f;
                this->cameraShakeIntensity = 10.0f * (1.0f - (dist / 600.0f));
            }
        }
      });

  // 4. Pass liquids to enemies
  for (auto &entity : tempLevel.dynamicEntities) {
    if (Enemy *enemy = dynamic_cast<Enemy *>(entity.get())) {
      enemy->setLiquidSim(liquids.get());
    }
  }
}

void PlayState::exit() {
  AudioManager::getInstance()->stopSFX("xletsexplore");
  physics.reset();
  player.reset();

  EventBus::getInstance()->clearAllListeners();
}

void PlayState::handleInput() {
  if (deathTimer >= 0.0f) return;
  
  if (IsKeyPressed(KEY_ESCAPE)) {
    game->pushState(GameStateType::PAUSE);
    return;
  }
  if (player) {
    player->handleInput();
  }
}

void PlayState::update(float dt) {
  if (GameManager::getInstance()->tickGhostTimer(dt)) {
    auto ghost = EntityFactory::createGhost(player->getX() - 600.0f,
                                            player->getY() - 600.0f);
    pendingEntities.push_back(std::move(ghost));
    AudioManager::getInstance()->playSFX("xghost");
    AudioManager::getInstance()->playBGM("mBoss");
  }

  // Merge pending items
  for (auto &item : pendingItems) {
    tempLevel.items.push_back(std::move(item));
  }
  pendingItems.clear();

  // Merge pending entities
  for (auto &ent : pendingEntities) {
    if (auto *enemy = dynamic_cast<Enemy *>(ent.get())) {
      enemy->setTileMap(tempLevel.tileMap.get());
      enemy->setLiquidSim(liquids.get());
    }
    tempLevel.dynamicEntities.push_back(std::move(ent));
  }
  pendingEntities.clear();

  if (player) {
    player->update(dt, nullptr);

    if (physics) {
      physics->resolveEntityTileCollision(player.get());

      for (auto &entity : tempLevel.dynamicEntities) {
        if (entity && entity->isAlive()) {
          entity->update(dt, player.get());
          entity->applyGravity(dt);
          physics->resolveEntityTileCollision(entity.get());

          if (liquids && liquids->isLavaAt(entity->getAABB())) {
            if (auto *enemy = dynamic_cast<Enemy *>(entity.get())) {
              // Only destroy enemies that aren't immune to lava (like Flame)
              enemy->takeDamage(999);
            }
          }
        }
      }

      for (auto &item : tempLevel.items) {
        if (item && !item->isPickedUp()) {
          if (item->isEmbedded) {
              int tx = static_cast<int>(item->getX() / 32.0f);
              int ty = static_cast<int>(item->getY() / 32.0f);
              if (!tempLevel.tileMap->isSolid(tx, ty)) {
                  item->isEmbedded = false;
                  item->setPassesThroughWalls(false);
              } else {
                  // Keep rendering it, but don't apply physics/updates
                  continue;
              }
          }
          item->update(dt, player.get());
          physics->resolveEntityTileCollision(item.get());
        }
      }

      for (auto &dec : tempLevel.decorations) {
        if (dec) {
          dec->update(dt, player.get());
        }
      }
    }

    // ---- Entity Collisions ----
    Rectangle pAABB = player->getAABB();
    bool whipActive = player->getIsWhipHitThisFrame();
    Rectangle whipBox = player->getWhipHitbox();

    // 1. Player vs DynamicEntities (Enemies/Ghost)
    for (auto &entity : tempLevel.dynamicEntities) {
      if (entity && entity->isAlive()) {
        if (auto *enemy = dynamic_cast<Enemy *>(entity.get())) {
          if (whipActive &&
              physics->checkAABBOverlap(whipBox, enemy->getAABB())) {
            enemy->takeDamage(1); // Whip does 1 damage
            enemy->setVelocity(player->getWhipHitbox().x > player->getX() ? 150.0f : -150.0f, -100.0f);
            AudioManager::getInstance()->playSFX("xhit");
            continue; // Skip collision damage this frame
          }

          if (physics->checkAABBOverlap(pAABB, enemy->getAABB())) {
            if (!dynamic_cast<Spike *>(enemy)) {
              player->takeDamage(enemy->getDamage());
            }
          }
        } else if (auto *arrow = dynamic_cast<Arrow *>(entity.get())) {
          if (arrow->isLethal() &&
              physics->checkAABBOverlap(pAABB, arrow->getAABB())) {
            player->takeDamage(1); // Take 1 heart damage
            player->setVelocity(arrow->getVelocityX() > 0 ? 300.0f : -300.0f,
                                -200.0f);
            arrow->destroy();
          }
        }
      }
    }

    // 2. Player vs Traps
    for (auto &trap : tempLevel.traps) {
      if (trap) {
        trap->updateTrap(dt, player.get(), tempLevel.dynamicEntities,
                         tempLevel.items, tempLevel.tileMap.get());

        if (trap->getDamage() > 0 &&
            physics->checkAABBOverlap(pAABB, trap->getAABB())) {
          if (!player->isInvincible()) {
            player->takeDamage(trap->getDamage());
            player->setVelocity(
                player->getX() < trap->getX() ? -250.0f : 250.0f, -200.0f);
          }
        }
      }
    }

    // 3. Enemies vs Traps
    for (auto &entity : tempLevel.dynamicEntities) {
      if (entity && entity->isAlive()) {
        Rectangle eAABB = entity->getAABB();
        for (auto &trap : tempLevel.traps) {
          if (trap && trap->getDamage() > 0 &&
              physics->checkAABBOverlap(eAABB, trap->getAABB())) {
            if (auto *enemy = dynamic_cast<Enemy *>(entity.get())) {
              enemy->takeDamage(trap->getDamage());
              enemy->setVelocity(
                  enemy->getX() < trap->getX() ? -200.0f : 200.0f, -150.0f);
            }
          }
        }
      }
    }

    // 4. DynamicEntity vs DynamicEntity (Soft Push-Out & Arrow Hits)
    for (size_t i = 0; i < tempLevel.dynamicEntities.size(); ++i) {
      if (!tempLevel.dynamicEntities[i] ||
          !tempLevel.dynamicEntities[i]->isAlive())
        continue;
      for (size_t j = i + 1; j < tempLevel.dynamicEntities.size(); ++j) {
        if (!tempLevel.dynamicEntities[j] ||
            !tempLevel.dynamicEntities[j]->isAlive())
          continue;

        auto &e1 = tempLevel.dynamicEntities[i];
        auto &e2 = tempLevel.dynamicEntities[j];
        Rectangle a = e1->getAABB();
        Rectangle b = e2->getAABB();

        if (physics->checkAABBOverlap(a, b)) {
          Arrow *arrow = dynamic_cast<Arrow *>(e1.get());
          Enemy *enemy = dynamic_cast<Enemy *>(e2.get());

          if (!arrow) {
            arrow = dynamic_cast<Arrow *>(e2.get());
            enemy = dynamic_cast<Enemy *>(e1.get());
          }

          if (arrow && enemy && arrow->isLethal()) {
            enemy->takeDamage(100); // Instantly kill enemy
            arrow->destroy();
            // Arrow is destroyed, stop processing it if it was e1 or e2
            if (!e1->isAlive())
              break;
            if (!e2->isAlive())
              continue;
          } else if (dynamic_cast<Spike *>(e1.get()) &&
                     dynamic_cast<Enemy *>(e2.get())) {
            auto spike = dynamic_cast<Spike *>(e1.get());
            auto otherEnemy = dynamic_cast<Enemy *>(e2.get());
            if (otherEnemy->getVelocityY() > 10.0f) {
              otherEnemy->takeDamage(100);
              spike->setBlood();
            }
          } else if (dynamic_cast<Spike *>(e2.get()) &&
                     dynamic_cast<Enemy *>(e1.get())) {
            auto spike = dynamic_cast<Spike *>(e2.get());
            auto otherEnemy = dynamic_cast<Enemy *>(e1.get());
            if (otherEnemy->getVelocityY() > 10.0f) {
              otherEnemy->takeDamage(100);
              spike->setBlood();
            }
          } else if (dynamic_cast<Enemy *>(e1.get()) &&
                     dynamic_cast<Enemy *>(e2.get()) &&
                     !dynamic_cast<Spike *>(e1.get()) &&
                     !dynamic_cast<Spike *>(e2.get())) {
            // Push apart horizontally
            if (a.x < b.x) {
              e1->setVelocity(e1->getVelocityX() - 50.0f, e1->getVelocityY());
              e2->setVelocity(e2->getVelocityX() + 50.0f, e2->getVelocityY());
            } else {
              e1->setVelocity(e1->getVelocityX() + 50.0f, e1->getVelocityY());
              e2->setVelocity(e2->getVelocityX() - 50.0f, e2->getVelocityY());
            }
          }
        }
      }
    }

    // 5. Player vs Items
    // Slightly expand the player's AABB for more forgiving item collection
    Rectangle pickupBox = pAABB;
    pickupBox.x -= 4.0f;
    pickupBox.y -= 4.0f;
    pickupBox.width += 8.0f;
    pickupBox.height += 8.0f;

    if (player->getHealth() > 0) {
      for (auto &item : tempLevel.items) {
        if (item && !item->isPickedUp() && !item->isShopItem &&
            item->getType() != ItemType::CHEST) {
          if (physics->checkAABBOverlap(pickupBox, item->getAABB())) {
            item->activate(player.get());
          }
        }
      }
    }

    // Merge pending items
    for (auto &item : pendingItems) {
      tempLevel.items.push_back(std::move(item));
    }
    pendingItems.clear();

    // Camera smooth follow with boundary clamping
    Vector2 desiredTarget = {player->getX() + 16, player->getY() + 16};

    float mapWidth = 40.0f * 32.0f; // 4 rooms * 10 tiles * 32 pixels = 1280
    float mapHeight = 40.0f * 32.0f;

    float halfScreenWidth = (GetScreenWidth() / 2.0f) / camera.zoom;
    float halfScreenHeight = (GetScreenHeight() / 2.0f) / camera.zoom;

    // Allow the camera to see 4 tiles (128 pixels) into the infinite bedrock
    float borderPixelsX = 4.0f * 32.0f;
    float borderPixelsY = 4.0f * 32.0f;

    // Clamp X
    if (desiredTarget.x < halfScreenWidth - borderPixelsX)
      desiredTarget.x = halfScreenWidth - borderPixelsX;
    if (desiredTarget.x > mapWidth + borderPixelsX - halfScreenWidth)
      desiredTarget.x = mapWidth + borderPixelsX - halfScreenWidth;

    // Clamp Y
    if (desiredTarget.y < halfScreenHeight - borderPixelsY)
      desiredTarget.y = halfScreenHeight - borderPixelsY;
    if (desiredTarget.y > mapHeight + borderPixelsY - halfScreenHeight)
      desiredTarget.y = mapHeight + borderPixelsY - halfScreenHeight;

    if (deathTimer < 0.0f) {
      camera.target = Vector2Lerp(camera.target, desiredTarget, 5.0f * dt);
    }

    if (cameraShakeTimer > 0.0f) {
      cameraShakeTimer -= dt;
      float offsetX = ((float)GetRandomValue(-100, 100) / 100.0f) * cameraShakeIntensity;
      float offsetY = ((float)GetRandomValue(-100, 100) / 100.0f) * cameraShakeIntensity;
      camera.offset.x = (GetScreenWidth() / 2.0f) + offsetX;
      camera.offset.y = (GetScreenHeight() / 2.0f) + offsetY;
    } else {
      camera.offset.x = GetScreenWidth() / 2.0f;
      camera.offset.y = GetScreenHeight() / 2.0f;
    }

    if (liquids) {
      liquids->update(dt);
      liquids->updateSpurts(dt, player->getX(), player->getY());
    }

    if (minimap && player->getHealth() > 0) {
      minimap->update(player->getX(), player->getY());
    }

    if (lighting) {
      lighting->clearLights();

      // Add Player Torch
      // Use the exact float position (in tile coordinates) for smooth, sub-tile
      // distance falloff
      Rectangle pRect = player->getAABB();
      float trueX = (pRect.x + pRect.width / 2.0f) / 32.0f;
      float trueY = (pRect.y + pRect.height / 2.0f) / 32.0f;

      // Create a smooth organic flicker using composite sine waves and a tiny
      // bit of random noise
      double time = GetTime();
      float flicker = 0.0f;
      flicker += std::sin(time * 12.0) * 0.02f;
      flicker += std::sin(time * 23.0) * 0.015f;
      flicker += std::sin(time * 5.0) * 0.01f;
      flicker += ((float)GetRandomValue(-100, 100) / 100.0f) *
                 0.005f; // micro crackles

      float intensity = 0.95f + flicker;
      float radius = 4.5f + (flicker * 1.5f);

      if (tempLevel.modifier == FloorModifier::DARK_FLOOR) {
        radius *= 0.5f;
      }

      Vector3 torchColor = {intensity, intensity * 0.9f, intensity * 0.6f};
      lighting->addLight(trueX, trueY, torchColor, radius);

      // Shop Lantern glow
      if (tempLevel.shopArea.width > 0) {
        float shopCx =
            (tempLevel.shopArea.x + tempLevel.shopArea.width / 2.0f) /
            tempLevel.tileMap->getTileSize();
        float shopCy = (tempLevel.shopArea.y + 160.0f) /
                       tempLevel.tileMap->getTileSize(); // At y=5 in the room
        double t = GetTime();
        float flicker = std::sin(t * 15.0) * 0.03f + std::sin(t * 22.0) * 0.02f;
        Vector3 lanternColor = {1.4f, 1.2f, 0.4f}; // Bright gold
        lighting->addLight(shopCx, shopCy, lanternColor, 8.0f + (flicker * 2.0f));
      }

      // Add Lava Glow (Optimized: Camera Culling + Surface Exposure)
      Vector2 screenTL = GetScreenToWorld2D(Vector2{0, 0}, camera);
      Vector2 screenBR = GetScreenToWorld2D(Vector2{(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

      int startX = std::max(0, (int)(screenTL.x / tempLevel.tileMap->getTileSize()) - 5);
      int startY = std::max(0, (int)(screenTL.y / tempLevel.tileMap->getTileSize()) - 5);
      int endX = std::min(tempLevel.tileMap->getWidth() - 1, (int)(screenBR.x / tempLevel.tileMap->getTileSize()) + 5);
      int endY = std::min(tempLevel.tileMap->getHeight() - 1, (int)(screenBR.y / tempLevel.tileMap->getTileSize()) + 5);

      for (int y = startY; y <= endY; y++) {
          for (int x = startX; x <= endX; x++) {
              if (tempLevel.tileMap->getTile(x, y) == TileType::LAVA) {
                  // Only emit light if exposed to air
                  bool exposed = false;
                  if (x > 0 && tempLevel.tileMap->getTile(x - 1, y) == TileType::NOTHING) exposed = true;
                  else if (x < tempLevel.tileMap->getWidth() - 1 && tempLevel.tileMap->getTile(x + 1, y) == TileType::NOTHING) exposed = true;
                  else if (y > 0 && tempLevel.tileMap->getTile(x, y - 1) == TileType::NOTHING) exposed = true;
                  else if (y < tempLevel.tileMap->getHeight() - 1 && tempLevel.tileMap->getTile(x, y + 1) == TileType::NOTHING) exposed = true;

                  if (exposed) {
                      double t = GetTime();
                      float lavaFlicker = std::sin(t * 8.0 + x * 0.5 + y) * 0.05f;
                      Vector3 lavaColor = {1.0f + lavaFlicker, 0.3f + lavaFlicker*0.5f, 0.0f};
                      lighting->addLight(x + 0.5f, y + 0.5f, lavaColor, 3.5f);
                  }
              }
          }
      }

      // Process Explosion Flashes
      for (auto it = explosionFlashes.begin(); it != explosionFlashes.end(); ) {
          it->timer -= dt;
          if (it->timer <= 0) {
              it = explosionFlashes.erase(it);
          } else {
              float progress = it->timer / 0.3f;
              Vector3 expColor = {1.5f * progress, 1.2f * progress, 0.8f * progress};
              lighting->addLight(it->x / tempLevel.tileMap->getTileSize(), it->y / tempLevel.tileMap->getTileSize(), expColor, 10.0f * progress);
              ++it;
          }
      }

      lighting->update(tempLevel.tileMap.get());
    }

    // Handle Shop UI Interaction
    if (shop && tempLevel.shopArea.width > 0 && player->getHealth() > 0) {
      Rectangle pRect = player->getAABB();
      if (CheckCollisionRecs(pRect, tempLevel.shopArea)) {
        if (IsKeyPressed(KEY_Y) && !IsKeyDown(KEY_UP) && !IsKeyDown(KEY_W)) {
          shop->setPlayerInShop(!shop->isPlayerInShop());
        } else if (IsKeyPressed(KEY_ESCAPE) && shop->isPlayerInShop()) {
          shop->setPlayerInShop(false);
        }
        if (shop->isPlayerInShop()) {
          shop->handleInput(player.get());
        }
      } else {
        shop->setPlayerInShop(false);
      }
    }

    // (Auto-pickup loop removed; merged with block 5)
    if (combo) {
      combo->update(dt);
    }

    // Death check
    if (!player->isAlive()) {
      if (deathTimer < 0.0f) {
        // Just died, start the sequence
        deathTimer = 2.5f;
        player->setPassesThroughWalls(true);
        player->setVelocity(0.0f, -400.0f); // Classic death hop
      }
    }
    
    // Process death sequence timer
    if (deathTimer >= 0.0f) {
      deathTimer -= dt;
      if (deathTimer <= 0.0f) {
        game->changeState(GameStateType::GAME_OVER);
        return;
      }
    }

    // Exit door animation completion & check
    if (player->isDoorAnimPlaying()) {
      if (player->isDoorAnimFinished()) {
        Rectangle pRect = player->getAABB();
        int cx = pRect.x + pRect.width / 2;
        int cy = pRect.y + pRect.height / 2;
        int tx = cx / tempLevel.tileMap->getTileSize();
        int ty = cy / tempLevel.tileMap->getTileSize();

        if (tx >= 0 && tx < tempLevel.tileMap->getWidth() && ty >= 0 &&
            ty < tempLevel.tileMap->getHeight()) {
          if (tempLevel.tileMap->getTile(tx, ty) == TileType::EXIT) {
            GameManager::getInstance()->syncPlayerStats(
                player->getHealth(), player->getBombs(), player->getRopes(),
                player->getGold());
            game->changeState(GameStateType::TRANSITION);
            return;
          }
        }
      }
    } else {
      // Exit and Chest interaction (requires manual UP+Y input)
      if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && IsKeyPressed(KEY_Y)) {
        Rectangle pRect = player->getAABB();
        
        // 1. Check for Chests first
        for (auto &item : tempLevel.items) {
            if (item && item->getType() == ItemType::CHEST) {
                if (physics->checkAABBOverlap(pRect, item->getAABB())) {
                    item->activate(player.get());
                }
            }
        }

        // 2. Check for Exit
        int cx = pRect.x + pRect.width / 2;
        int cy = pRect.y + pRect.height / 2;
        int tx = cx / tempLevel.tileMap->getTileSize();
        int ty = cy / tempLevel.tileMap->getTileSize();

        if (tx >= 0 && tx < tempLevel.tileMap->getWidth() && ty >= 0 &&
            ty < tempLevel.tileMap->getHeight()) {
          if (tempLevel.tileMap->getTile(tx, ty) == TileType::EXIT) {
            // Horizontally snap player to center of the exit door
            float targetX = tx * 32.0f + 8.0f;
            player->move(targetX - player->getX(), 0.0f);
            player->startDoorEnterAnim();
          }
        }
      }
    }
  }
}

void PlayState::render() {
  ClearBackground(BLACK);

  BeginMode2D(camera);

  // Draw grid to visualize movement for debugging
  for (int i = -1000; i < 1000; i += 32) {
    DrawLine(i, -1000, i, 1000, DARKGRAY);
    DrawLine(-1000, i, 1000, i, DARKGRAY);
  }

  if (tempLevel.tileMap) {
    tempLevel.tileMap->renderParallaxBackground(camera);
    if (lighting) {
      tempLevel.tileMap->render(camera, lighting->getLightMap(),
                                false); // Background pass
    }

    // Shop Lantern is now a Lamp decoration entity and rendered below.
  }

  auto getEntityLight = [&](float x, float y, float w, float h) -> float {
    if (!lighting || !tempLevel.tileMap)
      return 1.0f;
    int tx = static_cast<int>((x + w / 2) / tempLevel.tileMap->getTileSize());
    int ty = static_cast<int>((y + h / 2) / tempLevel.tileMap->getTileSize());
    const auto &lMap = lighting->getLightMap();
    if (ty >= 0 && static_cast<size_t>(ty) < lMap.size() && tx >= 0 && static_cast<size_t>(tx) < lMap[ty].size()) {
      return (lMap[ty][tx].x + lMap[ty][tx].y + lMap[ty][tx].z) / 3.0f;
    }
    return 1.0f;
  };

  for (auto &item : tempLevel.items) {
    if (item && item->isAlive() && !item->isPickedUp()) {
      item->render(getEntityLight(item->getX(), item->getY(),
                                  item->getAABB().width,
                                  item->getAABB().height));
    }
  }
  for (auto &dec : tempLevel.decorations) {
    if (dec && dec->isAlive()) {
      dec->render(getEntityLight(dec->getX(), dec->getY(), dec->getAABB().width,
                                 dec->getAABB().height));
    }
  }
  for (auto &trap : tempLevel.traps) {
    if (trap && trap->isAlive()) {
      trap->render(getEntityLight(trap->getX(), trap->getY(),
                                  trap->getAABB().width,
                                  trap->getAABB().height));
    }
  }
  for (auto &enemy : tempLevel.dynamicEntities) {
    if (enemy && enemy->isAlive()) {
      // Draw all dynamic entities EXCEPT the ghost
      if (!dynamic_cast<NemesisGhost *>(enemy.get())) {
        enemy->render(getEntityLight(enemy->getX(), enemy->getY(),
                                     enemy->getAABB().width,
                                     enemy->getAABB().height));
      }
    }
  }

  if (player) {
    player->render(getEntityLight(player->getX(), player->getY(),
                                  player->getAABB().width,
                                  player->getAABB().height));
  }

  // Render foreground tiles LAST so they overlap the player's head and entities
  if (tempLevel.tileMap && lighting) {
    if (liquids)
      liquids->render(camera);
    tempLevel.tileMap->render(camera, lighting->getLightMap(),
                              true); // Foreground pass (Solid blocks)
  }


  // Render Ghost OVER foreground tiles as a transparent shadow
  for (auto &enemy : tempLevel.dynamicEntities) {
    if (enemy && enemy->isAlive()) {
      if (dynamic_cast<NemesisGhost *>(enemy.get())) {
        enemy->render(getEntityLight(enemy->getX(), enemy->getY(),
                                     enemy->getAABB().width,
                                     enemy->getAABB().height));
      }
    }
  }

  if (combo) {
    combo->render(game->getFont());
  }

  EndMode2D();

  if (minimap) {
    minimap->render();
  }

  // ---- Render HUD ----
  if (player) {
    float scale = 4.0f; // Scale up the HUD
    float iconSize = 16.0f * scale;
    float fontSize = 30.0f;

    // Helper lambda to draw an icon and text
    auto drawHudElement = [&](int iconIndex, const char *text, float x,
                              float y) {
      Rectangle src = {iconIndex * 16.0f, 0.0f, 16.0f, 16.0f};
      Rectangle dest = {x, y, iconSize, iconSize};
      DrawTexturePro(hudIcons, src, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);

      // Text offset to align with the icon
      float textY = y + (iconSize / 2.0f) - (fontSize / 2.0f);
      DrawTextEx(game->getFont(), text, {x + iconSize + 10.0f, textY}, fontSize,
                 2.0f, WHITE);
    };

    float startX = 20.0f;
    float startY = 20.0f;

    // 0: Heart, 2: Rope, 3: Bomb, 4: Gold
    drawHudElement(0, TextFormat("%d", player->getHealth()), startX, startY);
    drawHudElement(3, TextFormat("%d", player->getBombs()), startX + 180.0f,
                   startY);
    drawHudElement(2, TextFormat("%d", player->getRopes()), startX + 360.0f,
                   startY);
    drawHudElement(4, TextFormat("%d", player->getGold()), startX + 540.0f,
                   startY);

    // Floor on the right side
    float floorY = startY + (iconSize / 2.0f) - (fontSize / 2.0f);
    const char *floorText =
        TextFormat("FLOOR %d", GameManager::getInstance()->getFloor());
    Vector2 floorSize =
        MeasureTextEx(game->getFont(), floorText, fontSize, 2.0f);
    DrawTextEx(game->getFont(), floorText,
               {1280.0f - 20.0f - floorSize.x, floorY}, fontSize, 2.0f, WHITE);
  }

  if (combo) {
    combo->renderHUD(game->getFont());
  }

  if (shop) {
    if (shop->isPlayerInShop()) {
      shop->render(game->getFont());
    } else if (tempLevel.shopArea.width > 0 && player && player->getHealth() > 0 &&
               CheckCollisionRecs(player->getAABB(), tempLevel.shopArea)) {
      const char *text = "PRESS 'Y' TO OPEN OR CLOSE SHOP";
      float fontSize = 25.0f;
      Vector2 textSize = MeasureTextEx(game->getFont(), text, fontSize, 2.0f);
      DrawTextEx(game->getFont(), text,
                 {(1280.0f - textSize.x) / 2.0f, 720.0f - 100.0f}, fontSize,
                 2.0f, WHITE);
    }
  }
}

/*
=======================================================
=========================PAUSE=========================
=======================================================
*/

void PauseState::enter() {
  AudioManager::getInstance()->playSFX("xpause");
}

void PauseState::exit() {}

void PauseState::handleInput() {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
    selectedIndex = (selectedIndex == 0) ? 1 : 0;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    if (selectedIndex == 0) {
      game->popState();
    } else {
      game->changeState(GameStateType::MENU);
    }
  }
}

void PauseState::update(float dt) {}

void PauseState::render() {
  DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 150});

  drawCenteredText("PAUSED", GetScreenHeight() / 2 - 100, 60.0f, WHITE);

  Color resumeColor = (selectedIndex == 0) ? YELLOW : GRAY;
  Color quitColor = (selectedIndex == 1) ? YELLOW : GRAY;

  drawCenteredText("RESUME", GetScreenHeight() / 2, 40.0f, resumeColor);
  drawCenteredText("QUIT", GetScreenHeight() / 2 + 60, 40.0f, quitColor);
}

/*
=======================================================
=========================GAMEOVER======================
=======================================================
*/

void GameOverState::enter() {
  finalScore = GameManager::getInstance()->getScore();
  finalFloor = GameManager::getInstance()->getFloor();
  nameEntered = false;
  letterCount = 0;
  nameInput[0] = '\0';
}

void GameOverState::exit() {}

void GameOverState::handleInput() {
  if (!nameEntered) {
    int key = GetCharPressed();
    while (key > 0) {
      if ((key >= 32) && (key <= 125) && (letterCount < 3)) {
        nameInput[letterCount] = (char)key;
        nameInput[letterCount + 1] = '\0';
        letterCount++;
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (letterCount > 0) {
        letterCount--;
        nameInput[letterCount] = '\0';
      }
    }

    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
      nameEntered = true;
      GameManager::getInstance()->saveHighScore(std::string(nameInput));
      leaderboard = GameManager::getInstance()->loadHighScores();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
      nameEntered = true;
      leaderboard = GameManager::getInstance()->loadHighScores();
    }
  } else {
    if (IsKeyPressed(KEY_ENTER)) {
      game->changeState(GameStateType::MENU);
    }
  }
}

void GameOverState::update(float dt) {}

void GameOverState::render() {
  ClearBackground(MAROON);

  if (!nameEntered) {
    drawCenteredText("GAME OVER", GetScreenHeight() / 2 - 120, 70.0f, WHITE);
    drawCenteredText(TextFormat("SCORE: %d", finalScore),
                     GetScreenHeight() / 2 - 20, 40.0f, GOLD);
    drawCenteredText(TextFormat("FLOOR REACHED: %d", finalFloor),
                     GetScreenHeight() / 2 + 40, 30.0f, LIGHTGRAY);
    drawCenteredText("ENTER INITIALS:", GetScreenHeight() / 2 + 110, 30.0f,
                     WHITE);

    std::string displayStr = nameInput;
    if (((int)(GetTime() * 2)) % 2 == 0 && letterCount < 3) {
      displayStr += "_";
    }
    drawCenteredText(displayStr.c_str(), GetScreenHeight() / 2 + 150, 50.0f,
                     YELLOW);
    drawCenteredText("PRESS ESC TO SKIP", GetScreenHeight() - 100.0f, 20.0f,
                     LIGHTGRAY);
  } else {
    drawCenteredText("HIGH SCORES", 80.0f, 60.0f, GOLD);

    int yOffset = 180;
    int count = 0;
    for (const auto &entry : leaderboard) {
      if (count >= 5)
        break;
      std::string text =
          TextFormat("%d. %s - %d pts (Floor %d)", count + 1,
                     entry.name.c_str(), entry.score, entry.floorsReached);
      drawCenteredText(text.c_str(), yOffset, 30.0f, WHITE);
      yOffset += 60;
      count++;
    }

    if (((int)(GetTime() * 2)) % 2 == 0) {
      drawCenteredText("PRESS ENTER TO CONTINUE", GetScreenHeight() - 100.0f,
                       20.0f, LIGHTGRAY);
    }
  }
}

/*
=======================================================
=========================VICTORY=======================
=======================================================
*/
VictoryState::VictoryState() = default;
VictoryState::~VictoryState() = default;

void VictoryState::enter() {
  AudioManager::getInstance()->playBGM("mVictory");
  finalScore = GameManager::getInstance()->getScore();
  if (finalScore <= 0) {
    finalScore = GetRandomValue(65000, 195000); // Random score for test
  }
  finalFloor = GameManager::getInstance()->getFloor();
  if (finalFloor <= 0) {
    finalFloor = 16;
  }
  nameEntered = false;
  letterCount = 0;
  nameInput[0] = '\0';

  currentScene = EndingScene::SCENE1_TUNNEL;
  sceneTimer = 0.0f;
  stepsTaken = 0;
  stepAnimTimer = 0.0f;

  // 1. Build TunnelMap 60x30 matching TransitionState structure with temple tiles
  tunnelMap = std::make_unique<TileMap>(60, 30, 32);
  for (int y = 0; y < 30; y++) {
    for (int x = 0; x < 60; x++) {
      tunnelMap->setTile(x, y, TileType::TEMPLE_ROCK);
    }
  }
  // Tunnel opens from x = 10 to 59 (x = 0..9 is the solid left wall)
  for (int y = 8; y <= 11; y++) {
    for (int x = 10; x < 60; x++) {
      tunnelMap->setTile(x, y, TileType::NOTHING);
    }
  }
  // Entrance door on the left wall at x = 10, y = 11
  tunnelMap->setTile(10, 11, TileType::ENTRANCE);

  // Floor is row 12 (walkway)
  for (int x = 10; x < 60; x++) {
    tunnelMap->setTile(x, 12, TileType::TEMPLE_ROCK);
  }
  // Row 13 is empty space (cách 1 hàng trống)
  for (int x = 10; x < 60; x++) {
    tunnelMap->setTile(x, 13, TileType::NOTHING);
  }
  // Rows 14 to 29 are empty space for Lava (render lava all the way down)
  for (int y = 14; y < 30; y++) {
    for (int x = 0; x < 60; x++) {
      tunnelMap->setTile(x, y, TileType::NOTHING);
    }
  }

  tunnelMap->setTileset(EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png"));
  tunnelMap->setJungleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png"));
  tunnelMap->setTempleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png"));
  tunnelMap->setRopeTexture(EntityFactory::getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"));

  lavaSim = std::make_unique<LiquidSimulator>(tunnelMap.get());
  for (int y = 14; y < 30; y++) {
    for (int x = 0; x < 60; x++) {
      lavaSim->addLiquid(x, y, 255, LiquidType::LAVA);
    }
  }

  physics = std::make_unique<PhysicsSystem>(tunnelMap.get());
  cutscenePlayer = std::make_unique<Player>(
      10 * 32.0f + 8.0f, 11 * 32.0f + 8.0f,
      GameManager::getInstance()->getSelectedCharacter());
  cutscenePlayer->setTileMap(tunnelMap.get());
  cutscenePlayer->startDoorSpawnAnim();

  lighting = std::make_unique<LightingSystem>(60, 30);
  lighting->setAmbientLight({0.35f, 0.35f, 0.45f});

  camera.target = {(10 * 32.0f + 16.0f), (10 * 32.0f)};
  camera.offset = {(float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;

  for (int x = 0; x < 40; x++) {
    for (int y = 0; y < 10; y++) {
      sandGrid[x][y] = GetRandomValue(0, 1);
    }
  }

  CharacterType charType = GameManager::getInstance()->getSelectedCharacter();
  std::string playerTexPath = "assets/characters/explorer.png";
  if (charType == CharacterType::NINJA) {
      playerTexPath = "assets/characters/ninja.png";
  } else if (charType == CharacterType::TANK) {
      playerTexPath = "assets/characters/tank.png";
  }
  playerSpriteSheet = EntityFactory::getTexture(playerTexPath);

  skyTex = EntityFactory::getTexture("assets/ending/sEnd2BG.png");
  mountainTex = EntityFactory::getTexture("assets/ending/sBGEnd3.png");
  sandTex = EntityFactory::getTexture("assets/ending/sDesert.png");
  sand2Tex = EntityFactory::getTexture("assets/ending/sDesert2.png");
  sandTopTex = EntityFactory::getTexture("assets/ending/sDesertTop.png");
  palmTreeTex = EntityFactory::getTexture("assets/ending/sPalmTree.png");
  shrubTex = EntityFactory::getTexture("assets/ending/sShrub.png");
  bigTreasureTex = EntityFactory::getTexture("assets/ending/sBigTreasure.png");

  tallyStatus = 0;
  tallyTimer = 0.0f;
  currentTallyScore = 0.0f;
  gems.clear();
  fadeAlpha = 0.0f;
  blackScreenTimer = 0.0f;
}

void VictoryState::exit() {
  AudioManager::getInstance()->stopBGM();
}

void VictoryState::handleInput() {
  if (currentScene != EndingScene::SCENE5_SUMMARY) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
      currentScene = EndingScene::SCENE5_SUMMARY;
      sceneTimer = 0.0f;
      nameEntered = false;
      letterCount = 0;
      nameInput[0] = '\0';
    }
    return;
  }

  if (!nameEntered) {
    int key = GetCharPressed();
    while (key > 0) {
      if ((key >= 32) && (key <= 125) && (letterCount < 3)) {
        nameInput[letterCount] = (char)key;
        nameInput[letterCount + 1] = '\0';
        letterCount++;
      }
      key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (letterCount > 0) {
        letterCount--;
        nameInput[letterCount] = '\0';
      }
    }

    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
      nameEntered = true;
      GameManager::getInstance()->saveHighScore(std::string(nameInput));
      leaderboard = GameManager::getInstance()->loadHighScores();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
      nameEntered = true;
      leaderboard = GameManager::getInstance()->loadHighScores();
    }
  } else {
    if (IsKeyPressed(KEY_ENTER)) {
      game->changeState(GameStateType::MENU);
    }
  }
}

void VictoryState::update(float dt) {
  sceneTimer += dt;

  switch (currentScene) {
    case EndingScene::SCENE1_TUNNEL:
      updateScene1(dt);
      break;
    case EndingScene::SCENE2_DESERT_FALL:
      updateScene2(dt);
      break;
    case EndingScene::SCENE3_TALLY:
      updateScene3(dt);
      break;
    case EndingScene::SCENE4_BLACK_SCREEN:
      updateScene4(dt);
      break;
    case EndingScene::SCENE5_SUMMARY:
      updateScene5(dt);
      break;
  }
}

void VictoryState::updateScene1(float dt) {
  if (cutscenePlayer && physics && tunnelMap) {
    if (cutscenePlayer->isDoorAnimPlaying()) {
      cutscenePlayer->update(dt, cutscenePlayer.get());
    } else {
      // Automatically walk right at 175px/s
      cutscenePlayer->setVelocity(175.0f, cutscenePlayer->getVelocityY());
      cutscenePlayer->update(dt, cutscenePlayer.get());
      cutscenePlayer->applyGravity(dt);
      physics->resolveEntityTileCollision(cutscenePlayer.get());
      stepAnimTimer += dt;
    }

    // Update camera to follow player smoothly
    camera.target.x = cutscenePlayer->getX() + cutscenePlayer->getAABB().width / 2.0f;
    camera.target.y = 10 * 32.0f; // Lock Y axis to tunnel center

    if (lavaSim) {
      lavaSim->update(dt);
    }

    if (lighting) {
      lighting->clearLights();
      float trueX = (cutscenePlayer->getX() + cutscenePlayer->getAABB().width / 2.0f) /
                    tunnelMap->getTileSize();
      float trueY = (cutscenePlayer->getY() + cutscenePlayer->getAABB().height / 2.0f) /
                    tunnelMap->getTileSize();
      double time = GetTime();
      float flicker = std::sin(time * 12.0) * 0.02f;
      flicker += std::sin(time * 23.0) * 0.015f;
      Vector3 torchColor = {0.95f + flicker, (0.95f + flicker) * 0.9f, (0.95f + flicker) * 0.6f};
      lighting->addLight(trueX, trueY, torchColor,
                         4.5f + (flicker * 1.5f));
      lighting->update(tunnelMap.get());
    }

    // 15 steps ~ 2.5s - 3.0s
    if (stepAnimTimer >= 3.0f) {
      currentScene = EndingScene::SCENE2_DESERT_FALL;
      sceneTimer = 0.0f;
      tallyStatus = 0;
      tallyTimer = 0.0f;
      player.position = { 540.0f, -100.0f }; // Start falling in the horizontal center
      player.velocity = { 0.0f, 0.0f };
      player.isFacingRight = true;
      chest.position = { 660.0f, -128.0f };
      chest.velocity = { 0.0f, 0.0f };
    }
  }
}

void VictoryState::updateScene2(float dt) {
  if (tallyStatus == 0) {
    player.velocity.y += 1000.0f * dt;
    player.position.y += player.velocity.y * dt;
    if (player.position.y >= 520.0f) {
      player.position.y = 520.0f;
      player.velocity.y = 0.0f;
      AudioManager::getInstance()->playSFX("xland");
      tallyStatus = 1;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 1) {
    tallyTimer += dt;
    if (tallyTimer >= 2.0f) {
      tallyStatus = 2;
      chest.position = { 660.0f, -128.0f };
      chest.velocity = { 0.0f, 0.0f };
    }
  }
  else if (tallyStatus == 2) {
    chest.velocity.y += 1000.0f * dt;
    chest.position.y += chest.velocity.y * dt;
    
    // So target chest y coordinate is 520 - 128 = 392.
    if (chest.position.y >= 392.0f) {
      chest.position.y = 392.0f;
      chest.velocity.y = 0.0f;
      AudioManager::getInstance()->playSFX("xtfall"); // Heavy fall SFX
      
      // Make player bounce!
      player.velocity.y = -450.0f;
      tallyStatus = 3;
    }
  }
  else if (tallyStatus == 3) {
    player.velocity.y += 1000.0f * dt;
    player.position.y += player.velocity.y * dt;
    
    if (player.position.y >= 520.0f) {
      player.position.y = 520.0f;
      player.velocity.y = 0.0f;
      AudioManager::getInstance()->playSFX("xland");
      tallyStatus = 4;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 4) {
    tallyTimer += dt;
    if (tallyTimer >= 0.5f) {
      currentScene = EndingScene::SCENE3_TALLY;
      sceneTimer = 0.0f;
      tallyStatus = 0;
      tallyTimer = 0.0f;
      currentTallyScore = 0.0f;
    }
  }
}

void VictoryState::updateScene3(float dt) {
  // Spawn gems periodically
  int prevTick = (int)((sceneTimer - dt) * 10);
  int currTick = (int)(sceneTimer * 10);
  if (currTick > prevTick && tallyStatus >= 2 && tallyStatus < 5) {
    GemDrop gem;
    gem.position = { (float)GetRandomValue(50, 1230), -20.0f };
    gem.velocity = { 0.0f, (float)GetRandomValue(300, 500) };
    gem.type = GetRandomValue(0, 3);
    gem.rotation = (float)GetRandomValue(0, 360);
    gems.push_back(gem);
  }

  // Update gems
  for (auto &gem : gems) {
    gem.position.y += gem.velocity.y * dt;
    gem.rotation += 180.0f * dt;
  }
  gems.erase(std::remove_if(gems.begin(), gems.end(), [](const GemDrop &g) {
    return g.position.y > 750.0f;
  }), gems.end());

  // Tally state transitions
  if (tallyStatus == 0) {
    tallyTimer += dt;
    if (tallyTimer >= 1.0f) {
      tallyStatus = 1;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 1) {
    tallyTimer += dt;
    if (tallyTimer >= 1.0f) {
      tallyStatus = 2;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 2) {
    if (finalScore <= 0) {
      currentTallyScore = 0.0f;
      tallyStatus = 3;
      tallyTimer = 0.0f;
    } else {
      float prevScore = currentTallyScore;
      currentTallyScore += finalScore * dt * 0.5f; // takes 2 seconds to tally up
      if ((int)(currentTallyScore / 2500) > (int)(prevScore / 2500)) {
        AudioManager::getInstance()->playSFX("xcoin");
      }
      if (currentTallyScore >= finalScore) {
        currentTallyScore = finalScore;
        AudioManager::getInstance()->playSFX("xgem");
        tallyStatus = 3;
        tallyTimer = 0.0f;
      }
    }
  }
  else if (tallyStatus == 3) {
    tallyTimer += dt;
    if (tallyTimer >= 1.0f) {
      tallyStatus = 4;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 4) {
    tallyTimer += dt;
    if (tallyTimer >= 1.0f) {
      tallyStatus = 5;
      tallyTimer = 0.0f;
    }
  }
  else if (tallyStatus == 5) {
    fadeAlpha += dt * 0.5f; // 2 seconds fade
    if (fadeAlpha >= 1.0f) {
      fadeAlpha = 1.0f;
      currentScene = EndingScene::SCENE4_BLACK_SCREEN;
      sceneTimer = 0.0f;
      blackScreenTimer = 0.0f;
    }
  }
}

void VictoryState::updateScene4(float dt) {
  blackScreenTimer += dt;
  if (blackScreenTimer >= 5.0f) {
    currentScene = EndingScene::SCENE5_SUMMARY;
    sceneTimer = 0.0f;
    nameEntered = false;
    letterCount = 0;
    nameInput[0] = '\0';
  }
}

void VictoryState::updateScene5(float dt) {
}

void VictoryState::render() {
  switch (currentScene) {
    case EndingScene::SCENE1_TUNNEL:
      renderScene1();
      break;
    case EndingScene::SCENE2_DESERT_FALL:
      renderScene2();
      break;
    case EndingScene::SCENE3_TALLY:
      renderScene3();
      break;
    case EndingScene::SCENE4_BLACK_SCREEN:
      renderScene4();
      break;
    case EndingScene::SCENE5_SUMMARY:
      renderScene5();
      break;
  }
}

void VictoryState::renderScene1() {
  ClearBackground(BLACK);
  BeginMode2D(camera);
  if (tunnelMap) {
    tunnelMap->renderParallaxBackground(camera);
    if (lighting)
      tunnelMap->render(camera, lighting->getLightMap(), false);
  }
  if (lavaSim) {
    lavaSim->render(camera);
  }
  if (cutscenePlayer) {
    cutscenePlayer->render(1.0f);
  }
  if (tunnelMap && lighting) {
    tunnelMap->render(camera, lighting->getLightMap(), true);
  }
  EndMode2D();
}

void VictoryState::renderScene2() {
  ClearBackground(BLACK);
  
  // 1. Sky: Vibrant blue sky gradient from top to mountain line (y=0 to y=520)
  DrawRectangleGradientV(0, 0, 1280, 520, Color{ 50, 120, 210, 255 }, Color{ 140, 185, 230, 255 });
  
  // Draw clean sky texture overlay from sEnd2BG.png (y=40..130) for authentic Spelunky clouds/sky
  DrawTexturePro(skyTex, { 0.0f, 40.0f, 640.0f, 95.0f }, { 0.0f, 0.0f, 1280.0f, 520.0f }, { 0.0f, 0.0f }, 0.0f, Color{ 255, 255, 255, 220 });

  // 2. Mountains (sBGEnd3.png)
  float mountW = 480.0f * 2.0f;
  float mountH = 112.0f * 2.0f;
  float mountY = 520.0f - mountH + 8.0f; // Align mountain base with sand surface
  for (float mx = 0.0f; mx < 1280.0f; mx += mountW) {
    DrawTexturePro(mountainTex, { 0.0f, 0.0f, 480.0f, 112.0f }, { mx, mountY, mountW, mountH }, { 0.0f, 0.0f }, 0.0f, WHITE);
  }
  
  // 3. Solid Sand Base (no black gaps anywhere!)
  DrawRectangle(0, 520, 1280, 200, Color{ 251, 213, 98, 255 });
  
  // Sand Tiles Pattern
  for (int ty = 0; ty < 7; ty++) {
    for (int tx = 0; tx < 40; tx++) {
      Texture2D tex = (sandGrid[tx][ty % 5] == 0) ? sandTex : sand2Tex;
      DrawTexturePro(tex, { 0.0f, 0.0f, 16.0f, 16.0f }, { (float)tx * 32.0f, 520.0f + (float)ty * 32.0f, 32.0f, 32.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
    }
  }

  // Sand Top Dune Waves (sDesertTop with source {0, 12, 16, 4} mapped directly on top of y=520)
  for (int tx = 0; tx < 40; tx++) {
    DrawTexturePro(sandTopTex, { 0.0f, 12.0f, 16.0f, 4.0f }, { (float)tx * 32.0f, 520.0f - 8.0f, 32.0f, 8.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  }
  
  // 4. Large Palm Trees (Scaled up)
  DrawTexturePro(palmTreeTex, { 0.0f, 0.0f, 32.0f, 128.0f }, { 10.0f, 520.0f - 420.0f, 110.0f, 420.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  DrawTexturePro(palmTreeTex, { 0.0f, 0.0f, -32.0f, 128.0f }, { 1160.0f, 520.0f - 420.0f, 110.0f, 420.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  
  // 5. Large Shrubs (Scaled up)
  DrawTexturePro(shrubTex, { 0.0f, 0.0f, 16.0f, 16.0f }, { 110.0f, 520.0f - 60.0f, 60.0f, 60.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  DrawTexturePro(shrubTex, { 0.0f, 0.0f, -16.0f, 16.0f }, { 1110.0f, 520.0f - 60.0f, 60.0f, 60.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  
  // 6. Big Treasure (Center-Right at x=660)
  if (currentScene == EndingScene::SCENE3_TALLY || (currentScene == EndingScene::SCENE2_DESERT_FALL && tallyStatus >= 2)) {
    float chestY = (currentScene == EndingScene::SCENE3_TALLY) ? 392.0f : chest.position.y;
    DrawTexturePro(bigTreasureTex, { 0.0f, 0.0f, 32.0f, 32.0f }, { 660.0f, chestY, 128.0f, 128.0f }, { 0.0f, 0.0f }, 0.0f, WHITE);
  }
  
  // 7. Player (Center-Left at x=540, scaled 2x to 80x80)
  if (playerSpriteSheet.id != 0) {
    int row = 0;
    int col = 0;
    float playerY = player.position.y;
    if (currentScene == EndingScene::SCENE2_DESERT_FALL) {
      if (tallyStatus == 0) { // falling
        row = 9;
        col = 1;
      } else if (tallyStatus == 1) { // stunned
        row = 2;
        col = 3;
      } else if (tallyStatus == 3) { // jumping/bouncing
        row = 9;
        col = 0;
      } else { // standing
        row = 0;
        col = 0;
      }
    } else { // Scene 3 (Tally) and onwards: player stands firmly on the ground next to treasure
      row = 0;
      col = 0;
      playerY = 520.0f;
    }
    float srcW = player.isFacingRight ? 80.0f : -80.0f;
    Rectangle src = { col * 80.0f, row * 80.0f, srcW, 80.0f };
    Rectangle dest = { player.position.x - 40.0f, playerY - 70.0f, 80.0f, 80.0f };
    DrawTexturePro(playerSpriteSheet, src, dest, {0, 0}, 0.0f, WHITE);
  }
}

void VictoryState::renderScene3() {
  renderScene2();
  
  // Gems rain
  Texture2D rubiesTex = EntityFactory::getTexture("assets/sprites/8x8/gfx_rubies.png");
  Texture2D goldTex = EntityFactory::getTexture("assets/sprites/8x8/gold.png");
  for (const auto &gem : gems) {
    if (gem.type == 0) {
      DrawTexturePro(goldTex, { 0.0f, 0.0f, 8.0f, 8.0f }, { gem.position.x, gem.position.y, 24.0f, 24.0f }, { 12.0f, 12.0f }, gem.rotation, WHITE);
    } else {
      int col = gem.type - 1;
      DrawTexturePro(rubiesTex, { col * 8.0f, 0.0f, 8.0f, 8.0f }, { gem.position.x, gem.position.y, 24.0f, 24.0f }, { 12.0f, 12.0f }, gem.rotation, WHITE);
    }
  }
  
  // Draw Tally Scoreboard Panel in upper center
  float boardW = 620.0f;
  float boardH = 240.0f;
  float boardX = (GetScreenWidth() - boardW) / 2.0f;
  float boardY = 45.0f;

  // Background panel with rounded corners and border
  DrawRectangleRounded({ boardX - 4, boardY - 4, boardW + 8, boardH + 8 }, 0.08f, 6, Color{ 30, 18, 10, 240 });
  DrawRectangleRounded({ boardX, boardY, boardW, boardH }, 0.08f, 6, Color{ 15, 12, 10, 220 });
  DrawRectangleRoundedLines({ boardX, boardY, boardW, boardH }, 0.08f, 6, Color{ 220, 175, 45, 255 });

  // Draw Tally UI texts inside the board
  if (tallyStatus >= 0) {
    drawCenteredText("YOU MADE IT!", boardY + 20, 42.0f, GOLD);
  }
  if (tallyStatus >= 1) {
    drawCenteredText("FINAL SCORE:", boardY + 75, 24.0f, WHITE);
  }
  if (tallyStatus >= 2) {
    drawCenteredText(TextFormat("%d", (int)currentTallyScore), boardY + 110, 36.0f, YELLOW);
  }
  if (tallyStatus >= 3) {
    srand(finalScore);
    int randomTimeTotal = 180 + (rand() % 240);
    int tallyMinutes = randomTimeTotal / 60;
    int tallySeconds = randomTimeTotal % 60;
    drawCenteredText(TextFormat("TIME: %02d:%02d", tallyMinutes, tallySeconds), boardY + 160, 24.0f, WHITE);
  }
  if (tallyStatus >= 4) {
    srand(finalScore);
    rand();
    int tallyKills = 10 + (rand() % 35);
    drawCenteredText(TextFormat("KILLS: %d", tallyKills), boardY + 195, 24.0f, WHITE);
  }
  
  // Black fade overlay
  if (fadeAlpha > 0.0f) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), ColorAlpha(BLACK, fadeAlpha));
  }
}

void VictoryState::renderScene4() {
  ClearBackground(BLACK);
  drawCenteredText("you shall be remembered as hero", GetScreenHeight() / 2 - 14, 28.0f, WHITE);
}

void VictoryState::renderScene5() {
  ClearBackground(BLACK);
  MenuBackground::render();

  if (!nameEntered) {
    // Split the big text so it doesn't overflow the screen width and center properly
    drawCenteredText("VICTORY", GetScreenHeight() / 2 - 170, 90.0f, GOLD);
    drawCenteredText("YOU ESCAPED!", GetScreenHeight() / 2 - 80, 50.0f, GRAY);
    
    drawCenteredText(TextFormat("FINAL SCORE: %d", finalScore),
                     GetScreenHeight() / 2 - 20, 40.0f, WHITE);
    drawCenteredText("ENTER INITIALS:", GetScreenHeight() / 2 + 50, 30.0f,
                     WHITE);

    std::string displayStr = nameInput;
    if (((int)(GetTime() * 2)) % 2 == 0 && letterCount < 3) {
      displayStr += "_";
    }
    drawCenteredText(displayStr.c_str(), GetScreenHeight() / 2 + 100, 50.0f,
                     YELLOW);
    drawCenteredText("PRESS ESC TO SKIP", GetScreenHeight() / 2 + 200, 20.0f,
                     LIGHTGRAY);
  } else {
    drawCenteredText("HIGH SCORES", GetScreenHeight() / 2 - 250, 60.0f, GOLD);

    int yOffset = GetScreenHeight() / 2 - 150;
    int count = 0;
    for (const auto &entry : leaderboard) {
      if (count >= 5)
        break;
      std::string text =
          TextFormat("%d. %s - %d pts (Floor %d)", count + 1,
                     entry.name.c_str(), entry.score, entry.floorsReached);
      drawCenteredText(text.c_str(), yOffset, 30.0f, WHITE);
      yOffset += 60;
      count++;
    }

    if (((int)(GetTime() * 2)) % 2 == 0) {
      drawCenteredText("PRESS ENTER TO CONTINUE", GetScreenHeight() / 2 + 200,
                       20.0f, LIGHTGRAY);
    }
  }
}

/*
=======================================================
=========================CHARSELECT====================
=======================================================
*/

void CharSelectState::enter() { 
  selectedIndex = 0; 
  // Preload character textures to prevent stutter when hovering
  EntityFactory::getTexture("assets/characters/explorer.png");
  EntityFactory::getTexture("assets/characters/ninja.png");
  EntityFactory::getTexture("assets/characters/tank.png");
}

void CharSelectState::exit() {}

void CharSelectState::handleInput() {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    selectedIndex = (selectedIndex + 2) % 3;
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    selectedIndex = (selectedIndex + 1) % 3;
  }
  if (IsKeyPressed(KEY_ENTER)) {
    GameManager::getInstance()->resetRun();
    GameManager::getInstance()->setSelectedCharacter(characters[selectedIndex]);
    game->changeState(GameStateType::PLAY);
  }
  if (IsKeyPressed(KEY_ESCAPE)) {
    game->changeState(GameStateType::MENU);
  }
}

void CharSelectState::update(float dt) {}

void CharSelectState::render() {
  ClearBackground(BLACK);

  MenuBackground::render();

  // Draw semi-transparent left panel
  DrawRectangle(0, 0, 600, 720, {0, 0, 0, 200});

  drawLeftText("CHARACTER", 50.0f, 60.0f, 50.0f, RAYWHITE);
  drawLeftText("SELECT", 50.0f, 110.0f, 50.0f, RAYWHITE);

  Color expColor = (selectedIndex == 0) ? YELLOW : LIGHTGRAY;
  Color ninColor = (selectedIndex == 1) ? YELLOW : LIGHTGRAY;
  Color tnkColor = (selectedIndex == 2) ? YELLOW : LIGHTGRAY;

  drawLeftText("EXPLORER", 50.0f, 300.0f, 30.0f, expColor);
  drawLeftText("NINJA", 50.0f, 370.0f, 30.0f, ninColor);
  drawLeftText("TANK", 50.0f, 440.0f, 30.0f, tnkColor);

  // Render the selected character sprite
  const char* texPaths[] = {
      "assets/characters/explorer.png",
      "assets/characters/ninja.png",
      "assets/characters/tank.png"
  };
  Texture2D charTex = EntityFactory::getTexture(texPaths[selectedIndex]);
  if (charTex.id != 0) {
      // Idle frame is row 0, col 0, width 80, height 80
      Rectangle src = {0, 0, 80, 80};
      // Draw at x=320, aligned vertically with text, scale 1x (80x80)
      Rectangle dest = {320, 275.0f + (selectedIndex * 70.0f), 80, 80};
      DrawTexturePro(charTex, src, dest, {0, 0}, 0.0f, WHITE);
  }

  drawLeftText("ENTER: Start", 50.0f, 650.0f, 20.0f, GRAY);
  drawLeftText("ESC: Back", 50.0f, 680.0f, 20.0f, GRAY);
}

/*
=======================================================
======================EDITOR MENU======================
=======================================================
*/

void LevelEditorMenuState::enter() { selectedOption = 0; }

void LevelEditorMenuState::exit() {}

void LevelEditorMenuState::handleInput() {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    selectedOption--;
    if (selectedOption < 0)
      selectedOption = 2;
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    selectedOption++;
    if (selectedOption > 2)
      selectedOption = 0;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    const char *filterPatterns[1] = {"*.lvl"};

    switch (selectedOption) {
    case 0: { // Play Custom Level
      const char *filepath = tinyfd_openFileDialog(
          "Play Custom Level", "levels/", 1, filterPatterns, "Level Files", 0);
      if (filepath) {
        GameManager::getInstance()->setCustomLevelPath(filepath);
        GameManager::getInstance()->setIsCustomLevel(true);
        game->changeState(GameStateType::PLAY);
      }
      break;
    }
    case 1: // Editor Sub-menu
      game->changeState(GameStateType::EDITOR_FILE_MENU);
      break;
    case 2: // Back
      game->changeState(GameStateType::MENU);
      break;
    }
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    game->changeState(GameStateType::MENU);
  }
}

void LevelEditorMenuState::update(float dt) {}

void LevelEditorMenuState::render() {
  ClearBackground(BLACK);

  MenuBackground::render();

  // Draw semi-transparent left panel
  DrawRectangle(0, 0, 600, 720, {0, 0, 0, 200});

  drawLeftText("LEVEL", 50.0f, 60.0f, 50.0f, RAYWHITE);
  drawLeftText("EDITOR", 50.0f, 110.0f, 50.0f, RAYWHITE);

  Color playColor = (selectedOption == 0) ? YELLOW : LIGHTGRAY;
  Color editColor = (selectedOption == 1) ? YELLOW : LIGHTGRAY;
  Color backColor = (selectedOption == 2) ? YELLOW : LIGHTGRAY;

  drawLeftText("PLAY CUSTOM LEVEL", 50.0f, 300.0f, 30.0f, playColor);
  drawLeftText("EDITOR", 50.0f, 370.0f, 30.0f, editColor);
  drawLeftText("BACK", 50.0f, 440.0f, 30.0f, backColor);

  drawLeftText("ENTER: Select", 50.0f, 650.0f, 20.0f, GRAY);
  drawLeftText("ESC: Back", 50.0f, 680.0f, 20.0f, GRAY);
}

void EditorFileMenuState::enter() { selectedOption = 0; }

void EditorFileMenuState::exit() {}

void EditorFileMenuState::handleInput() {
  if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
    selectedOption--;
    if (selectedOption < 0)
      selectedOption = 2;
  }
  if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
    selectedOption++;
    if (selectedOption > 2)
      selectedOption = 0;
  }

  if (IsKeyPressed(KEY_ENTER)) {
    const char *filterPatterns[1] = {"*.lvl"};

    switch (selectedOption) {
    case 0: { // New Level
      const char *filepath =
          tinyfd_saveFileDialog("New Custom Level", "levels/new_level.lvl", 1,
                                filterPatterns, "Level Files");
      if (filepath) {
        GameManager::getInstance()->setCustomLevelPath(filepath);
        // Create empty file
        std::ofstream out(filepath);
        if (out.is_open())
          out.close();
        GameManager::getInstance()->setLoadIntoEditor(false);
        game->changeState(GameStateType::EDITOR);
      }
      break;
    }
    case 1: { // Open Level
      const char *filepath = tinyfd_openFileDialog(
          "Open Custom Level", "levels/", 1, filterPatterns, "Level Files", 0);
      if (filepath) {
        GameManager::getInstance()->setCustomLevelPath(filepath);
        GameManager::getInstance()->setLoadIntoEditor(true);
        game->changeState(GameStateType::EDITOR);
      }
      break;
    }
    case 2: // Back
      game->changeState(GameStateType::EDITOR_MENU);
      break;
    }
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    game->changeState(GameStateType::EDITOR_MENU);
  }
}

void EditorFileMenuState::update(float dt) {}

void EditorFileMenuState::render() {
  ClearBackground(BLACK);

  MenuBackground::render();

  // Draw semi-transparent left panel
  DrawRectangle(0, 0, 600, 720, {0, 0, 0, 200});

  drawLeftText("FILE", 50.0f, 60.0f, 50.0f, RAYWHITE);
  drawLeftText("MENU", 50.0f, 110.0f, 50.0f, RAYWHITE);

  Color newColor = (selectedOption == 0) ? YELLOW : LIGHTGRAY;
  Color loadColor = (selectedOption == 1) ? YELLOW : LIGHTGRAY;
  Color backColor = (selectedOption == 2) ? YELLOW : LIGHTGRAY;

  drawLeftText("NEW LEVEL", 50.0f, 300.0f, 30.0f, newColor);
  drawLeftText("LOAD LEVEL", 50.0f, 370.0f, 30.0f, loadColor);
  drawLeftText("BACK", 50.0f, 440.0f, 30.0f, backColor);

  drawLeftText("ENTER: Select", 50.0f, 650.0f, 20.0f, GRAY);
  drawLeftText("ESC: Back", 50.0f, 680.0f, 20.0f, GRAY);
}

/*
=======================================================
===================LEVEL SELECT========================
=======================================================
*/

/*
=======================================================
=========================EDITOR====================
=======================================================
*/

EditorState::~EditorState() = default;

void EditorState::enter() {
  EntityFactory::preloadTextures();
  tileMap = std::make_unique<TileMap>(40, 32, 32);

  // Load tilesets so TileMap::render() can draw tiles properly
  tileMap->setTileset(EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png"));
  tileMap->setJungleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png"));
  tileMap->setTempleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png"));
  tileMap->setRopeTexture(EntityFactory::getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"));

  camera = {0};
  camera.target = Vector2{(float)(40 * 32) / 2.0f, (float)(32 * 32) / 2.0f}; // Center on map
  camera.offset = Vector2{(float)(GetScreenWidth() - 200) / 2.0f, (float)GetScreenHeight() / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  Texture2D caveTex =
      EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png");
  Texture2D jungleTex = 
      EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png");
  Texture2D templeTex = 
      EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png");

  tileMap->setTileset(caveTex);
  tileMap->setJungleTileset(jungleTex);
  tileMap->setTempleTileset(templeTex);
  Texture2D spikeTex = EntityFactory::getTexture(
      "assets/sprites/16x16/gfx_spike_collectibles_flame.png");

  Texture2D batSnakeTex = EntityFactory::getTexture(
      "assets/sprites/16x16/gfx_bat_snake_jetpack.png");
  Texture2D spiderTex =
      EntityFactory::getTexture("assets/sprites/16x16/gfx_spider_skeleton.png");

  Texture2D lavaTex = EntityFactory::getTexture("assets/sprites/lava/Lava.png");
  Texture2D waterTex =
      EntityFactory::getTexture("assets/sprites/16x16/water.png");

  paletteItems = {
      {TileType::CAVE_ROCK, caveTex, {0, 0, 16, 16}, "Rock"},
      {TileType::STONE_BLOCK, caveTex, {0, 16, 16, 16}, "Stone"},
      {TileType::LADDER, caveTex, {0, 64, 16, 16}, "Ladder"},
      {TileType::LADDER_DECK, caveTex, {16, 64, 16, 16}, "L. Deck"},
      {TileType::ARROW_TRAP_LEFT, caveTex, {0, 80, 16, 16}, "Trap L"},
      {TileType::ARROW_TRAP_RIGHT, caveTex, {16, 80, 16, 16}, "Trap R"},
      {TileType::SPIKE_TRAP, spikeTex, {0, 0, 16, 16}, "Spikes"},
      {TileType::ENTRANCE, caveTex, {0, 96, 16, 16}, "Enter"},
      {TileType::EXIT,
       caveTex,
       {16, 96, 16, 16},
       "Exit"},
      {TileType::CHEST, spikeTex, {32, 0, 16, 16}, "Chest"},
      {TileType::ENEMY_SNAKE, batSnakeTex, {0, 16, 16, 16}, "Snake"},
      {TileType::ENEMY_BAT, batSnakeTex, {0, 0, 16, 16}, "Bat"},
      {TileType::ENEMY_SPIDER, spiderTex, {0, 0, 16, 16}, "Spider"},
      {TileType::LAVA, lavaTex, {0, 0, 16, 16}, "Lava"},
      {TileType::WATER, waterTex, {16, 0, 16, 16}, "Water"}};
  selectedTileIdx = 0;
  statusMsg = "";
  statusTimer = 0.0f;
  isDragging = false;
  undoStack.clear();
  redoStack.clear();

  if (GameManager::getInstance()->getLoadIntoEditor()) {
    loadLevel(GameManager::getInstance()->getCustomLevelPath());
    GameManager::getInstance()->setLoadIntoEditor(false);
  }
}

void EditorState::exit() {}

void EditorState::placeTile(int tx, int ty, TileType newType) {
  TileType oldType = tileMap->getTile(tx, ty);
  if (oldType == newType) return; // No change
  tileMap->setTile(tx, ty, newType);
  undoStack.push_back({tx, ty, oldType, newType});
  redoStack.clear(); // New action invalidates redo history
}

void EditorState::undo() {
  if (undoStack.empty()) return;
  TileChange change = undoStack.back();
  undoStack.pop_back();
  tileMap->setTile(change.x, change.y, change.oldType);
  redoStack.push_back(change);
  statusMsg = "Undo";
  statusTimer = 1.0f;
}

void EditorState::redo() {
  if (redoStack.empty()) return;
  TileChange change = redoStack.back();
  redoStack.pop_back();
  tileMap->setTile(change.x, change.y, change.newType);
  undoStack.push_back(change);
  statusMsg = "Redo";
  statusTimer = 1.0f;
}

void EditorState::handleInput() {
  int ts = tileMap->getTileSize();
  int sidebarX = GetScreenWidth() - 200;

  // Zoom
  float wheelMove = GetMouseWheelMove();
  if (wheelMove != 0.0f) {
    // Zoom relative to mouse position
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    camera.offset = GetMousePosition();
    camera.target = mouseWorldPos;
    camera.zoom += wheelMove * 0.25f;
    if (camera.zoom < 0.25f)
      camera.zoom = 0.25f;
    if (camera.zoom > 5.0f)
      camera.zoom = 5.0f;
  }

  // Panning (Middle Mouse)
  if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
    isDragging = true;
    panDragStart = GetMousePosition();
  }
  if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
    Vector2 currentMouse = GetMousePosition();
    Vector2 delta = {currentMouse.x - panDragStart.x,
                     currentMouse.y - panDragStart.y};
    camera.target.x -= delta.x / camera.zoom;
    camera.target.y -= delta.y / camera.zoom;
    panDragStart = currentMouse;
  }
  if (IsMouseButtonReleased(MOUSE_MIDDLE_BUTTON)) {
    isDragging = false;
  }

  Vector2 mousePos = GetMousePosition();

  // Palette UI interaction
  if (mousePos.x > sidebarX) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      // Check palette clicks (Grid: 2 cols)
      for (int i = 0; i < (int)paletteItems.size(); i++) {
        int col = i % 2;
        int row = i / 2;
        float px = sidebarX + 10.0f + col * 95.0f;
        float py = 60.0f + row * 45.0f;
        Rectangle itemRect = {px - 5.0f, py - 5.0f, 90.0f, 40.0f};

        if (CheckCollisionPointRec(mousePos, itemRect)) {
          selectedTileIdx = i;
        }
      }
    }
  } else {
    // Mouse coordinates for editor
    Vector2 mouseWorld = GetScreenToWorld2D(mousePos, camera);
    int tx = (int)floorf(mouseWorld.x / (float)ts);
    int ty = (int)floorf(mouseWorld.y / (float)ts);
    mouseGridPos = {(float)tx, (float)ty};

    if (tx >= 0 && tx < 40 && ty >= 0 && ty < 32) {
      // Placement (with undo support)
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        placeTile(tx, ty, paletteItems[selectedTileIdx].type);
      }
      // Erase (with undo support)
      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        placeTile(tx, ty, TileType::NOTHING);
      }
    }
  }

  // Palette cycling with bracket keys
  if (IsKeyPressed(KEY_LEFT_BRACKET)) {
    selectedTileIdx--;
    if (selectedTileIdx < 0)
      selectedTileIdx = (int)paletteItems.size() - 1;
  }
  if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
    selectedTileIdx++;
    if (selectedTileIdx >= (int)paletteItems.size())
      selectedTileIdx = 0;
  }

  // Hotkeys (CTRL+S, CTRL+SHIFT+S, CTRL+O, CTRL+Z, CTRL+Y)
  bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

  // Undo / Redo
  if (ctrlDown && IsKeyPressed(KEY_Z)) {
    if (shiftDown) {
      redo();
    } else {
      undo();
    }
  }
  if (ctrlDown && IsKeyPressed(KEY_Y)) {
    redo();
  }

  if (ctrlDown && IsKeyPressed(KEY_S)) {
    if (shiftDown) {
      // Save As
      const char *filterPatterns[1] = {"*.lvl"};
      const char *filepath = tinyfd_saveFileDialog(
          "Save As", "levels/custom_level.lvl", 1, filterPatterns, "Level Files");
      if (filepath) {
        GameManager::getInstance()->setCustomLevelPath(filepath);
        saveLevel(filepath);
      }
    } else {
      // Normal Save (with empty path guard)
      std::string currentPath = GameManager::getInstance()->getCustomLevelPath();
      if (currentPath.empty()) {
        // Fallthrough to Save As
        const char *filterPatterns[1] = {"*.lvl"};
        const char *filepath = tinyfd_saveFileDialog(
            "Save As", "levels/custom_level.lvl", 1, filterPatterns, "Level Files");
        if (filepath) {
          GameManager::getInstance()->setCustomLevelPath(filepath);
          saveLevel(filepath);
        }
      } else {
        saveLevel(currentPath);
      }
    }
  }
  if (ctrlDown && !shiftDown && IsKeyPressed(KEY_O)) {
    const char *filterPatterns[1] = {"*.lvl"};
    const char *filepath = tinyfd_openFileDialog(
        "Open Level", "levels/", 1, filterPatterns, "Level Files", 0);
    if (filepath) {
      GameManager::getInstance()->setCustomLevelPath(filepath);
      loadLevel(filepath);
    }
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    game->changeState(GameStateType::EDITOR_MENU);
  }
}

void EditorState::update(float dt) {
  if (statusTimer > 0.0f) {
    statusTimer -= dt;
  }
}

void EditorState::render() {
  ClearBackground(DARKGRAY);

  int ts = tileMap->getTileSize();
  int sidebarX = GetScreenWidth() - 200;

  // Render Editor
  BeginMode2D(camera);
  std::vector<std::vector<Vector3>> dummyLight(32, std::vector<Vector3>(40, {1.0f, 1.0f, 1.0f}));
  tileMap->render(camera, dummyLight, false);
  tileMap->render(camera, dummyLight, true);

  // Overlay special entity tiles that TileMap::render() skips
  for (int y = 0; y < tileMap->getHeight(); y++) {
    for (int x = 0; x < tileMap->getWidth(); x++) {
      TileType t = tileMap->getTile(x, y);
      if (t == TileType::SPIKE_TRAP || t == TileType::CHEST ||
          t == TileType::ENEMY_SNAKE || t == TileType::ENEMY_BAT ||
          t == TileType::ENEMY_SPIDER || t == TileType::LAVA || t == TileType::WATER) {
        // Find texture inside paletteItems
        for (const auto &item : paletteItems) {
          if (item.type == t) {
            Rectangle dest = {(float)x * ts, (float)y * ts, (float)ts, (float)ts};
            
            // Adjust visual offset for sprites that are "flying" in the original spritesheet
            if (t == TileType::CHEST) {
                dest.y += (float)ts / 2.0f; // Shift down by half a tile
            }

            DrawTexturePro(item.tex, item.src, dest, {0, 0}, 0.0f, WHITE);
            break;
          }
        }
      }
    }
  }

  // Grid lines
  for (int i = 0; i <= 40; i++) {
    DrawLine(i * ts, 0, i * ts, 32 * ts, ColorAlpha(WHITE, 0.15f));
  }
  for (int j = 0; j <= 32; j++) {
    DrawLine(0, j * ts, 40 * ts, j * ts, ColorAlpha(WHITE, 0.15f));
  }

  // Cursor highlight with tile preview
  if (GetMousePosition().x <= sidebarX) {
    float cx = mouseGridPos.x * ts;
    float cy = mouseGridPos.y * ts;
    // Draw selected tile preview at 50% opacity
    DrawTexturePro(paletteItems[selectedTileIdx].tex,
                   paletteItems[selectedTileIdx].src,
                   {cx, cy, (float)ts, (float)ts},
                   {0, 0}, 0.0f, ColorAlpha(WHITE, 0.5f));
    // Draw outline
    DrawRectangleLines((int)cx, (int)cy, ts, ts, YELLOW);
  }
  EndMode2D();

  // Render Sidebar Palette
  DrawRectangle(sidebarX, 0, 200, GetScreenHeight(), ColorAlpha(BLACK, 0.9f));
  DrawText("PALETTE", sidebarX + 10, 10, 20, WHITE);
  DrawLine(sidebarX, 40, sidebarX + 200, 40, GRAY);

  for (int i = 0; i < (int)paletteItems.size(); i++) {
    int col = i % 2;
    int row = i / 2;
    float px = sidebarX + 10.0f + col * 95.0f;
    float py = 60.0f + row * 45.0f;

    Color bgColor = (i == selectedTileIdx) ? ColorAlpha(YELLOW, 0.3f) : BLANK;
    DrawRectangle(px - 5, py - 5, 90, 40, bgColor);

    Rectangle dest = {px, py, 24.0f, 24.0f};
    if (paletteItems[i].type == TileType::CHEST) {
        dest.y += 8.0f; // Visually center the chest in the palette button
    }
    DrawTexturePro(paletteItems[i].tex, paletteItems[i].src, dest, {0, 0}, 0.0f,
                   WHITE);

    // Shrink font size slightly to fit 2 columns
    DrawText(paletteItems[i].name.c_str(), px + 28, py + 6, 10, WHITE);
  }

  DrawLine(sidebarX, 520, sidebarX + 200, 520, GRAY);
  DrawText("L-Click: Place", sidebarX + 10, 530, 10, LIGHTGRAY);
  DrawText("R-Click: Erase", sidebarX + 10, 545, 10, LIGHTGRAY);
  DrawText("Scroll: Zoom", sidebarX + 10, 560, 10, LIGHTGRAY);
  DrawText("M-Drag: Pan Camera", sidebarX + 10, 575, 10, LIGHTGRAY);
  DrawText("[ / ]: Cycle Tiles", sidebarX + 10, 590, 10, LIGHTGRAY);
  DrawText("CTRL+Z/Y: Undo/Redo", sidebarX + 10, 605, 10, LIGHTGRAY);
  DrawText("CTRL+S: Save", sidebarX + 10, 620, 10, LIGHTGRAY);
  DrawText("CTRL+SHIFT+S: Save As", sidebarX + 10, 635, 10, LIGHTGRAY);
  DrawText("CTRL+O: Open Level", sidebarX + 10, 650, 10, LIGHTGRAY);

  if (statusTimer > 0.0f) {
    DrawText(statusMsg.c_str(), sidebarX + 10, 680, 15, GREEN);
  }
}

void EditorState::saveLevel(const std::string &path) {
  std::ofstream out(path);
  if (out.is_open()) {
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 40; x++) {
        out << (int)tileMap->getTile(x, y) << " ";
      }
      out << "\n";
    }
    out.close();
    statusMsg = "Saved!";
    statusTimer = 2.0f;
  } else {
    statusMsg = "Save Failed!";
    statusTimer = 2.0f;
  }
}

void EditorState::loadLevel(const std::string &path) {
  std::ifstream in(path);
  if (in.is_open()) {
    int tileVal;
    for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 40; x++) {
        if (in >> tileVal) {
          tileMap->setTile(x, y, (TileType)tileVal);
        }
      }
    }
    in.close();
    undoStack.clear();
    redoStack.clear();
    statusMsg = "Loaded!";
    statusTimer = 2.0f;
  } else {
    statusMsg = "Load Failed!";
    statusTimer = 2.0f;
  }
}

/*
=======================================================
=========================TRANSITION======================
=======================================================
*/

TransitionState::TransitionState() = default;
TransitionState::~TransitionState() = default;

void TransitionState::enter() {
  tunnelMap = std::make_unique<TileMap>(40, 15, 32);
  for (int y = 0; y < 15; y++) {
    for (int x = 0; x < 40; x++) {
      tunnelMap->setTile(x, y, TileType::CAVE_ROCK);
    }
  }
  for (int y = 8; y <= 11; y++) {
    for (int x = 15; x <= 25; x++) {
      tunnelMap->setTile(x, y, TileType::NOTHING);
    }
  }
  tunnelMap->setTile(16, 11, TileType::ENTRANCE);
  tunnelMap->setTile(24, 11, TileType::EXIT);
  
  tunnelMap->setTileset(EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png"));
  tunnelMap->setJungleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_junglebg.png"));
  tunnelMap->setTempleTileset(EntityFactory::getTexture("assets/tilemaps/gfx_templebg.png"));
  tunnelMap->setRopeTexture(EntityFactory::getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"));

  physics = std::make_unique<PhysicsSystem>(tunnelMap.get());
  player = std::make_unique<Player>(
      16 * 32.0f + 8.0f, 11 * 32.0f + 8.0f,
      GameManager::getInstance()->getSelectedCharacter());
  player->setTileMap(tunnelMap.get());
  player->startDoorSpawnAnim();

  lighting = std::make_unique<LightingSystem>(40, 15);
  // Lit up by 20% (0.20f) compared to normal caves
  lighting->setAmbientLight({0.35f, 0.35f, 0.45f});
  camera.target = {(20 * 32.0f), (10 * 32.0f)};
  camera.offset = {(float)GetScreenWidth() / 2.0f,
                   (float)GetScreenHeight() / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;
}

void TransitionState::exit() {
  EventBus::getInstance()->clearAllListeners();
  // std::unique_ptr handles cleanup automatically
}

void TransitionState::handleInput() {
  if (player) {
    player->handleInput();
  }
}

void TransitionState::update(float dt) {
  if (player && physics && tunnelMap) {
    player->update(dt);
    if (!player->isDoorAnimPlaying()) {
      player->applyGravity(dt);
      physics->resolveEntityTileCollision(player.get());
    }
    if (lighting) {
      lighting->clearLights();
    }
    if (player) {
      float trueX = (player->getX() + player->getAABB().width / 2.0f) /
                    tunnelMap->getTileSize();
      float trueY = (player->getY() + player->getAABB().height / 2.0f) /
                    tunnelMap->getTileSize();
      double time = GetTime();
      float flicker = std::sin(time * 12.0) * 0.02f;
      flicker += std::sin(time * 23.0) * 0.015f;
      flicker += std::sin(time * 5.0) * 0.01f;
      Vector3 torchColor = {0.95f + flicker, (0.95f + flicker) * 0.9f, (0.95f + flicker) * 0.6f};
      lighting->addLight(trueX, trueY, torchColor,
                         4.5f + (flicker * 1.5f));
      lighting->update(tunnelMap.get());
    }
    
    // Exit door animation completion & check
    if (player->isDoorAnimPlaying()) {
      if (player->isDoorAnimFinished()) {
        Rectangle pRect = player->getAABB();
        int tx = (pRect.x + pRect.width / 2) / tunnelMap->getTileSize();
        int ty = (pRect.y + pRect.height / 2) / tunnelMap->getTileSize();
        if (tx >= 0 && tx < tunnelMap->getWidth() && ty >= 0 &&
            ty < tunnelMap->getHeight()) {
          if (tunnelMap->getTile(tx, ty) == TileType::EXIT) {
            GameManager::getInstance()->syncPlayerStats(
                player->getHealth(), player->getBombs(), player->getRopes(),
                player->getGold());
            if (GameManager::getInstance()->getFloor() >= 3) {
                game->changeState(GameStateType::VICTORY);
            } else {
                GameManager::getInstance()->nextFloor();
                game->changeState(GameStateType::PLAY);
            }
            return;
          }
        }
      }
    } else {
      if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && IsKeyPressed(KEY_Y)) {
        Rectangle pRect = player->getAABB();
        int tx = (pRect.x + pRect.width / 2) / tunnelMap->getTileSize();
        int ty = (pRect.y + pRect.height / 2) / tunnelMap->getTileSize();
        if (tx >= 0 && tx < tunnelMap->getWidth() && ty >= 0 &&
            ty < tunnelMap->getHeight()) {
          if (tunnelMap->getTile(tx, ty) == TileType::EXIT) {
            float targetX = tx * 32.0f + 8.0f;
            player->move(targetX - player->getX(), 0.0f);
            player->startDoorEnterAnim();
          }
        }
      }
    }
  }
}

void TransitionState::render() {
  ClearBackground(BLACK);
  BeginMode2D(camera);
  if (tunnelMap) {
    tunnelMap->renderParallaxBackground(camera);
    if (lighting)
      tunnelMap->render(camera, lighting->getLightMap(), false);
  }
  if (player)
    player->render(1.0f);
  if (tunnelMap && lighting)
    tunnelMap->render(camera, lighting->getLightMap(), true);
  EndMode2D();

  drawCenteredText(
      TextFormat("FLOOR %d COMPLETE", GameManager::getInstance()->getFloor()),
      100.0f, 40.0f, GOLD);
  drawCenteredText(
      TextFormat("SCORE: %d", GameManager::getInstance()->getScore()), 160.0f,
      30.0f, WHITE);

  if (((int)(GetTime() * 2)) % 2 == 0) {
    drawCenteredText("Proceed to exit", GetScreenHeight() - 100.0f, 20.0f,
                     LIGHTGRAY);
  }
}

} // namespace Platformer