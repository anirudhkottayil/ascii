#define STB_TRUETYPE_IMPLEMENTATION
#include "utils/stb_truetype.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"
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

  const char* edges = "-|\\/";
  // const char* ramp = " .:-=+*#%@";
  const char* ramp = ".:+#@";
  int n = strlen(ramp);
  int n_edges = strlen(edges);
  GlyphBitmap glyphs[n + n_edges];

  // Load font

  FILE* fp = fopen("utils/Px437_IBM_VGA_9x16.ttf", "rb");
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

  // For future me: make sure to keep cell even. Otherwise the quadrant formula doesnt work 
  // and you end up with a lot of slashes instead.
  int cell = 14;
  if (cell % 2 != 0) cell++;
  int render_size = 12;
  float scale = stbtt_ScaleForPixelHeight(&font, (float)render_size);

  int x,y,channels;
  unsigned char *data = stbi_load("input/test1.jpg", &x, &y, &channels, 0);
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
  float* quad_buffer = calloc(rows * cols * 4, sizeof(float));
  if (quad_buffer == NULL){
    stbi_image_free(data);
    free(img_buffer);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }

  int rc = 0; int offset = 0; int half = cell / 2;
  int qrow = 0; int qcol = 0; int quad = 0;
  for (int row = 0; row < rows*cell; row++){
    int cc = 0;
    if ((row % cell == 0) && (row != 0)) rc++;
    for (int col = 0; col < cols*cell*channels; col += channels){
      if ((col % (cell*channels) == 0) && (col != 0)) cc++;
      offset = (row * x * channels) + col;
      r = data[offset];
      g = data[offset + 1];
      b = data[offset + 2];
      // img_buffer[(rc*cols) + cc] += ((r+g+b)/3.0f)/255.0f;
      float sum = (0.299f*r + 0.587f*g + 0.114f*b)/255.0f;
      img_buffer[(rc*cols) + cc] += sum;

      qrow = (row % cell) < half ? 0 : 1; // 0 top half 1 bottom half
      qcol = ((col / channels) % cell) < half ? 0 : 1; // 0 left 1 right
      quad = (qrow * 2) + qcol; //0 TL 1TR 2BL 3BR
      quad_buffer[((rc * cols) + cc) * 4 + quad] += sum;
    }
  }

  int quarter = (cell / 2) * (cell / 2); float cutoff = 0.227f;
  float tl, tr, bl, br, hor, vert, slash, back, strongest, edge_idx;
  for (int row = 0; row < (rows*cols); row++){
    tl = quad_buffer[row*4 + 0] / quarter;
    tr = quad_buffer[row*4 + 1] / quarter;
    bl = quad_buffer[row*4 + 2] / quarter;
    br = quad_buffer[row*4 + 3] / quarter;

    hor = fabsf((tl + tr)/2 - (bl + br)/2);
    vert = fabsf((tl + bl)/2 - (tr + br)/2);
    slash = fabsf(tr - bl);
    back = fabsf(tl - br);

    strongest = hor; edge_idx = 0;
    if (vert > strongest) {
      strongest = vert;
      edge_idx = 1;
    }
    if (slash > strongest) {
      strongest = slash;
      edge_idx = 2;
    }
    if (back > strongest) {
      strongest = back;
      edge_idx = 3;
    }
    float b = img_buffer[row] / (cell*cell);
    // b = powf(b, 0.6f);
    img_buffer[row] = strongest > cutoff ? (n + edge_idx) : pick(b, ramp);
  }

  unsigned char* canvas = calloc(render_size*render_size*rows*cols,1);
  if (canvas == NULL){
    printf("Memory allocation for canvas failed\n");
    free(img_buffer);
    free(ttf_buffer);
    free(quad_buffer);
    stbi_image_free(data);
    return 1;
  }

  for (int i = 0; i < n; i++){
    glyphs[i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, ramp[i], &glyphs[i].w, &glyphs[i].h, &glyphs[i].xoff, &glyphs[i].yoff);
  }
  for (int i = 0; i < n_edges; i++){
    glyphs[n+i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, edges[i], &glyphs[n+i].w, &glyphs[n+i].h, &glyphs[n+i].xoff, &glyphs[n+i].yoff);
}

  offset = 0;
  int glyph_w, glyph_h;
  for(int row = 0; row < rows; row++){
    for (int col = 0; col < cols; col++){
      offset = (row * cols) + col;
      int glyph_idx = (int)img_buffer[offset];
      glyph_w = glyphs[glyph_idx].w;
      glyph_h = glyphs[glyph_idx].h;
      int idx = row * render_size;
      for (int i = 0; i < glyph_h && i < render_size; i++){
        int copy_w = glyph_w < render_size ? glyph_w : render_size;
        memcpy(&canvas[(idx*cols*render_size) + (col * render_size)], &glyphs[glyph_idx].pixels[i*glyph_w],copy_w);
        idx++;
      }
    }
  }

  stbi_write_png("rendered_sz5.png", render_size*cols, render_size*rows, 1, canvas, render_size*cols);


  for (int i = 0; i < n + n_edges; i++){
    stbtt_FreeBitmap(glyphs[i].pixels, NULL);
  }

  free(canvas);
  free(img_buffer);
  free(ttf_buffer);
  free(quad_buffer);
  stbi_image_free(data);
  return 0;
}
  // FILE write
  // FILE* f = fopen("test1.txt", "w");
  // if (f == NULL){
  //   free(ttf_buffer);
  //   free(img_buffer);
  //   stbi_image_free(data);
  //   printf("File fail\n");
  //   return 1;
  // }
  // for (int row = 0; row < rows; row+=2){
  //   for (int col = 0; col < cols; col++){
  //     offset = (row*cols) + col;
  //     fprintf(f, "%c",ramp[(int)img_buffer[offset]]);
  //   }
  //   fprintf(f,"\n");
  // }
  // fclose(f);
