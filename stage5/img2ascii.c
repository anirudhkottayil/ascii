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
  // Load image
  // int x,y,n;
  // unsigned char *data = stbi_load("test1.jpg", &x, &y, &n, 0);
  // if (data == NULL){
  //   printf("Failed to load image\n");
  //   return 1;
  // }

  // Build ascii img buffer
  // int r,g,b; int idx = 0;
  // char* buffer = malloc(((x*y) + y + 1) * sizeof(char));
  // if (buffer == NULL){
  //   printf("Malloc failed\n");
  //   return 1;
  // }
  //
  // for (int i = 0; i < y;i++){
  //   for (int j = 0; j < x;){
  //     int offset = ((i*x)+j) * n; 
  //     r = data[offset];
  //     g = data[offset+1];
  //     b = data[offset+2];
  //     float brightness = ((r + g + b) / 3.0f) / 255.0f;
  //     char c = pick(brightness, ramp);
  //     buffer[idx++] = c;
  //     j += 3;
  //   }
  //   buffer[idx++] = '\n';
  // }
  // buffer[idx]= '\0';
  // free(buffer);


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
  float scale = stbtt_ScaleForPixelHeight(&font, 32.0f);

  int cell = 32;
  int x,y,channels,img_size;
  unsigned char *data = stbi_load("test1.jpg", &x, &y, &channels, 0);
  if (data == NULL){
    free(ttf_buffer);
    printf("Failed to load image\n");
    return 1;
  }
  img_size = x * y * n;

  // Build ascii img buffer
  int r,g,b; int idx = 0;
  int cols = x / cell;
  int rows = y / cell;
  unsigned char* img_buffer = calloc((((rows * cols)) ), sizeof(int));
  if (img_buffer == NULL){
    stbi_image_free(data);
    free(ttf_buffer);
    printf("Malloc img_buffer failed\n");
    return 1;
  }
  // int offset = 0;
  // for (int row = 0; row < rows; row++){
  //   int idx = 0;
  //   for (int col = 0; col < cols; col++){
  //     offset = ((row * cell* channels*x) + col*(cell*channels));
  //     int i = 0;
  //     while (i < cell){
  //       r = data[offset + i];
  //       g = data[offset + i + 1];
  //       b = data[offset + i + 2];
  //       img_buffer[(row*cols) + idx] += r + g + b;
  //       i += 3;
  //     }
  //     idx++;
  //   }
  // }

  for (int row = 0; row < rows; row++){
    int idx = 0;
    for (int col = 0; col < cols; col++){
      int sum = 0;
      for (int dy = 0; dy < cell; dy++){
        int row_start = ((row*cell + dy) * x + col*cell) * channels;
        for (int dx = 0; dx < cell; dx++){
          int p = row_start + dx*channels;
          r = data[p];
          g = data[p + 1];
          b = data[p + 2];
          sum += r + g + b;
        }
      }
      img_buffer[(row*cols) + idx] = sum / (cell*cell);
      idx++;
    }
  }

  float brightness;
  for (int i = 0; i < (rows*cols); i++){
    brightness = (img_buffer[i]/3.0f) / 255.0f;
    img_buffer[i] = pick(brightness, ramp);
  }
  unsigned char* canvas = calloc(cell*cell*rows*cols, 1);
  if (canvas == NULL){
    free(img_buffer);
    free(ttf_buffer);
    stbi_image_free(data);
    printf("Failed to allocate memory for canvas\n");
    return 1;
  }
  for (int i = 0; i < n; i++){
    glyphs[i].pixels = stbtt_GetCodepointBitmap(&font, scale, scale, ramp[i],
                                                  &glyphs[i].w, &glyphs[i].h,
                                                  &glyphs[i].xoff, &glyphs[i].yoff);
  }

  int count;
  for (int row = 0; row < rows; row++){
    for (int col = 0; col < cols; col++){
      int num = row*cell;
      for (int i = 0; i < glyphs[img_buffer[row*cols+col]].h && i < cell; i++){
        int copy_w = (glyphs[img_buffer[row*cols+col]].w < cell) ? glyphs[img_buffer[row*cols+col]].w : cell;
        memcpy(&canvas[(num*cell*cols) + col*(cell)], &glyphs[img_buffer[row*cols+col]].pixels[i * glyphs[img_buffer[row*cols+col]].w], copy_w);
        num +=1;
      }
    }
  }
  for (int i = 0; i < n; i++){
    stbtt_FreeBitmap(glyphs[i].pixels, NULL);
  }
  stbi_write_jpg("rendered_im.jpg", cols*cell, rows*cell,1, canvas, 100);
  free(canvas);
  free(img_buffer);
  free(ttf_buffer);

  // printf("File successfully written");

  // float block_w = x / width;
  // float block_h = y / height;
  //
  // printf("Width: %d\n", x);
  // printf("Height: %d\n", y);
  // printf("Channels: %d\n", n);
  //
  // printf("block_w: %lf\n", block_w);
  // printf("block_h: %lf\n", block_h);
  //
  // for (int i = 0; i < x; ;){
  //   for (int j = 0; j < y; ;){
  //
  //   }
  //   i += block_h;
  // }


  stbi_image_free(data);
  return 0;
}
