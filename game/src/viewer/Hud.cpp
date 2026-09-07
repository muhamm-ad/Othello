#include "viewer/GameViewer.hpp"
#include "viewer/Style.hpp"
#include "viewer/Ui.hpp"

#include <algorithm>
#include <cmath>

using namespace viewer;

namespace {
constexpr float kGameOverCardH = 252.f;
constexpr float kConfirmCardH = 232.f;
} // namespace

GameViewer::DialogLayout GameViewer::computeDialogLayout(float cardH) const {
    const DialogCard d = computeDialogCard(window_.getSize(), 420.f, cardH);
    return {d.card, d.leftBtn, d.rightBtn};
}

void GameViewer::handleGameOverClick(sf::Vector2f mousePos) {
    const DialogLayout gl = computeDialogLayout(kGameOverCardH);
    if (pointInRect(mousePos, gl.right)) {
        requestConfirm(ConfirmKind::Restart);
    } else if (pointInRect(mousePos, gl.left)) {
        requestConfirm(ConfirmKind::Quit);
    }
}

void GameViewer::requestConfirm(ConfirmKind kind) {
    if (kind == ConfirmKind::None) {
        return;
    }
    confirmKind_ = kind;
}

void GameViewer::confirmDialog() {
    const ConfirmKind kind = confirmKind_;
    confirmKind_ = ConfirmKind::None;
    if (kind == ConfirmKind::Restart) {
        resetGame();
    } else if (kind == ConfirmKind::Quit) {
        window_.close();
    }
}

void GameViewer::cancelDialog() { confirmKind_ = ConfirmKind::None; }

void GameViewer::handleConfirmKey(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::Escape || key == sf::Keyboard::Key::N) {
        cancelDialog();
        return;
    }
    if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Y) {
        confirmDialog();
        return;
    }
    if (confirmKind_ == ConfirmKind::Restart && key == sf::Keyboard::Key::R) {
        confirmDialog();
        return;
    }
    if (confirmKind_ == ConfirmKind::Quit && key == sf::Keyboard::Key::Q) {
        confirmDialog();
    }
}

void GameViewer::handleConfirmClick(sf::Vector2f mousePos) {
    const DialogLayout cl = computeDialogLayout(kConfirmCardH);
    if (pointInRect(mousePos, cl.right)) {
        confirmDialog();
    } else if (pointInRect(mousePos, cl.left) || !pointInRect(mousePos, cl.card)) {
        cancelDialog();
    }
}

void GameViewer::drawSidebar(const Layout &layout) {
    const float fs = layout.fontScale;
    auto sz = [&](float base) { return std::max(8u, static_cast<unsigned int>(std::round(base * fs))); };

    const float padTop = 20.f * fs;
    const float padLR = clampf(18.f * fs, 14.f, layout.side * 0.10f);
    const float sbW = layout.side;
    const float sbH = layout.sidebarH;
    float x = layout.sidebarX + padLR;
    float y = layout.sidebarY + padTop;
    const float innerW = std::max(8.f, sbW - 2.f * padLR);
    const float rowH = 58.f * fs;
    const float rowGap = 12.f * fs;
    const float discSize = 26.f * fs;
    const float rowPad = 16.f * fs;
    const float shortcutH = 22.f * fs;
    const float shortcutGap = 26.f * fs;

    if (!fontsLoaded_) {
        drawRoundedRect(window_, {layout.sidebarX, layout.sidebarY}, {sbW, sbH}, layout.cell * 0.28f,
                        kSurface);
        return;
    }

    drawSoftShadow(window_, {layout.sidebarX, layout.sidebarY}, {sbW, sbH}, layout.cell * 0.28f, 8.f,
                   16.f, 0.14f);
    drawRoundedRect(window_, {layout.sidebarX, layout.sidebarY}, {sbW, sbH}, layout.cell * 0.28f,
                    kSurface);

    sf::Text wordmark(fontBold_, "OTHELLO", sz(25));
    wordmark.setFillColor(kInk);
    wordmark.setLetterSpacing(3.4f);
    auto wb = wordmark.getLocalBounds();
    wordmark.setPosition({x + (innerW - wb.size.x) * 0.5f - wb.position.x, y});
    window_.draw(wordmark);
    y += sz(25) * 1.18f;

    sf::Text tagline(font_, "TURN THE BOARD", sz(10));
    tagline.setFillColor(kMuted);
    tagline.setLetterSpacing(2.4f);
    auto tb = tagline.getLocalBounds();
    tagline.setPosition({x + (innerW - tb.size.x) * 0.5f - tb.position.x, y});
    window_.draw(tagline);
    y += sz(10) * 1.9f;

    drawHairline(window_, {x, y}, innerW);
    y += 18.f * fs + 8.f;

    const auto [blackCount, whiteCount] = pieceCounts();
    const std::string humanName = playerName_.empty() ? "Player" : playerName_;

    auto drawScoreRow = [&](float rowY, char player, int count, const std::string &name, bool isActive) {
        const bool selected = (currentPlayer_ == player) && phase_ != Phase::GameOver;
        if (selected) {
            drawRoundedRect(window_, {x, rowY}, {innerW, rowH}, 14.f * fs, kSurfaceRaised,
                            withAlpha(kClay, 0.30f * (isActive ? turnAnim_ : 1.f)), 1.f);
        } else {
            drawRoundedRect(window_, {x, rowY}, {innerW, rowH}, 14.f * fs, kSunken);
        }
        const sf::Texture &tex = player == PLAYER_BLACK ? discDarkTex_ : discLightTex_;
        drawDiscSprite(window_, tex, {x + rowPad + discSize * 0.5f, rowY + rowH * 0.5f}, discSize, discSize);

        sf::Text nameText(fontBold_, name, sz(13));
        nameText.setFillColor(selected ? kInk : kBody);
        auto nb = nameText.getLocalBounds();
        nameText.setPosition({x + rowPad + discSize + 12.f * fs,
                              rowY + (rowH - nb.size.y) * 0.5f - nb.position.y});
        window_.draw(nameText);

        sf::Text score(fontBold_, std::to_string(count), sz(32));
        score.setFillColor(selected ? kInk : kBody);
        auto scb = score.getLocalBounds();
        score.setPosition({x + innerW - rowPad - scb.size.x - scb.position.x,
                           rowY + rowH * 0.5f - sz(32) * 0.62f});
        window_.draw(score);
    };

    const bool darkIsCurrent = currentPlayer_ == PLAYER_BLACK;
    drawScoreRow(y, PLAYER_BLACK, blackCount, humanPlayer_ == PLAYER_BLACK ? humanName : "Engine", darkIsCurrent);
    y += rowH + rowGap;
    drawScoreRow(y, PLAYER_WHITE, whiteCount, humanPlayer_ == PLAYER_WHITE ? humanName : "Engine", !darkIsCurrent);
    y += rowH + 20.f * fs;

    drawHairline(window_, {x, y}, innerW);
    y += 18.f * fs;

    drawMutedLabel(window_, font_, "STATUS", {x, y}, sz(10));
    y += sz(10) * 1.9f;

    const auto lines = wrapText(font_, statusMessage_, sz(13), innerW);
    for (const auto &line: lines) {
        sf::Text lineText(font_, line, sz(13));
        lineText.setFillColor(kBody);
        lineText.setPosition({x, y});
        window_.draw(lineText);
        y += sz(13) * 1.55f;
    }

    // Bottom-anchored like a flex spacer; never overlaps the content above it.
    const float anchoredFooterY = layout.sidebarY + layout.sidebarH - padTop - 1.f - 10.f * fs - 3.f * shortcutGap;
    const float footerY = std::max(y + 14.f * fs, anchoredFooterY);

    drawHairline(window_, {x, footerY}, innerW);

    const struct {
        const char *key;
        const char *label;
    } shortcuts[] = {
        {"U", "Undo last move"},
        {"R", "New game"},
        {"Esc", "Quit"},
    };
    float fy = footerY + 16.f * fs;

    for (const auto &sc: shortcuts) {
        sf::Text key(font_, sc.key, sz(11));
        key.setFillColor(kBody);
        auto kb = key.getLocalBounds();
        const float chipW = std::max(24.f * fs, kb.size.x + 18.f * fs);

        drawRoundedRect(window_, {x, fy}, {chipW, shortcutH}, 6.f * fs, kKeycapBg);

        sf::RectangleShape edge({chipW, 2.f});
        edge.setPosition({x, fy + shortcutH - 2.f});
        edge.setFillColor(kKeycapEdge);
        window_.draw(edge);

        key.setPosition({x + (chipW - kb.size.x) * 0.5f - kb.position.x, fy + 4.f * fs});
        window_.draw(key);

        sf::Text label(font_, sc.label, sz(12));
        label.setFillColor(kEyebrow);
        label.setPosition({x + chipW + 10.f * fs, fy + 4.f * fs});
        window_.draw(label);
        fy += shortcutGap;
    }
}

void GameViewer::drawGameOverOverlay() {
    const float cardAlpha = clampf(gameOverAnim_ / 0.6f, 0.f, 1.f);
    drawVeil(window_, window_.getSize(), cardAlpha * (210.f / 255.f));

    const DialogLayout gl = computeDialogLayout(kGameOverCardH);
    const float lift = (1.f - easeOutBack(clampf(gameOverAnim_, 0.f, 1.f))) * 10.f;
    const sf::Vector2f cardPos = gl.card.position + sf::Vector2f(0.f, lift);
    const sf::FloatRect card{cardPos, gl.card.size};
    const sf::FloatRect quit{gl.left.position + sf::Vector2f(0.f, lift), gl.left.size};
    const sf::FloatRect play{gl.right.position + sf::Vector2f(0.f, lift), gl.right.size};

    drawRaisedCard(window_, card.position, card.size, 22.f, cardAlpha);

    if (!fontsLoaded_) {
        return;
    }

    const float cx = card.position.x + card.size.x * 0.5f;
    float y = drawDialogHeader(window_, font_, fontBold_, "FINAL", resultHeadline(), card, cardAlpha);
    y += 40.f;

    const auto [blackCount, whiteCount] = pieceCounts();
    const sf::Color darkColor = blackCount >= whiteCount ? kInk : kMuted;
    const sf::Color lightColor = whiteCount >= blackCount ? kInk : kMuted;

    sf::Text darkScore(fontBold_, std::to_string(blackCount), 32);
    darkScore.setFillColor(withAlpha(darkColor, cardAlpha));
    sf::Text lightScore(fontBold_, std::to_string(whiteCount), 32);
    lightScore.setFillColor(withAlpha(lightColor, cardAlpha));
    sf::Text dash(font_, "-", 16);
    dash.setFillColor(withAlpha(kMuted, cardAlpha));

    const float discSize = 22.f;
    const auto db = darkScore.getLocalBounds();
    const auto lb = lightScore.getLocalBounds();
    const auto dashb = dash.getLocalBounds();
    const float leftW = discSize + 10.f + db.size.x;
    const float rightW = lb.size.x + 10.f + discSize;
    const float totalW = leftW + 16.f + dashb.size.x + 16.f + rightW;
    float gx = cx - totalW * 0.5f;
    const float scoreMidY = y + 18.f;

    drawDiscSprite(window_, discDarkTex_, {gx + discSize * 0.5f, scoreMidY}, discSize, discSize, cardAlpha);
    darkScore.setPosition({
        gx + discSize + 10.f,
        scoreMidY - db.size.y * 0.5f - db.position.y
    });
    window_.draw(darkScore);
    gx += leftW + 16.f;

    dash.setPosition({gx, scoreMidY - dashb.size.y * 0.5f - dashb.position.y});
    window_.draw(dash);
    gx += dashb.size.x + 16.f;

    lightScore.setPosition({gx, scoreMidY - lb.size.y * 0.5f - lb.position.y});
    window_.draw(lightScore);
    drawDiscSprite(window_, discLightTex_, {gx + lb.size.x + 10.f + discSize * 0.5f, scoreMidY},
                   discSize, discSize, cardAlpha);

    drawDialogButton(window_, font_, quit, "Quit", ButtonKind::Secondary, cardAlpha);
    drawDialogButton(window_, fontBold_, play, "Restart", ButtonKind::Primary, cardAlpha);
}

void GameViewer::drawConfirmDialog() {
    drawVeil(window_, window_.getSize());

    const DialogLayout cl = computeDialogLayout(kConfirmCardH);
    drawRaisedCard(window_, cl.card.position, cl.card.size, 22.f);

    if (!fontsLoaded_) {
        return;
    }

    const bool quitting = confirmKind_ == ConfirmKind::Quit;
    const float cx = cl.card.position.x + cl.card.size.x * 0.5f;
    float y = drawDialogHeader(window_, font_, fontBold_, "CONFIRM",
                               quitting ? "LEAVE THE TABLE?" : "START A NEW GAME?", cl.card);
    y += 36.f;

    const char *body = quitting
                           ? "You can come back any time. This window will close."
                           : "The current board will be cleared. This cannot be undone.";
    drawWrappedCentered(window_, font_, body, 13, kBody, cx, y, cl.card.size.x - 64.f, 20.f);

    drawDialogButton(window_, font_, cl.left, "Cancel", ButtonKind::Secondary);
    drawDialogButton(window_, fontBold_, cl.right, quitting ? "Quit" : "Restart", ButtonKind::Primary);
}
