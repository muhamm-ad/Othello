#pragma once

constexpr char PLAYER_BLACK = 'B';
constexpr char PLAYER_WHITE = 'W';
constexpr char EMPTY = '-';

inline char opponentOf(char player) {
    return (player == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK;
}
