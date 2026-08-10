#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>

volatile sig_atomic_t got_sigint = 0;

void handler(){
  got_sigint = 1;
}

int walker(void){
  struct winsize w;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) return -1;
  int width = w.ws_col; int height = w.ws_row;
  int curr_height = 1; int curr_width = 1;
  int itr = 0;
  char arr[4] = {'|', '/', '-', '\\'};
  int c;
  while ((c = getchar()) != 'q' && got_sigint == 0){
    if (c == 'w'){


    }

  }
  // while(itr < limit){
  //   printf("\033[%d;%dH%c",1, itr + 1, arr[itr%4]);
  //   fflush(stdout);
  //   usleep(150000);
  //   printf("\033[%d;%dH ", 1, itr + 1);
  //   itr++; 
  // }

  return 0;
}



void destroy(struct termios *original){
  tcsetattr(STDIN_FILENO, TCSANOW, original);
}

int main(void){
  signal(SIGINT, handler);
  struct termios raw, org;
  tcgetattr(STDIN_FILENO, &org);
  raw = org;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  walker();
  destroy(&org);

  return 0;
}
