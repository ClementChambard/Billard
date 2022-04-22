#include "Scene.h"
#include "logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <GL/glew.h>

std::stack<glm::mat4> SceneNode::matrices;

SceneNode::SceneNode(Geometry* g, glm::mat4 m)
{
    matrixPropagate = m;
    matrixSelf = m;

    // if this body part has geometry, create a vertex buffer and fill it
    if (g != nullptr)
    {
        nbVert = g->getNbVertices();
        glGenVertexArrays(1, &vaoID);
        glBindVertexArray(vaoID);

        glGenBuffers(1, &vboID);
        glBindBuffer(GL_ARRAY_BUFFER, vboID);

            glBufferData(GL_ARRAY_BUFFER, (3 + 3 + 2) * sizeof(float)*nbVert, nullptr, GL_DYNAMIC_DRAW);

            glBufferSubData(GL_ARRAY_BUFFER, 0,                       sizeof(float)*3*nbVert, g->getVertices());
            glBufferSubData(GL_ARRAY_BUFFER, 3*sizeof(float)*nbVert,  3*sizeof(float)*nbVert, g->getNormals());
            glBufferSubData(GL_ARRAY_BUFFER, 2*3*sizeof(float)*nbVert,  2*sizeof(float)*nbVert, g->getUVs());

            glVertexAttribPointer(0, 3, GL_FLOAT, 0, 0, 0);
            glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, (void*)(nbVert*3*2*sizeof(float)));
            glVertexAttribPointer(2, 3, GL_FLOAT, 0, 0, (void*)(nbVert*3*sizeof(float)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

SceneNode::~SceneNode()
{
    // cleanup
    if (vaoID != 0) glDeleteVertexArrays(1, &vaoID);
    if (vboID != 0) glDeleteBuffers(1, &vboID);
    for (auto c : childs) delete c;
    childs.clear();
}

void SceneNode::Render(GLint uMV)
{
    // push matrix
    glm::mat4 m = matrixSelf;
    if (matrices.empty()) matrices.push(matrixPropagate);
    else
    {
        m = matrices.top() * m;
        matrices.push(matrices.top() * matrixPropagate);
    }

    // render self
    if (vaoID != 0)
    {
        glBindVertexArray(vaoID);
        if (mat != nullptr) mat->use();
        glUniformMatrix4fv(uMV, 1, false, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, nbVert);
        if (mat != nullptr) mat->unuse();
        glBindVertexArray(0);
    }

    // render all childs
    for (auto c : childs) c->Render(uMV);

    // pop matrix
    matrices.pop();
}


Scene::Scene() : root(new SceneNode(nullptr, glm::mat4(1.f))), view(glm::mat4(1.f)) {}

Scene::~Scene()
{
    delete root;
}

void Scene::Render(GLint uMV)
{
    // push the view matrix then render everything
    SceneNode::matrices.push(view);
    if (light != nullptr) {
        light->setView(view);
        light->sendUniforms();
    }
    root->Render(uMV);
    SceneNode::matrices.pop();
}
