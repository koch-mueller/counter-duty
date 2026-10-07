#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

class BaseCamera;
class BaseModel;
class IInteractable;
class OutlineShader;

// Ordnet Interaktionsobjekten Modelle zu und rendert das fokussierte Objekt als Outline
class HighlightRenderer
{
public:
    HighlightRenderer();
    ~HighlightRenderer();

    void update(IInteractable* interactable);

    void registerModel(const IInteractable* interactable, BaseModel* model);

    void clearModels();

    void draw(const BaseCamera& camera);

private:
    std::unique_ptr<OutlineShader> m_outlineShader;

    IInteractable* m_interactable;

    std::unordered_map<const IInteractable*, std::vector<BaseModel*>> m_models;
};
