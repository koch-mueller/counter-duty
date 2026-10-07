
#include "MenuRenderer.h"

#include "../ui/PauseMenu.h"

#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    // Menüs werden als Overlay gezeichnet. Vorherige OpenGL-Zustände werden deshalb gesichert und nach dem Rendern wiederhergestellt
    void beginMenuRendering(GLboolean& depthTestEnabled, GLboolean& cullFaceEnabled)
    {
        depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

        cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

        glDisable(GL_DEPTH_TEST);

        glDisable(GL_CULL_FACE);
    }

    void endMenuRendering(GLboolean depthTestEnabled, GLboolean cullFaceEnabled)
    {
        if (depthTestEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }

        if (cullFaceEnabled)
        {
            glEnable(GL_CULL_FACE);
        }
    }
}

MenuRenderer::MenuRenderer(GLFWwindow* window,
                           const char* atlasPath,
                           const char* vertexShaderPath,
                           const char* fragmentShaderPath)
    : m_window(window),
      m_fontRenderer()
{
    m_fontRenderer.load(atlasPath, vertexShaderPath, fragmentShaderPath);
}

bool MenuRenderer::isReady() const
{
    return m_window != nullptr && m_fontRenderer.isReady();
}

// Das 16:9-Referenzlayout wird gleichmäßig skaliert, die kleinere Achse bestimmt den Faktor
float MenuRenderer::uiScale(int framebufferWidth, int framebufferHeight) const
{
    const float referenceWidth = 1280.0f;

    const float referenceHeight = 720.0f;

    float widthScale = static_cast<float>(framebufferWidth) / referenceWidth;

    float heightScale = static_cast<float>(framebufferHeight) / referenceHeight;

    return std::clamp(std::min(widthScale, heightScale), 0.65f, 2.5f);
}

void MenuRenderer::drawIntro(float studioBrightness,
                             float titleBrightness,
                             const BaseCamera& camera)
{
    if (!isReady())
    {
        return;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    GLboolean depthTestEnabled = GL_FALSE;

    GLboolean cullFaceEnabled = GL_FALSE;

    beginMenuRendering(depthTestEnabled, cullFaceEnabled);

    float scale = uiScale(width, height);

    studioBrightness = std::clamp(studioBrightness, 0.0f, 1.0f);

    titleBrightness = std::clamp(titleBrightness, 0.0f, 1.0f);

    Color studioColor(studioBrightness, studioBrightness, studioBrightness);

    Color titleColor(titleBrightness, titleBrightness, titleBrightness);

    // Der Spieltitel erscheint oberhalb des Studio-Namens
    drawCenteredText("COUNTER DUTY",
                     static_cast<float>(height) * 0.36f,
                     58.0f * scale,
                     titleColor,
                     camera);

    drawCenteredText("CLOSING SHIFT",
                     static_cast<float>(height) * 0.44f,
                     24.0f * scale,
                     titleColor,
                     camera);

    drawCenteredText("KOCH STUDIOS",
                     static_cast<float>(height) * 0.57f,
                     38.0f * scale,
                     studioColor,
                     camera);

    endMenuRendering(depthTestEnabled, cullFaceEnabled);
}

void MenuRenderer::drawMainMenu(MainMenuItem selectedItem,
                                float masterVolume,
                                const BaseCamera& camera)
{
    if (!isReady())
    {
        return;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    GLboolean depthTestEnabled = GL_FALSE;

    GLboolean cullFaceEnabled = GL_FALSE;

    beginMenuRendering(depthTestEnabled, cullFaceEnabled);

    float scale = uiScale(width, height);

    const Color titleColor(1.0f, 1.0f, 1.0f);

    const Color subtitleColor(0.75f, 0.75f, 0.75f);

    drawCenteredTextWithShadow("COUNTER DUTY",
                               static_cast<float>(height) * 0.19f,
                               54.0f * scale,
                               titleColor,
                               camera);

    drawCenteredTextWithShadow("CLOSING SHIFT",
                               static_cast<float>(height) * 0.28f,
                               24.0f * scale,
                               subtitleColor,
                               camera);

    drawCenteredTextWithShadow("START",
                               static_cast<float>(height) * 0.48f,
                               30.0f * scale,
                               itemColor(selectedItem == MainMenuItem::Start),
                               camera);

    // Intern bleibt die Lautstärke normiert; für das Menü wird sie nur als Prozentwert dargestellt
    int volumePercent = static_cast<int>(std::round(std::clamp(masterVolume, 0.0f, 1.0f) * 100.0f));

    std::string volumeText = "LAUTSTAERKE  < " + std::to_string(volumePercent) + "% >";

    drawCenteredTextWithShadow(volumeText,
                               static_cast<float>(height) * 0.57f,
                               30.0f * scale,
                               itemColor(selectedItem == MainMenuItem::Volume),
                               camera);

    drawCenteredTextWithShadow("BEENDEN",
                               static_cast<float>(height) * 0.66f,
                               30.0f * scale,
                               itemColor(selectedItem == MainMenuItem::Exit),
                               camera);

    drawCenteredTextWithShadow("W/S AUSWAEHLEN   A/D LAUTSTAERKE   ENTER BESTAETIGEN",
                               static_cast<float>(height) * 0.90f,
                               16.0f * scale,
                               subtitleColor,
                               camera);

    endMenuRendering(depthTestEnabled, cullFaceEnabled);
}

void MenuRenderer::drawPauseMenu(const PauseMenu& pauseMenu,
                                 float masterVolume,
                                 const BaseCamera& camera)
{
    if (!isReady() || !pauseMenu.isOpen())
    {
        return;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    GLboolean depthTestEnabled = GL_FALSE;

    GLboolean cullFaceEnabled = GL_FALSE;

    beginMenuRendering(depthTestEnabled, cullFaceEnabled);

    float scale = uiScale(width, height);

    const Color titleColor(1.0f, 1.0f, 1.0f);

    const Color hintColor(0.75f, 0.75f, 0.75f);

    drawCenteredTextWithShadow("PAUSE",
                               static_cast<float>(height) * 0.20f,
                               46.0f * scale,
                               titleColor,
                               camera);

    const char* menuItems[] = {"FORTSETZEN", "LAUTSTAERKE", "MENU", "BEENDEN"};

    float currentY = static_cast<float>(height) * 0.39f;

    const float lineDistance = 54.0f * scale;

    for (std::size_t index = 0; index < 4; ++index)
    {
        std::string text = menuItems[index];

        if (index == 1)
        {
            int volumePercent =
                static_cast<int>(std::round(std::clamp(masterVolume, 0.0f, 1.0f) * 100.0f));

            text += "  < ";
            text += std::to_string(volumePercent);
            text += "% >";
        }

        drawCenteredTextWithShadow(text,
                                   currentY,
                                   27.0f * scale,
                                   itemColor(pauseMenu.selectedIndex() == index),
                                   camera);

        currentY += lineDistance;
    }

    drawCenteredTextWithShadow("W/S AUSWAEHLEN   A/D LAUTSTAERKE   ENTER BESTAETIGEN",
                               static_cast<float>(height) * 0.89f,
                               16.0f * scale,
                               hintColor,
                               camera);

    drawCenteredTextWithShadow("ESC FORTSETZEN",
                               static_cast<float>(height) * 0.94f,
                               16.0f * scale,
                               hintColor,
                               camera);

    endMenuRendering(depthTestEnabled, cullFaceEnabled);
}

void MenuRenderer::drawCenteredText(const std::string& text,
                                    float y,
                                    float characterHeight,
                                    const Color& color,
                                    const BaseCamera& camera)
{
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    float textWidth = m_fontRenderer.measureTextWidth(text, characterHeight);

    float x = (static_cast<float>(width) - textWidth) * 0.5f;

    m_fontRenderer.drawText(text, x, y, characterHeight, width, height, color, camera);
}

void MenuRenderer::drawCenteredTextWithShadow(const std::string& text,
                                              float y,
                                              float characterHeight,
                                              const Color& color,
                                              const BaseCamera& camera)
{
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    float scale = uiScale(width, height);

    float textWidth = m_fontRenderer.measureTextWidth(text, characterHeight);

    float x = (static_cast<float>(width) - textWidth) * 0.5f;

    // Ein zweiter, leicht versetzter Textzug erzeugt ohne zusätzliche Shader einen einfachen Schatten
    float shadowOffset = 2.0f * scale;

    m_fontRenderer.drawText(text,
                            x + shadowOffset,
                            y + shadowOffset,
                            characterHeight,
                            width,
                            height,
                            Color(0.02f, 0.02f, 0.02f),
                            camera);

    m_fontRenderer.drawText(text, x, y, characterHeight, width, height, color, camera);
}

Color MenuRenderer::itemColor(bool selected) const
{
    if (selected)
    {
        return Color(1.0f, 0.8f, 0.25f);
    }

    return Color(0.85f, 0.85f, 0.85f);
}
