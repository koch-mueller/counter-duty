//
// TexturedPlaneModel.cpp
//

#include "TexturedPlaneModel.h"

namespace
{
    struct PlaneVertex
    {
        Vector position;
        Vector normal;
        Vector tangent;
        Vector bitangent;

        float u;
        float v;
    };
} // namespace

TexturedPlaneModel::TexturedPlaneModel(float width,
                                       float height,
                                       float repeatU,
                                       float repeatV,
                                       PlaneOrientation orientation,
                                       bool flipNormal)
    : m_vertexBuffer(),
      m_indexBuffer()
{
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    PlaneVertex vertices[4];

    switch (orientation)
    {
        case PlaneOrientation::XY:
        {
            Vector normal(0.0f, 0.0f, 1.0f);
            Vector tangent(1.0f, 0.0f, 0.0f);
            Vector bitangent(0.0f, 1.0f, 0.0f);

            vertices[0] =
                {Vector(-halfWidth, -halfHeight, 0.0f), normal, tangent, bitangent, 0.0f, 0.0f};

            vertices[1] =
                {Vector(halfWidth, -halfHeight, 0.0f), normal, tangent, bitangent, repeatU, 0.0f};

            vertices[2] =
                {Vector(-halfWidth, halfHeight, 0.0f), normal, tangent, bitangent, 0.0f, repeatV};

            vertices[3] =
                {Vector(halfWidth, halfHeight, 0.0f), normal, tangent, bitangent, repeatU, repeatV};

            break;
        }

        case PlaneOrientation::XZ:
        {
            Vector normal(0.0f, 1.0f, 0.0f);
            Vector tangent(1.0f, 0.0f, 0.0f);
            Vector bitangent(0.0f, 0.0f, -1.0f);

            vertices[0] =
                {Vector(-halfWidth, 0.0f, halfHeight), normal, tangent, bitangent, 0.0f, 0.0f};

            vertices[1] =
                {Vector(halfWidth, 0.0f, halfHeight), normal, tangent, bitangent, repeatU, 0.0f};

            vertices[2] =
                {Vector(-halfWidth, 0.0f, -halfHeight), normal, tangent, bitangent, 0.0f, repeatV};

            vertices[3] = {Vector(halfWidth, 0.0f, -halfHeight),
                           normal,
                           tangent,
                           bitangent,
                           repeatU,
                           repeatV};

            break;
        }

        case PlaneOrientation::YZ:
        {
            Vector normal(1.0f, 0.0f, 0.0f);
            Vector tangent(0.0f, 0.0f, -1.0f);
            Vector bitangent(0.0f, 1.0f, 0.0f);

            vertices[0] =
                {Vector(0.0f, -halfHeight, halfWidth), normal, tangent, bitangent, 0.0f, 0.0f};

            vertices[1] =
                {Vector(0.0f, -halfHeight, -halfWidth), normal, tangent, bitangent, repeatU, 0.0f};

            vertices[2] =
                {Vector(0.0f, halfHeight, halfWidth), normal, tangent, bitangent, 0.0f, repeatV};

            vertices[3] = {Vector(0.0f, halfHeight, -halfWidth),
                           normal,
                           tangent,
                           bitangent,
                           repeatU,
                           repeatV};

            break;
        }
    }

    if (flipNormal)
    {
        for (PlaneVertex& vertex : vertices)
        {
            vertex.normal = -vertex.normal;
            vertex.bitangent = -vertex.bitangent;
        }
    }

    m_vertexBuffer.begin();

    for (const PlaneVertex& vertex : vertices)
    {
        m_vertexBuffer.addNormal(vertex.normal);

        m_vertexBuffer.addTexcoord0(vertex.u, vertex.v);

        // Der vorhandene VertexBuffer legt Texcoord1 auf Shader-Attribut 3
        m_vertexBuffer.addTexcoord1(vertex.tangent.X, vertex.tangent.Y, vertex.tangent.Z);

        // Texcoord2 liegt auf Shader-Attribut 4
        m_vertexBuffer.addTexcoord2(vertex.bitangent.X, vertex.bitangent.Y, vertex.bitangent.Z);

        m_vertexBuffer.addVertex(vertex.position);
    }

    m_vertexBuffer.end();

    m_indexBuffer.begin();

    if (!flipNormal)
    {
        m_indexBuffer.addIndex(0);
        m_indexBuffer.addIndex(1);
        m_indexBuffer.addIndex(2);

        m_indexBuffer.addIndex(1);
        m_indexBuffer.addIndex(3);
        m_indexBuffer.addIndex(2);
    }
    else
    {
        m_indexBuffer.addIndex(0);
        m_indexBuffer.addIndex(2);
        m_indexBuffer.addIndex(1);

        m_indexBuffer.addIndex(1);
        m_indexBuffer.addIndex(2);
        m_indexBuffer.addIndex(3);
    }

    m_indexBuffer.end();
}

void TexturedPlaneModel::draw(const BaseCamera& camera)
{
    BaseModel::draw(camera);

    m_vertexBuffer.activate();
    m_indexBuffer.activate();

    glDrawElements(GL_TRIANGLES, m_indexBuffer.indexCount(), m_indexBuffer.indexFormat(), nullptr);

    m_indexBuffer.deactivate();
    m_vertexBuffer.deactivate();
}
