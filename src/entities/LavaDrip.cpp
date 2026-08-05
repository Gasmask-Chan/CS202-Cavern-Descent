#include "LavaDrip.h"
#include <iostream>
#include "EntityFactory.h"
#include "../audio/AudioManager.h"

namespace Platformer {

LavaDrip::LavaDrip(float x, float y) : DynamicEntity(x, y, 8.0f, 8.0f) {
    texture = EntityFactory::getTexture("assets/sprites/lava/LavaDrip.png");
    currentFrame = 0;
    frameTime = 0.0f;
    totalTime = 0.0f;
    gravity = 600.0f;
    passesThroughWalls = false;
}

LavaDrip::~LavaDrip() {
    // Texture is managed by EntityFactory, do not unload here
}

void LavaDrip::update(float dt, Player* player) {
    if (!isAlive()) return;
    
    totalTime += dt;
    if (totalTime > 5.0f) { // Failsafe
        destroy();
        return;
    }

    if (isGrounded) {
        AudioManager::getInstance()->playSFX("xsmallexplode");
        destroy();
        return;
    }

    applyGravity(dt);
    move(0, vy * dt); // Only vertical movement needed

    // Animate
    frameTime += dt;
    if (frameTime >= 0.1f) {
        frameTime = 0.0f;
        currentFrame = (currentFrame + 1) % 5;
    }
}

void LavaDrip::render(float lightLevel) {
    if (!isAlive()) return;
    
    Rectangle source = { (float)currentFrame * 8.0f, 0.0f, 8.0f, 8.0f };
    Rectangle dest = { x, y, 8.0f, 8.0f };
    Vector2 origin = { 0.0f, 0.0f };
    
    Color tint = WHITE;
    tint.r = (unsigned char)(255 * lightLevel);
    tint.g = (unsigned char)(255 * lightLevel);
    tint.b = (unsigned char)(255 * lightLevel);
    
    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

} // namespace Platformer
