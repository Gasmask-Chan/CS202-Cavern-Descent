#pragma once

#include "Entity.h"

namespace Platformer {

class Trap {
protected:
    int damage;

public:
    Trap(float x, float y, int dmg);

    virtual void update(float dt);

    virtual void render(float lightLevel);

    int getDamage();
};

}
