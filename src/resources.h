#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"

std::unordered_map<std::string, Texture2D> LoadKeyTextures();
void UnloadKeyTextures(std::unordered_map<std::string, Texture2D> &keyTextures);
std::unordered_map<std::string, Sound> LoadSounds();
void UnloadSounds(std::unordered_map<std::string, Sound> &sounds);
void PlayMapSound(const std::unordered_map<std::string, Sound> &sounds, const std::string &name);