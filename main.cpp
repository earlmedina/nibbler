#include <deque>
#include <raylib.h>
#include <string>
#include <vector>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int TARGET_FPS = 60;

// Grid Constants
constexpr int CELL_SIZE = 20;
constexpr int COLS = SCREEN_WIDTH / CELL_SIZE;
constexpr int ROWS = SCREEN_HEIGHT / CELL_SIZE;

enum class Screen { StartMenu, Instructions, Options, Gameplay, GameOver };
enum class TextEffect { None, Bob, Blink };

// Struct used for drawing menu items
struct MenuItem {
    std::string label;
    Rectangle rect;
};

// Struct used for grid cells
// Note: depends on CELL_SIZE, COLS, and ROWS constants
struct Cell {
    int x;
    int y;

    /*
     * Overload operators for simpler comparison:
     * https://www.learncpp.com/cpp-tutorial/overloading-operators-using-member-functions/
     */

    // Overload + to make it easier to find new snake position
    Cell operator+ (Cell other) const {
        return { x + other.x, y + other.y} ;
    }

    // Overload == to make cell comparison easier
    bool operator== (Cell other) const {
        return x == other.x && y == other.y;
    }

    // Overload != to make cell comparison easier
    bool operator!= (Cell other) const {
        return x != other.x || y != other.y;
    }

    // Get Rectangle representation of cell
    Rectangle ToRect() const {
        return {
            float(x * CELL_SIZE),
            float(y * CELL_SIZE),
            float(CELL_SIZE),
            float(CELL_SIZE)
        };
    }

    // Check if cell is within specified bounds
    bool WithinBounds() const {
        return x >= 0 && x < COLS && y >= 0 && y < ROWS;
    }
};

// Direction constants used to simplify turning logic
// Namespace used here as a static class would be used in other languages:
// https://www.learncpp.com/cpp-tutorial/user-defined-namespaces-and-the-scope-resolution-operator/
namespace Direction {
    constexpr Cell Up = { 0, -1 };
    constexpr Cell Down = { 0, 1 };
    constexpr Cell Left = { -1, 0 };
    constexpr Cell Right = { 1, 0 };
}

// Note: used this CodeLogic video to get a general sense of how Snake movement logic
// should work in a Snake game: https://www.youtube.com/watch?v=dZF0zseLcY0
class Snake {
public:
    // Constructor
    Snake() {
        // Snake should reset to default on init
        Reset();
    }

    // Method used to return Snake attributes to default
    void Reset() {
        body.clear(); // Clear body deque
        turnQueue.clear(); // Clear any pending turns

        // By default, Snake will be moving right from the top left corner
        body.push_back({0, 0});
        currentDirection = Direction::Right;
        canTurn = false;
        isGrowing = false;
    }

    // Queue a turn which is checked against the turnQueue to determine whether it's valid:
    // - no reversing   - no same direction
    void Turn(const Cell newDirection) {
        // Get last direction - if the queue is empty it's the current direction otherwise the last move
        const Cell lastTurn = turnQueue.empty() ? currentDirection : turnQueue.back();

        // The last direction is the same or rerverse of the current one, do nothing.
        if (newDirection == lastTurn || (currentDirection.x == -lastTurn.x && currentDirection.y == -lastTurn.y))
            return;

        // All max 3 turns in queue: if under, then add the new direction
        if (turnQueue.size() < 3)
            turnQueue.push_back(newDirection);
    }

private:
    std::deque<Cell> body; // Deque data structure used to track snake body
    std::deque<Cell> turnQueue; // Deque data structure used to queue snake turns (used to prevent collisions caused by reverse moment)
    Cell currentDirection; // The current direction of movement
    bool canTurn = false; // Whether or not the snake can turn (e.g., a reverse direction is forbidden)
    bool isGrowing = false; // Flag used to denote whether or not the snake is growing (has just eaten food)
};

/**
 *  * Helper function used to draw centered text
 * @param text      - Text to draw
 * @param y         - Y position on screen
 * @param fontSize  - Text font size
 * @param color     - Text color
 * @param effect    - TextEffect enum
 */
void DrawCenteredText(const char *text, int y, int fontSize, Color color, TextEffect effect = TextEffect::None) {
    int textWidth = MeasureText(text, fontSize);
    const auto t = static_cast<float>(GetTime()); // Needs to be cast to float for sinf
    switch (effect) {
        // Draw text using bobbing effect
        // Approach modified from: https://stackoverflow.com/questions/67322860/how-do-i-make-a-simple-idle-bobbing-motion-animation
        // The general idea is to apply a sine wave to the Y position to make it bob
        case TextEffect::Bob: {
            int transformation = static_cast<int>(sinf(t * 2.0f) * 6.0f);
            DrawText(text, (SCREEN_WIDTH - textWidth) / 2, y + transformation, fontSize, color);
        } break;
        case TextEffect::Blink: {
            // Show flashing/blinking text
            // Adapted from this libgdx example: https://gamedev.stackexchange.com/questions/150504/how-to-make-a-sprite-blink-with-libgdx
            // and using Fade function: https://www.raylib.com/cheatsheet/cheatsheet.html
            float alpha = (sinf(t * 1.7f) * 0.5f) + 0.5f;
            DrawText(text, (SCREEN_WIDTH - textWidth) / 2, y, fontSize, Fade(color, alpha));
        } break;
            // Intentional fallthrough
        case TextEffect::None:
        default:
            DrawText(text, (SCREEN_WIDTH - textWidth) / 2, y, fontSize, color);
            break;
    }
}

/**
 * Sets rectangle values on menu items
 * @param menuItems - reference to MenuItem array
 */
void SetMenuItemRects(std::vector<MenuItem> &menuItems) {
    const float itemWidth = 260.0f;
    const float itemHeight = 44.0f;
    for (size_t i = 0; i < menuItems.size(); i++ ) {
        menuItems[i].rect = { (SCREEN_WIDTH - itemWidth) / 2.0f, 190.0f + i * 55.0f, itemWidth, itemHeight };
    }
}


int main() {
    const char *title = "Nibbler";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // Override default
    Screen currentScreen = Screen::StartMenu; // Set start screen to Start Menu

    // Create array to store start menu options
    std::vector<MenuItem> menuItems = {
    {"Play", {}},
    {"How to Play", {}},
    {"Options", {}},
    {"Quit", {}}
    };

    // Set rect values
    SetMenuItemRects(menuItems);
    int selectedItem = 0; // By default, first item is selected.

    bool quitGame = false;
    while (!WindowShouldClose() && !quitGame) {
        // Update
        //----------------------------------------------------------------------------------
        // Conditionally handle updates by screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                // Allow Up navigations with Up or W keys
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    selectedItem = (selectedItem - 1 + static_cast<int>(menuItems.size())) % static_cast<int>(menuItems.size()); // Wraparound if before start

                // Allow Down navigations with Down or S keys
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    selectedItem = (selectedItem + 1) % static_cast<int>(menuItems.size()); // Wraparound if past end

                // Handle User selection
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    switch (selectedItem) {
                        case 0: {
                            currentScreen = Screen::Gameplay;
                            break;
                        }
                        case 1: {
                            currentScreen = Screen::Instructions;
                            break;
                        }
                        case 2: {
                            currentScreen = Screen::Options;
                            break;
                        }
                        case 3: {
                            quitGame = true;
                            break;
                        }
                    }
                }
            } break;

            case Screen::Gameplay: {
                // Placeholder
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Instructions: {
                // Placeholder
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Options: {
                // Placeholder
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;
            case Screen::GameOver: {
                if (IsKeyPressed(KEY_ESCAPE))
                    quitGame = true;
            } break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground({ 20, 24, 36, 255 }); // Change background to dark blue

        // Conditionally render screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                float t = static_cast<float>(GetTime());

                // Draw title and subtitle
                DrawCenteredText(title, 60, 60, SKYBLUE, TextEffect::Bob);
                DrawCenteredText("A snake game made with raylib", 130, 20, LIGHTGRAY);

                // Draw menu items
                Color baseMenuItemColor = { 255, 255, 255, 15 };
                Color selectedMenuItemColor = { 255, 200, 0, 60 };

                for (size_t i = 0; i < menuItems.size(); i++) {
                    constexpr int menuFontSize = 30;
                    Rectangle r = menuItems[i].rect;
                    bool isSelected = static_cast<int>(i) == selectedItem;

                    // Draw menu item container
                    DrawRectangleRounded(r, 0.5f, 10,
                        isSelected ? selectedMenuItemColor : baseMenuItemColor);

                    // Draw menu item text
                    int textWidth = MeasureText(menuItems[i].label.c_str(), menuFontSize);
                    DrawText(menuItems[i].label.c_str(),
                        static_cast<int>(r.x + (r.width - textWidth) / 2),
                        static_cast<int>(r.y + (r.height - menuFontSize) / 2),
                        menuFontSize,
                        isSelected ? GOLD : RAYWHITE);
                }

                // Show flashing help text
                DrawCenteredText("Use Up/Down arrow keys or W/S to select - ENTER to select",
                    (SCREEN_HEIGHT - 30), 18, LIGHTGRAY, TextEffect::Blink);
            } break;

            case Screen::Instructions: {
                // Draw directions
                DrawText("Movement", 100, 20, 50,  GOLD);
                DrawText("Use arrow or WASD keys to navigate the snake", 100, 90, 20,  RAYWHITE);
                DrawText("Objectives", 100, 150, 50,  GOLD);
                DrawText("* Collect food to earn points", 100, 220, 20,  RAYWHITE);
                DrawText("* Do your best to avoid colliding with yourself or walls", 100, 260, 20,  RAYWHITE);

                // Draw flashing help text
                DrawCenteredText("Press ESC to return to Start Menu",
                    (SCREEN_HEIGHT - 30), 18,  LIGHTGRAY);
            } break;

            case Screen::Options: {

            } break;

            case Screen::Gameplay: {

            } break;

            case Screen::GameOver: {

            } break;
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}