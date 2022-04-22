#ifndef SCENE_H_
#define SCENE_H_

#include <stack>
#include <vector>
#include <map>
#include <GL/glew.h>

#include "Cone.h"
#include "Cylinder.h"
#include "Cube.h"
#include "Sphere.h"
#include "Shader.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"

class SceneNode
{
    public:
        SceneNode(Geometry* g, glm::mat4 m);
        ~SceneNode();

        void setMatrices(glm::mat4 const& m) { matrixPropagate = matrixSelf = m; }
        void setMatrices(glm::mat4 const& mp, glm::mat4 const& ms) { matrixPropagate = mp;  matrixSelf = ms; }
        void setMatrixS(glm::mat4 const& m) { matrixSelf = m; }
        void setMatrixP(glm::mat4 const& m) { matrixPropagate = m; }
        void setMaterial(Material* m) { mat = m; }
        SceneNode* addChild(SceneNode* bp) { childs.push_back(bp); return bp; }

        void Render(GLint uMV);

    private:
        Material* mat = nullptr;
        GLuint vaoID = 0;
        GLuint vboID = 0;
        unsigned int nbVert;
        std::vector<SceneNode*> childs;
        glm::mat4 matrixPropagate;
        glm::mat4 matrixSelf;

        static std::stack<glm::mat4> matrices;

        friend class Scene;
};

class Scene
{
    public:
        Scene();
        ~Scene();

        void Render(GLint uMV);

        void addPart(std::string name, SceneNode* node) { parts.insert({name, root->addChild(node)}); }
        void addPart(std::string name, std::string parent, SceneNode* node) { parts.insert({name, parts[parent]->addChild(node)}); }
        void partSetMaterial(std::string name, Material* m) { parts[name]->setMaterial(m); }
        void partSetMatrices(std::string name, glm::mat4 const& m) { parts[name]->setMatrices(m); }
        void setLight(Light* l) { light = l; }
        void setViewMat(glm::mat4 const& m) { view = m; }

    private:
        SceneNode* root;
        Light* light;

        glm::mat4 view;
        std::map<std::string, SceneNode*> parts;
};


#endif // SCENE_H_
