#pragma once

#include "../DynamicEntity.h"

namespace Platformer {

class TileMap;

class RopeProjectile : public DynamicEntity {
private:
    float startY;
    bool isUnfurling;
    float unfurlTimer;
    int currentLength;
    int maxLength;
    TileMap* tileMap;

public:
    RopeProjectile(float x, float y, float vy, TileMap* map);
    
    void update(float dt, Player* player) override;
    void render(float lightLevel) override;
};

}
