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

#include "../include/BoardHelper.hpp"

const char EMPTY = '-';
const char PLAYER_X = 'X';
const char PLAYER_O = 'O';
const int BOARD_SIZE = 8;

void BoardHelper::initBoard(std::vector<std::vector<char>> &board) {
    // Initialize the ( BOARD_SIZE x BOARD_SIZE ) board with empty spaces
    board = std::vector<std::vector<char>>(BOARD_SIZE, std::vector<char>(BOARD_SIZE, EMPTY));
    // Place the initial pieces in the center
    board[3][3] = PLAYER_X;
    board[3][4] = PLAYER_O;
    board[4][3] = PLAYER_O;
    board[4][4] = PLAYER_X;
}

void BoardHelper::printBoard(const std::vector<std::vector<char>> &board) {
    std::cout << "\n   0 1 2 3 4 5 6 7 \n";
    std::cout << "------------------\n";
    for (unsigned int row = 0; row < BOARD_SIZE; row++) {
        std::cout << row << "| ";
        for (unsigned int col = 0; col < BOARD_SIZE; col++) {
            std::cout << board[row][col] << " ";
        }
        std::cout << "\n";
    }
}

bool BoardHelper::isReversible(const std::vector<std::vector<char>> &board, const Position &pos, char player) {
    // Check the eight directions around the cell
    for (int row = -1; row <= 1; row++) {
        for (int col = -1; col <= 1; col++) {
            if (row == 0 && col == 0)
                continue; // Ignore the current cell
            int k = 1;
            while (true) {
                int newRow = (int) pos.row + row * k;
                int newCol = (int) pos.col + col * k;
                if (newRow < 0 || ((unsigned) newRow) >= board.size() || newCol < 0 ||
                    ((unsigned) newCol) >= board[newRow].size())
                    break; // Exit if we are outside the game board
                if (board[newRow][newCol] == EMPTY)
                    break; // Exit if the cell is empty
                if (board[newRow][newCol] == player) {
                    if (k > 1) {
                        // If we found a piece of the current player after finding a piece of the
                        // other player, then the move is valid
                        return true;
                    }
                    break;
                }
                k++;
            }
        }
    }
    return false;
}

bool BoardHelper::isValidMove(const std::vector<std::vector<char>> &board, const Position &pos, char player) {
    if (board[pos.row][pos.col] != EMPTY)
        return false; // Check if the cell is empty
    if (pos.row < 0 || pos.row >= BOARD_SIZE || pos.col < 0 || pos.col >= BOARD_SIZE)
        return false; // Check if the cell is within the game board limits

    return isReversible(board, pos, player);
}

void BoardHelper::reversePieces(std::vector<std::vector<char>> &board, const Position &pos,
                                char player) {
    // Check the eight directions around the cell
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0)
                continue; // Ignore the current cell
            int k = 1;
            while (true) {
                int newRow = (int) pos.row + i * k;
                int newCol = (int) pos.col + j * k;
                if (newRow < 0 || ((unsigned int) newRow) >= board.size() || newCol < 0 ||
                    ((unsigned int) newCol) >= board[newRow].size())
                    break; // Exit if we are outside the game board
                if (board[newRow][newCol] == EMPTY)
                    break; // Exit if the cell is empty
                if (board[newRow][newCol] == player) {
                    if (k > 1) {
                        for (int k2 = k - 1; k2 > 0; k2--)
                            board[pos.row + i * k2][pos.col + j * k2] = player;
                        break;
                    }
                    break;
                }
                k++;
            }
        }
    }
}

void BoardHelper::playMove(std::vector<std::vector<char>> &board, const Position &pos, char player) {
    // if (!isValidMove(board, pos, player)) throw invalid_argument("Invalid move");
    board[pos.row][pos.col] = player; // Place the current player's piece on the chosen cell
    reversePieces(board, pos, player);
}

std::vector<Position> BoardHelper::getAllPossibleMoves(const std::vector<std::vector<char>> &board, char player) {
    std::vector<Position> result;
    for (unsigned int row = 0; row < board.size(); ++row) {
        for (unsigned int col = 0; col < board[row].size(); col++) {
            Position p(row, col);
            if (isValidMove(board, p, player))
                result.emplace_back(p);
        }
    }
    return result;
}

bool BoardHelper::isGameFinished(const std::vector<std::vector<char>> &board) {
    return (getAllPossibleMoves(board, 'X').empty() && getAllPossibleMoves(board, 'O').empty());
}

bool BoardHelper::switchPlayer(const std::vector<std::vector<char>> &board, char &player) {
    char newPlayer = (player == PLAYER_X) ? PLAYER_O : PLAYER_X;
    auto newPlayerMoves = getAllPossibleMoves(board, newPlayer);
    auto currentPlayerMoves = getAllPossibleMoves(board, player);
    if (!newPlayerMoves.empty()) {
        player = newPlayer;
        return true;
    }
    if (!currentPlayerMoves.empty())
        return true;
    return false;
}

int BoardHelper::countPiecesPlayer(const std::vector<std::vector<char>> &board, char player) {
    int score = 0;
    for (const auto &row: board)
        for (const auto &col: row)
            if (col == player)
                score++;
    return score;
}

int BoardHelper::countPiecesTotal(const std::vector<std::vector<char>> &board) {
    int c = 0;
    for (const auto &row: board)
        for (const auto &col: row)
            if (col != EMPTY)
                c++;
    return c;
}

std::vector<std::vector<char>>
BoardHelper::getBoardAfterMove(const std::vector<std::vector<char>> &board, const Position &move, char player) {
    // get clone of old board
    std::vector<std::vector<char>> newBoard = board;
    // place piece
    newBoard[move.row][move.col] = player;
    // reverse pieces
    std::vector<Position> rev = getReversePoints(newBoard, player, move);
    for (const Position &pos: rev) {
        newBoard[pos.row][pos.col] = player;
    }
    return newBoard;
}

std::vector<Position>
BoardHelper::checkUp(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_up;
    int mRow = (int) pos.row - 1;
    int mCol = (int) pos.col;
    while (mRow > 0 && board[mRow][mCol] == o_player) {
        m_up.emplace_back(mRow, mCol);
        mRow--;
    }
    if (mRow >= 0 && board[mRow][mCol] == player && !m_up.empty())
        return m_up;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkUpRight(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_up_right;
    int mRow = (int) pos.row - 1;
    int mCol = (int) pos.col + 1;
    while (mRow > 0 && mCol < 7 && board[mRow][mCol] == o_player) {
        m_up_right.emplace_back(mRow, mCol);
        mRow--;
        mCol++;
    }
    if (mRow >= 0 && mCol <= 7 && board[mRow][mCol] == player && !m_up_right.empty())
        return m_up_right;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkRight(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_right;
    int mRow = (int) pos.row;
    int mCol = (int) pos.col + 1;
    while (mCol < 7 && board[mRow][mCol] == o_player) {
        m_right.emplace_back(mRow, mCol);
        mCol++;
    }
    if (mCol <= 7 && board[mRow][mCol] == player && !m_right.empty())
        return m_right;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkDownRight(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_down_right;
    int mRow = (int) pos.row + 1;
    int mCol = (int) pos.col + 1;
    while (mRow < 7 && mCol < 7 && board[mRow][mCol] == o_player) {
        m_down_right.emplace_back(mRow, mCol);
        mRow++;
        mCol++;
    }
    if (mRow <= 7 && mCol <= 7 && board[mRow][mCol] == player && !m_down_right.empty())
        return m_down_right;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkDown(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_down;
    int mRow = (int) pos.row + 1;
    int mCol = (int) pos.col;
    while (mRow < 7 && board[mRow][mCol] == o_player) {
        m_down.emplace_back(mRow, mCol);
        mRow++;
    }
    if (mRow <= 7 && board[mRow][mCol] == player && !m_down.empty())
        return m_down;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkDownLeft(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_down_left;
    int mRow = (int) pos.row + 1;
    int mCol = (int) pos.col - 1;
    while (mRow < 7 && mCol > 0 && board[mRow][mCol] == o_player) {
        m_down_left.emplace_back(mRow, mCol);
        mRow++;
        mCol--;
    }
    if (mRow <= 7 && mCol >= 0 && board[mRow][mCol] == player && !m_down_left.empty())
        return m_down_left;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkLeft(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_left;
    int mRow = (int) pos.row;
    int mCol = (int) pos.col - 1;
    while (mCol > 0 && board[mRow][mCol] == o_player) {
        m_left.emplace_back(mRow, mCol);
        mCol--;
    }
    if (mCol >= 0 && board[mRow][mCol] == player && !m_left.empty())
        return m_left;
    else
        return {};
}

std::vector<Position>
BoardHelper::checkUpLeft(std::vector<std::vector<char>> &board, char player, const Position &pos) {
    char o_player = ((player == PLAYER_X) ? PLAYER_O : PLAYER_X);
    std::vector<Position> m_up_left;
    int mRow = (int) pos.row - 1;
    int mCol = (int) pos.col - 1;
    while (mRow > 0 && mCol > 0 && board[mRow][mCol] == o_player) {
        m_up_left.emplace_back(mRow, mCol);
        mRow--;
        mCol--;
    }
    if (mRow >= 0 && mCol >= 0 && board[mRow][mCol] == player && !m_up_left.empty())
        return m_up_left;
    else
        return {};
}

std::vector<Position>
BoardHelper::getReversePoints(std::vector<std::vector<char>> &o_board, char player, const Position &pos) {
    std::vector<Position> allReversePoints;
    std::vector<Position> temp;

    temp = checkUp(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkUpRight(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkRight(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkDownRight(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkDown(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkDownLeft(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkLeft(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());
    temp = checkUpLeft(o_board, player, pos);
    allReversePoints.insert(allReversePoints.end(), temp.begin(), temp.end());

    return allReversePoints;
}