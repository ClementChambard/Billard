#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <iostream>
#include <GL/glew.h>

class Texture {

    public:
        Texture(std::string filename);
        Texture(const char* filename) : Texture(std::string(filename)) {}
        ~Texture();

        void bind();
        void unbind();

        void getSize(int& w, int& h) { w = this->w; h = this->h; }

        static void setTexUni(GLint uni_tex);

    private:
        GLuint texID;
        int w, h;
        static GLint uni_tex;
};

#endif // TEXTURE_H_
