#include "BitmapFontRenderer.h"

#include "../../engine/Camera.h"

#include <cstdio>

namespace
{
    constexpr unsigned char kFirstCharacter = 32;

    constexpr unsigned char kLastCharacter = 126;

    constexpr int kAtlasColumns = 16;

    constexpr int kAtlasRows = 6;

    constexpr float kCharacterWidthRatio = 0.6f;
}

BitmapFontRenderer::BitmapFontRenderer()
    : m_ready(false),
      m_screenPositionLocation(-1),
      m_characterSizeLocation(-1),
      m_screenSizeLocation(-1),
      m_uvMinLocation(-1),
      m_uvMaxLocation(-1),
      m_textColorLocation(-1),
      m_fontAtlasLocation(-1)
{
}

bool BitmapFontRenderer::load(const char* atlasPath,
                              const char* vertexShaderPath,
                              const char* fragmentShaderPath)
{
    m_ready = false;

    if (atlasPath == nullptr || vertexShaderPath == nullptr || fragmentShaderPath == nullptr)
    {
        return false;
    }

    if (!m_fontTexture.load(atlasPath))
    {
        std::fprintf(stderr, "Font atlas failed to load\n");

        return false;
    }

    m_fontTexture.activate(0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_fontTexture.deactivate();

    if (!m_shader.load(vertexShaderPath, fragmentShaderPath))
    {
        std::fprintf(stderr, "Font shader failed to load\n");

        return false;
    }

    if (!retrieveShaderParameters())
    {
        std::fprintf(stderr, "Font shader parameter retrieval failed\n");

        return false;
    }

    // Alle Zeichen verwenden dasselbe Quad; pro Glyph ändern sich nur Position und UV-Bereich
    createCharacterQuad();

    m_ready = m_characterBuffer.vertexCount() == 6;

    return m_ready;
}

void BitmapFontRenderer::drawText(const std::string& text,
                                  float x,
                                  float y,
                                  float characterHeight,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  const Color& color,
                                  const BaseCamera& camera)
{
    if (!m_ready || text.empty() || characterHeight <= 0.0f || framebufferWidth <= 0
        || framebufferHeight <= 0)
    {
        return;
    }

    const float glyphWidth = characterHeight;

    const float characterWidth = characterHeight * kCharacterWidthRatio;

    float currentX = x;

    m_shader.activate(camera);

    m_fontTexture.activate(0);
    m_characterBuffer.activate();

    m_shader.setParameter(m_characterSizeLocation, Vector(glyphWidth, characterHeight, 0.0f));

    m_shader.setParameter(
        m_screenSizeLocation,
        Vector(static_cast<float>(framebufferWidth), static_cast<float>(framebufferHeight), 0.0f));

    m_shader.setParameter(m_textColorLocation, color);

    m_shader.setParameter(m_fontAtlasLocation, 0);

    // Der Atlas enthält druckbare ASCII-Zeichen in einem festen Raster. Jede Glyphe wird einzeln mit den passenden UV-Koordinaten auf dasselbe Quad gezeichnet
    for (char textCharacter : text)
    {
        unsigned char character = static_cast<unsigned char>(textCharacter);

        if (character == ' ' || !isValidCharacter(character))
        {
            currentX += characterWidth;

            continue;
        }

        float uMin = 0.0f;
        float vMin = 0.0f;
        float uMax = 0.0f;
        float vMax = 0.0f;

        calculateTextureCoordinates(character, uMin, vMin, uMax, vMax);

        m_shader.setParameter(m_screenPositionLocation, Vector(currentX, y, 0.0f));

        m_shader.setParameter(m_uvMinLocation, Vector(uMin, vMin, 0.0f));

        m_shader.setParameter(m_uvMaxLocation, Vector(uMax, vMax, 0.0f));

        glDrawArrays(GL_TRIANGLES, 0, m_characterBuffer.vertexCount());

        currentX += characterWidth;
    }

    m_characterBuffer.deactivate();
    m_fontTexture.deactivate();
    m_shader.deactivate();
}

float BitmapFontRenderer::measureTextWidth(const std::string& text, float characterHeight) const
{
    if (text.empty() || characterHeight <= 0.0f)
    {
        return 0.0f;
    }

    const float glyphWidth = characterHeight;

    const float characterWidth = characterHeight * kCharacterWidthRatio;

    return glyphWidth + static_cast<float>(text.length() - 1) * characterWidth;
}

bool BitmapFontRenderer::isReady() const
{
    return m_ready;
}

// Das Einheits-Quad wird vom Font-Shader auf Pixelgröße und Bildschirmposition skaliert
void BitmapFontRenderer::createCharacterQuad()
{
    m_characterBuffer.begin();

    m_characterBuffer.addTexcoord0(0.0f, 0.0f);

    m_characterBuffer.addVertex(0.0f, 0.0f, 0.0f);

    m_characterBuffer.addTexcoord0(1.0f, 0.0f);

    m_characterBuffer.addVertex(1.0f, 0.0f, 0.0f);

    m_characterBuffer.addTexcoord0(1.0f, 1.0f);

    m_characterBuffer.addVertex(1.0f, 1.0f, 0.0f);

    m_characterBuffer.addTexcoord0(0.0f, 0.0f);

    m_characterBuffer.addVertex(0.0f, 0.0f, 0.0f);

    m_characterBuffer.addTexcoord0(1.0f, 1.0f);

    m_characterBuffer.addVertex(1.0f, 1.0f, 0.0f);

    m_characterBuffer.addTexcoord0(0.0f, 1.0f);

    m_characterBuffer.addVertex(0.0f, 1.0f, 0.0f);

    m_characterBuffer.end();
}

bool BitmapFontRenderer::retrieveShaderParameters()
{
    m_screenPositionLocation = m_shader.getParameterID("ScreenPosition");

    m_characterSizeLocation = m_shader.getParameterID("CharacterSize");

    m_screenSizeLocation = m_shader.getParameterID("ScreenSize");

    m_uvMinLocation = m_shader.getParameterID("UVMin");

    m_uvMaxLocation = m_shader.getParameterID("UVMax");

    m_textColorLocation = m_shader.getParameterID("TextColor");

    m_fontAtlasLocation = m_shader.getParameterID("FontAtlas");

    return m_screenPositionLocation >= 0 && m_characterSizeLocation >= 0
           && m_screenSizeLocation >= 0 && m_uvMinLocation >= 0 && m_uvMaxLocation >= 0
           && m_textColorLocation >= 0 && m_fontAtlasLocation >= 0;
}

bool BitmapFontRenderer::isValidCharacter(unsigned char character) const
{
    return character >= kFirstCharacter && character <= kLastCharacter;
}

// Berechnet aus dem ASCII-Index die Zelle im 16x6-Fontatlas
void BitmapFontRenderer::calculateTextureCoordinates(
    unsigned char character, float& uMin, float& vMin, float& uMax, float& vMax) const
{
    if (!isValidCharacter(character))
    {
        uMin = 0.0f;
        vMin = 0.0f;
        uMax = 0.0f;
        vMax = 0.0f;

        return;
    }

    int characterIndex = static_cast<int>(character - kFirstCharacter);

    int column = characterIndex % kAtlasColumns;

    int row = characterIndex / kAtlasColumns;

    float cellWidth = 1.0f / static_cast<float>(kAtlasColumns);

    float cellHeight = 1.0f / static_cast<float>(kAtlasRows);

    uMin = static_cast<float>(column) * cellWidth;

    vMin = static_cast<float>(row) * cellHeight;

    uMax = uMin + cellWidth;

    vMax = vMin + cellHeight;
}
