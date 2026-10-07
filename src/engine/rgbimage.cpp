#include "rgbimage.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

namespace
{
    const int kHorizontalSobelKernel[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};

    const int kVerticalSobelKernel[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

#pragma pack(push, 1)

    struct BMPFileHeader
    {
        std::uint16_t type = 0x4D42;

        std::uint32_t size = 0;

        std::uint16_t reserved1 = 0;

        std::uint16_t reserved2 = 0;

        std::uint32_t pixelDataOffset = 54;
    };

    struct BMPInfoHeader
    {
        std::uint32_t size = 40;

        std::int32_t width = 0;

        std::int32_t height = 0;

        std::uint16_t planes = 1;

        std::uint16_t bitsPerPixel = 24;

        std::uint32_t compression = 0;

        std::uint32_t imageSize = 0;

        std::int32_t horizontalResolution = 0;

        std::int32_t verticalResolution = 0;

        std::uint32_t usedColors = 0;

        std::uint32_t importantColors = 0;
    };

#pragma pack(pop)
} // namespace

RGBImage::RGBImage(unsigned int width, unsigned int height)
    : m_pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)),
      m_width(width),
      m_height(height)
{
}

void RGBImage::setPixelColor(unsigned int x, unsigned int y, const Color& color)
{
    if (x >= m_width || y >= m_height)
    {
        return;
    }

    m_pixels[pixelIndex(x, y)] = color;
}

const Color& RGBImage::getPixelColor(unsigned int x, unsigned int y) const
{
    if (x >= m_width || y >= m_height)
    {
        throw std::out_of_range("Pixel out of bounds");
    }

    return m_pixels[pixelIndex(x, y)];
}

bool RGBImage::saveToDisk(const char* filename)
{
    if (filename == nullptr || m_width == 0 || m_height == 0)
    {
        return false;
    }

    std::FILE* file = std::fopen(filename, "wb");

    if (file == nullptr)
    {
        return false;
    }

    std::uint32_t rowSize = (m_width * 3u + 3u) & ~3u;

    std::uint32_t imageSize = rowSize * m_height;

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    fileHeader.size = fileHeader.pixelDataOffset + imageSize;

    infoHeader.width = static_cast<std::int32_t>(m_width);

    infoHeader.height = static_cast<std::int32_t>(m_height);

    infoHeader.imageSize = imageSize;

    bool headerWritten = std::fwrite(&fileHeader, sizeof(fileHeader), 1, file) == 1
                         && std::fwrite(&infoHeader, sizeof(infoHeader), 1, file) == 1;

    if (!headerWritten)
    {
        std::fclose(file);

        return false;
    }

    const unsigned char padding[3] = {0, 0, 0};

    unsigned int paddingSize = rowSize - m_width * 3u;

    for (int y = static_cast<int>(m_height) - 1; y >= 0; --y)
    {
        for (unsigned int x = 0; x < m_width; ++x)
        {
            const Color& color = getPixelColor(x, static_cast<unsigned int>(y));

            const unsigned char pixel[3] = {convertColorChannel(color.B),

                                            convertColorChannel(color.G),

                                            convertColorChannel(color.R)};

            bool pixelWritten = std::fwrite(pixel, sizeof(pixel), 1, file) == 1;

            if (!pixelWritten)
            {
                std::fclose(file);

                return false;
            }
        }

        if (paddingSize == 0)
        {
            continue;
        }

        bool paddingWritten = std::fwrite(padding, paddingSize, 1, file) == 1;

        if (!paddingWritten)
        {
            std::fclose(file);

            return false;
        }
    }

    return std::fclose(file) == 0;
}

unsigned int RGBImage::width() const
{
    return m_width;
}

unsigned int RGBImage::height() const
{
    return m_height;
}

RGBImage& RGBImage::SobelFilter(RGBImage& destination, const RGBImage& source, float factor)
{
    RGBImage result(source.width(), source.height());

    for (unsigned int y = 0; y < source.height(); ++y)
    {
        for (unsigned int x = 0; x < source.width(); ++x)
        {
            float horizontalGradient = 0.0f;

            float verticalGradient = 0.0f;

            for (int kernelY = -1; kernelY <= 1; ++kernelY)
            {
                for (int kernelX = -1; kernelX <= 1; ++kernelX)
                {
                    int sourceX = static_cast<int>(x) + kernelX;

                    int sourceY = static_cast<int>(y) + kernelY;

                    bool isOutsideImage = sourceX < 0 || sourceY < 0
                                          || sourceX >= static_cast<int>(source.width())
                                          || sourceY >= static_cast<int>(source.height());

                    if (isOutsideImage)
                    {
                        continue;
                    }

                    const Color& color = source.getPixelColor(static_cast<unsigned int>(sourceX),
                                                              static_cast<unsigned int>(sourceY));

                    float brightness = (color.R + color.G + color.B) / 3.0f;

                    horizontalGradient +=
                        brightness
                        * static_cast<float>(kHorizontalSobelKernel[kernelY + 1][kernelX + 1]);

                    verticalGradient +=
                        brightness
                        * static_cast<float>(kVerticalSobelKernel[kernelY + 1][kernelX + 1]);
                }
            }

            float edgeStrength = std::sqrt(horizontalGradient * horizontalGradient
                                           + verticalGradient * verticalGradient)
                                 * factor;

            result.setPixelColor(x, y, Color(edgeStrength, edgeStrength, edgeStrength));
        }
    }

    destination = result;

    return destination;
}

unsigned char RGBImage::convertColorChannel(float value)
{
    if (value < 0.0f)
    {
        value = 0.0f;
    }
    else if (value > 1.0f)
    {
        value = 1.0f;
    }

    return static_cast<unsigned char>(value * 255.0f);
}

unsigned int RGBImage::pixelIndex(unsigned int x, unsigned int y) const
{
    return x + y * m_width;
}
