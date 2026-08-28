#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef struct { int w, h, xoff, yoff; unsigned char *pixels; } GlyphBitmap;

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return index;
}

int main(void){
  // Screen dimensions
  // struct winsize w;
  // if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return -1;
  // int width = w.ws_col; int height = w.ws_row;
  // printf("Screen Width: %d\n", width);
  // printf("Screen Height: %d\n", height);

  const char* ramp = " .:-=+*#%@";
  int n = strlen(ramp);
  GlyphBitmap glyphs[n];

  // Load font

  FILE* fp = fopen("Px437_IBM_VGA_9x16.ttf", "rb");
  if (fp == NULL){
    printf("Unable to open font file\n");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp,0,SEEK_SET);
  unsigned char* ttf_buffer = malloc(size);
  if (ttf_buffer == NULL){
    fclose(fp);
    printf("Memory allocation failed for font buffer\n");
    return 1;
  }
  fread(ttf_buffer, 1, size, fp);
  fclose(fp);

  stbtt_fontinfo font;
  if (!stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0))){
    free(ttf_buffer);
    printf("Failed to initialize font\n");
    return 1;
  }
  float scale = stbtt_ScaleForPixelHeight(&font, 56.0f);

  int cell = 56;
  int x,y,channels;
  unsigned char *data = stbi_load("test1.jpg", &x, &y, &channels, 0);
  if (data == NULL){
    free(ttf_buffer);
    printf("Failed to load image\n");
    return 1;
  }

  // Build ascii img buffer
  int r,g,b; int idx = 0;
  int cols = x / cell;
  int rows = y / cell;
  float* img_buffer = calloc(rows * cols , sizeof(float));
  if (img_buffer == NULL){
    stbi_image_free(data);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }
  int rc = 0; int offset = 0;
  for (int row = 0; row < rows*cell; row++){
    int cc = 0;
    if ((row % cell == 0) && (row != 0)) rc++;
    for (int col = 0; col < cols*cell*channels; col += channels){
      if ((col % (cell*channels) == 0) && (col != 0)) cc++;
      offset = (row * x * channels) + col;
      r = data[offset];
      g = data[offset + 1];
      b = data[offset + 2];
      img_buffer[(rc*cols) + cc] += ((r+g+b)/3.0f)/255.0f;
    }
  }

  for (int row = 0; row < (rows*cols); row++){
    img_buffer[row] = pick(img_buffer[row] / (cell*cell), ramp);
  }

  unsigned char* canvas = calloc(cell*cell*rows*cols,1);
  if (canvas == NULL){
    printf("Memory allocation for canvas failed\n");
    free(img_buffer);
    free(ttf_buffer);
    stbi_image_free(data);
    return 1;
  }

  for (int i = 0; i < n; i++){
    glyphs[i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, ramp[i], &glyphs[i].w, &glyphs[i].h, &glyphs[i].xoff, &glyphs[i].yoff);
  }

  offset = 0;
  int glyph_w, glyph_h;
  for(int row = 0; row < rows; row++){
    for (int col = 0; col < cols; col++){
      offset = (row * cols) + col;
      int glyph_idx = (int)img_buffer[offset];
      glyph_w = glyphs[glyph_idx].w;
      glyph_h = glyphs[glyph_idx].h;
      int idx = row * cell;
      for (int i = 0; i < glyph_h && i < cell; i++){
        int copy_w = glyph_w < cell ? glyph_w : cell;
        memcpy(&canvas[(idx*cols*cell) + (col * cell)], &glyphs[glyph_idx].pixels[i*glyph_w],copy_w);
        idx++;
      }
    }
  }

  stbi_write_png("rendered_im3.png", cell*cols, cell*rows, 1, canvas, cell*cols);


  for (int i = 0; i < n; i++){
    stbtt_FreeBitmap(glyphs[i].pixels, NULL);
  }

  free(img_buffer);
  free(ttf_buffer);

  stbi_image_free(data);
  return 0;
}
