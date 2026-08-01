#include "Entity.h"

namespace Platformer {

Entity::Entity(float x, float y, float w, float h) : x(x), y(y), width(w), height(h), isActive(true) {
    sprite.id = 0; // Initialize empty
}

Entity::~Entity() {}

void Entity::update(float dt, Player* player) {
    // Base implementation is empty. Subclasses override to add per-frame logic.
}

void Entity::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { tintVal, tintVal, tintVal, 255 };
    if (sprite.id != 0) {
        if (srcRect.width == 0) {
            DrawTextureEx(sprite, Vector2{x + renderOffsetX, y + renderOffsetY}, 0.0f, 1.0f, tint);
        } else {
            Rectangle destRect = { x + renderOffsetX, y + renderOffsetY, width, height };
            DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
        }
    }
}

void Entity::setSprite(Texture2D tex, Rectangle src) {
    sprite = tex;
    srcRect = src;
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
