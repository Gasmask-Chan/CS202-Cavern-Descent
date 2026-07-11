#include "Bomb.h"
#include "../core/EventBus.h"
#include <cmath>

namespace Platformer {

Bomb::Bomb(float x, float y, float vx, float vy)
    : DynamicEntity(x, y, 16.0f, 16.0f), fuseTimer(2.5f), prevVy(0.0f), prevVx(0.0f) {
    this->vx = vx;
    this->vy = vy;
    gravity = 600.0f; // slightly lighter gravity for floaty throw
}

Bomb::~Bomb() {}

void Bomb::update(float dt, Player* player) {
    fuseTimer -= dt;
    if (fuseTimer <= 0) {
        // Explode!
        EventData data;
        data.worldX = x + width / 2.0f;
        data.worldY = y + height / 2.0f;
        EventBus::getInstance()->publish(EventType::EVENT_BOMB_EXPLODE, data);
        destroy();
        return;
    }

    prevVx = vx;
    prevVy = vy;

    applyGravity(dt);
    
    // Apply horizontal friction when grounded
    if (isGrounded) {
        vx *= 0.8f;
        if (std::abs(vx) < 5.0f) vx = 0;
    }

    move(vx * dt, vy * dt);
    
    // Animate sprite based on fuse? (For now, use basic update)
    DynamicEntity::update(dt, player);
}

void Bomb::render(float lightLevel) {
    // Draw a blinking bomb if close to exploding!
    Color color = (std::fmod(fuseTimer, 0.2f) < 0.1f) ? RED : BLACK;
    color = ColorTint(color, Color{ (unsigned char)(255 * lightLevel), (unsigned char)(255 * lightLevel), (unsigned char)(255 * lightLevel), 255 });
    DrawCircleV(Vector2{x + width/2.0f, y + height/2.0f}, width/2.0f, color);
}

}
