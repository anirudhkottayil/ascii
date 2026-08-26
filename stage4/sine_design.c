#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t got_sigint = 0;

void handler(int sig){
  got_sigint = 1;
}

#define width 32
#define height 20
#define size ((height*width*20) + height + 1)

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;
  return ramp[index];
}


void print_wave(const char *ramp, float divisor) {
    float time = 0.0f;
    int r = 1; int b = 0;
    char frame_buffer[size];
    printf("\033[2J");
    while (got_sigint == 0){
    int idx = 0;
    printf("\033[H");
      for (int row = 0; row < height; row++){
        for (int col = 0; col < width; col++) {
            float raw = sinf((col / divisor) + time) + sinf((row / divisor) + time);
            float brightness = (raw + 2.0f) / 4.0f;
            r = brightness * 255;
            b = 255 - r;
            char c = pick(brightness, ramp);
            int len = snprintf(&frame_buffer[idx], size - idx, "\033[38;2;%d;0;%dm", r, b);
            idx += len;
            frame_buffer[idx++] = c;
        }
        frame_buffer[idx++] = '\n';
    }
    frame_buffer[idx] = '\0';
    printf("%s",frame_buffer);
    fflush(stdout);
    usleep(2000);
    time = time + 0.025f;
  }
  printf("\033[0m\033[2J\033[H");
}

int main(void){
  signal(SIGINT, handler);
  const char* ramp = " .:-=+*#%@";
  print_wave(ramp, 6);
  return 0;
}
