#include "AudioManager.h"
#include <iostream>
#include <algorithm>

namespace Platformer {

AudioManager::AudioManager() : currentBGMName(""), sfxVolume(1.0f), bgmVolume(0.4f) {
    InitAudioDevice(); // Required by Raylib before any audio loading/playing
    currentBGM = nullptr;
}

AudioManager::~AudioManager() {
    // Free all loaded SFX
    for (auto& pair : sfxCache) {
        UnloadSound(pair.second);
    }
    sfxCache.clear();

    // Free loaded BGM
    for (auto& pair : bgmCache) {
        UnloadMusicStream(pair.second);
    }
    bgmCache.clear();
    currentBGM = nullptr;
    CloseAudioDevice();
}

AudioManager* AudioManager::getInstance() {
    static AudioManager instance;
    return &instance;
}

void AudioManager::loadSFX(const std::string& name, const std::string& filePath) {
    Sound sound = LoadSound(filePath.c_str());
    // In raylib, a successful load has a non-null buffer
    if (sound.stream.buffer != nullptr) {
        SetSoundVolume(sound, sfxVolume);
        sfxCache[name] = sound;
    } else {
        std::cerr << "AudioManager: Failed to load SFX: " << filePath << std::endl;
    }
}

void AudioManager::loadBGM(const std::string& name, const std::string& filePath) {
    Music m = LoadMusicStream(filePath.c_str());
    if (m.stream.buffer != nullptr) {
        bgmCache[name] = m;
    } else {
        std::cerr << "AudioManager Warning: Failed to load BGM: " << filePath << std::endl;
    }
}

void AudioManager::playSFX(const std::string& name) {
    auto it = sfxCache.find(name);
    if (it != sfxCache.end()) {
        SetSoundVolume(it->second, sfxVolume); // Ensure current volume is applied
        PlaySound(it->second);
    }
}

void AudioManager::playBGM(const std::string& name) {
    if (name == currentBGMName) return;

    auto it = bgmCache.find(name);
    if (it == bgmCache.end()) {
        return;
    }

    if (currentBGM != nullptr) {
        StopMusicStream(*currentBGM);
    }

    currentBGM = &it->second;
    SetMusicVolume(*currentBGM, bgmVolume);
    PlayMusicStream(*currentBGM);
    currentBGMName = name;
}

void AudioManager::stopBGM() {
    if (currentBGM != nullptr) {
        StopMusicStream(*currentBGM);
        currentBGMName = "";
        currentBGM = nullptr;
    }
}

void AudioManager::updateBGM() {
    // Raylib requires this to be called every frame to stream the music
    if (currentBGM != nullptr) {
        UpdateMusicStream(*currentBGM);
    }
}

void AudioManager::setVolume(float sfx, float bgm) {
    // Clamp volumes between 0.0 and 1.0 (0% to 100%)
    sfxVolume = std::clamp(sfx, 0.0f, 1.0f);
    bgmVolume = std::clamp(bgm, 0.0f, 1.0f);

    // Immediately apply to currently playing BGM
    if (currentBGM != nullptr) {
        SetMusicVolume(*currentBGM, bgmVolume);
    }
}

}
