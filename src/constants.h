#pragma once
#include "raylib.h"

// For sprint 2
constexpr bool SHOW_LIVE_SCORE = false;
constexpr bool ENABLE_GROWTH = false;

// Window Constants
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
constexpr int TARGET_FPS = 60;

// Menu Constants
constexpr Color MENU_ITEM_BASE_COLOR = { 255, 255, 255, 15 };
constexpr Color MENU_ITEM_SELECTED_COLOR = { 255, 200, 0, 60 };
constexpr Color MENU_ITEM_BASE_TEXT_COLOR = RAYWHITE;
constexpr Color MENU_ITEM_SELECTED_TEXT_COLOR = GOLD;
constexpr float MENU_FONT_SIZE = 30.0f;

// Text Constants
constexpr Color SCREEN_TITLE_COLOR = { 155, 227, 184, 255};
constexpr Color HEADING_COLOR = GOLD;
constexpr float HEADING_FONT_SIZE = 21.0f;
constexpr float BODY_FONT_SIZE = 17.0f;
constexpr int BODY_LINE_HEIGHT = 22;
constexpr float HEADING_SPACING = 1.2f;
constexpr float OPTIONS_HEADING_FONT_SIZE = 28.0f;

// Hint Bar Constants
constexpr int HINT_BAR_HEIGHT = 60;
constexpr int HINT_BAR_Y = SCREEN_HEIGHT - HINT_BAR_HEIGHT;

// Hint Line Constants
constexpr float KEY_HEIGHT = 36.0f; // Height of keyboard key
constexpr float KEY_PADDING = 8.0f; // Space between key label and its box
constexpr float HINT_GAP = 6.0f; // Space between hint line parts
constexpr float HINT_SPACER = 30.0f; // Space inserted for empty string part
constexpr float HINT_FONT_SIZE = 20.0f;
constexpr float HINT_SPACING = 0.5f; // For font spacing with custom font

// Grid Constants
constexpr int CELL_SIZE = 20;
constexpr int COLS = SCREEN_WIDTH / CELL_SIZE;
constexpr int ROWS = HINT_BAR_Y / CELL_SIZE; // exclude hint footer
constexpr int BOARD_X = 0; // The actual X coord where the board starts - includes just in case the design changes
constexpr int BOARD_Y = 0;
constexpr Color BACKGROUND_COLOR = { 20, 24, 36, 255 };
constexpr Color ALTERNATE_COLOR = { 245, 245, 245, 20 };

// Misc
constexpr Color QUIT_PROMPT_COLOR = { 24, 30, 40, 255 };

// Step Constants (for snake movement speed)
constexpr float INITIAL_STEP_INTERVAL = 0.15f; // Time between drawing steps (higher = slower snake, lower = faster snake)
constexpr float FOOD_SPEED_BOOST = 0.005f; // Time subtracted from current step time after food is eaten (to make snake go slightly faster and make the game increasingly difficult)
constexpr float MIN_STEP_INTERVAL = 0.05f; // The minimum time between drawing steps (determines the snake's max speed)

// Enums
enum class Screen { StartMenu, HowToPlay, Options, Gameplay, GameOver, QuitPrompt };
enum class TextEffect { None, Bob, Blink };
enum class GameEndType { None, WallCollision, SelfCollision, BoardFull };
enum class MoveResult { Ok, WallCollision, SelfCollision };
enum SpeedSetting { SLOW, NORMAL, FAST };
