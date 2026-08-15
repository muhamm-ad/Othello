/*
 * Othello - C++
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 */

#include "../include/GameViewer.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace {

const sf::Color kBgTop(18, 42, 36);
const sf::Color kBgBottom(10, 24, 22);
const sf::Color kBoardGreen(27, 122, 74);
const sf::Color kBoardGreenDark(18, 92, 56);
const sf::Color kGridLine(12, 58, 40, 160);
const sf::Color kFrame(56, 34, 22);
const sf::Color kFrameHighlight(92, 58, 36);
const sf::Color kDiscDark(22, 24, 28);
const sf::Color kDiscLight(236, 232, 220);
const sf::Color kHint(255, 214, 120, 140);
const sf::Color kPanel(14, 28, 26, 220);
const sf::Color kText(235, 240, 232);
const sf::Color kMuted(170, 190, 180);
const sf::Color kAccent(232, 168, 72);

float easeOutBack(float t) {
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float u = t - 1.f;
    return 1.f + c3 * u * u * u + c1 * u * u;
}

std::filesystem::path findAssetRoot() {
    namespace fs = std::filesystem;
    const std::vector<fs::path> candidates = {
        fs::current_path() / "gameViewer" / "assets",
        fs::current_path() / "assets",
        fs::current_path().parent_path() / "gameViewer" / "assets",
        fs::path("gameViewer/assets"),
        fs::path("../gameViewer/assets"),
    };
    for (const auto &base : candidates) {
        if (fs::exists(base / "fonts" / "DejaVuSans.ttf")) {
            return base;
        }
    }
    return fs::path("gameViewer/assets");
}

} // namespace

GameViewer::GameViewer(char humanPlayer)
    : window_(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Othello",
              sf::Style::Titlebar | sf::Style::Close),
      humanPlayer_(humanPlayer),
      aiPlayer_(humanPlayer == PLAYER_X ? PLAYER_O : PLAYER_X),
      currentPlayer_(PLAYER_X) {
    window_.setFramerateLimit(60);
    window_.setVerticalSyncEnabled(true);
    fontsLoaded_ = loadFonts();
    BoardHelper::initBoard(board_);
    setStatus(humanPlayer_ == PLAYER_X ? "Your turn — click a highlighted square."
                                       : "AI opens. Watching the board…");
    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.45f;
    }
}

bool GameViewer::loadFonts() {
    const auto root = findAssetRoot();
    const auto regular = root / "fonts" / "DejaVuSans.ttf";
    const auto bold = root / "fonts" / "DejaVuSans-Bold.ttf";
    if (!font_.loadFromFile(regular.string())) {
        std::cerr << "Failed to load font: " << regular << std::endl;
        return false;
    }
    if (!fontBold_.loadFromFile(bold.string())) {
        fontBold_ = font_;
    }
    return true;
}

void GameViewer::setStatus(const std::string &message) { statusMessage_ = message; }

int GameViewer::run() {
    while (window_.isOpen()) {
        const float dt = clock_.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
    return 0;
}

void GameViewer::processEvents() {
    sf::Event event{};
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window_.close();
            } else if (event.key.code == sf::Keyboard::R && phase_ == Phase::GameOver) {
                resetGame();
            }
        } else if (event.type == sf::Event::MouseButtonPressed &&
                   event.mouseButton.button == sf::Mouse::Left) {
            if (phase_ == Phase::GameOver) {
                const sf::Vector2f mouse(static_cast<float>(event.mouseButton.x),
                                         static_cast<float>(event.mouseButton.y));
                // Restart hit area in overlay button
                if (mouse.x >= 330.f && mouse.x <= 650.f && mouse.y >= 420.f && mouse.y <= 480.f) {
                    resetGame();
                }
            } else if (phase_ == Phase::Playing && currentPlayer_ == humanPlayer_) {
                handleBoardClick(sf::Vector2f(static_cast<float>(event.mouseButton.x),
                                              static_cast<float>(event.mouseButton.y)));
            }
        }
    }
}

void GameViewer::update(float dt) {
    pulseTime_ += dt;
    for (auto &anim : discAnims_) {
        anim.age += dt;
    }
    discAnims_.erase(std::remove_if(discAnims_.begin(), discAnims_.end(),
                                    [](const DiscAnim &a) { return a.age > 0.35f; }),
                     discAnims_.end());

    if (phase_ == Phase::AiThinking && aiMovePending_) {
        aiDelay_ -= dt;
        if (aiDelay_ <= 0.f) {
            playAiMove();
        }
    }
}

void GameViewer::handleBoardClick(sf::Vector2f mousePos) {
    const auto cell = cellFromPoint(mousePos);
    if (!cell) {
        return;
    }
    tryHumanMove(*cell);
}

void GameViewer::tryHumanMove(const Position &move) {
    if (!BoardHelper::isValidMove(board_, move, currentPlayer_)) {
        setStatus("Illegal move — sandwich at least one opponent disc.");
        return;
    }

    BoardHelper::playMove(board_, move, currentPlayer_);
    discAnims_.push_back({move, currentPlayer_, 0.f});

    if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
        phase_ = Phase::GameOver;
        setStatus(outcomeMessage());
        return;
    }

    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.35f;
        setStatus("AI is thinking…");
    } else {
        setStatus("Opponent had no move — your turn again.");
    }
}

void GameViewer::playAiMove() {
    aiMovePending_ = false;
    const Position move = Solver::getBestMovePosition(board_, aiPlayer_, MIN_MAX_DEPTH);

    if (!BoardHelper::isValidMove(board_, move, aiPlayer_)) {
        // Safety: if solver returns something unusable, end or skip.
        if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
            phase_ = Phase::GameOver;
            setStatus(outcomeMessage());
        } else {
            phase_ = Phase::Playing;
            setStatus("Your turn.");
        }
        return;
    }

    BoardHelper::playMove(board_, move, aiPlayer_);
    discAnims_.push_back({move, aiPlayer_, 0.f});

    std::ostringstream oss;
    oss << "AI played {" << move.getRow() << ", " << move.getCol() << "}.";
    setStatus(oss.str());

    if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
        phase_ = Phase::GameOver;
        setStatus(outcomeMessage());
        return;
    }

    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.25f;
        setStatus("You had no move — AI plays again.");
    } else {
        phase_ = Phase::Playing;
        setStatus(statusMessage_ + " Your turn.");
    }
}

void GameViewer::resetGame() {
    BoardHelper::initBoard(board_);
    currentPlayer_ = PLAYER_X;
    phase_ = Phase::Playing;
    discAnims_.clear();
    pulseTime_ = 0.f;
    aiDelay_ = 0.f;
    aiMovePending_ = false;
    setStatus(humanPlayer_ == PLAYER_X ? "New game — click a highlighted square."
                                       : "New game — AI opens.");
    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.45f;
    }
}

std::optional<Position> GameViewer::cellFromPoint(sf::Vector2f point) const {
    if (point.x < BOARD_ORIGIN_X || point.y < BOARD_ORIGIN_Y ||
        point.x >= BOARD_ORIGIN_X + BOARD_PIXEL_SIZE ||
        point.y >= BOARD_ORIGIN_Y + BOARD_PIXEL_SIZE) {
        return std::nullopt;
    }
    const unsigned int col =
        static_cast<unsigned int>((point.x - BOARD_ORIGIN_X) / CELL_SIZE);
    const unsigned int row =
        static_cast<unsigned int>((point.y - BOARD_ORIGIN_Y) / CELL_SIZE);
    if (row >= BOARD_SIZE || col >= BOARD_SIZE) {
        return std::nullopt;
    }
    return Position(row, col);
}

sf::Vector2f GameViewer::cellCenter(const Position &pos) const {
    return {BOARD_ORIGIN_X + (static_cast<float>(pos.getCol()) + 0.5f) * CELL_SIZE,
            BOARD_ORIGIN_Y + (static_cast<float>(pos.getRow()) + 0.5f) * CELL_SIZE};
}

std::string GameViewer::outcomeMessage() const {
    const int x = BoardHelper::countPiecesPlayer(board_, PLAYER_X);
    const int o = BoardHelper::countPiecesPlayer(board_, PLAYER_O);
    const int diff = x - o;
    if (diff == 0) {
        return "Draw — evenly matched.";
    }
    const bool humanWon =
        (diff > 0 && humanPlayer_ == PLAYER_X) || (diff < 0 && humanPlayer_ == PLAYER_O);
    if (humanWon) {
        return "You win by " + std::to_string(std::abs(diff)) + "!";
    }
    return "AI wins by " + std::to_string(std::abs(diff)) + ".";
}

void GameViewer::drawBackground() {
    sf::Vertex sky[] = {
        sf::Vertex(sf::Vector2f(0.f, 0.f), kBgTop),
        sf::Vertex(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), 0.f), kBgTop),
        sf::Vertex(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)),
                   kBgBottom),
        sf::Vertex(sf::Vector2f(0.f, static_cast<float>(WINDOW_HEIGHT)), kBgBottom),
    };
    window_.draw(sky, 4, sf::Quads);

    // Soft radial-ish vignette using translucent circles
    sf::CircleShape glow(420.f);
    glow.setOrigin(420.f, 420.f);
    glow.setPosition(BOARD_ORIGIN_X + BOARD_PIXEL_SIZE * 0.5f,
                     BOARD_ORIGIN_Y + BOARD_PIXEL_SIZE * 0.5f);
    glow.setFillColor(sf::Color(40, 110, 80, 35));
    window_.draw(glow);
}

void GameViewer::drawBoard() {
    sf::RectangleShape frame(sf::Vector2f(BOARD_PIXEL_SIZE + 28.f, BOARD_PIXEL_SIZE + 28.f));
    frame.setPosition(BOARD_ORIGIN_X - 14.f, BOARD_ORIGIN_Y - 14.f);
    frame.setFillColor(kFrame);
    window_.draw(frame);

    sf::RectangleShape frameInner(sf::Vector2f(BOARD_PIXEL_SIZE + 8.f, BOARD_PIXEL_SIZE + 8.f));
    frameInner.setPosition(BOARD_ORIGIN_X - 4.f, BOARD_ORIGIN_Y - 4.f);
    frameInner.setFillColor(kFrameHighlight);
    window_.draw(frameInner);

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(BOARD_ORIGIN_X + col * CELL_SIZE, BOARD_ORIGIN_Y + row * CELL_SIZE);
            cell.setFillColor(((row + col) % 2 == 0) ? kBoardGreen : kBoardGreenDark);
            window_.draw(cell);
        }
    }

    for (int i = 0; i <= BOARD_SIZE; ++i) {
        sf::RectangleShape v(sf::Vector2f(2.f, BOARD_PIXEL_SIZE));
        v.setPosition(BOARD_ORIGIN_X + i * CELL_SIZE - 1.f, BOARD_ORIGIN_Y);
        v.setFillColor(kGridLine);
        window_.draw(v);

        sf::RectangleShape h(sf::Vector2f(BOARD_PIXEL_SIZE, 2.f));
        h.setPosition(BOARD_ORIGIN_X, BOARD_ORIGIN_Y + i * CELL_SIZE - 1.f);
        h.setFillColor(kGridLine);
        window_.draw(h);
    }

    if (fontsLoaded_) {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            sf::Text colLabel(std::to_string(i), font_, 16);
            colLabel.setFillColor(kMuted);
            colLabel.setPosition(BOARD_ORIGIN_X + i * CELL_SIZE + CELL_SIZE * 0.5f - 5.f,
                                 BOARD_ORIGIN_Y + BOARD_PIXEL_SIZE + 8.f);
            window_.draw(colLabel);

            sf::Text rowLabel(std::to_string(i), font_, 16);
            rowLabel.setFillColor(kMuted);
            rowLabel.setPosition(BOARD_ORIGIN_X - 24.f,
                                 BOARD_ORIGIN_Y + i * CELL_SIZE + CELL_SIZE * 0.5f - 10.f);
            window_.draw(rowLabel);
        }
    }
}

void GameViewer::drawValidHints(float pulse) {
    if (phase_ != Phase::Playing || currentPlayer_ != humanPlayer_) {
        return;
    }
    const auto moves = BoardHelper::getAllPossibleMoves(board_, humanPlayer_);
    const float alpha = 90.f + 70.f * (0.5f + 0.5f * std::sin(pulse * 3.2f));
    for (const auto &move : moves) {
        sf::CircleShape hint(10.f);
        hint.setOrigin(10.f, 10.f);
        hint.setPosition(cellCenter(move));
        sf::Color c = kHint;
        c.a = static_cast<sf::Uint8>(std::clamp(alpha, 0.f, 255.f));
        hint.setFillColor(c);
        window_.draw(hint);
    }
}

void GameViewer::drawDiscs() {
    auto animScaleFor = [&](const Position &pos) -> float {
        for (const auto &anim : discAnims_) {
            if (anim.pos == pos) {
                const float t = std::clamp(anim.age / 0.28f, 0.f, 1.f);
                return easeOutBack(t);
            }
        }
        return 1.f;
    };

    for (unsigned int row = 0; row < BOARD_SIZE; ++row) {
        for (unsigned int col = 0; col < BOARD_SIZE; ++col) {
            const char piece = board_[row][col];
            if (piece == EMPTY) {
                continue;
            }
            const Position pos(row, col);
            const float scale = animScaleFor(pos);
            const float radius = 28.f * scale;

            sf::CircleShape shadow(radius);
            shadow.setOrigin(radius, radius);
            auto center = cellCenter(pos);
            shadow.setPosition(center.x + 3.f, center.y + 4.f);
            shadow.setFillColor(sf::Color(0, 0, 0, 70));
            window_.draw(shadow);

            sf::CircleShape disc(radius);
            disc.setOrigin(radius, radius);
            disc.setPosition(center);
            disc.setFillColor(piece == PLAYER_X ? kDiscDark : kDiscLight);
            window_.draw(disc);

            sf::CircleShape gloss(radius * 0.35f);
            gloss.setOrigin(radius * 0.35f, radius * 0.35f);
            gloss.setPosition(center.x - radius * 0.25f, center.y - radius * 0.3f);
            gloss.setFillColor(piece == PLAYER_X ? sf::Color(255, 255, 255, 35)
                                                 : sf::Color(255, 255, 255, 90));
            window_.draw(gloss);
        }
    }
}

void GameViewer::drawSidebar() {
    sf::RectangleShape panel(sf::Vector2f(260.f, BOARD_PIXEL_SIZE + 28.f));
    panel.setPosition(700.f, BOARD_ORIGIN_Y - 14.f);
    panel.setFillColor(kPanel);
    window_.draw(panel);

    if (!fontsLoaded_) {
        return;
    }

    sf::Text brand("OTHELLO", fontBold_, 34);
    brand.setFillColor(kText);
    brand.setPosition(720.f, 50.f);
    window_.draw(brand);

    sf::Text tagline("Minimax · Alpha-Beta", font_, 14);
    tagline.setFillColor(kMuted);
    tagline.setPosition(722.f, 92.f);
    window_.draw(tagline);

    const int xCount = BoardHelper::countPiecesPlayer(board_, PLAYER_X);
    const int oCount = BoardHelper::countPiecesPlayer(board_, PLAYER_O);

    auto drawScoreRow = [&](float y, char player, int count, const std::string &label) {
        sf::CircleShape disc(14.f);
        disc.setPosition(722.f, y);
        disc.setFillColor(player == PLAYER_X ? kDiscDark : kDiscLight);
        window_.draw(disc);

        sf::Text name(label, font_, 18);
        name.setFillColor(kText);
        name.setPosition(760.f, y - 2.f);
        window_.draw(name);

        sf::Text score(std::to_string(count), fontBold_, 28);
        score.setFillColor(kAccent);
        score.setPosition(900.f, y - 8.f);
        window_.draw(score);
    };

    const std::string xLabel =
        std::string("Dark (X)") + (humanPlayer_ == PLAYER_X ? " · you" : " · AI");
    const std::string oLabel =
        std::string("Light (O)") + (humanPlayer_ == PLAYER_O ? " · you" : " · AI");
    drawScoreRow(150.f, PLAYER_X, xCount, xLabel);
    drawScoreRow(210.f, PLAYER_O, oCount, oLabel);

    sf::Text turnTitle("Turn", font_, 14);
    turnTitle.setFillColor(kMuted);
    turnTitle.setPosition(722.f, 290.f);
    window_.draw(turnTitle);

    std::string turnText;
    if (phase_ == Phase::GameOver) {
        turnText = "Game over";
    } else if (phase_ == Phase::AiThinking) {
        turnText = "AI thinking";
    } else if (currentPlayer_ == humanPlayer_) {
        turnText = "Your move";
    } else {
        turnText = "AI move";
    }
    sf::Text turn(turnText, fontBold_, 24);
    turn.setFillColor(phase_ == Phase::AiThinking ? kAccent : kText);
    turn.setPosition(722.f, 312.f);
    window_.draw(turn);

    sf::Text status(statusMessage_, font_, 16);
    status.setFillColor(kText);
    status.setPosition(722.f, 380.f);
    // Simple wrap
    std::string wrapped;
    std::string word;
    std::istringstream iss(statusMessage_);
    float lineWidth = 0.f;
    while (iss >> word) {
        sf::Text probe(word + " ", font_, 16);
        if (lineWidth + probe.getLocalBounds().width > 220.f && !wrapped.empty()) {
            wrapped += "\n";
            lineWidth = 0.f;
        } else if (!wrapped.empty()) {
            wrapped += " ";
        }
        wrapped += word;
        lineWidth += probe.getLocalBounds().width;
    }
    status.setString(wrapped);
    window_.draw(status);

    sf::Text help("Click a glowing cell to play.\nEsc quits · R restarts\nwhen the match ends.",
                  font_, 14);
    help.setFillColor(kMuted);
    help.setPosition(722.f, 540.f);
    window_.draw(help);
}

void GameViewer::drawGameOverOverlay() {
    if (phase_ != Phase::GameOver) {
        return;
    }

    sf::RectangleShape veil(sf::Vector2f(static_cast<float>(WINDOW_WIDTH),
                                         static_cast<float>(WINDOW_HEIGHT)));
    veil.setFillColor(sf::Color(6, 14, 12, 160));
    window_.draw(veil);

    sf::RectangleShape card(sf::Vector2f(420.f, 240.f));
    card.setPosition(280.f, 240.f);
    card.setFillColor(sf::Color(16, 34, 30, 245));
    window_.draw(card);

    if (!fontsLoaded_) {
        return;
    }

    sf::Text title("Match complete", fontBold_, 30);
    title.setFillColor(kText);
    title.setPosition(320.f, 270.f);
    window_.draw(title);

    sf::Text result(outcomeMessage(), font_, 20);
    result.setFillColor(kAccent);
    result.setPosition(320.f, 325.f);
    window_.draw(result);

    const int x = BoardHelper::countPiecesPlayer(board_, PLAYER_X);
    const int o = BoardHelper::countPiecesPlayer(board_, PLAYER_O);
    sf::Text tally("X " + std::to_string(x) + "  —  O " + std::to_string(o), font_, 18);
    tally.setFillColor(kMuted);
    tally.setPosition(320.f, 365.f);
    window_.draw(tally);

    sf::RectangleShape button(sf::Vector2f(320.f, 52.f));
    button.setPosition(330.f, 420.f);
    button.setFillColor(kAccent);
    window_.draw(button);

    sf::Text buttonLabel("Play again  (R)", fontBold_, 20);
    buttonLabel.setFillColor(sf::Color(28, 24, 16));
    buttonLabel.setPosition(390.f, 432.f);
    window_.draw(buttonLabel);
}

void GameViewer::render() {
    window_.clear();
    drawBackground();
    drawBoard();
    drawValidHints(pulseTime_);
    drawDiscs();
    drawSidebar();
    drawGameOverOverlay();
    window_.display();
}
