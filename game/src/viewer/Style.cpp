#include "viewer/Style.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>

namespace viewer {
namespace {

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
            const float rad = deg * kPi / 180.f;
            shape.setPoint(static_cast<std::size_t>(idx++),
                           {c.cx + std::cos(rad) * radius, c.cy + std::sin(rad) * radius});
        }
    }
    return shape;
}

} // namespace

float clampf(float v, float lo, float hi) { return std::max(lo, std::min(hi, v)); }

float easeOutBack(float t) {
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float u = t - 1.f;
    return 1.f + c3 * u * u * u + c1 * u * u;
}

float easeOutQuad(float t) { return 1.f - (1.f - t) * (1.f - t); }

float easeInOutSine(float t) { return 0.5f - 0.5f * std::cos(kPi * t); }

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

void drawRoundedRect(sf::RenderTarget &target, sf::Vector2f pos, sf::Vector2f size, float radius,
                     sf::Color fill, sf::Color outline, float outlineThickness) {
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
                             float radius, sf::Color top, sf::Color bottom, int cornerPoints) {
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
                    float offsetY, float blur, float peakAlpha, int layers) {
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

void drawShellBackground(sf::RenderTarget &target, sf::Vector2u winSize) {
    drawRoundedRectGradient(target, {0.f, 0.f},
                            {static_cast<float>(winSize.x), static_cast<float>(winSize.y)}, 0.f,
                            kShellTop, kShellBottom);
}

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
        fs::current_path() / "game" / "assets",
        fs::current_path() / "assets",
        fs::current_path().parent_path() / "game" / "assets",
        fs::path("game/assets"),
        fs::path("../game/assets"),
    };
    for (const auto &base: candidates) {
        if (fs::exists(base / "fonts" / "DejaVuSans.ttf")) {
            return base;
        }
    }
    return fs::path("game/assets");
}

void generateDiscTextures(sf::Texture &dark, sf::Texture &light, sf::Texture &shadow) {
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

    dark = buildDisc(kDiscDarkHi, kDiscDarkMid, 0.48f, kDiscDarkBase);
    light = buildDisc(kDiscLightHi, kDiscLightMid, 0.52f, kDiscLightBase);

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
    (void) shadow.loadFromImage(image);
    shadow.setSmooth(true);
}

} // namespace viewer
