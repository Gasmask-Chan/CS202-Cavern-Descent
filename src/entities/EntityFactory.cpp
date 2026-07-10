#include "EntityFactory.h"
#include <map>
#include <string>
#include "enemies/Snake.h"
#include "enemies/Bat.h"
#include "enemies/Spider.h"
#include "enemies/NemesisGhost.h"

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
            enemy = std::make_unique<Snake>(x + 4.0f, y + 8.0f, 24.0f, 24.0f);
            enemy->setSprite(getTexture(texBatSnake), Rectangle{0, 16, 16, 16});
            break;
        case 'B': 
            enemy = std::make_unique<Bat>(x + 4.0f, y, 24.0f, 24.0f);
            enemy->setSprite(getTexture(texBatSnake), Rectangle{0, 0, 16, 16});
            break;
        case 'P': 
            enemy = std::make_unique<Spider>(x + 4.0f, y, 24.0f, 24.0f);
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
            item = std::make_unique<LootPickup>(x, y, 32.0f, 32.0f, 500); // Gold Bar
            item->setSprite(getTexture(texGold), Rectangle{0, 0, 16, 16});
            break;
        case 'R': 
            item = std::make_unique<LootPickup>(x, y, 16.0f, 16.0f, 100); // Ruby
            item->setSprite(getTexture(texRubies), Rectangle{0, 0, 8, 8});
            break;
        case 'J': // Jade/Emerald?
            item = std::make_unique<LootPickup>(x, y, 32.0f, 32.0f, 1000);
            item->setSprite(getTexture(texSpikes), Rectangle{16, 0, 16, 16});
            break;
        case 'C': // Chest (Closed)
            item = std::make_unique<Chest>(x, y, 32.0f, 32.0f);
            item->setSprite(getTexture(texSpikes), Rectangle{32, 0, 16, 16}); // Row 0 Col 2
            break;
        case '$': 
            return createItem('C', x, y); // Chest
        case 'I': 
            return createItem('U', x, y); // Rope
        case 'Y': 
            return createItem('R', x, y); // Ruby
        case 'L': 
            return createItem('O', x, y); // Bomb
        case 'O': // Bomb
            item = std::make_unique<BombPickup>(x, y, 16.0f, 16.0f, 3); // 3 bombs
            item->setSprite(getTexture("assets/sprites/8x8/gfx_bomb.png"), Rectangle{0, 0, 8, 8});
            break;
        case 'U': // Rope
            item = std::make_unique<RopePickup>(x, y, 16.0f, 16.0f, 3); // 3 ropes
            item->setSprite(getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"), Rectangle{72, 0, 8, 8}); // Row 0 Col 9
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
