#include "Trap.h"

namespace Platformer {

Trap::Trap(float x, float y, float w, float h, int dmg)
    : Entity(x, y, w, h), damage(dmg) {}

void Trap::update(float dt) {
    Entity::update(dt);
}

void Trap::render(float lightLevel) {
    Entity::render(lightLevel);
}

int Trap::getDamage() {
    return damage;
}

}
