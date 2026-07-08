#include "EntityFactory.h"
#include <map>
#include <string>
#include "Snake.h"
#include "Bat.h"
#include "Spider.h"
#include "NemesisGhost.h"

namespace Platformer {

static std::map<std::string, Texture2D> textureCache;

static Texture2D getTexture(const std::string& path) {
    if (textureCache.find(path) == textureCache.end()) {
        Image img = LoadImage(path.c_str());
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        Color chromaKey = GetImageColor(img, 0, 0); 
        ImageColorReplace(&img, chromaKey, BLANK);
        textureCache[path] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    return textureCache[path];
}

std::unique_ptr<DynamicEntity> EntityFactory::createEnemy(char code, float x, float y) {
    std::string texBatSnake = "assets/sprites/16x16/gfx_bat_snake_jetpack.png";
    std::string texSpider = "assets/sprites/16x16/gfx_spider_skeleton.png";
    
    std::unique_ptr<Enemy> enemy = nullptr;

    switch (code) {
        case 'S': 
            enemy = std::make_unique<Snake>(x, y, 32.0f, 32.0f);
            enemy->setSprite(getTexture(texBatSnake), Rectangle{0, 16, 16, 16});
            break;
        case 'B': 
            enemy = std::make_unique<Bat>(x, y, 32.0f, 32.0f);
            enemy->setSprite(getTexture(texBatSnake), Rectangle{0, 0, 16, 16});
            break;
        case 'P': 
            enemy = std::make_unique<Spider>(x, y, 32.0f, 32.0f);
            enemy->setSprite(getTexture(texSpider), Rectangle{0, 0, 16, 16});
            break;
        default: return nullptr;
    }
    return enemy;
}

std::unique_ptr<Item> EntityFactory::createItem(char code, float x, float y) {
    std::string texGold = "assets/sprites/16x16/gfx_goldbars.png";
    std::string texSpikes = "assets/sprites/16x16/gfx_spike_collectibles_flame.png";
    std::string texRubies = "assets/sprites/8x8/gfx_rubies.png";

    std::unique_ptr<Item> item = nullptr;

    switch (code) {
        case 'G': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texGold), Rectangle{0, 0, 16, 16});
            break;
        case 'R': 
            item = std::make_unique<Item>(x, y, 16.0f, 16.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texRubies), Rectangle{0, 0, 8, 8});
            break;
        case 'J': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{16, 0, 16, 16});
            break;
        case 'C': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{64, 0, 16, 16});
            break;
        case 'L': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{32, 0, 16, 16});
            break;
        case 'Y': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{80, 0, 16, 16});
            break;
        case 'I': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{0, 0, 16, 16});
            break;
        case '$': 
            item = std::make_unique<Item>(x, y, 32.0f, 32.0f, ItemType::TREASURE);
            item->setSprite(getTexture(texSpikes), Rectangle{64, 0, 16, 16});
            break;
        default: return nullptr;
    }
    return item;
}

std::unique_ptr<Trap> EntityFactory::createTrap(char code, float x, float y) {
    std::string texSpikes = "assets/sprites/16x16/gfx_spike_collectibles_flame.png";
    
    std::unique_ptr<Trap> trap = nullptr;
    
    if (code == '^') {
        trap = std::make_unique<Trap>(x, y, 32.0f, 32.0f, 1);
        trap->setSprite(getTexture(texSpikes), Rectangle{0, 0, 16, 16});
        return trap;
    } else if (code == '>' || code == '<') {
        // Arrow traps
        return std::make_unique<Trap>(x, y, 64.0f, 64.0f, 1);
    }
    
    return nullptr;
}

}
