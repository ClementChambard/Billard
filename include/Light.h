#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>
#include <GL/glew.h>

class Light {

    public:
        /**
         *  Constructor :
         *   - pos (glm::vec3) : the position of the light
         *   - col (glm::vec3) : the color of the light (rgb normalized)
         */
        Light(glm::vec3 pos, glm::vec3 col) : position(pos), camSpacePosition(pos), color(col) {}

        /**
         *  Sends the uniform values corresponding to this light to the current shader
         */
        void sendUniforms();

        /**
         *  Recalculate the camera space position of the light with the new view matrix
         *   - v (glm::mat4 const&) : the view matrix
         */
        void setView(glm::mat4 const& v);

        /**
         *  Returns the camera space position of the light
         */
        glm::vec3 getCamSpacePos() const { return camSpacePosition; }

        /**
         * Used to set the uniform locations used by sendUniforms()
         *  - uni_pos (GLuint) : the location of the light position uniform
         *  - uni_color (GLuint) : the location of the light color uniform
         */
        static void setUniformLocations(GLint uni_pos, GLint uni_color);

    private:
        glm::vec3 position;
        glm::vec3 camSpacePosition;
        glm::vec3 color;
        static GLint uni_pos, uni_color;
};

#endif // LIGHT_H_
