#pragma once

#include "../Entity.h"
#include <vector>
#include <memory>

namespace Platformer {

class Trap : public Entity {
protected:
    int damage;

public:
    Trap(float x, float y, float w, float h, int dmg);

    virtual void update(float dt, class Player* player = nullptr) override;

    // Overloaded update for traps that need world context (like ArrowTrap line of sight)
    virtual void updateTrap(float dt, class Player* player, const std::vector<std::unique_ptr<class DynamicEntity>>& enemies, const std::vector<std::unique_ptr<class Item>>& items, class TileMap* tileMap = nullptr);

    virtual void render(float lightLevel);

    int getDamage();
};

}
