#include "Particle.h"

namespace Platformer {

Particle::Particle(float x, float y, float vx, float vy, float lifetime)
    : DynamicEntity(x, y, 4.0f, 4.0f), lifetime(lifetime), maxLifetime(lifetime) {
    this->vx = vx;
    this->vy = vy;
    this->passesThroughWalls = false;
    this->gravity = 600.0f; // Slightly lower gravity for particles
    this->renderOffsetX = -4.0f;
    this->renderOffsetY = -8.0f;
}

void Particle::update(float dt, Player* player) {
    if (isGrounded) {
        vx = 0.0f;
        vy = 0.0f;
    } else {
        applyGravity(dt);
    }
    
    move(vx * dt, vy * dt);
    
    lifetime -= dt;
    if (lifetime <= 0.0f) {
        destroy();
    }
}

void Particle::render(float lightLevel) {
    if (!isAlive()) return;
    
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    unsigned char alpha = 255;
    
    // Fade out during the last half of its lifetime
    if (lifetime < maxLifetime / 2.0f) {
        alpha = static_cast<unsigned char>((lifetime / (maxLifetime / 2.0f)) * 255.0f);
    }
    
    Color tint = { tintVal, tintVal, tintVal, alpha };
    
    if (sprite.id != 0) {
        Rectangle destRect = { x + renderOffsetX, y + renderOffsetY, 12.0f, 12.0f };
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
    }
}

}
