#pragma once

#include "color.h"

#include <vector>

class RGBImage
{
public:
    RGBImage(unsigned int width, unsigned int height);

    RGBImage(const RGBImage&) = default;

    RGBImage& operator=(const RGBImage&) = default;

    RGBImage(RGBImage&&) noexcept = default;

    RGBImage& operator=(RGBImage&&) noexcept = default;

    ~RGBImage() = default;

    void setPixelColor(unsigned int x, unsigned int y, const Color& color);

    const Color& getPixelColor(unsigned int x, unsigned int y) const;

    bool saveToDisk(const char* filename);

    unsigned int width() const;
    unsigned int height() const;

    static RGBImage& SobelFilter(RGBImage& destination,
                                 const RGBImage& source,
                                 float factor = 1.0f);

    static unsigned char convertColorChannel(float value);

private:
    unsigned int pixelIndex(unsigned int x, unsigned int y) const;

    std::vector<Color> m_pixels;

    unsigned int m_width;
    unsigned int m_height;
};
