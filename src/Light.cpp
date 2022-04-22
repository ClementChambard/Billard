#include "Light.h"

GLint Light::uni_color, Light::uni_pos;

void Light::sendUniforms()
{
    glUniform3f(uni_color, color.r, color.g, color.b);
    glUniform3f(uni_pos, camSpacePosition.x, camSpacePosition.y, camSpacePosition.z);
}

void Light::setUniformLocations(GLint uni_pos, GLint uni_color)
{
    Light::uni_color = uni_color;
    Light::uni_pos = uni_pos;
}

void Light::setView(glm::mat4 const& v)
{
    camSpacePosition = glm::vec3(v * glm::vec4(position, 1.f));
}
