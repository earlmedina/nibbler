#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <initializer_list>

#include "constants.h"
#include "raylib.h"

// Struct used for drawing menu items
struct MenuItem {
    std::string label;
    Rectangle rect;
    bool isBack = false; // Used to determine if "back" sound should play
};

// Struct used to model hint content part
struct HintPart {
    std::string text;
    bool isKey; // whether content is a key
};

void DrawCenteredText(const char *text, int y, int fontSize, Color color, TextEffect effect = TextEffect::None);
void DrawTextLines(int x, int y, std::initializer_list<const char *> lines, int fontSize = BODY_FONT_SIZE,
    float lineHeight = BODY_LINE_HEIGHT, Color color = RAYWHITE);

void DrawTextLinesEx(float x, float y, std::initializer_list<const char *> lines, const Font &font, float fontSize = BODY_FONT_SIZE,
    float lineHeight = BODY_LINE_HEIGHT, Color color = RAYWHITE);

void LayoutMenu(std::vector<MenuItem> &menuItems, float startY, float itemWidth, float itemHeight, float itemSpacing);
void DrawMenuItems(const std::vector<MenuItem> &menuItems, int selectedMenuItem);
bool IsArrowKey(const std::string &keyLabel);
float GetKeyBoxWidth(const std::string &keyLabel);
float GetKeyWidth(const std::string &keyLabel, const std::unordered_map<std::string, Texture2D> &keyTextures);
float GetHintPartWidth(const HintPart &part, const std::unordered_map<std::string, Texture2D> &keyTextures, const Font &font);
float MeasureHintLine(const std::vector<HintPart> &parts, const std::unordered_map<std::string, Texture2D> &keyTextures, const Font &font);
float DrawKeyBox(const std::string &keyLabel, float x, float y);
float DrawKey(const std::unordered_map<std::string, Texture2D> &keyTextures, const std::string &keyLabel, float x, float y);
float DrawHintLine(const std::vector<HintPart> &parts, float x, float y, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font);
void DrawHintBar(const std::vector<HintPart> &hintLine, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font);
void DrawStartMenuScreen(const char *title, const char *subTitle, const std::vector<MenuItem> &startMenuItems,
    const int selectedStartMenuItem, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font);
void DrawHowToPlayContent(Font font);

void HandleKeyboardNavigation(int &selectedItem, int itemCount);

void HandleMouseNavigation(const std::vector<MenuItem> &menuItems, bool &clicked, int &selectedItem);

bool HandleMenuSelection(const std::vector<MenuItem> &menuItems, int &selectedMenuItem, std::unordered_map<std::string, Sound> &sounds);

bool UpdateRowFocus(int &focusedRowIdx, const std::vector<Rectangle> &rowRects);
/**** Hint Lines ****/
inline const std::vector<HintPart> MENU_HINT_LINE = {
    { "Move:", false }, { "Up", true }, { "Down", true }, { "or", false },
    { "W", true },{ "S", true },  {"", false},
    {"Select:", false}, { "Enter", true }, { "or", false }, { "Click", false }
};

inline const std::vector<HintPart> HOW_TO_PLAY_HINT_LINE = {
    { "Enter", true }, { "or", false }, { "Click: ", false },
    { "Back to Start Menu", false }
};

inline const std::vector<HintPart> GAMEPLAY_HINT_LINE = {
    { "Steer:", false }, { "Up", true }, { "Left", true }, { "Down", true }, { "Right", true },
    { "or", false }, { "W", true }, { "A", true }, { "S", true }, { "D", true },
    { " | Hitting a wall ends the game!", false }
};
