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
        if (this->deltaTime > 0.033f) this->deltaTime = 0.033f; // Clamp to 30fps to prevent physics tunneling
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
    AudioManager* audio = AudioManager::getInstance();
    
    // Load BGMs
    audio->loadBGM("mTitle", "assets/audio/bgm/mTitle.ogg");
    audio->loadBGM("mCave", "assets/audio/bgm/mCave.ogg");
    audio->loadBGM("mLush", "assets/audio/bgm/mLush.ogg");
    audio->loadBGM("mIce", "assets/audio/bgm/mIce.ogg");
    audio->loadBGM("mTemple", "assets/audio/bgm/mTemple.ogg");
    audio->loadBGM("mBoss", "assets/audio/bgm/mBoss.ogg");
    audio->loadBGM("mVictory", "assets/audio/bgm/mVictory.ogg");
    audio->loadBGM("mCredits", "assets/audio/bgm/mCredits.ogg");

    // Load SFXs
    audio->loadSFX("xjump", "assets/audio/sfx/xjump.wav");
    audio->loadSFX("xland", "assets/audio/sfx/xland.wav");
    audio->loadSFX("xsteps", "assets/audio/sfx/xsteps.wav");
    audio->loadSFX("xclimb1", "assets/audio/sfx/xclimb1.wav");
    audio->loadSFX("xclimb2", "assets/audio/sfx/xclimb2.wav");
    audio->loadSFX("xpush", "assets/audio/sfx/xpush.wav");
    audio->loadSFX("xhurt", "assets/audio/sfx/xhurt.wav");
    audio->loadSFX("xdie", "assets/audio/sfx/xdie.wav");
    audio->loadSFX("xkiss", "assets/audio/sfx/xkiss.wav");
    audio->loadSFX("xletsexplore", "assets/audio/sfx/xletsexplore.wav");
    audio->loadSFX("xbat", "assets/audio/sfx/xbat.wav");
    audio->loadSFX("xspiderjump", "assets/audio/sfx/xspiderjump.wav");
    audio->loadSFX("xghost", "assets/audio/sfx/xghost.wav");
    audio->loadSFX("xwhip", "assets/audio/sfx/xwhip.wav");
    audio->loadSFX("xpickup", "assets/audio/sfx/xpickup.wav");
    audio->loadSFX("xgem", "assets/audio/sfx/xgem.wav");
    audio->loadSFX("xcoin", "assets/audio/sfx/xcoin.wav");
    audio->loadSFX("xchestopen", "assets/audio/sfx/xchestopen.wav");
    audio->loadSFX("xbreak", "assets/audio/sfx/xbreak.wav");
    audio->loadSFX("xthrow", "assets/audio/sfx/xthrow.wav");
    audio->loadSFX("xarrowtrap", "assets/audio/sfx/xarrowtrap.wav");
    audio->loadSFX("xbombready", "assets/audio/sfx/xbombready.wav");
    audio->loadSFX("xexplosion", "assets/audio/sfx/xexplosion.wav");
    audio->loadSFX("xsmallexplode", "assets/audio/sfx/xsmallexplode.wav");
    audio->loadSFX("xignite", "assets/audio/sfx/xignite.wav");
    audio->loadSFX("xflame", "assets/audio/sfx/xflame.wav");
    audio->loadSFX("xtfall", "assets/audio/sfx/xtfall.wav");
    audio->loadSFX("xsplash", "assets/audio/sfx/xsplash.wav");
    audio->loadSFX("xblink1", "assets/audio/sfx/xblink1.wav");
    audio->loadSFX("xblink2", "assets/audio/sfx/xblink2.wav");
    audio->loadSFX("xclick", "assets/audio/sfx/xclick.wav");
    audio->loadSFX("xpause", "assets/audio/sfx/xpause.wav");
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
    AudioManager::getInstance()->updateBGM();
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
            case GameStateType::VICTORY:
                newState = new VictoryState();
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
            case GameStateType::NONE:
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