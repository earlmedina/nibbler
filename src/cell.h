#pragma once
#include "constants.h"
#include "raylib.h"


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