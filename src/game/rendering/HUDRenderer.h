#pragma once

#include "BitmapFontRenderer.h"

#include <string>

struct GLFWwindow;

class BaseCamera;
class HUDModel;
class PauseMenu;
class TerminalUI;
class ToastManager;

// Zeichnet Gameplay-HUD, Terminal und den vorhandenen Pause-Overlay-Pfad
class HUDRenderer
{
public:
    HUDRenderer(GLFWwindow* window,
                const char* fontAtlasPath,
                const char* vertexShaderPath,
                const char* fragmentShaderPath);

    void setDebugOverlayVisible(bool visible);

    void drawGameplay(const BaseCamera& camera,
                      const HUDModel& hudModel,
                      const ToastManager& toastManager);

    void drawTerminal(const BaseCamera& camera, const TerminalUI& terminalUI);

    void drawPauseMenu(const BaseCamera& camera, const PauseMenu& pauseMenu);

private:
    float uiScale(int framebufferWidth, int framebufferHeight) const;

    void drawTextWithShadow(const std::string& text,
                            float x,
                            float y,
                            float characterHeight,
                            int framebufferWidth,
                            int framebufferHeight,
                            const Color& color,
                            const BaseCamera& camera);

    void drawCenteredText(const std::string& text,
                          float y,
                          float characterHeight,
                          int framebufferWidth,
                          int framebufferHeight,
                          const Color& color,
                          const BaseCamera& camera);

    void drawCrosshair(int framebufferWidth,
                       int framebufferHeight,
                       const BaseCamera& camera);

    void drawOrder(int framebufferWidth,
                   int framebufferHeight,
                   const BaseCamera& camera,
                   const HUDModel& hudModel);

    void drawObjective(int framebufferWidth,
                       int framebufferHeight,
                       const BaseCamera& camera,
                       const HUDModel& hudModel);

    void drawContextPrompt(int framebufferWidth,
                           int framebufferHeight,
                           const BaseCamera& camera,
                           const HUDModel& hudModel);

    void drawCarryHint(int framebufferWidth,
                       int framebufferHeight,
                       const BaseCamera& camera,
                       const HUDModel& hudModel);

    void drawScanHint(int framebufferWidth,
                      int framebufferHeight,
                      const BaseCamera& camera,
                      const HUDModel& hudModel);

    void drawToast(int framebufferWidth,
                   int framebufferHeight,
                   const BaseCamera& camera,
                   const ToastManager& toastManager);

    void drawDebugOverlay(int framebufferWidth,
                          int framebufferHeight,
                          const BaseCamera& camera);

    GLFWwindow* m_window;

    BitmapFontRenderer m_fontRenderer;

    bool m_debugOverlayVisible;
};
