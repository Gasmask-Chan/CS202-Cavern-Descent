#include "DynamicEntity.h"

namespace Platformer {

DynamicEntity::DynamicEntity(float x, float y, float w, float h) 
    : Entity(x, y, w, h), vx(0.0f), vy(0.0f), gravity(800.0f), isGrounded(false), isFacingRight(true), passesThroughWalls(false) {
}

void DynamicEntity::setPassesThroughWalls(bool pass) {
    passesThroughWalls = pass;
}

void DynamicEntity::applyGravity(float dt) {
    if (!isGrounded) {
        vy += gravity * dt;
        if (vy > 800.0f) vy = 800.0f; // Terminal velocity to prevent tunneling
    }
}

void DynamicEntity::move(float dx, float dy) {
    x += dx;
    y += dy;
}

void DynamicEntity::setVelocity(float vx, float vy) {
    this->vx = vx;
    this->vy = vy;
}

float DynamicEntity::getVelocityX() { return vx; }
float DynamicEntity::getVelocityY() {
    return vy;
}

bool DynamicEntity::getIsGrounded() const {
    return isGrounded;
}

}
