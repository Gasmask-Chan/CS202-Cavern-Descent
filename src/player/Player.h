#pragma once

#include "../Config.h"
#include "../entities/DynamicEntity.h"
#include "MovementStrategy.h"
#include "../core/GameState.h" 
#include "../level/TileMap.h"

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

    bool isWhipping;
    float whipTimer;
    TileMap* tileMap;
    Texture2D whipSprite;

    // Animation state
    enum class AnimState { IDLE, RUN, JUMP, FALL, LOOK_UP, LOOK_UP_END, DUCK, CRAWL, WHIP } currentAnim;
    float frameTimer;
    int currentFrame;
    Rectangle frameRec;

public:
    Player(float x, float y, CharacterType type);
    virtual ~Player();

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
    void whipAttack();
    void setMovementStrategy(MovementStrategy* s);

    /**
     * @brief Sets the tile map reference for the player, allowing interaction with the environment (e.g., breaking blocks).
     * 
     * @param map Pointer to the active TileMap.
     */
    void setTileMap(TileMap* map) { tileMap = map; }
    
    int getHealth();
    int getBombs();
    int getRopes();
    int getGold();
};

}
