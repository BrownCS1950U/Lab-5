// terrain.cpp
#include "terrain.h"
#include "mesh.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

namespace gl {

Terrain::~Terrain() {
    glDeleteProgram(program_);
    glDeleteProgram(skyProgram_);
}

void Terrain::generate(const std::string& dir) {
    // compile terrain shaders
    std::string vert = "../res/shaders/terrain_vertex.glsl";
    std::string frag = "../res/shaders/terrain_fragment.glsl";
    GLuint vs = Shader::init_shaders(GL_VERTEX_SHADER, vert.c_str());
    GLuint fs = Shader::init_shaders(GL_FRAGMENT_SHADER, frag.c_str());
    program_ = Shader::init_program(vs, fs);

    // build geometry
    regenerate();

    // uniforms & textures
    cacheUniformLocations();
    loadTextures(dir);

    // setup skybox
    const char* skyVert = "../res/shaders/sky_vertex.glsl";
    const char* skyFrag = "../res/shaders/sky_fragment.glsl";
    GLuint sky_vs = Shader::init_shaders(GL_VERTEX_SHADER, skyVert);
    GLuint sky_fs = Shader::init_shaders(GL_FRAGMENT_SHADER, skyFrag);
    skyProgram_ = Shader::init_program(sky_vs, sky_fs);
    initSky(dir);

    setSkybox("Clouds", "../data/");
}

void Terrain::regenerate() {
    quad_ = Quad(width_, height_, quality_);
    m_data_.clear();
    createGeometry();
}

void Terrain::initSky(const std::string& dir) {
    // compile sky shaders

    std::string base = dir + "Skyboxes/" + skyboxName_ + "/";
    std::cout << base << std::endl;
    std::vector<std::string> faces = {
        base + "right.jpg",
        base + "left.jpg",
        base + "top.jpg",
        base + "bottom.jpg",
        base + "front.jpg",
        base + "back.jpg"
    };
    loc_.uSkybox = Texture::LoadCubemap(faces);

    // generate inverted-sphere mesh
    std::vector<float> verts;
    std::vector<uint32_t> indices;
    const unsigned X_SEG = 64, Y_SEG = 64;
    for(unsigned y = 0; y <= Y_SEG; ++y) {
      for(unsigned x = 0; x <= X_SEG; ++x) {
        float xSeg = (float)x / X_SEG;
        float ySeg = (float)y / Y_SEG;
        float xPos = cos(xSeg * 2.0f * M_PI) * sin(ySeg * M_PI);
        float yPos = cos(ySeg * M_PI);
        float zPos = sin(xSeg * 2.0f * M_PI) * sin(ySeg * M_PI);
        verts.insert(verts.end(), {xPos,yPos,zPos});
      }
    }
    bool odd = false;
    for(unsigned y = 0; y < Y_SEG; ++y) {
      if(!odd) {
        for(unsigned x = 0; x <= X_SEG; ++x) {
          indices.push_back(y*(X_SEG+1)+x);
          indices.push_back((y+1)*(X_SEG+1)+x);
        }
      } else {
        for(int x = X_SEG; x >= 0; --x) {
          indices.push_back((y+1)*(X_SEG+1)+x);
          indices.push_back(y*(X_SEG+1)+x);
        }
      }
      odd = !odd;
    }
    skyIndexCount_ = indices.size();

    // upload to GPU
    glGenVertexArrays(1,&skyVAO_);
    glGenBuffers(1,&skyVBO_);
    glGenBuffers(1,&skyEBO_);
    glBindVertexArray(skyVAO_);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO_);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyEBO_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glBindVertexArray(0);
}

void Terrain::setSkybox(const std::string& name, const std::string& dir) {
    skyboxName_ = name;
    // delete old cubemap if needed
    glDeleteTextures(1, (GLuint*)&loc_.uSkybox);
    initSky(dir);
}


void Terrain::createGeometry() {
    GLsizei stride = (3+3+2)*sizeof(float);
    GLuint vao, vbo;
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, quad_.getSize()*sizeof(float), quad_.getData().data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(6*sizeof(float)));
    glBindVertexArray(0);

    DataTex dt;
    DrawObject o{};
    o.vao = vao;
    o.vbo = vbo;
    o.numTriangles = quad_.getSize()/(stride/sizeof(float))/3;
    dt.m_draw_objects.push_back(o);
    m_data_.push_back(dt);
}

void Terrain::cacheUniformLocations() {
    auto L = [&](const char* n){ return glGetUniformLocation(program_, n); };
    loc_.uModel       = L("uModel");
    loc_.uViewProj    = L("uViewProj");
    loc_.uHeightTex   = L("uHeightTex");
    loc_.uHeightScale = L("uHeightScale");
    loc_.uTexel       = L("uTexel");

    loc_.uWaterLevel  = L("uWaterLevel");
    loc_.uRockLine    = L("uRockLine");
    loc_.uSnowLine    = L("uSnowLine");

    loc_.uSunDir      = L("uSunDir");
    loc_.uSunColor    = L("uSunColor");
    loc_.uAmbient     = L("uAmbient");

    loc_.uUVScale = glGetUniformLocation(program_, "uUVScale");
    loc_.uBlendW  = glGetUniformLocation(program_, "uBlendW");
}

void Terrain::loadTextures(std::string d) {
    heightMap_ = Texture::LoadTexture(d, "heightmap.jpg");
}

void Terrain::render(int mode) {
    // draw sky
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glUseProgram(skyProgram_);
    glm::mat4 view = Camera::getViewMatrix();
    glm::mat4 proj = Camera::getProjection(1920.f/1080.f);
    glm::mat4 skyVP = proj * glm::mat4(glm::mat3(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram_,"uViewProj"),1,GL_FALSE,&skyVP[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, loc_.uSkybox);
    glUniform1i(glGetUniformLocation(skyProgram_,"uSkybox"),0);
    glBindVertexArray(skyVAO_);
    glDrawElements(GL_TRIANGLE_STRIP, skyIndexCount_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    // draw terrain
    glUseProgram(program_);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 viewProj = proj * view;

    glUniformMatrix4fv(loc_.uViewProj,1,GL_FALSE,&viewProj[0][0]);
    glUniformMatrix4fv(loc_.uModel,1,GL_FALSE,&model[0][0]);

    // apply tweakable parameters
    glUniform1f(loc_.uHeightScale,  heightScale_);
    glUniform2fv(loc_.uUVScale,     1, glm::value_ptr(uvScale_));
    glUniform1f(loc_.uTexel,        texelSize_);

    glUniform1f(loc_.uWaterLevel,   waterLevel_);
    glUniform1f(loc_.uRockLine,     rockLine_);
    glUniform1f(loc_.uSnowLine,     snowLine_);
    glUniform1f(loc_.uBlendW, blendWidth_);

    glUniform3fv(loc_.uSunDir,      1, glm::value_ptr(sunDir_));
    glUniform3fv(loc_.uSunColor,    1, glm::value_ptr(sunColor_));
    glUniform3fv(loc_.uAmbient,     1, glm::value_ptr(ambientColor_));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightMap_);
    glUniform1i(loc_.uHeightTex, 0);

    auto& data = m_data_.front();
    GLenum poly = mode==1?GL_LINE:(mode==2?GL_POINT:GL_FILL);
    if(mode==1) glLineWidth(1.f);
    if(mode==2) glPointSize(5.f);
    Mesh::draw(GL_FRONT_AND_BACK, poly, program_, data);
}

} // namespace gl