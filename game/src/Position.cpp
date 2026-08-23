#include "Position.hpp"

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
