#include "Player.h"
#include "../core/EventBus.h"
#include <algorithm>

namespace Platformer {

Player::Player(float x, float y, CharacterType type) : DynamicEntity(x, y, 16.0f, 24.0f) {
    bombs = 3;
    ropes = 3;
    gold = 0;
    invincibilityTimer = 0.0f;
    isSubmerged = false;
    isWhipping = false;
    whipTimer = 0.0f;
    tileMap = nullptr;
    
    currentAnim = AnimState::IDLE;
    frameTimer = 0.0f;
    currentFrame = 0;
    frameRec = {0.0f, 0.0f, 80.0f, 80.0f};
    switch (type) {
        case CharacterType::NINJA:
            sprite = LoadTexture("assets/characters/ninja.png");
            break;
        case CharacterType::TANK:
            sprite = LoadTexture("assets/characters/tank.png");
            break;
        case CharacterType::EXPLORER:
        default:
            sprite = LoadTexture("assets/characters/explorer.png");
            break;
    }
    
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
    UnloadTexture(sprite);
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
    
    // Sprint
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        vx *= 1.6f;
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
    
    if (IsKeyPressed(KEY_J)) {
        whipAttack();
    }
    if (IsKeyPressed(KEY_K)) {
        useBomb();
    }
    if (IsKeyPressed(KEY_L)) {
        useRope();
    }
}

void Player::update(float dt) {
    // ALWAYS apply gravity so the player constantly pushes into the floor.
    // This ensures physics->resolveEntityTileCollision() always detects the floor
    // and keeps isGrounded = true steadily, preventing the idle animation from flickering to FALL.
    vy += (gravity * moveStrategy->getGravityScale()) * dt;

    float currentVx = vx;
    if (isSubmerged) {
        currentVx *= 0.5f;
    }
    
    move(currentVx * dt, vy * dt);
    
    if (invincibilityTimer > 0) {
        invincibilityTimer -= dt;
    }
    
    // Animation state machine
    AnimState newAnim = AnimState::IDLE;
    if (isWhipping) {
        whipTimer += dt;
        currentVx = 0; // Lock horizontal movement while whipping
        newAnim = AnimState::WHIP;
        
        // Active hitbox on frame 4 (0.15s to 0.20s)
        if (whipTimer >= 0.15f && whipTimer - dt < 0.15f) {
            float hitWidth = 24.0f;
            float hitHeight = 12.0f;
            float hitX = isFacingRight ? (x + width) : (x - hitWidth);
            float hitY = y + 4.0f; // Upper-middle part of the player
            Rectangle whipHitbox = { hitX, hitY, hitWidth, hitHeight };
            
            if (tileMap) {
                int startTx = (int)(whipHitbox.x / 32.0f);
                int startTy = (int)(whipHitbox.y / 32.0f);
                int endTx = (int)((whipHitbox.x + whipHitbox.width - 0.001f) / 32.0f);
                int endTy = (int)((whipHitbox.y + whipHitbox.height - 0.001f) / 32.0f);
                
                for (int ty = startTy; ty <= endTy; ty++) {
                    for (int tx = startTx; tx <= endTx; tx++) {
                        if (tileMap->isCracked(tx, ty)) {
                            // TODO: Replace with LevelManager->breakCrackedBlock() once Person B implements LevelManager.
                            // This is currently bypassing the SFX, particle events, and potential item drops!
                            tileMap->destroyBlock(tx, ty);
                        }
                    }
                }
            }
        }
    } else if (!isGrounded) {
        if (vy < 0) newAnim = AnimState::JUMP;
        else newAnim = AnimState::FALL;
    } else {
        if (IsKeyDown(KEY_W)) {
            newAnim = AnimState::LOOK_UP;
        } else if (vx > 0.1f || vx < -0.1f) {
            newAnim = AnimState::RUN;
        } else if (IsKeyDown(KEY_S)) {
            if (vx > 0.1f || vx < -0.1f) newAnim = AnimState::CRAWL;
            else newAnim = AnimState::DUCK;
        } else if (currentAnim == AnimState::LOOK_UP || currentAnim == AnimState::LOOK_UP_END) {
            newAnim = AnimState::LOOK_UP_END;
        } else {
            newAnim = AnimState::IDLE;
        }
    }
    
    if (newAnim != currentAnim) {
        currentAnim = newAnim;
        currentFrame = 0;
        frameTimer = 0.0f;
    }
    
    frameTimer += dt;
    float frameDuration = 0.1f;
    
    int maxFrames = 1;
    int row = 0;
    int colOffset = 0;
    
    switch (currentAnim) {
        case AnimState::IDLE:
            maxFrames = 1;
            row = 0;
            colOffset = 0;
            break;
        case AnimState::RUN:
            maxFrames = 8;
            row = 0;
            colOffset = 1;
            frameDuration = 0.05f; // Fast running animation
            break;
        case AnimState::JUMP:
            maxFrames = 1;
            row = 9;
            colOffset = 0;
            break;
        case AnimState::FALL:
            maxFrames = 1;
            row = 9;
            colOffset = 1;
            break;
        case AnimState::LOOK_UP:
            maxFrames = 4;
            row = 8;
            colOffset = 0;
            frameDuration = 0.05f; // Fast running animation
            break;
        case AnimState::LOOK_UP_END:
            maxFrames = 4;
            row = 8;
            colOffset = 3;
            frameDuration = 0.05f; // Fast running animation
            break;
        case AnimState::DUCK:
            maxFrames = 1;
            row = 1;
            colOffset = 0;
            break;
        case AnimState::CRAWL:
            maxFrames = 6;
            row = 1;
            colOffset = 1;
            frameDuration = 0.08f;
            break;
        case AnimState::WHIP:
            maxFrames = 6;
            row = 4;
            colOffset = 0;
            frameDuration = 0.05f;
            break;
    }
    
    if (frameTimer >= frameDuration) {
        frameTimer -= frameDuration; // Prevent dt leak!
        if (currentAnim == AnimState::LOOK_UP && currentFrame == maxFrames - 1) {
            // Stay on the last frame while holding up
        } else if (currentAnim == AnimState::LOOK_UP_END && currentFrame == maxFrames - 1) {
            // Finished stopping look up, return to idle
            currentAnim = AnimState::IDLE;
            currentFrame = 0;
            row = 0;
            colOffset = 0;
        } else if (currentAnim == AnimState::WHIP && currentFrame == maxFrames - 1) {
            // Finished whip attack, return to idle
            isWhipping = false;
            currentAnim = AnimState::IDLE;
            currentFrame = 0;
            row = 0;
            colOffset = 0;
        } else {
            currentFrame = (currentFrame + 1) % maxFrames;
        }
    }
    
    frameRec.x = (colOffset + currentFrame) * 80.0f;
    frameRec.y = row * 80.0f;
    frameRec.width = isFacingRight ? 80.0f : -80.0f;
}

void Player::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { tintVal, tintVal, tintVal, 255 };
    
    if (invincibilityTimer > 0.0f) {
        tint.a = 128; // Flash effect
    }
    
    if (sprite.id != 0) {
        // Sprite is originally 80x80. Scaled to 40x40. AABB is now 16x24.
        // X offset: (40 - 16) / 2 = 12px to horizontally center the 40px sprite over the 16px AABB.
        // Y offset: The 40x40 sprite has 5px of transparent padding at the bottom.
        // To make the visible feet (at destRec.y + 35) touch the floor (at y + 24), destRec.y must be y - 11.0f.
        Rectangle destRec = { x - 12.0f, y - 11.0f, 40.0f, 40.0f };
        DrawTexturePro(sprite, frameRec, destRec, Vector2{0.0f, 0.0f}, 0.0f, tint);
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
    if (!isWhipping) {
        isWhipping = true;
        whipTimer = 0.0f;
    }
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
