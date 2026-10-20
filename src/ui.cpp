//
// Created by Earl Medina on 10/15/26.
//

#include "ui.h"
#include "constants.h"
#include "raylib.h"
#include "resources.h"
#include <cmath>

/**
 *  * Helper function used to draw centered text
 * @param text      - Text to draw
 * @param y         - Y position on screen
 * @param fontSize  - Text font size
 * @param color     - Text color
 * @param effect    - TextEffect enum
 */
void DrawCenteredText(const char *text, int y, int fontSize, Color color, TextEffect effect) {
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
void DrawTextLines(int x, int y, std::initializer_list<const char *> lines, int fontSize,
    int lineHeight, Color color) {

    for (const char *line : lines) {
        DrawText(line, x, y, fontSize, color);
        y += lineHeight;
    }
}

void DrawTextLinesEx(float x, float y, std::initializer_list<const char *> lines, const Font &font, float fontSize,
    float lineHeight, Color color) {

    for (const char *line : lines) {
        //DrawText(line, x, y, fontSize, color);
        DrawTextEx(font, line, Vector2{ roundf(x), roundf(y) }, fontSize, HINT_SPACING, color); // Use roundf because otherwise it can look blurry
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

/**
 * Draws menu items with selection highlight
 * @param menuItems - reference to MenuItem array
 * @param selectedMenuItem - the index of the selected menu item
 */
void DrawMenuItems(const std::vector<MenuItem> &menuItems, int selectedMenuItem) {
    for (size_t i = 0; i < menuItems.size(); i++) {
        Rectangle r = menuItems[i].rect;
        bool isSelected = static_cast<int>(i) == selectedMenuItem;
        bool isBack = menuItems[i].type == MenuItemType::Back;
        bool isSpecial = menuItems[i].type == MenuItemType::Special;

        // Draw menu item container - the item is marked as isBack, use a lighter
        // color to distinguish it, isSpecial is green...
        DrawRectangleRounded(r, 0.5f, 8, isSelected
            ? MENU_ITEM_SELECTED_COLOR : isBack
                ? DARKGRAY : isSpecial
                    ? MENU_ITEM_SPECIAL_COLOR
                    : MENU_ITEM_BASE_COLOR);

        // Draw menu item text
        int textWidth = MeasureText(menuItems[i].label.c_str(), MENU_FONT_SIZE);
        DrawText(menuItems[i].label.c_str(),
            static_cast<int>(r.x + (r.width - textWidth) / 2),
            static_cast<int>(r.y + (r.height - MENU_FONT_SIZE) / 2),
            MENU_FONT_SIZE,
            isSelected ? MENU_ITEM_SELECTED_TEXT_COLOR : MENU_ITEM_BASE_TEXT_COLOR);
    }
}

/**
 * Helper function to determine if a key label is an arrow key
 * @param keyLabel - reference to string label for key
 * @return - true/false
 */
bool IsArrowKey(const std::string &keyLabel) {
    return keyLabel == "Up" || keyLabel == "Down" || keyLabel == "Left" || keyLabel == "Right";
}

/**
 * Get Width of Key Box drawn from text
 * @param keyLabel  - reference to string label for key
 * @return - calculated width as float
 */
float GetKeyBoxWidth(const std::string &keyLabel) {
    // Arrow and single char labels should be square
    if (IsArrowKey(keyLabel) || keyLabel.size() == 1)
        return KEY_HEIGHT;
    // Otherwise, measure text and account for x padding on both sides
    return static_cast<float>(MeasureText(keyLabel.c_str(), HINT_FONT_SIZE)) + 2 * KEY_PADDING;
}

/**
 * Get width of Key drawn from texture. Useful for case where Enter texture wasn't loaded and is actually frawn from text.
 * @param keyLabel  - reference to string label for key
 * @param keyTextures - reference to key texture map
 * @return
 */
float GetKeyWidth(const std::string &keyLabel, const std::unordered_map<std::string, Texture2D> &keyTextures) {
    // If texture was loaded, return key height
    if (keyTextures.contains(keyLabel))
        return KEY_HEIGHT;
    // Otherwise, the key is drawn from text and we need the key box width
    return GetKeyBoxWidth(keyLabel);
}

/**
 * Measures the input hint part width. Considers font when determining width.
 * @param part - reference to single HintPart
 * @param keyTextures - reference to key texture map
 * @param font - reference to font
 * @return - float value representing HintPart width
 */
float GetHintPartWidth(const HintPart &part, const std::unordered_map<std::string, Texture2D> &keyTextures, const Font &font) {
    // For empty strings, return spacer.
    if (part.text.empty())
        return HINT_SPACER;

    // For keys, return the calculated key box width
    if (part.isKey)
        return GetKeyWidth(part.text, keyTextures);

    // Otherwise, it's plain text so return MeasureText result
    return MeasureTextEx(font, part.text.c_str(), HINT_FONT_SIZE, HINT_SPACING).x;
}

/**
 * Measures the input hint line width (used for centering). Considers font when determining width.
 * @param parts - reference to HintPart array
 * @param keyTextures - reference to key texture map
 * @param font - reference to font
 * @return - float value representing line width
 */
float MeasureHintLine(const std::vector<HintPart> &parts, const std::unordered_map<std::string, Texture2D> &keyTextures, const Font &font) {
    float totalWidth = 0.0f;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0)
            totalWidth += HINT_GAP; // Account for space between hint parts
        totalWidth += GetHintPartWidth(parts[i], keyTextures, font);
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

/**
 * Draws key from loaded key textures
 * @param keyTextures - reference to key texture map
 * @param keyLabel - reference to key label
 * @param x - start x where to draw key
 * @param y - start y where to draw key
 * @return - float value representing "end" of key texture to know where to start next part
 */
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
    return x + KEY_HEIGHT; // Return "end" of key texture to know where to start next part
}

/**
 * Takes an array of hint parts and draws them at the specific top left position
 * @param parts - array of hint parts
 * @param x  - x position where line should start
 * @param y  - y position where line should start
 * @return - x position where the line ends
 */
float DrawHintLine(const std::vector<HintPart> &parts, float x, float y, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font) {
    for (size_t i = 0; i < parts.size(); i++) {
        const HintPart &part = parts[i]; // reference part

        // Add space between parts
        if (i > 0)
            x += HINT_GAP;

        if (!part.text.empty()) {
            if (part.isKey)
                DrawKey(keyTextures, part.text, x, y);
            else
                DrawTextEx(font, part.text.c_str(), Vector2{ roundf(x), roundf(y) }, HINT_FONT_SIZE, HINT_SPACING, LIGHTGRAY);
                //DrawText(part.text.c_str(), static_cast<int>(x), static_cast<int>(y), HINT_FONT_SIZE, LIGHTGRAY);
        }
        x += GetHintPartWidth(part, keyTextures, font);
    }
    return x;
}

/**
 * Same as DrawHintLine, but x-centered
 * @param parts - array of hint parts
 * @param y  - y position where line should start
 * @return - x position where the line ends
 */
float DrawCenteredHintLine(const std::vector<HintPart> &parts, float y, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font) {
    const float centerX = (SCREEN_WIDTH - MeasureHintLine(parts, keyTextures, font)) / 2.0f; // Figure out where line should start to be x-centered
    return DrawHintLine(parts, centerX, y, keyTextures, font);
}

/**
 * Helper function to draw the hint bar footer shown on screens
 * @param hintLine - array of hint parts representing a line to draw
 */
void DrawHintBar(const std::vector<HintPart> &hintLine, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font) {
    // Draw to separate hint bar from play grid
    DrawLine(0, HINT_BAR_Y, SCREEN_WIDTH, HINT_BAR_Y, Fade(RAYWHITE, 0.5));
    const float x = (SCREEN_WIDTH - MeasureHintLine(hintLine, keyTextures, font)) / 2.0f; // Figure out where line should start to be x-centered
    const float y = HINT_BAR_Y + (HINT_BAR_HEIGHT - HINT_FONT_SIZE) / 2.0f; // Figure out where line should start to be y-centered
    DrawHintLine(hintLine, x, y, keyTextures, font);
}

/**
 * Helper function to draw start menu screen. Used for dimming effect when drawing quit prompt.
 * @param title - title text
 * @param subTitle - subtitle text
 * @param startMenuItems - MenuItem array
 * @param selectedStartMenuItem - selected start menu item index
 */
void DrawStartMenuScreen(const char *title, const char *subTitle, const std::vector<MenuItem> &startMenuItems,
    const int selectedStartMenuItem, std::unordered_map<std::string, Texture2D> &keyTextures, Font &font) {
    // Draw title and subtitle
    DrawCenteredText(title, 60, 60, SCREEN_TITLE_COLOR, TextEffect::Bob);
    DrawCenteredText(subTitle, 130, 20, LIGHTGRAY);

    // Draw menu items
    DrawMenuItems(startMenuItems, selectedStartMenuItem);

    // Show Hint Bar
    DrawHintBar(MENU_HINT_LINE, keyTextures, font);
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
bool HandleMenuSelection(const std::vector<MenuItem> &menuItems, int &selectedMenuItem, std::unordered_map<std::string, Sound> &sounds) {
    const int previousSelectedItem = selectedMenuItem;
    HandleKeyboardNavigation(selectedMenuItem, static_cast<int>(menuItems.size()));
    bool clicked = false;
    HandleMouseNavigation(menuItems, clicked, selectedMenuItem);

    // Play movement sound
    if (selectedMenuItem != previousSelectedItem)
        PlayMapSound(sounds, "move");

    bool hasSelection = clicked || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER); // Include keypad enter

    // Play selection sound
    if (hasSelection) {
        if (menuItems[selectedMenuItem].type == MenuItemType::Back)
            PlayMapSound(sounds, "back");
        else {
            if (menuItems[selectedMenuItem].label == "Quit")
                PlayMapSound(sounds, "confirm");
            else
                PlayMapSound(sounds, "select");
        }
    }

    return hasSelection;
}

/**
 * Used for Options screen to navigate setting rows with Up/Down or W/S
 * using keyboard and mouse
 * @param focusedRowIdx - reference to the var holding currently focused row index
 * @param rowRects - Array of Rectangles corresponding to rows
 * @return - true if focus changed (used to play sounds)
 */
bool UpdateRowFocus(int &focusedRowIdx, const std::vector<Rectangle> &rowRects) {
    const int rowCount = static_cast<int>(rowRects.size()); // get row count
    const int prevFocusedRowIdx = focusedRowIdx; // Set prev row to know if there was a change

    // Upwards navigation with wraparound
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        focusedRowIdx = (focusedRowIdx - 1 + rowCount) % rowCount;
    }
    // Downwards navigation with wraparound
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        focusedRowIdx = (focusedRowIdx + 1) % rowCount;
    }

    // Handle mouse movement/focus
    const Vector2 delta = GetMouseDelta();
    if (delta.x != 0.0f || delta.y != 0.0f) {
        const Vector2 mouse = GetMousePosition();
        for (int i= 0; i < rowCount; i++) {
            if (CheckCollisionPointRec(mouse, rowRects[i]))
                focusedRowIdx = i;
        }
    }
    return focusedRowIdx != prevFocusedRowIdx;
}

/**
 * Draws outline around target rect (control's rect). Used for keyboard focus.
 * @param rect - the Rectangle to draw an outline around
 */
void DrawFocusOutline(const Rectangle rect) {
    const Rectangle outline = { rect.x - FOCUS_GAP, rect.y - FOCUS_GAP,
        rect.width + 2* FOCUS_GAP, rect.height + 2 * FOCUS_GAP };
    DrawRectangleRoundedLines(outline, 0.3, 8, TOGGLE_FOCUS_COLOR);
}