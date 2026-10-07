#pragma once

#include "../../engine/BaseShader.h"

// Einfarbiger Shader für die vergrößerte Outline eines fokussierten Modells
class OutlineShader : public BaseShader
{
public:
    OutlineShader();

    void activate(const BaseCamera& camera) const override;

private:
    GLint m_modelViewProjectionLocation;

    GLint m_outlineColorLocation;

    GLint m_outlineScaleLocation;
};
