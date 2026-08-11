#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>

volatile sig_atomic_t got_sigint = 0;

void handler(int sig){
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
  printf("\033[2J");
  printf("\033[%d;%dH%c",curr_height, curr_width, arr[itr%4]);
  itr++;
  while (got_sigint == 0){
    c = getchar();
    if (c == 'q') return 0;
    if (c == 'w'){
      if (curr_height == 1) continue;
      printf("\033[%d;%dH ", curr_height, curr_width);
      printf("\033[%d;%dH%c",curr_height - 1, curr_width, arr[itr%4]);
      fflush(stdout);
      curr_height--;
    } else if (c == 'a'){
      if (curr_width == 1) continue;
      printf("\033[%d;%dH ", curr_height, curr_width);
      printf("\033[%d;%dH%c",curr_height, curr_width - 1, arr[itr%4]);
      fflush(stdout);
      curr_width--;
    } else if (c == 's'){
      if (curr_height == height) continue;
      printf("\033[%d;%dH ", curr_height, curr_width);
      printf("\033[%d;%dH%c",curr_height + 1, curr_width, arr[itr%4]);
      fflush(stdout);
      curr_height++;
    } else if (c == 'd'){
      if (curr_width == width) continue;
      printf("\033[%d;%dH ", curr_height, curr_width);
      printf("\033[%d;%dH%c",curr_height, curr_width + 1, arr[itr%4]);
      fflush(stdout);
      curr_width++;
    } else {
      continue;
    }
    usleep(150000);
    itr++;
  }
  return 0;
}



void destroy(struct termios *original){
  tcsetattr(STDIN_FILENO, TCSANOW, original);
}

int main(void){
  signal(SIGINT, handler);
  struct termios raw, org;
  int rc;
  rc = tcgetattr(STDIN_FILENO, &org);
  if (rc == -1) return 1;
  raw = org;
  raw.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);

  int rv = walker();
  destroy(&org);

  return 0;
}
