#pragma once

#include "../Config.h"
#include <string>
#include <vector>

namespace Platformer {

struct FloatingText {
    float x;
    float y;
    std::string text;
    Color color;
    float life;      // Current lifetime
    float maxLife;   // Maximum lifetime
    float speed;     // Upward speed
};

class ComboSystem {
private:
    int currentMultiplier;
    float comboTimer;
    float maxComboTime;
    std::vector<FloatingText> floatingTexts;

public:
    ComboSystem();
    ~ComboSystem();

    void update(float dt);
    void render(Font font);
    void renderHUD(Font font);

    // Event handlers
    void onTreasureCollected(int baseGold, float worldX, float worldY);
    void onEnemyKilled(int basePoints, float worldX, float worldY);

private:
    void spawnFloatingText(const std::string& text, float x, float y, Color color);
    void addComboPoints(int basePoints, float x, float y);
};

}
