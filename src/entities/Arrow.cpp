#include "Arrow.h"
#include <cmath>

namespace Platformer {

Arrow::Arrow(float x, float y, float vx) 
    : DynamicEntity(x, y, 16.0f, 16.0f), _stuck(false) {
    this->vx = vx;
    this->vy = 0;
    this->_prevVx = vx;
    this->_prevVy = 0;
    
    // 90 is right, 270 is left
    _angle = (vx > 0) ? 90.0f : 270.0f;
    isFacingRight = (vx > 0);
}

void Arrow::update(float dt, Player* player) {
    if (_stuck) {
        return;
    }

    // Apply gravity
    applyGravity(dt);
    
    // Check for collisions by seeing if velocity was zeroed by PhysicsSystem in the last frame
    if (vx == 0 && _prevVx != 0) {
        // Hit a wall
        vx = -_prevVx * 0.4f; // bounce and lose speed
        _angle += 180.0f; // reverse angle roughly
    }
    
    if (vy == 0 && _prevVy > 0) {
        // Hit the floor
        if (std::abs(vx) > 20.0f) {
            vy = -_prevVy * 0.3f; // bounce up
            vx *= 0.6f; // friction
        } else {
            // Come to rest
            vx = 0;
            vy = 0;
            _stuck = true;
            _angle = isFacingRight ? 90.0f : 270.0f;
        }
    }
    
    if (!_stuck) {
        // User math: _angle += 8 * (1 / _x_speed)
        // Guard against div by zero
        if (std::abs(vx) > 0.1f) {
            float angleDelta = 8.0f * (100.0f / vx); 
            // The original 1/x_speed might have used small speed values like 4.0. We use 400.0, so scaling is needed.
            // If vx = 400, angleDelta = 8 * 0.25 = 2 degrees per frame.
            _angle += angleDelta;
        }
        
        // Normalize angle 0-360
        while (_angle < 0) _angle += 360.0f;
        while (_angle >= 360.0f) _angle -= 360.0f;
    }

    move(vx * dt, vy * dt);
    
    _prevVx = vx;
    _prevVy = vy;
}

void Arrow::render(float lightLevel) {
    if (sprite.id == 0) return;
    
    // 16 frame spritesheet mapping (0-15)
    // _angle is 0-360. Each frame is 22.5 degrees.
    int frame_num = (int)std::floor(_angle / 22.5f);
    if (frame_num < 0) frame_num = 0;
    if (frame_num > 15) frame_num = 15;
    
    // 8x8 frames. Texture is 48x32 (6 cols x 4 rows)
    int col = frame_num % 6;
    int row = frame_num / 6;
    
    srcRect = { col * 8.0f, row * 8.0f, 8.0f, 8.0f };
    
    // Render
    Color tint = { 
        (unsigned char)(255 * lightLevel), 
        (unsigned char)(255 * lightLevel), 
        (unsigned char)(255 * lightLevel), 
        255 
    };
    
    Rectangle destRect = { x, y, width, height };
    DrawTexturePro(sprite, srcRect, destRect, {0, 0}, 0.0f, tint);
}

}
