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
        /**
         *  Constructor :
         *   - g (Geometry*) : the 3D model associated to the node (can be nullptr for a node with no geometry)
         *   - m (glm::mat4) : the transformation matrix of the node
         */
        SceneNode(Geometry* g, glm::mat4 m);
        // destructor
        ~SceneNode();

        /**
         *  Used to set the matrices of the node
         *   - m (glm::mat4 const&) : the new propagation and transformation matrix of the node
         */
        void setMatrices(glm::mat4 const& m) { matrixPropagate = matrixSelf = m; }

        /**
         *  Used to set the matrices of the node
         *   - mp (glm::mat4 const&) : the new propagation matrix of the node
         *   - ms (glm::mat4 const&) : the new transformation matrix of the node
         */
        void setMatrices(glm::mat4 const& mp, glm::mat4 const& ms) { matrixPropagate = mp;  matrixSelf = ms; }

        /**
         *  Used to set the matrices of the node
         *   - m (glm::mat4 const&) : the new transformation matrix of the node
         */
        void setMatrixS(glm::mat4 const& m) { matrixSelf = m; }

        /**
         *  Used to set the matrices of the node
         *   - m (glm::mat4 const&) : the new propagation matrix of the node
         */
        void setMatrixP(glm::mat4 const& m) { matrixPropagate = m; }

        /**
         *  Used to set the material of the node
         *   - m (Material*) : the new material of the node
         */
        void setMaterial(Material* m) { mat = m; }

        /**
         *  Used to add a child node to this node
         *   - bp (SceneNode*) : the node to add as a child
         */
        SceneNode* addChild(SceneNode* bp) { childs.push_back(bp); return bp; }

        /**
         *  Used to render the node
         *   - uMV (GLuint) : the uniform location for the model view matrix
         */
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
        // constructor
        Scene();
        // destructor
        ~Scene();

        /**
         *  Will render the entire scene
         *   - uMV (GLuint) : the uniform location for the model view matrix
         */
        void Render(GLint uMV);

        /**
         *  Add a part at the root of the tree.
         *   - name (std::string) : the name of the node. This will be used as the identifier for the node
         *   - node (SceneNode*) : the associated node.
         */
        void addPart(std::string name, SceneNode* node) { parts.insert({name, root->addChild(node)}); }

        /**
         *  Add a part to the tree with the specified parent.
         *   - name (std::string) : the name of the node. This will be used as the identifier for the node
         *   - parent (std::string) : the name of the parent node.
         *   - node (SceneNode*) : the associated node.
         */
        void addPart(std::string name, std::string parent, SceneNode* node) { parts.insert({name, parts[parent]->addChild(node)}); }

        /**
         *  Will call setMaterial() on the given node
         *   - name (std::string) : the name of the node
         *   - m (Material*) : the new material of the node
         */
        void partSetMaterial(std::string name, Material* m) { parts[name]->setMaterial(m); }

        /**
         *  Will call setMatrices() on the given node
         *   - name (std::string) : the name of the node
         *   - m (glm::mat4 const&) : the new matrix of the node
         */
        void partSetMatrices(std::string name, glm::mat4 const& m) { parts[name]->setMatrices(m); }

        /**
         *  Will set which light is illuminating the scene
         *   - l (Light*) : the light
         *   TODO: multiple lights ?
         */
        void setLight(Light* l) { light = l; }

        /**
         *  Will set the view matrix which is the root matrix of the scene
         *   - m (glm::mat4 const&) : the view matrix
         */
        void setViewMat(glm::mat4 const& m) { view = m; }

    private:
        SceneNode* root;
        Light* light;

        glm::mat4 view;
        std::map<std::string, SceneNode*> parts;
};


#endif // SCENE_H_
