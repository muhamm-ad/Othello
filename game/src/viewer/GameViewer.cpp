#include "viewer/GameViewer.hpp"
#include "Solver.hpp"
#include "viewer/Style.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

GameViewer::GameViewer()
    : window_(sf::VideoMode({DEFAULT_WIDTH, DEFAULT_HEIGHT}), "Othello",
              sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize),
      phase_(Phase::Setup) {
    window_.setFramerateLimit(60);
    window_.setVerticalSyncEnabled(true);
    window_.setMinimumSize(std::optional<sf::Vector2u>{{MIN_WIDTH, MIN_HEIGHT}});
    fontsLoaded_ = loadFonts();
    viewer::generateDiscTextures(discDarkTex_, discLightTex_, discShadowTex_);
    BoardHelper::initBoard(board_);
}

bool GameViewer::loadFonts() {
    const auto root = viewer::findAssetRoot();
    const auto regular = root / "fonts" / "DejaVuSans.ttf";
    const auto bold = root / "fonts" / "DejaVuSans-Bold.ttf";
    if (!font_.openFromFile(regular)) {
        std::cerr << "Failed to load font: " << regular << std::endl;
        return false;
    }
    if (!fontBold_.openFromFile(bold)) {
        fontBold_ = font_;
    }
    return true;
}

void GameViewer::setStatus(const std::string &message) { statusMessage_ = message; }

size_t GameViewer::depthForDifficulty(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Casual:
            return 1;
        case Difficulty::Steady:
            return 3;
        case Difficulty::Sharp:
            return 5;
        case Difficulty::Ruthless:
            return 7;
    }
    return 6;
}

const char *GameViewer::difficultyLabel(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Casual:
            return "Casual";
        case Difficulty::Steady:
            return "Steady";
        case Difficulty::Sharp:
            return "Sharp";
        case Difficulty::Ruthless:
            return "Ruthless";
    }
    return "Sharp";
}

void GameViewer::switchTo(char player) {
    currentPlayer_ = player;
    turnAnim_ = 0.f;
}

void GameViewer::pushHistory() { history_.push_back({board_, currentPlayer_}); }

void GameViewer::startGame() {
    humanPlayer_ = selectedPiece_;
    aiPlayer_ = opponentOf(humanPlayer_);
    aiDepth_ = depthForDifficulty(selectedDifficulty_);

    BoardHelper::initBoard(board_);
    dropAnims_.clear();
    flipAnims_.clear();
    ringAnims_.clear();
    history_.clear();
    pulseTime_ = 0.f;
    aiDelay_ = 0.f;
    aiMovePending_ = false;
    turnAnim_ = 1.f;

    switchTo(PLAYER_BLACK);
    if (humanPlayer_ == PLAYER_BLACK) {
        phase_ = Phase::Playing;
        setStatus("Your move.");
    } else {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.45f;
        setStatus("Engine opens the game.");
    }
}

void GameViewer::resetGame() {
    // The setup card is shown only before the first move (per the design
    // spec); "New game" / "Play again" replay instantly with the same
    // difficulty, side and name rather than reopening it.
    startGame();
}

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
    while (const std::optional event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            if (confirmKind_ == ConfirmKind::Quit) {
                window_.close();
            } else {
                requestConfirm(ConfirmKind::Quit);
            }
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            const unsigned int width = std::max(resized->size.x, MIN_WIDTH);
            const unsigned int height = std::max(resized->size.y, MIN_HEIGHT);
            if (width != resized->size.x || height != resized->size.y) {
                window_.setSize({width, height});
            }
            const auto size = window_.getSize();
            window_.setView(sf::View(sf::FloatRect({0.f, 0.f}, {
                static_cast<float>(size.x),
                static_cast<float>(size.y)
            })));
        } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (confirmKind_ != ConfirmKind::None) {
                handleConfirmKey(keyPressed->code);
            } else if (keyPressed->code == sf::Keyboard::Key::Escape ||
                       (keyPressed->code == sf::Keyboard::Key::Q &&
                        !(phase_ == Phase::Setup && nameFocused_))) {
                requestConfirm(ConfirmKind::Quit);
            } else if (phase_ == Phase::Setup) {
                handleSetupKey(keyPressed->code);
            } else if (keyPressed->code == sf::Keyboard::Key::R &&
                       (phase_ == Phase::GameOver || phase_ == Phase::Playing ||
                        phase_ == Phase::AiThinking)) {
                requestConfirm(ConfirmKind::Restart);
            } else if (keyPressed->code == sf::Keyboard::Key::U &&
                       (phase_ == Phase::Playing || phase_ == Phase::GameOver)) {
                undoLastMove();
            }
        } else if (const auto *textEntered = event->getIf<sf::Event::TextEntered>()) {
            if (phase_ == Phase::Setup && nameFocused_ && confirmKind_ == ConfirmKind::None) {
                handleTextEntered(textEntered->unicode);
            }
        } else if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                const sf::Vector2f mouse(static_cast<float>(mousePressed->position.x),
                                         static_cast<float>(mousePressed->position.y));
                if (confirmKind_ != ConfirmKind::None) {
                    handleConfirmClick(mouse);
                } else if (phase_ == Phase::Setup) {
                    handleSetupClick(mouse);
                } else if (phase_ == Phase::GameOver) {
                    handleGameOverClick(mouse);
                } else if (phase_ == Phase::Playing && currentPlayer_ == humanPlayer_) {
                    handleBoardClick(mouse);
                }
            }
        }
    }
}

void GameViewer::update(float dt) {
    pulseTime_ += dt;
    caretTime_ += dt;

    for (auto &anim: dropAnims_) {
        anim.age += dt;
    }
    dropAnims_.erase(std::remove_if(dropAnims_.begin(), dropAnims_.end(),
                                    [](const DropAnim &a) { return a.age > 0.3f; }),
                     dropAnims_.end());

    for (auto &anim: flipAnims_) {
        anim.age += dt;
    }
    flipAnims_.erase(std::remove_if(flipAnims_.begin(), flipAnims_.end(),
                                    [](const FlipAnim &a) { return a.age > a.delay + 0.32f; }),
                     flipAnims_.end());

    for (auto &anim: ringAnims_) {
        anim.age += dt;
    }
    ringAnims_.erase(std::remove_if(ringAnims_.begin(), ringAnims_.end(),
                                    [](const RingAnim &a) { return a.age > 0.6f; }),
                     ringAnims_.end());

    turnAnim_ = std::min(1.f, turnAnim_ + dt / 0.14f);

    if (phase_ == Phase::GameOver && !gameOverAnimStarted_) {
        gameOverAnimStarted_ = true;
        gameOverDelay_ = 0.06f;
        gameOverAnim_ = 0.f;
    }
    if (phase_ != Phase::GameOver) {
        gameOverAnimStarted_ = false;
    } else if (gameOverDelay_ > 0.f) {
        gameOverDelay_ -= dt;
    } else {
        gameOverAnim_ = std::min(1.f, gameOverAnim_ + dt / 0.26f);
    }

    if (confirmKind_ == ConfirmKind::None && phase_ == Phase::AiThinking && aiMovePending_) {
        aiDelay_ -= dt;
        if (aiDelay_ <= 0.f) {
            playAiMove();
        }
    }
}

int GameViewer::spawnMoveAnims(const Position &move, const std::vector<std::vector<char>> &before,
                               char player, bool withRing) {
    dropAnims_.push_back({move, player, 0.f});
    if (withRing) {
        ringAnims_.push_back({move, 0.f});
    }

    int flipped = 0;
    for (unsigned int r = 0; r < BOARD_SIZE; ++r) {
        for (unsigned int c = 0; c < BOARD_SIZE; ++c) {
            if (r == move.getRow() && c == move.getCol()) {
                continue;
            }
            if (before[r][c] != EMPTY && before[r][c] != board_[r][c]) {
                const Position p(r, c);
                const int dist = std::max(std::abs(static_cast<int>(r) - static_cast<int>(move.getRow())),
                                          std::abs(static_cast<int>(c) - static_cast<int>(move.getCol())));
                flipAnims_.push_back({
                    p, before[r][c], board_[r][c],
                    static_cast<float>(std::max(0, dist - 1)) * 0.045f, 0.f
                });
                ++flipped;
            }
        }
    }
    return flipped;
}

void GameViewer::tryHumanMove(const Position &move) {
    if (!BoardHelper::isValidMove(board_, move, currentPlayer_)) {
        setStatus("That square doesn't flip anything.");
        return;
    }

    const auto before = board_;
    pushHistory();
    BoardHelper::playMove(board_, move, currentPlayer_);
    const int flipped = spawnMoveAnims(move, before, currentPlayer_, true);

    std::ostringstream oss;
    oss << "You played " << static_cast<char>('a' + move.getCol()) << (move.getRow() + 1)
            << ", flipping " << flipped << (flipped == 1 ? " disc." : " discs.");
    setStatus(oss.str());

    if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
        turnAnim_ = 1.f;
        phase_ = Phase::GameOver;
        setStatus(outcomeMessage());
        return;
    }
    switchTo(currentPlayer_);

    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.35f;
    } else {
        setStatus(oss.str() + " No legal move for the engine - turn passes back.");
    }
}

void GameViewer::playAiMove() {
    aiMovePending_ = false;
    const Position move = Solver::getBestMovePosition(board_, aiPlayer_, static_cast<int>(aiDepth_));

    if (!BoardHelper::isValidMove(board_, move, aiPlayer_)) {
        if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
            phase_ = Phase::GameOver;
            setStatus(outcomeMessage());
        } else {
            switchTo(currentPlayer_);
            phase_ = Phase::Playing;
            setStatus("No legal move for the engine - your turn.");
        }
        return;
    }

    const auto before = board_;
    pushHistory();
    BoardHelper::playMove(board_, move, aiPlayer_);
    const int flipped = spawnMoveAnims(move, before, aiPlayer_, false);

    std::ostringstream oss;
    oss << "Engine took " << static_cast<char>('a' + move.getCol()) << (move.getRow() + 1)
            << " and flipped " << flipped << (flipped == 1 ? " disc." : " discs.");

    if (!BoardHelper::switchPlayer(board_, currentPlayer_)) {
        turnAnim_ = 1.f;
        phase_ = Phase::GameOver;
        setStatus(outcomeMessage());
        return;
    }
    switchTo(currentPlayer_);

    if (currentPlayer_ == aiPlayer_) {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.25f;
        setStatus("No legal move for you - the engine plays again.");
    } else {
        phase_ = Phase::Playing;
        setStatus(oss.str());
    }
}

void GameViewer::undoLastMove() {
    if (history_.empty()) {
        return;
    }
    // Undo the human's last move together with the engine's reply, so control
    // returns to the human — matches the "Undo last move" shortcut in the sidebar.
    HistoryEntry restored = history_.back();
    history_.pop_back();
    while (!history_.empty() && restored.player != humanPlayer_) {
        restored = history_.back();
        history_.pop_back();
    }
    board_ = restored.board;
    switchTo(restored.player);
    dropAnims_.clear();
    flipAnims_.clear();
    ringAnims_.clear();
    if (restored.player == humanPlayer_) {
        phase_ = Phase::Playing;
        aiMovePending_ = false;
    } else {
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.4f;
    }
    setStatus("Move undone.");
}

std::pair<int, int> GameViewer::pieceCounts() const {
    return {BoardHelper::countPlayerPieces(board_, PLAYER_BLACK),
            BoardHelper::countPlayerPieces(board_, PLAYER_WHITE)};
}

std::string GameViewer::outcomeMessage() const {
    const auto [blackCount, whiteCount] = pieceCounts();
    const int diff = blackCount - whiteCount;
    if (diff == 0) {
        return "Draw, " + std::to_string(blackCount) + " to " + std::to_string(whiteCount) +
               ". Press R for a new game.";
    }
    const bool humanWon =
            (diff > 0 && humanPlayer_ == PLAYER_BLACK) || (diff < 0 && humanPlayer_ == PLAYER_WHITE);
    const int winnerScore = std::max(blackCount, whiteCount);
    const int loserScore = std::min(blackCount, whiteCount);
    if (humanWon) {
        return "You win, " + std::to_string(winnerScore) + " to " + std::to_string(loserScore) +
               ". Press R for a new game.";
    }
    return "The engine wins, " + std::to_string(winnerScore) + " to " + std::to_string(loserScore) +
           ". Press R for a new game.";
}

std::string GameViewer::resultHeadline() const {
    const auto [blackCount, whiteCount] = pieceCounts();
    const int diff = blackCount - whiteCount;
    if (diff == 0) {
        return "DRAW";
    }
    const bool humanWon =
            (diff > 0 && humanPlayer_ == PLAYER_BLACK) || (diff < 0 && humanPlayer_ == PLAYER_WHITE);
    return humanWon ? "YOU WIN" : "YOU LOSE";
}

void GameViewer::render() {
    window_.clear(viewer::kShellBottom);
    if (phase_ == Phase::Setup) {
        drawSetupScreen();
    } else {
        const Layout layout = computeLayout();
        drawMainWindow(layout);
        if (phase_ == Phase::GameOver) {
            drawGameOverOverlay();
        }
    }
    if (confirmKind_ != ConfirmKind::None) {
        drawConfirmDialog();
    }
    window_.display();
}
