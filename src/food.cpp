//
// Created by Earl Medina on 10/15/26.
//

#include "food.h"
#include "constants.h"
#include "raylib.h"
#include "snake.h"

Cell Food::Position() const {
    return position;
}

/**
 * Spawn food in new location
 * @param snake - Snake object by reference
 * @return boolean value indicating whether the spawn was successful
 */
bool Food::Spawn(const Snake &snake) {
    // Handle highly unlikely edge case: player has covered grid with snake body
    if (snake.Length() >= static_cast<size_t>(COLS * ROWS)) {
        return false;
    }

    // Keep spawning until a valid location is found
    do {
        position = {GetRandomValue(0, COLS - 1), GetRandomValue(0, ROWS - 1)};
    } while (snake.Contains(position));
    return true;
}

void Food::Draw() const {
    // Need to get centerpoint from rect to draw circle in correct location
    Rectangle r = position.ToRect();
    int centerX = static_cast<int>(r.x + r.width / 2); // r is float so need to cast to int
    int centerY = static_cast<int>(r.y + r.height / 2);
    DrawCircle(centerX, centerY, CELL_SIZE * 0.5f, MAROON);
}
