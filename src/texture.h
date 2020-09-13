#include "GL/gl3w.h" // for glTextureID

struct Texture {
  int width = 0;
  int height = 0;
  GLuint id = 0;
};

Texture createTexture(unsigned char *data, int x, int y, int channels);
void deleteTexture(const Texture &tex);
