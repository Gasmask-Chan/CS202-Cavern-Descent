#pragma once

#include "../level/LevelGenerator.h"
#include "../physics/PhysicsSystem.h"
#include "../level/TileMap.h"
#include "../level/LightingSystem.h"
#include <vector>
#include "../ui/Minimap.h"

namespace Platformer {

class Game;
class Player;

enum class GameStateType {
    MENU,
    PLAY,
    PAUSE,
    GAME_OVER,
    CHAR_SELECT,
    EDITOR,
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

class PlayState : public GameState {
private:
    Player* player;
    Camera2D camera;
    std::unique_ptr<LevelGenerator> tempGenerator;
    GeneratedLevel tempLevel;
    PhysicsSystem* physics;
    std::unique_ptr<Minimap> minimap;
    std::unique_ptr<LightingSystem> lighting;
    
public:
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

class EditorState : public GameState {
public:
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