#include <raylib.h>
#include <string>

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
constexpr int TARGET_FPS = 60;

enum class Screen { StartMenu, Options, Gameplay, GameOver };

// Struct used for drawing menu items
struct MenuItem {
    std::string label;
    Rectangle rect;
};

/**
 *  * Helper function used to draw centered text
 * @param text
 * @param y
 * @param fontSize
 * @param color
 * @param addBobEffect
 */
void DrawCenteredText(const char *text, int y, int fontSize, Color color, bool addBobEffect = false) {
    int textWidth = MeasureText(text, fontSize);
    if (addBobEffect) {
        // Draw text using bobbing effect
        // Approach modified from: https://stackoverflow.com/questions/67322860/how-do-i-make-a-simple-idle-bobbing-motion-animation
        // The general idea is to apply a sine wave to the Y position to make it bob
        float t = static_cast<float>(GetTime()); // Needs to be cast to float for sinf
        int transformation = (int)(sinf(t * 2.0f) * 6.0f);
        DrawText(text, (SCREEN_WIDTH - textWidth) / 2, y + transformation, fontSize, color);
    }
    else
        DrawText(text, (SCREEN_WIDTH - textWidth) / 2, y, fontSize, color);
}


int main() {
    const char *title = "Nibbler";
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title);
    SetTargetFPS(TARGET_FPS);

    Screen currentScreen = Screen::StartMenu; // Set start screen to Start Menu

    while (WindowShouldClose() == false) {

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground({ 20, 24, 36, 255 }); // Change background to dark blue

        // Conditionally render screen
        switch (currentScreen) {
            case Screen::StartMenu: {
                DrawCenteredText(title, 80, 60, SKYBLUE, true);
                DrawCenteredText("A snake game made with raylib", 155, 20, LIGHTGRAY);
            }
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}