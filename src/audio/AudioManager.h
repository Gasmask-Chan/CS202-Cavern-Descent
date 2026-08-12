#pragma once

#include <unordered_map>
#include <string>
#include "../Config.h" // Assumes raylib.h is included here

namespace Platformer {

class AudioManager {
private:
    std::unordered_map<std::string, Sound> sfxCache;
    std::unordered_map<std::string, Music> bgmCache;
    Music* currentBGM;
    std::string currentBGMName;
    float sfxVolume;
    float bgmVolume;

    // Private constructor/destructor for Singleton
    AudioManager();
    ~AudioManager();

public:
    static AudioManager* getInstance();

    // Prevent copying
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    /**
     * @brief Loads a sound file into the cache so it can be played quickly by name.
     */
    void loadSFX(const std::string& name, const std::string& filePath);

    /**
     * @brief Looks up name in sfxCache. If found, plays it. If not found, logs a warning.
     */
    void playSFX(const std::string& name);

    /**
     * @brief Loads a BGM file path into the mapping so it can be played by name.
     */
    void loadBGM(const std::string& name, const std::string& filePath);

    /**
     * @brief Stops current BGM, loads the new music file from the mapped path, and starts playing it.
     */
    void playBGM(const std::string& name);

    /**
     * @brief Stops the currently playing BGM stream.
     */
    void stopBGM();

    /**
     * @brief Must be called every frame to keep the BGM buffer fed.
     */
    void updateBGM();

    /**
     * @brief Clamps both values to [0.0, 1.0] and applies them to current audio streams.
     */
    void setVolume(float sfx, float bgm);
};

}
