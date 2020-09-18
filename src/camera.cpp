#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : m_position(0, 0, 0)
    , m_rotation(0, 0, 0)
{
}

void Camera::setPosition(glm::vec3 pos)
{
    m_position = pos;
}

void Camera::setRotation(glm::vec3 rot)
{
    m_rotation = rot;
}

glm::mat4 Camera::createViewMatrix()
{
    glm::mat4 view(1.0f);
    view = glm::rotate(view, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    view = glm::rotate(view, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    view = glm::rotate(view, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    view = glm::translate(view, -m_position);
    return view;
}
