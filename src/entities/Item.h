#pragma once

#include "Entity.h"

namespace Platformer {

class Player;

enum ItemType {
    TREASURE,
    HEALTH_CRATE,
    BOMB_PICKUP,
    ROPE_PICKUP
};

class Item : public Entity {
protected:
    ItemType type;
    bool isCollected;

public:
    Item(float x, float y, float w, float h, ItemType type);

    virtual void activate(Player* player);

    virtual void render(float lightLevel);

    void collect();

    bool isPickedUp();

    ItemType getType();
};

}