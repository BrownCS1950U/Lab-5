#pragma once

#include <vector>
#include <glm/glm.hpp>

class Quad {
public:
    Quad();
    explicit Quad(float width, float height, int quality = 1);

    std::vector<float> updateParams(float width, float height, int quality);


    std::vector<float>& getData();
    size_t getSize();
    [[nodiscard]] std::vector<glm::vec3> getNormals() const;
    [[nodiscard]] std::vector<glm::vec2> getTexCoords() const;

private:
    float m_width;
    float m_height;
    int m_quality;
    std::vector<float> m_vertexData;
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_textureCoords;

    void setVertexData();
    void appendVertexData(const glm::vec3 &position, const glm::vec3 &normal, const glm::vec2 &texCoord);
    static inline void insertVec3(std::vector<float> &data, const glm::vec3 &v);
    static inline void insertVec2(std::vector<float> &data, const glm::vec2 &v);
};
