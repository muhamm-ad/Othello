#include "GameViewer.hpp"
#include "Solver.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>

// Visual direction: "Pressed felt & bone" — a warm paper shell, a raised felt
// board plinth, chunky domed discs, and a single clay accent carrying every
// piece of live information. See design/Othello UI.dc.html for the full spec.

namespace {
    // ---------------------------------------------------------------- palette --
    const sf::Color kShellTop(241, 237, 230);
    const sf::Color kShellBottom(230, 224, 214);
    const sf::Color kSurface(247, 244, 238);
    const sf::Color kSurfaceRaised(255, 253, 248);
    const sf::Color kPlinthTop(240, 234, 224);
    const sf::Color kPlinthBottom(230, 223, 211);
    const sf::Color kSquareLight(203, 211, 196);
    const sf::Color kSquareDark(183, 193, 176);
    const sf::Color kClay(196, 113, 75);
    const sf::Color kClayText(176, 101, 63);
    const sf::Color kInk(51, 50, 45);
    const sf::Color kBody(110, 105, 95);
    const sf::Color kMuted(154, 146, 133);
    const sf::Color kEyebrow(139, 133, 122);
    const sf::Color kHelperMuted(169, 161, 146);
    const sf::Color kHairline(226, 219, 206);
    const sf::Color kSunken(241, 237, 229);
    const sf::Color kKeycapBg(234, 228, 218);
    const sf::Color kKeycapEdge(216, 209, 197);
    const sf::Color kDiscDarkBase(46, 45, 40);
    const sf::Color kDiscDarkMid(60, 59, 53);
    const sf::Color kDiscDarkHi(87, 85, 76);
    const sf::Color kDiscLightBase(231, 223, 207);
    const sf::Color kDiscLightMid(248, 243, 233);
    const sf::Color kDiscLightHi(255, 254, 250);

    constexpr int kDiscTexSize = 256;

    // ------------------------------------------------------------- small math --
    float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

    float easeOutBack(float t) {
        constexpr float c1 = 1.70158f;
        constexpr float c3 = c1 + 1.f;
        const float u = t - 1.f;
        return 1.f + c3 * u * u * u + c1 * u * u;
    }

    float easeOutQuad(float t) { return 1.f - (1.f - t) * (1.f - t); }
    float easeInOutSine(float t) { return 0.5f - 0.5f * std::cos(3.14159265f * t); }

    float smoothstep(float edge0, float edge1, float x) {
        const float t = clampf((x - edge0) / (edge1 - edge0), 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

    sf::Color lerpColor(const sf::Color &a, const sf::Color &b, float t) {
        t = clampf(t, 0.f, 1.f);
        return sf::Color(
            static_cast<std::uint8_t>(a.r + (b.r - a.r) * t),
            static_cast<std::uint8_t>(a.g + (b.g - a.g) * t),
            static_cast<std::uint8_t>(a.b + (b.b - a.b) * t),
            static_cast<std::uint8_t>(a.a + (b.a - a.a) * t));
    }

    sf::Color withAlpha(sf::Color c, float alpha01) {
        c.a = static_cast<std::uint8_t>(clampf(alpha01, 0.f, 1.f) * 255.f);
        return c;
    }

    bool pointInRect(sf::Vector2f p, const sf::FloatRect &r) {
        return p.x >= r.position.x && p.x <= r.position.x + r.size.x && p.y >= r.position.y &&
               p.y <= r.position.y + r.size.y;
    }

    // ---------------------------------------------------------- rounded rects --
    sf::ConvexShape roundedRectShape(sf::Vector2f size, float radius, int cornerPoints = 6) {
        radius = std::max(0.f, std::min(radius, std::min(size.x, size.y) * 0.5f));
        sf::ConvexShape shape;
        shape.setPointCount(static_cast<std::size_t>(cornerPoints) * 4);
        struct Corner {
            float cx, cy, startDeg;
        };
        const Corner corners[4] = {
            {size.x - radius, radius, -90.f},
            {size.x - radius, size.y - radius, 0.f},
            {radius, size.y - radius, 90.f},
            {radius, radius, 180.f},
        };
        int idx = 0;
        for (const auto &c: corners) {
            for (int i = 0; i < cornerPoints; ++i) {
                const float deg = c.startDeg + 90.f * static_cast<float>(i) /
                                  static_cast<float>(std::max(1, cornerPoints - 1));
                const float rad = deg * 3.14159265f / 180.f;
                shape.setPoint(static_cast<std::size_t>(idx++),
                               {c.cx + std::cos(rad) * radius, c.cy + std::sin(rad) * radius});
            }
        }
        return shape;
    }

    void drawRoundedRect(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                         sf::Color fill, sf::Color outline = sf::Color::Transparent,
                         float outlineThickness = 0.f) {
        sf::ConvexShape shape = roundedRectShape(size, radius);
        shape.setPosition(pos);
        shape.setFillColor(fill);
        if (outlineThickness != 0.f) {
            shape.setOutlineColor(outline);
            shape.setOutlineThickness(outlineThickness);
        }
        target.draw(shape);
    }

    void drawRoundedRectGradient(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size,
                                 float radius, sf::Color top, sf::Color bottom, int cornerPoints = 6) {
        sf::ConvexShape ref = roundedRectShape(size, radius, cornerPoints);
        const std::size_t perim = ref.getPointCount();
        sf::VertexArray fan(sf::PrimitiveType::TriangleFan, perim + 2);
        const sf::Color centerColor = lerpColor(top, bottom, 0.5f);
        fan[0].position = pos + sf::Vector2f(size.x * 0.5f, size.y * 0.5f);
        fan[0].color = centerColor;
        for (std::size_t i = 0; i < perim; ++i) {
            const sf::Vector2f p = ref.getPoint(i);
            fan[i + 1].position = pos + p;
            fan[i + 1].color = lerpColor(top, bottom, clampf(p.y / size.y, 0.f, 1.f));
        }
        fan[perim + 1].position = fan[1].position;
        fan[perim + 1].color = fan[1].color;
        target.draw(fan);
    }

    void drawSoftShadow(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                        float offsetY, float blur, float peakAlpha, int layers = 5) {
        for (int i = 0; i < layers; ++i) {
            const float t = static_cast<float>(i) / static_cast<float>(layers);
            const float grow = blur * t;
            const sf::Vector2f layerSize(size.x + grow * 2.f, size.y + grow * 2.f);
            const sf::Vector2f layerPos(pos.x - grow, pos.y - grow + offsetY);
            const float layerAlpha = peakAlpha * (1.f - t) * (1.f - t);
            drawRoundedRect(target, layerPos, layerSize, radius + grow,
                            withAlpha(sf::Color(60, 52, 40), layerAlpha));
        }
    }

    // --------------------------------------------------------------- text ops --
    std::vector<std::string> wrapText(const sf::Font &font, const std::string &str,
                                      unsigned int charSize, float maxWidth) {
        std::vector<std::string> lines;
        std::istringstream iss(str);
        std::string word, current;
        float lineWidth = 0.f;
        while (iss >> word) {
            sf::Text probe(font, word + " ", charSize);
            const float wordWidth = probe.getLocalBounds().size.x;
            if (!current.empty() && lineWidth + wordWidth > maxWidth) {
                lines.push_back(current);
                current.clear();
                lineWidth = 0.f;
            }
            current += (current.empty() ? "" : " ") + word;
            lineWidth += wordWidth;
        }
        if (!current.empty()) {
            lines.push_back(current);
        }
        return lines;
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
        for (const auto &base: candidates) {
            if (fs::exists(base / "fonts" / "DejaVuSans.ttf")) {
                return base;
            }
        }
        return fs::path("gameViewer/assets");
    }
} // namespace

GameViewer::GameViewer()
    : window_(sf::VideoMode({DEFAULT_WIDTH, DEFAULT_HEIGHT}), "Othello",
              sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize),
      phase_(Phase::Setup) {
    window_.setFramerateLimit(60);
    window_.setVerticalSyncEnabled(true);
    window_.setMinimumSize(sf::Vector2u(MIN_WIDTH, MIN_HEIGHT));
    fontsLoaded_ = loadFonts();
    generateTextures();
    BoardHelper::initBoard(board_);
}

bool GameViewer::loadFonts() {
    const auto root = findAssetRoot();
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

void GameViewer::generateTextures() {
    // Two hand-authored "domes": a radial highlight sitting at (34%,26%) of the
    // disc, matching the design's radial-gradient(120% 120% at 34% 26%, ...).
    auto buildDisc = [](sf::Color c0, sf::Color c1, float stop1, sf::Color c2) {
        sf::Image image({static_cast<unsigned int>(kDiscTexSize), static_cast<unsigned int>(kDiscTexSize)},
                        sf::Color::Transparent);
        const float size = static_cast<float>(kDiscTexSize);
        const float ex = 0.34f * size;
        const float ey = 0.26f * size;
        const float er = 0.6f * size;
        const float cx = size * 0.5f;
        const float cy = size * 0.5f;
        const float edge = size * 0.5f;
        for (unsigned int y = 0; y < static_cast<unsigned int>(kDiscTexSize); ++y) {
            for (unsigned int x = 0; x < static_cast<unsigned int>(kDiscTexSize); ++x) {
                const float px = static_cast<float>(x) + 0.5f;
                const float py = static_cast<float>(y) + 0.5f;
                const float dx = (px - ex) / er;
                const float dy = (py - ey) / er;
                float t = std::sqrt(dx * dx + dy * dy);
                t = clampf(t, 0.f, 1.f);
                sf::Color col = (t <= stop1)
                                    ? lerpColor(c0, c1, t / stop1)
                                    : lerpColor(c1, c2, (t - stop1) / (1.f - stop1));
                const float dcx = px - cx;
                const float dcy = py - cy;
                const float dist = std::sqrt(dcx * dcx + dcy * dcy);
                const float alpha = 1.f - smoothstep(edge - 1.5f, edge, dist);
                col.a = static_cast<std::uint8_t>(alpha * 255.f);
                image.setPixel({x, y}, col);
            }
        }
        sf::Texture tex;
        (void) tex.loadFromImage(image);
        tex.setSmooth(true);
        return tex;
    };

    discDarkTex_ = buildDisc(kDiscDarkHi, kDiscDarkMid, 0.48f, kDiscDarkBase);
    discLightTex_ = buildDisc(kDiscLightHi, kDiscLightMid, 0.52f, kDiscLightBase);

    // Soft circular drop shadow: solid core, blurred edge.
    {
        sf::Image image({static_cast<unsigned int>(kDiscTexSize), static_cast<unsigned int>(kDiscTexSize)},
                        sf::Color::Transparent);
        const float size = static_cast<float>(kDiscTexSize);
        const float cx = size * 0.5f;
        const float cy = size * 0.5f;
        const float radius = size * 0.5f;
        for (unsigned int y = 0; y < static_cast<unsigned int>(kDiscTexSize); ++y) {
            for (unsigned int x = 0; x < static_cast<unsigned int>(kDiscTexSize); ++x) {
                const float px = static_cast<float>(x) + 0.5f - cx;
                const float py = static_cast<float>(y) + 0.5f - cy;
                const float t = clampf(std::sqrt(px * px + py * py) / radius, 0.f, 1.f);
                const float alpha = (t < 0.55f) ? 1.f : 1.f - smoothstep(0.55f, 1.f, t);
                image.setPixel({x, y}, sf::Color(0, 0, 0, static_cast<std::uint8_t>(alpha * 255.f)));
            }
        }
        (void) discShadowTex_.loadFromImage(image);
        discShadowTex_.setSmooth(true);
    }
}

void GameViewer::setStatus(const std::string &message) { statusMessage_ = message; }

size_t GameViewer::depthForDifficulty(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::Casual:
            return 2;
        case Difficulty::Steady:
            return 4;
        case Difficulty::Sharp:
            return 6;
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
            window_.close();
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            const sf::FloatRect visibleArea({0.f, 0.f},
                                            {
                                                static_cast<float>(resized->size.x),
                                                static_cast<float>(resized->size.y)
                                            });
            window_.setView(sf::View(visibleArea));
        } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                window_.close();
            } else if (phase_ == Phase::Setup) {
                handleSetupKey(keyPressed->code);
            } else if (keyPressed->code == sf::Keyboard::Key::R &&
                       (phase_ == Phase::GameOver || phase_ == Phase::Playing ||
                        phase_ == Phase::AiThinking)) {
                resetGame();
            } else if (keyPressed->code == sf::Keyboard::Key::U &&
                       (phase_ == Phase::Playing || phase_ == Phase::GameOver)) {
                undoLastMove();
            }
        } else if (const auto *textEntered = event->getIf<sf::Event::TextEntered>()) {
            if (phase_ == Phase::Setup && nameFocused_) {
                handleTextEntered(textEntered->unicode);
            }
        } else if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                const sf::Vector2f mouse(static_cast<float>(mousePressed->position.x),
                                         static_cast<float>(mousePressed->position.y));
                if (phase_ == Phase::Setup) {
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

void GameViewer::handleTextEntered(char32_t unicode) {
    if (unicode == 8) {
        // backspace
        if (!playerName_.empty()) {
            playerName_.pop_back();
        }
        return;
    }
    if (unicode < 32 || unicode == 127) {
        return;
    }
    if (playerName_.size() >= NAME_MAX_LEN) {
        return;
    }
    if (unicode < 128) {
        playerName_.push_back(static_cast<char>(unicode));
    }
}

void GameViewer::handleSetupKey(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::Enter) {
        nameFocused_ = false;
        startGame();
        return;
    }
    if (key == sf::Keyboard::Key::Tab) {
        nameFocused_ = !nameFocused_;
        return;
    }
    if (nameFocused_) {
        return;
    }
    switch (key) {
        case sf::Keyboard::Key::Num1:
            selectedDifficulty_ = Difficulty::Casual;
            break;
        case sf::Keyboard::Key::Num2:
            selectedDifficulty_ = Difficulty::Steady;
            break;
        case sf::Keyboard::Key::Num3:
            selectedDifficulty_ = Difficulty::Sharp;
            break;
        case sf::Keyboard::Key::Num4:
            selectedDifficulty_ = Difficulty::Ruthless;
            break;
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::Right: {
            const bool dark = selectedPiece_ == PLAYER_BLACK;
            selectedPiece_ = dark ? PLAYER_WHITE : PLAYER_BLACK;
            break;
        }
        default:
            break;
    }
}

GameViewer::SetupLayout GameViewer::computeSetupLayout() const {
    // Single source of truth for the setup card: every drawn label/control and
    // every click hit-box comes from this one accumulation pass, so the two
    // can never drift apart. Vertical metrics mirror the design spec (block
    // gap 30, label-to-control gap 11, wordmark gap 8, button-to-hint gap 14)
    // and are uniformly shrunk by `scale` if the window is too short to fit
    // the card's natural ~654px content height.
    const auto winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);

    constexpr float padTop = 44.f, padLR = 44.f, padBottom = 38.f;
    constexpr float blockGap = 30.f;
    constexpr float wordmarkH = 32.f, taglineH = 14.f, gapWordTag = 8.f;
    constexpr float labelH = 14.f, gapLabelControl = 11.f;
    constexpr float nameFieldH = 52.f, diffTrackH = 64.f, sideTilesH = 96.f;
    constexpr float startBtnH = 52.f, gapBtnHint = 14.f, hintH = 14.f;
    constexpr float naturalContent = (wordmarkH + gapWordTag + taglineH) + 1.f +
                                     (labelH + gapLabelControl + nameFieldH) +
                                     (labelH + gapLabelControl + diffTrackH) +
                                     (labelH + gapLabelControl + sideTilesH) +
                                     (startBtnH + gapBtnHint + hintH);
    constexpr float naturalTotal = padTop + naturalContent + 5.f * blockGap + padBottom;

    const float cardW = std::min(560.f, w - 80.f);
    const float scale = std::min(1.f, (h - 80.f) / naturalTotal);
    const float cardH = naturalTotal * scale;
    const float cx = (w - cardW) * 0.5f;
    const float cy = (h - cardH) * 0.5f;

    SetupLayout sl{};
    sl.card = {{cx, cy}, {cardW, cardH}};
    sl.scale = scale;

    const float x = cx + padLR * scale;
    const float innerW = cardW - 2.f * padLR * scale;
    float y = cy + padTop * scale;

    sl.wordmarkPos = {x, y};
    y += (wordmarkH + gapWordTag) * scale;
    sl.taglinePos = {x, y};
    y += taglineH * scale + blockGap * scale;

    sl.dividerY = y;
    y += 1.f * scale + blockGap * scale;

    sl.nameLabelPos = {x, y};
    y += (labelH + gapLabelControl) * scale;
    sl.nameField = {{x, y}, {innerW, nameFieldH * scale}};
    y += nameFieldH * scale + blockGap * scale;

    sl.diffLabelPos = {x, y};
    y += (labelH + gapLabelControl) * scale;
    sl.diffTrack = {{x, y}, {innerW, diffTrackH * scale}};
    const float diffPad = 4.f * scale, diffGap = 4.f * scale;
    const float diffItemW = (innerW - 2.f * diffPad - 3.f * diffGap) / 4.f;
    for (int i = 0; i < 4; ++i) {
        sl.diff[i] = {
            {x + diffPad + static_cast<float>(i) * (diffItemW + diffGap), y + diffPad},
            {diffItemW, diffTrackH * scale - 2.f * diffPad}
        };
    }
    y += diffTrackH * scale + blockGap * scale;

    sl.playLabelPos = {x, y};
    y += (labelH + gapLabelControl) * scale;
    const float sideGap = 12.f * scale;
    const float sideItemW = (innerW - sideGap) * 0.5f;
    sl.side[0] = {{x, y}, {sideItemW, sideTilesH * scale}};
    sl.side[1] = {{x + sideItemW + sideGap, y}, {sideItemW, sideTilesH * scale}};
    y += sideTilesH * scale + blockGap * scale;

    sl.start = {{x, y}, {innerW, startBtnH * scale}};
    y += startBtnH * scale + gapBtnHint * scale;
    sl.hintY = y;

    return sl;
}

void GameViewer::handleSetupClick(sf::Vector2f mousePos) {
    const SetupLayout sl = computeSetupLayout();
    if (pointInRect(mousePos, sl.nameField)) {
        nameFocused_ = true;
        return;
    }
    nameFocused_ = false;
    for (int i = 0; i < 4; ++i) {
        if (pointInRect(mousePos, sl.diff[i])) {
            selectedDifficulty_ = static_cast<Difficulty>(i);
            return;
        }
    }
    if (pointInRect(mousePos, sl.side[0])) {
        selectedPiece_ = PLAYER_BLACK;
        return;
    }
    if (pointInRect(mousePos, sl.side[1])) {
        selectedPiece_ = PLAYER_WHITE;
        return;
    }
    if (pointInRect(mousePos, sl.start)) {
        startGame();
    }
}

GameViewer::GameOverLayout GameViewer::computeGameOverLayout() const {
    GameOverLayout gl{};
    const auto winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);
    const float cardW = 440.f;
    const float cardH = 300.f;
    const float cx = (w - cardW) * 0.5f;
    const float cy = (h - cardH) * 0.5f;
    gl.card = {{cx, cy}, {cardW, cardH}};
    const float padLR = 34.f;
    gl.playAgain = {{cx + padLR, cy + cardH - 34.f - 46.f - 10.f - 42.f}, {cardW - 2.f * padLR, 46.f}};
    gl.quit = {{cx + padLR, cy + cardH - 34.f - 42.f}, {cardW - 2.f * padLR, 42.f}};
    return gl;
}

void GameViewer::handleGameOverClick(sf::Vector2f mousePos) {
    const GameOverLayout gl = computeGameOverLayout();
    if (pointInRect(mousePos, gl.playAgain)) {
        resetGame();
    } else if (pointInRect(mousePos, gl.quit)) {
        window_.close();
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
        setStatus("That square doesn't flip anything.");
        return;
    }

    const auto before = board_;
    pushHistory();
    BoardHelper::playMove(board_, move, currentPlayer_);
    dropAnims_.push_back({move, currentPlayer_, 0.f});
    ringAnims_.push_back({move, 0.f});

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
    dropAnims_.push_back({move, aiPlayer_, 0.f});

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
        // Reached the state right before the human's move — normal case.
        phase_ = Phase::Playing;
        aiMovePending_ = false;
    } else {
        // Ran out of history before finding a human-to-move state: this can
        // only happen when the engine opened and the human undoes their very
        // first move. Replay back to the start and let the engine open again.
        phase_ = Phase::AiThinking;
        aiMovePending_ = true;
        aiDelay_ = 0.4f;
    }
    setStatus("Move undone.");
}

void GameViewer::resetGame() {
    // The setup card is shown only before the first move (per the design
    // spec); "New game" / "Play again" replay instantly with the same
    // difficulty, side and name rather than reopening it.
    startGame();
}

std::optional<Position> GameViewer::cellFromPoint(sf::Vector2f point) const {
    const Layout layout = computeLayout();
    if (point.x < layout.boardX + COORD_BAND || point.y < layout.boardY ||
        point.x >= layout.boardX + COORD_BAND + layout.field ||
        point.y >= layout.boardY + layout.field) {
        return std::nullopt;
    }
    const unsigned int col = static_cast<unsigned int>((point.x - layout.boardX - COORD_BAND) / layout.cell);
    const unsigned int row = static_cast<unsigned int>((point.y - layout.boardY) / layout.cell);
    if (row >= BOARD_SIZE || col >= BOARD_SIZE) {
        return std::nullopt;
    }
    return Position(row, col);
}

sf::Vector2f GameViewer::cellCenter(const Position &pos, const Layout &layout) {
    return {
        layout.boardX + COORD_BAND + (static_cast<float>(pos.getCol()) + 0.5f) * layout.cell,
        layout.boardY + (static_cast<float>(pos.getRow()) + 0.5f) * layout.cell
    };
}

std::string GameViewer::outcomeMessage() const {
    const int blackCount = BoardHelper::countPlayerPieces(board_, PLAYER_BLACK);
    const int whiteCount = BoardHelper::countPlayerPieces(board_, PLAYER_WHITE);
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
    const int blackCount = BoardHelper::countPlayerPieces(board_, PLAYER_BLACK);
    const int whiteCount = BoardHelper::countPlayerPieces(board_, PLAYER_WHITE);
    const int diff = blackCount - whiteCount;
    if (diff == 0) {
        return "DRAW";
    }
    const bool humanWon =
            (diff > 0 && humanPlayer_ == PLAYER_BLACK) || (diff < 0 && humanPlayer_ == PLAYER_WHITE);
    return humanWon ? "YOU WIN" : "YOU LOSE";
}

GameViewer::Layout GameViewer::computeLayout() const {
    const auto winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);

    Layout l;
    l.pad = std::round(clampf(28.f, h * 0.058f, 56.f));
    l.side = std::round(clampf(300.f, w * 0.30f, 380.f));
    l.gap = std::round(clampf(24.f, w * 0.032f, 44.f));
    const float avail = std::min(h - 2.f * l.pad, w - 2.f * l.pad - l.gap - l.side);
    float cell = std::floor((avail - 2.f * COORD_BAND) / 8.f);
    cell = clampf(cell, 44.f, 104.f);
    l.cell = cell;
    l.field = cell * 8.f;
    l.plinth = l.field + 2.f * COORD_BAND;

    const float totalWidth = l.plinth + l.gap + l.side;
    const float left = (w - totalWidth) * 0.5f;
    const float top = (h - l.plinth) * 0.5f;
    l.boardX = left;
    l.boardY = top;
    l.sidebarX = left + l.plinth + l.gap;
    l.sidebarY = top;
    l.fontScale = clampf(cell / 78.f, 0.62f, 1.45f);
    return l;
}

// ------------------------------------------------------------------ setup --
void GameViewer::drawSetupScreen() {
    const auto winSize = window_.getSize();
    drawRoundedRectGradient(window_, {0.f, 0.f},
                            {static_cast<float>(winSize.x), static_cast<float>(winSize.y)}, 0.f,
                            kShellTop, kShellBottom);

    const SetupLayout sl = computeSetupLayout();
    drawSoftShadow(window_, sl.card.position, sl.card.size, 24.f * sl.scale, 10.f, 26.f, 0.22f);
    drawRoundedRect(window_, sl.card.position, sl.card.size, 24.f * sl.scale, sf::Color(249, 246, 241));

    if (!fontsLoaded_) {
        return;
    }

    const float s = sl.scale;
    const float innerW = sl.card.size.x - 2.f * (sl.nameLabelPos.x - sl.card.position.x);

    sf::Text wordmark(fontBold_, "OTHELLO", std::max(8u, static_cast<unsigned int>(25.f * s)));
    wordmark.setFillColor(kInk);
    wordmark.setLetterSpacing(3.6f);
    wordmark.setPosition(sl.wordmarkPos);
    window_.draw(wordmark);

    sf::Text tagline(font_, "TURN THE BOARD", std::max(8u, static_cast<unsigned int>(10.f * s)));
    tagline.setFillColor(kMuted);
    tagline.setLetterSpacing(2.4f);
    tagline.setPosition(sl.taglinePos);
    window_.draw(tagline);

    sf::RectangleShape divider1({innerW, 1.f});
    divider1.setPosition({sl.nameLabelPos.x, sl.dividerY});
    divider1.setFillColor(kHairline);
    window_.draw(divider1);

    sf::Text nameLabel(font_, "YOUR NAME", std::max(8u, static_cast<unsigned int>(10.f * s)));
    nameLabel.setFillColor(kMuted);
    nameLabel.setLetterSpacing(2.2f);
    nameLabel.setPosition(sl.nameLabelPos);
    window_.draw(nameLabel);

    drawRoundedRect(window_, sl.nameField.position, sl.nameField.size, 13.f * s, kSunken,
                    withAlpha(kClay, nameFocused_ ? 0.55f : 0.f), nameFocused_ ? -1.5f : 0.f);
    const std::string displayName = playerName_.empty() ? "Player" : playerName_;
    sf::Text nameText(font_, displayName, std::max(8u, static_cast<unsigned int>(15.f * s)));
    nameText.setFillColor(playerName_.empty() ? kMuted : kInk);
    nameText.setPosition({
        sl.nameField.position.x + 18.f * s,
        sl.nameField.position.y + sl.nameField.size.y * 0.5f - 8.f * s
    });
    window_.draw(nameText);
    if (nameFocused_ && std::fmod(caretTime_, 0.53f) < 0.265f) {
        const float caretX = sl.nameField.position.x + 18.f * s +
                             (playerName_.empty() ? 0.f : nameText.getLocalBounds().size.x + 2.f);
        sf::RectangleShape caret({std::max(1.f, 2.f * s), 20.f * s});
        caret.setPosition({caretX, sl.nameField.position.y + sl.nameField.size.y * 0.5f - 10.f * s});
        caret.setFillColor(kClay);
        window_.draw(caret);
    }

    sf::Text diffLabel(font_, "DIFFICULTY", std::max(8u, static_cast<unsigned int>(10.f * s)));
    diffLabel.setFillColor(kMuted);
    diffLabel.setLetterSpacing(2.2f);
    diffLabel.setPosition(sl.diffLabelPos);
    window_.draw(diffLabel);

    drawRoundedRect(window_, sl.diffTrack.position, sl.diffTrack.size, 15.f * s, kSunken);
    for (int i = 0; i < 4; ++i) {
        const bool selected = static_cast<int>(selectedDifficulty_) == i;
        if (selected) {
            drawRoundedRect(window_, sl.diff[i].position, sl.diff[i].size, 11.f * s, kSurfaceRaised,
                            withAlpha(kClay, 0.4f), 1.5f);
        }
        sf::Text label(selected ? fontBold_ : font_, difficultyLabel(static_cast<Difficulty>(i)),
                       std::max(8u, static_cast<unsigned int>(13.f * s)));
        label.setFillColor(selected ? kInk : kEyebrow);
        const auto bounds = label.getLocalBounds();
        label.setPosition({
            sl.diff[i].position.x + (sl.diff[i].size.x - bounds.size.x) * 0.5f,
            sl.diff[i].position.y + (sl.diff[i].size.y - bounds.size.y) * 0.5f - bounds.position.y
        });
        window_.draw(label);
    }

    sf::Text playLabel(font_, "YOU PLAY", std::max(8u, static_cast<unsigned int>(10.f * s)));
    playLabel.setFillColor(kMuted);
    playLabel.setLetterSpacing(2.2f);
    playLabel.setPosition(sl.playLabelPos);
    window_.draw(playLabel);

    const struct {
        char value;
        const char *label;
        const char *sub;
        const sf::Texture &tex;
    } pieceOptions[] = {
        {PLAYER_BLACK, "Dark", "you open", discDarkTex_},
        {PLAYER_WHITE, "Light", "engine opens", discLightTex_},
    };
    for (int i = 0; i < 2; ++i) {
        const bool selected = selectedPiece_ == pieceOptions[i].value;
        drawRoundedRect(window_, sl.side[i].position, sl.side[i].size, 15.f * s,
                        selected ? kSurfaceRaised : kSunken, withAlpha(kClay, selected ? 0.4f : 0.f),
                        selected ? 1.5f : 0.f);
        const float cx = sl.side[i].position.x + sl.side[i].size.x * 0.5f;
        const float discY = sl.side[i].position.y + 26.f * s;
        sf::Sprite discSprite(pieceOptions[i].tex);
        discSprite.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        const float discScale = 34.f * s / static_cast<float>(kDiscTexSize);
        discSprite.setScale({discScale, discScale});
        discSprite.setPosition({cx, discY});
        window_.draw(discSprite);

        sf::Text label(fontBold_, pieceOptions[i].label, std::max(8u, static_cast<unsigned int>(13.f * s)));
        label.setFillColor(selected ? kInk : kBody);
        auto lb = label.getLocalBounds();
        label.setPosition({cx - lb.size.x * 0.5f, discY + 24.f * s});
        window_.draw(label);

        sf::Text sub(font_, pieceOptions[i].sub, std::max(7u, static_cast<unsigned int>(10.f * s)));
        sub.setFillColor(kMuted);
        auto sb = sub.getLocalBounds();
        sub.setPosition({cx - sb.size.x * 0.5f, discY + 45.f * s});
        window_.draw(sub);
    }

    drawRoundedRect(window_, sl.start.position, sl.start.size, 14.f * s, kClay);
    sf::Text startLabel(fontBold_, "Start game", std::max(8u, static_cast<unsigned int>(14.f * s)));
    startLabel.setFillColor(sf::Color(255, 247, 241));
    startLabel.setLetterSpacing(1.2f);
    auto slb = startLabel.getLocalBounds();
    const float enterChipW = 40.f * s;
    const float groupW = slb.size.x + 12.f * s + enterChipW;
    float gx = sl.start.position.x + (sl.start.size.x - groupW) * 0.5f;
    startLabel.setPosition({gx, sl.start.position.y + (sl.start.size.y - slb.size.y) * 0.5f - slb.position.y});
    window_.draw(startLabel);
    gx += slb.size.x + 12.f * s;
    drawRoundedRect(window_, {gx, sl.start.position.y + sl.start.size.y * 0.5f - 9.f * s}, {enterChipW, 18.f * s},
                    5.f * s, sf::Color(255, 255, 255, 51));
    sf::Text enterLabel(font_, "Enter", std::max(7u, static_cast<unsigned int>(10.f * s)));
    enterLabel.setFillColor(sf::Color(255, 247, 241));
    auto elb = enterLabel.getLocalBounds();
    enterLabel.setPosition({
        gx + (enterChipW - elb.size.x) * 0.5f,
        sl.start.position.y + sl.start.size.y * 0.5f - 9.f * s + 3.f * s
    });
    window_.draw(enterLabel);

    sf::Text hint(font_, "Tab / arrows to move  -  1-4 sets difficulty  -  Esc quits",
                  std::max(7u, static_cast<unsigned int>(11.f * s)));
    hint.setFillColor(kHelperMuted);
    auto hlb = hint.getLocalBounds();
    hint.setPosition({sl.card.position.x + (sl.card.size.x - hlb.size.x) * 0.5f, sl.hintY});
    window_.draw(hint);
}

// -------------------------------------------------------------- main window --
void GameViewer::drawMainWindow(const Layout &layout) {
    drawRoundedRectGradient(window_, {0.f, 0.f},
                            {static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)},
                            0.f, kShellTop, kShellBottom);
    drawBoardPlinth(layout);
    drawValidHints(layout);
    drawDiscs(layout);
    drawSidebar(layout);
}

void GameViewer::drawBoardPlinth(const Layout &layout) {
    const sf::Vector2f plinthPos(layout.boardX, layout.boardY);
    const sf::Vector2f plinthSize(layout.plinth, layout.plinth);
    const float plinthRadius = layout.cell * 0.28f;
    drawSoftShadow(window_, plinthPos, plinthSize, plinthRadius, 8.f, 18.f, 0.16f);
    drawRoundedRectGradient(window_, plinthPos, plinthSize, plinthRadius, kPlinthTop, kPlinthBottom);

    if (fontsLoaded_) {
        const unsigned int labelSize = std::max(8u, static_cast<unsigned int>(std::round(11.f * layout.fontScale)));
        for (int i = 0; i < BOARD_SIZE; ++i) {
            sf::Text rank(font_, std::to_string(BOARD_SIZE - i), labelSize);
            rank.setFillColor(kMuted);
            auto rb = rank.getLocalBounds();
            rank.setPosition({
                layout.boardX + (COORD_BAND - rb.size.x) * 0.5f - rb.position.x,
                layout.boardY + static_cast<float>(i) * layout.cell + (layout.cell - rb.size.y) * 0.5f
            });
            window_.draw(rank);

            sf::Text file(font_, std::string(1, static_cast<char>('a' + i)), labelSize);
            file.setFillColor(kMuted);
            auto fb = file.getLocalBounds();
            file.setPosition({
                layout.boardX + COORD_BAND + static_cast<float>(i) * layout.cell +
                (layout.cell - fb.size.x) * 0.5f - fb.position.x,
                layout.boardY + layout.field + (COORD_BAND - fb.size.y) * 0.5f
            });
            window_.draw(file);
        }
    }

    const sf::Vector2f fieldPos(layout.boardX + COORD_BAND, layout.boardY);
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            sf::RectangleShape square({layout.cell, layout.cell});
            square.setPosition({
                fieldPos.x + static_cast<float>(col) * layout.cell,
                fieldPos.y + static_cast<float>(row) * layout.cell
            });
            square.setFillColor(((row + col) % 2 == 0) ? kSquareLight : kSquareDark);
            window_.draw(square);
        }
    }
}

void GameViewer::drawValidHints(const Layout &layout) {
    if (phase_ != Phase::Playing || currentPlayer_ != humanPlayer_) {
        return;
    }
    const auto moves = BoardHelper::getAllPossibleMoves(board_, humanPlayer_);
    const float wave = 0.5f + 0.5f * std::sin(pulseTime_ * (2.f * 3.14159265f / 1.6f));
    const float fillAlpha = (108.f + 92.f * wave) / 255.f;
    const float radius = layout.cell * 0.128f * (1.f + 0.10f * wave);
    for (const auto &move: moves) {
        sf::CircleShape hint(radius);
        hint.setOrigin({radius, radius});
        hint.setPosition(cellCenter(move, layout));
        hint.setFillColor(withAlpha(kClay, fillAlpha));
        hint.setOutlineColor(withAlpha(kClay, 0.62f));
        hint.setOutlineThickness(std::max(1.f, 2.f * layout.cell / 78.f));
        window_.draw(hint);
    }
}

void GameViewer::drawDiscs(const Layout &layout) {
    auto drawDiscSprite = [&](const sf::Texture &tex, sf::Vector2f center, float widthPx, float heightPx) {
        sf::Sprite sprite(tex);
        sprite.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        sprite.setScale({widthPx / static_cast<float>(kDiscTexSize), heightPx / static_cast<float>(kDiscTexSize)});
        sprite.setPosition(center);
        window_.draw(sprite);
    };
    auto drawShadowSprite = [&](sf::Vector2f center, float widthPx, float heightPx, float alpha) {
        sf::Sprite sprite(discShadowTex_);
        sprite.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        sprite.setScale({widthPx / static_cast<float>(kDiscTexSize), heightPx / static_cast<float>(kDiscTexSize)});
        sprite.setPosition(center);
        sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(clampf(alpha, 0.f, 1.f) * 255.f)));
        window_.draw(sprite);
    };

    const float discD = layout.cell * 0.72f;
    const float shadowD = layout.cell * 0.86f;
    const float shadowOffsetY = layout.cell * 0.055f;

    // Cells mid-flip are rendered separately from the static board contents.
    std::vector<bool> skip(static_cast<std::size_t>(BOARD_SIZE * BOARD_SIZE), false);
    for (const auto &anim: flipAnims_) {
        skip[anim.pos.getRow() * BOARD_SIZE + anim.pos.getCol()] = true;
        const sf::Vector2f center = cellCenter(anim.pos, layout);
        const bool started = anim.age >= anim.delay;
        if (!started) {
            const float baseAlpha = anim.from == PLAYER_BLACK ? 0.70f : 0.52f;
            drawShadowSprite({center.x, center.y + shadowOffsetY}, shadowD, shadowD, baseAlpha);
            drawDiscSprite(anim.from == PLAYER_BLACK ? discDarkTex_ : discLightTex_, center, discD, discD);
            continue;
        }
        const float t = clampf((anim.age - anim.delay) / 0.3f, 0.f, 1.f);
        const float eased = easeInOutSine(t);
        const float scaleX = std::fabs(std::cos(3.14159265f * eased));
        const char shown = (t < 0.5f) ? anim.from : anim.to;
        const float shadowMul = 1.f - 0.6f * (1.f - scaleX);
        const float baseAlpha = (shown == PLAYER_BLACK ? 0.70f : 0.52f) * shadowMul;
        drawShadowSprite({center.x, center.y + shadowOffsetY}, shadowD * std::max(scaleX, 0.08f), shadowD, baseAlpha);
        drawDiscSprite(shown == PLAYER_BLACK ? discDarkTex_ : discLightTex_, center,
                       discD * std::max(scaleX, 0.06f), discD);
    }

    for (unsigned int row = 0; row < BOARD_SIZE; ++row) {
        for (unsigned int col = 0; col < BOARD_SIZE; ++col) {
            if (skip[row * BOARD_SIZE + col]) {
                continue;
            }
            const char piece = board_[row][col];
            if (piece == EMPTY) {
                continue;
            }
            const Position pos(row, col);
            const sf::Vector2f center = cellCenter(pos, layout);

            float scale = 1.f;
            float alphaMul = 1.f;
            for (const auto &anim: dropAnims_) {
                if (anim.pos == pos) {
                    const float t = clampf(anim.age / 0.24f, 0.f, 1.f);
                    scale = 0.55f + 0.45f * easeOutBack(t);
                    alphaMul = clampf(anim.age / 0.09f, 0.f, 1.f);
                    break;
                }
            }

            const float baseAlpha = (piece == PLAYER_BLACK ? 0.70f : 0.52f) * alphaMul;
            drawShadowSprite({center.x, center.y + shadowOffsetY}, shadowD * scale, shadowD * scale, baseAlpha);
            sf::Sprite sprite(piece == PLAYER_BLACK ? discDarkTex_ : discLightTex_);
            sprite.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
            const float s = (discD * scale) / static_cast<float>(kDiscTexSize);
            sprite.setScale({s, s});
            sprite.setPosition(center);
            sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(clampf(alphaMul, 0.f, 1.f) * 255.f)));
            window_.draw(sprite);
        }
    }

    for (const auto &ring: ringAnims_) {
        const float t = clampf(ring.age / 0.6f, 0.f, 1.f);
        const float eased = easeOutQuad(t);
        const float radius = layout.cell * (0.36f + 0.075f * eased);
        const float alpha = (96.f * (1.f - eased)) / 255.f;
        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(cellCenter(ring.pos, layout));
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineColor(withAlpha(kClay, alpha));
        circle.setOutlineThickness(std::max(1.f, 2.f * layout.cell / 78.f));
        window_.draw(circle);
    }
}

// ----------------------------------------------------------------- sidebar --
void GameViewer::drawSidebar(const Layout &layout) {
    const float fs = layout.fontScale;
    auto sz = [&](float base) { return std::max(8u, static_cast<unsigned int>(std::round(base * fs))); };

    const float padTop = 28.f;
    const float padLR = 26.f;
    const float sbW = layout.side;
    float x = layout.sidebarX + padLR;
    float y = layout.sidebarY + padTop;
    const float innerW = sbW - 2.f * padLR;

    if (!fontsLoaded_) {
        drawRoundedRect(window_, {layout.sidebarX, layout.sidebarY}, {sbW, layout.plinth}, layout.cell * 0.28f,
                        kSurface);
        return;
    }

    drawSoftShadow(window_, {layout.sidebarX, layout.sidebarY}, {sbW, layout.plinth}, layout.cell * 0.28f, 8.f,
                   16.f, 0.14f);
    drawRoundedRect(window_, {layout.sidebarX, layout.sidebarY}, {sbW, layout.plinth}, layout.cell * 0.28f,
                    kSurface);

    sf::Text wordmark(fontBold_, "OTHELLO", sz(25));
    wordmark.setFillColor(kInk);
    wordmark.setLetterSpacing(3.4f);
    wordmark.setPosition({x, y});
    window_.draw(wordmark);
    y += sz(25) * 1.18f;

    sf::Text tagline(font_, "TURN THE BOARD", sz(10));
    tagline.setFillColor(kMuted);
    tagline.setLetterSpacing(2.4f);
    tagline.setPosition({x, y});
    window_.draw(tagline);
    y += sz(10) * 1.9f;

    sf::RectangleShape divider({innerW, 1.f});
    divider.setPosition({x, y});
    divider.setFillColor(kHairline);
    window_.draw(divider);
    y += 26.f;

    // -------------------------------------------------------------- scores --
    const int blackCount = BoardHelper::countPlayerPieces(board_, PLAYER_BLACK);
    const int whiteCount = BoardHelper::countPlayerPieces(board_, PLAYER_WHITE);
    const std::string humanName = playerName_.empty() ? "Player" : playerName_;

    auto drawScoreRow = [&](float rowY, char player, int count, const std::string &name, bool isActive) {
        const bool selected = (currentPlayer_ == player) && phase_ != Phase::GameOver;
        const float rowH = 58.f;
        if (selected) {
            drawRoundedRect(window_, {x, rowY}, {innerW, rowH}, 14.f, kSurfaceRaised,
                            withAlpha(kClay, 0.30f * (isActive ? turnAnim_ : 1.f)), 1.f);
        } else {
            drawRoundedRect(window_, {x, rowY}, {innerW, rowH}, 14.f, kSunken);
        }
        const sf::Texture &tex = player == PLAYER_BLACK ? discDarkTex_ : discLightTex_;
        sf::Sprite disc(tex);
        disc.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        const float discSize = 26.f;
        disc.setScale({discSize / kDiscTexSize, discSize / kDiscTexSize});
        disc.setPosition({x + 16.f + discSize * 0.5f, rowY + rowH * 0.5f});
        window_.draw(disc);

        sf::Text nameText(fontBold_, name, sz(13));
        nameText.setFillColor(selected ? kInk : kBody);
        nameText.setPosition({x + 16.f + discSize + 12.f, rowY + 11.f});
        window_.draw(nameText);

        sf::Text roleText(font_, player == PLAYER_BLACK ? "DARK" : "LIGHT", sz(10));
        roleText.setFillColor(kMuted);
        roleText.setLetterSpacing(1.8f);
        roleText.setPosition({x + 16.f + discSize + 12.f, rowY + 30.f});
        window_.draw(roleText);

        sf::Text score(fontBold_, std::to_string(count), sz(32));
        score.setFillColor(selected ? kInk : kBody);
        auto scb = score.getLocalBounds();
        score.setPosition({x + innerW - 16.f - scb.size.x - scb.position.x, rowY + rowH * 0.5f - sz(32) * 0.62f});
        window_.draw(score);
    };

    const bool darkIsCurrent = currentPlayer_ == PLAYER_BLACK;
    drawScoreRow(y, PLAYER_BLACK, blackCount, humanPlayer_ == PLAYER_BLACK ? humanName : "Engine", darkIsCurrent);
    y += 58.f + 12.f;
    drawScoreRow(y, PLAYER_WHITE, whiteCount, humanPlayer_ == PLAYER_WHITE ? humanName : "Engine", !darkIsCurrent);
    y += 58.f + 12.f;

    const int total = std::max(1, blackCount + whiteCount);
    sf::RectangleShape barBg({innerW, 6.f});
    barBg.setPosition({x, y});
    barBg.setFillColor(kHairline);
    window_.draw(barBg);
    sf::RectangleShape barFill({innerW * static_cast<float>(blackCount) / static_cast<float>(total), 6.f});
    barFill.setPosition({x, y});
    barFill.setFillColor(kDiscDarkMid);
    window_.draw(barFill);
    y += 6.f + 26.f;

    // --------------------------------------------------------- turn state --
    if (phase_ == Phase::AiThinking) {
        const float blockH = 68.f;
        drawRoundedRect(window_, {x, y}, {innerW, blockH}, 14.f, kSunken);
        sf::Sprite disc(discLightTex_);
        disc.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        disc.setScale({18.f / kDiscTexSize, 18.f / kDiscTexSize});
        disc.setPosition({x + 16.f + 9.f, y + 27.f});
        if (aiPlayer_ == PLAYER_BLACK) {
            disc.setTexture(discDarkTex_);
        }
        window_.draw(disc);

        sf::Text title(fontBold_, "Engine thinking", sz(15));
        title.setFillColor(kInk);
        title.setPosition({x + 16.f + 18.f + 11.f, y + 15.f});
        window_.draw(title);

        sf::Text depth(font_, "depth " + std::to_string(aiDepth_), sz(11));
        depth.setFillColor(kClayText);
        auto db = depth.getLocalBounds();
        depth.setPosition({x + innerW - 16.f - db.size.x - db.position.x, y + 18.f});
        window_.draw(depth);

        const float trackY = y + blockH - 16.f - 4.f;
        sf::RectangleShape track({innerW - 32.f, 4.f});
        track.setPosition({x + 16.f, trackY});
        track.setFillColor(kHairline);
        window_.draw(track);
        const float sweepW = (innerW - 32.f) * 0.38f;
        const float phaseT = std::fmod(pulseTime_, 1.2f) / 1.2f;
        const float sweepX = -sweepW + phaseT * (innerW - 32.f + sweepW);
        sf::RectangleShape fill({sweepW, 4.f});
        fill.setPosition({x + 16.f + clampf(sweepX, 0.f, innerW - 32.f - sweepW), trackY});
        fill.setFillColor(kClay);
        window_.draw(fill);
        y += blockH + 26.f;
    } else if (phase_ == Phase::Playing && currentPlayer_ == humanPlayer_) {
        const float blockH = 56.f;
        drawRoundedRect(window_, {x, y}, {innerW, blockH}, 14.f, withAlpha(kClay, 0.10f),
                        withAlpha(kClay, 0.24f), -1.f);
        sf::Sprite disc(humanPlayer_ == PLAYER_BLACK ? discDarkTex_ : discLightTex_);
        disc.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
        disc.setScale({18.f / kDiscTexSize, 18.f / kDiscTexSize});
        disc.setPosition({x + 16.f + 9.f, y + blockH * 0.5f});
        window_.draw(disc);

        sf::Text title(fontBold_, "Your move", sz(15));
        title.setFillColor(kInk);
        title.setPosition({x + 16.f + 18.f + 11.f, y + (blockH - sz(15)) * 0.5f - 2.f});
        window_.draw(title);

        const auto legal = BoardHelper::getAllPossibleMoves(board_, humanPlayer_);
        sf::Text legalText(font_, std::to_string(legal.size()) + " legal", sz(11));
        legalText.setFillColor(kClayText);
        auto lb = legalText.getLocalBounds();
        legalText.setPosition({x + innerW - 16.f - lb.size.x - lb.position.x, y + (blockH - sz(11)) * 0.5f});
        window_.draw(legalText);
        y += blockH + 26.f;
    }

    // -------------------------------------------------------------- status --
    sf::Text statusLabel(font_, "STATUS", sz(10));
    statusLabel.setFillColor(kMuted);
    statusLabel.setLetterSpacing(2.2f);
    statusLabel.setPosition({x, y});
    window_.draw(statusLabel);
    y += sz(10) * 1.9f;

    const auto lines = wrapText(font_, statusMessage_, sz(13), innerW);
    for (const auto &line: lines) {
        sf::Text lineText(font_, line, sz(13));
        lineText.setFillColor(kBody);
        lineText.setPosition({x, y});
        window_.draw(lineText);
        y += sz(13) * 1.55f;
    }

    // ------------------------------------------------------------- footer --
    // Bottom-anchored like a flex spacer, but never overlaps the content
    // above it — if the status text runs long on a short window, the footer
    // is pushed down instead of colliding with it.
    const float anchoredFooterY = layout.sidebarY + layout.plinth - padTop - 1.f - 10.f - 3.f * 26.f;
    const float footerY = std::max(y + 14.f, anchoredFooterY);
    sf::RectangleShape divider2({innerW, 1.f});
    divider2.setPosition({x, footerY});
    divider2.setFillColor(kHairline);
    window_.draw(divider2);

    const struct {
        const char *key;
        const char *label;
    } shortcuts[] = {
        {"U", "Undo last move"},
        {"R", "New game"},
        {"Esc", "Quit"},
    };
    float fy = footerY + 16.f;
    for (const auto &sc: shortcuts) {
        sf::Text key(font_, sc.key, sz(11));
        key.setFillColor(kBody);
        auto kb = key.getLocalBounds();
        const float chipW = std::max(24.f, kb.size.x + 18.f);
        drawRoundedRect(window_, {x, fy}, {chipW, 22.f}, 6.f, kKeycapBg);
        sf::RectangleShape edge({chipW, 2.f});
        edge.setPosition({x, fy + 20.f});
        edge.setFillColor(kKeycapEdge);
        window_.draw(edge);
        key.setPosition({x + (chipW - kb.size.x) * 0.5f - kb.position.x, fy + 4.f});
        window_.draw(key);

        sf::Text label(font_, sc.label, sz(12));
        label.setFillColor(kEyebrow);
        label.setPosition({x + chipW + 10.f, fy + 4.f});
        window_.draw(label);
        fy += 26.f;
    }
}

// -------------------------------------------------------------- game over --
void GameViewer::drawGameOverOverlay() {
    const auto winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);
    const float scrimAlpha = clampf(gameOverAnim_, 0.f, 1.f) * (210.f / 255.f);
    sf::RectangleShape veil({w, h});
    veil.setFillColor(withAlpha(sf::Color(231, 225, 216), scrimAlpha));
    window_.draw(veil);

    if (!fontsLoaded_) {
        return;
    }

    const GameOverLayout gl = computeGameOverLayout();
    const float lift = (1.f - easeOutBack(clampf(gameOverAnim_, 0.f, 1.f))) * 14.f;
    const sf::Vector2f cardPos = gl.card.position + sf::Vector2f(0.f, lift);
    const float cardAlpha = clampf(gameOverAnim_ / 0.6f, 0.f, 1.f);

    drawSoftShadow(window_, cardPos, gl.card.size, 24.f, 12.f, 28.f, 0.24f * cardAlpha);
    drawRoundedRect(window_, cardPos, gl.card.size, 24.f, withAlpha(sf::Color(249, 246, 241), cardAlpha));

    float y = cardPos.y + 36.f;
    const float cx = cardPos.x + gl.card.size.x * 0.5f;

    sf::Text finalLabel(font_, "FINAL", 10);
    finalLabel.setFillColor(withAlpha(kMuted, cardAlpha));
    finalLabel.setLetterSpacing(2.6f);
    auto fb = finalLabel.getLocalBounds();
    finalLabel.setPosition({cx - fb.size.x * 0.5f, y});
    window_.draw(finalLabel);
    y += 30.f;

    sf::Text headline(fontBold_, resultHeadline(), 30);
    headline.setFillColor(withAlpha(kInk, cardAlpha));
    headline.setLetterSpacing(1.6f);
    auto hb = headline.getLocalBounds();
    headline.setPosition({cx - hb.size.x * 0.5f, y});
    window_.draw(headline);
    y += 56.f;

    const int blackCount = BoardHelper::countPlayerPieces(board_, PLAYER_BLACK);
    const int whiteCount = BoardHelper::countPlayerPieces(board_, PLAYER_WHITE);
    const sf::Color darkColor = blackCount >= whiteCount ? kInk : kMuted;
    const sf::Color lightColor = whiteCount >= blackCount ? kInk : kMuted;

    sf::Text darkScore(fontBold_, std::to_string(blackCount), 44);
    darkScore.setFillColor(withAlpha(darkColor, cardAlpha));
    sf::Text lightScore(fontBold_, std::to_string(whiteCount), 44);
    lightScore.setFillColor(withAlpha(lightColor, cardAlpha));
    sf::Text dot(font_, ".", 20);
    dot.setFillColor(withAlpha(kMuted, cardAlpha));

    const float discSize = 26.f;
    const float groupGap = 26.f;
    const auto db = darkScore.getLocalBounds();
    const auto lb = lightScore.getLocalBounds();
    const float leftW = discSize + 12.f + db.size.x;
    const float rightW = lb.size.x + 12.f + discSize;
    const float totalW = leftW + groupGap + 20.f + groupGap + rightW;
    float gx = cx - totalW * 0.5f;

    sf::Sprite darkDisc(discDarkTex_);
    darkDisc.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
    darkDisc.setScale({discSize / kDiscTexSize, discSize / kDiscTexSize});
    darkDisc.setPosition({gx + discSize * 0.5f, y + 22.f});
    darkDisc.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(cardAlpha * 255.f)));
    window_.draw(darkDisc);
    darkScore.setPosition({gx + discSize + 12.f, y});
    window_.draw(darkScore);
    gx += leftW + groupGap;

    dot.setPosition({gx, y + 8.f});
    window_.draw(dot);
    gx += 20.f + groupGap;

    lightScore.setPosition({gx, y});
    window_.draw(lightScore);
    sf::Sprite lightDisc(discLightTex_);
    lightDisc.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
    lightDisc.setScale({discSize / kDiscTexSize, discSize / kDiscTexSize});
    lightDisc.setPosition({gx + lb.size.x + 12.f + discSize * 0.5f, y + 22.f});
    lightDisc.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(cardAlpha * 255.f)));
    window_.draw(lightDisc);

    drawRoundedRect(window_, gl.playAgain.position + sf::Vector2f(0.f, lift), gl.playAgain.size, 13.f,
                    withAlpha(kClay, cardAlpha));
    sf::Text playLabel(fontBold_, "Play again", 14);
    playLabel.setFillColor(withAlpha(sf::Color(255, 247, 241), cardAlpha));
    auto plb = playLabel.getLocalBounds();
    const float chipW = 34.f;
    const float groupW2 = plb.size.x + 12.f + chipW;
    float px = gl.playAgain.position.x + (gl.playAgain.size.x - groupW2) * 0.5f;
    const float py = gl.playAgain.position.y + lift + (gl.playAgain.size.y - plb.size.y) * 0.5f - plb.position.y;
    playLabel.setPosition({px, py});
    window_.draw(playLabel);
    px += plb.size.x + 12.f;
    drawRoundedRect(window_, {px, gl.playAgain.position.y + lift + gl.playAgain.size.y * 0.5f - 9.f},
                    {chipW, 18.f}, 5.f, sf::Color(255, 255, 255, static_cast<std::uint8_t>(51 * cardAlpha)));
    sf::Text rKey(font_, "R", 10);
    rKey.setFillColor(withAlpha(sf::Color(255, 247, 241), cardAlpha));
    auto rkb = rKey.getLocalBounds();
    rKey.setPosition({
        px + (chipW - rkb.size.x) * 0.5f, gl.playAgain.position.y + lift + gl.playAgain.size.y * 0.5f - 9.f + 3.f
    });
    window_.draw(rKey);

    drawRoundedRect(window_, gl.quit.position + sf::Vector2f(0.f, lift), gl.quit.size, 13.f,
                    sf::Color::Transparent, withAlpha(sf::Color(223, 216, 203), cardAlpha), 1.f);
    sf::Text quitLabel(font_, "Quit", 13);
    quitLabel.setFillColor(withAlpha(kEyebrow, cardAlpha));
    auto qlb = quitLabel.getLocalBounds();
    const float chipW2 = 34.f;
    const float groupW3 = qlb.size.x + 12.f + chipW2;
    float qx = gl.quit.position.x + (gl.quit.size.x - groupW3) * 0.5f;
    const float qy = gl.quit.position.y + lift + (gl.quit.size.y - qlb.size.y) * 0.5f - qlb.position.y;
    quitLabel.setPosition({qx, qy});
    window_.draw(quitLabel);
    qx += qlb.size.x + 12.f;
    drawRoundedRect(window_, {qx, gl.quit.position.y + lift + gl.quit.size.y * 0.5f - 9.f}, {chipW2, 18.f}, 5.f,
                    withAlpha(kKeycapBg, cardAlpha));
    sf::Text escKey(font_, "Esc", 10);
    escKey.setFillColor(withAlpha(kEyebrow, cardAlpha));
    auto ekb = escKey.getLocalBounds();
    escKey.setPosition({
        qx + (chipW2 - ekb.size.x) * 0.5f, gl.quit.position.y + lift + gl.quit.size.y * 0.5f - 9.f + 3.f
    });
    window_.draw(escKey);
}

void GameViewer::render() {
    window_.clear(kShellBottom);
    if (phase_ == Phase::Setup) {
        drawSetupScreen();
    } else {
        const Layout layout = computeLayout();
        drawMainWindow(layout);
        if (phase_ == Phase::GameOver) {
            drawGameOverOverlay();
        }
    }
    window_.display();
}
