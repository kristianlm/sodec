
#include <stdio.h>

#define ONE '#'
#define ZERO '.'

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
  int max_pulse_len = 0;

  char toggle_lenghts [ 19 ];
  char toggle_pos = 0;
  char toggle_proc = 0;

  char current = ONE;

  while((len = getrle(&c)) >= 0) {

    if(c != ZERO && c != ONE) {
      fprintf(stderr, "skipping '%c'\n", c);
      continue; // skip invalid chars
    }
    
    if(c == ONE) 
      if(toggle_pos % 2 != 1) {
	toggle_pos ++;
	fprintf(stderr, "toggle pos mismatch @ %d\n", toggle_pos);
      }
    else
      if(toggle_pos % 2 != 0) {
	toggle_pos ++;
	fprintf(stderr, "toggle pos mismatch @ %d for ZERO\n", toggle_pos);
      }

    toggle_lenghts[toggle_pos] = len;

    if(c == ONE) {
      if(len >= max_pulse_len) {
	max_pulse_len = len;
	toggle_proc = toggle_pos;	
	fprintf(stderr, "len now %d", max_pulse_len);
      }
    }
    if(c == ONE || c == ZERO) {
      toggle_lenghts[toggle_pos] = len;
    }
    toggle_pos++;
    toggle_pos = toggle_pos % sizeof(toggle_lenghts);
    printf("%c x %d\n", c, len);

    printf("\n");
    int i;
    for(i = 0 ; i < sizeof(toggle_lenghts) ; i++) {
      int size = toggle_lenghts[i];
      char sym = '.';
      if(i % 2 != 0) sym = '#';

      int j;
      for(j = 0 ; j < size ; j++)
	printf("%c", sym);
    }

  }

  printf("\n");
}
