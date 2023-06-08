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

#include <limits>

#include "include/BoardHelper.hpp"
#include "include/Solver.hpp"

constexpr size_t MIN_MAX_DEPTH = 6; // Level of the game
constexpr char PLAYER_X = 'X';
constexpr char PLAYER_O = 'O';

void displayStart() {
    std::cout << "**********************************" << std::endl;
    std::cout << "*       Welcome to Othello!      *" << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << std::endl;
    std::cout << "Rules:" << std::endl;
    std::cout << "1. The game is played on an 8x8 board." << std::endl;
    std::cout << "2. The game starts with four discs placed in the middle of the board." << std::endl;
    std::cout << "3. Two discs are player X's, and two discs are player O's." << std::endl;
    std::cout << "4. Player X starts the game." << std::endl;
    std::cout << "5. Players take turns placing discs, with each disc being placed on an empty space." << std::endl;
    std::cout << "6. A player can place a disc anywhere on the board, as long as it sandwiches opponent's discs between the new disc and any of their existing discs." << std::endl;
    std::cout << "7. All of the opponent's discs that are sandwiched are flipped over to become the current player's discs." << std::endl;
    std::cout << "8. If a player cannot make a valid move, they skip their turn." << std::endl;
    std::cout << "9. The game ends when neither player can make a valid move, usually when the board is full." << std::endl;
    std::cout << "10. The player with the most discs of their color wins the game." << std::endl;
    std::cout << std::endl;
    std::cout << "Good luck and have fun!" << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << std::endl;
    std::cout << "Press ENTER to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
}

int scoreX(const std::vector<std::vector<char>> &board) {
    int X = 0;
    int O = 0;
    for (auto &line : board)
        for (auto c : line) {
            X += (c == PLAYER_X);
            O += (c == PLAYER_O);
        }
    return X - O;
}

void displayEnd(const std::vector<std::vector<char>> &board, const char humanPlayer, const char aiPlayer) {
    std::cout << "\n**********************************\n";
    BoardHelper::printBoard(board);
    int s = scoreX(board);
    if (s > 0 && humanPlayer == PLAYER_X || s < 0 && humanPlayer == PLAYER_O) {
        std::cout << "   Congratulations! You have won by " << std::abs(s) << " points!\n";
    } else if (s != 0) {
        std::cout << "   The AI, player " << aiPlayer << ", has won by " << std::abs(s) << " points!\n";
        std::cout << "   My AI doesn't mess around, try again if you dare! :D\n";
    } else {
        std::cout << "   It's a draw! Well played! You've matched your wits against AI.\n";
    }
    std::cout << "**********************************\n";
}

Position readUserMove() {
    Position move;
    if (!(std::cin >> move)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw InvalidPositionFormatException();
    }
    return move;
}

int main(int argc, char *argv[]) {

    if (argc != 2 || ((argv[1][0] != PLAYER_X) && (argv[1][0] != PLAYER_O))) {
        std::cerr << "Usage :" << std::endl;
        std::cerr << argv[0] << " [" << PLAYER_X << "|" << PLAYER_O << "]" << std::endl;
        return 0;
    }

    char humanPlayer = argv[1][0];
    char aiPlayer = (humanPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
    char currentPlayer = PLAYER_X;

    displayStart();

    std::vector<std::vector<char>> board;
    BoardHelper::initBoard(board);
    Position move;

    if (currentPlayer == humanPlayer)
        BoardHelper::printBoard(board);

    while (true) {
        try {
            if (currentPlayer == humanPlayer) {
                std::cout << "\nYour move, Player " << humanPlayer << " (format: {row, col}): ";
                move = readUserMove();
            } else {
                move = Solver::getBestMovePosition(board, aiPlayer, MIN_MAX_DEPTH);
                std::cout << "\nAI's move, Player " << aiPlayer << ": " << move << std::endl;
            }
            if (BoardHelper::isValidMove(board, move, currentPlayer)) {
                BoardHelper::playMove(board, move, currentPlayer);
                BoardHelper::printBoard(board);
                if (!BoardHelper::switchPlayer(board, currentPlayer)) {
                    break;
                }
            } else {
                std::cerr << "Invalid move! You must sandwich your opponent's disc between your new disc and one of your existing ones.\n";
            }
        } catch (const InvalidPositionFormatException &e) {
            std::cerr << e.what() << ". Please enter your move in the correct format: {row,col}\n";
        }
    }

    displayEnd(board, humanPlayer, aiPlayer);

    return 0;
}