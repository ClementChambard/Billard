#ifndef LENSFLARE_H_
#define LENSFLARE_H_

#include <vector>

#include "Light.h"
#include "Texture.h"
#include "Shader.h"
#include "Query.h"

class LensFlare {
    public:
        /**
         *  Will initialize all openGL objects
         */
        static void Init();

        /**
         *  Will destroy all openGL objects
         */
        static void Cleanup();

        /**
         *  Used to choose the light that will produce lens flare
         *   - l (Light*) : the light in question
         */
        static void setLight(Light* l);

        /**
         *  Adds a texture and a scale to the list of texture to render
         *   - tex (Texture*) : the texture to render to the screen
         *   - scale (float)  : the size in screen coordinate of the texture (will draw a square)
         */
        static void addTexture(Texture* tex, float scale);

        /**
         *  Renders the lens flare on top of everything
         *   - proj (glm::mat4 const&) : the projection matrix, used to get light position in screen space
         */
        static void render(glm::mat4 const& proj);

    private:
        static bool initialized;               // can't use if not initialized
        static std::vector<Texture*> textures; // texture 0 is on the light, texture n is the furthest away
        static std::vector<float> scales;      // the scale in screen space to render the texture of teh same index
        static Light* mainLight;
        static Shader* shader;
        static Query* query;
        static GLuint VAOid, VBOid, uni_alpha;
        static float occlusion;

};

#endif // LENSFLARE_H_
