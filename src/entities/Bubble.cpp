#include "Bubble.h"
#include "EntityFactory.h"

namespace Platformer {

Bubble::Bubble(float x, float y, LiquidSimulator* sim)
    : DynamicEntity(x, y, 8.0f, 8.0f), liquidSim(sim), currentFrame(0), animTimer(0.0f) {
    setSprite(EntityFactory::getTexture("assets/sprites/8x8/bubble.png"), {0, 0, 8, 8});
    
    // Assign random upward velocity roughly equivalent to 0.1 to 0.3 pixels per frame (at 60 FPS = 6.0f to 18.0f)
    vy = - (GetRandomValue(60, 180) / 10.0f);
    vx = 0.0f;
    gravity = 0.0f;
    isGrounded = false;
}

void Bubble::update(float dt, Player* player) {
    if (!isActive) return;

    // Movement
    y += vy * dt;
    x += vx * dt;

    // Death by Surfacing
    if (liquidSim && !liquidSim->isWaterAt(getAABB())) {
        destroy(); 
        return;
    }

    // Animation Speed: 20% of normal framerate = 12fps (0.083s per frame)
    animTimer += dt;
    if (animTimer >= 0.083f) {
        animTimer -= 0.083f;
        currentFrame++;
        // Death by Time (Animation End)
        if (currentFrame >= totalFrames) {
            destroy(); 
            return;
        }
        setSprite(EntityFactory::getTexture("assets/sprites/8x8/bubble.png"), {(float)currentFrame * 8.0f, 0.0f, 8.0f, 8.0f});
    }
}

void Bubble::render(float lightLevel) {
    if (!isActive) return;
    Entity::render(lightLevel); // Base render uses setSprite srcRect
}

}
