#include "OutlineShader.h"

namespace
{
#ifdef WIN32
    const char* kVertexShaderPath = "../../assets/shaders/highlight/"
                                    "vshighlight.glsl";

    const char* kFragmentShaderPath = "../../assets/shaders/highlight/"
                                      "fshighlight.glsl";
#else
    const char* kVertexShaderPath = "../assets/shaders/highlight/"
                                    "vshighlight.glsl";

    const char* kFragmentShaderPath = "../assets/shaders/highlight/"
                                      "fshighlight.glsl";
#endif

    const Color kOutlineColor(1.0f, 0.8f, 0.1f);

    constexpr float kOutlineScale = 1.015f;
}

OutlineShader::OutlineShader()
    : m_modelViewProjectionLocation(-1),
      m_outlineColorLocation(-1),
      m_outlineScaleLocation(-1)
{
    if (!load(kVertexShaderPath, kFragmentShaderPath))
    {
        throw std::exception();
    }

    m_modelViewProjectionLocation = getParameterID("ModelViewProjMat");

    m_outlineColorLocation = getParameterID("OutlineColor");

    m_outlineScaleLocation = getParameterID("OutlineScale");
}

void OutlineShader::activate(const BaseCamera& camera) const
{
    BaseShader::activate(camera);

    Matrix modelView = camera.getViewMatrix() * ModelTransform;

    Matrix modelViewProjection = camera.getProjectionMatrix() * modelView;

    setParameter(m_modelViewProjectionLocation, modelViewProjection);

    setParameter(m_outlineColorLocation, kOutlineColor);

    setParameter(m_outlineScaleLocation, kOutlineScale);
}
