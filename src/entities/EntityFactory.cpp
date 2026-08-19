#include "EntityFactory.h"
#include <map>
#include <string>
#include "traps/Trap.h"
#include "traps/ArrowTrap.h"
#include "projectiles/Arrow.h"
#include "enemies/Snake.h"
#include "enemies/Bat.h"
#include "enemies/Spider.h"
#include "enemies/NemesisGhost.h"
#include "enemies/Spike.h"
#include "enemies/Flame.h"
#include "effects/Explosion.h"
#include "effects/Particle.h"

namespace Platformer {

static std::map<std::string, Texture2D> textureCache;

Texture2D EntityFactory::getTexture(const std::string& path) {
    if (textureCache.find(path) == textureCache.end()) {
        Image img = LoadImage(path.c_str());
        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
        
        // Skip chroma keying for liquid textures which are solid or already have alpha
        if (path.find("Lava") == std::string::npos && path.find("water") == std::string::npos) {
            Color chromaKey = GetImageColor(img, 0, 0); 
            ImageColorReplace(&img, chromaKey, BLANK);
        }
        
        textureCache[path] = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    return textureCache[path];
}

void EntityFactory::preloadTextures() {
    getTexture("assets/sprites/8x8/gfx_rubies.png");
    getTexture("assets/sprites/16x16/gfx_spike_collectibles_flame.png");
    getTexture("assets/sprites/8x8/gfx_bomb.png");
    getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png");
    getTexture("assets/sprites/16x16/gfx_goldbars.png");
    getTexture("assets/sprites/16x16/gfx_bat_snake_jetpack.png");
    getTexture("assets/sprites/16x16/gfx_spider_skeleton.png");
    getTexture("assets/npc/Ghost.png");
    getTexture("assets/sprites/lava/LavaDrip.png");
    getTexture("assets/sprites/8x8/bubble.png");
    getTexture("assets/sprites/64x64/gfx_explosion.png");
    getTexture("assets/sprites/8x8/gfx_arrow.png");
    getTexture("assets/tilemaps/gfx_cavebg.png");
    getTexture("assets/tilemaps/gfx_junglebg.png");
    getTexture("assets/tilemaps/gfx_templebg.png");
}

void EntityFactory::unloadTextures() {
    for (auto& pair : textureCache) {
        UnloadTexture(pair.second);
    }
    textureCache.clear();
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
        case '^': 
            enemy = std::make_unique<Spike>(x, y, 32.0f, 32.0f);
            enemy->setSprite(getTexture("assets/sprites/16x16/gfx_spike_collectibles_flame.png"), Rectangle{0, 0, 16, 16});
            break;
        case 'F':
            enemy = std::make_unique<Flame>(x, y, -400.0f); // Default vy
            enemy->setSprite(getTexture("assets/sprites/16x16/gfx_spike_collectibles_flame.png"), Rectangle{32, 16, 16, 16}); // Base Flame Frame
            break;
        default: return nullptr;
    }
    return enemy;
}

std::unique_ptr<DynamicEntity> EntityFactory::createGhost(float x, float y) {
    auto ghost = std::make_unique<NemesisGhost>(x, y, 32.0f, 32.0f);
    ghost->setSprite(getTexture("assets/npc/Ghost.png"), Rectangle{0, 0, 160, 160});
    return ghost;
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
            item->renderOffsetY = 16.0f;
            break;
        case 'R': {
            item = std::make_unique<LootPickup>(x, y, 16.0f, 16.0f, 100); // Ruby
            float col = (float)GetRandomValue(0, 2) * 8.0f;
            item->setSprite(getTexture(texRubies), Rectangle{col, 0, 8, 8});
            break;
        }
        case 'J': // Jade/Emerald?
            item = std::make_unique<LootPickup>(x, y, 32.0f, 32.0f, 1000);
            item->setSprite(getTexture(texSpikes), Rectangle{16, 0, 16, 16});
            break;
        case 'C': // Chest (Closed)
            item = std::make_unique<Chest>(x, y, 32.0f, 32.0f);
            item->setSprite(getTexture(texSpikes), Rectangle{32, 0, 16, 16}); // Row 0 Col 2
            item->renderOffsetY = 16.0f;
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

std::unique_ptr<Arrow> EntityFactory::createArrow(float x, float y, float vx) {
    auto arrow = std::make_unique<Arrow>(x, y, vx);
    arrow->setSprite(getTexture("assets/sprites/8x8/gfx_arrow.png"), Rectangle{0, 0, 8, 8});
    return arrow;
}

std::unique_ptr<Explosion> EntityFactory::createExplosion(float x, float y) {
    auto exp = std::make_unique<Explosion>(x, y);
    exp->setSprite(getTexture("assets/sprites/64x64/gfx_explosion.png"), Rectangle{0, 0, 64, 64});
    return exp;
}

std::unique_ptr<Particle> EntityFactory::createBloodParticle(float x, float y) {
    float vx = static_cast<float>(GetRandomValue(-150, 150));
    float vy = static_cast<float>(GetRandomValue(-250, -50));
    float lifetime = static_cast<float>(GetRandomValue(10, 30)) / 10.0f; // 1.0s to 3.0s
    
    auto particle = std::make_unique<Particle>(x, y, vx, vy, lifetime);
    
    // Choose random blood frame (columns 0 to 5)
    int frameCol = GetRandomValue(0, 5);
    particle->setSprite(getTexture("assets/sprites/8x8/gfx_blood_rock_rope_poof.png"), Rectangle{frameCol * 8.0f, 0, 8, 8});
    
    return particle;
}

std::unique_ptr<Trap> EntityFactory::createTrap(char code, float x, float y) {
    std::string texSpikes = "assets/sprites/16x16/gfx_spike_collectibles_flame.png";
    
    std::unique_ptr<Trap> trap = nullptr;
    
    if (code == '>') {
        // Arrow trap facing Right. 
        return std::make_unique<ArrowTrap>(x, y, true);
    } else if (code == '<') {
        // Arrow trap facing Left.
        return std::make_unique<ArrowTrap>(x, y, false);
    }
    
    return nullptr;
}

}
