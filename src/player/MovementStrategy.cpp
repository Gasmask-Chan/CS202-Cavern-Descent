#include "MovementStrategy.h"

namespace Platformer {

float ExplorerStrategy::getMoveSpeed() { return 200.0f; }
float ExplorerStrategy::getJumpForce() { return 450.0f; }
int ExplorerStrategy::getMaxHealth() { return 4; }
float ExplorerStrategy::getGravityScale() { return 1.0f; }

float NinjaStrategy::getMoveSpeed() { return 280.0f; }
float NinjaStrategy::getJumpForce() { return 550.0f; }
int NinjaStrategy::getMaxHealth() { return 2; }
float NinjaStrategy::getGravityScale() { return 0.85f; }

float TankStrategy::getMoveSpeed() { return 140.0f; }
float TankStrategy::getJumpForce() { return 380.0f; }
int TankStrategy::getMaxHealth() { return 6; }
float TankStrategy::getGravityScale() { return 1.2f; }

}
