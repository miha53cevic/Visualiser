#include "quad.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#ifdef _WIN32
#include <SDL.h>
#elif __linux__
#include <SDL2/SDL.h>
#endif

// Comment out if using visual studio community 2017
// so it uses the local directory
//#define RELEASE

Quad::Quad(glm::vec2 screenSize)
{
    prepQuadData();

    // Load shader
    m_shader.setAttribute(0, "position");
    m_shader.setAttribute(1, "textureCoords");

#ifdef RELEASE
    // Get basepath for the assets folder
    char* basePath = SDL_GetBasePath();
    m_shader.createProgram(basePath + std::string("assets/shaders/quad"));
    SDL_free(basePath);
#else
    m_shader.createProgram("assets/shaders/quad");
#endif

    m_shader.setUniformLocation("model");
    m_shader.setUniformLocation("projection");
    m_shader.setUniformLocation("colour");

    // Create projection matrix (2D so we use ortho)
    glm::mat4 projection = glm::ortho(0.0f, (float)screenSize.x, (float)screenSize.y, 0.0f, -1.0f, 1.0f);

    m_shader.Bind();
    m_shader.loadMatrix(m_shader.getUniformLocation("projection"), projection);
    m_shader.Unbind();
}

void Quad::setPosition(float x, float y)
{
    m_position = { x, y };
}

glm::vec2 Quad::getPosition()
{
    return m_position;
}

void Quad::setSize(float w, float h)
{
    m_size = { w, h };
}

glm::vec2 Quad::getSize()
{
    return m_size;
}

void Quad::setRotation(float rotation)
{
    m_rotation = rotation;
}

float Quad::getRotation()
{
    return m_rotation;
}

void Quad::setColour(glm::vec4 colour)
{
    m_colour = colour;
}

glm::vec4 Quad::getColour()
{
    return m_colour;
}

void Quad::Draw()
{
    m_shader.Bind();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(m_position, 0.0f));

    //model = glm::translate(model, glm::vec3(0.5f * m_size.x, 0.5f * m_size.y, 0.0f));
    model = glm::rotate(model, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    //model = glm::translate(model, glm::vec3(-0.5f * m_size.x, -0.5f * m_size.y, 0.0f));

    model = glm::scale(model, glm::vec3(m_size, 1.0f));

    m_shader.loadMatrix(m_shader.getUniformLocation("model"), model);
    m_shader.loadVector4(m_shader.getUniformLocation("colour"), m_colour);

    m_VAO.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_VAO.Unbind();

    m_shader.Unbind();
}

void Quad::prepQuadData()
{
    std::vector<float> verticies = {
        // pos      
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    std::vector<float> textureCoords = {
        // tex
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f
    };

    setVBO(verticies, 0, 2);
    setVBO(textureCoords, 1, 2);

    // Set default values
    m_position = { 0, 0 };
    m_size     = { 0, 0 };
    m_rotation = 0.0f;
    m_colour   = { 255, 255, 255, 255 };
}

void Quad::setVBO(const std::vector<GLfloat>& data, int attributeID, int size, int DrawMode)
{
    m_VAO.Bind();
    auto VBO = std::make_unique<gl::VertexBufferObject>();
    VBO->setData(data, attributeID, size, DrawMode);
    m_VBOs.push_back(std::move(VBO));
    m_VAO.Unbind();
}