#include "Enemy.h"
#include "../../core/EventBus.h"
namespace Platformer {

std::shared_ptr<EnemyState> Enemy::idleState = std::make_shared<IdleState>();
std::shared_ptr<EnemyState> Enemy::chaseState = std::make_shared<ChaseState>();
std::shared_ptr<EnemyState> Enemy::returnState = std::make_shared<ReturnState>();

Enemy::Enemy(float x, float y, float w, float h, int hp, int dmg)
    : DynamicEntity(x, y, w, h), health(hp), damage(dmg), currentStateObj(Enemy::idleState),
      animTimer(0.0f), animSpeed(0.1f), currentFrame(0), numFrames(1), baseFrameX(0), baseFrameY(0) {
}

Enemy::~Enemy() {}

void Enemy::setAnimation(int frames, float speed, int baseX, int baseY) {
    if (baseFrameX != baseX || baseFrameY != baseY || numFrames != frames || animSpeed != speed) {
        numFrames = frames;
        animSpeed = speed;
        baseFrameX = baseX;
        baseFrameY = baseY;
        currentFrame = 0;
        animTimer = 0.0f;
    }
}

void Enemy::update(float dt, Player* player) {
    DynamicEntity::update(dt, player);

    if (health <= 0 && isAlive()) {
        EventData data;
        data.amount = 100; // Base points for an enemy, could be customizable later
        data.worldX = x + width / 2.0f;
        data.worldY = y;
        EventBus::getInstance()->publish(EventType::EVENT_ENEMY_KILLED, data);
        
        destroy();
        return;
    }

    if (currentStateObj) {
        currentStateObj->update(this, dt, player);
    }

    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0;
        currentFrame = (currentFrame + 1) % numFrames;
    }

    updateSpriteRect();
}

void Enemy::updateSpriteRect() {
    if (sprite.id != 0) {
        float absW = std::abs(srcRect.width);
        float absH = std::abs(srcRect.height);

        srcRect.x = (baseFrameX + currentFrame) * absW;
        srcRect.y = baseFrameY * absH;

        if (isFacingRight) {
            srcRect.width = -absW;
        } else {
            srcRect.width = absW;
        }
    }
}

void Enemy::render(float lightLevel) {
    unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
    Color tint = { tintVal, tintVal, tintVal, 255 };
    if (sprite.id != 0) {
        float drawX = x - (32.0f - width) / 2.0f;
        float drawY = y - (32.0f - height);
        Rectangle destRect = { drawX, drawY, 32.0f, 32.0f };
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
    }
}

int Enemy::getHealth() const { return health; }
int Enemy::getDamage() const { return damage; }
std::shared_ptr<EnemyState> Enemy::getState() const { return currentStateObj; }

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    vy = -150.0f;
    
    EventData data;
    data.amount = dmg;
    data.worldX = x + width / 2.0f;
    data.worldY = y + height / 2.0f;
    EventBus::getInstance()->publish(EventType::EVENT_ENEMY_DAMAGED, data);
}

void Enemy::changeState(std::shared_ptr<EnemyState> newState) {
    if (currentStateObj) {
        currentStateObj->exit(this);
    }
    currentStateObj = newState;
    if (currentStateObj) {
        currentStateObj->enter(this);
    }
}

}
