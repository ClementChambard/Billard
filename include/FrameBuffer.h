#ifndef FRAMEBUFFER_INCLUDED_H
#define FRAMEBUFFER_INCLUDED_H

#include <GL/glew.h>

class FrameBuffer {
    public:
        enum type { NONE = 0, DEPTH_TEXTURE, DEPTH_RENDER_BUFFER };

        FrameBuffer(unsigned int width, unsigned int height, type db);
        FrameBuffer(unsigned int width, unsigned int height);
        ~FrameBuffer();

        void bind();
        void unbind();

        void resolveToFBO(FrameBuffer* fbo);
        void resolveToScreen();

        GLuint getColorTexture() const { return colorTextureID; }
        GLuint getDepthTexture() const { return depthTextureID; }

        unsigned int getWidth () const { return width ; }
        unsigned int getHeight() const { return height; }

    private:
        unsigned int width;
        unsigned int height;

        GLuint frameBufferID = 0;
        GLuint colorTextureID = 0;
        GLuint depthTextureID = 0;
        GLuint depthBufferID = 0;
        GLuint colorBufferID = 0;

        bool multisample = false;

        void initialiseFrameBuffer(type dbType);
        void createFrameBuffer();
        void createTextureAttachment();
        void createDepthTextureAttachment();
        void createMultisampleColorAttachement();
        void createDepthBufferAttachment();
};

#endif
