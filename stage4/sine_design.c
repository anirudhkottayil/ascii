#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define width 32
#define height 20

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return ramp[index];
}


void print_wave(const char *ramp, float divisor) {
    float time = 0.0f;
    int size = (height*width) + height + 1;
    char frame_buffer[size];
    printf("\033[2J");
    while (1){
    int idx = 0;
    printf("\033[H");
      for (int row = 0; row < 20; row++){
        for (int col = 0; col < width; col++) {
            float raw = sinf((col / divisor) + time) + sinf((row / divisor) + time);
            float brightness = (raw + 2.0f) / 4.0f;
            char c = pick(brightness, ramp);
            frame_buffer[idx++] = c;
        }
        frame_buffer[idx++] = '\n';
    }
    frame_buffer[idx] = '\0';
    write(0,frame_buffer, size);
    fflush(stdout);
    usleep(2000);
    time = time + 0.025f;
  }
}

int main(void){
  const char* ramp = " .:-=+*#%@";
  print_wave(ramp, 6);
  return 0;
}
