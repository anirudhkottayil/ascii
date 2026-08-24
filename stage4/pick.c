#include <stdio.h>
#include <string.h>

char pick(float brightness, const char* ramp){
  int n = strlen(ramp);
  int index = (int)(brightness * (n - 1) + 0.5f);
  if (index > n-1) index = n - 1;
  if (index < 0) index = 0;

  return ramp[index];
}


int main(){
  const char* ramp = " .:-=+*#%@";
  char c = pick(0.5, ramp);
  printf("%c\n", c);

  return 0;
}
