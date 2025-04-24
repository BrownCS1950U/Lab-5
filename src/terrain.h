// terrain.h
#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Quad.h"
#include "mesh.h"    // for DataTex, DrawObject
#include "texture.h"
#include "shaders.h"
#include "camera.h"

namespace gl {

struct Terrain {
    Terrain() = default;
    ~Terrain();

    // initialize shaders, geometry, textures, sky
    void generate(const std::string& dir);
    void initSky(const std::string& dir);
    void setSkybox(const std::string& name, const std::string& dir);
    void render(int mode);

    // —– tweakable parameters exposed to ImGui —–
    float       width_        = 100.f;
    float       height_       = 100.f;
    int         quality_      = 3000;

    float       heightScale_  = 5.0f;
    glm::vec2   uvScale_      = glm::vec2(1.0f);
    float       texelSize_    = 1.0f / 700.0f;

    float       waterLevel_   = 0.0f;
    float       rockLine_     = 1.0f;
    float       snowLine_     = 3.0f;
    float       blendWidth_   = 10.0f;

    glm::vec3   sunDir_       = glm::normalize(glm::vec3(0.2f, -1.0f, 0.2f));
    glm::vec3   sunColor_     = glm::vec3(1.0f, 0.9f, 0.8f);
    glm::vec3   ambientColor_ = glm::vec3(0.2f, 0.2f, 0.3f);

    std::string skyboxName_;

    // rebuild mesh when geometry parameters change
    void regenerate();

private:
    Quad                    quad_;
    GLuint                  program_     = 0;
    GLuint                  skyProgram_  = 0;
    GLuint                  skyVAO_      = 0;
    GLuint                  skyVBO_      = 0;
    GLuint                  skyEBO_      = 0;
    size_t                  skyIndexCount_ = 0;
    GLuint                  heightMap_   = 0;

    struct UniformLocs {
        GLint uModel;
        GLint uViewProj;
        GLint uHeightTex;
        GLint uHeightScale;
        GLint uTexel;
        GLint uWaterLevel;
        GLint uRockLine;
        GLint uSnowLine;
        GLint uSunDir;
        GLint uSunColor;
        GLint uAmbient;
        GLint uSkybox;
        GLint uUVScale;
        GLint uBlendW;
    } loc_;

    std::vector<DataTex>    m_data_;

    void createGeometry();
    void cacheUniformLocations();
    void loadTextures(std::string dir);
};

} // namespace gl