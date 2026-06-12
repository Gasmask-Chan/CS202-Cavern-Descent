#pragma once

#include "Entity.h"

namespace Platformer {

enum ItemType {

};

class Item {
protected:
    ItemType type;
    bool isCollected;

public:
    Item(float x, float y, ItemType type);

    //Uncomment this after finished adding `Player.h`
    // virtual void active(Player* player);

    virtual void render(float lightLevel);

    void collect();

    bool isPickedUp();
};

}