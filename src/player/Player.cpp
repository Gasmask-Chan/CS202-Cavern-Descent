#include "Player.h"
#include "../entities/Item.h"
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
    whipHitThisFrame = false;
    tileMap = nullptr;
    liquidSim = nullptr;
    bubbleTimer = 0;
    
    Image whipImg = LoadImage("assets/sprites/16x16/gfx_spike_collectibles_flame.png");
    ImageFormat(&whipImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // Ensure alpha channel exists
    Color bg = GetImageColor(whipImg, 0, 0);
    ImageColorReplace(&whipImg, bg, BLANK);
    whipSprite = LoadTextureFromImage(whipImg);
    UnloadImage(whipImg);
    
    currentAnim = AnimState::IDLE;
    frameTimer = 0.0f;
    currentFrame = 0;
    isClimbing = false;
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
    UnloadTexture(whipSprite);
    if (moveStrategy) {
        delete moveStrategy;
    }
}

void Player::handleInput() {
    if (isWhipping || !moveStrategy) return;

    bool onLadder = false;
    if (tileMap) {
        int tx = static_cast<int>((x + width / 2) / tileMap->getTileSize());
        int ty = static_cast<int>((y + height / 2) / tileMap->getTileSize());
        int feetTy = static_cast<int>((y + height + 2.0f) / tileMap->getTileSize());
        
        onLadder = tileMap->isLadder(tx, ty);
        
        // If we are standing ON a ladder deck, pressing down should let us climb
        if (!onLadder && tileMap->isLadder(tx, feetTy) && IsKeyDown(KEY_S)) {
            onLadder = true;
        }
    }

    if (onLadder && (IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) && !isClimbing) {
        isClimbing = true;
        // Snap to center horizontally
        x = static_cast<int>((x + width / 2) / tileMap->getTileSize()) * tileMap->getTileSize() + tileMap->getTileSize() / 2.0f - width / 2.0f;
        vx = 0.0f;
        vy = 0.0f;
    }

    if (isClimbing) {
        vx = 0.0f;
        
        // Cấp gia tốc dọc
        if (IsKeyDown(KEY_W)) {
            vy = -moveStrategy->getMoveSpeed() * 0.5f;
        } else if (IsKeyDown(KEY_S)) {
            vy = moveStrategy->getMoveSpeed() * 0.5f;
        } else {
            // Đứng yên trên thang (treo lơ lửng)
            vy = 0.0f;
        }

        if (IsKeyPressed(KEY_SPACE)) {
            isClimbing = false;
            vy = -moveStrategy->getJumpForce() * 0.8f; // slightly weaker jump off ladder
            isGrounded = false;
        } else {
            return; // Skip normal walking input
        }
    }

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
    
    if (isSwimming) {
        if (IsKeyDown(KEY_W)) {
            vy -= 600.0f * GetFrameTime();
        }
        if (IsKeyDown(KEY_S)) {
            vy += 600.0f * GetFrameTime(); // Diving
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

void Player::update(float dt, Player* player) {
    if (!moveStrategy) return;
    // Check water submersion
    if (liquidSim) {
        isSubmerged = liquidSim->isWaterAt(getAABB());
        isSwimming = isSubmerged;
        isDiving = isSubmerged && vy > 0;
    }

    if (isClimbing) {
        bool onLadder = false;
        if (tileMap) {
            int tx = static_cast<int>((x + width / 2) / tileMap->getTileSize());
            int ty = static_cast<int>((y + height / 2) / tileMap->getTileSize());
            onLadder = tileMap->isLadder(tx, ty);
        }
        if (!onLadder) {
            isClimbing = false;
            gravity = 800.0f;
        } else {
            gravity = 0.0f; // Disable gravity while climbing
        }
    } else if (isSwimming) {
        gravity = 0.0f; // Disable gravity while swimming
    } else {
        gravity = 800.0f;
    }
    // Prevent physics instability/sinking during lag spikes or when game is backgrounded
    if (dt > 0.033f) dt = 0.033f; 

    // ALWAYS apply gravity so the player constantly pushes into the floor.
    // This ensures physics->resolveEntityTileCollision() always detects the floor
    // and keeps isGrounded = true steadily, preventing the idle animation from flickering to FALL.
    vy += (gravity * moveStrategy->getGravityScale()) * dt;

    float currentVx = vx;
    if (isSwimming) {
        currentVx *= 0.6f; // Heavy friction (60% speed)
        vy *= 0.85f;       // Water drag dampens movement
        
        Rectangle upperHalf = getAABB();
        upperHalf.height /= 2.0f;
        
        if (IsKeyDown(KEY_S)) {
            vy += 500.0f * dt; // Actively swim down
        } else if (liquidSim->isWaterAt(upperHalf)) {
            vy -= 250.0f * dt; // Deep underwater, float up naturally
        } else {
            vy += 200.0f * dt; // At surface, sink slightly to maintain half-air half-water
        }

        // Swimming Jump (stroke up)
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_K)) {
            vy = -180.0f;
        }
        
        // Spelunky-style Bubble Spawning (Multiple bubbles)
        bubbleTimer -= 1;
        if (bubbleTimer <= 0) {
            int numBubbles = GetRandomValue(1, 3);
            for(int i = 0; i < numBubbles; i++) {
                EventData ed;
                ed.worldX = x + width / 2.0f + GetRandomValue(-6, 6);
                ed.worldY = y - 4.0f + GetRandomValue(-4, 4);
                EventBus::getInstance()->publish(EventType::EVENT_SPAWN_BUBBLE, ed);
            }
            bubbleTimer = GetRandomValue(15, 40); 
        }
    } else {
        bubbleTimer = 0;
    }
    
    move(currentVx * dt, vy * dt);
    
    if (invincibilityTimer > 0) {
        invincibilityTimer -= dt;
    }
    
    whipHitThisFrame = false;
    
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
            whipHitThisFrame = true;
            
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
    } else if (isSwimming) {
        newAnim = AnimState::SWIM;
    } else if (isClimbing) {
        newAnim = AnimState::CLIMB;
    } else if (!isGrounded) {
        if (vy < 0) newAnim = AnimState::JUMP;
        else newAnim = AnimState::FALL;
    } else {
        if (IsKeyDown(KEY_W)) {
            newAnim = AnimState::LOOK_UP;
        } else if (IsKeyDown(KEY_S)) {
            if (vx > 0.1f || vx < -0.1f) newAnim = AnimState::CRAWL;
            else newAnim = AnimState::DUCK;
        } else if (vx > 0.1f || vx < -0.1f) {
            newAnim = AnimState::RUN;
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
        case AnimState::CLIMB:
            maxFrames = 6;
            row = 6;
            colOffset = 0;
            frameDuration = 0.15f;
            if (vy == 0) maxFrames = 1; // Pause animation if not moving
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
        case AnimState::SWIM:
            maxFrames = 4;
            row = 2;
            colOffset = 0;
            frameDuration = 0.15f;
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
        Rectangle destRec = { x - 12.0f, y - 12.0f, 40.0f, 40.0f };
        
        int whipFrame = 0;
        Rectangle wSrc, wDest;
        
        // Setup Whip Drawing Data
        if (isWhipping && whipSprite.id != 0) {
            whipFrame = (currentFrame < 4) ? 0 : 1; // Winding back (frames 0-3), lashing forward (frames 4-5)
            float srcX = (whipFrame == 0) ? 32.0f : 16.0f; // offset 2 (wind back), offset 1 (lash forward)
            float srcY = 3 * 16.0f; // row 3
            // Native sprite is left-facing. If facing right, width is -16.0f to flip it.
            wSrc = { srcX, srcY, isFacingRight ? -16.0f : 16.0f, 16.0f };
            
            // Scale 16x16 whip to 24x24
            float scale = 24.0f;
            if (whipFrame == 0) {
                // Winding back (behind player)
                wDest = { isFacingRight ? (x - 24.0f) : (x + 16.0f), y, scale, scale };
            } else {
                // Lashing forward (in front of player)
                wDest = { isFacingRight ? (x + 20.0f) : (x - 28.0f), y + 3.0f, scale, scale };
            }
        }

        if (invincibilityTimer <= 0.0f || (int)(invincibilityTimer * 10) % 2 == 0) {
            // Draw Whip Wind-Back (Behind Player)
            if (isWhipping && whipSprite.id != 0 && whipFrame == 0) {
                DrawTexturePro(whipSprite, wSrc, wDest, Vector2{0.0f, 0.0f}, 0.0f, tint);
            }

            // Draw Player
            DrawTexturePro(sprite, frameRec, destRec, Vector2{0.0f, 0.0f}, 0.0f, tint);
            
            // Draw Whip Lash-Forward (In front of Player)
            if (isWhipping && whipSprite.id != 0 && whipFrame == 1) {
                DrawTexturePro(whipSprite, wSrc, wDest, Vector2{0.0f, 0.0f}, 0.0f, tint);
            }
        }
    } else {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height), tint);
    }
}

void Player::takeDamage(int dmg) {
    if (invincibilityTimer > 0.0f) return;
    
    health -= dmg;
    invincibilityTimer = 1.5f;
    vy = -300.0f;
    vx = isFacingRight ? -200.0f : 200.0f;
    isClimbing = false; // Fall off ladder when taking damage
    
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
        EventData data;
        data.worldX = x + width / 2.0f;
        data.worldY = y + height / 2.0f;
        data.vx = vx + (isFacingRight ? 150.0f : -150.0f);
        data.vy = -150.0f;
        EventBus::getInstance()->publish(EventType::EVENT_SPAWN_BOMB, data);
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

void Player::addBomb(int amount) { bombs += amount; }
void Player::addRope(int amount) { ropes += amount; }

bool Player::getIsWhipHitThisFrame() const {
    return whipHitThisFrame;
}

Rectangle Player::getWhipHitbox() const {
    float hitWidth = 24.0f;
    float hitHeight = 12.0f;
    float hitX = isFacingRight ? (x + width) : (x - hitWidth);
    float hitY = y + 4.0f;
    return { hitX, hitY, hitWidth, hitHeight };
}

}
