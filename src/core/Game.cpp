#include "Game.h"

namespace Platformer {

Game::Game() {
    currentState = nullptr;
    isRunning = false;
    deltaTime = 0.0;
}

Game::~Game() {
    if (currentState) {
        delete currentState;
    }
}

void Game::run() {
    init();
    while (!WindowShouldClose()) {
        this->deltaTime = GetFrameTime();
        handleInput();
        update(deltaTime);
        render();
    }

    CloseWindow();
}

void Game::init() {
    InitWindow(1280, 720, "Cavern Descent");
    currentState = new MenuState;
    isRunning = true;
}

void Game::handleInput() {
    currentState->handleInput();
}

void Game::update(float dt) {
    currentState->update(dt);
}

void Game::render() {
    BeginDrawing();
    ClearBackground(GREEN);

    currentState->render();

    EndDrawing();
}

}