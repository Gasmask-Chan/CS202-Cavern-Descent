#pragma once
#include "DynamicEntity.h"

namespace Platformer {

class LavaDrip : public DynamicEntity {
public:
    LavaDrip(float x, float y);
    virtual ~LavaDrip();

    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;

private:
    Texture2D texture;
    int currentFrame;
    float frameTime;
    float totalTime;
};

} // namespace Platformer
