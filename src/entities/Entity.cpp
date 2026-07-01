#include "Entity.h"

namespace Platformer {

Entity::Entity(float x, float y, float w, float h) : x(x), y(y), width(w), height(h), isActive(true) {
    sprite.id = 0; // Initialize empty
}

Entity::~Entity() {}

void Entity::update(float dt) {
    // Virtual base implementation
}

void Entity::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { tintVal, tintVal, tintVal, 255 };
    if (sprite.id != 0) {
        DrawTextureEx(sprite, Vector2{x, y}, 0.0f, 1.0f, tint);
    } else {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height), tint);
    }
}

Rectangle Entity::getAABB() {
    return Rectangle{x, y, width, height};
}

bool Entity::isAlive() {
    return isActive;
}

void Entity::destroy() {
    isActive = false;
}

float Entity::getX() { return x; }

float Entity::getY() { return y; }

}
