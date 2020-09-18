#include "cube.h"

#ifdef _WIN32
#include <SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#endif
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

Cube::Cube(glm::vec2 screenSize)
    : m_position(0, 0, 0)
    , m_rotation(0, 0, 0)
    , m_scale(1, 1, 1)
    , m_colour(1, 1, 1, 1)
{
    m_cubeShader.setAttribute(0, "position");

    // Get basepath for the assets folder
    char* basePath = SDL_GetBasePath();
    m_cubeShader.createProgram(basePath + std::string("assets/shaders/cube"));
    SDL_free(basePath);

    m_cubeShader.setUniformLocation("modelMatrix");
    m_cubeShader.setUniformLocation("projectionMatrix");
    m_cubeShader.setUniformLocation("viewMatrix");
    m_cubeShader.setUniformLocation("colour");

    // Load projection matrix
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), screenSize.x / screenSize.y, 0.1f, 1000.0f);
    m_cubeShader.Bind();
    m_cubeShader.loadMatrix(m_cubeShader.getUniformLocation("projectionMatrix"), projectionMatrix);
    m_cubeShader.Unbind();

    prepData();
}

void Cube::setPosition(glm::vec3 pos)
{
    m_position = pos;
}

void Cube::setRotation(glm::vec3 rot)
{
    m_rotation = rot;
}

void Cube::setScale(glm::vec3 scale)
{
    m_scale = scale;
}

void Cube::setColour(glm::vec4 colour)
{
    m_colour = colour;
    m_colour /= 255.0f;
}

void Cube::Draw(Camera* camera)
{
    m_cubeShader.Bind();

    // Load view matrix
    m_cubeShader.loadMatrix(m_cubeShader.getUniformLocation("viewMatrix"), camera->createViewMatrix());

    // Load model matrix
    m_cubeShader.loadMatrix(m_cubeShader.getUniformLocation("modelMatrix"), createModelMatrix());

    // Load Colour
    m_cubeShader.loadVector4(m_cubeShader.getUniformLocation("colour"), m_colour);

    m_VAO.Bind();
    glDrawElements(GL_TRIANGLES, m_EBO.size, GL_UNSIGNED_INT, 0);
    m_VAO.Unbind();

    m_cubeShader.Unbind();
}

void Cube::setVBO(const std::vector<GLfloat>& data, int attributeID, int size)
{
    m_VAO.Bind();
    auto VBO = std::make_unique<gl::VertexBufferObject>();
    VBO->setData(data, attributeID, size);
    m_VBOs.push_back(std::move(VBO));
    m_VAO.Unbind();
}

void Cube::setEBO(const std::vector<GLuint>& data)
{
    m_VAO.Bind();
    m_EBO.setData(data);
    m_VAO.Unbind();
}

void Cube::prepData()
{
    std::vector<GLfloat> verticies = {
        // Back face
        1,1,0,
        1,0,0,
        0,0,0,
        0,1,0,

        // Front face
        0,1,1,
        0,0,1,
        1,0,1,
        1,1,1,

        // Right face
        1,1,1,
        1,0,1,
        1,0,0,
        1,1,0,

        // Left Face
        0,1,0,
        0,0,0,
        0,0,1,
        0,1,1,

        // Top face
        0,1,1,
        1,1,1,
        1,1,0,
        0,1,0,

        // Bottom face
        0,0,1,
        0,0,0,
        1,0,0,
        1,0,1
    };

    std::vector<GLuint> indicies = {
         0,1,3,
         3,1,2,
         4,5,7,
         7,5,6,
         8,9,11,
         11,9,10,
         12,13,15,
         15,13,14,
         16,17,19,
         19,17,18,
         20,21,23,
         23,21,22
    };

    setVBO(verticies, 0, 3);
    setEBO(indicies);
}

glm::mat4 Cube::createModelMatrix()
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, m_scale);
    return model;
}