
#include <stdio.h>


char getrle_first = 1;
int getrle_last;
int getrle(char* found) {
  short read;
  int length = 1;
  if(getrle_first)
    length = 0;
  if(getrle_last == EOF)
    return -1;

  while((read = getchar()) != EOF) {
    if(read != getrle_last && !getrle_first)
      break;

    getrle_last = (char)read;
    getrle_first = 0;
    length++;
  }

  *found = getrle_last;
  getrle_last = read;
  return length;
}

void main() {
  char c;
  int len;

  while((len = getrle(&c)) >= 0) {
    printf("%c x %d, ", c, len);
  }
  printf("\n");
}
