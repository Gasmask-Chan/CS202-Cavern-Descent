#include "Enemy.h"

namespace Platformer {

Enemy::Enemy(float x, float y, float w, float h, int hp, int dmg)
    : DynamicEntity(x, y, w, h), health(hp), damage(dmg), currentState(EnemyState::IDLE),
      animTimer(0.0f), animSpeed(0.2f), currentFrame(0), numFrames(1), baseFrameX(0), baseFrameY(0) {
}

Enemy::~Enemy() {}

void Enemy::setAnimation(int frames, float speed, int baseX, int baseY) {
    if (baseFrameX != baseX || baseFrameY != baseY) {
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

    if (numFrames > 1) {
        animTimer += dt;
        if (animTimer >= animSpeed) {
            animTimer -= animSpeed;
            currentFrame = (currentFrame + 1) % numFrames;
        }
    } else {
        currentFrame = 0;
    }

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
EnemyState Enemy::getState() const { return currentState; }

void Enemy::takeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        destroy();
    }
}

}
