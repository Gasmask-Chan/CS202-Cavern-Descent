#pragma once

#include "DynamicEntity.h"

namespace Platformer {

class Particle : public DynamicEntity {
private:
    float lifetime;
    float maxLifetime;
    
public:
    Particle(float x, float y, float vx, float vy, float lifetime);
    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
};

}
