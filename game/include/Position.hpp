/*
 * Othello - C++
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#pragma once

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

/**
 * @brief The Position class represents a position on the Othello game board.
 */
class Position {
public:
    /**
     * @brief Default constructor. Initializes row and column to 0.
     */
    Position() : row(0), col(0) {};

    /**
     * @brief Constructor with row and column parameters.
     * @param row The row of the position.
     * @param col The column of the position.
     */
    Position(unsigned int row, unsigned int col) : row(row), col(col) {};

    /**
     * @brief Copy constructor.
     * @param other The other position to copy.
     */
    Position(const Position& other) = default;;

    /**
     * @brief Get the row of the position.
     * @return The row of the position.
     */
    unsigned int getRow() const { return row; };

    /**
     * @brief Get the column of the position.
     * @return The column of the position.
     */
    unsigned int getCol() const { return col; };

    /**
     * @brief Calculate the Euclidean distance between this and another position.
     * @param other The other position.
     * @return The Euclidean distance.
     */
    double euclideanDistance(const Position& other) const;

    /**
     * @brief Calculate the Manhattan distance between this and another position.
     * @param other The other position.
     * @return The Manhattan distance.
     */
    double manhattanDistance(const Position& other) const;

    /**
     * @brief Overloaded equality operator.
     * @param other The other position.
     * @return True if the positions are equal, false otherwise.
     */
    bool operator==(const Position& other) const;

    /**
     * @brief Overloaded inequality operator.
     * @param other The other position.
     * @return True if the positions are not equal, false otherwise.
     */
    bool operator!=(const Position& other) const;


private:
    unsigned int row;
    unsigned int col;

    /**
     * @brief Overloaded stream output operator.
     * @param os The output stream.
     * @param pos The position.
     * @return The output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Position& pos);

    /**
     * @brief Overloaded stream input operator.
     * @param is The input stream.
     * @param pos The position.
     * @return The input stream.
     */
    friend std::istream& operator>>(std::istream& is, Position& pos);

    friend class BoardHelper;
};