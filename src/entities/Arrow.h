#pragma once
#include "DynamicEntity.h"

namespace Platformer {

class Arrow : public DynamicEntity {
private:
    float _angle;
    float _prevVx;
    float _prevVy;
    bool _stuck;

public:
    Arrow(float x, float y, float vx);
    
    void update(float dt, class Player* player) override;
    
    void render(float lightLevel) override;
    
    int getDamage() const { return 20; }
    bool isStuck() const { return _stuck; }
};

}
