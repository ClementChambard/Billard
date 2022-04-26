#include "Texture.h"

#include <SDL2/SDL_image.h>

Texture::Texture(std::string filename)
{
    SDL_Surface* img = IMG_Load(filename.c_str());
    SDL_Surface* rgbImg = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(img);

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rgbImg->w, rgbImg->h, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)rgbImg->pixels);
      glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    w = rgbImg->w;
    h = rgbImg->h;

    SDL_FreeSurface(rgbImg);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texID);
}

void Texture::bind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
}

void Texture::unbind()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
