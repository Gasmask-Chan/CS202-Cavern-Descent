#pragma once

#include "Entity.h"

namespace Platformer {

class Trap : public Entity {
protected:
    int damage;

public:
    Trap(float x, float y, float w, float h, int dmg);

    virtual void update(float dt, class Player* player = nullptr) override;

    virtual void render(float lightLevel);

    int getDamage();
};

}
