#include "Material.h"

GLint Material::uni_K;
GLint Material::uni_alpha;
GLint Material::uni_color;

Material::Material(glm::vec3 color, float Ka, float Kd, float Ks, float alpha)
{
    this->color = color;
    this->Ka = Ka;
    this->Kd = Kd;
    this->Ks = Ks;
    this->alpha = alpha;
}

Material::Material(Texture* tex, float Ka, float Kd, float Ks, float alpha)
{
    this->tex = tex;
    this->Ka = Ka;
    this->Kd = Kd;
    this->Ks = Ks;
    this->alpha = alpha;
}

void Material::setUniformLocations(GLint uni_K, GLint uni_alpha, GLint uni_color)
{
    Material::uni_K = uni_K;
    Material::uni_alpha = uni_alpha;
    Material::uni_color = uni_color;
}

void Material::use()
{
    glUniform1f(uni_alpha, alpha);
    glUniform3f(uni_K, Ka, Kd, Ks);
    glUniform3f(uni_color, color.r, color.g, color.b);
    if (tex != nullptr) tex->bind();
}

void Material::unuse()
{
    if (tex != nullptr) tex->unbind();
}
