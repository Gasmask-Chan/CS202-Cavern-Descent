#pragma once

#include "../Config.h"

namespace Platformer {

enum class GameStateType {
    MENU,
    PLAY,
    PAUSE,
    GAME_OVER,
    CHAR_SELECT,
    EDITOR
};

class GameState {
public:
    virtual ~GameState() = default;

    void enter();

    void exit();

    void handleInput();

    void update(float dt);

    void render();
};

class MenuState : public GameState {

};

class PlayState : public GameState {

};

class PauseState : public GameState {

};

class GameOverState : public GameState {

};

class CharSelectState : public GameState {

};

class EditorState : public GameState {

};

}