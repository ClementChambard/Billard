#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Texture.h"

class Material {

    public:

        // deprecated constructor that used color instead of texture
        //Material(glm::vec3 color, float Ka, float Kd, float Ks, float alpha);

        /**
         *  Constructor :
         *   - tex (Texture*) : the texture of the material
         *   - K[ads], alpha (float) : the illumination constants of the material */
        Material(Texture* tex, float Ka, float Kd, float Ks, float alpha);

        /**
         * Used to send the uniforms and activate the texture
         */
        void use();

        /**
         * Used to deactivate the texture
         */
        void unuse();

        /**
         * Used to set the uniform locations used by use()
         *  - uni_K (GLuint) : the location of the lighting constants uniform
         *  - uni_alpha (GLuint) : the location of the alpha uniform
         *  - uni_color (GLuint) : the location of the color uniform *deprecated*
         */
        static void setUniformLocations(GLint uni_K, GLint uni_alpha, GLint uni_color);

    private:
        glm::vec3 color = glm::vec3(1.f, 1.f, 1.f);
        Texture* tex = nullptr;
        float Ka, Kd, Ks, alpha;
        static GLint uni_K, uni_alpha, uni_color;
};


#endif // MATERIAL_H_
