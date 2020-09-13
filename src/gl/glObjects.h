#ifndef GLOBJECTS_H
#define GLOBJECTS_H

#include <string>
#include <vector>
#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "stb_image/stb_image.h"

namespace gl
{
    class Shader
    {
    public:
        Shader();
        ~Shader();

        void createProgram(const std::string& fileName);

        void Bind();
        void Unbind();

        void setAttribute(int attributeID, std::string var_name);
        void setUniformLocation(std::string uniform_name);

        int getUniformLocation(std::string uniform_name);

        void loadFloat(int location, float value);
        void loadVector2(int location, glm::vec2 vector);
        void loadVector3(int location, glm::vec3 vector);
        void loadVector4(int location, glm::vec4 vector);
        void loadBool(int location, bool value);
        void loadMatrix(int location, const glm::mat4x4& matrix);

    private:
        static const unsigned int NUM_SHADERS = 2;

        std::vector<std::pair<int, std::string>> m_attributes;
        std::map<std::string, int> m_uniformLocations;

        GLuint CreateShader(const std::string& text, unsigned int type);
        std::string LoadShader(const std::string& fileName);

        GLuint m_program;
        GLuint m_shaders[NUM_SHADERS];
    };
};

namespace gl
{
    struct VertexArray
    {
        VertexArray();
        ~VertexArray();

        void Bind();
        void Unbind();

        GLuint VAO;
    };
};

namespace gl
{
    struct VertexBufferObject
    {
        VertexBufferObject();
        ~VertexBufferObject();

        GLuint VBO;

        void setData(const std::vector<GLfloat>& data, int attributeID, int size, int DrawMode = GL_STATIC_DRAW);
        void setSubData(const std::vector<GLfloat>& data);
    };
};

namespace gl
{
    struct ElementArrayBuffer
    {
        ElementArrayBuffer();
        ~ElementArrayBuffer();

        void setData(const std::vector<GLuint>& indicies, int DrawMode = GL_STATIC_DRAW);
        void setSubData(const std::vector<GLuint>& indicies);

        GLuint EBO;
        GLuint size;
    };
};

namespace gl
{
    struct Texture
    {
        Texture();
        Texture(std::string texture_path);
        ~Texture();

        void loadTexture(std::string texture_path);
        void activateAndBind();

        GLuint texture;
    };
};

namespace gl
{
    struct TextureAtlas
    {
        TextureAtlas(const std::string path, int image_size, int individualTexture_size);

        std::vector<GLfloat> getTextureCoords(const glm::ivec2& coords);

        Texture texture;

        const GLfloat TEX_PER_ROW;
        const GLfloat INDV_TEX_SIZE;
        const GLfloat PIXEL_SIZE;
    };
};

/*
    Usage in code
    -------------
    #define GLOBJECTS_IMPLEMENTATION
    #include "glObjects.h"

    Explanation
    -----------
    We use a the define guard GLOBJECTS_IMPLEMENTATION SO THAT THE .h file is compiled and treated
    as a .cpp file.
    If we didn't use this approach including this header would give us multiple definitions for each
    function or class that we are including because a .h file doesn't create a .o or object file which
    is then cached and used for loading library implementations or definitions.
*/
#ifdef GLOBJECTS_IMPLEMENTATION

//////////////////////////////
// SHADER IMPLEMENTATION   //
/////////////////////////////

#include <fstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

gl::Shader::Shader()
{
    m_program = -1;
}

gl::Shader::~Shader()
{
    for (unsigned int i = 0; i < NUM_SHADERS; i++)
    {
        glDetachShader(m_program, m_shaders[i]);
        glDeleteShader(m_shaders[i]);
    }

    glDeleteProgram(m_program);
}

void gl::Shader::createProgram(const std::string & fileName)
{
    m_program = glCreateProgram();
    m_shaders[0] = CreateShader(LoadShader(fileName + ".vert"), GL_VERTEX_SHADER);
    m_shaders[1] = CreateShader(LoadShader(fileName + ".frag"), GL_FRAGMENT_SHADER);

    for (unsigned int i = 0; i < NUM_SHADERS; i++)
        glAttachShader(m_program, m_shaders[i]);


    // Bind attributes
    for (auto& attribute : m_attributes)
    {
        glBindAttribLocation(m_program, attribute.first, attribute.second.c_str());
    }

    glLinkProgram(m_program);
    glValidateProgram(m_program);
}

void gl::Shader::Bind()
{
    glUseProgram(m_program);
}

void gl::Shader::Unbind()
{
    glUseProgram(0);
}

void gl::Shader::setAttribute(int attributeID, std::string var_name)
{
    m_attributes.push_back(std::make_pair(attributeID, var_name));
}

void gl::Shader::setUniformLocation(std::string uniform_name)
{
    if (m_program == -1)
        std::cout << "CreateProgram hasn't been called yet!\n";

    int location = glGetUniformLocation(m_program, uniform_name.c_str());
    m_uniformLocations.insert(std::make_pair(uniform_name, location));
}

int gl::Shader::getUniformLocation(std::string uniform_name)
{
    if (m_uniformLocations.find(uniform_name) == m_uniformLocations.end())
    {
        std::cout << "Found no location for the uniform: " << uniform_name << "\n";
        return -1;
    }
    else
        return m_uniformLocations.at(uniform_name);
}

void gl::Shader::loadFloat(int location, float value)
{
    glUniform1f(location, value);
}

void gl::Shader::loadVector2(int location, glm::vec2 vector)
{
    glUniform2f(location, vector.x, vector.y);
}

void gl::Shader::loadVector3(int location, glm::vec3 vector)
{
    glUniform3f(location, vector.x, vector.y, vector.z);
}

void gl::Shader::loadVector4(int location, glm::vec4 vector)
{
    glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
}

void gl::Shader::loadBool(int location, bool value)
{
    float out = 0.0f;
    if (value)
        out = 1.0f;

    glUniform1f(location, out);
}

void gl::Shader::loadMatrix(int location, const glm::mat4x4 & matrix)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

GLuint gl::Shader::CreateShader(const std::string & text, unsigned int type)
{
    GLuint shader = glCreateShader(type);

    if (shader == 0)
        std::cerr << "Error compiling shader type " << type << std::endl;

    const GLchar* p[1];
    p[0] = text.c_str();
    GLint lengths[1];
    lengths[0] = text.length();

    glShaderSource(shader, 1, p, lengths);
    glCompileShader(shader);

    return shader;
}

std::string gl::Shader::LoadShader(const std::string & fileName)
{
    std::ifstream file;
    file.open((fileName).c_str());

    std::string output;
    std::string line;

    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, line);
            output.append(line + "\n");
        }
    }
    else
    {
        std::cerr << "Unable to load shader: " << fileName << std::endl;
    }

    return output;
}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////
//    VAO IMPLEMENTATION    //
/////////////////////////////

gl::VertexArray::VertexArray()
{
    glGenVertexArrays(1, &VAO);
}

gl::VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &VAO);
}

void gl::VertexArray::Bind()
{
    glBindVertexArray(VAO);
}

void gl::VertexArray::Unbind()
{
    glBindVertexArray(0);
}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////
//    VBO IMPLEMENTATION    //
/////////////////////////////

gl::VertexBufferObject::VertexBufferObject()
{
    glGenBuffers(1, &VBO);
}

gl::VertexBufferObject::~VertexBufferObject()
{
    glDeleteBuffers(1, &VBO);
}

void gl::VertexBufferObject::setData(const std::vector<GLfloat>& data, int attributeID, int size, int DrawMode)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), DrawMode);

    glEnableVertexAttribArray(attributeID);
    glVertexAttribPointer(attributeID, size, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void gl::VertexBufferObject::setSubData(const std::vector<GLfloat>& data)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * data.size(), data.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////
//    EBO IMPLEMENTATION    //
/////////////////////////////
gl::ElementArrayBuffer::ElementArrayBuffer()
{
    glGenBuffers(1, &EBO);
}

gl::ElementArrayBuffer::~ElementArrayBuffer()
{
    glDeleteBuffers(1, &EBO);
}

void gl::ElementArrayBuffer::setData(const std::vector<GLuint>& indicies, int DrawMode)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicies.size(), indicies.data(), DrawMode);

    size = indicies.size();
}

void gl::ElementArrayBuffer::setSubData(const std::vector<GLuint>& indicies)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * indicies.size(), indicies.data());

    size = indicies.size();
}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////
// Texture IMPLEMENTATION   //
/////////////////////////////
gl::Texture::Texture()
{
    texture = -1;
}

gl::Texture::Texture(std::string texture_path)
{
    loadTexture(texture_path);
}

gl::Texture::~Texture()
{
    glDeleteTextures(1, &texture);
}

void gl::Texture::loadTexture(std::string texture_path)
{
    // Load the texture using stb_image.h
    int width, height, nrChannels;
    unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    if (!data)
    {
        // Error
        texture = -1;
    }
    else
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Send texture data to the GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        // Must add these otherwise the texture doesn't load
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Free image memory
        stbi_image_free(data);
    }
}

void gl::Texture::activateAndBind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
}


/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
// TextureAtlas IMPLEMENTATION //
/////////////////////////////////
// image_size: the width or height of the image (must be 2^x)
// example: 256x256 image, image_size = 256
// individualTexture_size: size of each texture
// example: 64x64 image has 4 textures the size of 32x32, individualTexture_size = 32
gl::TextureAtlas::TextureAtlas(const std::string path, int image_size, int individualTexture_size)
    : TEX_PER_ROW((GLfloat)image_size / (GLfloat)individualTexture_size)
    , INDV_TEX_SIZE(1.0f / TEX_PER_ROW)
    , PIXEL_SIZE(1.0f / (GLfloat)image_size)
{
    texture.loadTexture(path);
}

// get the coordinates of the texture at given position in the 2D grid
std::vector<GLfloat> gl::TextureAtlas::getTextureCoords(const glm::ivec2 & coords)
{
    GLfloat xMin = (coords.x * INDV_TEX_SIZE) + 0.5f * PIXEL_SIZE;
    GLfloat yMin = (coords.y * INDV_TEX_SIZE) + 0.5f * PIXEL_SIZE;

    GLfloat xMax = (xMin + INDV_TEX_SIZE) - PIXEL_SIZE;
    GLfloat yMax = (yMin + INDV_TEX_SIZE) - PIXEL_SIZE;

    return { xMin, yMin, xMin, yMax, xMax, yMax, xMax, yMin };
}

#endif // GLOBJECTS_IMPLEMENTATION

#endif // GLOBJECTS_H