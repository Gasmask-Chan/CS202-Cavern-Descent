#pragma once

#include "../level/LevelGenerator.h"
#include "../physics/PhysicsSystem.h"
#include "../level/TileMap.h"
#include "../level/LightingSystem.h"
#include "../liquid/LiquidSimulator.h"
#include <vector>
#include <string>
#include <memory>
#include "../ui/Minimap.h"

namespace Platformer {

class ShopSystem;
class Game;

struct HighScoreEntry {
    std::string name;
    int score;
    int floorsReached;
};

class Game;
class Player;

enum class GameStateType {
    MENU,
    PLAY,
    PAUSE,
    GAME_OVER,
    VICTORY,
    CHAR_SELECT,
    EDITOR_MENU,
    EDITOR_FILE_MENU,
    EDITOR,
    TRANSITION,
    NONE
};

enum class CharacterType {
    EXPLORER,
    NINJA,
    TANK
};

class GameState {
protected:
    Game* game;

public:
    virtual ~GameState() = default;

    /**
     * @brief Stores the owning `Game` pointer. Called by `Game::changeState()` and `Game::init()` immediately after creating a new state.
     * 
     * @param g 
     */
    void setGame(Game* g);

    void drawCenteredText(const char* text, float y, float fontSize, Color color);
    void drawCenteredAt(const char* text, float centerX, float y, float fontSize, Color color);
    void drawLeftText(const char* text, float x, float y, float fontSize, Color color);

    virtual void enter() = 0;

    virtual void exit() = 0;

    virtual void handleInput() = 0;

    virtual void update(float dt) = 0;

    virtual void render() = 0;
};

class MenuState : public GameState {
private:
    int selectedOption;
    Texture2D background;

public:
    /**
     * @brief Loads menu background texture. Starts menu BGM via `AudioManager`. Sets `selectedOption = 0`.
     * 
     */
    void enter() override;

    void exit() override;

    /**
     * @brief Up/Down arrows change `selectedOption` (0=Start, 1=Editor, 2=Quit). Enter key triggers: 0→`CharSelectState`, 1→`EditorState`, 2→exit.
     * 
     */
    void handleInput() override;

    void update(float dt) override;

    void render() override;
};

struct ExplosionFlash {
    float x;
    float y;
    float timer;
};

class PlayState : public GameState {
private:
    std::unique_ptr<Player> player;
    Camera2D camera;
    std::unique_ptr<LevelGenerator> tempGenerator;
    GeneratedLevel tempLevel;
    std::unique_ptr<PhysicsSystem> physics;
    std::unique_ptr<Minimap> minimap;
    std::unique_ptr<LightingSystem> lighting;
    std::unique_ptr<LiquidSimulator> liquids;
    std::unique_ptr<class ComboSystem> combo;
    std::unique_ptr<ShopSystem> shop;
    std::vector<std::unique_ptr<Item>> pendingItems;
    std::vector<std::unique_ptr<DynamicEntity>> pendingEntities;
    std::vector<ExplosionFlash> explosionFlashes;
    
    float cameraShakeTimer;
    float cameraShakeIntensity;
    
    bool ghostSpawned;
    float deathTimer;
    
    Texture2D hudIcons;
    Texture2D shopkeeperTex;
    Font hudFont;
    
public:
    PlayState();
    ~PlayState();
    PlayState(const PlayState&) = delete;
    PlayState& operator=(const PlayState&) = delete;

    /**
     * @brief 	Creates `LevelManager`, `PhysicsSystem`, `LightingSystem`, `LiquidSimulator`, `ComboSystem`, `Minimap`, `HUD`. Calls `levelManager->generateFloor(1)`. Starts zone BGM.
     * 
     */
    void enter() override;

    void exit() override;

    void handleInput() override;

    /**
     * @brief Executes the 21-step update order: input → player → ghost → enemies → gravity → collisions → items → bombs → liquids → lighting → events → combo → camera → cleanup → death check.
     * 
     * @param dt 
     */
    void update(float dt) override;

    void render() override;
};

class PauseState : public GameState {
private:
    int selectedIndex = 0;
public:
    void enter() override;

    void exit() override;

    /**
     * @brief Escape key → return to `PlayState`. Up/Down select Resume/Quit. Enter triggers selected option.
     * 
     */
    void handleInput() override;

    void update(float dt) override;

    void render() override;
};

class GameOverState : public GameState {
private:
    int finalScore = 0;
    int finalFloor = 0;
    char nameInput[4] = "\0\0\0";
    int letterCount = 0;
    bool nameEntered = false;
    std::vector<HighScoreEntry> leaderboard;
public:
    /**
     * @brief Captures final score and floors reached from `GameManager`. Prompts for name entry for high score save.
     * 
     */
    void enter() override;

    void exit() override;

    void handleInput() override;

    void update(float dt) override;

    void render() override;
};

class VictoryState : public GameState {
public:
    enum class EndingScene {
        SCENE1_TUNNEL,
        SCENE2_DESERT_FALL,
        SCENE3_TALLY,
        SCENE4_BLACK_SCREEN,
        SCENE5_SUMMARY
    };

    struct CutsceneActor {
        Vector2 position = {0, 0};
        Vector2 velocity = {0, 0};
        float animTimer = 0.0f;
        int animFrame = 0;
        bool isFacingRight = true;
    };

    struct GemDrop {
        Vector2 position = {0, 0};
        Vector2 velocity = {0, 0};
        int type = 0; // 0=gold, 1=emerald, 2=sapphire, 3=ruby
        float rotation = 0.0f;
    };

private:
    int finalScore = 0;
    int finalFloor = 0;
    char nameInput[4] = "\0\0\0";
    int letterCount = 0;
    bool nameEntered = false;
    std::vector<HighScoreEntry> leaderboard;

    // Redesigned cutscene states & assets
    EndingScene currentScene = EndingScene::SCENE1_TUNNEL;
    float sceneTimer = 0.0f;
    int stepsTaken = 0;
    float stepAnimTimer = 0.0f;
    
    CutsceneActor player;
    CutsceneActor chest;
    float chestScale = 1.0f;
    
    // Grid-based random sand for Scene 2 (width=40 tiles, height=5 rows)
    int sandGrid[40][5];

    // Asset Textures
    Texture2D skyTex;
    Texture2D mountainTex;
    Texture2D sandTex;
    Texture2D sand2Tex;
    Texture2D sandTopTex;
    Texture2D palmTreeTex;
    Texture2D shrubTex;
    Texture2D bigTreasureTex;
    Texture2D playerSpriteSheet;
    Texture2D templeTex;

    // Scene 3 Tally fields
    int tallyStatus = 0; 
    float tallyTimer = 0.0f;
    float currentTallyScore = 0.0f;
    std::vector<GemDrop> gems;

    float fadeAlpha = 0.0f;
    float blackScreenTimer = 0.0f;

    // Scene 1 Objects (same structure as TransitionState)
    std::unique_ptr<Player> cutscenePlayer;
    std::unique_ptr<PhysicsSystem> physics;
    std::unique_ptr<TileMap> tunnelMap;
    std::unique_ptr<LightingSystem> lighting;
    std::unique_ptr<LiquidSimulator> lavaSim;

    // Camera for Mode2D tracking
    Camera2D camera = {0};

    // Helper functions
    void updateScene1(float dt);
    void updateScene2(float dt);
    void updateScene3(float dt);
    void updateScene4(float dt);
    void updateScene5(float dt);

    void renderScene1();
    void renderScene2();
    void renderScene3();
    void renderScene4();
    void renderScene5();

public:
    VictoryState();
    ~VictoryState() override;
    VictoryState(const VictoryState&) = delete;
    VictoryState& operator=(const VictoryState&) = delete;
    VictoryState(VictoryState&&) = delete;
    VictoryState& operator=(VictoryState&&) = delete;

    void enter() override;
    void exit() override;
    void handleInput() override;
    void update(float dt) override;
    void render() override;
};

class TransitionState : public GameState {
private:
    std::unique_ptr<Player> player;
    std::unique_ptr<PhysicsSystem> physics;
    std::unique_ptr<TileMap> tunnelMap;
    std::unique_ptr<LightingSystem> lighting;
    Camera2D camera = {0};
    
public:
    TransitionState();
    ~TransitionState() override;
    TransitionState(const TransitionState&) = delete;
    TransitionState& operator=(const TransitionState&) = delete;
    TransitionState(TransitionState&&) = delete;
    TransitionState& operator=(TransitionState&&) = delete;
    void enter() override;
    void exit() override;
    void handleInput() override;
    void update(float dt) override;
    void render() override;
};

class CharSelectState : public GameState {
private:
    int selectedIndex = 0;
    std::vector<CharacterType> characters = {
        CharacterType::EXPLORER, 
        CharacterType::NINJA, 
        CharacterType::TANK
    };

public:
    void enter() override;

    void exit() override;

    void handleInput() override;

    void update(float dt) override;

    void render() override;
};

class LevelEditorMenuState : public GameState {
private:
    int selectedOption = 0;
public:
    void enter() override;
    void exit() override;
    void handleInput() override;
    void update(float dt) override;
    void render() override;
};

class EditorFileMenuState : public GameState {
private:
    int selectedOption = 0;
public:
    void enter() override;
    void exit() override;
    void handleInput() override;
    void update(float dt) override;
    void render() override;
};



struct PaletteItem {
    TileType type;
    Texture2D tex;
    Rectangle src;
    std::string name;
};

struct TileChange {
    int x, y;
    TileType oldType;
    TileType newType;
};

class EditorState : public GameState {
private:
    std::unique_ptr<class TileMap> tileMap;
    Camera2D camera;
    std::vector<PaletteItem> paletteItems;
    int selectedTileIdx = 0;
    Vector2 mouseGridPos = {0,0};
    std::string statusMsg = "";
    float statusTimer = 0.0f;
    Vector2 panDragStart = {0, 0};
    bool isDragging = false;
    
    // Undo/Redo
    std::vector<TileChange> undoStack;
    std::vector<TileChange> redoStack;
    void placeTile(int tx, int ty, TileType newType);
    void undo();
    void redo();
    
    void saveLevel(const std::string& path);
    void loadLevel(const std::string& path);

public:
    ~EditorState() override;
    /**
     * @brief Creates `LevelEditor` instance. Initializes empty tilemap. Shows tile/entity palette UI.
     * 
     */
    void enter() override;

    void exit() override;

    void handleInput() override;

    void update(float dt) override;

    void render() override;
};

}