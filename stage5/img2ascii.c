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

typedef struct { int w, h, xoff, yoff; unsigned char *pixels; } GlyphBitmap;

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return index;
}

int main(int argc, char** argv){

  const char* input_path = argv[1];
  const char* output_path = argv[2];
  int cell = atoi(argv[3]);
  int render_size = atoi(argv[4]);
  const char* font_path = argv[5];
  static const char edges[] = "-|\\/";
  // static const char* ramp = " .:-=+*#%@";
  static const char ramp[] = ".:+#@";
  #define RAMP_LEN (sizeof(ramp) - 1)
  #define EDGE_LEN (sizeof(edges) - 1)
  int n = RAMP_LEN;
  int n_edges = EDGE_LEN;
  GlyphBitmap glyphs[n + n_edges];

  // Load font
  FILE* fp = fopen(font_path, "rb");
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
  if (cell % 2 != 0) cell++;
  float scale = stbtt_ScaleForPixelHeight(&font, (float)render_size);

  int channels = 3;
  int x,y,comp;
  unsigned char *data = stbi_load(input_path, &x, &y, &comp, channels);
  if (data == NULL){
    free(ttf_buffer);
    printf("Failed to load image\n");
    return 1;
  }

  // Build ascii img buffer
  int r,g,b;
  int cols = x / cell;
  int rows = y / cell;
  float* img_buffer = calloc(rows * cols , sizeof(float));
  if (img_buffer == NULL){
    stbi_image_free(data);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }
  float* color_buffer = calloc(rows * cols * 3, sizeof(float));
  if (color_buffer == NULL){
    free(img_buffer);
    stbi_image_free(data);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }
  float* quad_buffer = calloc(rows * cols * 4, sizeof(float));
  if (quad_buffer == NULL){
    stbi_image_free(data);
    free(img_buffer);
    free(color_buffer);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }

  // Load img data into buffer while recording color and cell features
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
      color_buffer[((rc*cols) +cc) * 3] += r;
      color_buffer[(((rc*cols) +cc) * 3) + 1] += g;
      color_buffer[(((rc*cols) +cc) * 3) + 2] += b;
      // img_buffer[(rc*cols) + cc] += ((r+g+b)/3.0f)/255.0f;
      float sum = (0.299f*r + 0.587f*g + 0.114f*b)/255.0f;
      img_buffer[(rc*cols) + cc] += sum;

      qrow = (row % cell) < half ? 0 : 1; // 0 top half 1 bottom half
      qcol = ((col / channels) % cell) < half ? 0 : 1; // 0 left 1 right
      quad = (qrow * 2) + qcol; //0 TL 1TR 2BL 3BR
      quad_buffer[((rc * cols) + cc) * 4 + quad] += sum;
    }
  }

  // normalize the values (in range btw 0 to 1) for each cell and pick a ramp character or edge character
  int quarter = (cell / 2) * (cell / 2); float cutoff = 0.227f;
  float tl, tr, bl, br, hor, vert, slash, back, strongest, edge_idx;
  for (int row = 0; row < (rows*cols); row++){
    tl = quad_buffer[row*4 + 0] / quarter;
    tr = quad_buffer[row*4 + 1] / quarter;
    bl = quad_buffer[row*4 + 2] / quarter;
    br = quad_buffer[row*4 + 3] / quarter;

    color_buffer[row*3 + 0] /= (cell * cell);
    color_buffer[row*3 + 1] /= (cell * cell);
    color_buffer[row*3 + 2] /= (cell * cell);

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

  // For future if a black and while mode feature is added
  // int color_mode = 1;
  // int num_channels = color_mode ? 3 : 1;
  int num_channels = 3;

  unsigned char* canvas = calloc(render_size*render_size*rows*cols*num_channels,1);
  if (canvas == NULL){
    printf("Memory allocation for canvas failed\n");
    free(img_buffer);
    free(ttf_buffer);
    free(quad_buffer);
    free(color_buffer);
    stbi_image_free(data);
    return 1;
  }

  // Get the map for each character in ramp stored in glyphs.pixels
  for (int i = 0; i < n; i++){
    glyphs[i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, ramp[i], &glyphs[i].w, &glyphs[i].h, &glyphs[i].xoff, &glyphs[i].yoff);
  }
  // Get the map for the edges
  for (int i = 0; i < n_edges; i++){
    glyphs[n+i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, edges[i], &glyphs[n+i].w, &glyphs[n+i].h, &glyphs[n+i].xoff, &glyphs[n+i].yoff);
}

  // Loop through img_buffer and for each cell copy th relevant character from glyphs to canvas
  offset = 0;
  int cidx = 0;
  int glyph_w, glyph_h;
  for(int row = 0; row < rows; row++){
    for (int col = 0; col < cols; col++){
      offset = (row * cols) + col;
      cidx = offset * 3;
      float cr = color_buffer[cidx], cg = color_buffer[cidx+1], cb = color_buffer[cidx+2];
      int glyph_idx = (int)img_buffer[offset];
      glyph_w = glyphs[glyph_idx].w;
      glyph_h = glyphs[glyph_idx].h;
      int idx = row * render_size;
      for (int i = 0; i < glyph_h && i < render_size; i++){
        unsigned char* glyph_row = &glyphs[glyph_idx].pixels[i*glyph_w];
        int base_px = (idx*cols*render_size) + (col * render_size);
        int copy_w = glyph_w < render_size ? glyph_w : render_size;
        for (int j = 0; j < copy_w; j++){
          unsigned char pix = glyph_row[j];
          int base_off = (base_px + j) * 3;
          canvas[base_off + 0] = (unsigned char) (cr * pix / 255.0f);
          canvas[base_off + 1] = (unsigned char) (cg * pix / 255.0f);
          canvas[base_off + 2] = (unsigned char) (cb * pix / 255.0f);
        }
        idx++;
      }
    }
  }

  // Write the image to a .png using canvas
  stbi_write_png(output_path, render_size*cols, render_size*rows, num_channels, canvas, render_size*cols*num_channels);

  // Free all allocated memory
  for (int i = 0; i < n + n_edges; i++){
    stbtt_FreeBitmap(glyphs[i].pixels, NULL);
  }
  free(canvas);
  free(img_buffer);
  free(ttf_buffer);
  free(quad_buffer);
  free(color_buffer);
  stbi_image_free(data);
  return 0;
}
