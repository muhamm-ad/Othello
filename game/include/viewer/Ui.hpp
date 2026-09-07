#pragma once

#include "viewer/Style.hpp"

#include <string>

namespace viewer {

struct DialogCard {
  sf::FloatRect card;
  sf::FloatRect leftBtn;
  sf::FloatRect rightBtn;
};

enum class ButtonKind { Primary, Secondary };

DialogCard computeDialogCard(sf::Vector2u winSize, float cardW, float cardH);

void drawVeil(sf::RenderTarget &target, sf::Vector2u winSize, float alpha01 = 210.f / 255.f);

void drawRaisedCard(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                    float fillAlpha = 1.f, float shadowBlur = 24.f);

void drawDialogButton(sf::RenderTarget &target, const sf::Font &font, const sf::FloatRect &rect,
                      const std::string &label, ButtonKind kind, float alpha = 1.f);

/** @brief Eyebrow + title; returns the y used for the title. */
float drawDialogHeader(sf::RenderTarget &target, const sf::Font &font, const sf::Font &fontBold,
                       const std::string &eyebrow, const std::string &title,
                       const sf::FloatRect &card, float alpha = 1.f);

void drawHairline(sf::RenderTarget &target, sf::Vector2f pos, float width);

void drawMutedLabel(sf::RenderTarget &target, const sf::Font &font, const std::string &text,
                    sf::Vector2f pos, unsigned int charSize, float letterSpacing = 2.2f);

void drawDiscSprite(sf::RenderTarget &target, const sf::Texture &tex, sf::Vector2f center,
                    float widthPx, float heightPx, float alpha = 1.f);

void placeTextCentered(sf::Text &text, const sf::FloatRect &rect);

void placeTextCenteredX(sf::Text &text, float cx, float y);

float drawWrappedCentered(sf::RenderTarget &target, const sf::Font &font, const std::string &body,
                          unsigned int charSize, sf::Color fill, float cx, float y, float maxWidth,
                          float lineGap);

} // namespace viewer
