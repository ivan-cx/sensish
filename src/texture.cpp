#include "texture.h"

#include <GLFW/glfw3.h>
#include <assert.h>

void deleteTexture(const Texture &tex) { glDeleteTextures(1, &tex.id); }

Texture createTexture(unsigned char *data, int x, int y, int channels) {
  Texture result{};
  result.width = x;
  result.height = y;
  glGenTextures(1, &result.id);
  glBindTexture(GL_TEXTURE_2D, result.id);
  // disable filtering of the texture, user can see "raw" image:
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

  GLint internalFormat{};
  GLenum inputFormat{};

  switch (channels) {
  case 1: {
    internalFormat = GL_R8;
    inputFormat = GL_RED;
    GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    break;
  }
  case 2: {
    internalFormat = GL_RG8;
    inputFormat = GL_RG;
    GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    break;
  }
  case 3: {
    internalFormat = GL_RGB8;
    inputFormat = GL_RGB;
    break;
  }
  case 4: {
    internalFormat = GL_RGBA8;
    inputFormat = GL_RGBA;
    break;
  }
  default:
    assert(false);
    return result;
  }

  /*void glTexImage2D(GLenum target,
          GLint level, - mip
          GLint internalformat, - how data should be stored inside OpenGL
          GLsizei width,
          GLsizei height,
          GLint border, - border width in pixels - 0 or 1
          GLenum format, - input format
          GLenum type, - input data type
          const void * data); */
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, result.width, result.height, 0,
               inputFormat, GL_UNSIGNED_BYTE, data);

  return result;
}

