#pragma once

#include "Entity.h"

namespace Platformer {

enum ItemType {

};

class Item : public Entity {
protected:
    ItemType type;
    bool isCollected;

public:
    Item(float x, float y, float w, float h, ItemType type);

    //Uncomment this after finished adding `Player.h`
    // virtual void active(Player* player);

    virtual void render(float lightLevel);

    void collect();

    bool isPickedUp();
};

}