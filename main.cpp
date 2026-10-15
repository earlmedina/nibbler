#include <deque>
#include <raylib.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

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
constexpr int MENU_FONT_SIZE = 30;

// Text Constants
constexpr Color SCREEN_TITLE_COLOR = { 155, 227, 184, 255};
constexpr Color HEADING_COLOR = GOLD;
constexpr int HEADING_FONT_SIZE = 21.0f;
constexpr float BODY_FONT_SIZE = 17.0f;
constexpr int BODY_LINE_HEIGHT = 22;
constexpr float HEADING_SPACING = 1.2f;

// Hint Bar Constants
constexpr int HINT_BAR_HEIGHT = 60;
constexpr int HINT_BAR_Y = SCREEN_HEIGHT - HINT_BAR_HEIGHT;

// Hint Line Constants
constexpr float KEY_HEIGHT = 36.0f; // Height of keyboard key
constexpr float KEY_PADDING = 8.0f; // Space between key label and its box
constexpr float HINT_GAP = 6.0f; // Space between hint line parts
constexpr float HINT_SPACER = 30.0f; // Space inserted for empty string part
constexpr float HINT_FONT_SIZE = 20.0f;

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

enum class Screen { StartMenu, HowToPlay, Options, Gameplay, GameOver, QuitPrompt };
enum class TextEffect { None, Bob, Blink };
enum class GameEndType { None, WallCollision, SelfCollision, BoardFull };
enum class MoveResult { Ok, WallCollision, SelfCollision };

// Struct used for drawing menu items
struct MenuItem {
    std::string label;
    Rectangle rect;
};

// Struct used to model hint content part
struct HintPart {
    std::string text;
    bool isKey; // whether content is a key
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
            float(BOARD_X + x * CELL_SIZE),
            float(BOARD_Y + y * CELL_SIZE),
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

/**** Hint Lines ****/
const std::vector<HintPart> MENU_HINT_LINE = {
    { "Move: ", false }, { "Up", true }, { "Down", true }, { " or ", false },
    { "W", true },{ "S", true },  {"", false},
    {"Select: ", false}, { "Enter", true }, { " or ", false }, { "Click", false }
};

const std::vector<HintPart> HOW_TO_PLAY_HINT_LINE = {
    { "Enter", true }, { "or", false }, { "Click", false },
    { "Back to Start Menu", false }
};

const std::vector<HintPart> GAMEPLAY_HINT_LINE = {
    { "Steer: ", false }, { "Up", true }, { "Left", true }, { "Down", true }, { "Right", true },
    { " or ", false }, { "W", true }, { "A", true }, { "S", true }, { "D", true },
    {"", false },
    { "Hitting a wall ends the game!" }
};

/**
 * Helper function that loads 128x128 key textures from paths
 * Note: must be called after InitWindow!
 * @return map with loaded textures
 */
std::unordered_map<std::string, Texture2D> LoadKeyTextures() {
    std::unordered_map<const char *, const char *> texturePathMap = {
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
        const Texture2D t = LoadTexture(path);
        if (t.id == 0)
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
 * Helper function that draws several lines of text at once
 * @param x x-coord
 * @param y y-coord
 * @param lines lines of text as initializer list: https://stackoverflow.com/questions/27753420/initializer-list-vs-vector
 * @param fontSize font size
 * @param lineHeight line height (space between lines)
 * @param color text color
 */
void DrawTextLines(int x, int y, std::initializer_list<const char *> lines, int fontSize = BODY_FONT_SIZE,
    int lineHeight = BODY_LINE_HEIGHT, Color color = RAYWHITE) {

    for (const char *line : lines) {
        DrawText(line, x, y, fontSize, color);
        y += lineHeight;
    }
}

void DrawTextLinesEx(float x, float y, std::initializer_list<const char *> lines, const Font &font, float fontSize = BODY_FONT_SIZE,
    float lineHeight = BODY_LINE_HEIGHT, Color color = RAYWHITE) {

    for (const char *line : lines) {
        //DrawText(line, x, y, fontSize, color);
        DrawTextEx(font, line, (Vector2){ roundf(x), roundf(y) }, fontSize, 0.5, color); // Use roundf because otherwise it can look blurry
        y += lineHeight;
    }
}

/**
 * Prepares centered menu layout by setting rectangle values on menu items
 * @param menuItems reference to MenuItem array
 * @param startY Y position of top left corner of menu
 * @param itemWidth width of menu items
 * @param itemHeight height of menu items
 * @param itemSpacing distance between menu items
 */
void LayoutMenu(std::vector<MenuItem> &menuItems, float startY, float itemWidth, float itemHeight, float itemSpacing) {
    for (size_t i = 0; i < menuItems.size(); i++ ) {
        menuItems[i].rect = { (SCREEN_WIDTH - itemWidth) / 2.0f, startY + i * itemSpacing, itemWidth, itemHeight };
    }
}

void DrawMenuItems(const std::vector<MenuItem> &menuItems, int selectedMenuItem) {
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

// Helper function to determine if a key label is an arrow key
bool IsArrowKey(const std::string &keyLabel) {
    return keyLabel == "Up" || keyLabel == "Down" || keyLabel == "Left" || keyLabel == "Right";
}

// Get Width of KeyBox drawn from text
float GetKeyBoxWidth(const std::string &keyLabel) {
    // Arrow and single char labels should be square
    if (IsArrowKey(keyLabel) || keyLabel.size() == 1)
        return KEY_HEIGHT;
    // Otherwise, measure text and account for x padding on both sides
    return static_cast<float>(MeasureText(keyLabel.c_str(), HINT_FONT_SIZE)) + 2 * KEY_PADDING;
}

// Get width of Key drawn from texture. Useful for case where Enter texture wasn't loaded.
float GetKeyWidth(const std::string &keyLabel, const std::unordered_map<std::string, Texture2D> &keyTextures) {
    // If texture was loaded, return key height
    if (keyTextures.contains(keyLabel))
        return KEY_HEIGHT;
    // Otherwise, the key is drawn from text and we need the key box width
    return GetKeyBoxWidth(keyLabel);
}

float GetHintPartWidth(const HintPart &part, const std::unordered_map<std::string, Texture2D> &keyTextures) {
    // For empty strings, return spacer.
    if (part.text.empty())
        return HINT_SPACER;

    // For keys, return the calculated key box width
    if (part.isKey)
        return GetKeyWidth(part.text, keyTextures);

    // Otherwise, it's plain text so return MeasureText result
    return static_cast<float>(MeasureText(part.text.c_str(), HINT_FONT_SIZE));
}

float MeasureHintLine(const std::vector<HintPart> &parts, const std::unordered_map<std::string, Texture2D> &keyTextures) {
    float totalWidth = 0.0f;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0)
            totalWidth += HINT_GAP; // Account for space between hint parts
        totalWidth += GetHintPartWidth(parts[i], keyTextures);
    }
    return totalWidth;
}

/**
 * Draws a key label inside a box so that it looks like a keyboard key
 * @param keyLabel - key text
 * @param x - left edge of the key box
 * @param y - top of the key label
 * @return The x position of the key's right edge
 */
float DrawKeyBox(const std::string &keyLabel, float x, float y) {
    // Draw key box
    const Rectangle box = { x, y - KEY_PADDING, GetKeyBoxWidth(keyLabel), KEY_HEIGHT };
    DrawRectangleRounded(box, 0.3f, 6, MENU_ITEM_BASE_COLOR);
    DrawRectangleLinesEx(box, 1.0f, Fade(RAYWHITE, 0.35f));

    // If it's an arrow key, draw key label as triangle
    if (IsArrowKey(keyLabel)) {
        const float cx = box.x + box.width / 2.0f; // Calculate center x of box
        const float cy = box.y + box.height / 2.0f; // Calculate center y of box
        const float r = box.height * 0.2f; // Triangle coords are based on center, so this is the triangle "radius".

        // Draw arrows as triangles
        // Note: DrawTriangle points are in CCW order: https://github.com/raysan5/raylib/issues/941

        if (keyLabel == "Up")
            DrawTriangle({ cx, cy - r }, { cx - r, cy + r },
        { cx + r, cy + r }, RAYWHITE);


        else if (keyLabel == "Down")
            DrawTriangle({ cx, cy + r }, { cx + r, cy - r },
        { cx - r, cy - r }, RAYWHITE);
        else if (keyLabel == "Left")
            DrawTriangle({ cx - r, cy }, { cx + r, cy + r },
        { cx + r, cy - r }, RAYWHITE);
        else
            DrawTriangle({ cx + r, cy }, { cx - r, cy - r },
        { cx - r, cy + r }, RAYWHITE);
        return x + box.width; // Return "end" of keybox to know where to start next part
    }

    // Draw key label centered in the box
    const float keyLabelWidth = static_cast<float>(MeasureText(keyLabel.c_str(), HINT_FONT_SIZE));
    DrawText(keyLabel.c_str(), static_cast<int>(x + (box.width - keyLabelWidth) / 2.0f),
        static_cast<int>(y), HINT_FONT_SIZE, RAYWHITE);

    return x + box.width; // Return "end" of keybox to know where to start next part
}

float DrawKey(const std::unordered_map<std::string, Texture2D> &keyTextures, const std::string &keyLabel, float x, float y) {
    // Try to get texture from map
    const auto iterator = keyTextures.find(keyLabel);

    // Fallback: if the texture couldn't be loaded, draw the key using DrawKeyBox
    if (iterator == keyTextures.end())
        return DrawKeyBox(keyLabel, x, y);

    // Otherwise, get the texture ref
    const Texture2D &texture = iterator->second; // Note: second is Texture2D

    // Get the source rectangle
    const Rectangle sourceRect = { 0.0f, 0.0f,
        static_cast<float>(texture.width), static_cast<float>(texture.height)  };

    // Define destination rect where we want to draw the texture
    // Notes:
    //   - Use roundf here to avoid blurriness
    //   - Subtract key padding since we're using the whole texture
    const Rectangle destRect = { roundf(x), roundf(y - KEY_PADDING), KEY_HEIGHT, KEY_HEIGHT };

    // Draw scaled key texture
    DrawTexturePro(texture, sourceRect, destRect, { 0.0f, 0.0f }, 0.0f, WHITE);
    return x + KEY_HEIGHT; // Return "end" of key txcture to know where to start next part
}

/**
 * Takes an array of hint parts and draws them at the specific top left position
 * @param parts - array of hint parts
 * @param x  - x position where line should start
 * @param y  - y position where line should start
 * @return - x position where the line ends
 */
float DrawHintLine(const std::vector<HintPart> &parts, float x, float y, std::unordered_map<std::string, Texture2D> &keyTextures) {
    for (size_t i = 0; i < parts.size(); i++) {
        const HintPart &part = parts[i]; // reference part

        // Add space between parts
        if (i > 0)
            x += HINT_GAP;

        if (!part.text.empty()) {
            if (part.isKey)
                DrawKey(keyTextures, part.text, x, y);
            else
                DrawText(part.text.c_str(), static_cast<int>(x), static_cast<int>(y), HINT_FONT_SIZE, LIGHTGRAY);
        }
        x += GetHintPartWidth(part, keyTextures);
    }
    return x;
}

/**
 * Helper function to draw the hint bar footer shown on screens
 * @param hintLine - array of hint parts representing a line to draw
 */
void DrawHintBar(const std::vector<HintPart> &hintLine, std::unordered_map<std::string, Texture2D> &keyTextures) {
    // Draw to separate hint bar from play grid
    DrawLine(0, HINT_BAR_Y, SCREEN_WIDTH, HINT_BAR_Y, Fade(RAYWHITE, 0.5));
    const float x = (SCREEN_WIDTH - MeasureHintLine(hintLine, keyTextures)) / 2.0f; // Figure out where line should start to be x-centered
    const float y = HINT_BAR_Y + (HINT_BAR_HEIGHT - HINT_FONT_SIZE) / 2.0f; // Figure out where line should start to be y-centered
    DrawHintLine(hintLine, x, y, keyTextures);
}

/**
 * Helper function to draw start menu screen. Used for dimming effect when drawing quit prompt.
 * @param title - title text
 * @param subTitle - subtitle text
 * @param startMenuItems - MenuItem array
 * @param selectedStartMenuItem - selected start menu item index
 */
void DrawStartMenuScreen(const char *title, const char *subTitle, const std::vector<MenuItem> &startMenuItems,
    const int selectedStartMenuItem, std::unordered_map<std::string, Texture2D> &keyTextures) {
    // Draw title and subtitle
    DrawCenteredText(title, 60, 60, SCREEN_TITLE_COLOR, TextEffect::Bob);
    DrawCenteredText(subTitle, 130, 20, LIGHTGRAY);

    // Draw menu items
    DrawMenuItems(startMenuItems, selectedStartMenuItem);

    // Show Hint Bar
    DrawHintBar(MENU_HINT_LINE, keyTextures);
}

/**
 * Helper function to draw info text shown on How To screen
 */
void DrawHowToPlayContent(Font font) {
    DrawCenteredText("How to Play", 20, 40, SCREEN_TITLE_COLOR);
    // Left column content
    constexpr  int leftX = 40;
    //DrawText("GOAL", leftX, 84, HEADING_FONT_SIZE, HEADING_COLOR);
    DrawTextEx(font, "GOAL", { leftX, 84 }, HEADING_FONT_SIZE, HEADING_SPACING, HEADING_COLOR);
    DrawTextLinesEx(leftX, 110, {
        "Steer the snake to food (red circles).",
        "Each piece you eat earns 1 point. How",
        "high can you score?"
    }, font);
    DrawTextEx(font, "CONTROLS", { leftX, 190 }, HEADING_FONT_SIZE, HEADING_SPACING, HEADING_COLOR);
    DrawTextLinesEx(leftX, 216, {
        "While playing, steer with arrow keys",
        "or WASD. To navigate menus, use the",
        "same keys and select with Enter, or",
        "just click an option. If one way does",
        "not work, try the other."
    }, font);

    DrawTextEx(font, "WATCH OUT", { leftX, 340 }, HEADING_FONT_SIZE, HEADING_SPACING, HEADING_COLOR);
    DrawTextLinesEx(leftX, 366, {
        "Hitting a wall ends the game, so be",
        "careful when you're close to the edge",
        "of the window!"
    }, font);

    // Right column content
    constexpr int rightX = 420;
    DrawTextEx(font, "STEPS", { rightX, 84 }, HEADING_FONT_SIZE, HEADING_SPACING, HEADING_COLOR);
    DrawTextLinesEx(rightX, 110, {
        "1. Select Play (press Enter or click).",
        "2. Press an arrow key or WASD to start.",
        "   The snake won't move until you do!",
        "3. Steer to the food and avoid the",
        "   walls.",
        "4. When the game ends, choose ",
        "   Play Again or Start Menu."
    }, font);

    DrawTextEx(font, "NEED TO BACK OUT OF A SCREEN?", { rightX, 280 }, HEADING_FONT_SIZE, HEADING_SPACING, HEADING_COLOR);
    DrawTextLinesEx(rightX, 306, {
        "You can always go back: Select Back on",
        "this screen, No on the Quit prompt, and",
        "Start Menu after a game."
    }, font);

}

/**
 * Helper function used to handle menu navigation via keyboard
 * @param selectedItem reference to item index
 * @param itemCount the count of menu items
 */
void HandleKeyboardNavigation(int &selectedItem, int itemCount) {
    // Allow Up navigations with Up or W keys
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        selectedItem = (selectedItem - 1 + itemCount) % itemCount; // Wraparound if before start

    // Allow Down navigations with Down or S keys
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        selectedItem = (selectedItem + 1) % itemCount; // Wraparound if past end
}

/**
 * Helper function used to handle menu navigation via mouse
 * @param menuItems reference to menuItems array
 * @param clicked reference to bool click variable
 * @param selectedItem reference to item index
 */
void HandleMouseNavigation(const std::vector<MenuItem> &menuItems, bool &clicked, int &selectedItem) {
    const Vector2 mousePos = GetMousePosition();

    // Check if mouse has moved to determine focus
    const Vector2 mouseDelta = GetMouseDelta();
    const bool mouseMoved = mouseDelta.x != 0.0f || mouseDelta.y != 0.0f;

    // Check if any of the menu items was clicked
    const bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    for (size_t i = 0; i < menuItems.size(); i++) {
        if (CheckCollisionPointRec(mousePos, menuItems[i].rect)) {
            if (mouseMoved || mousePressed)
                selectedItem = static_cast<int>(i);
            if (mousePressed)
                clicked = true;
        }
    }
}

/**
 * Helper function that handles menu selection
 * @param menuItems reference MenuItem array
 * @param selectedMenuItem reference to selected menu item index
 * @return bool indicated whether a highlighted menu item was clicked or Enter was pressed
 */
bool HandleMenuSelection(const std::vector<MenuItem> &menuItems, int &selectedMenuItem) {
    HandleKeyboardNavigation(selectedMenuItem, static_cast<int>(menuItems.size()));

    bool clicked = false;
    HandleMouseNavigation(menuItems, clicked, selectedMenuItem);
    return clicked || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER); // Include keypad enter
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

        // Set snake start position
        const int startHeadX = COLS / 2 - 3;
        const int startHeadY = ROWS / 2;
        body.push_back({startHeadX, startHeadY});
        // Make the snake longer for sprint 1 demonstration purposes
        if (!ENABLE_GROWTH) {
            body.push_back({startHeadX - 1, startHeadY});
            body.push_back({startHeadX - 2, startHeadY});
        }
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
     * @return MoveResult enum value
     */
    MoveResult Move() {
        // If the turnQueue is not empty, then set currentDirection to next queued direction and pop
        // (Otherwise, there's no queued turns and we simply move in the current direction
        if (!turnQueue.empty()) {
            currentDirection = turnQueue.front();
            turnQueue.pop_front();
        }

        // For movement, we determine what the new head will be
        const Cell nextHead = Head() + currentDirection;

        // If the next head is not within bounds (collides with wall) return WallCollision result
        if (!nextHead.WithinBounds())
            return MoveResult::WallCollision;

        /********* Collision Checks *********/
        // Body Collision:
        // For this check, we need to know what the next body size will be
        // - Case 1: The snake has not eaten, so we pop the tail and use body size minus 1
        // - Case 2: The snake has eaten, so the tail is not popped and the full body size is used
        const size_t newBodySize = isGrowing ? body.size() : body.size() - 1;

        // Check new head position against body
        for (size_t i = 0; i < newBodySize; i++) {
            if (body[i] == nextHead) {
                return MoveResult::SelfCollision; // Return SelfCollision if snake self collides
            }
        }

        // Otherwise, push next head to front
        body.push_front(nextHead);

        // And, pop tail if necessary
        if (isGrowing)
            isGrowing = false;
        else
            body.pop_back();

        return MoveResult::Ok; // return Ok for collisionless move
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
        gameStarted = false; // Reset gameStarted flag
        gameOver = false; // Reset gameOver flag
        gameEndType = GameEndType::None; // Reset game end type
        stepInterval = INITIAL_STEP_INTERVAL; // Reset to initial step interval
        timeSinceStep = 0.0f; // Reset step time
    }

    /**
     * Queue snake turns from player input
     */
    void HandleInput() {
        bool directionKeyPressed = false;
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            snake.QueueTurn(Direction::Up);
            directionKeyPressed = true;
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            snake.QueueTurn(Direction::Down);
            directionKeyPressed = true;
        }
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            snake.QueueTurn(Direction::Left);
            directionKeyPressed = true;
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            snake.QueueTurn(Direction::Right);
            directionKeyPressed = true;
        }
        if (directionKeyPressed && !gameStarted) {
            gameStarted = true;
            timeSinceStep = 0.0f;
        }
    }

    /**
     * Apply game logic by step
     * @param dt - Frame time
     */
    void Update(float dt) {
        if (gameOver || !gameStarted) return;

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
        if (SHOW_LIVE_SCORE)
            DrawCenteredText(TextFormat("Score: %d", score), 10, 20, { 255, 203, 0, 150 });

        // Draw start text
        if (!gameStarted) {
            const int startPromptFontSize = 20;
            const char *startPromptText = "Press an arrow key or WASD to start";

            const float startY = 380.0f;
            const float startPromptHeight = 60.0f;
            const float startPromptWidth = 40 + MeasureText(startPromptText, startPromptFontSize);;

            const Rectangle startPromptRect = { (SCREEN_WIDTH - startPromptWidth) / 2,
                startY, startPromptWidth, startPromptHeight };
            DrawRectangleRec(startPromptRect, Fade(BLACK, 0.6f));
            DrawRectangleLinesEx(startPromptRect, 1.0f, GOLD);

            const int textStartY = startY + (startPromptHeight - startPromptFontSize) / 2;
            DrawCenteredText(startPromptText, textStartY, startPromptFontSize, RAYWHITE);
        }
    }

    bool IsGameOver() const {
        return gameOver;
    }

    int Score() const {
        return score;
    }

    const char *GetGameEndMessage() const {
        switch (gameEndType) {
            case GameEndType::BoardFull: return "Congratulations! You filled the board!";
            case GameEndType::SelfCollision: return "You ran into yourself!";
            case GameEndType::WallCollision: return "You hit a wall!";
            case GameEndType::None: // Intentional fallthrough
            default: return ""; // Print nothing
        }
    }

private:
    Snake snake; // Default initialization
    Food food;  // Default initialization
    int score = 0;
    bool gameStarted = false;
    bool gameOver = false;
    GameEndType gameEndType = GameEndType::None;
    float stepInterval = INITIAL_STEP_INTERVAL;
    float timeSinceStep = 0.0f;

    // From CodeLucky explanation:
    // - The game runs a timer
    // - 5-15 times every second the game updates = 0.05f - 0.15f --- smaller time = faster snake
    // - In every frame, the snake moves one step
    // https://www.youtube.com/watch?v=dZF0zseLcY0
    void Step() {
        // If the snake move results in a collision, end the game.
        MoveResult snakeMoveResult = snake.Move();
        if (snakeMoveResult != MoveResult::Ok) {
            gameEndType = snakeMoveResult == MoveResult::SelfCollision
                ? GameEndType::SelfCollision
                : GameEndType::WallCollision;
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
            if (ENABLE_GROWTH)
                snake.Grow();
            score++;
            stepInterval = fmaxf(MIN_STEP_INTERVAL, stepInterval - FOOD_SPEED_BOOST);
            if (!food.Spawn(snake)) {
                score += 1000000;
                gameEndType = GameEndType::BoardFull;
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
    const char *subTitle = "A quick, fun classic arcade game";
    SetConfigFlags(FLAG_WINDOW_HIGHDPI); // Looks blurry on dev machine otherwise
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    ChangeDirectory(GetApplicationDirectory()); // For proper font loading
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL); // Override default

    // Load how to font
    int howToFontSize = static_cast<int>(roundf(32 * GetWindowScaleDPI().x)); // For better scaling
    Font howToFont = LoadFontEx("resources/Menlo-Regular.ttf", howToFontSize, nullptr, 0);
    GenTextureMipmaps(&howToFont.texture);
    SetTextureFilter(howToFont.texture, TEXTURE_FILTER_POINT);

    // Load key textures
    std::unordered_map<std::string, Texture2D> keyTextures = LoadKeyTextures();

    Screen currentScreen = Screen::StartMenu; // Set start screen to Start Menu

    /************ MENU SETUP ************/
    // Create array to store menu options and then set placement values

    /*** Start Menu ***/
    std::vector<MenuItem> startMenuItems = {
    { "Play", {} },
    { "How to Play", {} },
    //{ "Options", {} },
    { "Quit", {} }
    };

    // Set placement
    LayoutMenu(startMenuItems, 220.0f, 320.0f, 56.0f, 72.0f);
    int selectedStartMenuItem = 0; // By default, first item (Play) is selected.

    /*** How to Play ***/
    std::vector<MenuItem> howToPlayItems = {
    {"Back", {} },
    };

    LayoutMenu(howToPlayItems, 470.0f, 200.0f, 52.0f, 0.0f);
    int selectedHowToPlayItem = 0;

    /*** Game Over ***/
    std::vector<MenuItem> gameOverItems = {
        {"Play Again", {} },
        {"Start Menu", {} },
    };

    LayoutMenu(gameOverItems, 350.0f, 320.0f, 56.0f, 72.0f);
    int selectedGameOverItem = 0;

    /*** Quit Prompt ***/
    std::vector<MenuItem> quitPromptItems = {
        {"Yes", {} },
        {"No", {} }
    };
    LayoutMenu(quitPromptItems, 270.0f, 240.0f, 52.0f, 68.0f);
    int selectedQuitPromptItem = 1;

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
                // Handle User selection
                if (HandleMenuSelection(startMenuItems, selectedStartMenuItem)) {
                    switch (selectedStartMenuItem) {
                        case 0: {
                            if (game.IsGameOver())
                                game.NewGame();
                            currentScreen = Screen::Gameplay;
                            break;
                        }
                        case 1: {
                            currentScreen = Screen::HowToPlay;
                            break;
                        }
                        // case 2: {
                        //     currentScreen = Screen::Options;
                        //     break;
                        // }
                        case 2: {
                            selectedQuitPromptItem = 1;
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

            case Screen::HowToPlay: {
                if (HandleMenuSelection(howToPlayItems, selectedHowToPlayItem))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::Options: {
                if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
                    currentScreen = Screen::StartMenu;
            } break;

            case Screen::GameOver: {
                if (HandleMenuSelection(gameOverItems, selectedGameOverItem)) {
                    if (selectedGameOverItem == 0) {
                        game.NewGame();
                        currentScreen = Screen::Gameplay;
                    } else {
                        selectedGameOverItem = 0;
                        currentScreen = Screen::StartMenu;
                    }
                }
            } break;

            case Screen::QuitPrompt: {
                if (HandleMenuSelection(quitPromptItems, selectedQuitPromptItem)) {
                    if (selectedQuitPromptItem == 0)
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
                DrawStartMenuScreen(title, subTitle, startMenuItems, selectedStartMenuItem, keyTextures);
            } break;

            case Screen::HowToPlay: {
                // Draw main How to Play Content
                DrawHowToPlayContent(howToFont);

                // Draw Back button
                DrawMenuItems(howToPlayItems, selectedHowToPlayItem);

                // Show Hint Bar
                DrawHintBar(HOW_TO_PLAY_HINT_LINE, keyTextures);
            } break;

            case Screen::Options: {

            } break;

            case Screen::Gameplay: {
                game.Draw();

                // Show Hint Bar
                DrawHintBar(GAMEPLAY_HINT_LINE, keyTextures);
            } break;

            case Screen::GameOver: {
                DrawCenteredText("GAME OVER", 100, 64, ORANGE, TextEffect::Blink);
                // Explain why the game ended
                DrawCenteredText(game.GetGameEndMessage(), 180, 24, LIGHTGRAY);

                // Draw Score
                DrawCenteredText(TextFormat("FINAL SCORE:   %d", game.Score()), 230, 32, LIGHTGRAY);
                //DrawCenteredText(TextFormat("FINAL SCORE: %d    High Score: %d", game.Score(), highScore), 184, 24, LIGHTGRAY);

                // Draw menu
                DrawMenuItems(gameOverItems, selectedGameOverItem);

                // Draw hint bar
                DrawHintBar(MENU_HINT_LINE, keyTextures);
            } break;

            case Screen::QuitPrompt: {
                // Draw a dimmed start menu in the background
                DrawStartMenuScreen(title, subTitle, startMenuItems, selectedStartMenuItem, keyTextures);
                DrawRectangle(0, 0, SCREEN_WIDTH, HINT_BAR_Y, Fade(BLACK, 0.6f));

                // Draw Quit Prompt
                const float roundness = 0.1f;
                const Rectangle dialog = { 200.0f, 150.0f, SCREEN_WIDTH / 2, 280.0f };
                DrawRectangleRounded(dialog, roundness, 8, QUIT_PROMPT_COLOR);
                DrawRectangleRoundedLines(dialog, roundness, 8, ORANGE);
                DrawCenteredText("Quit Game?", 176, 36, ORANGE);
                DrawCenteredText("The game window will close.", 230, 24, LIGHTGRAY);

                // Draw menu items
                DrawMenuItems(quitPromptItems, selectedQuitPromptItem);
            } break;
        }
        EndDrawing();
    }
    // Unload resources
    UnloadFont(howToFont);
    UnloadKeyTextures(keyTextures);
    CloseWindow();

    return 0;
}