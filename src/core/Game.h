#pragma once

#include "../Config.h"
#include "GameState.h"
#include <vector>

namespace Platformer {

class Game {
private:
    std::vector<GameState*> stateStack;
    bool isRunning;
    float deltaTime;

    Font globalFont;

    enum class StateAction { NONE, CHANGE, PUSH, POP };
    StateAction pendingAction = StateAction::NONE;
    GameStateType pendingState;

    void applyPendingStateChanges();

public:
    Game();
    
    Font getFont() const { return globalFont; }
    
    ~Game();

    /**
     * @brief Main entry point. Calls `init()`, then enters the main `while (!WindowShouldClose())` loop calling `handleInput()`, `update(dt)`, `render()` each frame. Calls `cleanup()` on exit.
     * 
     */
    void run();

    /**
     * @brief Initializes Raylib window (`InitWindow`), sets target FPS to 60, initializes `GameManager` and `AudioManager` singletons, loads all shared textures and sounds, creates the initial `MenuState`, calls `setGame(this)` and `enter()` on the initial state.
     * 
     */
    void init();

    /**
     * @brief Delegates to `currentState->handleInput()`. No game-level input processing - all input is state-specific.
     * 
     */
    void handleInput();

    /**
     * @brief Passes `GetFrameTime()` delta to `currentState->update(dt)`. Checks for pending state transitions queued by `changeState()`.
     * 
     * @param dt 
     */
    void update(float dt);

    /**
     * @brief Calls `BeginDrawing()`, delegates to `currentState->render()`, then `EndDrawing()`.
     * 
     */
    void render();

    /**
     * @brief Deletes current `GameState`, calls `CloseAudioDevice()`, unloads all textures via Raylib, calls `CloseWindow()`.
     * 
     */
    void cleanup();

    /**
     * @brief Pushes a new state onto the state stack (e.g., PauseMenu over PlayState).
     */
    void pushState(GameStateType state);

    /**
     * @brief Pops the current state off the stack, resuming the one below it.
     */
    void popState();

    /**
     * @brief Cleans up and clears the state stack, pushes the new state as the sole active state.
     * 
     * @param state 
     */
    void changeState(GameStateType state);

    /**
     * @brief Set `isRunning = false`
     * 
     */
    void quit();
};

}