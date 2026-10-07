#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif

#include <stdio.h>

#include "Application.h"
#include "freeimage.h"

#include "../assets/AssetLoader.h"

#include "../game/audio/AudioPlayer.h"
#include "../game/player/FirstPersonCamera.h"
#include "../game/rendering/MenuRenderer.h"
#include "../game/ui/GameIntro.h"

#include <memory>
#include <cstdlib>
#include <vector>

void PrintOpenGLVersion();
void CaptureFramebufferIfRequested(GLFWwindow* window);

int main()
{
    FreeImage_Initialise();

    if (!glfwInit())
    {
        fprintf(stderr, "ERROR: could not start GLFW3\n");

        return 1;
    }

#ifdef __APPLE__

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#endif

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();

    if (primaryMonitor == nullptr)
    {
        fprintf(stderr, "ERROR: no primary monitor found\n");

        glfwTerminate();

        FreeImage_DeInitialise();

        return 1;
    }

    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

    if (videoMode == nullptr)
    {
        fprintf(stderr, "ERROR: no video mode found\n");

        glfwTerminate();

        FreeImage_DeInitialise();

        return 1;
    }

    glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);

    glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);

    glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);

    glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);

    // Das Spiel startet im nativen Vollbildmodus des primären Monitors
    GLFWwindow* window = glfwCreateWindow(videoMode->width,
                                          videoMode->height,
                                          "Counter Duty: Closing Shift",
                                          primaryMonitor,
                                          nullptr);

    if (window == nullptr)
    {
        fprintf(stderr, "ERROR: can not open window with GLFW3\n");

        glfwTerminate();

        FreeImage_DeInitialise();

        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

#if WIN32

    glewExperimental = GL_TRUE;

    glewInit();

#endif

    PrintOpenGLVersion();

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    std::unique_ptr<Application> application;

    {
        AssetLoader introAssets;

        FirstPersonCamera introCamera(window);

        MenuRenderer introRenderer(window,
                                   introAssets.assetPath("fonts/font_atlas.png").c_str(),
                                   introAssets.assetPath("shaders/font/vsfont.glsl").c_str(),
                                   introAssets.assetPath("shaders/font/fsfont.glsl").c_str());

        AudioPlayer introAudio;

        GameIntro intro;

        intro.reset();

        double lastTime = glfwGetTime();

        while (!glfwWindowShouldClose(window) && !intro.isFinished())
        {
            double now = glfwGetTime();

            float deltaTime = static_cast<float>(now - lastTime);

            lastTime = now;

            glfwPollEvents();

            intro.update(deltaTime);

            if (intro.consumeSoundRequest())
            {
                introAudio.playSound(introAssets.preferredAssetPath("local/sounds/intro.wav",
                                                                    "sounds/intro.wav"));
            }

            introCamera.update();

            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            introRenderer.drawIntro(intro.studioBrightness(), intro.titleBrightness(), introCamera);

            glfwSwapBuffers(window);

             // Das Laden beginnt erst, wenn sowohl KOCH STUDIOS als auch der Spieltitel komplett eingeblendet sind
             // Während Application und Store laden, bleibt dadurch ein fertiger Intro- Bildschirm sichtbar
            if (application == nullptr && intro.canStartLoading())
            {
                application = std::make_unique<Application>(window);

                application->prepare();

                intro.notifyLoadingComplete();

                lastTime = glfwGetTime();
            }
        }
    }

    if (glfwWindowShouldClose(window))
    {
        application.reset();

        glfwTerminate();

        FreeImage_DeInitialise();

        return 0;
    }

    if (application == nullptr)
    {
        application = std::make_unique<Application>(window);

        application->prepare();
    }

    application->start();

    // Klassischer Game-Loop: Events lesen, Spielzustand aktualisieren, Frame zeichnen
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();

        float deltaTime = static_cast<float>(now - lastTime);

        lastTime = now;

        glfwPollEvents();

        application->update(deltaTime);

        application->draw();

        CaptureFramebufferIfRequested(window);

        glfwSwapBuffers(window);
    }

    application->end();

    application.reset();

    glfwTerminate();

    FreeImage_DeInitialise();

    return 0;
}

void PrintOpenGLVersion()
{
    const GLubyte* renderer = glGetString(GL_RENDERER);

    const GLubyte* version = glGetString(GL_VERSION);

    printf("Renderer: %s\n", renderer);

    printf("OpenGL version supported %s\n", version);
}

void CaptureFramebufferIfRequested(GLFWwindow* window)
{
    static bool captureAttempted = false;

    if (captureAttempted)
    {
        return;
    }

    const char* outputPath = std::getenv("COUNTER_DUTY_SCREENSHOT");

    if (outputPath == nullptr || outputPath[0] == '\0')
    {
        return;
    }

    captureAttempted = true;

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    constexpr int channelCount = 3;
    std::vector<BYTE> pixels(static_cast<std::size_t>(width) * height * channelCount);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    FIBITMAP* bitmap = FreeImage_ConvertFromRawBits(pixels.data(),
                                                    width,
                                                    height,
                                                    width * channelCount,
                                                    24,
                                                    FI_RGBA_RED_MASK,
                                                    FI_RGBA_GREEN_MASK,
                                                    FI_RGBA_BLUE_MASK,
                                                    false);

    if (bitmap == nullptr)
    {
        return;
    }

    FreeImage_Save(FIF_PNG, bitmap, outputPath, PNG_Z_BEST_SPEED);
    FreeImage_Unload(bitmap);
}
