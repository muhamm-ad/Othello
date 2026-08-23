#pragma once

#include "BoardHelper.hpp"
#include "Player.hpp"

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief SFML window that renders the board ("pressed felt & bone" visual
 * direction) and drives human vs AI play.
 */
class GameViewer {
public:
    GameViewer();

    /**
     * @brief Runs the window event/render loop until the player quits.
     * @return Process exit code (0 on normal close).
     */
    int run();

private:
    enum class Phase {
        Setup,
        Playing,
        AiThinking,
        GameOver
    };

    enum class Difficulty {
        Casual,
        Steady,
        Sharp,
        Ruthless
    };

    struct DropAnim {
        Position pos;
        char player;
        float age = 0.f;
    };

    struct FlipAnim {
        Position pos;
        char from;
        char to;
        float delay = 0.f;
        float age = 0.f;
    };

    struct RingAnim {
        Position pos;
        float age = 0.f;
    };

    struct HistoryEntry {
        std::vector<std::vector<char> > board;
        char player;
    };

    /** @brief Live geometry recomputed from the current window size every frame. */
    struct Layout {
        float pad = 44.f;
        float side = 332.f;
        float gap = 36.f;
        float cell = 78.f;
        float field = 624.f;
        float plinth = 672.f;
        float boardX = 0.f;
        float boardY = 0.f;
        float sidebarX = 0.f;
        float sidebarY = 0.f;
        float fontScale = 1.f;
    };

    struct SetupLayout {
        sf::FloatRect card;
        float scale = 1.f;
        sf::Vector2f wordmarkPos;
        sf::Vector2f taglinePos;
        float dividerY = 0.f;
        sf::Vector2f nameLabelPos;
        sf::FloatRect nameField;
        sf::Vector2f diffLabelPos;
        sf::FloatRect diffTrack;
        sf::FloatRect diff[4];
        sf::Vector2f playLabelPos;
        sf::FloatRect side[2];
        sf::FloatRect start;
        float hintY = 0.f;
    };

    struct GameOverLayout {
        sf::FloatRect card;
        sf::FloatRect playAgain;
        sf::FloatRect quit;
    };

    static constexpr int BOARD_SIZE = 8;
    static constexpr unsigned int DEFAULT_WIDTH = 1120;
    static constexpr unsigned int DEFAULT_HEIGHT = 760;
    static constexpr unsigned int MIN_WIDTH = 880;
    static constexpr unsigned int MIN_HEIGHT = 640;
    static constexpr float COORD_BAND = 24.f;
    static constexpr size_t NAME_MAX_LEN = 16;

    void processEvents();

    void update(float dt);

    void render();

    [[nodiscard]] Layout computeLayout() const;

    [[nodiscard]] SetupLayout computeSetupLayout() const;

    [[nodiscard]] GameOverLayout computeGameOverLayout() const;

    void handleBoardClick(sf::Vector2f mousePos);

    void handleSetupClick(sf::Vector2f mousePos);

    void handleSetupKey(sf::Keyboard::Key key);

    void handleTextEntered(char32_t unicode);

    void handleGameOverClick(sf::Vector2f mousePos);

    void tryHumanMove(const Position &move);

    void playAiMove();

    void startGame();

    void resetGame();

    void undoLastMove();

    void pushHistory();

    void switchTo(char player);

    [[nodiscard]] std::optional<Position> cellFromPoint(sf::Vector2f point) const;

    [[nodiscard]] static sf::Vector2f cellCenter(const Position &pos, const Layout &layout);

    [[nodiscard]] std::string outcomeMessage() const;

    [[nodiscard]] std::string resultHeadline() const;

    [[nodiscard]] static size_t depthForDifficulty(Difficulty difficulty);

    [[nodiscard]] static const char *difficultyLabel(Difficulty difficulty);

    void drawSetupScreen();

    void drawMainWindow(const Layout &layout);

    void drawBoardPlinth(const Layout &layout);

    void drawDiscs(const Layout &layout);

    void drawValidHints(const Layout &layout);

    void drawSidebar(const Layout &layout);

    void drawGameOverOverlay();

    void generateTextures();

    bool loadFonts();

    void setStatus(const std::string &message);

    sf::RenderWindow window_;
    sf::Font font_;
    sf::Font fontBold_;
    bool fontsLoaded_ = false;

    sf::Texture discDarkTex_;
    sf::Texture discLightTex_;
    sf::Texture discShadowTex_;

    std::vector<std::vector<char> > board_;
    char humanPlayer_ = PLAYER_BLACK;
    char aiPlayer_ = PLAYER_WHITE;
    char currentPlayer_ = PLAYER_BLACK;
    Phase phase_ = Phase::Setup;
    size_t aiDepth_ = 6;

    Difficulty selectedDifficulty_ = Difficulty::Sharp;
    char selectedPiece_ = PLAYER_BLACK;
    std::string playerName_;
    bool nameFocused_ = true;
    float caretTime_ = 0.f;

    std::string statusMessage_;
    std::vector<DropAnim> dropAnims_;
    std::vector<FlipAnim> flipAnims_;
    std::vector<RingAnim> ringAnims_;
    std::vector<HistoryEntry> history_;

    float pulseTime_ = 0.f;
    float aiDelay_ = 0.f;
    bool aiMovePending_ = false;
    float turnAnim_ = 1.f;
    float gameOverDelay_ = 0.f;
    float gameOverAnim_ = 0.f;
    bool gameOverAnimStarted_ = false;

    sf::Clock clock_;
};
