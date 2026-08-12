#pragma once

#include <string>
#include "../entities/items/Item.h"

namespace Platformer {

class Item;

struct ShopItem {
    ItemType type;
    std::string name;
    int price;
    bool isSold;
    Item* physicalItem;
};

}
