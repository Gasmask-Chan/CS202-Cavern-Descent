#pragma once

namespace Platformer {

class MovementStrategy {
public:
    virtual ~MovementStrategy() = default;
    virtual float getMoveSpeed() = 0;
    virtual float getJumpForce() = 0;
    virtual int getMaxHealth() = 0;
    virtual float getGravityScale() = 0;
};

class ExplorerStrategy : public MovementStrategy {
public:
    float getMoveSpeed() override;
    float getJumpForce() override;
    int getMaxHealth() override;
    float getGravityScale() override;
};

class NinjaStrategy : public MovementStrategy {
public:
    float getMoveSpeed() override;
    float getJumpForce() override;
    int getMaxHealth() override;
    float getGravityScale() override;
};

class TankStrategy : public MovementStrategy {
public:
    float getMoveSpeed() override;
    float getJumpForce() override;
    int getMaxHealth() override;
    float getGravityScale() override;
};

}
