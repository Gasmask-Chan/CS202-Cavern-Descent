#pragma once

#include "../DynamicEntity.h"

namespace Platformer {

class Explosion : public DynamicEntity {
private:
    int currentFrame;
    float frameTimer;

public:
    Explosion(float x, float y);
    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
};

} // namespace Platformer
