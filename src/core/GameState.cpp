#include "GameState.h"
#include "Game.h"
#include "GameManager.h"
#include "../audio/AudioManager.h"
#include "../player/Player.h"
#include "../ui/Minimap.h"
#include "raymath.h"
#include "../core/EventBus.h"
#include "../entities/EntityFactory.h"
#include "../entities/Item.h"
#include "../entities/Trap.h"
#include "../entities/Arrow.h"
#include "../entities/Bomb.h"
#include "../entities/enemies/Enemy.h"
#include "../entities/enemies/NemesisGhost.h"
#include "../entities/enemies/Spike.h"

namespace Platformer {

void GameState::setGame(Game* game) {
    this->game = game;
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
                game->changeState(GameStateType::EDITOR);
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

    DrawText("CAVERN DESCENT", 400, 150, 60, RAYWHITE);

    Color startColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
    Color editorColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
    Color quitColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

    DrawText("START GAME", 500, 350, 40, startColor);
    DrawText("LEVEL EDITOR", 500, 420, 40, editorColor);
    DrawText("QUIT", 500, 490, 40, quitColor);

    DrawText("Use UP/DOWN to navigate, ENTER to select", 400, 650, 20, GRAY);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

void PlayState::enter() {
    EntityFactory::preloadTextures();
    // Initialize Camera
    camera.offset = Vector2{ 1280.0f / 2.0f, 720.0f / 2.0f }; // Center screen
    camera.rotation = 0.0f;
    camera.zoom = 2.0f;

    tempGenerator = std::make_unique<LevelGenerator>();
    tempLevel = tempGenerator->generate(1, ZoneType::CAVE);

    physics = new PhysicsSystem(tempLevel.tileMap.get());

    // TODO: Person B will implement LevelManager to spawn the player.
    // For now, we manually instantiate a temporary Player at the generated spawn point.
    player = new Player(tempLevel.playerSpawn.x, tempLevel.playerSpawn.y, GameManager::getInstance()->getSelectedCharacter());
    player->setTileMap(tempLevel.tileMap.get());
    
    minimap = std::make_unique<Minimap>(tempLevel.exitPos);
    
    lighting = std::make_unique<LightingSystem>(tempLevel.tileMap->getWidth(), tempLevel.tileMap->getHeight());
    
    camera.target = Vector2{ player->getX(), player->getY() };
    
    ghostTimer = 0.0f;
    ghostSpawned = false;

    EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_ITEM);
    EventBus::getInstance()->subscribe(EventType::EVENT_SPAWN_ITEM, [this](EventData data) {
        auto item = EntityFactory::createItem(data.amount, data.worldX, data.worldY);
        if (item) {
            item->setVelocity(data.vx, data.vy);
            this->pendingItems.push_back(std::move(item));
        }
    });

    EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_BOMB);
    EventBus::getInstance()->subscribe(EventType::EVENT_SPAWN_BOMB, [this](EventData data) {
        auto bomb = std::make_unique<Bomb>(data.worldX, data.worldY, data.vx, data.vy);
        this->pendingEntities.push_back(std::move(bomb));
    });

    EventBus::getInstance()->clearListeners(EventType::EVENT_SPAWN_ARROW);
    EventBus::getInstance()->subscribe(EventType::EVENT_SPAWN_ARROW, [this](EventData data) {
        auto arrow = EntityFactory::createArrow(data.worldX, data.worldY, data.vx);
        this->pendingEntities.push_back(std::move(arrow));
    });

    EventBus::getInstance()->clearListeners(EventType::EVENT_BOMB_EXPLODE);
    EventBus::getInstance()->subscribe(EventType::EVENT_BOMB_EXPLODE, [this](EventData data) {
        AudioManager::getInstance()->playSFX("explosion");
        float explosionRadius = 80.0f; // roughly 2.5 tiles (32 * 2.5 = 80)
        
        // 1. Destroy Terrain
        int tx = (int)(data.worldX / 32.0f);
        int ty = (int)(data.worldY / 32.0f);
        for (int y = ty - 2; y <= ty + 2; y++) {
            for (int x = tx - 2; x <= tx + 2; x++) {
                if (x > 0 && x < tempLevel.tileMap->getWidth() - 1 && y > 0 && y < tempLevel.tileMap->getHeight() - 1) {
                    if (tempLevel.tileMap->isSolid(x, y)) {
                        tempLevel.tileMap->destroyBlock(x, y);
                    }
                }
            }
        }
        
        // 2. Damage Player
        if (player) {
            float dx = player->getX() + player->getAABB().width / 2.0f - data.worldX;
            float dy = player->getY() + player->getAABB().height / 2.0f - data.worldY;
            float dist = std::sqrt(dx*dx + dy*dy);
            if (dist < explosionRadius) {
                player->takeDamage(10);
                player->setVelocity(dx > 0 ? 300.0f : -300.0f, -200.0f);
            }
        }
        
        // 3. Damage Entities
        for (auto& entity : tempLevel.dynamicEntities) {
            if (entity && entity->isAlive()) {
                if (Enemy* enemy = dynamic_cast<Enemy*>(entity.get())) {
                    float dx = enemy->getX() + enemy->getAABB().width / 2.0f - data.worldX;
                    float dy = enemy->getY() + enemy->getAABB().height / 2.0f - data.worldY;
                    float dist = std::sqrt(dx*dx + dy*dy);
                    if (dist < explosionRadius) {
                        enemy->takeDamage(10);
                        enemy->setVelocity(dx > 0 ? 300.0f : -300.0f, -200.0f);
                    }
                }
            }
        }
    });
}

void PlayState::exit() {
    if (physics) {
        delete physics;
        physics = nullptr;
    }
    // Cleanup temporary player until LevelManager manages it.
    if (player) {
        delete player;
        player = nullptr;
    }
}

void PlayState::handleInput() {
    if (player) {
        player->handleInput();
    }
}

void PlayState::update(float dt) {
    if (!ghostSpawned) {
        ghostTimer += dt;
        if (ghostTimer >= 60.0f) { // 1 minute
            ghostSpawned = true;
            auto ghost = EntityFactory::createGhost(player->getX() - 600.0f, player->getY() - 600.0f);
            pendingEntities.push_back(std::move(ghost));
            AudioManager::getInstance()->playSFX("ghost_spawn");
        }
    }

    // Merge pending items
    for (auto& item : pendingItems) {
        tempLevel.items.push_back(std::move(item));
    }
    pendingItems.clear();

    // Merge pending entities
    for (auto& ent : pendingEntities) {
        tempLevel.dynamicEntities.push_back(std::move(ent));
    }
    pendingEntities.clear();

    if (player) {
        player->update(dt, nullptr);
        
        if (physics) {
            physics->resolveEntityTileCollision(player);
            
            for (auto& entity : tempLevel.dynamicEntities) {
                if (entity && entity->isAlive()) {
                    entity->update(dt, player);
                    physics->resolveEntityTileCollision(entity.get());
                }
            }
            
            for (auto& item : tempLevel.items) {
                if (item && !item->isPickedUp()) {
                    item->update(dt, player);
                    physics->resolveEntityTileCollision(item.get());
                }
            }
        }
        
        // ---- Entity Collisions ----
        Rectangle pAABB = player->getAABB();
        bool whipActive = player->getIsWhipHitThisFrame();
        Rectangle whipBox = player->getWhipHitbox();
        
        // 1. Player vs DynamicEntities (Enemies/Ghost)
        for (auto& entity : tempLevel.dynamicEntities) {
            if (entity && entity->isAlive()) {
                if (auto* enemy = dynamic_cast<Enemy*>(entity.get())) {
                    if (whipActive && physics->checkAABBOverlap(whipBox, enemy->getAABB())) {
                        enemy->takeDamage(1); // Whip does 1 damage
                        AudioManager::getInstance()->playSFX("hit");
                    }
                    
                    if (physics->checkAABBOverlap(pAABB, enemy->getAABB())) {
                        if (!dynamic_cast<Spike*>(enemy)) {
                            player->takeDamage(enemy->getDamage());
                        }
                    }
                } else if (auto* arrow = dynamic_cast<Arrow*>(entity.get())) {
                    if (!arrow->isStuck() && std::abs(arrow->getVelocityX()) > 1.5f && physics->checkAABBOverlap(pAABB, arrow->getAABB())) {
                        player->takeDamage(2); // Take 2 hearts damage
                        player->setVelocity(arrow->getVelocityX() > 0 ? 300.0f : -300.0f, -200.0f);
                        arrow->destroy();
                    }
                }
            }
        }

        // 2. Player vs Traps
        for (auto& trap : tempLevel.traps) {
            if (trap) {
                trap->updateTrap(dt, player, tempLevel.dynamicEntities, tempLevel.items, tempLevel.tileMap.get());
                
                if (trap->getDamage() > 0 && physics->checkAABBOverlap(pAABB, trap->getAABB())) {
                    if (!player->isInvincible()) {
                        player->takeDamage(trap->getDamage());
                        player->setVelocity(player->getX() < trap->getX() ? -250.0f : 250.0f, -200.0f);
                    }
                }
            }
        }

        // 3. Enemies vs Traps
        for (auto& entity : tempLevel.dynamicEntities) {
            if (entity && entity->isAlive()) {
                Rectangle eAABB = entity->getAABB();
                for (auto& trap : tempLevel.traps) {
                    if (trap && trap->getDamage() > 0 && physics->checkAABBOverlap(eAABB, trap->getAABB())) {
                        if (auto* enemy = dynamic_cast<Enemy*>(entity.get())) {
                            enemy->takeDamage(trap->getDamage());
                            enemy->setVelocity(enemy->getX() < trap->getX() ? -200.0f : 200.0f, -150.0f);
                        }
                    }
                }
            }
        }

        // 4. DynamicEntity vs DynamicEntity (Soft Push-Out & Arrow Hits)
        for (size_t i = 0; i < tempLevel.dynamicEntities.size(); ++i) {
            if (!tempLevel.dynamicEntities[i] || !tempLevel.dynamicEntities[i]->isAlive()) continue;
            for (size_t j = i + 1; j < tempLevel.dynamicEntities.size(); ++j) {
                if (!tempLevel.dynamicEntities[j] || !tempLevel.dynamicEntities[j]->isAlive()) continue;
                
                auto& e1 = tempLevel.dynamicEntities[i];
                auto& e2 = tempLevel.dynamicEntities[j];
                Rectangle a = e1->getAABB();
                Rectangle b = e2->getAABB();
                
                if (physics->checkAABBOverlap(a, b)) {
                    Arrow* arrow = dynamic_cast<Arrow*>(e1.get());
                    Enemy* enemy = dynamic_cast<Enemy*>(e2.get());
                    
                    if (!arrow) {
                        arrow = dynamic_cast<Arrow*>(e2.get());
                        enemy = dynamic_cast<Enemy*>(e1.get());
                    }
                    
                    if (arrow && enemy && !arrow->isStuck() && std::abs(arrow->getVelocityX()) > 1.5f) {
                        enemy->takeDamage(100); // Instantly kill enemy
                        arrow->destroy();
                    } else if (dynamic_cast<Spike*>(e1.get()) && dynamic_cast<Enemy*>(e2.get())) {
                        auto spike = dynamic_cast<Spike*>(e1.get());
                        auto otherEnemy = dynamic_cast<Enemy*>(e2.get());
                        if (otherEnemy->getVelocityY() > 50.0f) {
                            otherEnemy->takeDamage(100);
                            spike->setBlood();
                        }
                    } else if (dynamic_cast<Spike*>(e2.get()) && dynamic_cast<Enemy*>(e1.get())) {
                        auto spike = dynamic_cast<Spike*>(e2.get());
                        auto otherEnemy = dynamic_cast<Enemy*>(e1.get());
                        if (otherEnemy->getVelocityY() > 50.0f) {
                            otherEnemy->takeDamage(100);
                            spike->setBlood();
                        }
                    } else if (dynamic_cast<Enemy*>(e1.get()) && dynamic_cast<Enemy*>(e2.get()) && !dynamic_cast<Spike*>(e1.get()) && !dynamic_cast<Spike*>(e2.get())) {
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
        for (auto& item : tempLevel.items) {
            if (item && !item->isPickedUp()) {
                if (physics->checkAABBOverlap(pAABB, item->getAABB())) {
                    item->activate(player);
                }
            }
        }
        
        // Merge pending items
        for (auto& item : pendingItems) {
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
        if (desiredTarget.x < halfScreenWidth - borderPixelsX) desiredTarget.x = halfScreenWidth - borderPixelsX;
        if (desiredTarget.x > mapWidth + borderPixelsX - halfScreenWidth) desiredTarget.x = mapWidth + borderPixelsX - halfScreenWidth;
        
        // Clamp Y
        if (desiredTarget.y < halfScreenHeight - borderPixelsY) desiredTarget.y = halfScreenHeight - borderPixelsY;
        if (desiredTarget.y > mapHeight + borderPixelsY - halfScreenHeight) desiredTarget.y = mapHeight + borderPixelsY - halfScreenHeight;

        camera.target = Vector2Lerp(camera.target, desiredTarget, 5.0f * dt);
        
        if (minimap) {
            minimap->update(player->getX(), player->getY());
        }
        
        if (lighting) {
            lighting->clearLights();
            
            // Add Player Torch
            // Use the exact float position (in tile coordinates) for smooth, sub-tile distance falloff
            Rectangle pRect = player->getAABB();
            float trueX = (pRect.x + pRect.width / 2.0f) / 32.0f;
            float trueY = (pRect.y + pRect.height / 2.0f) / 32.0f;
            
            // Create a smooth organic flicker using composite sine waves and a tiny bit of random noise
            double time = GetTime();
            float flicker = 0.0f;
            flicker += std::sin(time * 12.0) * 0.02f;
            flicker += std::sin(time * 23.0) * 0.015f;
            flicker += std::sin(time * 5.0) * 0.01f;
            flicker += ((float)GetRandomValue(-100, 100) / 100.0f) * 0.005f; // micro crackles
            
            float intensity = 0.95f + flicker;
            float radius = 24.0f + (flicker * 15.0f);
            
            lighting->addLight(trueX, trueY, intensity, radius);
            
            lighting->update(tempLevel.tileMap.get());
        }

        // Handle Shop Item Interaction (Y Key)
        if (IsKeyPressed(KEY_Y)) {
            for (auto& item : tempLevel.items) {
                if (item && !item->isPickedUp() && item->isShopItem && item->getType() != ItemType::CHEST) {
                    Rectangle pRect = player->getAABB();
                    Rectangle iRect = item->getAABB();
                    // Check overlap and activate manually
                    if (pRect.x < iRect.x + iRect.width && pRect.x + pRect.width > iRect.x &&
                        pRect.y < iRect.y + iRect.height && pRect.y + pRect.height > iRect.y) {
                        item->activate(player);
                        break;
                    }
                }
            }
        }

        // Auto-pickup normal items
        for (auto& item : tempLevel.items) {
            if (item && !item->isPickedUp() && !item->isShopItem && item->getType() != ItemType::CHEST && item->getIsGrounded()) {
                Rectangle pRect = player->getAABB();
                Rectangle iRect = item->getAABB();
                // Check overlap and activate automatically
                if (pRect.x < iRect.x + iRect.width && pRect.x + pRect.width > iRect.x &&
                    pRect.y < iRect.y + iRect.height && pRect.y + pRect.height > iRect.y) {
                    item->activate(player);
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
            tempLevel.tileMap->render(camera, lighting->getLightMap(), false); // Background pass
        }
    }

    auto getEntityLight = [&](float x, float y, float w, float h) -> float {
        if (!lighting || !tempLevel.tileMap) return 1.0f;
        int tx = static_cast<int>((x + w/2) / tempLevel.tileMap->getTileSize());
        int ty = static_cast<int>((y + h/2) / tempLevel.tileMap->getTileSize());
        const auto& lMap = lighting->getLightMap();
        if (ty >= 0 && ty < lMap.size() && tx >= 0 && tx < lMap[ty].size()) {
            return lMap[ty][tx];
        }
        return 1.0f;
    };

    for (auto& item : tempLevel.items) {
        if (item && item->isAlive() && !item->isPickedUp()) {
            item->render(getEntityLight(item->getX(), item->getY(), item->getAABB().width, item->getAABB().height));
        }
    }
    for (auto& trap : tempLevel.traps) {
        if (trap && trap->isAlive()) {
            trap->render(getEntityLight(trap->getX(), trap->getY(), trap->getAABB().width, trap->getAABB().height));
        }
    }
    for (auto& enemy : tempLevel.dynamicEntities) {
        if (enemy && enemy->isAlive()) {
            // Draw all dynamic entities EXCEPT the ghost
            if (!dynamic_cast<NemesisGhost*>(enemy.get())) {
                enemy->render(getEntityLight(enemy->getX(), enemy->getY(), enemy->getAABB().width, enemy->getAABB().height));
            }
        }
    }

    if (player) {
        player->render(getEntityLight(player->getX(), player->getY(), player->getAABB().width, player->getAABB().height));
    }

    // Render foreground tiles LAST so they overlap the player's head and entities
    if (tempLevel.tileMap && lighting) {
        tempLevel.tileMap->render(camera, lighting->getLightMap(), true); // Foreground pass (Solid blocks)
    }
    
    // Render Ghost OVER foreground tiles as a transparent shadow
    for (auto& enemy : tempLevel.dynamicEntities) {
        if (enemy && enemy->isAlive()) {
            if (dynamic_cast<NemesisGhost*>(enemy.get())) {
                enemy->render(getEntityLight(enemy->getX(), enemy->getY(), enemy->getAABB().width, enemy->getAABB().height));
            }
        }
    }
    
    EndMode2D();
    
    if (minimap) {
        minimap->render();
    }
}

/*
=======================================================
=========================PAUSE=========================
=======================================================
*/

void PauseState::enter() {

}

void PauseState::exit() {

}

void PauseState::handleInput() {

}

void PauseState::update(float dt) {

}

void PauseState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================GAMEOVER======================
=======================================================
*/

void GameOverState::enter() {

}

void GameOverState::exit() {

}

void GameOverState::handleInput() {

}

void GameOverState::update(float dt) {

}

void GameOverState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================CHARSELECT====================
=======================================================
*/

void CharSelectState::enter() {
    selectedIndex = 0;
}

void CharSelectState::exit() {

}

void CharSelectState::handleInput() {
    if (IsKeyPressed(KEY_LEFT)) {
        selectedIndex = (selectedIndex + 2) % 3;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        selectedIndex = (selectedIndex + 1) % 3;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        GameManager::getInstance()->setSelectedCharacter(characters[selectedIndex]);
        game->changeState(GameStateType::PLAY);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->changeState(GameStateType::MENU);
    }
}

void CharSelectState::update(float dt) {

}

void CharSelectState::render() {
    ClearBackground(BLACK);
    
    DrawText("CHARACTER SELECT", 450, 200, 40, RAYWHITE);
    
    Color expColor = (selectedIndex == 0) ? YELLOW : DARKGRAY;
    Color ninColor = (selectedIndex == 1) ? BLUE : DARKGRAY;
    Color tnkColor = (selectedIndex == 2) ? RED : DARKGRAY;
    
    DrawText("EXPLORER", 300, 400, 30, expColor);
    DrawText("NINJA", 600, 400, 30, ninColor);
    DrawText("TANK", 900, 400, 30, tnkColor);
    
    DrawText("Press ENTER to start, ESC to return", 420, 600, 20, GRAY);
}

/*
=======================================================
=========================EDITOR====================
=======================================================
*/

void EditorState::enter() {

}

void EditorState::exit() {

}

void EditorState::handleInput() {

}

void EditorState::update(float dt) {

}

void EditorState::render() {
    ClearBackground(GREEN);
}

}