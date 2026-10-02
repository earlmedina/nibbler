#include <raylib.h>
int main() {
    InitWindow(500, 500, "Initial setup!");
    SetTargetFPS(60);

    while (WindowShouldClose() == false) {
        BeginDrawing();
        DrawCircle(250, 250, 50, SKYBLUE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}