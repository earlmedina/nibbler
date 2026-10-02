#include <raylib.h>
#include <string>

enum class Screen { StartMenu, Options, Gameplay, GameOver };

// Struct used for drawing menu items
struct MenuItem {
    std::string label;
    Rectangle rect;
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    const int targetFPS = 60;

    InitWindow(screenWidth, screenHeight, "Nibbler");
    SetTargetFPS(targetFPS);

    while (WindowShouldClose() == false) {
        BeginDrawing();
        DrawCircle(250, 250, 50, SKYBLUE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}