#include "Item.h"

namespace Platformer {

Item::Item(float x, float y, float w, float h, ItemType type)
    : Entity(x, y, w, h), type(type), isCollected(false) {}

void Item::activate(Player* player) {
    // Default implementation does nothing
}

void Item::render(float lightLevel) {
    Entity::render(lightLevel);
}

void Item::collect() {
    isCollected = true;
    destroy();
}

bool Item::isPickedUp() {
    return isCollected;
}

ItemType Item::getType() {
    return type;
}

}
