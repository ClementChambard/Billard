#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
    pos = {0,4,4};
    lookat = glm::vec3(0,0,-1);
    glm::vec3 right_vec = glm::cross(lookat, up);
    glm::vec3 up2 = glm::normalize(glm::cross(right_vec, lookat));
    viewMat = glm::lookAt(pos, pos + lookat, up2);
    pitch = 0;
    yaw = 3.141592f;
}

void Camera::move(float pitch, float yaw, float forward, float right, float up)
{
    this->pitch += pitch;
    this->yaw += yaw;
    this->pitch = glm::clamp(this->pitch, -3.14159f / 2.f, 3.141592f / 2.f);
    if (this->yaw > 6.28318f) this->yaw -= 6.28318f;
    if (this->yaw < 0) this->yaw += 6.28318f;

    lookat = glm::vec3(glm::cos(this->pitch) * glm::sin(this->yaw), glm::sin(this->pitch), glm::cos(this->pitch) * glm::cos(this->yaw));
    glm::vec3 right_vec = glm::normalize(glm::cross(lookat, this->up));
    glm::vec3 up2 = glm::normalize(glm::cross(right_vec, lookat));

    pos += glm::normalize(glm::vec3(lookat.x, 0, lookat.z)) * forward;
    pos += right * right_vec;
    pos += up * this->up;

    viewMat = glm::lookAt(pos, pos + lookat, up2);
}
