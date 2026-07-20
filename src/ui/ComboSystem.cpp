#include "ComboSystem.h"
#include "../core/GameManager.h"
#include "../core/EventBus.h"

namespace Platformer {

ComboSystem::ComboSystem() : currentMultiplier(1), comboTimer(0.0f), maxComboTime(3.0f) {
}

ComboSystem::~ComboSystem() {
}

void ComboSystem::update(float dt) {
    if (comboTimer > 0.0f) {
        comboTimer -= dt;
        if (comboTimer <= 0.0f) {
            comboTimer = 0.0f;
            currentMultiplier = 1;
        }
    }

    for (auto it = floatingTexts.begin(); it != floatingTexts.end(); ) {
        it->y -= it->speed * dt;
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = floatingTexts.erase(it);
        } else {
            ++it;
        }
    }
}

void ComboSystem::render(Font font) {
    for (const auto& ft : floatingTexts) {
        float alpha = ft.life / ft.maxLife;
        Color c = ft.color;
        c.a = (unsigned char)(255 * alpha);
        
        // Draw in world space using DrawTextEx to scale properly and use the game font
        // Using fontSize 13.0f
        DrawTextEx(font, ft.text.c_str(), Vector2{ft.x, ft.y}, 13.0f, 1.0f, c);
    }
}

void ComboSystem::renderHUD(Font font) {
    if (currentMultiplier > 1) {
        int screenHeight = GetScreenHeight();
        const char* text = TextFormat("COMBO x%d", currentMultiplier);
        Vector2 textSize = MeasureTextEx(font, text, 30.0f, 2.0f);
        DrawTextEx(font, text, Vector2{ (float)GetScreenWidth() / 2.0f - textSize.x / 2.0f, (float)screenHeight - 80.0f }, 30.0f, 2.0f, GOLD);
        
        // Draw combo timer bar
        float barWidth = 200.0f;
        float barHeight = 10.0f;
        float currentWidth = (comboTimer / maxComboTime) * barWidth;
        DrawRectangle(GetScreenWidth() / 2 - 100, screenHeight - 40, (int)barWidth, (int)barHeight, DARKGRAY);
        DrawRectangle(GetScreenWidth() / 2 - 100, screenHeight - 40, (int)currentWidth, (int)barHeight, ORANGE);
    }
}

void ComboSystem::spawnFloatingText(const std::string& text, float x, float y, Color color) {
    // Add a small random offset so multiple text items spawned at the same exact time/location don't perfectly overlap
    float offsetX = (float)GetRandomValue(-10, 10);
    float offsetY = (float)GetRandomValue(-10, 10);
    floatingTexts.push_back({x + offsetX, y + offsetY, text, color, 1.5f, 1.5f, 50.0f});
}

void ComboSystem::addComboPoints(int basePoints, float x, float y) {
    int totalPoints = basePoints * currentMultiplier;
    GameManager::getInstance()->addScore(totalPoints);
    
    std::string text = "+" + std::to_string(totalPoints);
    if (currentMultiplier > 1) {
        text += " x" + std::to_string(currentMultiplier) + "!";
    }
    
    spawnFloatingText(text, x, y, GOLD);
    
    comboTimer = maxComboTime;
    currentMultiplier++;
}

void ComboSystem::onTreasureCollected(int baseGold, float worldX, float worldY) {
    addComboPoints(baseGold, worldX, worldY);
}

void ComboSystem::onEnemyKilled(int basePoints, float worldX, float worldY) {
    addComboPoints(basePoints, worldX, worldY);
}

}
