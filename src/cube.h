#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include <memory>

#include "gl/glObjects.h"

class Camera;

class Cube
{
public:
    Cube(glm::vec2 screenSize);

    void setPosition(glm::vec3 pos);
    void setRotation(glm::vec3 rot);
    void setScale(glm::vec3 scale);
    void setColour(glm::vec4 colour);

    void Draw(Camera* camera);

private:
    gl::VertexArray m_VAO;
    std::vector<std::unique_ptr<gl::VertexBufferObject>> m_VBOs;
    gl::ElementArrayBuffer m_EBO;

    gl::Shader m_cubeShader;

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    glm::vec4 m_colour;

    void setVBO(const std::vector<GLfloat>& data, int attributeID, int size);
    void setEBO(const std::vector<GLuint>& data);
    void prepData();
    
    glm::mat4 createModelMatrix();
};

#endif