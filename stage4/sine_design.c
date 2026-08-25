#include <stdio.h>
#include <string.h>
#include <math.h>

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return ramp[index];
}


void print_wave(const char *ramp, int width, float divisor, int mode) {
    for (int row = 0; row < 20; row++){
    float input = (float)row;
      for (int col = 0; col < width; col++) {
          if (mode == 1) input = (float) col;
          if (mode == 2) input = (float) col + row;
          float raw = sinf(input / divisor);          // -1 to 1
          float brightness = (raw + 1.0f) / 2.0f;    // remapped to 0 to 1
          putchar(pick(brightness, ramp));
      }
      putchar('\n');
  }
}

int main(void){
  const char* ramp = " .:-=+*#%@";
  print_wave(ramp, 100, 6, 0); //row (horizontal)
  print_wave(ramp, 100, 6, 1); //col (vertical)
  print_wave(ramp, 100, 6, 2); // row + col (diagonal)
  return 0;
}
