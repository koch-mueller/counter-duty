
#include "HUDRenderer.h"

#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif

#include "../orders/Orders.h"
#include "../products/ProductDefinition.h"

#include "../ui/HUDModel.h"
#include "../ui/PauseMenu.h"
#include "../ui/TerminalUI.h"
#include "../ui/ToastManager.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <string>

HUDRenderer::HUDRenderer(GLFWwindow* window,
                         const char* fontAtlasPath,
                         const char* vertexShaderPath,
                         const char* fragmentShaderPath)
    : m_window(window),
      m_debugOverlayVisible(false)
{
    bool fontLoaded = m_fontRenderer.load(fontAtlasPath, vertexShaderPath, fragmentShaderPath);

    if (!fontLoaded)
    {
        std::fprintf(stderr, "Failed to load font for HUDRenderer\n");
    }
}

void HUDRenderer::setDebugOverlayVisible(bool visible)
{
    m_debugOverlayVisible = visible;
}

void HUDRenderer::drawGameplay(const BaseCamera& camera,
                                const HUDModel& hudModel,
                                const ToastManager& toastManager)
 {
     if (m_window == nullptr)
     {
         return;
     }

     int framebufferWidth = 0;
     int framebufferHeight = 0;

     glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

     if (framebufferWidth <= 0 || framebufferHeight <= 0)
     {
         return;
     }

     // HUD-Elemente liegen im Screen-Space. Den 3D-Zustand sichern wir und stellen ihn danach wieder her
     GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

     GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

     glDisable(GL_DEPTH_TEST);

     glDisable(GL_CULL_FACE);

     drawCrosshair(framebufferWidth, framebufferHeight, camera);

     drawOrder(framebufferWidth, framebufferHeight, camera, hudModel);

     drawObjective(framebufferWidth, framebufferHeight, camera, hudModel);

     drawContextPrompt(framebufferWidth, framebufferHeight, camera, hudModel);

     drawCarryHint(framebufferWidth, framebufferHeight, camera, hudModel);

     drawScanHint(framebufferWidth, framebufferHeight, camera, hudModel);

     drawToast(framebufferWidth, framebufferHeight, camera, toastManager);

     drawDebugOverlay(framebufferWidth, framebufferHeight, camera);

     if (depthTestEnabled)
     {
         glEnable(GL_DEPTH_TEST);
     }

     if (cullFaceEnabled)
     {
         glEnable(GL_CULL_FACE);
     }
 }

void HUDRenderer::drawCrosshair(int framebufferWidth,
                                int framebufferHeight,
                                const BaseCamera& camera)
{
    const std::string crosshair = "+";

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float characterHeight =
        22.0f * scale;

    float textWidth =
        m_fontRenderer.measureTextWidth(crosshair, characterHeight);

    float x =
        (static_cast<float>(framebufferWidth) - textWidth) * 0.5f;

    float y =
        (static_cast<float>(framebufferHeight) - characterHeight) * 0.5f;

    drawTextWithShadow(crosshair,
                       x,
                       y,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 1.0f, 1.0f),
                       camera);
}

void HUDRenderer::drawTerminal(const BaseCamera& camera,
                               const TerminalUI& terminalUI)
{
    if (m_window == nullptr || !terminalUI.isOpen())
    {
        return;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;

    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        return;
    }

    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    // UI-Abstaende werden aus der Framebuffer-Groesse skaliert und bleiben dadurch aufloesungsunabhaengig
    float scale = uiScale(framebufferWidth, framebufferHeight);

    drawCenteredText("AUFTRAGSTERMINAL",
                     70.0f * scale,
                     30.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(1.0f, 1.0f, 1.0f),
                     camera);

    drawCenteredText("========================================",
                     110.0f * scale,
                     18.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(0.65f, 0.65f, 0.65f),
                     camera);

    const Order* order = terminalUI.order();

    float currentY = 150.0f * scale;

    if (order != nullptr)
    {
        std::string title;

        if (order->state() == OrderState::WaitingForAcceptance)
        {
            title = "NEUER AUFTRAG " + std::to_string(order->id());
        }
        else
        {
            title = "AKTIVER AUFTRAG " + std::to_string(order->id());
        }

        drawCenteredText(title,
                         currentY,
                         22.0f * scale,
                         framebufferWidth,
                         framebufferHeight,
                         Color(1.0f, 0.85f, 0.25f),
                         camera);

        currentY += 45.0f * scale;

        for (const OrderLine& line : order->lines())
        {
            if (line.definition() == nullptr)
            {
                continue;
            }

            std::string text;

            if (order->state() == OrderState::WaitingForAcceptance)
            {
                text = std::to_string(line.requiredAmount())
                       + "x "
                       + line.definition()->name();
            }
            else
            {
                text = line.definition()->name()
                       + " Scan: "
                       + std::to_string(line.scannedAmount())
                       + "/"
                       + std::to_string(line.requiredAmount());
            }

            drawCenteredText(text,
                             currentY,
                             20.0f * scale,
                             framebufferWidth,
                             framebufferHeight,
                             Color(1.0f, 1.0f, 1.0f),
                             camera);

            currentY += 32.0f * scale;
        }
    }

    if (terminalUI.canAcceptOrder())
    {
        drawCenteredText("[E/ENTER] Auftrag annehmen",
                         static_cast<float>(framebufferHeight) - 105.0f * scale,
                         19.0f * scale,
                         framebufferWidth,
                         framebufferHeight,
                         Color(0.35f, 1.0f, 0.35f),
                         camera);
    }
    else
    {
        drawCenteredText("Auftrag angenommen",
                         static_cast<float>(framebufferHeight) - 105.0f * scale,
                         19.0f * scale,
                         framebufferWidth,
                         framebufferHeight,
                         Color(0.35f, 1.0f, 0.35f),
                         camera);
    }

    drawCenteredText("[ESC] Schliessen",
                     static_cast<float>(framebufferHeight) - 70.0f * scale,
                     18.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(1.0f, 1.0f, 1.0f),
                     camera);

    if (depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }

    if (cullFaceEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}

void HUDRenderer::drawPauseMenu(const BaseCamera& camera,
                                const PauseMenu& pauseMenu)
{
    if (m_window == nullptr || !pauseMenu.isOpen())
    {
        return;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;

    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);

    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        return;
    }

    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    float scale = uiScale(framebufferWidth, framebufferHeight);

    drawCenteredText("PAUSE",
                     120.0f * scale,
                     34.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(1.0f, 1.0f, 1.0f),
                     camera);

    drawCenteredText("==============================",
                     165.0f * scale,
                     18.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(0.65f, 0.65f, 0.65f),
                     camera);

    const char* menuItems[] = {
        "Weiterspielen",
        "Spiel neu starten",
        "Beenden"};

    float currentY = 225.0f * scale;

    for (std::size_t index = 0; index < 3; ++index)
    {
        bool selected = pauseMenu.selectedIndex() == index;

        std::string text = selected ? "> " : "  ";

        text += menuItems[index];

        Color color =
            selected
                ? Color(1.0f, 0.85f, 0.25f)
                : Color(1.0f, 1.0f, 1.0f);

        drawCenteredText(text,
                         currentY,
                         23.0f * scale,
                         framebufferWidth,
                         framebufferHeight,
                         color,
                         camera);

        currentY += 45.0f * scale;
    }

    drawCenteredText("W/S oder Pfeile - Auswahl",
                     static_cast<float>(framebufferHeight) - 105.0f * scale,
                     17.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(0.85f, 0.85f, 0.85f),
                     camera);

    drawCenteredText("[E/ENTER] Bestaetigen | [ESC] Zurueck",
                     static_cast<float>(framebufferHeight) - 70.0f * scale,
                     17.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     Color(0.85f, 0.85f, 0.85f),
                     camera);

    if (depthTestEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }

    if (cullFaceEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
}

// Die kleinere Achsenskalierung verhindert Abschneiden bei stark abweichenden Seitenverhaeltnissen
float HUDRenderer::uiScale(int framebufferWidth,
                           int framebufferHeight) const
{
    float widthScale =
        static_cast<float>(framebufferWidth) / 800.0f;

    float heightScale =
        static_cast<float>(framebufferHeight) / 600.0f;

    float scale =
        std::min(widthScale, heightScale);

    return std::max(0.75f, std::min(scale, 1.5f));
}

void HUDRenderer::drawTextWithShadow(const std::string& text,
                                     float x,
                                     float y,
                                     float characterHeight,
                                     int framebufferWidth,
                                     int framebufferHeight,
                                     const Color& color,
                                     const BaseCamera& camera)
{
    if (text.empty())
    {
        return;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float shadowOffset =
        2.0f * scale;

    m_fontRenderer.drawText(text,
                            x + shadowOffset,
                            y + shadowOffset,
                            characterHeight,
                            framebufferWidth,
                            framebufferHeight,
                            Color(0.0f, 0.0f, 0.0f),
                            camera);

    m_fontRenderer.drawText(text,
                            x,
                            y,
                            characterHeight,
                            framebufferWidth,
                            framebufferHeight,
                            color,
                            camera);
}

void HUDRenderer::drawCenteredText(const std::string& text,
                                   float y,
                                   float characterHeight,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const Color& color,
                                   const BaseCamera& camera)
{
    float textWidth =
        m_fontRenderer.measureTextWidth(text, characterHeight);

    float x =
        (static_cast<float>(framebufferWidth) - textWidth) * 0.5f;

    drawTextWithShadow(text,
                       x,
                       y,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       color,
                       camera);
}

// Zeigt nur den fuer den Spieler relevanten Packstatus; die eigentliche Orderlogik bleibt im Order-System
void HUDRenderer::drawOrder(int framebufferWidth,
                            int framebufferHeight,
                            const BaseCamera& camera,
                            const HUDModel& hudModel)
{
    if (!hudModel.hasActiveOrder())
    {
        return;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float startX =
        20.0f * scale;

    float currentY =
        20.0f * scale;

    drawTextWithShadow("AUFTRAG " + std::to_string(hudModel.orderId()),
                       startX,
                       currentY,
                       21.0f * scale,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 1.0f, 1.0f),
                       camera);

    currentY += 30.0f * scale;

    for (const HUDOrderLine& line : hudModel.orderLines())
    {
        Color lineColor(1.0f, 1.0f, 1.0f);

        if (line.packedAmount == line.requiredAmount
            && line.requiredAmount > 0)
        {
            lineColor =
                Color(0.3f, 1.0f, 0.3f);
        }
        else if (line.scannedOutsideAmount > 0)
        {
            lineColor =
                Color(1.0f, 0.85f, 0.25f);
        }

        drawTextWithShadow(line.productName
                               + " ("
                               + std::to_string(line.requiredAmount)
                               + ")",
                           startX,
                           currentY,
                           17.0f * scale,
                           framebufferWidth,
                           framebufferHeight,
                           lineColor,
                           camera);

        currentY += 21.0f * scale;

        std::string statusText =
            " Offen: "
            + std::to_string(line.notScannedAmount)
            + " Scan: "
            + std::to_string(line.scannedOutsideAmount)
            + " Box: "
            + std::to_string(line.packedAmount);

        drawTextWithShadow(statusText,
                           startX,
                           currentY,
                           15.0f * scale,
                           framebufferWidth,
                           framebufferHeight,
                           lineColor,
                           camera);

        currentY += 27.0f * scale;
    }

    for (const std::string& warning : hudModel.boxWarnings())
    {
        drawTextWithShadow(warning,
                           startX,
                           currentY,
                           15.0f * scale,
                           framebufferWidth,
                           framebufferHeight,
                           Color(1.0f, 0.3f, 0.2f),
                           camera);

        currentY += 22.0f * scale;
    }

    if (hudModel.isOrderFullyPacked())
    {
        drawTextWithShadow("Auftrag vollstaendig verpackt",
                           startX,
                           currentY,
                           16.0f * scale,
                           framebufferWidth,
                           framebufferHeight,
                           Color(0.3f, 1.0f, 0.3f),
                           camera);
    }
}

void HUDRenderer::drawObjective(int framebufferWidth,
                                int framebufferHeight,
                                const BaseCamera& camera,
                                const HUDModel& hudModel)
{
    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float margin =
        20.0f * scale;

    float characterHeight =
        15.0f * scale;

    float currentY =
        20.0f * scale;

    if (!hudModel.objectiveText().empty())
    {
        const std::string title = "AUFGABE";

        float titleWidth =
            m_fontRenderer.measureTextWidth(
                title,
                17.0f * scale);

        float titleX =
            static_cast<float>(framebufferWidth)
            - titleWidth
            - margin;

        drawTextWithShadow(title,
                           titleX,
                           currentY,
                           17.0f * scale,
                           framebufferWidth,
                           framebufferHeight,
                           Color(1.0f, 0.85f, 0.25f),
                           camera);

        currentY += 25.0f * scale;

        float textWidth =
            m_fontRenderer.measureTextWidth(
                hudModel.objectiveText(),
                characterHeight);

        float x =
            static_cast<float>(framebufferWidth)
            - textWidth
            - margin;

        drawTextWithShadow(hudModel.objectiveText(),
                           x,
                           currentY,
                           characterHeight,
                           framebufferWidth,
                           framebufferHeight,
                           Color(1.0f, 1.0f, 1.0f),
                           camera);

        currentY += 45.0f * scale;
    }

    if (hudModel.eventText().empty())
    {
        return;
    }

    const std::string title = "EREIGNIS";

    float titleWidth =
        m_fontRenderer.measureTextWidth(
            title,
            17.0f * scale);

    float titleX =
        static_cast<float>(framebufferWidth)
        - titleWidth
        - margin;

    drawTextWithShadow(title,
                       titleX,
                       currentY,
                       17.0f * scale,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 0.5f, 0.2f),
                       camera);

    currentY += 25.0f * scale;

    float textWidth =
        m_fontRenderer.measureTextWidth(
            hudModel.eventText(),
            characterHeight);

    float x =
        static_cast<float>(framebufferWidth)
        - textWidth
        - margin;

    drawTextWithShadow(hudModel.eventText(),
                       x,
                       currentY,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 1.0f, 1.0f),
                       camera);
}

void HUDRenderer::drawContextPrompt(int framebufferWidth,
                                    int framebufferHeight,
                                    const BaseCamera& camera,
                                    const HUDModel& hudModel)
{
    if (hudModel.contextPrompt().empty())
    {
        return;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float characterHeight =
        19.0f * scale;

    float textWidth =
        m_fontRenderer.measureTextWidth(
            hudModel.contextPrompt(),
            characterHeight);

    float x =
        (static_cast<float>(framebufferWidth) - textWidth) * 0.5f;

    float y =
        static_cast<float>(framebufferHeight) * 0.5f
        + 38.0f * scale;

    drawTextWithShadow(hudModel.contextPrompt(),
                       x,
                       y,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 1.0f, 1.0f),
                       camera);
}

void HUDRenderer::drawCarryHint(int framebufferWidth,
                                int framebufferHeight,
                                const BaseCamera& camera,
                                const HUDModel& hudModel)
{
    if (hudModel.carryHint().empty())
    {
        return;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float characterHeight =
        16.0f * scale;

    float margin =
        20.0f * scale;

    float textWidth =
        m_fontRenderer.measureTextWidth(
            hudModel.carryHint(),
            characterHeight);

    float x =
        static_cast<float>(framebufferWidth)
        - textWidth
        - margin;

    float y =
        static_cast<float>(framebufferHeight)
        - characterHeight
        - margin;

    drawTextWithShadow(hudModel.carryHint(),
                       x,
                       y,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 1.0f, 1.0f),
                       camera);
}

void HUDRenderer::drawScanHint(int framebufferWidth,
                               int framebufferHeight,
                               const BaseCamera& camera,
                               const HUDModel& hudModel)
{
    if (hudModel.scanHint().empty())
    {
        return;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    float characterHeight =
        15.0f * scale;

    float margin =
        20.0f * scale;

    float textWidth =
        m_fontRenderer.measureTextWidth(
            hudModel.scanHint(),
            characterHeight);

    float x =
        static_cast<float>(framebufferWidth)
        - textWidth
        - margin;

    float y =
        static_cast<float>(framebufferHeight)
        - 55.0f * scale;

    drawTextWithShadow(hudModel.scanHint(),
                       x,
                       y,
                       characterHeight,
                       framebufferWidth,
                       framebufferHeight,
                       Color(1.0f, 0.85f, 0.25f),
                       camera);
}

// Toast-Typen werden visuell ueber Farbe und Text unterschieden
void HUDRenderer::drawToast(int framebufferWidth,
                            int framebufferHeight,
                            const BaseCamera& camera,
                            const ToastManager& toastManager)
{
    if (!toastManager.hasCurrentToast())
    {
        return;
    }

    const ToastMessage& toast =
        toastManager.currentToast();

    Color color(1.0f, 1.0f, 1.0f);

    switch (toast.type)
    {
        case ToastType::Info:
            color =
                Color(1.0f, 1.0f, 1.0f);
            break;

        case ToastType::Success:
            color =
                Color(0.3f, 1.0f, 0.3f);
            break;

        case ToastType::Warning:
            color =
                Color(1.0f, 0.8f, 0.2f);
            break;

        case ToastType::Error:
            color =
                Color(1.0f, 0.25f, 0.2f);
            break;
    }

    float scale =
        uiScale(framebufferWidth, framebufferHeight);

    drawCenteredText(toast.text,
                     static_cast<float>(framebufferHeight)
                         - 100.0f * scale,
                     18.0f * scale,
                     framebufferWidth,
                     framebufferHeight,
                     color,
                     camera);
}

// Legende fuer die Farben der Debug-Collider und Interaktionsbereiche
void HUDRenderer::drawDebugOverlay(int framebufferWidth,
                                   int framebufferHeight,
                                   const BaseCamera& camera)
{
    if (!m_debugOverlayVisible)
    {
        return;
    }

    struct DebugLegendLine
    {
        const char* text;
        Color color;
    };

    const DebugLegendLine legendLines[] = {
        {"Rot: Spieler und Spieler-Blocker", Color(1.0f, 0.1f, 0.1f)},
        {"Gelb: Interaktions-Blocker", Color(1.0f, 0.8f, 0.0f)},
        {"Blau: statische und Karton-Physik", Color(0.1f, 0.35f, 1.0f)},
        {"Cyan: Produkt-Physik", Color(0.0f, 1.0f, 1.0f)},
        {"Orange: aktive Interaktion", Color(1.0f, 0.45f, 0.0f)},
        {"Grau: inaktive Interaktion", Color(0.45f, 0.45f, 0.45f)},
        {"Weiss: aktueller Fokus", Color(1.0f, 1.0f, 1.0f)},
        {"Gruen: Scannerzone", Color(0.0f, 1.0f, 0.2f)},
        {"Hellgruen: Lieferzone", Color(0.6f, 1.0f, 0.0f)},
        {"Magenta: Karton-Innenraum", Color(1.0f, 0.0f, 1.0f)},
        {"Pink: Deckel-Freiraum", Color(1.0f, 0.25f, 0.65f)}};

    const std::string title = "DEBUG F3 - Hitboxen ausblenden";

    const float characterHeight = 17.0f;

    const float lineHeight = 21.0f;

    const float margin = 20.0f;

    float maximumTextWidth = m_fontRenderer.measureTextWidth(title, characterHeight);

    for (const DebugLegendLine& line : legendLines)
    {
        float textWidth = m_fontRenderer.measureTextWidth(line.text, characterHeight);

        if (textWidth > maximumTextWidth)
        {
            maximumTextWidth = textWidth;
        }
    }

    float x = static_cast<float>(framebufferWidth) - maximumTextWidth - margin;

    float y = margin;

    m_fontRenderer.drawText(title,
                            x,
                            y,
                            characterHeight,
                            framebufferWidth,
                            framebufferHeight,
                            Color(1.0f, 1.0f, 1.0f),
                            camera);

    y += lineHeight * 1.4f;

    for (const DebugLegendLine& line : legendLines)
    {
        m_fontRenderer.drawText(line.text,
                                x,
                                y,
                                characterHeight,
                                framebufferWidth,
                                framebufferHeight,
                                line.color,
                                camera);

        y += lineHeight;
    }
}
