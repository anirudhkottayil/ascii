#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(void){
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return -1;
  int limit = w.ws_col;
  int itr = 0;
  char arr[4] = {'|', '/', '-', '\\'};
  while(itr < limit){
    printf("\033[%d;%dH%c",1, itr + 1, arr[itr%4]);
    fflush(stdout);
    usleep(150000);
    printf("\033[%d;%dH ", 1, itr + 1);
    itr++; 
  }


  return 0;
}
