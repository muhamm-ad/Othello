#include "viewer/GameViewer.hpp"
#include "viewer/Style.hpp"
#include "viewer/Ui.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace viewer;

void GameViewer::handleBoardClick(sf::Vector2f mousePos) {
    const auto cell = cellFromPoint(mousePos);
    if (!cell) {
        return;
    }
    tryHumanMove(*cell);
}

std::optional<Position> GameViewer::cellFromPoint(sf::Vector2f point) const {
    const Layout layout = computeLayout();
    if (point.x < layout.fieldX || point.y < layout.fieldY ||
        point.x >= layout.fieldX + layout.field ||
        point.y >= layout.fieldY + layout.field) {
        return std::nullopt;
    }
    const unsigned int col = static_cast<unsigned int>((point.x - layout.fieldX) / layout.cell);
    const unsigned int row = static_cast<unsigned int>((point.y - layout.fieldY) / layout.cell);
    if (row >= BOARD_SIZE || col >= BOARD_SIZE) {
        return std::nullopt;
    }
    return Position(row, col);
}

sf::Vector2f GameViewer::cellCenter(const Position &pos, const Layout &layout) {
    return {
        layout.fieldX + (static_cast<float>(pos.getCol()) + 0.5f) * layout.cell,
        layout.fieldY + (static_cast<float>(pos.getRow()) + 0.5f) * layout.cell
    };
}

GameViewer::Layout GameViewer::computeLayout() const {
    const auto winSize = window_.getSize();
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);
    const float shortest = std::min(w, h);

    Layout l;
    l.pad = std::round(clampf(shortest * 0.018f, 10.f, 20.f));
    l.gap = std::round(clampf(shortest * 0.014f, 8.f, 16.f));
    l.side = std::round(clampf(w * 0.22f, 220.f, 300.f));

    const float innerH = std::max(0.f, h - 2.f * l.pad);
    const float innerW = std::max(0.f, w - 2.f * l.pad);
    float plinth = std::min(innerH, std::max(0.f, innerW - l.gap - l.side));

    float cell = (plinth - 2.f * COORD_BAND) / 8.f;
    cell = std::max(cell, 44.f);
    l.cell = cell;
    l.field = cell * 8.f;
    l.plinth = l.field + 2.f * COORD_BAND;

    const float totalW = l.plinth + l.gap + l.side;
    const float left = (w - totalW) * 0.5f;
    const float top = (h - l.plinth) * 0.5f;
    l.boardX = left;
    l.boardY = top;
    l.fieldX = l.boardX + COORD_BAND;
    l.fieldY = l.boardY + COORD_BAND;
    l.sidebarX = l.boardX + l.plinth + l.gap;
    l.sidebarY = top;
    l.sidebarH = l.plinth;
    l.fontScale = clampf(l.cell / 78.f, 0.62f, 2.0f);
    return l;
}

void GameViewer::drawMainWindow(const Layout &layout) {
    drawShellBackground(window_, window_.getSize());
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
                layout.fieldY + static_cast<float>(i) * layout.cell + (layout.cell - rb.size.y) * 0.5f
            });
            window_.draw(rank);

            sf::Text file(font_, std::string(1, static_cast<char>('a' + i)), labelSize);
            file.setFillColor(kMuted);
            auto fb = file.getLocalBounds();
            file.setPosition({
                layout.fieldX + static_cast<float>(i) * layout.cell +
                (layout.cell - fb.size.x) * 0.5f - fb.position.x,
                layout.fieldY + layout.field + (COORD_BAND - fb.size.y) * 0.5f
            });
            window_.draw(file);
        }
    }

    const sf::Vector2f fieldPos(layout.fieldX, layout.fieldY);
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
    const float wave = 0.5f + 0.5f * std::sin(pulseTime_ * (2.f * kPi / 1.6f));
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
    const float discD = layout.cell * 0.72f;
    const float shadowD = layout.cell * 0.86f;
    const float shadowOffsetY = layout.cell * 0.055f;

    auto drawShadow = [&](sf::Vector2f center, float widthPx, float heightPx, float alpha) {
        drawDiscSprite(window_, discShadowTex_, center, widthPx, heightPx, alpha);
    };

    // Cells mid-flip are rendered separately from the static board contents.
    std::vector<bool> skip(static_cast<std::size_t>(BOARD_SIZE * BOARD_SIZE), false);
    for (const auto &anim: flipAnims_) {
        skip[anim.pos.getRow() * BOARD_SIZE + anim.pos.getCol()] = true;
        const sf::Vector2f center = cellCenter(anim.pos, layout);
        const bool started = anim.age >= anim.delay;
        if (!started) {
            const float baseAlpha = anim.from == PLAYER_BLACK ? 0.70f : 0.52f;
            drawShadow({center.x, center.y + shadowOffsetY}, shadowD, shadowD, baseAlpha);
            drawDiscSprite(window_, anim.from == PLAYER_BLACK ? discDarkTex_ : discLightTex_,
                           center, discD, discD);
            continue;
        }
        const float t = clampf((anim.age - anim.delay) / 0.3f, 0.f, 1.f);
        const float eased = easeInOutSine(t);
        const float scaleX = std::fabs(std::cos(kPi * eased));
        const char shown = (t < 0.5f) ? anim.from : anim.to;
        const float shadowMul = 1.f - 0.6f * (1.f - scaleX);
        const float baseAlpha = (shown == PLAYER_BLACK ? 0.70f : 0.52f) * shadowMul;
        drawShadow({center.x, center.y + shadowOffsetY}, shadowD * std::max(scaleX, 0.08f), shadowD, baseAlpha);
        drawDiscSprite(window_, shown == PLAYER_BLACK ? discDarkTex_ : discLightTex_, center,
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
            drawShadow({center.x, center.y + shadowOffsetY}, shadowD * scale, shadowD * scale, baseAlpha);
            drawDiscSprite(window_, piece == PLAYER_BLACK ? discDarkTex_ : discLightTex_,
                           center, discD * scale, discD * scale, alphaMul);
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
