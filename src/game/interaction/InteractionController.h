#pragma once

#include <string>

class CarrySystem;
class FirstPersonCamera;
class HighlightRenderer;
class ICarryable;
class InteractionSystem;
class PlayerController;

// Beschreibt, ob in diesem Frame ein Objekt aufgenommen oder abgelegt wurde
enum class CarryActionType
{
    None,
    PickedUp,
    Dropped
};

// Übergibt Carry-Änderungen an Systeme, die darauf reagieren müssen
struct CarryAction
{
    CarryActionType type = CarryActionType::None;

    ICarryable* object = nullptr;
};

// Verbindet Eingabe, Interaktionsfokus, Highlighting und Carry-Aktionen
class InteractionController
{
public:
    InteractionController(InteractionSystem& interactionSystem,
                          CarrySystem& carrySystem,
                          HighlightRenderer& highlighter);

    void update(const FirstPersonCamera& camera, const PlayerController& player, float deltaTime, bool blockInteraction);

    CarryAction consumeCarryAction();
    
    const std::string& interactionPrompt() const;

    const std::string& carryHint() const;

private:
    void handleInteraction(const FirstPersonCamera& camera, const PlayerController& player);

    void updateInterface();

    InteractionSystem& m_interactionSystem;
    CarrySystem& m_carrySystem;
    HighlightRenderer& m_highlighter;

    CarryAction m_pendingCarryAction;
    
    std::string m_interactionPrompt;

    std::string m_carryHint;
};
