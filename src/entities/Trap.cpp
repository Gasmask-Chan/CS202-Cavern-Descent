#include "Trap.h"

namespace Platformer {

Trap::Trap(float x, float y, float w, float h, int dmg)
    : Entity(x, y, w, h), damage(dmg) {}

void Trap::update(float dt, Player* player) {
    // Base trap logic (static damage) doesn't need to do much here since collision is handled in GameState
}

void Trap::updateTrap(float dt, Player* player, const std::vector<std::unique_ptr<DynamicEntity>>& enemies, const std::vector<std::unique_ptr<Item>>& items) {
    // Call the basic update by default
    update(dt, player);
}

void Trap::render(float lightLevel) {
    Entity::render(lightLevel);
}

int Trap::getDamage() {
    return damage;
}

}
