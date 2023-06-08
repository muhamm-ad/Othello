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

#include "../include/Position.hpp"
#include <cassert>
#include <cmath>

double Position::euclideanDistance(const Position &other) const {
    unsigned int deltaY = row - other.row;
    unsigned int deltaX = col - other.col;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

double Position::manhattanDistance(const Position &other) const {
    return (row - other.row) + (col - other.col);
}

bool Position::operator==(const Position &other) const {
    return (row == other.row && col == other.col);
}

bool Position::operator!=(const Position &other) const { return !(*this == other); }

std::ostream &operator<<(std::ostream &os, const Position &pos) {
    os << "{row=" << pos.row << ","
       << " col=" << pos.col << "}";
    return os;
}

std::istream &operator>>(std::istream &is, Position &pos) {
    char po;
    char vir;
    char pf;
    is >> po;
    if (is) {
        is >> pos.row >> vir >> pos.col >> pf;
        if (po != '{' || vir != ',' || pf != '}') {
            throw InvalidPositionFormatException();
        }
    } else {
        throw InvalidPositionFormatException();
    }
    return is;
}

