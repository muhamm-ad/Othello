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

#include "BoardHelper.hpp"
#include "Evaluator.hpp"

class Solver {

  public:

    /**
     * @brief Determines the best move for a player on a given game board state.
     * @param board Current game board state represented as a 2D character std::vector.
     * @param player Character representing the player ('X' or 'O').
     * @param depth Depth of the search tree.
     * @return Position Best move for the player as a Position object.
     */
    static Position getBestMovePosition(const std::vector<std::vector<char>> &board, char player, int depth);

  private:

    /**
     * @brief Minimax algorithm with alpha-beta pruning to determine the best move score.
     *
     * @param node Current game board state represented as a 2D character std::vector.
     * @param player Character representing the player ('X' or 'O').
     * @param depth Remaining depth of the search tree.
     * @param max Boolean flag indicating whether the function is maximizing or minimizing.
     * @param alpha Alpha value for alpha-beta pruning.
     * @param beta Beta value for alpha-beta pruning.
     * @return int Score of the best move.
     */
    static int miniMaxAlphaBeta(
            std::vector<std::vector<char>> &node, char player, int depth, bool max,int alpha, int beta);
};
