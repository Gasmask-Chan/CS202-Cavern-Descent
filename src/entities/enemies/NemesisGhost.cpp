#include "NemesisGhost.h"
#include "../../player/Player.h"
#include <cmath>

namespace Platformer {

NemesisGhost::NemesisGhost(float x, float y, float w, float h)
    : Enemy(x, y, w, h, 999, 1) { 
    changeState(Enemy::chaseState);
    gravity = 0; 
    passesThroughWalls = true; // Set flag to true so PhysicsSystem ignores it
    setAnimation(16, 0.1f, 0, 0); // 16 frames custom sequence
}

NemesisGhost::~NemesisGhost() {}

void NemesisGhost::update(float dt, Player* player) {
    Enemy::update(dt, player);
    move(vx * dt, vy * dt);
}

void NemesisGhost::updateSpriteRect() {
    // Override srcRect for Nemesis ghost custom multi-row animation
    static const int frames[16][2] = {
        {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5},
        {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5},
        {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}
    };
    
    if (sprite.id != 0 && currentFrame < 16) {
        float absW = 160.0f;
        float absH = 160.0f;
        int row = frames[currentFrame][0];
        int col = frames[currentFrame][1];
        
        srcRect.x = col * absW;
        srcRect.y = row * absH;
        
        float drawWidth = absW;
        // Normalize frames so the entire animation naturally faces RIGHT
        if (row == 2 && col >= 1 && col <= 4) { 
            drawWidth = -drawWidth; 
        }
        
        // Apply global facing direction
        if (!isFacingRight) {
            drawWidth = -drawWidth;
        }
        
        srcRect.width = drawWidth;
    }
}

void NemesisGhost::handleIdle(float dt, Player* player) {
    // Nemesis ghost doesn't idle
    changeState(Enemy::chaseState);
}

void NemesisGhost::handleChase(float dt, Player* player) {
    if (!player) return;
    
    float dx = player->getX() - x;
    float dy = player->getY() - y;
    float dist = std::sqrt(dx*dx + dy*dy);
    
    if (dist > 0.0f) {
        float speed = 60.0f; // Relentless pursuit
        vx = (dx / dist) * speed;
        vy = (dy / dist) * speed;
        isFacingRight = (vx > 0);
    }
}

void NemesisGhost::handleReturn(float dt, Player* player) {
    // Nemesis ghost doesn't return
    changeState(Enemy::chaseState);
}

void NemesisGhost::render(float lightLevel) {
    if (sprite.id != 0 && isActive) {
        // Render with lower opacity to act like a transparent shadow
        Color tint = { 
            static_cast<unsigned char>(255 * lightLevel), 
            static_cast<unsigned char>(255 * lightLevel), 
            static_cast<unsigned char>(255 * lightLevel), 
            160 // Semi-transparent opacity (0-255)
        };
        Rectangle dest = { x, y, width, height };
        DrawTexturePro(sprite, srcRect, dest, Vector2{0, 0}, 0.0f, tint);
    }
}

}
