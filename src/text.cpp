// Most code in this files is written by Justin Meiners:
// https://github.com/justinmeiners/stb-truetype-example/blob/master/main.c
// and is licnesed under following terms:
//
// MIT License
//
// Copyright (c) 2017 Justin Meiners
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "text.h"

#include <math.h>
#include <stb.h>
#include <stb_truetype.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
static unsigned char *fontBuffer;
static stbtt_fontinfo info;

void watermark_init_font(const char *filename) {
  long size;

  FILE *fontFile = fopen(filename, "rb");

  if (!fontFile) {
    printf("failed to load font file: %s\n", filename);
    return;
  }

  fseek(fontFile, 0, SEEK_END);
  size = ftell(fontFile);       /* how long is the file ? */
  fseek(fontFile, 0, SEEK_SET); /* reset */

  if (size <= 0) {
    printf("failed to load font file: %s\n", filename);
    return;
  }

  fontBuffer = (unsigned char *)malloc(size);

  const size_t items_read = fread(fontBuffer, size, 1, fontFile);
  fclose(fontFile);

  /* prepare font */
  if (!items_read ||
      !stbtt_InitFont(&info, fontBuffer,
                      stbtt_GetFontOffsetForIndex(fontBuffer, 0))) {
    printf("failed to load font file: %s\n", filename);
    return;
  }
}

void watermark_draw_text(unsigned char *texture, int width, int height,
                         int channels, const WatermarkText &text) {
  int l_h = 32;
  /* create a bitmap for the phrase */
  unsigned char *bitmap =
      (unsigned char *)calloc(width * height, sizeof(unsigned char));

  /* calculate font scaling */
  float scale = stbtt_ScaleForPixelHeight(&info, l_h);

  stb__wchar codepoints[TEXT_LINE_SIZE];
  // TODO all 3 lines:
  stb_from_utf8(codepoints, text.lines[0], TEXT_LINE_SIZE);

  int x = 0;

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

  ascent = roundf(ascent * scale);
  descent = roundf(descent * scale);
  size_t i = 0;
  while (codepoints[i] != 0) {
    /* how wide is this character */
    int ax;
    int lsb;
    stbtt_GetCodepointHMetrics(&info, codepoints[i], &ax, &lsb);

    /* get bounding box for character (may be offset to account for chars that
     * dip above or below the line */
    int c_x1, c_y1, c_x2, c_y2;
    stbtt_GetCodepointBitmapBox(&info, codepoints[i], scale, scale, &c_x1,
                                &c_y1, &c_x2, &c_y2);

    /* compute y (different characters have different heights */
    int y = ascent + c_y1;

    /* render character (stride and offset is important here) */
    int byteOffset = x + roundf(lsb * scale) + (y * width);
    stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1,
                              c_y2 - c_y1, width, scale, scale, codepoints[i]);

    /* advance x */
    x += roundf(ax * scale);

    /* add kerning */
    int kern;
    kern =
        stbtt_GetCodepointKernAdvance(&info, codepoints[i], codepoints[i + 1]);
    x += roundf(kern * scale);
    ++i;
  }

  const size_t pixel_count = width * height;
  for (size_t i = 0; i < pixel_count; ++i) {
    if (bitmap[i])
      texture[i * channels] = 0xFF;
  }

  free(bitmap);
}

