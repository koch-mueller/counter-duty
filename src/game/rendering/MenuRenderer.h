#pragma once

#include "BitmapFontRenderer.h"

#include "../ui/MainMenu.h"

#include <string>

class BaseCamera;
class PauseMenu;
struct GLFWwindow;

// Zeichnet Haupt- und Pausenmenü sowie deren Overlays
class MenuRenderer
{
public:
    MenuRenderer(GLFWwindow* window,
                 const char* atlasPath,
                 const char* vertexShaderPath,
                 const char* fragmentShaderPath);

    bool isReady() const;

    void drawIntro(float studioBrightness, float titleBrightness, const BaseCamera& camera);

    void drawMainMenu(MainMenuItem selectedItem, float masterVolume, const BaseCamera& camera);

    void drawPauseMenu(const PauseMenu& pauseMenu, float masterVolume, const BaseCamera& camera);

private:
    float uiScale(int framebufferWidth, int framebufferHeight) const;

    void drawCenteredText(const std::string& text,
                          float y,
                          float characterHeight,
                          const Color& color,
                          const BaseCamera& camera);

    void drawCenteredTextWithShadow(const std::string& text,
                                    float y,
                                    float characterHeight,
                                    const Color& color,
                                    const BaseCamera& camera);

    Color itemColor(bool selected) const;

    GLFWwindow* m_window;

    BitmapFontRenderer m_fontRenderer;
};
