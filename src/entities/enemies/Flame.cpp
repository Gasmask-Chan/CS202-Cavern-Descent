#include "Flame.h"
#include "../../player/Player.h"
#include <cmath>

namespace Platformer {

Flame::Flame(float x, float y, float initialVy)
    : Enemy(x, y, 16.0f, 16.0f, 1, 1) { // 1 damage, 1 hp
    changeState(Enemy::idleState);
    vy = initialVy;
    vx = 0.0f;
    gravity = 800.0f;
    passesThroughWalls = false; // Need collision to destroy on ground
    setAnimation(4, 0.1f, 2, 1); // 4 frames, 0.1s speed, starting at x=32 (col 2), y=16 (row 1)
}

void Flame::update(float dt, Player* player) {
    if (!isAlive()) return;
    
    applyGravity(dt);
    move(vx * dt, vy * dt);

    // Air friction to slow down horizontal movement
    vx *= 0.95f;
    
    // Enemy::update doesn't apply gravity automatically if we override it completely, 
    // but we can call Enemy::update(dt, player) to handle default stuff, then do our logic.
    Enemy::update(dt, player);
    
    // If it hits the ground/ceiling (vy becomes 0 after physics resolution, or it's grounded)
    if (isGrounded || (vy == 0 && std::abs(gravity) > 0)) {
        destroy();
    }
}

void Flame::render(float lightLevel) {
    // Flame is a light source itself, so it ignores level lighting and always draws at full brightness
    if (sprite.id != 0) {
        Rectangle destRect = { x + width / 2.0f, y + height / 2.0f, width, height };
        Vector2 origin = { width / 2.0f, height / 2.0f };
        
        // Point the fireball in the direction of its velocity
        float rotation = 0.0f;
        if (vx != 0.0f || vy != 0.0f) {
            rotation = atan2(vy, vx) * 180.0f / PI;
        }

        DrawTexturePro(sprite, srcRect, destRect, origin, rotation, WHITE);
    }
}

void Flame::takeDamage(int amt) {
    // Immune to all damage, especially its own lava
}

}
