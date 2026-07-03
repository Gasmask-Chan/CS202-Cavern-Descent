#include "EntityFactory.h"
#include <map>
#include <string>

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

class VisualEnemy : public DynamicEntity {
    Rectangle srcRect;
public:
    VisualEnemy(float x, float y, const std::string& texPath, Rectangle src) 
        : DynamicEntity(x, y, src.width * 2.0f, src.height * 2.0f), srcRect(src) {
        sprite = getTexture(texPath);
    }

    void render(float lightLevel) override {
        unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
        Color tint = { tintVal, tintVal, tintVal, 255 };
        Rectangle destRect = { x, y, width, height }; 
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
    }
};

class VisualItem : public Item {
    Rectangle srcRect;
public:
    VisualItem(float x, float y, const std::string& texPath, Rectangle src, ItemType t) 
        : Item(x, y, src.width * 2.0f, src.height * 2.0f, t), srcRect(src) {
        sprite = getTexture(texPath);
    }

    void render(float lightLevel) override {
        unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
        Color tint = { tintVal, tintVal, tintVal, 255 };
        Rectangle destRect = { x, y, width, height };
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
    }
};

class VisualTrap : public Trap {
    Rectangle srcRect;
public:
    VisualTrap(float x, float y, const std::string& texPath, Rectangle src) 
        : Trap(x, y, src.width * 2.0f, src.height * 2.0f, 1), srcRect(src) {
        sprite = getTexture(texPath);
    }

    void render(float lightLevel) override {
        unsigned char tintVal = static_cast<unsigned char>(255.0f * lightLevel);
        Color tint = { tintVal, tintVal, tintVal, 255 };
        Rectangle destRect = { x, y, width, height };
        DrawTexturePro(sprite, srcRect, destRect, Vector2{0,0}, 0.0f, tint);
    }
};

std::unique_ptr<DynamicEntity> EntityFactory::createEnemy(char code, float x, float y) {
    std::string texBatSnake = "assets/sprites/16x16/gfx_bat_snake_jetpack.png";
    std::string texSpider = "assets/sprites/16x16/gfx_spider_skeleton.png";
    std::string texCaveman = "assets/sprites/16x16/gfx_caveman_damsel.png";
    std::string texShop = "assets/sprites/16x16/gfx_shopkeeper.png";

    switch (code) {
        case 'S': return std::make_unique<VisualEnemy>(x, y, texBatSnake, Rectangle{0, 16, 16, 16}); // Snake
        case 'B': return std::make_unique<VisualEnemy>(x, y, texBatSnake, Rectangle{0, 0, 16, 16}); // Bat
        case 'P': return std::make_unique<VisualEnemy>(x, y, texSpider, Rectangle{0, 0, 16, 16}); // Spider
        case 'X': return std::make_unique<VisualEnemy>(x, y, texSpider, Rectangle{0, 48, 16, 16}); // Skeleton (col 0, row 3)
        case 'C': return std::make_unique<VisualEnemy>(x, y, texCaveman, Rectangle{0, 32, 16, 16}); // Caveman (col 0, row 2)
        case 'D': return std::make_unique<VisualEnemy>(x, y, texCaveman, Rectangle{0, 80, 16, 16}); // Damsel (col 0, row 5)
        case 'K': return std::make_unique<VisualEnemy>(x, y, texShop, Rectangle{0, 64, 16, 16}); // Shopkeeper (col 0, row 4)
        default: return nullptr;
    }
}

std::unique_ptr<Item> EntityFactory::createItem(char code, float x, float y) {
    std::string texGold = "assets/sprites/16x16/gfx_goldbars.png";
    std::string texSpikes = "assets/sprites/16x16/gfx_spike_collectibles_flame.png";
    std::string texRubies = "assets/sprites/8x8/gfx_rubies.png";

    switch (code) {
        case 'G': return std::make_unique<VisualItem>(x, y, texGold, Rectangle{0, 0, 16, 16}, ItemType::TREASURE); // Goldbars
        case 'R': return std::make_unique<VisualItem>(x, y, texRubies, Rectangle{0, 0, 8, 8}, ItemType::TREASURE); // Rubies (8x8)
        case 'J': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{16, 0, 16, 16}, ItemType::TREASURE); // Jar (using col 1)
        case 'C': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{64, 0, 16, 16}, ItemType::TREASURE); // Crate (col 4, row 0)
        case 'L': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{32, 0, 16, 16}, ItemType::TREASURE); // Chest (col 2, row 0)
        case 'Y': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{80, 0, 16, 16}, ItemType::TREASURE); // Key (col 5)
        case 'I': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{0, 0, 16, 16}, ItemType::TREASURE); // Idol (using col 0)
        case '$': return std::make_unique<VisualItem>(x, y, texSpikes, Rectangle{64, 0, 16, 16}, ItemType::TREASURE); // Shop Item (using Crate visual for now)
        default: return nullptr;
    }
}

std::unique_ptr<Trap> EntityFactory::createTrap(char code, float x, float y) {
    std::string texSpikes = "assets/sprites/16x16/gfx_spike_collectibles_flame.png";
    switch (code) {
        case '^': return std::make_unique<VisualTrap>(x, y, texSpikes, Rectangle{0, 0, 16, 16}); // Spikes
        case '>': 
        case '<': 
            // Arrow traps are handled as map tiles + invisible traps. No sprite rendering needed here, TileMap draws it.
            return std::make_unique<Trap>(x, y, 64, 64, 1); 
        default: return nullptr;
    }
}

}
