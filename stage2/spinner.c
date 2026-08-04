#include <stdio.h>
#include <unistd.h>



int main(void){
  int itr = 0;
  char arr[4] = {'|', '/', '-', '\\'};
  while(1){
    printf("\033[H%c", arr[itr%4]);
    fflush(stdout);
    usleep(150000);
    itr++; 
  }


  return 0;
}
