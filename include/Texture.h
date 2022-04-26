#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <iostream>
#include <GL/glew.h>

class Texture {

    public:
        /**
         *  Constructor : loads texture from file
         *   - filename (std::string) : the image file from which to load the texture
         */
        Texture(std::string filename);
        // destructor
        ~Texture();

        /**
         *  Binds the texture for drawing
         */
        void bind();

        /**
         *  Unbinds the texture for drawing
         */
        void unbind();

        /**
         *  Returns the size of the texture
         *   - w (int&) : the width of the texture
         *   - h (int&) : the height of the texture
         */
        void getSize(int& w, int& h) { w = this->w; h = this->h; }

    private:
        GLuint texID;
        int w, h;
};

#endif // TEXTURE_H_
