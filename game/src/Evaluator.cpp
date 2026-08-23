#include "Evaluator.hpp"
#include "Player.hpp"

constexpr int BOARD_SIZE = 8;
constexpr int MAX_PIECES = 64;
const std::vector<std::pair<int, int> > CORNERS = {
    std::make_pair(0, 0), std::make_pair(0, 7),
    std::make_pair(7, 0), std::make_pair(7, 7)
};

Evaluator::GamePhase Evaluator::getGamePhase(const std::vector<std::vector<char> > &board) {
    int totalPiecesCount = BoardHelper::countTotalPieces(board);
    if (totalPiecesCount < 20)
        return EARLY_GAME;
    else if (totalPiecesCount <= 58)
        return MID_GAME;
    else
        return LATE_GAME;
}

int Evaluator::getEvaluation(const std::vector<std::vector<char> > &board, char player) {
    // terminal
    if (BoardHelper::isGameFinished(board)) {
        return 1000 * evalDiscDiff(board, player);
    }
    // semi-terminal
    if (getGamePhase(board) == EARLY_GAME) {
        return 1000 * evalCorner(board, player) + 50 * evalMobility(board, player) +
               30 * evalPositionalScore(board, player) + 30 * evalEdgeControl(board, player);
    } else if (getGamePhase(board) == MID_GAME) {
        return 1000 * evalCorner(board, player) + 20 * evalMobility(board, player) +
               10 * evalDiscDiff(board, player) + 100 * evalParity(board) +
               50 * evalPositionalScore(board, player) + 50 * evalEdgeControl(board, player);
    } else {
        // LATE_GAME
        return 1000 * evalCorner(board, player) + 100 * evalMobility(board, player) +
               500 * evalDiscDiff(board, player) + 500 * evalParity(board) +
               100 * evalPositionalScore(board, player) + 100 * evalEdgeControl(board, player);
    }
}

/**
 * Disc difference (Measures the difference in the number of discs on the board. Has zero weight in
 * the opening, but increases to a moderate weight in the MID_GAME, and to a significant weight in
 * the endgame.)
 */
int Evaluator::evalDiscDiff(const std::vector<std::vector<char> > &board, char player) {
    char opponentPlayer = opponentOf(player);

    int playerPiecesCount = BoardHelper::countPlayerPieces(board, player);
    int opponentPiecesCount = BoardHelper::countPlayerPieces(board, opponentPlayer);

    return 100 * (playerPiecesCount - opponentPiecesCount) /
           (playerPiecesCount + opponentPiecesCount);
}

/**
 * Mobility (Measures the number of moves the player is currently able to make. Has significant
 * weight in the opening game, but diminishes to zero weight towards the endgame.)
 */
int Evaluator::evalMobility(const std::vector<std::vector<char> > &board, char player) {
    char opponentPlayer = opponentOf(player);

    int playerMoveCount = static_cast<int>(BoardHelper::getAllPossibleMoves(board, player).size());
    int opponentMoveCount =
            static_cast<int>(BoardHelper::getAllPossibleMoves(board, opponentPlayer).size());

    return 100 * (playerMoveCount - opponentMoveCount) / (playerMoveCount + opponentMoveCount + 1);
}

/**
 * Corner Grab (Measures if the current player can take a corner with its next move, Weighted highly
 * at all times.)
 */
int Evaluator::evalCorner(const std::vector<std::vector<char> > &board, char player) {
    char opponentPlayer = opponentOf(player);

    int playerCorners = 0;
    int opponentCorners = 0;

    for (const auto &[key, value]: CORNERS) {
        if (board[key][value] == player)
            playerCorners++;
        if (board[key][value] == opponentPlayer)
            opponentCorners++;
    }

    return 100 * (playerCorners - opponentCorners) / (playerCorners + opponentCorners + 1);
}

/**
 * Parity (Measures who is expected to make the last move of the game. Has zero weight in the
 * opening, but increases to a very large weight in the MID_GAME and endgame.)
 */
int Evaluator::evalParity(const std::vector<std::vector<char> > &board) {
    int remainingDiscs = MAX_PIECES - BoardHelper::countTotalPieces(board);
    return remainingDiscs % 2 == 0 ? -1 : 1;
}

const int scoreTable[8][8] = {
    {120, -20, 20, 5, 5, 20, -20, 120},
    {-20, -40, -5, -5, -5, -5, -40, -20},
    {20, -5, 15, 3, 3, 15, -5, 20},
    {5, -5, 3, 3, 3, 3, -5, 5},
    {5, -5, 3, 3, 3, 3, -5, 5},
    {20, -5, 15, 3, 3, 15, -5, 20},
    {-20, -40, -5, -5, -5, -5, -40, -20},
    {120, -20, 20, 5, 5, 20, -20, 120}
};

/**
 * This heuristic checks how well the player has managed to place their discs on the board. The
 * heuristic uses a predefined positional weight matrix, which assigns higher scores to stable
 * positions (corners and edges) and lower scores to unstable positions (adjacent to corners).
 */
int Evaluator::evalPositionalScore(const std::vector<std::vector<char> > &board, char player) {
    char oPlayer = opponentOf(player);

    int myEdges = 0;
    int opEdges = 0;

    for (int i = 1; i < BOARD_SIZE - 1; i++) {
        if (board[i][0] == player)
            myEdges++;
        if (board[i][BOARD_SIZE - 1] == player)
            myEdges++;
        if (board[0][i] == player)
            myEdges++;
        if (board[BOARD_SIZE - 1][i] == player)
            myEdges++;

        if (board[i][0] == oPlayer)
            opEdges++;
        if (board[i][BOARD_SIZE - 1] == oPlayer)
            opEdges++;
        if (board[0][i] == oPlayer)
            opEdges++;
        if (board[BOARD_SIZE - 1][i] == oPlayer)
            opEdges++;
    }

    if (myEdges + opEdges == 0)
        return 0;
    return 100 * (myEdges - opEdges) / (myEdges + opEdges);
}

/**
 * This heuristic is based on the observation that having
 * more discs on the edge of the board often provides an advantage, as these discs are more stable
 * (i.e., less likely to be flipped).
 */
int Evaluator::evalEdgeControl(const std::vector<std::vector<char> > &board, char player) {
    int score = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == player) {
                score += scoreTable[i][j];
            } else if (board[i][j] != EMPTY) {
                score -= scoreTable[i][j];
            }
        }
    }
    return score;
}
