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

#include <vector>
#include <climits>
#include "../include/Solver.hpp"


constexpr char EMPTY = '-';
constexpr char PLAYER_X = 'X';
constexpr char PLAYER_O = 'O';
constexpr int BOARD_SIZE = 8;

Position Solver::getBestMovePosition(const std::vector<std::vector<char>> &board, char player, int depth) {
    int bestScore = INT_MIN;
    Position bestMove(-1, -1);
    for (const Position &move: BoardHelper::getAllPossibleMoves(board, player)) {
        // create new node
        std::vector<std::vector<char>> newNode =
                BoardHelper::getBoardAfterMove(board, move, player);
        // recursive call
        int childScore = miniMaxAlphaBeta(newNode, player, depth - 1, false, INT_MIN, INT_MAX);
        if (childScore > bestScore) {
            bestScore = childScore;
            bestMove = move;
        }
    }
    return bestMove;
}

int Solver::miniMaxAlphaBeta(
        std::vector<std::vector<char>> &node, char player, int depth, bool max, int alpha, int beta) {
    // if terminal reached or depth limit reached evaluate
    if (depth == 0 || BoardHelper::isGameFinished(node)) {
        return Evaluator::getEvaluation(node, player);
    }
    char o_player = (player == PLAYER_X) ? PLAYER_O : PLAYER_X;

    const auto &moves = max ? BoardHelper::getAllPossibleMoves(node, player)
                            : BoardHelper::getAllPossibleMoves(node, o_player);
    if (moves.empty()) { // if no moves available then forfeit turn
        return miniMaxAlphaBeta(node, player, depth - 1, !max, alpha, beta);
    }

    int score = max ? INT_MIN : INT_MAX;
    for (const Position &move: moves) {
        // create new node
        std::vector<std::vector<char>> newNode =
                BoardHelper::getBoardAfterMove(node, move, max ? player : o_player);
        int childScore =
                miniMaxAlphaBeta(newNode, player, depth - 1, !max, alpha, beta); // recursive call

        if (max) { // maximizing
            if (childScore > score)
                score = childScore;
            if (score > alpha)
                alpha = score; // update alpha
        } else {               // minimizing
            if (childScore < score)
                score = childScore;
            if (score < beta)
                beta = score; // update beta
        }

        if (beta <= alpha)
            break; // Cutoff
    }
    return score;
}