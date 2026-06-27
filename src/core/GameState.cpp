#include "GameState.h"
#include "Game.h"
#include "GameManager.h"
#include "../audio/AudioManager.h"

namespace Platformer {

void GameState::setGame(Game* game) {
    this->game = game;
}

/*
=======================================================
=========================MENU==========================
=======================================================
*/

void MenuState::enter() {
    selectedOption = 0;
    // Placeholder: load background texture here
    // Placeholder: start menu BGM here
    // Platformer::AudioManager::getInstance()->playBGM("assets/music/placeholder_menu.ogg");
}

void MenuState::exit() {
    // Placeholder: stop BGM if needed
    // Platformer::AudioManager::getInstance()->stopBGM();
}

void MenuState::handleInput() {
    if (IsKeyPressed(KEY_UP)) {
        selectedOption = (selectedOption + 2) % 3;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        selectedOption = (selectedOption + 1) % 3;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        switch (selectedOption) {
            case 0:
                game->changeState(GameStateType::CHAR_SELECT);
                break;
            case 1:
                game->changeState(GameStateType::EDITOR);
                break;
            case 2:
                game->quit();
                break;
        }
    }
}

void MenuState::update(float dt) {
    // Placeholder: Update background animation or effects if added later
}

void MenuState::render() {
    ClearBackground(BLACK);

    // Placeholder: draw background texture here

    DrawText("CAVERN DESCENT", 400, 150, 60, RAYWHITE);

    Color startColor = (selectedOption == 0) ? YELLOW : DARKGRAY;
    Color editorColor = (selectedOption == 1) ? YELLOW : DARKGRAY;
    Color quitColor = (selectedOption == 2) ? YELLOW : DARKGRAY;

    DrawText("START GAME", 500, 350, 40, startColor);
    DrawText("LEVEL EDITOR", 500, 420, 40, editorColor);
    DrawText("QUIT", 500, 490, 40, quitColor);

    DrawText("Use UP/DOWN to navigate, ENTER to select", 400, 650, 20, GRAY);
}

/*
=======================================================
=========================PLAY==========================
=======================================================
*/

void PlayState::enter() {
    tempGenerator = std::make_unique<LevelGenerator>();
    tempLevel = tempGenerator->generate(1, ZoneType::CAVE);
}

void PlayState::exit() {

}

void PlayState::handleInput() {

}

void PlayState::update(float dt) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->changeState(GameStateType::MENU);
    }
}

void PlayState::render() {
    ClearBackground(BLACK);
    
    if (tempLevel.tileMap) {
        Camera2D cam = { 0 };
        cam.zoom = 1.0f;
        cam.offset = Vector2{ 0, 0 };
        cam.target = Vector2{ 0, 0 };
        
        BeginMode2D(cam);
        
        // Dummy light map (full brightness)
        std::vector<std::vector<float>> lightMap(tempLevel.tileMap->getHeight(), std::vector<float>(tempLevel.tileMap->getWidth(), 1.0f));
        tempLevel.tileMap->render(cam, lightMap);
        
        EndMode2D();
    }
    
    DrawText("TEMPORARY SPRINT 2 TEST BED", 10, 10, 20, YELLOW);
    DrawText("Use ESC to return to menu.", 10, 40, 20, WHITE);
}

/*
=======================================================
=========================PAUSE=========================
=======================================================
*/

void PauseState::enter() {

}

void PauseState::exit() {

}

void PauseState::handleInput() {

}

void PauseState::update(float dt) {

}

void PauseState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================GAMEOVER======================
=======================================================
*/

void GameOverState::enter() {

}

void GameOverState::exit() {

}

void GameOverState::handleInput() {

}

void GameOverState::update(float dt) {

}

void GameOverState::render() {
    ClearBackground(GREEN);
}

/*
=======================================================
=========================CHARSELECT====================
=======================================================
*/

void CharSelectState::enter() {
    selectedIndex = 0;
}

void CharSelectState::exit() {

}

void CharSelectState::handleInput() {
    if (IsKeyPressed(KEY_LEFT)) {
        selectedIndex = (selectedIndex + 2) % 3;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        selectedIndex = (selectedIndex + 1) % 3;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        GameManager::getInstance()->setSelectedCharacter(static_cast<CharacterType>(selectedIndex));
        game->changeState(GameStateType::PLAY);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        game->changeState(GameStateType::MENU);
    }
}

void CharSelectState::update(float dt) {

}

void CharSelectState::render() {
    ClearBackground(BLACK);
    
    DrawText("CHARACTER SELECT", 450, 200, 40, RAYWHITE);
    
    Color expColor = (selectedIndex == 0) ? YELLOW : DARKGRAY;
    Color ninColor = (selectedIndex == 1) ? BLUE : DARKGRAY;
    Color tnkColor = (selectedIndex == 2) ? RED : DARKGRAY;
    
    DrawText("EXPLORER", 300, 400, 30, expColor);
    DrawText("NINJA", 600, 400, 30, ninColor);
    DrawText("TANK", 900, 400, 30, tnkColor);
    
    DrawText("Press ENTER to start, ESC to return", 420, 600, 20, GRAY);
}

/*
=======================================================
=========================EDITOR====================
=======================================================
*/

void EditorState::enter() {

}

void EditorState::exit() {

}

void EditorState::handleInput() {

}

void EditorState::update(float dt) {

}

void EditorState::render() {
    ClearBackground(GREEN);
}

}