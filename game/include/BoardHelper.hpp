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

#include "Position.hpp"
#include <iostream>
#include <vector>

/**
 * @brief Helper class that provides various utilities for the Othello board.
 */
class BoardHelper {
public:
    /**
     * @brief Constructs a new Board Helper object.
     */
    BoardHelper() = default;

    /**
     * @brief Initializes the Othello board.
     * @param board 2D char vector representing the board.
     */
    static void initBoard(std::vector<std::vector<char>> &board);

    /**
     * @brief Prints the current state of the board.
     * @param board 2D char vector representing the board.
     */
    static void printBoard(const std::vector<std::vector<char>> &board);

    /**
     * @brief Checks if a given move results in reversed pieces.
     *
     * @param board 2D char vector representing the current state of the board.
     * @param pos The position at which the player wishes to make a move.
     * @param player The character representing the current player.
     * @return true if the move results in reversed pieces, false otherwise.
     */
    static bool isReversible(const std::vector<std::vector<char>> &board, const Position &pos, char player);

    /**
     * @brief Checks if a move is valid for the current player.
     * @param board 2D char vector representing the current state of the board.
     * @param pos The position at which the player wishes to make a move.
     * @param player The character representing the current player.
     * @return true if the move is valid, false otherwise.
     */
    static bool isValidMove(const std::vector<std::vector<char>> &board, const Position &pos, char player);

    /**
     * @brief Reverses the opponent's pieces according to Othello rules after a valid move.
     * @param board 2D char vector representing the current state of the board. It will be updated
     * with the new board state after reversing pieces.
     * @param pos The position at which the player has just placed a piece.
     * @param player The character representing the current player.
     */
    static void reversePieces(std::vector<std::vector<char>> &board, const Position &pos, char player);

    /**
     * @brief Places the player's piece at the specified position and reverses any opponent pieces
     * accordingly.
     * @param board 2D char vector representing the current state of the board. It will be updated
     * with the new board state after reversing pieces.
     * @param pos The position at which the player has just placed a piece.
     * @param player The character representing the current player.
     */
    static void playMove(std::vector<std::vector<char>> &board, const Position &pos, char player);

    /**
     * @brief Returns all possible moves for the current player.
     * @param board 2D char vector representing the board.
     * @param player The current player.
     * @return Vector of possible positions.
     */
    static std::vector<Position> getAllPossibleMoves(const std::vector<std::vector<char>> &board, char player);

    /**
     * @brief Checks if the game is finished.
     * @param board 2D char vector representing the board.
     * @return true if the game is finished, false otherwise.
     */
    static bool isGameFinished(const std::vector<std::vector<char>> &board);

    /**
     * @brief Switches the player if the current player has no valid moves. If no one can play,
     * returns false.
     * @param board 2D char vector representing the board.
     * @param player The current player.
     * @return true if a player can play, false otherwise.
     */
    static bool switchPlayer(const std::vector<std::vector<char>> &board, char &player);

    /**
     * @brief Counts the number of pieces for a given player.
     * @param board 2D char vector representing the board.
     * @param player The current player.
     * @return The number of pieces for the given player.
     */
    static int countPiecesPlayer(const std::vector<std::vector<char>> &board, char player);

    /**
     * @brief Returns the total number of pieces on the board.
     * @param board 2D char vector representing the board.
     * @return The total number of pieces on the board.
     */
     static int countPiecesTotal(const std::vector<std::vector<char>> &board) ;

    /**
     * @brief Returns a new game board after playing a move.
     * @param board 2D char vector representing the board.
     * @param move The position of the move.
     * @param player The current player.
     * @return A new board after the move.
     */
    static std::vector<std::vector<char>> getBoardAfterMove(const std::vector<std::vector<char>> &board,
                                                               const Position &move, char player);

private:
    /**
     * @brief Returns the positions of the pieces to reverse after playing a move.
     * @param oBoard 2D char vector representing the original board.
     * @param player The current player.
     * @param pos The position of the move.
     * @return Vector of positions to reverse.
     */
    static std::vector<Position>
    getReversePoints(std::vector<std::vector<char>> &oBoard, char player, const Position &pos);

    /** @brief Helper function to check in the up direction. */
    static std::vector<Position> checkUp(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the up-right direction. */
    static std::vector<Position> checkUpRight(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the right direction. */
    static std::vector<Position> checkRight(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the down-right direction. */
    static std::vector<Position>
    checkDownRight(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the down direction. */
    static std::vector<Position> checkDown(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the down-left direction. */
    static std::vector<Position> checkDownLeft(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the left direction. */
    static std::vector<Position> checkLeft(std::vector<std::vector<char>> &board, char player, const Position &pos);

    /** @brief Helper function to check in the up-left direction. */
    static std::vector<Position> checkUpLeft(std::vector<std::vector<char>> &board, char player, const Position &pos);
};
