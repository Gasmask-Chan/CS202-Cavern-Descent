#include "Bomb.h"
#include "../core/EventBus.h"
#include "EntityFactory.h"
#include <cmath>

namespace Platformer {

Bomb::Bomb(float x, float y, float vx, float vy)
    : DynamicEntity(x, y, 16.0f, 16.0f), fuseTimer(2.5f), prevVy(0.0f), prevVx(0.0f) {
    this->vx = vx;
    this->vy = vy;
    gravity = 600.0f; // slightly lighter gravity for floaty throw
    
    sprite = EntityFactory::getTexture("assets/sprites/8x8/gfx_bomb.png");
    
    srcRect = {0.0f, 0.0f, 8.0f, 8.0f};
}

Bomb::~Bomb() {
    // Texture is managed by EntityFactory cache, do not unload here
}

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
    // Use the second frame (ticking bomb with red color) if close to exploding
    if (fuseTimer < 1.0f && std::fmod(fuseTimer, 0.2f) < 0.1f) {
        srcRect.x = 8.0f; // Ticking frame
    } else {
        srcRect.x = 0.0f; // Normal frame
    }
    
    unsigned char lv = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = Color{ lv, lv, lv, 255 };
    
    Rectangle destRec = { x, y, width, height };
    DrawTexturePro(sprite, srcRect, destRec, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

}
