#include "LensFlare.h"

bool LensFlare::initialized = false;
std::vector<Texture*> LensFlare::textures;
std::vector<float> LensFlare::scales;
Light* LensFlare::mainLight;
Shader* LensFlare::shader;
Query* LensFlare::query;
GLuint LensFlare::VAOid, LensFlare::VBOid, LensFlare::uni_alpha;
float LensFlare::occlusion = 1.f;

void LensFlare::Init()
{
    // create the vao and vbo
    glGenVertexArrays(1, &VAOid);
    glBindVertexArray(VAOid);

    glGenBuffers(1, &VBOid);
    glBindBuffer(GL_ARRAY_BUFFER, VBOid);

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 3 * 2, nullptr, GL_DYNAMIC_DRAW);

        // set the UVs now since we will not change it later
        float UVs[] = {0.f, 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f};
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*3*6, sizeof(float)*2*6, UVs);

        glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, (void*)(6*3*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // create the shader and get the uniforms
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
    uni_alpha = glGetUniformLocation(shader->getProgramID(), "uAlpha");
    GLint uni_tex = glGetUniformLocation(shader->getProgramID(), "uTex");

    // set the texture uniform now since we will not need to change it later
    glUseProgram(shader->getProgramID());
    glUniform1i(uni_tex, 0);
    glUseProgram(0);

    // create a query for occlusion
    query = new Query(GL_SAMPLES_PASSED);

    initialized = true;
}

void LensFlare::Cleanup()
{
    glDeleteBuffers(1, &VBOid);
    glDeleteVertexArrays(1, &VAOid);
    delete shader;
    delete query;
    initialized = false;
}

void LensFlare::setLight(Light *l)
{
    mainLight = l;
}

void LensFlare::addTexture(Texture *tex, float scale)
{
    textures.push_back(tex);
    scales.push_back(scale);
}

void LensFlare::render(glm::mat4 const& proj)
{
    if (!initialized) return;

    // calculate the light position in screen space
    glm::vec4 lightPos4 = proj * glm::vec4(mainLight->getCamSpacePos(),1.f);
    glm::vec3 lightPos = glm::vec3(lightPos4 / lightPos4.w);
    glm::vec2 offset = glm::vec2(lightPos);

    // the flare is brighter the closest the light source is to the center of the screen
    float brightness = 1 - (glm::length(offset) / 1.4f);
    if (brightness <= 0) return;

    // a few variables to update the offset later
    glm::vec2 offsetDir = glm::normalize(-offset);
    float offsetAmmount = glm::length(offset) * 2.f/(float)textures.size();
    float aspectRatio = 800.f/600.f;

    // bind everything
    glUseProgram(shader->getProgramID());
    glBindVertexArray(VAOid);

    // this part is based on ThinMatrix's youtube tutorial (see the Query class)
    if (query->isResultReady())
    {
        // the occlusion value depends on the result of the query
        float samples = query->getResult();
        int TOTAL_SAMPLES = 0.25*0.1*0.1*800*800;
        occlusion = glm::min(samples / TOTAL_SAMPLES, 1.f);
    }
    if (!query->isInUse())
    {
        // disable drawing to anything
        glColorMask(false,false,false,false);
        glDepthMask(false);

        query->start();

        // draw a quad at the position of the light source
        float w = 0.1f ;
        float h = w * aspectRatio ;
        glm::vec2 posTL = offset - glm::vec2( w/2.f, -h/2.f);
        glm::vec2 posTR = offset - glm::vec2(-w/2.f, -h/2.f);
        glm::vec2 posBR = offset - glm::vec2(-w/2.f,  h/2.f);
        glm::vec2 posBL = offset - glm::vec2( w/2.f,  h/2.f);

        float Pos[] = {posTL.x, posTL.y, lightPos.z, posBR.x, posBR.y, lightPos.z, posTR.x, posTR.y, lightPos.z, posTL.x, posTL.y, lightPos.z, posBL.x, posBL.y, lightPos.z, posBR.x, posBR.y, lightPos.z};

        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*6, Pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        query->end();

        // reenable drawing to everything
        glColorMask(true,true,true,true);
        glDepthMask(true);
    }

    // Draw on top of everything with an additive blendmode
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glBlendEquation(GL_ADD);

    // Set the transparency
    float alpha = occlusion * brightness/2.f;
    glUniform1f(uni_alpha, alpha);

    // Draw all textures : calculate the vertex positions and update the offset
    for (size_t i = 0; i < textures.size(); i++)
    {
        float scale = scales[i];
        textures[i]->bind();
        float w = scale ;
        float h = w * aspectRatio ;
        glm::vec2 posTL = offset - glm::vec2( w/2.f, -h/2.f);
        glm::vec2 posTR = offset - glm::vec2(-w/2.f, -h/2.f);
        glm::vec2 posBR = offset - glm::vec2(-w/2.f,  h/2.f);
        glm::vec2 posBL = offset - glm::vec2( w/2.f,  h/2.f);

        float Pos[] = {posTL.x, posTL.y, 1.f, posBR.x, posBR.y, 1.f, posTR.x, posTR.y, 1.f, posTL.x, posTL.y, 1.f, posBL.x, posBL.y, 1.f, posBR.x, posBR.y, 1.f};

        glBindBuffer(GL_ARRAY_BUFFER, VBOid);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*3*6, Pos);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        textures[i]->unbind();

        offset += offsetDir * offsetAmmount;
    }

    // Reset the values and unbind everything
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}
