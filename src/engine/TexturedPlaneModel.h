//
// TexturedPlaneModel.h
//

#pragma once

#include "BaseModel.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

enum class PlaneOrientation
{
    XY,
    XZ,
    YZ
};

class TexturedPlaneModel : public BaseModel
{
public:
    TexturedPlaneModel(float width,
                       float height,
                       float repeatU,
                       float repeatV,
                       PlaneOrientation orientation,
                       bool flipNormal = false);

    void draw(const BaseCamera& camera) override;

private:
    VertexBuffer m_vertexBuffer;
    IndexBuffer m_indexBuffer;
};
