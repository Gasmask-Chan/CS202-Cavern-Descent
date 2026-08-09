#pragma once

#include "../DynamicEntity.h"

namespace Platformer {

class Player;

enum ItemType {
    LOOT_PICKUP,
    BOMB_PICKUP,
    ROPE_PICKUP,
    CHEST,
    HEALTH_PICKUP
};

class Item : public DynamicEntity {
protected:
    ItemType type;
    bool isCollected;
    float prevVy; // Used for bouncing logic

public:
    bool isShopItem = false;
    bool isHeld = false;
    bool isEmbedded = false;
    
    Item(float x, float y, float w, float h, ItemType type);

    virtual void update(float dt, class Player* player = nullptr) override;

    virtual void activate(Player* player);

    virtual void render(float lightLevel) override;

    void collect();

    bool isPickedUp();

    ItemType getType();
};

class LootPickup : public Item {
private:
    int value;
public:
    LootPickup(float x, float y, float w, float h, int val);
    void activate(Player* player) override;
};



class Chest : public Item {
private:
    bool isOpened;
public:
    Chest(float x, float y, float w, float h);
    void update(float dt, class Player* player = nullptr) override;
    void activate(Player* player) override;
};

class BombPickup : public Item {
private:
    int amount;
public:
    BombPickup(float x, float y, float w, float h, int amount);
    void activate(Player* player) override;
};

class RopePickup : public Item {
private:
    int amount;
public:
    RopePickup(float x, float y, float w, float h, int amount);
    void activate(Player* player) override;
};

}