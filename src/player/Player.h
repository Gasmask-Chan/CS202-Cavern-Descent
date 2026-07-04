#pragma once

#include "../Config.h"
#include "../entities/DynamicEntity.h"
#include "MovementStrategy.h"
#include "../core/GameState.h" 

namespace Platformer {

class Player : public DynamicEntity {
private:
    int health;
    int maxHealth;
    int bombs;
    int ropes;
    int gold;
    MovementStrategy* moveStrategy;
    float invincibilityTimer;
    bool isSubmerged;

    // Animation state
    enum class AnimState { IDLE, RUN, JUMP, FALL, LOOK_UP, LOOK_UP_END, DUCK, CRAWL } currentAnim;
    float frameTimer;
    int currentFrame;
    Rectangle frameRec;

public:
    Player(float x, float y, CharacterType type);
    virtual ~Player();

    void handleInput();
    void update(float dt) override;
    void render(float lightLevel) override;
    
    void takeDamage(int dmg);
    void heal(int amount);
    void collectGold(int amount);
    bool useBomb();
    bool useRope();
    void whipAttack();
    void setMovementStrategy(MovementStrategy* s);
    
    int getHealth();
    int getBombs();
    int getRopes();
    int getGold();
};

}
