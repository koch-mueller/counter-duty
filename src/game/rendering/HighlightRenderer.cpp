#include "HighlightRenderer.h"

#include "OutlineShader.h"

#include "../../engine/BaseModel.h"

HighlightRenderer::HighlightRenderer()
    : m_outlineShader(std::make_unique<OutlineShader>()),
      m_interactable(nullptr)
{
}

HighlightRenderer::~HighlightRenderer() = default;

void HighlightRenderer::update(IInteractable* interactable)
{
    m_interactable = interactable;
}

void HighlightRenderer::registerModel(const IInteractable* interactable, BaseModel* model)
{
    if (interactable == nullptr || model == nullptr)
    {
        return;
    }

    m_models[interactable].push_back(model);
}

void HighlightRenderer::clearModels()
{
    m_models.clear();
}

void HighlightRenderer::draw(const BaseCamera& camera)
{
    if (m_interactable == nullptr || m_outlineShader == nullptr)
    {
        return;
    }

    auto modelEntry = m_models.find(m_interactable);

    if (modelEntry == m_models.end())
    {
        return;
    }

    // Für die Outline werden nur die vergrößerten Rückseiten sichtbar gezeichnet
    // Depth-Writes bleiben aus, damit die Kontur die eigentliche Szene nicht verändert
    GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);

    GLint previousCullFace = GL_BACK;

    glGetIntegerv(GL_CULL_FACE_MODE, &previousCullFace);

    GLboolean previousDepthMask = GL_TRUE;

    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

    glEnable(GL_CULL_FACE);

    glCullFace(GL_FRONT);

    glDepthMask(GL_FALSE);

    for (BaseModel* model : modelEntry->second)
    {
        if (model == nullptr)
        {
            continue;
        }

        model->drawWithShader(camera, m_outlineShader.get());
    }

    glDepthMask(previousDepthMask);

    glCullFace(previousCullFace);

    if (!cullFaceWasEnabled)
    {
        glDisable(GL_CULL_FACE);
    }
}
