#ifndef LENSFLARE_H_
#define LENSFLARE_H_

#include <vector>

#include "Light.h"
#include "Texture.h"
#include "Shader.h"
#include "FrameBuffer.h"

class LensFlare {
    public:
        static void Init();
        static void Cleanup();

        static void setLight(Light* l);
        static void setTextures(std::vector<Texture*> const& tex);

        static void render(glm::mat4 const& proj);
    private:
        static Light* mainLight;
        static std::vector<Texture*> textures; // texture 0 is on the light, texture n is the furthest away
        static Shader* shader;
        static GLuint VAOid, VBOid;

};

#endif // LENSFLARE_H_
