#include "Player.h"
#include "../core/EventBus.h"
#include <algorithm>

namespace Platformer {

Player::Player(float x, float y, CharacterType type) : DynamicEntity(x, y, 32.0f, 32.0f) {
    bombs = 3;
    ropes = 3;
    gold = 0;
    invincibilityTimer = 0.0f;
    isSubmerged = false;
    
    if (type == CharacterType::EXPLORER) {
        moveStrategy = new ExplorerStrategy();
    } else if (type == CharacterType::NINJA) {
        moveStrategy = new NinjaStrategy();
    } else if (type == CharacterType::TANK) {
        moveStrategy = new TankStrategy();
    } else {
        moveStrategy = new ExplorerStrategy();
    }
    
    maxHealth = moveStrategy->getMaxHealth();
    health = maxHealth;
}

Player::~Player() {
    if (moveStrategy) {
        delete moveStrategy;
    }
}

void Player::handleInput() {
    if (IsKeyDown(KEY_A)) {
        vx = -moveStrategy->getMoveSpeed();
        isFacingRight = false;
    } else if (IsKeyDown(KEY_D)) {
        vx = moveStrategy->getMoveSpeed();
        isFacingRight = true;
    } else {
        vx = 0;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (isGrounded) {
            vy = -moveStrategy->getJumpForce();
            isGrounded = false;
        }
    }
    
    if (IsKeyReleased(KEY_SPACE)) {
        if (vy < 0) {
            vy *= 0.5f;
        }
    }
    
    if (IsKeyPressed(KEY_Z)) {
        whipAttack();
    }
    if (IsKeyPressed(KEY_X)) {
        useBomb();
    }
    if (IsKeyPressed(KEY_C)) {
        useRope();
    }
}

void Player::update(float dt) {
    if (!isGrounded) {
        vy += (gravity * moveStrategy->getGravityScale()) * dt;
    }

    float currentVx = vx;
    if (isSubmerged) {
        currentVx *= 0.5f;
    }
    
    move(currentVx * dt, vy * dt);
    
    if (invincibilityTimer > 0) {
        invincibilityTimer -= dt;
    }
}

void Player::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { tintVal, tintVal, tintVal, 255 };
    
    if (invincibilityTimer > 0.0f) {
        tint.a = 128; // Flash effect
    }
    
    if (sprite.id != 0) {
        DrawTextureEx(sprite, Vector2{x, y}, 0.0f, 1.0f, tint);
    } else {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height), tint);
    }
}

void Player::takeDamage(int dmg) {
    if (invincibilityTimer > 0.0f) return;
    
    health -= dmg;
    invincibilityTimer = 1.5f;
    
    EventData data;
    data.amount = dmg;
    EventBus::getInstance()->publish(EventType::EVENT_PLAYER_DAMAGED, data);
    
    if (health <= 0) {
        EventBus::getInstance()->publish(EventType::EVENT_PLAYER_DEATH, data);
    }
}

void Player::heal(int amount) {
    health += amount;
    if (health > maxHealth) {
        health = maxHealth;
    }
}

void Player::collectGold(int amount) {
    gold += amount;
    EventData data;
    data.amount = amount;
    EventBus::getInstance()->publish(EventType::EVENT_GOLD_COLLECTED, data);
}

bool Player::useBomb() {
    if (bombs > 0) {
        bombs--;
        // TODO: Create Bomb projectile entity 
        return true;
    }
    return false;
}

bool Player::useRope() {
    if (ropes > 0) {
        ropes--;
        // TODO: Create vertical Rope entity above player
        return true;
    }
    return false;
}

void Player::whipAttack() {
    // TODO: Calculate 1-tile wide attack rect.
    // Check TileType::CRACKED and call LevelManager to break it.
    // Check overlap with enemies in range to deal damage.
}

void Player::setMovementStrategy(MovementStrategy* s) {
    if (moveStrategy) {
        delete moveStrategy;
    }
    moveStrategy = s;
    maxHealth = moveStrategy->getMaxHealth();
    if (health > maxHealth) {
        health = maxHealth;
    }
}

int Player::getHealth() { return health; }
int Player::getBombs() { return bombs; }
int Player::getRopes() { return ropes; }
int Player::getGold() { return gold; }

}
