#pragma once

#include "../Config.h"
#include "../entities/DynamicEntity.h"
#include "MovementStrategy.h"
#include "../core/GameState.h" 
#include "../level/TileMap.h"
#include "../liquid/LiquidSimulator.h"

namespace Platformer {

class Player : public DynamicEntity {
private:
    int health;
    int maxHealth;
    int bombs;
    int ropes;
    int gold;
    std::unique_ptr<MovementStrategy> moveStrategy;
    float invincibilityTimer;
    bool isSubmerged;
    bool isSwimming;
    bool isDiving;
    bool wasSubmerged;
    bool isWhipping;
    float whipTimer;
    float stepTimer;
    float climbTimer;
    bool whipHitThisFrame;
    bool isClimbing;
    bool isGodMode;
    int cheatSequence;
    TileMap* tileMap;
    LiquidSimulator* liquidSim;
    Texture2D whipSprite;
    
    int bubbleTimer;

    // Animation state
    enum class AnimState { IDLE, RUN, JUMP, FALL, LOOK_UP, LOOK_UP_END, DUCK, CRAWL, WHIP, CLIMB, SWIM, DEAD } currentAnim;
    float frameTimer;
    int currentFrame;
    Rectangle frameRec;

public:
    Player(float x, float y, CharacterType type);
    virtual ~Player();

    bool isAlive() override;

    void handleInput();
    void update(float dt, class Player* player = nullptr) override;
    void render(float lightLevel) override;
    
    void takeDamage(int dmg);
    void heal(int amount);
    void collectGold(int amount);
    void addBomb(int amount);
    void addRope(int amount);
    bool useBomb();
    bool useRope();
    
    void onHitGround(float impactVelocity) override;
    bool isInvincible() const { return invincibilityTimer > 0.0f; }
    void whipAttack();
    void setMovementStrategy(std::unique_ptr<MovementStrategy> s);

    /**
     * @brief Sets the tile map reference for the player, allowing interaction with the environment (e.g., breaking blocks).
     * 
     * @param map Pointer to the active TileMap.
     */
    void setTileMap(TileMap* map) { tileMap = map; }
    
    /**
     * @brief Sets the liquid simulator reference for the player, allowing interaction with water (swimming).
     * 
     * @param sim Pointer to the active LiquidSimulator.
     */
    void setLiquidSimulator(LiquidSimulator* sim) { liquidSim = sim; }
    
    int getHealth();
    int getBombs();
    int getRopes();
    int getGold();
    void spendGold(int amount);
    
    bool getIsWhipHitThisFrame() const;
    Rectangle getWhipHitbox() const;
    bool getIsClimbing() const { return isClimbing; }
};

}
