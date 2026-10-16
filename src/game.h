#pragma once
#include "food.h"
#include "constants.h"
#include "snake.h"

class Game {
public:
    Game();
    void NewGame();
    void HandleInput();
    void Update(float dt);

    void Draw() const;

    bool IsGameOver() const;

    GameEndType GetGameEndType() const;

    bool HasStarted() const;

    int Score() const;

    const char *GetGameEndMessage() const;

private:
    Snake snake; // Default initialization
    Food food;  // Default initialization
    int score = 0;
    bool gameStarted = false;
    bool gameOver = false;
    GameEndType gameEndType = GameEndType::None;
    float stepInterval = INITIAL_STEP_INTERVAL;
    float timeSinceStep = 0.0f;

    void Step();
    static void DrawGrid(); // Mark static as it doesn't use any internal vars
};
