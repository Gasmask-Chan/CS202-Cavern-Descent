#pragma once

#include "../Config.h"
#include "GameState.h"

namespace Platformer {

class Game {
private:
    GameState* currentState;
    bool isRunning;
    float deltaTime;

public:
    Game();
    
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
     * @brief Calls `currentState->exit()`, deletes old state, creates a new state based on the `GameStateType` enum via a switch statement, calls `setGame(this)` and `enter()` on the new state. Ensures clean resource handoff between states.
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