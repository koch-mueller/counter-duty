#pragma once

#include "../../engine/BaseShader.h"
#include "../../engine/color.h"
#include "../../engine/Texture.h"
#include "../../engine/VertexBuffer.h"

#include <string>

class BaseCamera;

// Rendert HUD-Text aus einer Bitmap-Font-Textur direkt in Bildschirmkoordinaten
class BitmapFontRenderer
{
public:
    BitmapFontRenderer();

    bool load(const char* atlasPath, const char* vertexShaderPath, const char* fragmentShaderPath);

    void drawText(const std::string& text,
                  float x,
                  float y,
                  float characterHeight,
                  int framebufferWidth,
                  int framebufferHeight,
                  const Color& color,
                  const BaseCamera& camera);

    float measureTextWidth(const std::string& text, float characterHeight) const;

    bool isReady() const;

private:
    void createCharacterQuad();

    bool retrieveShaderParameters();

    bool isValidCharacter(unsigned char character) const;

    void calculateTextureCoordinates(
        unsigned char character, float& uMin, float& vMin, float& uMax, float& vMax) const;

    Texture m_fontTexture;
    BaseShader m_shader;
    VertexBuffer m_characterBuffer;

    bool m_ready;

    GLint m_screenPositionLocation;
    GLint m_characterSizeLocation;
    GLint m_screenSizeLocation;
    GLint m_uvMinLocation;
    GLint m_uvMaxLocation;
    GLint m_textColorLocation;
    GLint m_fontAtlasLocation;
};
