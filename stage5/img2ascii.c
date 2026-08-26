#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return ramp[index];
}

int main(void){
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return -1;
  int width = w.ws_col; int height = w.ws_row;
  printf("Screen Width: %d\n", width);
  printf("Screen Height: %d\n", height);

  const char* ramp = " .:-=+*#%@";
  int x,y,n;
  unsigned char *data = stbi_load("test1.jpg", &x, &y, &n, 0);
  if (data == NULL){
    printf("Failed to load image\n");
    return 1;
  }

  int r,g,b; int idx = 0;
  char* buffer = malloc(((x*y) + y + 1) * sizeof(char));
  if (buffer == NULL){
    printf("Malloc failed\n");
    return 1;
  }

  for (int i = 0; i < y;i++){
    for (int j = 0; j < x;){
      int offset = ((i*x)+j) * n; 
      r = data[offset];
      g = data[offset+1];
      b = data[offset+2];
      float brightness = ((r + g + b) / 3.0f) / 255.0f;
      char c = pick(brightness, ramp);
      buffer[idx++] = c;
      j += 3;
    }
    buffer[idx++] = '\n';
  }
  buffer[idx]= '\0';

  FILE *fptr;
  fptr = fopen("newasc.txt", "w");
  if (fptr == NULL){
    printf("Failed to open file");
    free(buffer);
    return 1;
  }
  fprintf(fptr, "%s", buffer);
  fclose(fptr);
  free(buffer);

  printf("File successfully written");

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
