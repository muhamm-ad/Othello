#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <string>
#include <vector>

namespace viewer {

// Visual direction: "Pressed felt & bone" — warm paper shell, raised felt
// plinth, chunky domed discs, and a single clay accent. See
// design/Othello UI.dc.html for the full spec.

inline const sf::Color kShellTop(241, 237, 230);
inline const sf::Color kShellBottom(230, 224, 214);
inline const sf::Color kSurface(247, 244, 238);
inline const sf::Color kSurfaceRaised(255, 253, 248);
inline const sf::Color kPlinthTop(240, 234, 224);
inline const sf::Color kPlinthBottom(230, 223, 211);
inline const sf::Color kSquareLight(203, 211, 196);
inline const sf::Color kSquareDark(183, 193, 176);
inline const sf::Color kClay(196, 113, 75);
inline const sf::Color kClayText(176, 101, 63);
inline const sf::Color kInk(51, 50, 45);
inline const sf::Color kBody(110, 105, 95);
inline const sf::Color kMuted(154, 146, 133);
inline const sf::Color kEyebrow(139, 133, 122);
inline const sf::Color kHelperMuted(169, 161, 146);
inline const sf::Color kHairline(226, 219, 206);
inline const sf::Color kSunken(241, 237, 229);
inline const sf::Color kKeycapBg(234, 228, 218);
inline const sf::Color kKeycapEdge(216, 209, 197);
inline const sf::Color kDiscDarkBase(46, 45, 40);
inline const sf::Color kDiscDarkMid(60, 59, 53);
inline const sf::Color kDiscDarkHi(87, 85, 76);
inline const sf::Color kDiscLightBase(231, 223, 207);
inline const sf::Color kDiscLightMid(248, 243, 233);
inline const sf::Color kDiscLightHi(255, 254, 250);
inline const sf::Color kCardFill(249, 246, 241);
inline const sf::Color kVeil(231, 225, 216);
inline const sf::Color kOnClay(255, 247, 241);

constexpr int kDiscTexSize = 256;
constexpr float kPi = 3.14159265f;

float clampf(float v, float lo, float hi);

float easeOutBack(float t);

float easeOutQuad(float t);

float easeInOutSine(float t);

float smoothstep(float edge0, float edge1, float x);

sf::Color lerpColor(const sf::Color &a, const sf::Color &b, float t);

sf::Color withAlpha(sf::Color c, float alpha01);

bool pointInRect(sf::Vector2f p, const sf::FloatRect &r);

void drawRoundedRect(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                     sf::Color fill, sf::Color outline = sf::Color::Transparent,
                     float outlineThickness = 0.f);

void drawRoundedRectGradient(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size,
                             float radius, sf::Color top, sf::Color bottom, int cornerPoints = 6);

void drawSoftShadow(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                    float offsetY, float blur, float peakAlpha, int layers = 5);

void drawShellBackground(sf::RenderTarget &target, sf::Vector2u winSize);

std::vector<std::string> wrapText(const sf::Font &font, const std::string &str,
                                  unsigned int charSize, float maxWidth);

std::filesystem::path findAssetRoot();

void generateDiscTextures(sf::Texture &dark, sf::Texture &light, sf::Texture &shadow);

} // namespace viewer
