#include "GameState.h"
#include "../audio/AudioManager.h"
#include "../core/EventBus.h"
#include "../entities/Arrow.h"
#include "../entities/Bomb.h"
#include "../entities/EntityFactory.h"
#include "../entities/Item.h"
#include "../entities/LavaDrip.h"
#include "../entities/Trap.h"
#include "../entities/enemies/Enemy.h"
#include "../entities/enemies/NemesisGhost.h"
#include "../entities/enemies/Spike.h"
#include "../level/LevelGenerator.h"
#include "../player/Player.h"
#include "../shop/ShopSystem.h"
#include "../ui/ComboSystem.h"
#include "../ui/Minimap.h"
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

/*
=======================================================
=========================MENU==========================
=======================================================
*/

void MenuState::enter() {
  selectedOption = 0;
  // Placeholder: load background texture here
  // Placeholder: start menu BGM here
  // Platformer::AudioManager::getInstance()->playBGM("assets/music/placeholder_menu.ogg");
}

void MenuState::exit() {
  // Placeholder: stop BGM if needed
  // Platformer::AudioManager::getInstance()->stopBGM();
}

void MenuState::handleInput() {
  if (IsKeyPressed(KEY_UP)) {
    selectedOption = (selectedOption + 2) % 3;
  }
  if (IsKeyPressed(KEY_DOWN)) {
    selectedOption = (selectedOption + 1) % 3;
  }
  if (IsKeyPressed(KEY_ENTER)) {
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

  // Placeholder: draw background texture here

  drawCenteredText("CAVERN DESCENT", 150.0f, 60.0f, RAYWHITE);

  Color startColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
  Color editorColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
  Color quitColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

  drawCenteredText("START GAME", 350.0f, 40.0f, startColor);
  drawCenteredText("LEVEL EDITOR", 420.0f, 40.0f, editorColor);
  drawCenteredText("QUIT", 490.0f, 40.0f, quitColor);

  drawCenteredText("Use UP/DOWN to navigate, ENTER to select", 650.0f, 20.0f,
                   GRAY);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

PlayState::PlayState() = default;
PlayState::~PlayState() = default;

void PlayState::enter() {
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

  if (GameManager::getInstance()->getIsCustomLevel()) {
    tempLevel.tileMap = std::make_unique<TileMap>(40, 32, 32);
    tempLevel.playerSpawn = {32.0f, 32.0f}; // default fallback
    tempLevel.exitPos = {200.0f, 200.0f};
    tempLevel.shopArea = {0, 0, 0, 0};

    std::ifstream in(GameManager::getInstance()->getCustomLevelPath());
    if (in.is_open()) {
      int tileVal;
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 40; x++) {
          if (in >> tileVal) {
            TileType type = (TileType)tileVal;

            if (type == TileType::ENTRANCE) {
              tempLevel.playerSpawn = {(float)x * 32.0f + 16.0f,
                                       (float)y * 32.0f + 16.0f};
            } else if (type == TileType::EXIT) {
              tempLevel.exitPos = {(float)x * 32.0f, (float)y * 32.0f};
            }

            tempLevel.tileMap->setTile(x, y, type);
          }
        }
      }
      in.close();

      // Auto-Beautify CAVE_ROCK (Turn into Cave Up, Down, Gold, etc)
      auto isSolid = [&](int x, int y) {
        if (x < 0 || y < 0 || x >= 40 || y >= 32)
          return true;
        TileType t = tempLevel.tileMap->getTile(x, y);
        return t != TileType::NOTHING && t != TileType::LADDER &&
               t != TileType::LADDER_DECK && t != TileType::ENTRANCE &&
               t != TileType::EXIT && t != TileType::ENEMY_SNAKE &&
               t != TileType::ENEMY_BAT && t != TileType::ENEMY_SPIDER &&
               t != TileType::CHEST && t != TileType::LAVA &&
               t != TileType::WATER && t != TileType::SPIKE_TRAP &&
               t != TileType::ARROW_TRAP_LEFT && t != TileType::ARROW_TRAP_RIGHT;
      };

      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 40; x++) {
          TileType type = tempLevel.tileMap->getTile(x, y);

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
              int r = GetRandomValue(1, 100);
              if (r <= 5)
                newTile = TileType::CAVE_SOME_GOLD;
              else if (r <= 7)
                newTile = TileType::CAVE_MUCH_GOLD;
            }
            tempLevel.tileMap->setTile(x, y, newTile);
          }
        }
      }

      // Spawn Custom Entities pass
      for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 40; x++) {
          TileType type = tempLevel.tileMap->getTile(x, y);
          float px = x * 32.0f;
          float py = y * 32.0f;

          if (type == TileType::CAVE_SOME_GOLD ||
              type == TileType::CAVE_MUCH_GOLD) {
            type = TileType::CAVE_ROCK;
            auto gold = EntityFactory::createItem('G', px, py);
            if (gold)
              tempLevel.items.push_back(std::move(gold));
          } else if (type == TileType::CHEST) {
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
    }
  } else {
    tempLevel = tempGenerator->generate(GameManager::getInstance()->getFloor(),
                                        ZoneType::CAVE);
  }

  physics = std::make_unique<PhysicsSystem>(tempLevel.tileMap.get());
  player = std::make_unique<Player>(
      tempLevel.playerSpawn.x, tempLevel.playerSpawn.y,
      GameManager::getInstance()->getSelectedCharacter());
  player->setTileMap(tempLevel.tileMap.get());

  minimap = std::make_unique<Minimap>(tempLevel.exitPos);

  lighting = std::make_unique<LightingSystem>(tempLevel.tileMap->getWidth(),
                                              tempLevel.tileMap->getHeight());

  liquids = std::make_unique<LiquidSimulator>(tempLevel.tileMap.get());
  player->setLiquidSimulator(liquids.get());
  for (const auto &liq : tempLevel.initialLiquids) {
    liquids->addLiquid(liq.gx, liq.gy, 255, liq.type);
  }

  camera.target = Vector2{player->getX(), player->getY()};

  ghostTimer = 0.0f;
  ghostSpawned = false;

  combo = std::make_unique<ComboSystem>();
  // Initialize shop
  shop = std::make_unique<ShopSystem>();
  shop->initializeFromItems(tempLevel.items,
                            GameManager::getInstance()->getFloor());

  EventBus::getInstance()->clearListeners(EventType::EVENT_GOLD_COLLECTED);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_GOLD_COLLECTED, [this](EventData data) {
        if (this->combo)
          this->combo->onTreasureCollected(data.amount, data.worldX,
                                           data.worldY);
      });

  EventBus::getInstance()->clearListeners(EventType::EVENT_ENEMY_KILLED);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_ENEMY_KILLED, [this](EventData data) {
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
        auto flame = EntityFactory::createEnemy('F', data.worldX, data.worldY);
        if (flame) {
          if (data.vy != 0.0f) {
            flame->setVelocity(0.0f, data.vy);
          }
          this->pendingEntities.push_back(std::move(flame));
        }
      });

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

  EventBus::getInstance()->clearListeners(EventType::EVENT_BOMB_EXPLODE);
  EventBus::getInstance()->subscribe(
      EventType::EVENT_BOMB_EXPLODE, [this](EventData data) {
        AudioManager::getInstance()->playSFX("explosion");
        float explosionRadius = 80.0f; // roughly 2.5 tiles (32 * 2.5 = 80)

        // 1. Destroy Terrain
        int tx = (int)(data.worldX / 32.0f);
        int ty = (int)(data.worldY / 32.0f);
        for (int y = ty - 2; y <= ty + 2; y++) {
          for (int x = tx - 2; x <= tx + 2; x++) {
            if (x > 0 && x < tempLevel.tileMap->getWidth() - 1 && y > 0 &&
                y < tempLevel.tileMap->getHeight() - 1) {
              if (tempLevel.tileMap->isSolid(x, y)) {
                tempLevel.tileMap->destroyBlock(x, y);
              }
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
            player->takeDamage(10);
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
      });

  // 4. Pass liquids to enemies
  for (auto &entity : tempLevel.dynamicEntities) {
    if (Enemy *enemy = dynamic_cast<Enemy *>(entity.get())) {
      enemy->setLiquidSim(liquids.get());
    }
  }
}

void PlayState::exit() {
  physics.reset();
  player.reset();

  EventBus::getInstance()->clearAllListeners();
}

void PlayState::handleInput() {
  if (IsKeyPressed(KEY_ESCAPE)) {
    game->pushState(GameStateType::PAUSE);
    return;
  }
  if (player) {
    player->handleInput();
  }
}

void PlayState::update(float dt) {
  if (!ghostSpawned) {
    ghostTimer += dt;
    if (ghostTimer >= 60.0f) { // 1 minute
      ghostSpawned = true;
      auto ghost = EntityFactory::createGhost(player->getX() - 600.0f,
                                              player->getY() - 600.0f);
      pendingEntities.push_back(std::move(ghost));
      AudioManager::getInstance()->playSFX("ghost_spawn");
    }
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
            AudioManager::getInstance()->playSFX("hit");
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
            player->takeDamage(2); // Take 2 hearts damage
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

    for (auto &item : tempLevel.items) {
      if (item && !item->isPickedUp() && !item->isShopItem &&
          item->getType() != ItemType::CHEST) {
        if (physics->checkAABBOverlap(pickupBox, item->getAABB())) {
          item->activate(player.get());
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

    camera.target = Vector2Lerp(camera.target, desiredTarget, 5.0f * dt);

    if (liquids) {
      liquids->update(dt);
      liquids->updateSpurts(dt, player->getX(), player->getY());
    }

    if (minimap) {
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

      lighting->addLight(trueX, trueY, intensity, radius);

      // Shop Lantern glow
      if (tempLevel.shopArea.width > 0) {
        float shopCx =
            (tempLevel.shopArea.x + tempLevel.shopArea.width / 2.0f) /
            tempLevel.tileMap->getTileSize();
        float shopCy = (tempLevel.shopArea.y + 160.0f) /
                       tempLevel.tileMap->getTileSize(); // At y=5 in the room
        double t = GetTime();
        float flicker = std::sin(t * 15.0) * 0.03f + std::sin(t * 22.0) * 0.02f;
        lighting->addLight(shopCx, shopCy, 1.2f + flicker,
                           8.0f + (flicker * 2.0f));
      }

      lighting->update(tempLevel.tileMap.get());
    }

    // Handle Shop UI Interaction
    if (shop && tempLevel.shopArea.width > 0) {
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
    if (player->getHealth() <= 0) {
      game->changeState(GameStateType::GAME_OVER);
      return;
    }

    // Exit check (requires manual UP+Y input)
    if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && IsKeyPressed(KEY_Y)) {
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
    if (ty >= 0 && ty < lMap.size() && tx >= 0 && tx < lMap[ty].size()) {
      return lMap[ty][tx];
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
    } else if (tempLevel.shopArea.width > 0 && player &&
               CheckCollisionRecs(player->getAABB(), tempLevel.shopArea)) {
      const char *text = "PRESS 'Y' TO OPEN OR CLOSE SHOP";
      float fontSize = 30.0f;
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

void PauseState::enter() {}

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
=========================CHARSELECT====================
=======================================================
*/

void CharSelectState::enter() { selectedIndex = 0; }

void CharSelectState::exit() {}

void CharSelectState::handleInput() {
  if (IsKeyPressed(KEY_LEFT)) {
    selectedIndex = (selectedIndex + 2) % 3;
  }
  if (IsKeyPressed(KEY_RIGHT)) {
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

  drawCenteredText("CHARACTER SELECT", 200.0f, 40.0f, RAYWHITE);

  Color expColor = (selectedIndex == 0) ? YELLOW : DARKGRAY;
  Color ninColor = (selectedIndex == 1) ? YELLOW : DARKGRAY;
  Color tnkColor = (selectedIndex == 2) ? YELLOW : DARKGRAY;

  drawCenteredAt("EXPLORER", 320.0f, 400.0f, 30.0f, expColor);
  drawCenteredAt("NINJA", 640.0f, 400.0f, 30.0f, ninColor);
  drawCenteredAt("TANK", 960.0f, 400.0f, 30.0f, tnkColor);

  drawCenteredText("Press ENTER to start, ESC to return", 600.0f, 20.0f, GRAY);
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

  drawCenteredText("LEVEL EDITOR MENU", 150.0f, 60.0f, RAYWHITE);

  Color playColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
  Color editColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
  Color backColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

  drawCenteredText("PLAY CUSTOM LEVEL", 300.0f, 40.0f, playColor);
  drawCenteredText("EDITOR", 370.0f, 40.0f, editColor);
  drawCenteredText("BACK", 440.0f, 40.0f, backColor);

  drawCenteredText("Use UP/DOWN to navigate, ENTER to select", 650.0f, 20.0f,
                   GRAY);
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
        GameManager::getInstance()->setLoadIntoEditor(true);
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

  drawCenteredText("FILE MENU", 150.0f, 60.0f, RAYWHITE);

  Color newColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
  Color openColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
  Color backColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

  drawCenteredText("NEW LEVEL", 300.0f, 40.0f, newColor);
  drawCenteredText("OPEN LEVEL", 370.0f, 40.0f, openColor);
  drawCenteredText("BACK", 440.0f, 40.0f, backColor);

  drawCenteredText("Use UP/DOWN to navigate, ENTER to select", 650.0f, 20.0f,
                   GRAY);
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

void EditorState::enter() {
  EntityFactory::preloadTextures();
  tileMap = std::make_unique<TileMap>(40, 32, 16);

  camera = {0};
  camera.target = Vector2{0.0f, 0.0f};
  camera.offset = Vector2{0.0f, 0.0f};
  camera.rotation = 0.0f;
  camera.zoom = 2.0f;

  Texture2D caveTex =
      EntityFactory::getTexture("assets/tilemaps/gfx_cavebg.png");
  Texture2D spikeTex = EntityFactory::getTexture(
      "assets/sprites/16x16/gfx_spike_collectibles_flame.png");
  Texture2D arrowTex =
      EntityFactory::getTexture("assets/sprites/8x8/gfx_arrow.png");
  Texture2D batSnakeTex = EntityFactory::getTexture(
      "assets/sprites/16x16/gfx_bat_snake_jetpack.png");
  Texture2D spiderTex =
      EntityFactory::getTexture("assets/sprites/16x16/gfx_spider_skeleton.png");
  Texture2D goldTex =
      EntityFactory::getTexture("assets/sprites/16x16/gfx_goldbars.png");
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
      {TileType::ENTRANCE, caveTex, {0, 96, 16, 16}, "Enter"}, // col 0 row 6
      {TileType::EXIT,
       caveTex,
       {16, 96, 16, 16},
       "Exit"}, // Using exit door col 1 row 6
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

  if (GameManager::getInstance()->getLoadIntoEditor()) {
    loadLevel(GameManager::getInstance()->getCustomLevelPath());
    GameManager::getInstance()->setLoadIntoEditor(false);
  }
}

void EditorState::exit() {}

void EditorState::handleInput() {
  // Zoom
  float wheelMove = GetMouseWheelMove();
  if (wheelMove != 0.0f) {
    // Zoom relative to mouse position
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    camera.offset = GetMousePosition();
    camera.target = mouseWorldPos;
    camera.zoom += wheelMove * 0.25f;
    if (camera.zoom < 0.5f)
      camera.zoom = 0.5f;
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
  if (mousePos.x > 1080) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      // Check palette clicks (Grid: 2 cols)
      for (int i = 0; i < (int)paletteItems.size(); i++) {
        int col = i % 2;
        int row = i / 2;
        float px = 1080.0f + 10.0f + col * 95.0f;
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
    int tx = (int)floorf(mouseWorld.x / 16.0f);
    int ty = (int)floorf(mouseWorld.y / 16.0f);
    mouseGridPos = {(float)tx, (float)ty};

    if (tx >= 0 && tx < 40 && ty >= 0 && ty < 32) {
      // Placement
      if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        tileMap->setTile(tx, ty, paletteItems[selectedTileIdx].type);
      }
      // Erase
      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
        tileMap->setTile(tx, ty, TileType::NOTHING);
      }
    }
  }

  // Hotkeys (CTRL+S, CTRL+SHIFT+S, CTRL+O)
  bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
  bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

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
      // Normal Save
      saveLevel(GameManager::getInstance()->getCustomLevelPath());
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

  // Render Editor
  BeginMode2D(camera);
  std::vector<std::vector<float>> dummyLight(32, std::vector<float>(40, 1.0f));
  tileMap->render(camera, dummyLight, false);
  tileMap->render(camera, dummyLight, true);

  // Overlay custom items missing from standard TileMap renderer
  for (int y = 0; y < tileMap->getHeight(); y++) {
    for (int x = 0; x < tileMap->getWidth(); x++) {
      TileType t = tileMap->getTile(x, y);
      if (static_cast<int>(t) > 42) {
        // Find texture inside paletteItems
        for (const auto &item : paletteItems) {
          if (item.type == t) {
            Rectangle dest = {x * 16.0f, y * 16.0f, 16.0f, 16.0f};
            DrawTexturePro(item.tex, item.src, dest, {0, 0}, 0.0f, WHITE);
            break;
          }
        }
      }
    }
  }

  for (int i = 0; i <= 40; i++) {
    DrawLine(i * 16, 0, i * 16, 32 * 16, ColorAlpha(WHITE, 0.2f));
  }
  for (int j = 0; j <= 32; j++) {
    DrawLine(0, j * 16, 40 * 16, j * 16, ColorAlpha(WHITE, 0.2f));
  }

  if (GetMousePosition().x <= 1080) {
    DrawRectangle(mouseGridPos.x * 16, mouseGridPos.y * 16, 16, 16,
                  ColorAlpha(YELLOW, 0.5f));
  }
  EndMode2D();

  // Render Sidebar Palette
  DrawRectangle(1080, 0, 200, 720, ColorAlpha(BLACK, 0.9f));
  DrawText("PALETTE", 1090, 10, 20, WHITE);
  DrawLine(1080, 40, 1280, 40, GRAY);

  for (int i = 0; i < (int)paletteItems.size(); i++) {
    int col = i % 2;
    int row = i / 2;
    float px = 1080.0f + 10.0f + col * 95.0f;
    float py = 60.0f + row * 45.0f;

    Color bgColor = (i == selectedTileIdx) ? ColorAlpha(YELLOW, 0.3f) : BLANK;
    DrawRectangle(px - 5, py - 5, 90, 40, bgColor);

    Rectangle dest = {px, py, 24.0f, 24.0f};
    DrawTexturePro(paletteItems[i].tex, paletteItems[i].src, dest, {0, 0}, 0.0f,
                   WHITE);

    // Shrink font size slightly to fit 2 columns
    DrawText(paletteItems[i].name.c_str(), px + 28, py + 6, 10, WHITE);
  }

  DrawLine(1080, 520, 1280, 520, GRAY);
  DrawText("L-Click: Place", 1090, 545, 10, LIGHTGRAY);
  DrawText("R-Click: Erase", 1090, 560, 10, LIGHTGRAY);
  DrawText("Scroll: Zoom", 1090, 575, 10, LIGHTGRAY);
  DrawText("M-Drag: Pan Camera", 1090, 590, 10, LIGHTGRAY);
  DrawText("CTRL+S: Save", 1090, 605, 10, LIGHTGRAY);
  DrawText("CTRL+SHIFT+S: Save As", 1090, 620, 10, LIGHTGRAY);
  DrawText("CTRL+O: Open Level", 1090, 635, 10, LIGHTGRAY);

  if (statusTimer > 0.0f) {
    DrawText(statusMsg.c_str(), 1090, 680, 15, GREEN);
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
  physics = std::make_unique<PhysicsSystem>(tunnelMap.get());
  player = std::make_unique<Player>(
      16 * 32.0f, 11 * 32.0f,
      GameManager::getInstance()->getSelectedCharacter());
  lighting = std::make_unique<LightingSystem>(40, 15);
  lighting->setAmbientLight(0.25f);
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
    player->applyGravity(dt);
    physics->resolveEntityTileCollision(player.get());
    if (lighting) {
      lighting->clearLights();
      float trueX = (player->getX() + player->getAABB().width / 2.0f) /
                    tunnelMap->getTileSize();
      float trueY = (player->getY() + player->getAABB().height / 2.0f) /
                    tunnelMap->getTileSize();
      double time = GetTime();
      float flicker =
          std::sin(time * 12.0) * 0.02f + std::sin(time * 23.0) * 0.015f;
      lighting->addLight(trueX, trueY, 0.95f + flicker,
                         4.5f + (flicker * 1.5f));
      lighting->update(tunnelMap.get());
    }
    if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && IsKeyPressed(KEY_Y)) {
      Rectangle pRect = player->getAABB();
      int tx = (pRect.x + pRect.width / 2) / tunnelMap->getTileSize();
      int ty = (pRect.y + pRect.height / 2) / tunnelMap->getTileSize();
      if (tx >= 0 && tx < tunnelMap->getWidth() && ty >= 0 &&
          ty < tunnelMap->getHeight()) {
        if (tunnelMap->getTile(tx, ty) == TileType::EXIT) {
          GameManager::getInstance()->syncPlayerStats(
              player->getHealth(), player->getBombs(), player->getRopes(),
              player->getGold());
          GameManager::getInstance()->nextFloor();
          game->changeState(GameStateType::PLAY);
          return;
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