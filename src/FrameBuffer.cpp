#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height, type db)
{
    this->width = width;
    this->height = height;
    initialiseFrameBuffer(db);
}

FrameBuffer::FrameBuffer(unsigned int width, unsigned int height)
{
    this->width = width;
    this->height = height;
    this->multisample = true;
    initialiseFrameBuffer(DEPTH_RENDER_BUFFER);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &frameBufferID);
    glDeleteTextures(1, &colorTextureID);
    glDeleteTextures(1, &depthTextureID);
    glDeleteRenderbuffers(1, &depthBufferID);
    glDeleteRenderbuffers(1, &colorBufferID);
}

void FrameBuffer::bind()
{
    //glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glViewport(0, 0, width, height);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 800, 600);
}

void FrameBuffer::resolveToFBO(FrameBuffer* fbo)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->frameBufferID);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->frameBufferID);
    glBlitFramebuffer(0, 0, width, height, 0, 0, fbo->width, fbo->height,
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    this->unbind();
}

void FrameBuffer::resolveToScreen()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->frameBufferID);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, width, height, 0, 0, 800, 600,
            GL_COLOR_BUFFER_BIT, GL_NEAREST);
    this->unbind();
}

void FrameBuffer::initialiseFrameBuffer(type dbType)
{
    createFrameBuffer();
    if (!multisample)
        createTextureAttachment();
    else
        createMultisampleColorAttachement();
    if (dbType == DEPTH_RENDER_BUFFER)
        createDepthBufferAttachment();
    else if (dbType == DEPTH_TEXTURE)
        createDepthTextureAttachment();
    unbind();
}

void FrameBuffer::createFrameBuffer()
{
    glGenFramebuffers(1, &frameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void FrameBuffer::createTextureAttachment()
{
    glGenTextures(1, &colorTextureID);
    glBindTexture(GL_TEXTURE_2D, colorTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTextureID, 0);
}

void FrameBuffer::createDepthTextureAttachment()
{
    glGenTextures(1, &depthTextureID);
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureID, 0);
}

void FrameBuffer::createMultisampleColorAttachement()
{
    glGenRenderbuffers(1, &colorBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBufferID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferID);
}

void FrameBuffer::createDepthBufferAttachment()
{
    glGenRenderbuffers(1, &depthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
    if (!multisample)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);
}
