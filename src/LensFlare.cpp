#include "LensFlare.h"

std::vector<Texture*> LensFlare::textures;
Light* LensFlare::mainLight;
GLuint LensFlare::VAOid, LensFlare::VBOid;
Shader* LensFlare::shader;
GLuint uni_alpha = 0;

void LensFlare::Init()
{
    glGenVertexArrays(1, &VAOid);
    glBindVertexArray(VAOid);

    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2 * 2, nullptr, GL_DYNAMIC_DRAW);

        float UVs[] = {0.f, 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f};
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*2*6, sizeof(float)*2*6, UVs);

        glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, (void*)(6*2*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // create the shader and get the uniform for the model-view matrix
    FILE* vertexFile = fopen("Shaders/lens.vert", "r");
    FILE* fragmentFile = fopen("Shaders/lens.frag", "r");
    shader = Shader::loadFromFiles(vertexFile, fragmentFile);
    fclose(vertexFile);
    fclose(fragmentFile);
    if (shader == nullptr)
    {
        ERROR("failed to load shaders");
        exit(1);
    }

    GLint uni_tex = glGetUniformLocation(shader->getProgramID(), "uTex");
    uni_alpha = glGetUniformLocation(shader->getProgramID(), "uAlpha");
    glUseProgram(shader->getProgramID());
    glUniform1i(uni_tex, 0);
    glUseProgram(0);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glBlendEquation(GL_ADD);
}

void LensFlare::Cleanup()
{
    glDeleteBuffers(1, &VBOid);
    glDeleteVertexArrays(1, &VAOid);
    delete shader;
}

void LensFlare::setLight(Light *l)
{
    mainLight = l;
}

void LensFlare::setTextures(std::vector<Texture*> const& tex)
{
    textures = tex;
}

void LensFlare::render(glm::mat4 const& proj)
{
    glm::vec4 lightPos4 = proj * glm::vec4(mainLight->getCamSpacePos(),1.f);
    glm::vec3 lightPos = glm::vec3(lightPos4 / lightPos4.w);

    if (lightPos.x < -1 || lightPos.y > 1 || lightPos.y < -1 || lightPos.x > 1) return;
    //Check if light is visible

    glm::vec2 offsetDir = glm::normalize(-glm::vec2(lightPos));
    glm::vec2 offset = glm::vec2(lightPos);
    float offsetAmmount = glm::length(offset) * 2.f/(float)textures.size();
    float width = 1.f;
    float height = 1.f *800.f/600.f;
    float scaling = 1 + 0.5f/textures.size();
    float alpha = glm::max(1.2f - glm::length(offset),0.f)/2.f;
    float alpha2 = alpha;
    if (alpha2 > 0.5f)
    {
        alpha2 = glm::max(0.5f -  6*(alpha2-0.5f),0.f);
    }

    int w0, h0; textures[0]->getSize(w0, h0);
    glUseProgram(shader->getProgramID());
    glBindVertexArray(VAOid);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glUniform1f(uni_alpha, alpha);
    for (auto t : textures)
    {
        t->bind();
        int wi, hi; t->getSize(wi, hi);
        float w = width * (float)wi / (float)w0;
        float h = height * (float)hi / (float)h0;
        glm::vec2 posTL = offset - glm::vec2( w/2.f, -h/2.f);
        glm::vec2 posTR = offset - glm::vec2(-w/2.f, -h/2.f);
        glm::vec2 posBR = offset - glm::vec2(-w/2.f,  h/2.f);
        glm::vec2 posBL = offset - glm::vec2( w/2.f,  h/2.f);

        float Pos[] = {posTL.x, posTL.y, posBR.x, posBR.y, posTR.x, posTR.y, posTL.x, posTL.y, posBL.x, posBL.y, posBR.x, posBR.y};

        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*2*6, Pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        t->unbind();

        if (width == 1) glUniform1f(uni_alpha, alpha2);
        width *= scaling;
        height *= scaling;
        offset += offsetDir * offsetAmmount;
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}
