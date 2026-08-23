#pragma once

#include "BoardHelper.hpp"

/**
 * @class Evaluator
 *
 * @brief This class is responsible for evaluating the board state in the Othello game.
 *
 * The Evaluator class contains several evaluation methods that return heuristic scores
 * based on the current state of the game board. Each method looks at different aspects
 * of the board, such as disc difference, mobility, corner control, edge control, and
 * positional score. These evaluations help in decision-making for the AI, determining
 * its next move.
 *
 * @note The weights assigned to each heuristic in the overall evaluation function
 * (get_Eval) might need to be adjusted to optimize AI performance.
 *
 * @see getGamePhase, get_Eval, evalDiscDiff, evalMobility, evalCorner,
 * evalParity, evalEdgeControl, evalPositionalScore,evalEdgeControl.
 */
class Evaluator {
public:
    /**
     * @brief Calculates the evaluation score for a given board position and player.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The evaluation score.
     */
    static int getEvaluation(const std::vector<std::vector<char> > &board, char player);

private:
    BoardHelper bHelper;

    // Evaluation Function Changes during Early-Game / Mid-Game / Late-Game
    enum GamePhase { EARLY_GAME, MID_GAME, LATE_GAME };

    /**
     * @brief Determines the current game phase based on the number of pieces on the board.
     * @param board The current game board.
     * @return The game phase (EARLY_GAME, MID_GAME, or LATE_GAME).
     */
    static GamePhase getGamePhase(const std::vector<std::vector<char> > &board);

    /**
     * @brief Evaluates the disc difference between the player and the opponent.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The disc difference score.
     */
    static int evalDiscDiff(const std::vector<std::vector<char> > &board, char player);

    /**
     * @brief Evaluates the mobility of the player by calculating the number of possible moves.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The mobility score.
     */
    static int evalMobility(const std::vector<std::vector<char> > &board, char player);

    /**
     * @brief Evaluates the corner grab potential of the player.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The corner grab score.
     */
    static int evalCorner(const std::vector<std::vector<char> > &board, char player);

    /**
     * @brief Evaluates the parity of the game based on the remaining number of discs to be placed
     * on the board.
     * @param board The current game board.
     * @return The parity score (-1 or 1).
     */
    static int evalParity(const std::vector<std::vector<char> > &board);

    /**
     * @brief Evaluates the positional score of the player.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The positional score.
     */
    static int evalPositionalScore(const std::vector<std::vector<char> > &board, char player);

    /**
     * @brief Evaluates the edge control of the player.
     * @param board The current game board.
     * @param player The player's disc character ('B' or 'W').
     * @return The edge control score.
     */
    static int evalEdgeControl(const std::vector<std::vector<char> > &board, char player);
};
