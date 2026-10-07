#pragma once

#include "../assets/AssetLoader.h"

#include "../engine/BaseModel.h"
#include "../engine/Camera.h"

#include "../game/audio/AudioPlayer.h"
#include "../game/carry/CarrySystem.h"
#include "../game/debug/DebugRenderer.h"
#include "../game/delivery/DeliveryBoxPhysicsController.h"
#include "../game/delivery/DeliveryController.h"
#include "../game/interaction/InteractionController.h"
#include "../game/interaction/InteractionSystem.h"
#include "../game/orders/OrderManager.h"
#include "../game/orders/OrderTerminal.h"
#include "../game/physics/PhysicsSystem.h"
#include "../game/player/FirstPersonCamera.h"
#include "../game/player/PlayerController.h"
#include "../game/rendering/HighlightRenderer.h"
#include "../game/rendering/HUDRenderer.h"
#include "../game/rendering/MenuRenderer.h"
#include "../game/scanner/ScannerController.h"
#include "../game/scene/StoreScene.h"
#include "../game/ui/GameState.h"
#include "../game/ui/HUDModel.h"
#include "../game/ui/MainMenu.h"
#include "../game/ui/PauseMenu.h"
#include "../game/ui/TerminalUI.h"
#include "../game/ui/ToastManager.h"

#include <list>
#include <memory>

struct GLFWwindow;

// Zentrale Anwendungsklasse, die Gameplay, Physik, Eingabe, Audio und UI pro Frame koordiniert
class Application
{
public:
    using ModelList = std::list<BaseModel*>;

    explicit Application(GLFWwindow* window);

    void prepare();

    void start();

    void update(float deltaTime);

    void draw();

    void end();

protected:
    void createScene();

    void initializeGameplay();

    void updateMainMenu(float deltaTime);

    void updateGameplay(float deltaTime);

    void updateMenuCamera(float deltaTime);

    void handleCarryAction(const CarryAction& carryAction);

    void handlePauseMenu();

    void handlePrimaryAction();

    void adjustMasterVolume(float amount);

    void updateHUDModel();

    GLFWwindow* m_window;

    AssetLoader m_assetLoader;

    StoreScene m_store;

    FirstPersonCamera m_camera;

    SimpleCamera m_menuCamera;

    PlayerController m_player;

    InteractionSystem m_interaction;

    CarrySystem m_carry;

    HighlightRenderer m_highlighter;

    HUDRenderer m_hud;

    MenuRenderer m_menuRenderer;

    HUDModel m_hudModel;

    ToastManager m_toastManager;

    OrderManager m_orderManager;

    TerminalUI m_terminalUI;

    PauseMenu m_pauseMenu;

    GameState m_gameState;

    MainMenu m_mainMenu;

    float m_menuCameraAngle;

    AudioPlayer m_audio;

    PhysicsSystem m_physics;

    PhysicsBodyId m_cleaningToolPhysicsId;

    DeliveryBoxPhysicsController m_deliveryBoxPhysicsController;

    DeliveryController m_deliveryController;

    DebugRenderer m_debugRenderer;

    ModelList m_models;

    std::unique_ptr<OrderTerminal> m_terminalInteraction;

    std::unique_ptr<InteractionController> m_interactionController;

    std::unique_ptr<ScannerController> m_scannerController;

    bool m_sceneCreated;
};
