#pragma once

#include "../DynamicEntity.h"

#include "EnemyState.h"

namespace Platformer {

class Enemy : public DynamicEntity {
protected:
    int health;
    int damage;
    std::shared_ptr<EnemyState> currentStateObj;
    class TileMap* tileMap = nullptr;

    float animTimer;
    float animSpeed;
    int currentFrame;
    int numFrames;
    int baseFrameX;
    int baseFrameY;

    virtual void updateSpriteRect();

public:
    Enemy(float x, float y, float w, float h, int hp = 1, int dmg = 1);
    virtual ~Enemy();

    void setAnimation(int frames, float speed, int baseX, int baseY);
    void setTileMap(class TileMap* map) { tileMap = map; }

    virtual void update(float dt, class Player* player = nullptr) override;
    virtual void render(float lightLevel) override;

    int getHealth() const;
    int getDamage() const;
    std::shared_ptr<EnemyState> getState() const;
    
    virtual void takeDamage(int dmg);

    // State Pattern core methods
    void changeState(std::shared_ptr<EnemyState> newState);

    // Virtual behavior hooks to be implemented by subclasses
    virtual void handleIdle(float dt, class Player* player) {}
    virtual void handleChase(float dt, class Player* player) {}
    virtual void handleReturn(float dt, class Player* player) {}

    // Global state instances to avoid reallocation overhead
    static std::shared_ptr<EnemyState> idleState;
    static std::shared_ptr<EnemyState> chaseState;
    static std::shared_ptr<EnemyState> returnState;
};

}
