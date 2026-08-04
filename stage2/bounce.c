#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(void){
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return -1;
  int width = w.ws_col;
  int height = w.ws_row;
  int curr_height = 1;
  int curr_width = 1;
  int velocity = -1;
  char arr[4] = {'|', '/', '-', '\\'};
  int itr = 1;
  int height_flag = 0;
  while(1){
    if (height_flag && curr_height == 1) height_flag = 0;
    if (curr_height == height){
      height_flag = 1;
    }
    if (curr_width == width || curr_width == 1){
      velocity = velocity * -1;
      curr_height += (height_flag == 0 ? 1 : -1);
    }
    printf("\033[%d;%dH%c",curr_height, curr_width, arr[itr%4]);
    fflush(stdout);
    usleep(150000);
    printf("\033[%d;%dH ", curr_height, curr_width);
    curr_width = curr_width + velocity;
    itr++;
  }


  return 0;
}
