#include "Application.h"

#ifdef WIN32
#include <GL/glew.h>
#include <glfw/glfw3.h>
#else
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <glfw/glfw3.h>
#endif

#include "../engine/ShaderLightMapper.h"

#include "../game/delivery/DeliveryBox.h"
#include "../game/orders/OrderCatalog.h"
#include "../game/products/Product.h"
#include "../game/scene/StoreLayout.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace
{
    constexpr float kVolumeStep = 0.05f;

    constexpr float kPi = 3.14159265358979323846f;

    constexpr float kMenuCameraRadius = 7.5f;

    constexpr float kMenuCameraHeight = 3.2f;

    constexpr float kMenuCameraTargetHeight = 1.25f;

    constexpr float kMenuCameraRotationSpeed = 0.10f;

    constexpr float kMenuCameraFov = 55.0f;
}

Application::Application(GLFWwindow* window)
    : m_window(window),
      m_assetLoader(),
      m_store(m_assetLoader),
      m_camera(window),
      m_menuCamera(),
      m_player(window, m_camera),
      m_interaction(3.0f),
      m_carry(),
      m_highlighter(),
      m_hud(window,
            m_assetLoader.assetPath("fonts/font_atlas.png").c_str(),
            m_assetLoader.assetPath("shaders/font/vsfont.glsl").c_str(),
            m_assetLoader.assetPath("shaders/font/fsfont.glsl").c_str()),
      m_menuRenderer(window,
                     m_assetLoader.assetPath("fonts/font_atlas.png").c_str(),
                     m_assetLoader.assetPath("shaders/font/vsfont.glsl").c_str(),
                     m_assetLoader.assetPath("shaders/font/fsfont.glsl").c_str()),
      m_hudModel(),
      m_toastManager(),
      m_orderManager(),
      m_terminalUI(m_orderManager),
      m_pauseMenu(),
      m_gameState(GameState::MainMenu),
      m_mainMenu(),
      m_menuCameraAngle(-0.6f),
      m_audio(),
      m_physics(),
      m_cleaningToolPhysicsId(0),
      m_deliveryBoxPhysicsController(m_physics, m_store),
      m_deliveryController(m_store,
                           m_orderManager,
                           m_physics,
                           m_deliveryBoxPhysicsController,
                           m_interaction,
                           m_carry,
                           m_toastManager),
      m_debugRenderer(window),
      m_models(),
      m_terminalInteraction(nullptr),
      m_interactionController(nullptr),
      m_scannerController(nullptr),
      m_sceneCreated(false)
{
    if (!m_audio.isReady())
    {
        std::cout << "Audio-System konnte nicht "
                  << "initialisiert werden" << std::endl;
    }
}

void Application::prepare()
{
    initializeGameplay();

    updateMenuCamera(0.0f);
}

void Application::start()
{
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);

    glCullFace(GL_BACK);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.02f, 0.02f, 0.025f, 1.0f);

    m_mainMenu.reset();

    m_pauseMenu.close();

    m_gameState = GameState::MainMenu;

    updateMenuCamera(0.0f);

    if (!m_audio.isMusicPlaying())
    {
        m_audio.startMusic(m_assetLoader.preferredAssetPath("local/sounds/music/music.mp3",
                                                            "sounds/music/music.wav"),
                           1.9f,
                           1.2f);
    }
}

void Application::update(float deltaTime)
{
    // Je nach oberstem Spielzustand wird nur der passende Eingabemodus aktualisiert, die Modi laufen nie parallel
    m_audio.update();

    bool gameplayInputEnabled = m_gameState == GameState::Playing && !m_terminalUI.isOpen();

    m_player.setGameplayInputEnabled(gameplayInputEnabled);

    m_player.setObjectRotationAllowed(m_carry.canRotateHeldObject());

    m_player.update(deltaTime);

    switch (m_gameState)
    {
        case GameState::MainMenu:
            updateMainMenu(deltaTime);
            return;

        case GameState::Paused:
            m_camera.update();

            if (m_player.escapePressed())
            {
                m_pauseMenu.close();

                m_gameState = GameState::Playing;

                return;
            }

            handlePauseMenu();

            return;

        case GameState::Playing:
            updateGameplay(deltaTime);

            return;

        case GameState::Intro:
            return;
    }
}

void Application::draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_gameState == GameState::MainMenu)
    {
        ShaderLightMapper::instance().activate();

        for (BaseModel* model : m_models)
        {
            if (model != nullptr)
            {
                model->draw(m_menuCamera);
            }
        }

        ShaderLightMapper::instance().deactivate();

        m_menuRenderer.drawMainMenu(m_mainMenu.selectedItem(),
                                    m_audio.masterVolume(),
                                    m_menuCamera);

#ifndef NDEBUG

        GLenum error = glGetError();

        assert(error == GL_NO_ERROR);

#endif

        return;
    }

    // Zuerst wird immer die 3D-Szene gezeichnet, HUD und Menüs kommen danach ohne Szenenbeleuchtung darüber
    ShaderLightMapper::instance().activate();

    for (BaseModel* model : m_models)
    {
        if (model != nullptr)
        {
            model->draw(m_camera);
        }
    }

    Product* heldProduct = dynamic_cast<Product*>(m_carry.heldObject());

    m_debugRenderer.draw(m_camera, m_player, m_interaction, heldProduct, m_store);

    ShaderLightMapper::instance().deactivate();

    if (m_gameState == GameState::Paused)
    {
        m_menuRenderer.drawPauseMenu(m_pauseMenu, m_audio.masterVolume(), m_camera);
    }
    else if (m_terminalUI.isOpen())
    {
        m_hud.drawTerminal(m_camera, m_terminalUI);
    }
    else
    {
        m_highlighter.draw(m_camera);

        m_hud.drawGameplay(m_camera, m_hudModel, m_toastManager);
    }

#ifndef NDEBUG

    GLenum error = glGetError();

    assert(error == GL_NO_ERROR);

#endif
}

void Application::end()
{
    for (BaseModel* model : m_models)
    {
        delete model;
    }

    m_models.clear();
}

void Application::initializeGameplay()
{
    if (m_sceneCreated)
    {
        return;
    }

    createScene();

    m_interactionController =
        std::make_unique<InteractionController>(m_interaction, m_carry, m_highlighter);

    m_scannerController =
        std::make_unique<ScannerController>(m_store.scannerStation(),
                                            m_orderManager,
                                            m_audio,
                                            m_toastManager,
                                            m_assetLoader.preferredAssetPath(
                                                "local/sounds/scanner.wav",
                                                "sounds/scanner.wav"));

    m_sceneCreated = true;
}

void Application::updateMainMenu(float deltaTime)
{
    // Das Hauptmenu steuert Auswahl und Lautstaerke, waehrend die 3D-Szene nur als Hintergrund weiterlaeuft
    updateMenuCamera(deltaTime);

    if (m_player.menuUpPressed())
    {
        m_mainMenu.selectPrevious();
    }
    else if (m_player.menuDownPressed())
    {
        m_mainMenu.selectNext();
    }

    if (m_mainMenu.selectedItem() == MainMenuItem::Volume)
    {
        if (m_player.menuLeftPressed())
        {
            adjustMasterVolume(-kVolumeStep);
        }
        else if (m_player.menuRightPressed())
        {
            adjustMasterVolume(kVolumeStep);
        }
    }

    if (!m_player.confirmPressed())
    {
        return;
    }

    switch (m_mainMenu.selectedItem())
    {
        case MainMenuItem::Start:
            m_gameState = GameState::Playing;

            return;

        case MainMenuItem::Volume:
            return;

        case MainMenuItem::Exit:
            if (m_window != nullptr)
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }

            return;
    }
}

void Application::updateGameplay(float deltaTime)
{
    m_debugRenderer.updateInput();

    m_hud.setDebugOverlayVisible(m_debugRenderer.isVisible());

    m_camera.update();

    if (m_player.escapePressed())
    {
        if (m_terminalUI.isOpen())
        {
            m_terminalUI.close();

            return;
        }

        m_pauseMenu.open();

        m_gameState = GameState::Paused;

        return;
    }

    if (m_terminalUI.isOpen())
    {
        m_terminalUI.update(m_player.confirmPressed());

        return;
    }

    m_toastManager.update(deltaTime);

    // Das HUDModel wird pro Frame neu aufgebaut, damit keine alten Hinweise stehen bleiben
    m_hudModel.beginFrame();

    // Während der Mop auf einem Spill steht, blockiert Wischen die normale E-Interaktion
    bool canCleanSpill = m_store.canCleanSpill();

    if (m_interactionController != nullptr)
    {
        m_interactionController->update(m_camera, m_player, deltaTime, canCleanSpill);

        handleCarryAction(m_interactionController->consumeCarryAction());
    }

    if (m_terminalInteraction != nullptr && m_terminalInteraction->consumeOpenRequested())
    {
        m_terminalUI.open();

        return;
    }

    canCleanSpill = m_store.canCleanSpill();

    if (canCleanSpill && m_player.interactHeld())
    {
        bool cleaningFinished = m_store.cleanSpill(deltaTime);

        if (cleaningFinished)
        {
            m_toastManager.push("Wieder sauber!", ToastType::Success, 2.5f);
        }
    }

    // Kinematische/tragbare Objekte werden vor dem Physikschritt auf ihren Gameplay-Transform gesetzt
    m_deliveryBoxPhysicsController.updateBeforePhysics();

    Product* heldProduct = dynamic_cast<Product*>(m_carry.heldObject());

    if (m_scannerController != nullptr)
    {
        m_scannerController->update(heldProduct, deltaTime);
    }

    m_physics.updateCarriedProduct(heldProduct);

    if (m_cleaningToolPhysicsId != 0)
    {
        m_physics.updateBodyTransform(m_cleaningToolPhysicsId, m_store.cleaningToolTransform());
    }

    m_physics.update(deltaTime);

    // Dynamische Ergebnisse werden nach der Simulation wieder in die Gameplay-Objekte zurückgeschrieben
    m_deliveryBoxPhysicsController.updateAfterPhysics();

    m_deliveryController.updateBeforeScene(m_player, deltaTime);

    m_store.update(deltaTime, m_models);

    if (m_store.consumeSpillStartedEvent())
    {
        m_toastManager.push("[!] Missgeschick in einem Gang", ToastType::Warning, 4.0f);
    }

    if (m_store.consumeShelfRefillCompletedEvent())
    {
        m_toastManager.push("Regal wurde nachgefuellt", ToastType::Info, 2.0f);
    }

    m_deliveryController.updateAfterScene();

    handlePrimaryAction();

    // Neu nachgefüllte Produkte entstehen während des Scene-Updates und werden erst danach registriert
    for (IInteractable* interactable : m_store.pendingInteractables())
    {
        m_interaction.addInteractable(interactable);
    }

    m_store.clearPendingInteractables();

    // Die Modellzuordnung wird neu aufgebaut, weil Produkte beim Nachfüllen oder Liefern entstehen/verschwinden können
    m_highlighter.clearModels();

    m_store.registerHighlightModels(m_highlighter);

    if (m_terminalInteraction != nullptr)
    {
        m_highlighter.registerModel(m_terminalInteraction.get(), m_store.terminalVisual());
    }

    updateHUDModel();
}

void Application::updateMenuCamera(float deltaTime)
{
    m_menuCameraAngle += deltaTime * kMenuCameraRotationSpeed;

    if (m_menuCameraAngle >= kPi * 2.0f)
    {
        m_menuCameraAngle -= kPi * 2.0f;
    }

    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(m_window, &width, &height);

    if (width <= 0 || height <= 0)
    {
        return;
    }

    glViewport(0, 0, width, height);

    float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // Die Menükamera läuft auf einer Kreisbahn um den Laden und dient nur als animierter Hintergrund
    Vector position(std::cos(m_menuCameraAngle) * kMenuCameraRadius,
                    kMenuCameraHeight,
                    std::sin(m_menuCameraAngle) * kMenuCameraRadius);

    Vector target(0.0f, kMenuCameraTargetHeight, 0.0f);

    Matrix view;

    view.lookAt(target, Vector(0.0f, 1.0f, 0.0f), position);

    Matrix projection;

    projection.perspective(kPi * kMenuCameraFov / 180.0f, aspectRatio, 0.1f, 100.0f);

    m_menuCamera.setViewMatrix(view);

    m_menuCamera.setProjectionMatrix(projection);
}

void Application::handleCarryAction(const CarryAction& carryAction)
{
    if (carryAction.object == nullptr || carryAction.type == CarryActionType::None)
    {
        return;
    }

    Product* product = dynamic_cast<Product*>(carryAction.object);

    if (carryAction.type == CarryActionType::PickedUp)
    {
        if (product != nullptr)
        {
            m_physics.startCarrying(product);
        }
    }
    else if (carryAction.type == CarryActionType::Dropped)
    {
        if (product != nullptr)
        {
            m_physics.releaseProduct(product);
        }
    }

    m_deliveryBoxPhysicsController.handleCarryAction(carryAction);
}

void Application::handlePauseMenu()
{
    // Menueingaben werden zuerst in eine PauseMenuAction uebersetzt und erst hier auf den GameState angewendet
    m_pauseMenu.update(m_player.menuUpPressed(),
                       m_player.menuDownPressed(),
                       m_player.confirmPressed());

    if (m_pauseMenu.isVolumeSelected())
    {
        if (m_player.menuLeftPressed())
        {
            adjustMasterVolume(-kVolumeStep);
        }
        else if (m_player.menuRightPressed())
        {
            adjustMasterVolume(kVolumeStep);
        }
    }

    PauseMenuAction action = m_pauseMenu.consumeAction();

    switch (action)
    {
        case PauseMenuAction::None:
            return;

        case PauseMenuAction::Resume:
            m_pauseMenu.close();

            m_gameState = GameState::Playing;

            return;

        case PauseMenuAction::MainMenu:
            m_pauseMenu.close();

            m_mainMenu.reset();

            m_gameState = GameState::MainMenu;

            updateMenuCamera(0.0f);

            return;

        case PauseMenuAction::Quit:
            if (m_window != nullptr)
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }

            return;
    }
}

void Application::createScene()
{
    m_interaction.clear();

    m_store.create(m_models);

    m_camera.setPosition(StoreLayout::playerStartPosition());

    m_camera.update();

    // Die unbeweglichen Scene-Collider werden einmalig als statische Physikkörper angelegt
    for (const AABB& physicsCollider : m_store.physicsColliders())
    {
        m_physics.addStaticBox(physicsCollider.center(), physicsCollider.size());
    }

    if (m_store.hasCleaningTool())
    {
        m_cleaningToolPhysicsId = m_physics.addKinematicBox(m_store.cleaningToolTransform(),
                                                            m_store.cleaningToolPhysicsSize());
    }

    m_deliveryBoxPhysicsController.resetToOpenBodies();

    m_carry.setBlockingColliders(m_store.physicsColliders());

    m_carry.setDynamicBlockingColliders(m_store.carryBlockingColliders());

    // Aufträge werden erst nach dem Produktkatalog erzeugt, damit jede Order auf gültige Produktdefinitionen zeigt
    OrderCatalog orderCatalog(m_store.productCatalog());

    m_orderManager.setOrders(orderCatalog.orders());

    m_orderManager.prepareNextOrder();

    m_deliveryController.reset();

    m_terminalInteraction =
        std::make_unique<OrderTerminal>(m_store.terminalBounds(), m_orderManager);

    m_interaction.addInteractable(m_terminalInteraction.get());

    for (IInteractable* interactable : m_store.interactables())
    {
        m_interaction.addInteractable(interactable);
    }

    m_player.setStaticColliders(&m_store.staticColliders());

    m_player.setDynamicColliders(&m_store.playerDynamicColliders());

    m_interaction.setBlockingColliders(m_store.interactionColliders());
}

void Application::handlePrimaryAction()
{
    if (!m_player.primaryActionPressed())
    {
        return;
    }

    if (m_deliveryController.requestDelivery(m_camera, m_models))
    {
        return;
    }

    if (m_scannerController == nullptr)
    {
        return;
    }

    Product* heldProduct = dynamic_cast<Product*>(m_carry.heldObject());

    m_scannerController->requestScan(heldProduct);
}

void Application::adjustMasterVolume(float amount)
{
    m_audio.setMasterVolume(m_audio.masterVolume() + amount);
}

void Application::updateHUDModel()
{
    m_hudModel.updateOrder(m_orderManager.activeOrder(), m_deliveryController.packingResult());

    if (m_interactionController != nullptr)
    {
        m_hudModel.setCarryHint(m_interactionController->carryHint());
    }

    if (m_scannerController != nullptr)
    {
        m_hudModel.setScanHint(m_scannerController->hintText());
    }

    if (m_store.isSpillActive())
    {
        m_hudModel.setEventText("Missgeschick in einem Gang");
    }

    if (m_orderManager.hasWaitingOrder())
    {
        m_hudModel.setObjectiveText("Auftrag am Terminal annehmen");
    }
    else if (m_orderManager.hasActiveOrder())
    {
        DeliveryBoxState boxState = m_store.deliveryBox().state();

        if (boxState == DeliveryBoxState::Sealed)
        {
            if (m_deliveryController.canDeliverCurrentBox(m_camera))
            {
                m_hudModel.setObjectiveText("Lieferung abgeben");
            }
            else
            {
                m_hudModel.setObjectiveText("Karton zur Lieferzone bringen");
            }
        }
        else if (boxState == DeliveryBoxState::Closing)
        {
            m_hudModel.setObjectiveText("Karton wird geschlossen");
        }
        else if (boxState == DeliveryBoxState::Blocked)
        {
            m_hudModel.setObjectiveText("Karton pruefen und erneut schliessen");
        }
        else if (m_deliveryController.packingResult().isComplete)
        {
            m_hudModel.setObjectiveText("Karton schliessen");
        }
        else
        {
            m_hudModel.setObjectiveText("Produkte scannen und verpacken");
        }
    }

    // Kontextaktionen haben eine feste Priorität, damit sich Scan-, Wisch- und Interaktionshinweise nicht überdecken
    if (m_deliveryController.canDeliverCurrentBox(m_camera))
    {
        m_hudModel.setContextPrompt("[LMB] Lieferung abgeben");

        return;
    }

    if (m_store.canCleanSpill())
    {
        m_hudModel.setContextPrompt("[E halten] Wischen");

        m_hudModel.setCarryHint("");

        return;
    }

    if (m_scannerController != nullptr && m_scannerController->isReady())
    {
        m_hudModel.setContextPrompt("[LMB] Produkt scannen");

        return;
    }

    if (m_interactionController != nullptr)
    {
        m_hudModel.setContextPrompt(m_interactionController->interactionPrompt());
    }
}
