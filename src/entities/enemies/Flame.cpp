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
}

void Flame::update(float dt, Player* player) {
    if (!isAlive()) return;
    
    // Animate sprite? Actually, we use a single static texture or rely on default Enemy rendering.
    // For now we don't have a special sprite set, so we can just use the Enemy's sprite setup.
    // We update physics via gravity (already handled in GameState by resolving tile collisions? No, GameState calls physics->resolveEntityTileCollision, but gravity must be applied manually or in Enemy::update)
    
    // Enemy::update doesn't apply gravity automatically if we override it completely, 
    // but we can call Enemy::update(dt, player) to handle default stuff, then do our logic.
    Enemy::update(dt, player);
    
    // If it hits the ground/ceiling (vy becomes 0 after physics resolution, or it's grounded)
    if (isGrounded || (vy == 0 && std::abs(gravity) > 0)) {
        destroy();
    }
}

void Flame::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { 255, 100, 0, 255 }; // Bright orange
    
    // Draw a burning trail or box
    DrawRectangle(x, y, width, height, tint);
    
    // If sprite is loaded, also try to draw it with tint
    if (sprite.id != 0) {
        Rectangle destRect = { x, y, width, height };
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, Color{255, 255, 255, tintVal});
    }
}

}
