/*
 * Othello - C++
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 */

#pragma once

#include "BoardHelper.hpp"
#include "Solver.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief SFML window that renders the board and drives human vs AI play.
 */
class GameViewer {
public:
    explicit GameViewer(char humanPlayer);

    /**
     * @brief Runs the window event/render loop until the player quits.
     * @return Process exit code (0 on normal close).
     */
    int run();

private:
    enum class Phase {
        Playing,
        AiThinking,
        GameOver
    };

    struct DiscAnim {
        Position pos;
        char player;
        float age;
    };

    static constexpr int BOARD_SIZE = 8;
    static constexpr unsigned int WINDOW_WIDTH = 980;
    static constexpr unsigned int WINDOW_HEIGHT = 720;
    static constexpr float BOARD_ORIGIN_X = 48.f;
    static constexpr float BOARD_ORIGIN_Y = 48.f;
    static constexpr float BOARD_PIXEL_SIZE = 624.f;
    static constexpr float CELL_SIZE = BOARD_PIXEL_SIZE / BOARD_SIZE;
    static constexpr size_t MIN_MAX_DEPTH = 6;

    static constexpr char PLAYER_X = 'X';
    static constexpr char PLAYER_O = 'O';
    static constexpr char EMPTY = '-';

    void processEvents();
    void update(float dt);
    void render();

    void handleBoardClick(sf::Vector2f mousePos);
    void tryHumanMove(const Position &move);
    void playAiMove();
    void resetGame();

    [[nodiscard]] std::optional<Position> cellFromPoint(sf::Vector2f point) const;
    [[nodiscard]] sf::Vector2f cellCenter(const Position &pos) const;
    [[nodiscard]] std::string outcomeMessage() const;

    void drawBackground();
    void drawBoard();
    void drawDiscs();
    void drawValidHints(float pulse);
    void drawSidebar();
    void drawGameOverOverlay();

    bool loadFonts();
    void setStatus(const std::string &message);

    sf::RenderWindow window_;
    sf::Font font_;
    sf::Font fontBold_;
    bool fontsLoaded_ = false;

    std::vector<std::vector<char>> board_;
    char humanPlayer_;
    char aiPlayer_;
    char currentPlayer_;
    Phase phase_ = Phase::Playing;

    std::string statusMessage_;
    std::vector<DiscAnim> discAnims_;
    float pulseTime_ = 0.f;
    float aiDelay_ = 0.f;
    bool aiMovePending_ = false;

    sf::Clock clock_;
};
