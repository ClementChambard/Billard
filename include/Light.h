#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>
#include <GL/glew.h>

class Light {

    public:
        Light(glm::vec3 pos, glm::vec3 col) : position(pos), camSpacePosition(pos), color(col) {}

        void sendUniforms();
        void setView(glm::mat4 const& v);

        glm::vec3 getCamSpacePos() const { return camSpacePosition; }

        static void setUniformLocations(GLint uni_pos, GLint uni_color);

    private:
        glm::vec3 position;
        glm::vec3 camSpacePosition;
        glm::vec3 color;
        static GLint uni_pos, uni_color;
};

#endif // LIGHT_H_
