#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "gl/glObjects.h"

class Quad
{
public:
    Quad(glm::vec2 screenSize);

    void      setPosition(float x, float y);
    glm::vec2 getPosition();

    void      setSize(float w, float h);
    glm::vec2 getSize();

    void      setRotation(float rotation);
    float     getRotation();

    void      setColour(glm::vec4 colour);
    glm::vec4 getColour();

    void Draw();

private:
    glm::vec2 m_position;
    glm::vec2 m_size;
    float     m_rotation;
    glm::vec4 m_colour;

    gl::VertexArray m_VAO;
    std::vector<std::unique_ptr<gl::VertexBufferObject>> m_VBOs;

    gl::Shader m_shader;

    void prepQuadData();

    void setVBO(const std::vector<GLfloat>& data, int attributeID, int size, int DrawMode = GL_STATIC_DRAW);
};

#endif