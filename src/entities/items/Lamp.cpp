#include "Lamp.h"
#include "../EntityFactory.h"
#include <cmath>

namespace Platformer {

Lamp::Lamp(float x, float y) : Entity(x, y, 32.0f, 32.0f), pulseTimer(0.0f) {
    // 32x32 size for rendering in world
    // Texture is 16x16, frames 25, 26, 27
    setSprite(EntityFactory::getTexture("assets/sprites/16x16/gfx_shopkeeper.png"), Rectangle{16.0f, 64.0f, 16.0f, 16.0f});
}

void Lamp::update(float dt, Player* player) {
    pulseTimer += dt;
    if (pulseTimer >= 3.0f) { // Cycle repeats every 3 seconds (each frame 1s)
        pulseTimer -= 3.0f;
    }
    
    // Frames 25, 26, 27 on a 6-column sprite sheet:
    // Frame 25: col 1, row 4 -> x = 1*16, y = 4*16
    // Frame 26: col 2, row 4 -> x = 2*16, y = 4*16
    // Frame 27: col 3, row 4 -> x = 3*16, y = 4*16
    
    int frameIndex = 0;
    if (pulseTimer < 1.0f) {
        frameIndex = 25; // x=16, y=64
    } else if (pulseTimer < 2.0f) {
        frameIndex = 26; // x=32, y=64
    } else {
        frameIndex = 27; // x=48, y=64
    }
    
    int cols = 6;
    float srcX = (frameIndex % cols) * 16.0f;
    float srcY = (frameIndex / cols) * 16.0f;
    
    srcRect = { srcX, srcY, 16.0f, 16.0f };
}

void Lamp::render(float lightLevel) {
    Entity::render(lightLevel);
}

}
