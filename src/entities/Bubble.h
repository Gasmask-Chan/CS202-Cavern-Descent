#pragma once

#include "DynamicEntity.h"
#include "../liquid/LiquidSimulator.h"

namespace Platformer {

class Bubble : public DynamicEntity {
private:
    LiquidSimulator* liquidSim;
    int currentFrame;
    float animTimer;
    static const int totalFrames = 9;

public:
    Bubble(float x, float y, LiquidSimulator* sim);
    virtual ~Bubble() = default;

    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
};

}
