#include "Explosion.h"

namespace Platformer {

Explosion::Explosion(float x, float y)
    : DynamicEntity(x, y, 160.0f, 160.0f), currentFrame(0), frameTimer(0.0f) {
    gravity = 0.0f;
    passesThroughWalls = true;
}

void Explosion::update(float dt, Player* player) {
    if (!isActive) return;

    frameTimer += dt;
    if (frameTimer >= 0.03f) { // ~30 fps
        frameTimer = 0.0f;
        currentFrame++;
        if (currentFrame >= 10) {
            destroy();
        }
    }
}

void Explosion::render(float lightLevel) {
    if (!isActive) return;

    int row = currentFrame / 5;
    int col = currentFrame % 5;
    Rectangle src = { (float)col * 64.0f, (float)row * 64.0f, 64.0f, 64.0f };
    Rectangle dest = { x, y, width, height };
    
    Color tint = Color{(unsigned char)(255 * lightLevel), (unsigned char)(255 * lightLevel), (unsigned char)(255 * lightLevel), 255};
    DrawTexturePro(sprite, src, dest, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

} // namespace Platformer
