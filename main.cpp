#include <deque>
#include <raylib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int TARGET_FPS = 60;

// Menu Constants
constexpr Color MENU_ITEM_BASE_COLOR = { 255, 255, 255, 15 };
constexpr Color MENU_ITEM_SELECTED_COLOR = { 255, 200, 0, 60 };
constexpr Color MENU_ITEM_BASE_TEXT_COLOR = RAYWHITE;
constexpr Color MENU_ITEM_SELECTED_TEXT_COLOR = GOLD;
constexpr int MENU_FONT_SIZE = 30;

// Grid Constants
constexpr int CELL_SIZE = 20;
constexpr int COLS = SCREEN_WIDTH / CELL_SIZE;
constexpr int ROWS = SCREEN_HEIGHT / CELL_SIZE;
constexpr Color BACKGROUND_COLOR = { 20, 24, 36, 255 };
constexpr Color ALTERNATE_COLOR = { 245, 245, 245, 20 };

// Step Constants (for snake movement speed)
constexpr float INITIAL_STEP_INTERVAL = 0.15f; // Time between drawing steps (higher = slower snake, lower = faster snake)
constexpr float FOOD_SPEED_BOOST = 0.005f; // Time subtracted from current step time after food is eaten (to make snake go slightly faster and make the game increasingly difficult)
constexpr float MIN_STEP_INTERVAL = 0.05f; // The minimum time between drawing steps (determines the snake's max speed)

enum class Screen { StartMenu, Instructions, Options, Gameplay, GameOver, QuitPrompt };
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

    /**
     * Get Rectangle representation of cell
     * @return cell as Rectangle
     */
    Rectangle ToRect() const {
        return {
            float(x * CELL_SIZE),
            float(y * CELL_SIZE),
            float(CELL_SIZE),
            float(CELL_SIZE)
        };
    }

    /**
     * Check if cell is within specified bounds
     * @return true if within bounds, false if not
     */
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

void DrawMenuItems(std::vector<MenuItem> &menuItems, int selectedMenuItem) {
    for (size_t i = 0; i < menuItems.size(); i++) {
        Rectangle r = menuItems[i].rect;
        bool isSelected = static_cast<int>(i) == selectedMenuItem;

        // Draw menu item container
        DrawRectangleRounded(r, 0.5f, 10,
            isSelected ? MENU_ITEM_SELECTED_COLOR : MENU_ITEM_BASE_COLOR);

        // Draw menu item text
        int textWidth = MeasureText(menuItems[i].label.c_str(), MENU_FONT_SIZE);
        DrawText(menuItems[i].label.c_str(),
            static_cast<int>(r.x + (r.width - textWidth) / 2),
            static_cast<int>(r.y + (r.height - MENU_FONT_SIZE) / 2),
            MENU_FONT_SIZE,
            isSelected ? MENU_ITEM_SELECTED_TEXT_COLOR : MENU_ITEM_BASE_TEXT_COLOR);
    }
}

class Snake {
public:
    // Constructor
    Snake() {
        // Snake should reset to default on init
        Reset();
    }

    /** Attributes **/
    Cell Head() const { return body.front(); }
    size_t Length() const { return body.size(); }

    /**
     * Method used to return Snake attributes to default
     */
    void Reset() {
        body.clear(); // Clear body deque
        turnQueue.clear(); // Clear any pending turns

        // By default, Snake will be moving right from the top left corner
        body.push_back({0, 0});
        currentDirection = Direction::Right;
        isGrowing = false;
    }

    /**
    * Queue a turn which is checked against the turnQueue to determine whether it's valid:
    *   - no reversing   - no same direction
     * @param newDirection - the next cell to move to
     */
    void QueueTurn(const Cell newDirection) {
        // Get last direction - if the queue is empty it's the current direction otherwise the last move
        const Cell lastTurn = turnQueue.empty() ? currentDirection : turnQueue.back();

        // The last direction is the same or rerverse of the current one, do nothing.
        if (newDirection == lastTurn || (newDirection.x == -lastTurn.x && newDirection.y == -lastTurn.y))
            return;

        // All max 3 turns in queue: if under, then add the new direction
        if (turnQueue.size() < 3)
            turnQueue.push_back(newDirection);
    }

    /**
     * Perform a movement based on turnQueue or current direction (if empty)
    *  Note: used this CodeLogic video to get a general sense of how Snake movement logic
    *  should work in a Snake game: https://www.youtube.com/watch?v=dZF0zseLcY0
     * @return false if there's a collision, true otherwise
     */
    bool Move() {
        // If the turnQueue is not empty, then set currentDirection to next queued direction and pop
        // (Otherwise, there's no queued turns and we simply move in the current direction
        if (!turnQueue.empty()) {
            currentDirection = turnQueue.front();
            turnQueue.pop_front();
        }

        // For movement, we determine what the new head will be
        const Cell nextHead = Head() + currentDirection;

        /********* Collision Checks *********/
        // Body Collision:
        // For this check, we need to know what the next body size will be
        // - Case 1: The snake has not eaten, so we pop the tail and use body size minus 1
        // - Case 2: The snake has eaten, so the tail is not popped and the full body size is used
        const size_t newBodySize = isGrowing ? body.size() : body.size() - 1;

        // Check new head position against body
        bool collidesBody = false;
        for (size_t i = 0; i < newBodySize; i++) {
            if (body[i] == nextHead) {
                collidesBody = true;
                break;
            }
        }

        // If the next head is not within bounds (collides with wall) or collides with the body, return false
        if (!nextHead.WithinBounds() || collidesBody)
            return false;

        // Otherwise, push next head to front
        body.push_front(nextHead);

        // And, pop tail if necessary
        if (isGrowing)
            isGrowing = false;
        else
            body.pop_back();

        return true; // return true for legal move
    }

    /**
     * Set isGrowing flag to true
     */
    void Grow() {
        isGrowing = true;
    }

    /**
     * Helper function that checks if the snake's body contains a cell.
     * Used to determine where to spawn food.
     * Note: written as const member function because we don't want to modify the snake:
     * https://www.learncpp.com/cpp-tutorial/const-class-objects-and-const-member-functions/
     * @param cell - the Cell to check
     * @return true if the snake occupies the cell, false otherwise.
     */
    bool Contains(Cell cell) const {
        // std::find "returns an iterator to the first element in the source range [first, last)
        // that satisfies specific criteria (or last if there is no such iterator)."
        // https://cppreference.com/cpp/algorithm/find
        return std::find(body.begin(), body.end(), cell) != body.end(); // If the find result is body.end(), the snake doesn't contain the cell!
    }

    // Draw the snake
    void Draw() const {
        for (size_t i = 0; i < body.size(); i++) {
            const Color color = i == 0 ? LIME : DARKGREEN;
            DrawRectangleRounded(body[i].ToRect(), 0.4f, 6, color);
        }
    }

private:
    std::deque<Cell> body; // Deque data structure used to track snake body
    std::deque<Cell> turnQueue; // Deque data structure used to queue snake turns (used to prevent collisions caused by reverse moment)
    Cell currentDirection; // The current direction of movement
    bool isGrowing = false; // Flag used to denote whether or not the snake is growing (has just eaten food)
};

class Food {
public:
    Cell Position() const {
        return position;
    }

    /**
     * Spawn food in new location
     * @param snake - Snake object by reference
     * @return boolean value indicating whether the spawn was successful
     */
    bool Spawn(const Snake &snake) {
        // Handle highly unlikely edge case: player has covered grid with snake body
        if (snake.Length() >= COLS * ROWS) {
            return false;
        }

        // Keep spawning until a valid location is found
        do {
            position = { GetRandomValue(0, COLS - 1), GetRandomValue(0, ROWS - 1) };
        } while (snake.Contains(position));
        return true;
    }

    void Draw() const {
        // Need to get centerpoint from rect to draw circle in correct location
        Rectangle r = position.ToRect();
        int centerX = static_cast<int>(r.x + r.width / 2); // r is float so need to cast to int
        int centerY = static_cast<int>(r.y + r.height / 2);
        DrawCircle(centerX, centerY, CELL_SIZE * 0.5f, MAROON);
    }

private:
    Cell position;
};

class Game {
public:
    Game() {
        NewGame(); // Set up new game on init
    }

    /**
     * Sets up new game by resetting fields to default
     */
    void NewGame() {
        snake.Reset(); // Reset snake position/size
        food.Spawn(snake); // Spawn food
        score = 0; // Reset score
        gameOver = false; // Reset gameOver flag
        stepInterval = INITIAL_STEP_INTERVAL; // Reset to initial step interval
        timeSinceStep = 0.0f; // Reset step time
    }

    /**
     * Queue snake turns from player input
     */
    void HandleInput() {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
            snake.QueueTurn(Direction::Up);
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
            snake.QueueTurn(Direction::Down);
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
            snake.QueueTurn(Direction::Left);
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
            snake.QueueTurn(Direction::Right);
    }

    /**
     * Apply game logic by step
     * @param dt - Frame time
     */
    void Update(float dt) {
        if (gameOver) return;

        timeSinceStep += dt; // Update step time
        while (timeSinceStep >= stepInterval && !gameOver) {
            timeSinceStep -= stepInterval; // Reset step time for next step
            Step();
        }
    }

    /**
     * Draw grid, food, snake, and score
     */
    void Draw() const {
        DrawGrid();
        snake.Draw();
        food.Draw();
        DrawCenteredText(TextFormat("Score: %d", score), 10, 20, { 255, 203, 0, 150 });

    }

    bool IsGameOver() const {
        return gameOver;
    }

    int Score() const {
        return score;
    }

private:
    Snake snake; // Default initialization
    Food food;  // Default initialization
    int score = 0;
    bool gameOver = false;
    float stepInterval = INITIAL_STEP_INTERVAL;
    float timeSinceStep = 0.0f;

    // From CodeLucky explanation:
    // - The game runs a timer
    // - 5-15 times every second the game updates = 0.05f - 0.15f --- smaller time = faster snake
    // - In every frame, the snake moves one step
    // https://www.youtube.com/watch?v=dZF0zseLcY0
    void Step() {
        // If the snake move results in a collision, end the game.
        if (!snake.Move()) {
            gameOver = true;
            return;
        }

        // If the snake head is at the food position:
        // - Set the isGrowing flag to true.
        // - Increment the score.
        // - Slightly increase the snake's speed by decreasing the step interval.
        // - Spawn new food - if, in the unlikely case there are no free cells left, end the game.
        //   - Add a million points to the score as the player has "beaten" the game.
        if (snake.Head() == food.Position()) {
            snake.Grow();
            score++;
            stepInterval = fmaxf(MIN_STEP_INTERVAL, stepInterval - FOOD_SPEED_BOOST);
            if (!food.Spawn(snake)) {
                score += 1000000;
                gameOver = true;
            }
        }

    }

    /**
     * Draws alternating squares against background to produce a checkerboard pattern
     */
    static void DrawGrid() {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                // Draw alternating squares to create checker effect against background
                if ((r + c) % 2 == 0) {
                    DrawRectangleRec(Cell{c, r}.ToRect(), ALTERNATE_COLOR);
                }
            }
        }
    }
};

int main() {
    const char *title = "Nibbler";

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // Override default
    Screen currentScreen = Screen::StartMenu; // Set start screen to Start Menu

    /*** Start Menu ***/
    // Create array to store start menu options
    std::vector<MenuItem> startMenuItems = {
    {"Play", {}},
    {"How to Play", {}},
    {"Options", {}},
    {"Quit", {}}
    };

    // Set rect values
    SetMenuItemRects(startMenuItems);
    int selectedStartMenuItem = 0; // By default, first item is selected.

    /*** Quit Prompt ***/
    // constexpr float quitItemWidth = 140.0f;
    // constexpr float quitItemHeight = 40.0f;
    // constexpr float quitItemStartX = (SCREEN_WIDTH - (2 * quitItemWidth)) / 2;
    // constexpr float quitItemStartY = 230.0f;
    // std::vector<MenuItem> quitPromptItems = {
    //     {"Yes", {quitItemStartX, quitItemStartY, quitItemWidth, quitItemHeight } },
    //     {"No", {quitItemStartX, quitItemStartY + quitItemHeight + 30.0f, quitItemWidth, quitItemHeight } }
    // };
    std::vector<MenuItem> quitPromptItems = {
        {"Yes", {} },
        {"No", {} }
    };
    SetMenuItemRects(quitPromptItems);
    int quitPromptSelectedItem = 1;

    /*** Game Vars ***/
    int highScore = 0; /***** placeholder *******/
    bool quitGame = false;
    Game game; // Init game

    while (!WindowShouldClose() && !quitGame) {
        const float dt = GetFrameTime(); // Passed to game

        // Update
        //----------------------------------------------------------------------------------
        // Conditionally handle updates by screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                // Allow Up navigations with Up or W keys
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    selectedStartMenuItem = (selectedStartMenuItem - 1 + static_cast<int>(startMenuItems.size())) % static_cast<int>(startMenuItems.size()); // Wraparound if before start

                // Allow Down navigations with Down or S keys
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    selectedStartMenuItem = (selectedStartMenuItem + 1) % static_cast<int>(startMenuItems.size()); // Wraparound if past end

                // Handle User selection
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    switch (selectedStartMenuItem) {
                        case 0: {
                            if (game.IsGameOver())
                                game.NewGame();
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
                            quitPromptSelectedItem = 1;
                            currentScreen = Screen::QuitPrompt;
                            break;
                        }
                    }
                }
            } break;

            case Screen::Gameplay: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                    currentScreen = Screen::StartMenu;
                    break;
                }

                // Handle input and update game
                game.HandleInput();
                game.Update(dt);

                // If game end, show Game Over screen
                if (game.IsGameOver()) {
                    if (game.Score() > highScore)
                        highScore = game.Score();

                    currentScreen = Screen::GameOver;
                }
            } break;

            case Screen::Instructions: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Options: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::GameOver: {
                if (IsKeyPressed(KEY_ENTER)) {
                    game.NewGame();
                    currentScreen = Screen::Gameplay;
                }
                if (IsKeyPressed(KEY_ESCAPE))
                    currentScreen = Screen::StartMenu;
                // if (IsKeyPressed(KEY_ESCAPE))
                //     quitGame = true;
            } break;

            case Screen::QuitPrompt: {
                // Allow Up navigations with Up or W keys
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                    quitPromptSelectedItem = (quitPromptSelectedItem - 1 + 2) % 2; // Wraparound

                // Allow Down navigations with Down or S keys
                if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                    quitPromptSelectedItem = (quitPromptSelectedItem + 1) % 2; // Wraparound

                // Handle User selection
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    if (quitPromptSelectedItem == 0)
                        quitGame = true;
                    else
                        currentScreen = Screen::StartMenu;
                }
            } break;
        }

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR); // Change background to dark blue

        // Conditionally render screen as seen in example: https://www.raylib.com/examples/core/loader.html?name=core_basic_screen_manager
        switch (currentScreen) {
            case Screen::StartMenu: {
                // Draw title and subtitle
                DrawCenteredText(title, 60, 60, SKYBLUE, TextEffect::Bob);
                DrawCenteredText("A snake game made with raylib", 130, 20, LIGHTGRAY);

                // Draw menu items
                DrawMenuItems(startMenuItems, selectedStartMenuItem);

                // Show flashing help text
                DrawCenteredText("Use Up/Down arrow keys or W/S to select - ENTER to select",
                    (SCREEN_HEIGHT - 30), 18, LIGHTGRAY, TextEffect::Blink);
            } break;

            case Screen::Instructions: {
                // Draw directions
                DrawText("Movement", 100, 20, 48,  GOLD);
                DrawText("Use arrow or WASD keys to navigate the snake", 100, 90, 20,  RAYWHITE);
                DrawText("Objectives", 100, 150, 48,  GOLD);
                DrawText("* Collect food to earn points", 100, 220, 20,  RAYWHITE);
                DrawText("* Do your best to avoid colliding with yourself or walls", 100, 260, 20,  RAYWHITE);

                // Draw flashing help text
                DrawCenteredText("Press ESC to return to Start Menu",
                    (SCREEN_HEIGHT - 30), 18,  LIGHTGRAY);
            } break;

            case Screen::Options: {

            } break;

            case Screen::Gameplay: {
                game.Draw();
            } break;

            case Screen::GameOver: {
                game.Draw(); // Draw the final game frame and overlay Game Over content
                DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5));
                DrawCenteredText("GAME OVER", 100, 64, RED);
                DrawCenteredText(TextFormat("Score: %d    High Score: %d", game.Score(), highScore), 184, 24, LIGHTGRAY);
                DrawCenteredText("Press Enter to Play Again", 260, 28, RAYWHITE, TextEffect::Blink);
                DrawCenteredText("Press Esc to Return to Start Menu", 305, 20, GRAY);
                //DrawCenteredText("Press Esc to Quit Game", 350, 20, GRAY);
            } break;

            case Screen::QuitPrompt: {
                DrawCenteredText("Quit Game?", 130, 48, GOLD);

                // Draw menu items
                DrawMenuItems(quitPromptItems, quitPromptSelectedItem);
            } break;
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}