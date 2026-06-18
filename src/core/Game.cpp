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

    cleanup();
}

void Game::init() {
    InitWindow(1280, 720, "Cavern Descent");
    SetTargetFPS(60);

    currentState = new MenuState;
    currentState->setGame(this);
    currentState->enter();

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

    currentState->render();

    EndDrawing();
}

void Game::cleanup() {
    if (currentState) {
        currentState->exit();
    }

    CloseWindow();
}

void Game::changeState(GameStateType state) {
    if (currentState) {
        currentState->exit();
        delete currentState;
        currentState = nullptr;
    }

    switch (state) {
        case GameStateType::MENU:
            currentState = new MenuState();
            break;
        case GameStateType::PLAY:
            currentState = new PlayState();
            break;
        case GameStateType::PAUSE:
            currentState = new PauseState();
            break;
        case GameStateType::GAME_OVER:   
            currentState = new GameOverState();
            break;
        case GameStateType::CHAR_SELECT:
            currentState = new CharSelectState();
            break;
        case GameStateType::EDITOR:
            currentState = new EditorState();
            break;
        default: 
            break;
}

    if (currentState) {
        currentState->setGame(this);
        currentState->enter();
    }
}

}