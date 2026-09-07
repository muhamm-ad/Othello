#include "viewer/Ui.hpp"

#include <cstdint>

namespace viewer {

DialogCard computeDialogCard(sf::Vector2u winSize, float cardW, float cardH) {
    DialogCard d{};
    const float w = static_cast<float>(winSize.x);
    const float h = static_cast<float>(winSize.y);
    const float cx = (w - cardW) * 0.5f;
    const float cy = (h - cardH) * 0.5f;
    d.card = {{cx, cy}, {cardW, cardH}};
    const float padLR = 28.f;
    const float btnW = (cardW - 2.f * padLR - 12.f) * 0.5f;
    const float btnH = 46.f;
    const float btnY = cy + cardH - 28.f - btnH;
    d.leftBtn = {{cx + padLR, btnY}, {btnW, btnH}};
    d.rightBtn = {{cx + padLR + btnW + 12.f, btnY}, {btnW, btnH}};
    return d;
}

void drawVeil(sf::RenderTarget &target, sf::Vector2u winSize, float alpha01) {
    sf::RectangleShape veil({static_cast<float>(winSize.x), static_cast<float>(winSize.y)});
    veil.setFillColor(withAlpha(kVeil, alpha01));
    target.draw(veil);
}

void drawRaisedCard(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                    float fillAlpha, float shadowBlur) {
    drawSoftShadow(target, pos, size, radius, 10.f, shadowBlur, 0.22f * fillAlpha);
    drawRoundedRect(target, pos, size, radius, withAlpha(kCardFill, fillAlpha));
}

void placeTextCentered(sf::Text &text, const sf::FloatRect &rect) {
    const auto b = text.getLocalBounds();
    text.setPosition({
        rect.position.x + (rect.size.x - b.size.x) * 0.5f - b.position.x,
        rect.position.y + (rect.size.y - b.size.y) * 0.5f - b.position.y
    });
}

void placeTextCenteredX(sf::Text &text, float cx, float y) {
    const auto b = text.getLocalBounds();
    text.setPosition({cx - b.size.x * 0.5f - b.position.x, y});
}

void drawDialogButton(sf::RenderTarget &target, const sf::Font &font, const sf::FloatRect &rect,
                      const std::string &label, ButtonKind kind, float alpha) {
    if (kind == ButtonKind::Primary) {
        drawRoundedRect(target, rect.position, rect.size, 13.f, withAlpha(kClay, alpha));
        sf::Text text(font, label, 14);
        text.setFillColor(withAlpha(kOnClay, alpha));
        placeTextCentered(text, rect);
        target.draw(text);
    } else {
        drawRoundedRect(target, rect.position, rect.size, 13.f, withAlpha(kSunken, alpha),
                        withAlpha(kHairline, alpha), 1.f);
        sf::Text text(font, label, 13);
        text.setFillColor(withAlpha(kEyebrow, alpha));
        placeTextCentered(text, rect);
        target.draw(text);
    }
}

float drawDialogHeader(sf::RenderTarget &target, const sf::Font &font, const sf::Font &fontBold,
                       const std::string &eyebrow, const std::string &title,
                       const sf::FloatRect &card, float alpha) {
    const float cx = card.position.x + card.size.x * 0.5f;
    float y = card.position.y + 28.f;

    sf::Text eyebrowText(font, eyebrow, 10);
    eyebrowText.setFillColor(withAlpha(kMuted, alpha));
    eyebrowText.setLetterSpacing(2.6f);
    placeTextCenteredX(eyebrowText, cx, y);
    target.draw(eyebrowText);
    y += 26.f;

    sf::Text titleText(fontBold, title, 20);
    titleText.setFillColor(withAlpha(kInk, alpha));
    titleText.setLetterSpacing(0.6f);
    placeTextCenteredX(titleText, cx, y);
    target.draw(titleText);
    return y;
}

void drawHairline(sf::RenderTarget &target, sf::Vector2f pos, float width) {
    sf::RectangleShape divider({width, 1.f});
    divider.setPosition(pos);
    divider.setFillColor(kHairline);
    target.draw(divider);
}

void drawMutedLabel(sf::RenderTarget &target, const sf::Font &font, const std::string &text,
                    sf::Vector2f pos, unsigned int charSize, float letterSpacing) {
    sf::Text label(font, text, charSize);
    label.setFillColor(kMuted);
    label.setLetterSpacing(letterSpacing);
    label.setPosition(pos);
    target.draw(label);
}

void drawDiscSprite(sf::RenderTarget &target, const sf::Texture &tex, sf::Vector2f center,
                    float widthPx, float heightPx, float alpha) {
    sf::Sprite sprite(tex);
    sprite.setOrigin({static_cast<float>(kDiscTexSize) * 0.5f, static_cast<float>(kDiscTexSize) * 0.5f});
    sprite.setScale({widthPx / static_cast<float>(kDiscTexSize), heightPx / static_cast<float>(kDiscTexSize)});
    sprite.setPosition(center);
    sprite.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(clampf(alpha, 0.f, 1.f) * 255.f)));
    target.draw(sprite);
}

float drawWrappedCentered(sf::RenderTarget &target, const sf::Font &font, const std::string &body,
                          unsigned int charSize, sf::Color fill, float cx, float y, float maxWidth,
                          float lineGap) {
    const auto lines = wrapText(font, body, charSize, maxWidth);
    for (const auto &line: lines) {
        sf::Text lineText(font, line, charSize);
        lineText.setFillColor(fill);
        placeTextCenteredX(lineText, cx, y);
        target.draw(lineText);
        y += lineGap;
    }
    return y;
}

} // namespace viewer
