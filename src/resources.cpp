#include "resources.h"
#include <utility>
#include <vector>

/**
 * Helper function that loads 128x128 key textures from paths. Must be called after InitWindow!
 * Note: textures sourced from https://kenney.nl/assets/input-prompts
 * @return map with loaded textures
 */
std::unordered_map<std::string, Texture2D> LoadKeyTextures() {
    std::unordered_map<std::string, std::string> texturePathMap = {
        { "Up", "resources/key_up.png" },
        { "Down", "resources/key_down.png" } ,
        { "Left", "resources/key_left.png" },
        { "Right", "resources/key_right.png" },
        { "W", "resources/key_w.png" },
        { "A", "resources/key_a.png" },
        { "S", "resources/key_s.png" },
        { "D", "resources/key_d.png" },
        { "Enter", "resources/key_enter.png" }
    };
    std::unordered_map<std::string, Texture2D> textures;
    for (const auto &[label, path] : texturePathMap) {
        const Texture2D t = LoadTexture(path.c_str());
        if (!IsTextureValid(t))
            continue; // If the texture couldn't be loaded, skip to next iteration (use DrawKeyBox fallback)
        textures[label] = t;
    }
    return textures;
}

/**
 * Cleanup helper function that unloads loaded key textures
 * @param keyTextures - reference to map containing key textures
 */
void UnloadKeyTextures(std::unordered_map<std::string, Texture2D> &keyTextures) {
    for (const auto &[label, texture] : keyTextures) {
        UnloadTexture(texture); // Unload texture to free memory
    }
    keyTextures.clear(); // clear map after finishing
}

/**
 * Helper function that loads sound effects Must be called after InitAudioDevice!
 * Note: sound effects sourced from https://ci.itch.io/arcade-sound-effects-pack
 * @return map with loaded sounds
 */
std::unordered_map<std::string, Sound> LoadSounds() {
    std::unordered_map<std::string, std::string> soundPathMap = {
        { "move", "resources/move.wav" },
        { "back", "resources/cancel.wav" } ,
        { "select", "resources/select.wav" } ,
        { "prompt", "resources/prompt.wav" },
        { "start", "resources/start.wav" } ,
        { "eat", "resources/collect.wav" },
        { "lose", "resources/lose.wav" },
        { "win", "resources/win.wav" },
    };
    std::unordered_map<std::string, Sound> sounds;
    for (const auto &[label, path] : soundPathMap) {
        const Sound s = LoadSound(path.c_str());
        if (!IsSoundValid(s))
            continue; // If the sound couldn't be loaded, move on
        sounds[label] = s;
    }
    return sounds;
}

/**
 * Cleanup helper function that unloads loaded Sounds
 * @param sounds - reference to map containing Sounds
 */
void UnloadSounds(std::unordered_map<std::string, Sound> &sounds) {
    for (const auto &[label, sound] : sounds) {
        UnloadSound(sound); // Unload sound to free memory
    }
    sounds.clear(); // clear map after finishing
}

/**
 * Helper function to look for and play sounds
 * @param sounds - a reference to sound map
 * @param name - reference to name of sound to play
 */
void PlayMapSound(const std::unordered_map<std::string, Sound> &sounds, const std::string &name) {
    const auto iterator = sounds.find(name);
    if (iterator != sounds.end())
        PlaySound(iterator->second);
}