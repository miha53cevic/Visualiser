#ifndef CAMERA_H
#define CAMERA_H

#include "glm/glm.hpp"

class Camera
{
public:
    Camera();

    void setPosition(glm::vec3 pos);
    void setRotation(glm::vec3 rot);

    glm::mat4 createViewMatrix();

private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
};

#endif