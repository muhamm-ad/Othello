#include "viewer/GameViewer.hpp"
#include "viewer/Style.hpp"
#include "viewer/Ui.hpp"

#include <algorithm>
#include <cmath>

using namespace viewer;

void GameViewer::handleTextEntered(char32_t unicode) {
    if (unicode == 8) {
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

void GameViewer::drawSetupScreen() {
    drawShellBackground(window_, window_.getSize());

    const SetupLayout sl = computeSetupLayout();
    drawRaisedCard(window_, sl.card.position, sl.card.size, 24.f * sl.scale, 1.f, 26.f);

    if (!fontsLoaded_) {
        return;
    }

    const float s = sl.scale;
    const float innerW = sl.card.size.x - 2.f * (sl.nameLabelPos.x - sl.card.position.x);
    auto sz = [&](float base, unsigned int min = 8u) {
        return std::max(min, static_cast<unsigned int>(base * s));
    };

    sf::Text wordmark(fontBold_, "OTHELLO", sz(25.f));
    wordmark.setFillColor(kInk);
    wordmark.setLetterSpacing(3.6f);
    wordmark.setPosition(sl.wordmarkPos);
    window_.draw(wordmark);

    sf::Text tagline(font_, "TURN THE BOARD", sz(10.f));
    tagline.setFillColor(kMuted);
    tagline.setLetterSpacing(2.4f);
    tagline.setPosition(sl.taglinePos);
    window_.draw(tagline);

    drawHairline(window_, {sl.nameLabelPos.x, sl.dividerY}, innerW);
    drawMutedLabel(window_, font_, "YOUR NAME", sl.nameLabelPos, sz(10.f));

    drawRoundedRect(window_, sl.nameField.position, sl.nameField.size, 13.f * s, kSunken,
                    withAlpha(kClay, nameFocused_ ? 0.55f : 0.f), nameFocused_ ? -1.5f : 0.f);
    const std::string displayName = playerName_.empty() ? "Player" : playerName_;
    sf::Text nameText(font_, displayName, sz(15.f));
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

    drawMutedLabel(window_, font_, "DIFFICULTY", sl.diffLabelPos, sz(10.f));
    drawRoundedRect(window_, sl.diffTrack.position, sl.diffTrack.size, 15.f * s, kSunken);
    for (int i = 0; i < 4; ++i) {
        const bool selected = static_cast<int>(selectedDifficulty_) == i;
        if (selected) {
            drawRoundedRect(window_, sl.diff[i].position, sl.diff[i].size, 11.f * s, kSurfaceRaised,
                            withAlpha(kClay, 0.4f), 1.5f);
        }
        sf::Text label(selected ? fontBold_ : font_, difficultyLabel(static_cast<Difficulty>(i)), sz(13.f));
        label.setFillColor(selected ? kInk : kEyebrow);
        const auto bounds = label.getLocalBounds();
        label.setPosition({
            sl.diff[i].position.x + (sl.diff[i].size.x - bounds.size.x) * 0.5f,
            sl.diff[i].position.y + (sl.diff[i].size.y - bounds.size.y) * 0.5f - bounds.position.y
        });
        window_.draw(label);
    }

    drawMutedLabel(window_, font_, "YOU PLAY", sl.playLabelPos, sz(10.f));

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
        drawDiscSprite(window_, pieceOptions[i].tex, {cx, discY}, 34.f * s, 34.f * s);

        sf::Text label(fontBold_, pieceOptions[i].label, sz(13.f));
        label.setFillColor(selected ? kInk : kBody);
        auto lb = label.getLocalBounds();
        label.setPosition({cx - lb.size.x * 0.5f, discY + 24.f * s});
        window_.draw(label);

        sf::Text sub(font_, pieceOptions[i].sub, sz(10.f, 7u));
        sub.setFillColor(kMuted);
        auto sb = sub.getLocalBounds();
        sub.setPosition({cx - sb.size.x * 0.5f, discY + 45.f * s});
        window_.draw(sub);
    }

    drawRoundedRect(window_, sl.start.position, sl.start.size, 14.f * s, kClay);
    sf::Text startLabel(fontBold_, "Start game", sz(14.f));
    startLabel.setFillColor(kOnClay);
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
    sf::Text enterLabel(font_, "Enter", sz(10.f, 7u));
    enterLabel.setFillColor(kOnClay);
    auto elb = enterLabel.getLocalBounds();
    enterLabel.setPosition({
        gx + (enterChipW - elb.size.x) * 0.5f,
        sl.start.position.y + sl.start.size.y * 0.5f - 9.f * s + 3.f * s
    });
    window_.draw(enterLabel);

    sf::Text hint(font_, "Tab / arrows to move  -  1-4 sets difficulty  -  Esc quits", sz(11.f, 7u));
    hint.setFillColor(kHelperMuted);
    auto hlb = hint.getLocalBounds();
    hint.setPosition({sl.card.position.x + (sl.card.size.x - hlb.size.x) * 0.5f, sl.hintY});
    window_.draw(hint);
}
