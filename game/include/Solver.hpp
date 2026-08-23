#pragma once

#include "BoardHelper.hpp"

class Solver {
public:
    /**
     * @brief Determines the best move for a player on a given game board state.
     * @param board Current game board state represented as a 2D character std::vector.
     * @param player Character representing the player ('B' or 'W').
     * @param depth Depth of the search tree.
     * @return Position Best move for the player as a Position object.
     */
    static Position getBestMovePosition(const std::vector<std::vector<char> > &board, char player, int depth);

private:
    /**
     * @brief Minimax algorithm with alpha-beta pruning to determine the best move score.
     *
     * @param node Current game board state represented as a 2D character std::vector.
     * @param player Character representing the player ('B' or 'W').
     * @param depth Remaining depth of the search tree.
     * @param max Boolean flag indicating whether the function is maximizing or minimizing.
     * @param alpha Alpha value for alpha-beta pruning.
     * @param beta Beta value for alpha-beta pruning.
     * @return int Score of the best move.
     */
    static int miniMaxAlphaBeta(
        std::vector<std::vector<char> > &node, char player, int depth, bool max, int alpha, int beta);
};
