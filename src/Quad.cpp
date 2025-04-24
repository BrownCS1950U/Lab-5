// Quad.cpp
#include "Quad.h"

Quad::Quad() = default;

Quad::Quad(float width, float height, int quality)
    : m_width(width)
    , m_height(height)
    , m_quality(std::max(quality, 1)) {

    setVertexData();
}

std::vector<float> Quad::updateParams(float width, float height, int quality) {
    m_width    = width;
    m_height   = height;
    m_quality  = std::max(quality, 1);

    m_vertexData.clear();
    m_positions  .clear();
    m_normals    .clear();
    m_textureCoords.clear();

    setVertexData();
    return m_vertexData;
}

void Quad::setVertexData() {
    float halfW = m_width  * 0.5f;
    float halfH = m_height * 0.5f;
    int   n     = m_quality;
    float dx    = m_width  / float(n);
    float dz    = m_height / float(n);
    float du    = 1.0f / float(n);
    float dv    = 1.0f / float(n);
    glm::vec3 normal(0,1,0);

    // for each cell in the grid
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            // corner positions
            glm::vec3 v0(-halfW + dx * i    , 0, -halfH + dz * j    ); // TL
            glm::vec3 v1(-halfW + dx * (i+1), 0, -halfH + dz * j    ); // TR
            glm::vec3 v2(-halfW + dx * (i+1), 0, -halfH + dz * (j+1)); // BR
            glm::vec3 v3(-halfW + dx * i    , 0, -halfH + dz * (j+1)); // BL

            // corner UVs
            glm::vec2 t0(du * i    , 1 - dv * j    );
            glm::vec2 t1(du * (i+1), 1 - dv * j    );
            glm::vec2 t2(du * (i+1), 1 - dv * (j+1));
            glm::vec2 t3(du * i    , 1 - dv * (j+1));

            // triangle 1: TL, BL, BR
            appendVertexData(v0, normal, t0);
            appendVertexData(v3, normal, t3);
            appendVertexData(v2, normal, t2);

            // triangle 2: BR, TR, TL
            appendVertexData(v2, normal, t2);
            appendVertexData(v1, normal, t1);
            appendVertexData(v0, normal, t0);
        }
    }
}

void Quad::appendVertexData(const glm::vec3 &p, const glm::vec3 &n, const glm::vec2 &t) {
    insertVec3(m_vertexData, p);
    m_positions.push_back(p);
    insertVec3(m_vertexData, n);
    m_normals.push_back(n);
    insertVec2(m_vertexData, t);
    m_textureCoords.push_back(t);
}

std::vector<float>& Quad::getData() {
    return m_vertexData;
}

size_t Quad::getSize() {
    return m_vertexData.size();
}

void Quad::insertVec3(std::vector<float> &d, const glm::vec3 &v) {
    d.push_back(v.x);
    d.push_back(v.y);
    d.push_back(v.z);
}

void Quad::insertVec2(std::vector<float> &d, const glm::vec2 &v) {
    d.push_back(v.x);
    d.push_back(v.y);
}
