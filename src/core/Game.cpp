#include "Game.h"
#include "../audio/AudioManager.h"

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
    while (!WindowShouldClose() && isRunning) {
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

    // Initialize audio device early
    AudioManager::getInstance();
    Image fontImg = LoadImage("assets/fonts/font.png");
    ImageFormat(&fontImg, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageColorReplace(&fontImg, BLACK, BLANK);
    
    globalFont = { 0 };
    globalFont.baseSize = 8;
    globalFont.glyphCount = 85; // 59 uppercase/symbols + 26 lowercase
    globalFont.glyphPadding = 0;
    globalFont.texture = LoadTextureFromImage(fontImg);
    globalFont.recs = (Rectangle *)MemAlloc(85 * sizeof(Rectangle));
    globalFont.glyphs = (GlyphInfo *)MemAlloc(85 * sizeof(GlyphInfo));
    
    // Uppercase and symbols (ASCII 32 to 90)
    for (int i = 0; i < 59; i++) {
        globalFont.recs[i] = { 0.0f, (float)(i * 8), 8.0f, 8.0f };
        globalFont.glyphs[i].value = 32 + i;
        globalFont.glyphs[i].offsetX = 0;
        globalFont.glyphs[i].offsetY = 0;
        globalFont.glyphs[i].advanceX = 8;
        globalFont.glyphs[i].image.data = nullptr; 
    }
    
    // Lowercase 'a' to 'z' mapped to 'A' to 'Z'
    for (int i = 0; i < 26; i++) {
        int idx = 59 + i;
        int upperIndex = 33 + i; // 'A' is 33rd char (65 - 32)
        globalFont.recs[idx] = { 0.0f, (float)(upperIndex * 8), 8.0f, 8.0f };
        globalFont.glyphs[idx].value = 97 + i; // 'a' is 97
        globalFont.glyphs[idx].offsetX = 0;
        globalFont.glyphs[idx].offsetY = 0;
        globalFont.glyphs[idx].advanceX = 8;
        globalFont.glyphs[idx].image.data = nullptr; 
    }
    UnloadImage(fontImg);

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
    
    UnloadFont(globalFont);

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

void Game::quit() {
    isRunning = false;
}

}