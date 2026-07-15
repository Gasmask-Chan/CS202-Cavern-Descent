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
#include "../entities/Bomb.h"
#include "../entities/enemies/Enemy.h"
#include "../entities/enemies/NemesisGhost.h"

namespace Platformer {

void GameState::setGame(Game* game) {
    this->game = game;
}

void GameState::drawCenteredText(const char* text, float y, float fontSize, Color color) {
    Vector2 size = MeasureTextEx(game->getFont(), text, fontSize, 2.0f);
    DrawTextEx(game->getFont(), text, { (1280.0f - size.x) / 2.0f, y }, fontSize, 2.0f, color);
}

void GameState::drawCenteredAt(const char* text, float centerX, float y, float fontSize, Color color) {
    Vector2 size = MeasureTextEx(game->getFont(), text, fontSize, 2.0f);
    DrawTextEx(game->getFont(), text, { centerX - size.x / 2.0f, y }, fontSize, 2.0f, color);
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

    drawCenteredText("CAVERN DESCENT", 150.0f, 60.0f, RAYWHITE);

    Color startColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
    Color editorColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
    Color quitColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

    drawCenteredText("START GAME", 350.0f, 40.0f, startColor);
    drawCenteredText("LEVEL EDITOR", 420.0f, 40.0f, editorColor);
    drawCenteredText("QUIT", 490.0f, 40.0f, quitColor);

    drawCenteredText("Use UP/DOWN to navigate, ENTER to select", 650.0f, 20.0f, GRAY);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

void PlayState::enter() {
    EntityFactory::preloadTextures();
    
    Image hudImg = LoadImage("assets/sprites/16x16/gfx_hud.png");
    ImageFormat(&hudImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color chromaKey = GetImageColor(hudImg, 0, 0); 
    ImageColorReplace(&hudImg, chromaKey, BLANK);
    hudIcons = LoadTextureFromImage(hudImg);
    UnloadImage(hudImg);
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
    UnloadTexture(hudIcons);
    
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
        if (ghostTimer >= 10.0f) { // 2.5 minutes
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
                        player->takeDamage(enemy->getDamage());
                    }
                }
            }
        }

        // 2. Player vs Traps
        for (auto& trap : tempLevel.traps) {
            if (trap) {
                if (physics->checkAABBOverlap(pAABB, trap->getAABB())) {
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
                    if (trap && physics->checkAABBOverlap(eAABB, trap->getAABB())) {
                        if (auto* enemy = dynamic_cast<Enemy*>(entity.get())) {
                            enemy->takeDamage(trap->getDamage());
                            enemy->setVelocity(enemy->getX() < trap->getX() ? -200.0f : 200.0f, -150.0f);
                        }
                    }
                }
            }
        }

        // 4. Enemy vs Enemy (Soft Push-Out)
        for (size_t i = 0; i < tempLevel.dynamicEntities.size(); ++i) {
            if (!tempLevel.dynamicEntities[i] || !tempLevel.dynamicEntities[i]->isAlive()) continue;
            for (size_t j = i + 1; j < tempLevel.dynamicEntities.size(); ++j) {
                if (!tempLevel.dynamicEntities[j] || !tempLevel.dynamicEntities[j]->isAlive()) continue;
                
                Rectangle a = tempLevel.dynamicEntities[i]->getAABB();
                Rectangle b = tempLevel.dynamicEntities[j]->getAABB();
                if (physics->checkAABBOverlap(a, b)) {
                    // Push apart horizontally
                    if (a.x < b.x) {
                        tempLevel.dynamicEntities[i]->setVelocity(tempLevel.dynamicEntities[i]->getVelocityX() - 50.0f, tempLevel.dynamicEntities[i]->getVelocityY());
                        tempLevel.dynamicEntities[j]->setVelocity(tempLevel.dynamicEntities[j]->getVelocityX() + 50.0f, tempLevel.dynamicEntities[j]->getVelocityY());
                    } else {
                        tempLevel.dynamicEntities[i]->setVelocity(tempLevel.dynamicEntities[i]->getVelocityX() + 50.0f, tempLevel.dynamicEntities[i]->getVelocityY());
                        tempLevel.dynamicEntities[j]->setVelocity(tempLevel.dynamicEntities[j]->getVelocityX() - 50.0f, tempLevel.dynamicEntities[j]->getVelocityY());
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
            
            float intensity = 1.0f + flicker;
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

    for (auto& item : tempLevel.items) {
        if (item && item->isAlive() && !item->isPickedUp()) item->render(1.0f);
    }
    for (auto& trap : tempLevel.traps) {
        if (trap && trap->isAlive()) trap->render(1.0f);
    }
    for (auto& enemy : tempLevel.dynamicEntities) {
        if (enemy && enemy->isAlive()) enemy->render(1.0f);
    }

    if (player) {
        player->render(1.0f); // Light level 1.0 (fully bright) for now
    }

    // Render foreground tiles LAST so they overlap the player's head and entities
    if (tempLevel.tileMap && lighting) {
        tempLevel.tileMap->render(camera, lighting->getLightMap(), true); // Foreground pass (Solid blocks)
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
        auto drawHudElement = [&](int iconIndex, const char* text, float x, float y) {
            Rectangle src = { iconIndex * 16.0f, 0.0f, 16.0f, 16.0f };
            Rectangle dest = { x, y, iconSize, iconSize };
            DrawTexturePro(hudIcons, src, dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            
            // Text offset to align with the icon
            float textY = y + (iconSize / 2.0f) - (fontSize / 2.0f);
            DrawTextEx(game->getFont(), text, {x + iconSize + 10.0f, textY}, fontSize, 2.0f, WHITE);
        };
        
        float startX = 20.0f;
        float startY = 20.0f;
        
        // 0: Heart, 2: Rope, 3: Bomb, 4: Gold
        drawHudElement(0, TextFormat("%d", player->getHealth()), startX, startY);
        drawHudElement(3, TextFormat("%d", player->getBombs()), startX + 180.0f, startY);
        drawHudElement(2, TextFormat("%d", player->getRopes()), startX + 360.0f, startY);
        drawHudElement(4, TextFormat("%d", player->getGold()), startX + 540.0f, startY);
        
        // Floor on the right side
        float floorY = startY + (iconSize / 2.0f) - (fontSize / 2.0f);
        const char* floorText = TextFormat("FLOOR %d", GameManager::getInstance()->getFloor());
        Vector2 floorSize = MeasureTextEx(game->getFont(), floorText, fontSize, 2.0f);
        DrawTextEx(game->getFont(), floorText, {1280.0f - 20.0f - floorSize.x, floorY}, fontSize, 2.0f, WHITE);
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