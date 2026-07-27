#include "Game.h"
#include "../audio/AudioManager.h"

namespace Platformer {

Game::Game() {
    isRunning = false;
    deltaTime = 0.0;
}

Game::~Game() {
    for (auto state : stateStack) {
        delete state;
    }
    stateStack.clear();
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
    SetExitKey(0); // Disable closing window on ESC so we can use it for Pause
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

    GameState* initialState = new MenuState;
    initialState->setGame(this);
    initialState->enter();
    stateStack.push_back(initialState);

    isRunning = true;
}

void Game::handleInput() {
    if (!stateStack.empty()) {
        stateStack.back()->handleInput();
    }
}

void Game::update(float dt) {
    if (!stateStack.empty()) {
        stateStack.back()->update(dt);
    }
    applyPendingStateChanges();
}

void Game::render() {
    BeginDrawing();
    for (auto state : stateStack) {
        state->render();
    }

    EndDrawing();
}

void Game::cleanup() {
    for (auto state : stateStack) {
        state->exit();
        delete state;
    }
    stateStack.clear();
    
    UnloadFont(globalFont);

    CloseWindow();
}

void Game::changeState(GameStateType state) {
    pendingAction = StateAction::CHANGE;
    pendingState = state;
}

void Game::pushState(GameStateType state) {
    pendingAction = StateAction::PUSH;
    pendingState = state;
}

void Game::applyPendingStateChanges() {
    if (pendingAction == StateAction::NONE) return;

    if (pendingAction == StateAction::CHANGE) {
        for (auto s : stateStack) {
            s->exit();
            delete s;
        }
        stateStack.clear();
    } else if (pendingAction == StateAction::POP) {
        if (!stateStack.empty()) {
            stateStack.back()->exit();
            delete stateStack.back();
            stateStack.pop_back();
        }
        pendingAction = StateAction::NONE;
        return; // Don't push a new state
    }

    if (pendingAction == StateAction::CHANGE || pendingAction == StateAction::PUSH) {
        GameState* newState = nullptr;
        switch (pendingState) {
            case GameStateType::MENU:
                newState = new MenuState();
                break;
            case GameStateType::PLAY:
                newState = new PlayState();
                break;
            case GameStateType::PAUSE:
                newState = new PauseState();
                break;
            case GameStateType::GAME_OVER:   
                newState = new GameOverState();
                break;
            case GameStateType::CHAR_SELECT:
                newState = new CharSelectState();
                break;
            case GameStateType::EDITOR_MENU:
                newState = new LevelEditorMenuState();
                break;
            case GameStateType::EDITOR_FILE_MENU:
                newState = new EditorFileMenuState();
                break;
            case GameStateType::EDITOR:
                newState = new EditorState();
                break;
            case GameStateType::TRANSITION:
                newState = new TransitionState();
                break;
        }

        if (newState) {
            newState->setGame(this);
            newState->enter();
            stateStack.push_back(newState);
        }
    }
    
    pendingAction = StateAction::NONE;
}

void Game::popState() {
    pendingAction = StateAction::POP;
}

void Game::quit() {
    isRunning = false;
}

}