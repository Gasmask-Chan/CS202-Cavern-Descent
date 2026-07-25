#pragma once
#include "Entity.h"

namespace Platformer {

class Lamp : public Entity {
private:
    float pulseTimer;
public:
    Lamp(float x, float y);
    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
};

}
